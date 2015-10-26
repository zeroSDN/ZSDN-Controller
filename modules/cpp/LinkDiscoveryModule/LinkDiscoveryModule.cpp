//
// Created by Matthias Strljic, Andre Kutzleb on 6/28/15.
//

#include "LinkDiscoveryModule.hpp"
#include <iostream>
#include "zsdn/topics/LinkDiscoveryModule_topics.hpp"
#include "zsdn/proto/LinkDiscoveryModule.pb.h"
#include <algorithm>
#include <zsdn/Configs.h>
#include <zsdn/proto/SwitchRegistryModule.pb.h>
#include <zsdn/topics/SwitchRegistryModule_topics.hpp>
#include <zsdn/topics/SwitchAdapter_topics.hpp>

#include <RequestUtils.h>


LinkDiscoveryModule::LinkDevice::InsertResult LinkDiscoveryModule::LinkDevice::insertLink
        (uint32_t port, std::shared_ptr<LinkDevice> linkedDevice, uint32_t endpointPort, uint64_t updateTime) {
    dataMutex.lock();
    InsertResult result;
    if (this->availablePorts_.empty()) {
        result = InsertResult::NO_VALID_PORT;
    } else {
        std::vector<uint32_t>::iterator existingPort =
                std::find(this->availablePorts_.begin(), this->availablePorts_.end(), port);
        // Check the port
        if (existingPort != this->availablePorts_.end()) {
            if (this->activeLinks_.count(port) > 0) { ;
                // if the element already exist just refill the struct
                if ((this->activeLinks_.find(port)->second.device->getId() == linkedDevice->getId()) &&
                    this->activeLinks_.find(port)->second.connectionPort == endpointPort) {
                    result = InsertResult::SUCCESS_NO_CHANGE;
                } else {
                    result = InsertResult::SUCCESS_MODIFY;
                }
                this->activeLinks_.find(port)->second.device = linkedDevice;
                this->activeLinks_.find(port)->second.connectionPort = endpointPort;
                this->activeLinks_.find(port)->second.timestamp = updateTime;

            } else {
                // insert new element to the map
                LinkDeviceTimestampTupel tupel;
                tupel.device = linkedDevice;
                tupel.timestamp = updateTime;
                tupel.connectionPort = endpointPort;
                this->activeLinks_.insert(std::pair<uint32_t, LinkDeviceTimestampTupel>
                                                  (port, tupel));
                result = InsertResult::SUCCESS_NEW;
            }

        } else {
            // Port does not exist
            result = InsertResult::NO_VALID_PORT;
        }
    }
    dataMutex.unlock();
    return result;
};

std::map<uint32_t, std::pair<std::shared_ptr<LinkDiscoveryModule::LinkDevice>, uint32_t>>
LinkDiscoveryModule::LinkDevice::eraseLinkToDevice(uint64_t deviceId) {
    std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>> result;
    dataMutex.lock();
    for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator it = this->activeLinks_.begin();
         it != this->activeLinks_.end();
         ++it) {
        if (it->second.device->getId() == deviceId) {
            result.insert(std::pair<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>(
                    it->first,
                    std::pair<std::shared_ptr<LinkDevice>, uint32_t>(it->second.device, it->second.connectionPort)));
        }
    }

    for (std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>::iterator deleteIt = result.begin();
         deleteIt != result.end(); ++deleteIt) {
        this->activeLinks_.erase(deleteIt->first);
    }
    dataMutex.unlock();
    return result;
};

void LinkDiscoveryModule::LinkDevice::flushOutDatedLinks
        (uint64_t borderDate, std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>& outDatedLinks) {
    dataMutex.lock();
    for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator it = this->activeLinks_.begin();
         it != this->activeLinks_.end();
         ++it) {
        if (it->second.timestamp < borderDate) {
            outDatedLinks.insert(std::pair<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>(
                    it->first,
                    std::pair<std::shared_ptr<LinkDevice>, uint32_t>(it->second.device, it->second.connectionPort)));
        }
    }
    for (std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>::iterator oIt = outDatedLinks.begin();
         oIt != outDatedLinks.end();
         ++oIt) {
        this->activeLinks_.erase(oIt->first);
    }

    dataMutex.unlock();
};

LinkDiscoveryModule::LinkDiscoveryModule(uint64_t instanceId) : AbstractModule(
        zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
        MODULE_VERSION, "LinkDiscoveryModule",
        {{SWITCH_REGISTRY_MODULE_DEP_TYPE, SWITCH_REGISTRY_MODULE_DEP_VERSION}}),
                                                                instanceId_(instanceId),
                                                                hashCounter_(0),
                                                                timeOutBorder_(0),
                                                                updateCycle(std::chrono::milliseconds(1000)) {
};

LinkDiscoveryModule::~LinkDiscoveryModule() {
};


uint64_t LinkDiscoveryModule::getNewHash() { return ++hashCounter_; }

bool LinkDiscoveryModule::setupSubscription() {
    bool result;
    try {

        getLogger().information("Start setup subscriptions.");


        int multicastGroups;

        try {
            bool configMulticastGroupsRead = getZmf()->getConfigurationProvider()->getAsInt(
                    zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS, multicastGroups);

            if (!configMulticastGroupsRead) {
                getLogger().error(
                        "Could not read config value " +
                        std::string(zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS));
                return false;
            }

            if (multicastGroups < 1 || multicastGroups > 256) {
                getLogger().error("ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS has to be in range [1,256)");
                return false;
            }

        } catch (Poco::Exception pe) {
            getLogger().error("failed to load configs: " + pe.message());
            return false;
        }

        for (int i = 0; i < multicastGroups; i++) {
            zmf::data::MessageType subscribeOFPacketInTopic_ = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().multicast_group(
                    i).custom_protocol(
                    LINK_DISCOVERY_MESSAGE_PAYLOAD_TYPE).build();
            this->getZmf()->subscribe(subscribeOFPacketInTopic_,
                                      [this](const zmf::data::ZmfMessage& msg, const ModuleUniqueId& sender) {
                                          this->handlePacketIn(msg, sender);
                                      });

        }
        getLogger().trace("Setup OpenFLow::PacketIn subscription.");
        this->getZmf()->subscribe(subscribeSwitchStateChangeTopic_,
                                  [this](const zmf::data::ZmfMessage& msg, const ModuleUniqueId& sender) {
                                      this->handleSwitchStateChanged(msg);
                                  });
        getLogger().trace("Setup SwitchRegistry::SwitchStateChange subscription.");
        result = true;
        getLogger().information("Setup all subscription.");
    } catch (Poco::Exception e) {
        std::string trace = e.what();
        getLogger().fatal("Error during subscription. What: " + trace);
        result = false;
    } catch (...) {
        getLogger().fatal("Error during subscription.");
        result = false;
    }
    return result;
}

bool LinkDiscoveryModule::setupDatabase() {
    bool result;
    getLogger().information("Starting to Request GetAllSwitches from a SwitchRegistryModule.");

    SwitchRegistryModule_Proto::Request request;

    SwitchRegistryModule_Proto::Request_GetAllSwitchesRequest* getAllSwitchesRequest =
            new SwitchRegistryModule_Proto::Request_GetAllSwitchesRequest();
    request.set_allocated_get_all_switches_request(getAllSwitchesRequest);

    SwitchRegistryModule_Proto::Reply reply;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            request,
                                            reply,
                                            sendGetAllSwitchesRequestTopic_,
                                            zsdn::MODULE_TYPE_ID_SwitchRegistryModule,
                                            SWITCH_REGISTRY_MODULE_DEP_VERSION);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {

            int counter = 0;
            for (int i = 0; i < reply.get_all_switches_reply().switches_size(); i++) {
                std::vector<uint32_t> ports;
                for (int portCount = 0;
                     portCount < reply.get_all_switches_reply().switches(i).switch_ports_size();
                     portCount++) {
                    ports.push_back(reply.get_all_switches_reply()
                                            .switches(i)
                                            .switch_ports(portCount)
                                            .attachment_point()
                                            .switch_port());
                }
                this->addLinkDevice(ports, reply.get_all_switches_reply().switches(i).switch_dpid(),
                                    static_cast<of_version_t>(reply.get_all_switches_reply()
                                            .switches(i).openflow_version()));
                counter++;
            }
            getLogger().information("Added " + std::to_string(counter) + " Switches to the LinkDiscover list.");
            result = true;
        }
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND: {
            std::string logMsg2 = "";
            std::string logMsg = "Request aborted. Currently no modules with the type "
                    "MODULE_TYPE_ID_SwitchRegistryModule are available.";
            getLogger().warning(logMsg);
            result = false;
        }

            break;
        case zsdn::RequestUtils::TIMEOUT: {
            getLogger().warning("Timeout when waiting for all Switches.");
            result = false;
        }

            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            getLogger().fatal("Not able to serialize request.");
            result = false;
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            getLogger().fatal("Failed to parse GetAllSwitchesReply.");
            result = false;
            break;
        default :
            getLogger().fatal("Default case, should never happen.");
            result = false;

    }
    return result;
}

void LinkDiscoveryModule::handleSwitchStateChanged(const zmf::data::ZmfMessage& changeMsg) {
    getLogger().trace("Recieved SwitchStateChanged event.");
    SwitchRegistryModule_Proto::From eltonJohn;

    bool parseOk = eltonJohn.ParseFromArray(changeMsg.getDataRaw(), changeMsg.getDataLength());
    if (parseOk) {
        const SwitchRegistryModule_Proto::From_SwitchEvent& switchEvent = eltonJohn.switch_event();
        switch (switchEvent.SwitchEventType_case()) {

            case SwitchRegistryModule_Proto::From_SwitchEvent::kSwitchAdded: {
                getLogger().trace("Process SwitchAddedEvent.");
                std::vector<uint32_t> ports;
                for (int counter = 0; counter < switchEvent.switch_added().switch_ports_size(); counter++) {
                    ports.push_back(switchEvent.switch_added().switch_ports(counter).attachment_point().switch_port());
                }
                this->addLinkDevice(ports, switchEvent.switch_added().switch_dpid(),
                                    static_cast<of_version_t>(switchEvent.switch_added().openflow_version()));
                getLogger().information("Added new LinkDevice with Id: " +
                                        std::to_string(switchEvent.switch_added().switch_dpid()) +
                                        " and " + std::to_string(ports.size()) + " ports.");
            }

                break;

            case SwitchRegistryModule_Proto::From_SwitchEvent::kSwitchRemoved: {
                if (this->removeLinkDevice(switchEvent.switch_removed().switch_dpid())) {
                    getLogger().information("Removed LinkDevice with Id: " +
                                            std::to_string(switchEvent.switch_removed().switch_dpid()));
                } else {
                    getLogger().information("Try to remove not existing LinkDevice with ID: " +
                                            std::to_string(switchEvent.switch_removed().switch_dpid())
                    );
                }
            }

                break;

            case SwitchRegistryModule_Proto::From_SwitchEvent::kSwitchChanged:
                break;

            case SwitchRegistryModule_Proto::From_SwitchEvent::SWITCHEVENTTYPE_NOT_SET:
                getLogger().warning("Recieved SwitchStateChanged event with unset type.");
                break;
        }
    } else {
        getLogger().warning("Recieved not parseable SwitchStateChanged message.");
    }
};

void LinkDiscoveryModule::handleLinkRemoved(uint64_t deviceId,
                                            std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>& removedLinks) {
    getLogger().trace("Start publishing " + std::to_string(removedLinks.size()) + " LinkRemoved events.");
    for (std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>::iterator remIt = removedLinks.begin();
         remIt != removedLinks.end();
         ++remIt) {
        LinkDiscoveryModule_Proto::From fromMsg;
        LinkDiscoveryModule_Proto::From_SwitchLinkEvent* linkEvent =
                new LinkDiscoveryModule_Proto::From_SwitchLinkEvent();
        common::topology::SwitchToSwitchLink* ssLink = new common::topology::SwitchToSwitchLink();
        getLogger().trace("Publishing LinkRemoved event from LinkDevice with ID: " + std::to_string(deviceId) +
                          " at port " + std::to_string(remIt->first) + " to LinkDevice with the ID " +
                          std::to_string(remIt->second.first->getId()) +
                          " at port " + std::to_string(remIt->second.second));
        this->fillSwitchToSwitchLink(ssLink, deviceId, remIt->first, remIt->second.first->getId(),
                                     remIt->second.second);
        linkEvent->set_allocated_switch_link_removed(ssLink);
        fromMsg.set_allocated_switch_link_event(linkEvent);
        getZmf()->publish(zmf::data::ZmfMessage(publishSwitchLinkChangeRemovedTopic_, fromMsg.SerializeAsString()));
    }

};

void LinkDiscoveryModule::handleLinkAdded(uint64_t deviceId, uint32_t devicePort, uint64_t endpointId,
                                          uint32_t endpointPort) {

    LinkDiscoveryModule_Proto::From fromMsg;
    LinkDiscoveryModule_Proto::From_SwitchLinkEvent* linkEvent = new LinkDiscoveryModule_Proto::From_SwitchLinkEvent();
    common::topology::SwitchToSwitchLink* ssLink = new common::topology::SwitchToSwitchLink();
    getLogger().trace("Start publishing LinkAdded events. From LinkDevice with ID " + std::to_string(deviceId) +
                      " at port " + std::to_string(devicePort) +
                      " to LinkDevice with the ID " + std::to_string(endpointId) +
                      " at port " + std::to_string(endpointPort));
    this->fillSwitchToSwitchLink(ssLink, deviceId, devicePort, endpointId, endpointPort);
    linkEvent->set_allocated_switch_link_added(ssLink);
    fromMsg.set_allocated_switch_link_event(linkEvent);
    getZmf()->publish(zmf::data::ZmfMessage(publishSwitchLinkChangeAddTopic_, fromMsg.SerializeAsString()));
}

void LinkDiscoveryModule::handlePacketIn(const zmf::data::ZmfMessage& message,
                                         const zmf::data::ModuleUniqueId& sender) {
    getLogger().trace("Recieved PacketIn message from module with ID " + std::to_string(sender.InstanceId));
    // Unpack ZmfMessage which contains OpenFlow packet
    of_packet_in_t* ofPacketIn = zsdn::of_object_new_from_data_string_copy(message.getData());

    of_octets_t ofPayload;
    of_packet_in_data_get(ofPacketIn, &ofPayload);

    uint32_t port;
    uint64_t senderId = sender.InstanceId;
    bool anyError = false;
    switch (ofPacketIn->version) {
        case OF_VERSION_1_0:
        case OF_VERSION_1_1:

            of_packet_in_in_port_get(ofPacketIn, &port);
            break;

        case OF_VERSION_1_2:
        case OF_VERSION_1_3:
        case OF_VERSION_1_4: {
            of_match_t match;
            anyError = (-1 == of_packet_in_match_get(ofPacketIn, &match));
            port = match.fields.in_port;
            break;
        }
        default:
            of_packet_in_delete(ofPacketIn);
            anyError = true;
    }

    if (!anyError) {
        getLogger().trace("Recieved valid PacketIn message.");
        uint8_t* payloadData = ofPayload.data;
        uint16_t payloadLength = ofPayload.bytes;

        Tins::EthernetII ethPacket(payloadData, payloadLength);


        if ((!ethPacket.src_addr().is_broadcast()) && (!ethPacket.dst_addr().is_broadcast()) &&
            ethPacket.payload_type() == LINK_DISCOVERY_MESSAGE_PAYLOAD_TYPE) {
            LinkDiscoveryModule_Proto::LinkDiscoveryMessage linkMessage;
            linkMessage.ParseFromArray(ethPacket.inner_pdu()->serialize().data(),
                                       ethPacket.inner_pdu()->serialize().size());
            this->handleDiscoverPacketIn(senderId, linkMessage.uniqueid(), linkMessage.sentimestamp(), port);
        }
        of_packet_in_delete(ofPacketIn);

    } else {
        getLogger().warning("Recieved not valid  PacketIn message.");
    }


}

void LinkDiscoveryModule::handleDiscoverPacketIn(uint64_t deviceId,
                                                 uint64_t hashCode,
                                                 uint64_t timestamp,
                                                 uint32_t inPort) {
    getLogger().trace("Start processing discover message message. Reciever ID: " + std::to_string(deviceId) +
                      " , Hash code: " + std::to_string(hashCode) +
                      " , Timestamp: " + std::to_string(timestamp) +
                      " , recieved port: " + std::to_string(inPort));
    if (timestamp >= timeOutBorder_) {
        DiscoverDevice recievedEndpoint;
        std::shared_ptr<LinkDevice> startPoint;
        std::shared_ptr<LinkDevice> endPoint;
        bool existActiveRequest = tryToGetActiveRequest(hashCode, recievedEndpoint);
        bool existStartPointLink = false;
        bool existEndPointLink = false;
        if (existActiveRequest) {
            existEndPointLink = tryToGetLinkDevice(deviceId, endPoint);
            existStartPointLink = tryToGetLinkDevice(recievedEndpoint.deviceId_, startPoint);
        }

        if (existActiveRequest &&
            existEndPointLink &&
            existStartPointLink) {
            uint64_t tempTimestamp = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
            switch (startPoint->insertLink(recievedEndpoint.discoverPort_,
                                           endPoint,
                                           inPort,
                                           tempTimestamp)) {

                case LinkDevice::NO_VALID_PORT:
                    this->getLogger().warning(
                            "Recieved corrupted message, described port of message does not exist on device.");
                    break;
                case LinkDevice::SUCCESS_NEW:
                    getLogger().trace("Added new Link");
                    this->handleLinkAdded(startPoint->getId(),
                                          recievedEndpoint.discoverPort_,
                                          endPoint->getId(),
                                          inPort);

                    break;
                case LinkDevice::SUCCESS_MODIFY:
                    getLogger().trace("Updated Link");
                    // TODO send update notification link has changed; Remove + add?
                    break;
                case LinkDevice::SUCCESS_NO_CHANGE:
                    getLogger().trace("Keep alive Link");
                    // TODO anything to do?
                    break;
            }

        } else {
            if (!existActiveRequest) {
                this->getLogger().warning("Recieved not outdated message with invalid hash.");
            } else if (!existEndPointLink &&
                       existStartPointLink) {
                this->getLogger().warning(
                        "The endpoint device of the link is not available. ID: " + std::to_string(deviceId));
            } else if (existEndPointLink &&
                       !existStartPointLink) {
                this->getLogger().warning("The startpoint device of the link is not available. ID: " +
                                          std::to_string(recievedEndpoint.discoverPort_));
            }

        }
    } else {
        getLogger().information("Recieved outdated message.");
    }
};

bool LinkDiscoveryModule::tryToGetLinkDevice(uint64_t deviceId, std::shared_ptr<LinkDevice>& resultDevice) {
    bool result = false;
    activeDevicesMutex_.lock();
    if (activeDevices_.count(deviceId) > 0) {
        resultDevice = activeDevices_.find(deviceId)->second;
        result = true;
    }
    activeDevicesMutex_.unlock();
    return result;
}

bool LinkDiscoveryModule::tryToGetActiveRequest(uint64_t requestHash, DiscoverDevice& resultDevice) {
    bool result = false;
    activeRequestMutex_.lock();
    if (activeRequest_.count(requestHash) > 0) {
        resultDevice = activeRequest_.find(requestHash)->second;
        result = true;
    } else {
        getLogger().information("Try to get not available discover request with hash: " + std::to_string(requestHash));
    }
    activeRequestMutex_.unlock();
    return result;
};

void LinkDiscoveryModule::insertActiveRequest(DiscoverDevice resultDevice) {
    activeRequestMutex_.lock();
    if (activeRequest_.count(resultDevice.hashCode_) == 0) {
        activeRequestOrder_.push_back(resultDevice.hashCode_);
        activeRequest_.insert(std::pair<uint64_t, DiscoverDevice>(resultDevice.hashCode_, resultDevice));
    } else {
        getLogger().warning("Failed to insert discover request with already registered hash: " +
                            std::to_string(resultDevice.hashCode_));
    }

    activeRequestMutex_.unlock();
}

void LinkDiscoveryModule::flushOutdatedRequests(uint64_t timeBorder) {
    getLogger().trace("Start to flush outdated requests.");
    activeRequestMutex_.lock();
    while (!activeRequestOrder_.empty() &&
           activeRequest_.find(activeRequestOrder_[0])->second.timestamp_ < timeBorder) {
        activeRequest_.erase(activeRequestOrder_[0]);
        activeRequestOrder_.erase(activeRequestOrder_.begin());
    }
    getLogger().trace("Finished to flush outdated requests.");
    activeRequestMutex_.unlock();
}

void LinkDiscoveryModule::backgroundLinkDiscoveryRun() {
    getLogger().trace("Requested to start background update thread.");
    if (!backgroundThreadInit_) {
        getLogger().trace("Background thread is not already running.");
        updateBackground_ = true;
        backgroundThread_.startFunc([this]() {
            while (updateBackground_) {
                getLogger().trace("Start background update.");
                std::this_thread::sleep_for(updateCycle);
                uint64_t temp = (std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
                activeDevicesMutex_.lock();
                std::vector<DiscoverDevice> newDiscovers;
                getLogger().trace("Sending link discover messages to " + std::to_string(activeDevices_.size()) +
                                  " devices and their ports");
                for (std::map<uint64_t, std::shared_ptr<LinkDevice>>::iterator it = activeDevices_.begin();
                     it != activeDevices_.end();
                     ++it) {
                    std::vector<uint32_t> ports = it->second->getAvailablePorts();
                    for (std::vector<uint32_t>::iterator portIt = ports.begin(); portIt != ports.end(); ++portIt) {
                        newDiscovers.push_back(DiscoverDevice(it->second->getId(),
                                                              this->getNewHash(),
                                                              temp,
                                                              *portIt,
                                                              it->second->getVersion()));
                    }
                    std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>> flushedLinks;
                    it->second->flushOutDatedLinks(this->timeOutBorder_, flushedLinks);
                    if (!flushedLinks.empty()) {
                        this->handleLinkRemoved(it->first, flushedLinks);
                    }
                }
                activeDevicesMutex_.unlock();
                timeOutBorder_ = temp;
                for (std::vector<DiscoverDevice>::iterator deviceIt = newDiscovers.begin();
                     deviceIt != newDiscovers.end() && updateBackground_;
                     ++deviceIt) {
                    LinkDiscoveryModule_Proto::LinkDiscoveryMessage msg;
                    msg.set_uniqueid(deviceIt->hashCode_);
                    msg.set_sentimestamp(deviceIt->timestamp_);

                    Tins::RawPDU* pdu = new Tins::RawPDU(msg.SerializeAsString());
                    Tins::EthernetII eth;
                    eth.inner_pdu(pdu);
                    eth.payload_type(this->LINK_DISCOVERY_MESSAGE_PAYLOAD_TYPE);

                    this->insertActiveRequest(*deviceIt);

                    // TODO das muss hier noch korrekt für alle versionen gemacht werden.
                    of_version_t version = deviceIt->ofVersion_;
                    of_packet_out_t* packetOut = of_packet_out_new(version);
                    of_octets_t ofPayload;

                    Tins::PDU::serialization_type seri = eth.serialize();
                    ofPayload.data = seri.data();
                    ofPayload.bytes = seri.size();


                    int poutResult = of_packet_out_data_set(packetOut, &ofPayload);
                    of_packet_out_buffer_id_set(packetOut, OF_BUFFER_ID_NO_BUFFER);

                    // TODO this may be different depending on the OF version, need to verify.extract to commons

                    of_list_action_t* list = of_list_action_new(version);
                    of_object_t output;// = of_action_output_new(version);
                    of_action_output_init(&output, version, -1, 1);
                    //of_action_output_init(output, version, -1, 1);
                    of_list_action_append_bind(list, &output);
                    of_port_no_t port_no = deviceIt->discoverPort_;
                    of_action_output_port_set(&output, port_no);
                    int pOoutActionResult = of_packet_out_actions_set(packetOut, list);


                    std::string messageStr = zsdn::of_object_serialize_to_data_string(packetOut);
                    of_list_action_delete(list);
                    //of_action_output_delete(output);
                    of_packet_out_delete(packetOut);
                    zmf::data::MessageType packetOutTopic = switchadapter_topics::TO().switch_adapter().switch_instance(
                            deviceIt->deviceId_).openflow().build();

                    this->getZmf()->publish(zmf::data::ZmfMessage(packetOutTopic, messageStr));
                }
                this->flushOutdatedRequests(this->timeOutBorder_);

            }
            getLogger().information("Finish background thread.");
        });
        backgroundThreadInit_ = true;
    } else {
        getLogger().warning("Try to start already running background thread.");
    }
};

bool LinkDiscoveryModule::addLinkDevice(std::vector<uint32_t> availablePorts, uint64_t switchDpid,
                                        of_version_t ofVersion) {
    activeDevicesMutex_.lock();

    std::shared_ptr<LinkDevice> newDevice(new LinkDevice(availablePorts, switchDpid, ofVersion));
    activeDevices_.insert(std::pair<uint64_t, std::shared_ptr<LinkDevice>>(switchDpid, newDevice));

    activeDevicesMutex_.unlock();
};

bool LinkDiscoveryModule::removeLinkDevice(uint64_t switchDpid) {
    activeDevicesMutex_.lock();
    if (this->activeDevices_.count(switchDpid) > 0) {
        getLogger().information("Remove LinkDevice with the ID: " + std::to_string(switchDpid));
        this->activeDevices_.erase(switchDpid);
        for (std::map<uint64_t, std::shared_ptr<LinkDevice>>::iterator it = this->activeDevices_.begin();
             it != this->activeDevices_.end(); ++it) {
            std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>> removedLinks =
                    it->second->eraseLinkToDevice(switchDpid);
            this->handleLinkRemoved(switchDpid, removedLinks);
        }
    } else {
        this->getLogger().information("Tried to remove not existing LinkDevice with ID: " + std::to_string(switchDpid));
    }


    activeDevicesMutex_.unlock();
}


bool LinkDiscoveryModule::enable() {
    getLogger().trace("Start enable of the Module.");
    // load update cycle out  of the config
    int updateCycleTemp;
    if (this->getZmf()->getConfigurationProvider()->getAsInt(LINK_DISCOVERY_CONFIG_UPDATE_CYCLE, updateCycleTemp)) {
        if (updateCycleTemp <= 0) {
            getLogger().warning("Wrong configured update cycle. Set Value: " + updateCycleTemp);
        } else {
            this->updateCycle = std::chrono::milliseconds(updateCycleTemp);
        }
    } else {
        getLogger().warning("Can't find config value: " + std::string(LINK_DISCOVERY_CONFIG_UPDATE_CYCLE));
    }
    bool result = setupSubscription() && setupDatabase();
    // start the backgroundthread on a positiv enable
    if (result) {
        this->backgroundLinkDiscoveryRun();
    }
    getLogger().information("Module enable is " + std::to_string(result));
    return result;
}


void LinkDiscoveryModule::disable() {

    updateBackground_ = false;
    try {
        backgroundThread_.join();
    } catch (...) {
        int i = 10;
    }
    this->activeRequest_.clear();
    this->activeRequestOrder_.clear();
    for (auto it = activeDevices_.begin(); it != activeDevices_.end(); ++it) {
        it->second->clearAll();
    }
    activeDevices_.clear();
}


zmf::data::ZmfOutReply LinkDiscoveryModule::handleRequest(const zmf::data::ZmfMessage& message,
                                                          const zmf::data::ModuleUniqueId& sender) {
    // Optional: Correctly reply by returning a reply message

    LinkDiscoveryModule_Proto::Request request;

    bool parseSuccess = request.ParseFromArray(message.getDataRaw(), message.getDataLength());
    if (!parseSuccess) {
        getLogger().warning(
                "For Request from " + sender.getString() + " received invalid ProtoBuffer request format.");
    } else {
        LinkDiscoveryModule_Proto::Reply reply;
        zmf::data::MessageType topic;
        switch (request.RequestMsg_case()) {
            case LinkDiscoveryModule_Proto::Request::kGetAllSwitchLinksRequest: {
                getLogger().information("Received AllSwitchLinksRequest");
                LinkDiscoveryModule_Proto::Reply_GetAllSwitchLinksReply* allSwitchLinksReply =
                        new LinkDiscoveryModule_Proto::Reply_GetAllSwitchLinksReply();
                this->activeDevicesMutex_.lock();
                for (std::map<uint64_t, std::shared_ptr<LinkDevice>>::iterator deviceIt = this->activeDevices_.begin();
                     deviceIt != this->activeDevices_.end(); ++deviceIt) {
                    std::map<uint32_t, LinkDeviceTimestampTupel> links = deviceIt->second->getActiveLinks();
                    for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator linkIt = links.begin();
                         linkIt != links.end(); ++linkIt) {
                        common::topology::SwitchToSwitchLink* ssLink = allSwitchLinksReply->add_switch_links();
                        fillSwitchToSwitchLink(ssLink, deviceIt->second->getId(), linkIt->first,
                                               linkIt->second.device->getId(), linkIt->second.connectionPort);
                    }
                }
                this->activeDevicesMutex_.unlock();
                getLogger().information("Reply to AllSwitchLinksRequest with " +
                                        std::to_string(allSwitchLinksReply->switch_links_size()) + " links.");
                reply.set_allocated_get_all_switch_links_reply(allSwitchLinksReply);
                topic = this->sendGetAllSwitchLinksReply_;
            }

                break;
            case LinkDiscoveryModule_Proto::Request::kGetLinksFromSwitchRequest: {

                LinkDiscoveryModule_Proto::Reply_GetLinksFromSwitchReply* fromSwitchReply =
                        new LinkDiscoveryModule_Proto::Reply_GetLinksFromSwitchReply();
                uint64_t fromId = request.get_links_from_switch_request().switch_dpid();
                std::shared_ptr<LinkDevice> fromDevice;
                bool containedFromId = tryToGetLinkDevice(fromId, fromDevice);
                getLogger().trace("Received GetLinksFromSwitchRequest for the switch with ID: "
                                  + std::to_string(fromId));
                if (containedFromId) {
                    std::map<uint32_t, LinkDeviceTimestampTupel> fromLinks = fromDevice->getActiveLinks();
                    for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator linkFromIt = fromLinks.begin();
                         linkFromIt != fromLinks.end(); ++linkFromIt) {
                        common::topology::SwitchToSwitchLink* ssLink = fromSwitchReply->add_links_from_switch();
                        fillSwitchToSwitchLink(ssLink, fromDevice->getId(), linkFromIt->first,
                                               linkFromIt->second.device->getId(), linkFromIt->second.connectionPort);
                    }

                } else {
                    getLogger().information("For the recieved GetLinksFromSwitchRequest the switch ID: " +
                                            std::to_string(fromId) + " in no correlating LinkDevice available.");
                }
                fromSwitchReply->set_switch_exists(containedFromId);
                reply.set_allocated_get_links_from_switch_reply(fromSwitchReply);
                topic = this->sendGetLinksFromSwitchReply_;
            }

                break;
            case LinkDiscoveryModule_Proto::Request::kGetLinksToSwitchRequest: {
                LinkDiscoveryModule_Proto::Reply_GetLinksToSwitchReply* toSwitchReply =
                        new LinkDiscoveryModule_Proto::Reply_GetLinksToSwitchReply();
                uint64_t toId = request.get_links_to_switch_request().switch_dpid();
                this->activeDevicesMutex_.lock();
                bool existToDevice = this->activeDevices_.count(toId) > 0;
                getLogger().information("Recieved GetLinksToSwitchRequest to the switch with ID: "
                                        + std::to_string(toId));
                if (existToDevice) {
                    for (std::map<uint64_t, std::shared_ptr<LinkDevice>>::iterator deviceIt = activeDevices_.begin();
                         deviceIt != this->activeDevices_.end(); ++deviceIt) {
                        if (deviceIt->second->getId() != toId) {
                            std::map<uint32_t, LinkDeviceTimestampTupel> links = deviceIt->second->getActiveLinks();
                            for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator linkIt = links.begin();
                                 linkIt != links.end(); ++linkIt) {
                                if (linkIt->second.device->getId() == toId) {
                                    common::topology::SwitchToSwitchLink* ssLink = toSwitchReply->add_links_to_switch();
                                    fillSwitchToSwitchLink(ssLink, deviceIt->second->getId(), linkIt->first,
                                                           linkIt->second.device->getId(),
                                                           linkIt->second.connectionPort);
                                }

                            }
                        }


                    }
                } else {
                    getLogger().information("For the recieved GetLinksToSwitchRequest the switch ID: " +
                                            std::to_string(toId) + " is no correlating LinkDevice available.");
                }
                this->activeDevicesMutex_.unlock();
                toSwitchReply->set_switch_exists(existToDevice);
                reply.set_allocated_get_links_to_switch_reply(toSwitchReply);
                topic = this->sendGetLinksToSwitchReply_;
            }

                break;

            case LinkDiscoveryModule_Proto::Request::kGetAllLinksOfSwitchRequest: {
                LinkDiscoveryModule_Proto::Reply_GetAllLinksOfSwitchReply* ofSwitchReply =
                        new LinkDiscoveryModule_Proto::Reply_GetAllLinksOfSwitchReply();
                uint64_t ofId = request.get_all_links_of_switch_request().switch_dpid();
                this->activeDevicesMutex_.lock();
                bool existOfDevice = this->activeDevices_.count(ofId) > 0;
                getLogger().information("Recieved GetAllLinksOfSwitchRequest of the switch with ID: "
                                        + std::to_string(ofId));
                if (existOfDevice) {
                    for (std::map<uint64_t, std::shared_ptr<LinkDevice>>::iterator deviceIt = activeDevices_.begin();
                         deviceIt != this->activeDevices_.end(); ++deviceIt) {
                        std::map<uint32_t, LinkDeviceTimestampTupel> links = deviceIt->second->getActiveLinks();
                        if (deviceIt->second->getId() == ofId) {
                            for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator linkIt = links.begin();
                                 linkIt != links.end(); ++linkIt) {
                                common::topology::SwitchToSwitchLink* ssLink = ofSwitchReply->add_links_from_switch();
                                fillSwitchToSwitchLink(ssLink, deviceIt->second->getId(), linkIt->first,
                                                       linkIt->second.device->getId(), linkIt->second.connectionPort);

                            }
                        } else {
                            for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator linkIt = links.begin();
                                 linkIt != links.end(); ++linkIt) {
                                if (linkIt->second.device->getId() == ofId) {
                                    common::topology::SwitchToSwitchLink* ssLink = ofSwitchReply->add_links_to_switch();
                                    fillSwitchToSwitchLink(ssLink, deviceIt->second->getId(), linkIt->first,
                                                           linkIt->second.device->getId(),
                                                           linkIt->second.connectionPort);
                                }

                            }
                        }


                    }
                } else {
                    getLogger().information("For the recieved GetAllLinksOfSwitchRequest the switch ID: " +
                                            std::to_string(ofId) + " is no correlating LinkDevice available.");
                }
                this->activeDevicesMutex_.unlock();
                ofSwitchReply->set_switch_exists(existOfDevice);
                reply.set_allocated_get_all_links_of_switch_reply(ofSwitchReply);
                topic = sendGetLinksOfSwitchReply_;
            }


                break;

            case LinkDiscoveryModule_Proto::Request::kGetLinksBetweenTwoSwitchesRequest: {
                LinkDiscoveryModule_Proto::Reply_GetLinksBetweenTwoSwitchesReply* betweenTwoSwitchesReply =
                        new LinkDiscoveryModule_Proto::Reply_GetLinksBetweenTwoSwitchesReply();
                std::shared_ptr<LinkDevice> deviceA, deviceB;
                bool existA = tryToGetLinkDevice(request.get_links_between_two_switches_request().switch_a_dpid(),
                                                 deviceA);
                bool existB = tryToGetLinkDevice(request.get_links_between_two_switches_request().switch_b_dpid(),
                                                 deviceB);
                getLogger().information("Recieved GetLinksBetweenTwoSwitchesRequest between the switchs with ID: "
                                        + std::to_string(
                        request.get_links_between_two_switches_request().switch_a_dpid()) +
                                        " and " +
                                        std::to_string(
                                                request.get_links_between_two_switches_request().switch_b_dpid()
                                        ));
                if (existA && existB) {

                    std::map<uint32_t, LinkDeviceTimestampTupel> linksA = deviceA->getActiveLinks();
                    for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator aIt = linksA.begin();
                         aIt != linksA.end();
                         ++aIt) {
                        if (aIt->second.device->getId() == deviceB->getId()) {
                            common::topology::SwitchToSwitchLink* ssLink =
                                    betweenTwoSwitchesReply->add_links_from_switch_a_to_switch_b();
                            fillSwitchToSwitchLink(ssLink, deviceA->getId(), aIt->first, aIt->second.device->getId(),
                                                   aIt->second.connectionPort);
                        }
                    }

                    std::map<uint32_t, LinkDeviceTimestampTupel> linksB = deviceB->getActiveLinks();
                    for (std::map<uint32_t, LinkDeviceTimestampTupel>::iterator bIt = linksB.begin();
                         bIt != linksB.end();
                         ++bIt) {
                        if (bIt->second.device->getId() == deviceA->getId()) {
                            common::topology::SwitchToSwitchLink* ssLink =
                                    betweenTwoSwitchesReply->add_links_from_switch_a_to_switch_b();
                            fillSwitchToSwitchLink(ssLink, deviceB->getId(), bIt->first, bIt->second.device->getId(),
                                                   bIt->second.connectionPort);
                        }
                    }

                    betweenTwoSwitchesReply->set_both_switches_exists(true);
                    reply.set_allocated_get_links_between_two_switches_reply(betweenTwoSwitchesReply);
                    topic = sendGetLinksBetweenSwitchesReply_;
                } else {
                    betweenTwoSwitchesReply->set_both_switches_exists(false);
                    reply.set_allocated_get_links_between_two_switches_reply(betweenTwoSwitchesReply);
                    getLogger().information("For the recieved GetLinksBetweenTwoSwitchesRequest the switch ID: "
                                            + std::to_string(
                            request.get_links_between_two_switches_request().switch_a_dpid()) +
                                            " availability is " + std::to_string(existA) +
                                            "and the of the switch wit ID: " + std::to_string(
                            request.get_links_between_two_switches_request().switch_b_dpid()) +
                                            " availability is " + std::to_string(existB));
                }

            }

                break;

            case LinkDiscoveryModule_Proto::Request::REQUESTMSG_NOT_SET:
                getLogger().warning("Received Request where RequestMessage was not set.");
                break;

            default:
                getLogger().information("Received unknown Request");


        }

        return zmf::data::ZmfOutReply::createImmediateReply(zmf::data::ZmfMessage(topic, reply.SerializeAsString()));
    }
    return zmf::data::ZmfOutReply::createNoReply();
}

void LinkDiscoveryModule::fillSwitchToSwitchLink(common::topology::SwitchToSwitchLink* container, uint64_t sourceId,
                                                 uint32_t sourcePort, uint64_t targetId, uint32_t targetPort) {
    common::topology::AttachmentPoint* sourceAttach = new common::topology::AttachmentPoint();
    sourceAttach->set_switch_port(sourcePort);
    sourceAttach->set_switch_dpid(sourceId);
    common::topology::AttachmentPoint* targetAttach = new common::topology::AttachmentPoint();
    targetAttach->set_switch_port(targetPort);
    targetAttach->set_switch_dpid(targetId);
    container->set_allocated_source(sourceAttach);
    container->set_allocated_target(targetAttach);
}

// Elton John tuts leid dem gelben Elf auch