#include "SwitchAdapter.hpp"
#include "SwitchConnectionUtil.h"
#include "zsdn/Configs.h"
#include <Poco/Net/NetException.h>
#include "zsdn/proto/SwitchAdapter.pb.h"

SwitchAdapter::SwitchAdapter(
        const uint64_t switchId,
        std::shared_ptr<Poco::Net::SocketStream> socketStream,
        std::shared_ptr<Poco::Net::StreamSocket> switchSocket,
        const of_version_t ofVersion)
        :
        AbstractModule(
                zmf::data::ModuleUniqueId(zsdn::MODULE_TYPE_ID_SwitchAdapter, switchId),
                MODULE_VERSION,
                "SwitchAdapter",
                {}),
        instanceId_(switchId),
        ofVersion_(ofVersion),
        switchSocketStream_(socketStream),
        switchSocketRaw_(switchSocket),
        socketOpen_(true) { }


SwitchAdapter::~SwitchAdapter() {
    disable();
}


bool SwitchAdapter::enable() {

    int multicastGroups;
    bool flushStreamToSwitchEveryMessage;
    bool experimentalTopicGeneration;

    try {
        bool experimentalTopicGenerationRead = getZmf()->getConfigurationProvider()->getAsBoolean(
                zsdn::ZSDN_SWITCH_ADAPTER_EXPERIMENTAL_FAST_TOPIC_GENERATION, experimentalTopicGeneration);

        if (!experimentalTopicGenerationRead) {
            getLogger().error(
                    "Could not read config value " +
                    std::string(zsdn::ZSDN_SWITCH_ADAPTER_EXPERIMENTAL_FAST_TOPIC_GENERATION));
            return false;
        }


        bool configMulticastGroupsRead = getZmf()->getConfigurationProvider()->getAsInt(
                zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS, multicastGroups);

        if (!configMulticastGroupsRead) {
            getLogger().error(
                    "Could not read config value " + std::string(zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS));
            return false;
        }

        if (multicastGroups < 1 || multicastGroups > 256) {
            getLogger().error("ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS has to be in range [1,256)");
            return false;
        }

        bool configFlushStreamRead = getZmf()->getConfigurationProvider()->getAsBoolean(
                zsdn::ZSDN_SWITCH_ADAPTER_FLUSH_STREAM_AFTER_EVERY_MESSAGE, flushStreamToSwitchEveryMessage);

        if (!configFlushStreamRead) {
            getLogger().error(
                    "Could not read config value " +
                    std::string(zsdn::ZSDN_SWITCH_ADAPTER_FLUSH_STREAM_AFTER_EVERY_MESSAGE));
            return false;
        }
    } catch (Poco::Exception pe) {
        getLogger().error("failed to load configs: " + pe.message());
        return false;
    }
    multicastModulo_ = (uint8_t) multicastGroups;
    flushStreamToSwitchEveryMessage_ = flushStreamToSwitchEveryMessage;
    experimentalTopicGeneration_ = experimentalTopicGeneration;


    // set openflow version as first bit of additional state. this is used by the SwitchRegistryModule to know
    // the openFlow version without requesting it from the switch
    this->getZmf()->onModuleAdditionalStateChanged({static_cast<uint8_t>(ofVersion_)});

    // start thread for handling messages: switch -> ZMF
    this->switchListenThread_ = std::shared_ptr<std::thread>(
            new std::thread(&SwitchAdapter::handleOpenFlowFromSwitch, this));

    // subscribe to messages: ZMF -> Switch
    zmf::data::MessageType openFlowTopic = switchadapter_topics::TO().switch_adapter().switch_instance(
            this->instanceId_).openflow().build();

    getZmf()->subscribe(openFlowTopic,
                        [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId senderID) {
                            handleOpenFlowFromZmf(msg.getData(), senderID);
                        });
    getLogger().information("subscribed to " + openFlowTopic.toString());

    return true;
}


void SwitchAdapter::disable() {
    getLogger().information("Attempting shutdown of switchAdapter through SwitchAdapter::disable.");
    shutdownSwitchAdapter();

    if (switchListenThread_ != nullptr && switchListenThread_->joinable()) {
        getLogger().information("joining switchListenThread_ now");
        try {
            switchListenThread_->join();
            getLogger().information("joined switchListenThread_");
        } catch (...) {  // Catch all
            getLogger().warning("Failed to join switchListenThread_");
        }
    }
}

zmf::data::ZmfOutReply SwitchAdapter::handleRequest(const zmf::data::ZmfMessage& message,
                                                    const zmf::data::ModuleUniqueId& sender) {
    SwitchAdapter_Proto::Request request;
    bool parseSuccess = request.ParseFromString(message.getData());
    if (!parseSuccess) {
        getLogger().warning("For Request from " + sender.getString() + " received invalid ProtoBuffer request format.");
        return zmf::data::ZmfOutReply::createNoReply();
    }
    switch (request.RequestMsg_case()) {
        case SwitchAdapter_Proto::Request::kHandleOpenflowMessagesRequest: {
            for (const std::string& ofMessage : request.handle_openflow_messages_request().open_flow_message()) {
                handleOpenFlowFromZmf(ofMessage, sender);
            }
            SwitchAdapter_Proto::Reply reply;
            reply.set_allocated_handle_openflow_messages_reply(
                    new SwitchAdapter_Proto::Reply_HandleOpenflowMessagesReply());
            zmf::data::ZmfMessage msg(handleOpenflowMessagesReplyTopic_, reply.SerializeAsString());
            return zmf::data::ZmfOutReply::createImmediateReply(msg);
        }
        case SwitchAdapter_Proto::Request::REQUESTMSG_NOT_SET:
            getLogger().warning("Received incomplete request from" + sender.getString() + " (REQUESTMSG_NOT_SET)");
            return zmf::data::ZmfOutReply::createNoReply();

        default:
            getLogger().information("Received unknown Request");
            return zmf::data::ZmfOutReply::createNoReply();

    }
}


void SwitchAdapter::handleOpenFlowFromZmf(const std::string& messageData,
                                          const zmf::data::ModuleUniqueId& sender) {


    if (socketOpen_) {
        getLogger().trace("received event from zmf");
    } else {
        getLogger().warning("Ignoring message, socket closed");
        return;
    }

    // if the message data is shorter, it can not even be a OpenFlow message.
    if (messageData.size() >= SwitchConnectionUtil::OF_HEADER_LENGTH) {

        uint8_t messageType = of_message_type_get((uint8_t*) messageData.data());

        switch (messageType) {
            case OF_OBJ_TYPE_HELLO: {
                getLogger().warning(
                        "Some other module sent a HELLO message for me to send to this switch, ignored it");
                break;
            }
            case OF_OBJ_TYPE_ECHO_REQUEST: {
                getLogger().warning(
                        "Some other module sent an ECHO_REQUEST message for me to send to this switch, ignored it");
                break;
            }
            case OF_OBJ_TYPE_ECHO_REPLY: {
                getLogger().warning(
                        "Some other module sent an ECHO_REPLY message for me to send to this switch, ignored it");
                break;
            }
            case OF_OBJ_TYPE_ROLE_REQUEST: {
                getLogger().warning(
                        "Some other module sent a ROLE_REQUEST message for me to send to this switch, ignored it so the system stays in working condition!");
                break;
            }

                // TODO
            default: {
                uint16_t ofLength = of_message_length_get((of_message_t) messageData.data());
                if (messageData.size() == ofLength) {

                    // need to lock since the switchListenThread could also currently write data to the switch
                    if (getLogger().trace()) {
                        getLogger().trace("Writing " + std::to_string(messageData.size()) + "bytes to socket");
                    }

                    bool writeSuccess;
                    this->switchStreamMutex_.lock();
                    try {

                        writeSuccess = SwitchConnectionUtil::writeOpenFlowToSocket(*switchSocketStream_,
                                                                                   (uint8_t*) (messageData.data()));
                        if (flushStreamToSwitchEveryMessage_ && writeSuccess) {
                            switchSocketStream_->flush();
                            writeSuccess = switchSocketStream_->good();
                        }
                    } catch (Poco::TimeoutException te) {
                        writeSuccess = false;
                        getLogger().error(te.message());
                    } catch (Poco::Net::NetException ne) {
                        writeSuccess = false;
                        getLogger().error(ne.message());
                    }
                    this->switchStreamMutex_.unlock();

                    if (writeSuccess) {
                        getLogger().trace("Sent message to switch");
                    } else {
                        getLogger().information("zmq event dispatch thread attempting shutdown of SwitchAdapter.");
                        shutdownSwitchAdapter();
                    }

                    break;
                } else {
                    getLogger().warning("Received message for switch with incorrect length - OF message length is " +
                                        std::to_string(ofLength) + " while zmf msg length is " +
                                        std::to_string(messageData.size()) + ". ignoring message.");
                }
            }
        }

    } else {
        getLogger().warning(
                "Got OpenFlow-message from ZMF which is shorter than 8 Byte - ignoring, most probably corrupted/defective");
    }


}

void SwitchAdapter::handleOpenFlowFromSwitch() {

    // buffer which is constantly reused and filled with one openflow message after each sucessful read from the switch socket
    uint8_t* receiveBuffer = new uint8_t[65536];
    of_object_t* message = nullptr;
    uint8_t messageType;
    bool ioSuccess = false;

    /// Echo reply is used often, we allocate one for reuse here.
    of_echo_reply_t* echoReply = of_echo_reply_new(this->ofVersion_);

    // As long as the connection to the switch is alive
    while (this->socketOpen_) {

        try {
            ioSuccess = SwitchConnectionUtil::parseOpenFlowFromSocketToBuffer(receiveBuffer, *switchSocketStream_);
        } catch (Poco::TimeoutException te) {
            ioSuccess = false;
            getLogger().error(te.message());
        } catch (Poco::Net::NetException ne) {
            ioSuccess = false;
            getLogger().error(ne.message());
        }

        if (!ioSuccess) {
            break;
        }


        messageType = of_message_type_get(receiveBuffer);
        message = of_object_new_from_message(receiveBuffer, of_message_length_get(receiveBuffer));
        // React to the OpenFlow packet according to its type.
        // Filters Hello, Echo Reply and Echo Request messages (respond to the latter).
        // Everything else wil be forwarded to ZMF.
        switch (messageType) {

            case OF_OBJ_TYPE_HELLO:   // ignore hello messages, do not forward message to ZMF
                getLogger().warning("received unexpected OF_HELLO message.");
                break;

            case OF_OBJ_TYPE_ECHO_REPLY: // do not forward echo reply message to ZMF
                getLogger().trace("received OF_ECHO_REPLY message.");
                break;

            case OF_OBJ_TYPE_ECHO_REQUEST:  // answer with an ECHO_REPLY message
            {
                // need to lock since the ZMF event thread could also currently write data to the switch
                this->switchStreamMutex_.lock();
                // always flush so that the connection won't be terminated due to a delayed ECHO_REPLY
                try {
                    ioSuccess = SwitchConnectionUtil::writeOpenFlowToSocket(*switchSocketStream_, echoReply);

                    if (ioSuccess) {
                        switchSocketStream_->flush();
                        ioSuccess = switchSocketStream_->good();
                    }

                } catch (Poco::TimeoutException te) {
                    ioSuccess = false;
                    getLogger().error(te.message());
                } catch (Poco::Net::NetException ne) {
                    ioSuccess = false;
                    getLogger().error(ne.message());
                }
                this->switchStreamMutex_.unlock();

                if (ioSuccess) {
                    getLogger().trace("received OF_ECHO_REQUEST, responded with OF_ECHO_REPLY.");
                    getZmf()->publish(
                            zmf::data::ZmfMessage(echoRequestTopic_, receiveBuffer, (size_t) message->length));
                    getLogger().trace("Published ECHO_REQUEST.");
                }
                break;
            }
            case OF_OBJ_TYPE_PACKET_IN: // packet in are published to ZMF with detailed topics
            {
                messageCount++;
                packetInTopic_.trimToLength(5); // TODO hardcoded trim

                packetInTopic_.appendMatch8((uint8_t) (messageCount % multicastModulo_));

                if (experimentalTopicGeneration_) {
                    SwitchConnectionUtil::buildSimplePacketInTopic(packetInTopic_, message);
                } else {
                    SwitchConnectionUtil::buildPacketInTopic(packetInTopic_, message, instanceId_);
                }


                getZmf()->publish(zmf::data::ZmfMessage(packetInTopic_, receiveBuffer, (size_t) message->length));
                if(getLogger().trace()) {
                    getLogger().trace("Published OF_PACKET_IN to " + packetInTopic_.toString());
                }
                break;
            }
            default: // any other openFlow message type.
            {
                MessageType openFlowTopic = switchadapter_topics::FROM()
                        .switch_adapter().openflow().custom_openflowtype(messageType).build();

                getZmf()->publish(zmf::data::ZmfMessage(openFlowTopic, receiveBuffer, (size_t) message->length));
                getLogger().trace("Published Message of type " + std::to_string(messageType) + ".");
                break;
            }
        }

        // frees the buffer so we can safely delete the struct pointed to by the message pointer.
        // (else we would delete our buffer)
        of_object_wire_buffer_steal(message, &receiveBuffer);
        of_object_delete(message);

        if (!ioSuccess) {
            break;
        }
    }

    // we either ended up here since the connection broke, or the connection was terminated on purpose.
    // Since ending up here means that the SwitchAdapter cannot function anymore (read from Socket is impossible)
    // we need to shutdown the ZMF instance ourselves, but only if no one else already did that.
    getLogger().information("swichListenThread_ attempting shutdown of SwitchAdapter.");
    shutdownSwitchAdapter();

    of_echo_reply_delete(echoReply);
    delete[] receiveBuffer;

    getLogger().information("switchListenThread terminates.");
}

void SwitchAdapter::shutdownSwitchAdapter() {
    bool open = true;

    // mutex so only one thread can ever succeeed to shutdown the switch socket.
    if (socketOpen_.compare_exchange_strong(open, false)) {
        getLogger().information("Initiating shutdown of SwitchAdapter.");
        this->getZmf()->requestStopInstance();
        getLogger().information("Request issued. Closing socket");
        try {
            switchSocketRaw_->shutdown();
            switchSocketRaw_->close();
            getLogger().information("Socket Closed.");
        } catch (...) {
            getLogger().error("Unknown exception when closing socket");
        }
    } else {
        getLogger().information(
                "Thread will silently shutdown, someone else already initiated the shutdown.");
    }
}
