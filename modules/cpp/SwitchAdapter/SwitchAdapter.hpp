#ifndef SWITCH_ADAPTER_H
#define SWITCH_ADAPTER_H

#include <zmf/AbstractModule.hpp>
#include <thread>
#include <loci/loci_base.h>
#include <Poco/Net/SocketStream.h>
#include <zsdn/topics/SwitchAdapterTopics.hpp>
#include "ModuleTypeIDs.hpp"
#include "Poco/Net/ServerSocket.h"

/**
 * @brief   A Module representing one OpenFlow switch.
 *
 * @details The SwitchAdapter wraps each OpenFlow capable switch as a ZMF Module instance.
 *          An OpenFlow switch connects to a SwitchAdapter instance as it would normally connect to a controller.
 *
 * @author  Alexander Kicherer
 * @author  Andre Kutzleb
 * @author  Matthias Hoppe
 *
 * @date    28.06.2015
 *
 * @date created on 28.6.2015
 * @remark  Refactored / rewritten by Andre Kutzleb on 18.07.2015
 *
 * @remark IMPROVEMENT: Consider making nagles algorithm toggleable to optimize throughput to the switch
 * @remark IMPROVEMENT: only passively answers echos and never times out currently.
 * @remark IMPROVEMENT  messages from zmf to switch: default is also the case for "illegal"(unknown/garbage)
 *                      openFlow messages. may have to filter those as well (unknown type)
 */
class SwitchAdapter : public zmf::AbstractModule {

public:
    /**
     * Constructs (but does not start) A SwitchAdapter.
     *
     * @param switchId
     *        the switchId of the switch represented by this SwitchAdapter.
     * @param socketStream
     *        the stream connected to the socket socket representing the connection to the switch.
     * @param switchSocket
     *        the socket representing the connection to the switch.
     * @param ofVersion
     *        the OpenFlow version used for communicating with the switch.
     */
    SwitchAdapter(
            const uint64_t switchId,
            std::shared_ptr<Poco::Net::SocketStream> socketStream,
            std::shared_ptr<Poco::Net::StreamSocket> switchSocket,
            const of_version_t ofVersion);

    /**
     * Destructor: In case this module is not correctly disabled by ZMF,
     * disable is called so that no socket or thread is left behind by the SwitchAdapter.
     */
    ~SwitchAdapter();


    /**
     * Handles a ZMF request message - handles bundle deploy.
     *
     * @param message the received ZMF message
     * @param sender the ID of the sender of the received ZMF message
     *
     * @return a @ZmfOutReply to the message
     */
    virtual zmf::data::ZmfOutReply handleRequest(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender) override;

protected:
    /**
     * starts a subscription for topic TO.SWITCH_ADAPTER.SWITCH_INSTANCE(this.switchId).OPENFLOW.*
     * which will be handled by handleOpenFlowFromZmf.
     *
     * Also starts a thread listing on the connection to the switch.
     *
     * @return true of enable successful, false if the socket to the switch is closed or startup otherwise failed.
     */
    virtual bool enable();

    /**
    * Closes the socket to the switch and stops the thread listening on the connection from the switch.
    */
    virtual void disable();

private:

    /// Flag used so switchListenThread_ and zmf event thread can shutdown the switchadapter without conflicts (double close, or send after close)
    std::atomic_bool socketOpen_;

    /// the byte in the topic after packetIN is totalMsgCount % multicastModulo_ - enabling loadbalancing for modules such as forwarding.
    uint8_t multicastModulo_;

    /// Switch to zmf message counter, used for distributing packetIns across multiple multicastgroups. can overflow.
    uint32_t messageCount = 0;

    // SwitchAdapter topics builder for switch thread (reading from switch)
    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType> switchThreadTopics_;
    // SwitchAdapter topics builder for zmf threads (e.g sending to switch)
    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType> zmfThreadTopics_;

    /// EchoRequest topic
    zmf::data::MessageType echoRequestTopic_ = zmfThreadTopics_.from().switch_adapter().openflow().echo_request().build();

    /// HandleOpenflowMessages reply topic
    zmf::data::MessageType handleOpenflowMessagesReplyTopic_ = zmfThreadTopics_.reply().switch_adapter().openflow().handle_openflow_message().build();

    /// packetInTopic which is reused for every publish - it gets reset to this length before every publish and individually rebuilt to represent its content
    zmf::data::MessageType packetInTopic_ = zmfThreadTopics_.from().switch_adapter().openflow().packet_in().build();

    /// This modules version. Behaviour and other things might be different in other versions of this module.
    static const uint16_t MODULE_VERSION = 0;

    /// the ID of the SwitchAdapter will be stored here (this is equal to the switchId)
    uint64_t instanceId_;

    /// the OpenFlow version the SwitchAdapter uses to communicate with its switch
    of_version_t ofVersion_;

    /**
     * if set to true, SwitchAdapters will flush the socket after every sent message.
     * flushing may affect throughput negatively but can decrease latency.
     * Flushing also prevents messages from waiting in the queue when no message follow after it.
     */
    bool flushStreamToSwitchEveryMessage_;

    /**
     * If true, a fast but possibly wrong result yielding function is used to generate the topics for packetIn publishs.
     */
    bool experimentalTopicGeneration_;

    // the Stream connected to the switch socket
    std::shared_ptr<Poco::Net::SocketStream> switchSocketStream_;

    // the Socket to the switch this SwitchAdapter is connected to
    std::shared_ptr<Poco::Net::StreamSocket> switchSocketRaw_;


    /// the thread listening on the connection from switch
    std::shared_ptr<std::thread> switchListenThread_;

    /**
     * the mutex used for synchronising writes to the stream to the switch.
     * This is required because the switchListenThread will send EchoReply messages
     * to the socket which should never collides with the writes of the
     * ZMF event handler thread (the ZMF-thread writing received OpenFlow messages to the switch).
     */
    std::mutex switchStreamMutex_;

    /**
     * This function is called when ZMF received a message directed at this switch instance.
     * The payload is always expected to be an OpenFlow message.
     * HELLO, ECHO_REQUEST, ECHO_REPLY and ROLE_REQUEST are filtered out, all other
     * OpenFlow messages are forwarded to the switch.
     *
     * @param message received OpenFlow message.
     * @param sender the sender where the message came from
     */
    void handleOpenFlowFromZmf(const std::string& messageData, const zmf::data::ModuleUniqueId& sender);

    /**
     * The switchListenThread will listen on the connection from the switch.
     *
     * Depending on the kind of OpenFlow message received, the following happens:
     *
     * HELLO and ECHO_REPLY messages are ignored.
     *
     * ECHO_REQUEST messages are answered with a ECHO_REPLY and also published to zmf.
     *
     * PACKET_IN messages are dissected - depending on their payload they are published to specific topics
     * to the ZMF. (e.g. [...].PACKET_IN.IPv4.TCP)
     *
     * All other OpenFlow messages are published to the ZMF with only their type added to the topic,
     * e.g. [...].OPENFLOW.ERROR
     *
     */
    void handleOpenFlowFromSwitch();

    /**
     * Gracefully shuts down the switch adapter. can be called by any thread, but is guaranteed to only
     * shutdown all resources once.
     */
    void shutdownSwitchAdapter();
};


#endif // SWITCH_ADAPTER_H
