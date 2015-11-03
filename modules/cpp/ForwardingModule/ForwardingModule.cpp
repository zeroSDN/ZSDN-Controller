//
// Created by Andre Kutzleb on 6/28/15.
//

#include "ForwardingModule.hpp"
#include <iostream>
#include <loci/of_message.h>
#include <tins/ethernetII.h>
#include <NetUtils.h>
#include <zsdn/Configs.h>
#include <zsdn/proto/DeviceModule.pb.h>
#include <zsdn/topics/DeviceModuleTopics.hpp>
#include <RequestUtils.h>
#include <zsdn/proto/TopologyModule.pb.h>
#include <Poco/StringTokenizer.h>
#include "Poco/Stopwatch.h"
#include "LociExtensions.h"

extern "C" {
#include "loci/loci.h"
}

ForwardingModule::ForwardingModule(
        uint64_t instanceId)
        :
        AbstractModule(zmf::data::ModuleUniqueId(zsdn::MODULE_TYPE_ID_ForwardingModule, instanceId),
                       MODULE_VERSION,
                       "ForwardingModule",
                       {
                               {deviceModuleDependencyType_,deviceModuleDependencyVersion_},
                               {topologyModuleDependencyType_,topologyModuleDependencyVersion_}
                       }) { }

ForwardingModule::~ForwardingModule() {
}


bool ForwardingModule::enable() {


    // Check config file to set the topics to subscribe to
    std::vector<uint8_t> multicastGroupsToSubscribeTo;
    int multicastGroups;
    int ruleExpireMillis;


    std::string moduleSpecificMulticastGroupsKey =
            std::string(zsdn::ZSDN_FORWARDING_MODULE_MULTICAST_GROUPS_FOR_INSTANCE_ID_) +
            std::to_string(getUniqueId().InstanceId);

    std::string moduleSpecificEthertypesIngoreKey =
            std::string(zsdn::ZSDN_FORWARDING_MODULE_IGNORE_ETHERTYPES_FOR_INSTANCE_ID_) +
            std::to_string(getUniqueId().InstanceId);

    try {
        std::string moduleSpecificMulticastGroups;
        bool moduleSpecificMulticastGroupsRead = getZmf()->getConfigurationProvider()->getAsString(
                moduleSpecificMulticastGroupsKey,
                moduleSpecificMulticastGroups);

        bool configMulticastGroupsRead = getZmf()->getConfigurationProvider()->getAsInt(
                zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS, multicastGroups);

        std::string moduleSpecificEthertypesToIgnore;
        bool configModuleSpecificEthertypesIgnoreRead = getZmf()->getConfigurationProvider()->getAsString(
                moduleSpecificEthertypesIngoreKey, moduleSpecificEthertypesToIgnore);

        bool configRuleExpireMillisRead = getZmf()->getConfigurationProvider()->getAsInt(
                zsdn::ZSDN_FORWARDING_MODULE_RULE_EXPIRE_THRESHOLD, ruleExpireMillis);

        if(!configRuleExpireMillisRead) {
            getLogger().error(
                    "Could not read config value " + std::string(zsdn::ZSDN_FORWARDING_MODULE_RULE_EXPIRE_THRESHOLD));
            return false;

        }


        if (!moduleSpecificMulticastGroupsRead) {
            getLogger().error("Could not read config value for " + moduleSpecificMulticastGroupsKey);
            return false;
        }
        if (!configMulticastGroupsRead) {
            getLogger().error(
                    "Could not read config value " + std::string(zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS));
            return false;
        }

        if (multicastGroups < 1 || multicastGroups > 256) {
            getLogger().error("ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS has to be in range [1,256)");
            return false;
        }

        if (configModuleSpecificEthertypesIgnoreRead) {
            // Split the comma seperated string to retreive the single topics
            Poco::StringTokenizer st(moduleSpecificEthertypesToIgnore, ",", Poco::StringTokenizer::TOK_TRIM);

            for (int i = 0; i < st.count(); i++) {
                try {
                    int ethertype = std::stoi(st[i]);
                    if (ethertype < 0x0000 || ethertype > 0xffff) {
                        getLogger().error(
                                "ethertypes must be in range [0x0000,0xffff]");
                        return false;
                    }

                    if (std::find(ignoreEthertypes_.begin(), ignoreEthertypes_.end(),
                                  (uint16_t) ethertype) == ignoreEthertypes_.end()) {
                        // someName not in name, add it
                        ignoreEthertypes_.push_back((uint16_t) ethertype);
                        getLogger().information("Ignoring ethertype " + std::to_string((uint16_t) ethertype));
                    }
                } catch (std::invalid_argument ia) {
                    getLogger().error("could not read ignore ethertypes, malformed values");
                    return false;
                }
            }


        }

        // Split the comma seperated string to retreive the single topics
        Poco::StringTokenizer st(moduleSpecificMulticastGroups, ",", Poco::StringTokenizer::TOK_TRIM);

        for (int i = 0; i < st.count(); i++) {
            try {
                int multicastGroup = std::stoi(st[i]);
                if (multicastGroup < 0 || multicastGroup > 255) {
                    getLogger().error(
                            std::string(zsdn::ZSDN_FORWARDING_MODULE_MULTICAST_GROUPS_FOR_INSTANCE_ID_) +
                            " has to be in range [0,255)");
                    return false;
                }

                if (multicastGroup >= multicastGroups) {
                    getLogger().error(
                            "multicastgroup " + std::to_string(multicastGroup) + " does not exist, there are only " +
                            std::to_string(multicastGroups) + " multicast groups.");
                    return false;
                }

                if (std::find(multicastGroupsToSubscribeTo.begin(), multicastGroupsToSubscribeTo.end(),
                              (uint8_t) multicastGroup) == multicastGroupsToSubscribeTo.end()) {
                    // someName not in name, add it
                    multicastGroupsToSubscribeTo.push_back((uint8_t) multicastGroup);
                }
            } catch (std::invalid_argument ia) {
                getLogger().error("could not read" +
                                  std::string(zsdn::ZSDN_FORWARDING_MODULE_MULTICAST_GROUPS_FOR_INSTANCE_ID_) +
                                  ", malformed values");
                return false;
            }
        }


    } catch (Poco::Exception pe) {
        getLogger().error("failed to load configs: " + pe.message());
        return false;
    }

    cacheExpireMillis_ = ruleExpireMillis;

    bool success = initiallyRequestAllDevices();

    if (success) {
        success = initiallyRequestTopology();
    }

    if (success) {


        // setup subscriptions to OpenFlow PacketIn events. Delegate to the handlePacketIn function
        for (uint8_t multicastGroup : multicastGroupsToSubscribeTo) {
            zmf::data::MessageType packetInTopic = switchAdapterTopics_.from().switch_adapter().openflow().packet_in().multicast_group(
                    multicastGroup).build();
            this->getZmf()->subscribe(packetInTopic,
                                      [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                                          this->handlePacketIn(msg);
                                      });
        }

        this->getZmf()->subscribe(deviceStateChangeTopic_,
                                  [this](const zmf::data::ZmfMessage& msg,
                                         const zmf::data::ModuleUniqueId& sender) {
                                      this->handleDeviceStateChanged(msg);
                                  });


        this->getZmf()->subscribe(topologyChangedTopic_,
                                  [this](const zmf::data::ZmfMessage& msg,
                                         const zmf::data::ModuleUniqueId& sender) {
                                      this->handleTopologyChanged(msg);
                                  });
        success = true;

    }

    return success;

}


void ForwardingModule::disable() {
    devices_.clear();
    routeCache.clear();
    currentGraph = std::shared_ptr<zsdn::NetworkGraph>(nullptr);
}

void ForwardingModule::handlePacketIn(const zmf::data::ZmfMessage& packetInMsg) {

    // Unpack ZmfMessage which contains OpenFlow packet

    of_object_t* ofMsgParsed = zsdn::of_object_new_from_data_string_copy(packetInMsg.getData());
    of_packet_in_t* ofPacketIn = ofMsgParsed;
    of_octets_t ofPayload;
    of_packet_in_data_get(ofPacketIn, &ofPayload);
    // extracted message data
    uint8_t* payloadData = ofPayload.data;
    uint16_t payloadLength = ofPayload.bytes;
    // get the ethernet message frame
    Tins::EthernetII ethPacket(payloadData, payloadLength);

    // some ethertype may be excluded from beeing handled by this module. we check if the received packet is one of them.
    bool ignoreEthertype = false;
    for (uint16_t ignoredType : ignoreEthertypes_) {
        if (ethPacket.payload_type() == ignoredType) {
            ignoreEthertype = true;
            break;
        }
    }
    if (ignoreEthertype) {
        getLogger().trace("Ignoring packet due to ethertype");
    }

    else {

        Tins::EthernetII::address_type srcAddress = ethPacket.src_addr();
        Tins::EthernetII::address_type dstAddress = ethPacket.dst_addr();

        // check for broadcast if == TRUE -> skip the message
        if (dstAddress.is_broadcast()) {
            this->getLogger().trace("Received Broadcast -> Message ignored");
        } else {
            this->getLogger().trace("Received Packet in");

            zsdn::MAC srcMac = zsdn::NetUtils::mac_address_tins_to_uint64(srcAddress);
            zsdn::MAC dstMac = zsdn::NetUtils::mac_address_tins_to_uint64(dstAddress);

            zsdn::Device* srcDevice = lookupDevice(srcMac);
            zsdn::Device* dstDevice = lookupDevice(dstMac);

            bool srcDeviceKnown = srcDevice != nullptr;
            bool dstDeviceKnown = dstDevice != nullptr;

            if (srcDeviceKnown && dstDeviceKnown) {
                handleMessageForKnownTarget(ofPacketIn->version,ethPacket, *srcDevice, *dstDevice);

            } else {
                this->getLogger().trace(
                        "Not able to install route for packet. srcDevice known: "
                        + std::to_string(srcDeviceKnown)
                        + ". dstDevice known: "
                        + std::to_string(dstDeviceKnown) + ".");
            }
        }
    }

    of_object_delete(ofPacketIn);

};

zsdn::Device* ForwardingModule::requestDevice(zsdn::MAC mac) {
    zsdn::Device* result = nullptr;
    // get all DeviceManager modules


    DeviceModule_Proto::Request containerMsg;

    // create GetDeviceByMac request message
    DeviceModule_Proto::Request_GetDeviceByMACaddressRequest* reqMsg =
            new DeviceModule_Proto::Request_GetDeviceByMACaddressRequest();
    reqMsg->set_mac_address_of_device(mac);
    containerMsg.set_allocated_get_device_by_mac_address_request(reqMsg);

    DeviceModule_Proto::Reply replyMsg;

    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(
                    *getZmf(),
                    containerMsg,
                    replyMsg,
                    requestDeviceByMacTopic_,
                    zsdn::MODULE_TYPE_ID_DeviceModule,
                    deviceModuleDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS : {
            getLogger().trace("received reply for getDeviceByMacAddress request.");

            // if == TRUE contains a device
            if (replyMsg.get_device_by_mac_address_reply().has_device()) {
                // insert the device inside the messge into the devices map
                const common::topology::Device& msgDevice = replyMsg.get_device_by_mac_address_reply().device();
                result = this->insertNewDevice(msgDevice.mac_address(),
                                               msgDevice.attachment_point().switch_dpid(),
                                               msgDevice.attachment_point().switch_port());
            } else {
                // no device inside the message
                getLogger().trace("requested Device is unknown by the Devicemanager.");
            }

        }
            break;

        case zsdn::RequestUtils::TIMEOUT:
            getLogger().information("request getDeviceByMacAddress timed out.");
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning("Cannot request device, no DeviceModule found.");
            break;

        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            getLogger().warning("Error parsing response from DeviceModule.");
            break;
    }
}


void ForwardingModule::handleDeviceStateChanged(const zmf::data::ZmfMessage& changeMsg) {

    DeviceModule_Proto::From msgContainer;
    msgContainer.ParseFromString(changeMsg.getData()); // parse message data to the protobuf message

    const DeviceModule_Proto::From_DeviceEvent& changeProtoMsg = msgContainer.device_event();

    switch (changeProtoMsg.DeviceEventType_case()) {

        case DeviceModule_Proto::From_DeviceEvent::kDeviceAdded:
            // check for added message and the device is not inside the device map
            if (this->devices_.count(changeProtoMsg.device_added().mac_address()) == 0) {

                // insert the new device
                this->insertNewDevice(changeProtoMsg.device_added().mac_address(),
                                      changeProtoMsg.device_added().attachment_point().switch_dpid(),
                                      changeProtoMsg.device_added().attachment_point().switch_port());

            }
            break;
        case DeviceModule_Proto::From_DeviceEvent::kDeviceRemoved:
            // if contains removed message and device is known
            if (this->devices_.count(changeProtoMsg.device_removed().mac_address()) > 0) {
                // delete device
                this->deleteDevice(changeProtoMsg.device_removed().mac_address());
            } else {

                // device is unknown and can't be removed
                if (this->devices_.count(changeProtoMsg.device_removed().mac_address()) == 0) {
                    this->getLogger().information(
                            "Received remove message of not existing Device / MAC combination -> nothing to do.");
                } else {
                    // received invalid message
                    this->getLogger().warning("Received invalid message.");
                }
            }
            break;
        case DeviceModule_Proto::From_DeviceEvent::kDeviceChanged:
            //TODO
            break;
        case DeviceModule_Proto::From_DeviceEvent::DEVICEEVENTTYPE_NOT_SET:
            this->getLogger().warning(
                    "Received invalid message where DEVICEEVENTTYPE_NOT_SET!");
            break;
    }

};

zsdn::Device* ForwardingModule::insertNewDevice(zsdn::MAC mac, zsdn::DPID switch_dpid,
                                                zsdn::Port switch_port) {
    zsdn::Device* result = nullptr;
    zsdn::Device newDevice{switch_dpid, switch_port, mac};

    this->devices_.emplace(newDevice.macAddress, newDevice);
    result = &devices_.find(mac)->second;
    return result;
};

void ForwardingModule::deleteDevice(zsdn::MAC mac) {
    if (this->devices_.count(mac) > 0) {
        this->devices_.erase(mac);
    } else {
        this->getLogger().warning("Tried to remove Device which  not exist. Device MAC: " + std::to_string(mac));
    }
};

bool ForwardingModule::initiallyRequestAllDevices() {

    bool result = false;

    DeviceModule_Proto::Request reqContainerMsg; // container message
    DeviceModule_Proto::Request_GetAllDevicesRequest* reqMsg =
            new DeviceModule_Proto::Request_GetAllDevicesRequest(); // getAllDevices message
    reqContainerMsg.set_allocated_get_all_devices_request(reqMsg);

    DeviceModule_Proto::Reply replyMsg;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(
                    *getZmf(),
                    reqContainerMsg,
                    replyMsg,
                    getAllDevicesMsgTopic_,
                    zsdn::MODULE_TYPE_ID_DeviceModule,
                    deviceModuleDependencyVersion_);


    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS : {
            // clean up all device inside the map
            this->devices_.clear();
            int containedDevices = replyMsg.get_all_devices_reply().devices().size();
            // iterate over all devices inside the message and insert them to the devices map.
            for (int counter = 0; counter < containedDevices; counter++) {
                common::topology::Device msgDevice = replyMsg.get_all_devices_reply().devices().Get(counter);
                this->insertNewDevice(msgDevice.mac_address(), msgDevice.attachment_point().switch_dpid(),
                                      msgDevice.attachment_point().switch_port());
            }
            result = true;
        }
            break;

        case zsdn::RequestUtils::TIMEOUT:
            getLogger().information("request getAllDevices timed out.");
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning("No module with type zsdn::MODULE_TYPE_ID_DeviceModule found (DeviceModule).");
            break;

    }

    return result;

};


void ForwardingModule::handleMessageForKnownTarget(const of_version_t ofVersion,
                                                   const Tins::EthernetII& eth,
                                                   const zsdn::Device& sourceDevice,
                                                   const zsdn::Device& destinationDevice) {


    // Never routed from this AttachmentPoint before
    if (routeCache.count(sourceDevice) == 0) {
        zsdn::Device sourceDeviceCopy = sourceDevice;

        routeCache.emplace(sourceDeviceCopy, std::shared_ptr<ExpiringTargetSet>(
                new ExpiringTargetSet(std::max(26,cacheExpireMillis_))));

        getLogger().information("First time attempting to route from " + sourceDeviceCopy.toString());
    }

    const std::shared_ptr<ExpiringTargetSet>& targetsOfSource = this->routeCache.find(sourceDevice)->second;

    bool hasRouteToDevice = targetsOfSource->has(destinationDevice);

    // Need to install a route.
    if (hasRouteToDevice) {
        // Ignore, route already exists.
        getLogger().information("Route from " + sourceDevice.toString() + " to " + destinationDevice.toString() +
                                " does already exist. ignoring packet.");
    }

    else {
        getLogger().information("Route from " + sourceDevice.toString() + " to " + destinationDevice.toString() +
                                " does not exist yet.");
        getLogger().information("Attempting to find shortest path.");

        Poco::Stopwatch timer;

        timer.start();
        std::vector<zsdn::AttachmentPoint> path = currentGraph->getShortestPath(sourceDevice, destinationDevice);
        timer.stop();
        if (path.size() > 0) {
            getLogger().information("Found shortest path in " + std::to_string(timer.elapsed()) + "µs (length" +
                                    std::to_string(path.size()) + ").");
            getLogger().information("Path is: " + zsdn::NetworkGraph::pathToString(path));
            getLogger().information("Installing flow rules along path now.");

            installFlowRulesAlongPath(ofVersion,path, destinationDevice);

            getLogger().information("Installed all flow rules.");

            zsdn::Device destinationDeviceCopy = destinationDevice;

            if(cacheExpireMillis_ > 0)  {
                targetsOfSource->add(destinationDeviceCopy, Poco::Void());
            }
        } else {
            getLogger().information(
                    "No route from " + sourceDevice.toString() + " to " + destinationDevice.toString() + " was found.");
        }


    }

}

void ForwardingModule::handleTopologyChanged(const zmf::data::ZmfMessage& message) {
    TopologyModule_Proto::From msgContainer;
    msgContainer.ParseFromString(message.getData());
    getLogger().information("Received new Topology.");
    const TopologyModule_Proto::From_TopologyChangedEvent& topoChangeMsg = msgContainer.topology_changed_event();

    const common::topology::Topology& newTopo = topoChangeMsg.topology();
    applyTopology(newTopo);
}

bool ForwardingModule::initiallyRequestTopology() {
    bool result = false;

    TopologyModule_Proto::Request reqContainerMsg; // container message
    TopologyModule_Proto::Request_GetTopologyRequest* reqMsg =
            new TopologyModule_Proto::Request_GetTopologyRequest();

    reqContainerMsg.set_allocated_get_topology_request(reqMsg);

    TopologyModule_Proto::Reply replyMsg;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(
                    *getZmf(),
                    reqContainerMsg,
                    replyMsg,
                    getTopologyTopic_,
                    zsdn::MODULE_TYPE_ID_TopologyModule,
                    topologyModuleDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS : {
            const common::topology::Topology& newTopo = replyMsg.get_topology_reply().topology();
            applyTopology(newTopo);
            result = true;

        }
            break;

        case zsdn::RequestUtils::TIMEOUT:
            getLogger().information("request getTopology timed out.");
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning("No module with type zsdn::MODULE_TYPE_ID_TopologyModule found.");
            break;

    }

    return result;
}


void ForwardingModule::applyTopology(const common::topology::Topology& newTopo) {

    Poco::Stopwatch w;
    w.start();
    currentGraph = std::shared_ptr<zsdn::NetworkGraph>(new zsdn::NetworkGraph(newTopo));
    w.stop();
    getLogger().information("Applied new topology in " + std::to_string(w.elapsed()) + "µs.");
    if (getLogger().information()) {
        getLogger().information("Graph reachability: \n" + printReachabilityCheck(newTopo));
    }
}

zsdn::Device* ForwardingModule::lookupDevice(const zsdn::MAC mac) {
    zsdn::Device* device = nullptr;
    // check if the device is known if == FALSE -> request the device by a DeviceManager module
    if ((this->devices_.count(mac) > 0)) {
        device = &this->devices_.find(mac)->second; // get the device out of the map
    } else {
        device = this->requestDevice(mac); // request the device by DeviceManager module
    }
    return device;
}

void ForwardingModule::installFlowRulesAlongPath(const of_version_t ofVersion,
                                                 const std::vector<zsdn::AttachmentPoint>& vector,
                                                 const zsdn::Device& destinationDevice) {
    // there must always be pairs of in and outports.
    if (vector.size() % 2 == 0) {

        for (int i = 0; i < vector.size(); i = i + 2) {
            const zsdn::AttachmentPoint& inPort = vector[i];
            const zsdn::AttachmentPoint& outPort = vector[i + 1];

            if (inPort.switchDpid != outPort.switchDpid) {
                throw Poco::IllegalStateException(
                        "switchIds of inPort and outPort do not match: " + std::to_string(inPort.switchDpid) + " != " +
                        std::to_string(outPort.switchDpid));
            }

            zsdn::DPID switchForFlow = inPort.switchDpid;

            // TODO hardcoded OF-version, table
            of_flow_add_t* flowToAdd = zsdn::create_layer2_forwarding_flow(ofVersion, 0,
            std::max(1,cacheExpireMillis_ / 1000),
                                                                           inPort.switchPort, outPort.switchPort,
                                                                           destinationDevice.macAddress);

            std::string serializedFlow = zsdn::of_object_serialize_to_data_string(flowToAdd);
            // could also use outPort.switchDpid, they have to be identical

            zmf::data::MessageType toSpecificSwitch = switchAdapterTopics_.to().switch_adapter().switch_instance(
                    switchForFlow).openflow().packet_out().build();


            getZmf()->publish(zmf::data::ZmfMessage(toSpecificSwitch, serializedFlow));

           of_flow_add_delete(flowToAdd);
        }
    }

    else {
        getLogger().warning("Cannot install path, path does not consist of pairs of two (in/out ports).");
    }
}

std::string ForwardingModule::printReachabilityCheck(const common::topology::Topology& topology) {
    std::stringstream builder;

    builder << "     ";
    for (const common::topology::Switch& fromSwitch : topology.switches()) {
        for (const common::topology::SwitchPort& fromSwitchPort : fromSwitch.switch_ports()) {

            std::string port;

            uint32_t fromSwitchPortNumber = fromSwitchPort.attachment_point().switch_port();
            switch (fromSwitchPortNumber) {
                case OF_PORT_DEST_LOCAL:
                    port = "L";
                    break;
                default:
                    port = std::to_string(fromSwitchPort.attachment_point().switch_port());
                    break;
            }
            builder << std::setw(3) << std::right << std::hex << fromSwitchPort.attachment_point().switch_dpid() <<
            ":" << port << " ";

        }
    }
    builder << &std::endl;

    for (const common::topology::Switch& fromSwitch : topology.switches()) {
        for (const common::topology::SwitchPort& fromSwitchPort : fromSwitch.switch_ports()) {
            std::string port;

            uint32_t fromSwitchPortNumber = fromSwitchPort.attachment_point().switch_port();
            switch (fromSwitchPortNumber) {
                case OF_PORT_DEST_LOCAL:
                    port = "L";
                    break;
                default:
                    port = std::to_string(fromSwitchPort.attachment_point().switch_port());
                    break;
            }
            builder << std::setw(3) << std::right << std::hex << fromSwitchPort.attachment_point().switch_dpid() <<
            ":" << port << " ";

            for (const common::topology::Switch& toSwitch : topology.switches()) {
                for (const common::topology::SwitchPort& toSwitchPort : toSwitch.switch_ports()) {
                    int length = currentGraph->getShortestPath(fromSwitchPort.attachment_point(),
                                                               toSwitchPort.attachment_point()).size();
                    std::string no = (toSwitchPort.attachment_point().switch_dpid() ==
                                      fromSwitchPort.attachment_point().switch_dpid() &&
                                      toSwitchPort.attachment_point().switch_port() ==
                                      fromSwitchPort.attachment_point().switch_port() ? "0" : (length > 0
                                                                                               ? std::to_string(
                                    length) : "-"));
                    builder << std::setw(3) << no << "   ";
                }
            }
            builder << &std::endl;

        }
    }

    return builder.str();

}
