#include "SimpleForwardingModule.hpp"
#include <zsdn/proto/DeviceModule.pb.h>
#include <zsdn/topics/SwitchAdapterTopics.hpp>
#include <Poco/StringTokenizer.h>
#include "zsdn/Configs.h"

SimpleForwardingModule::SimpleForwardingModule(
        uint64_t instanceId)
        :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
                       MODULE_VERSION,
                       "SimpleForwardingModule",
                       {{deviceMgrDependencyType_, deviceMgrDependencyVersion_}}) {
    tempPacketOut_OF_1_0 = of_packet_out_new(OF_VERSION_1_0);
    tempPacketOut_OF_1_3 = of_packet_out_new(OF_VERSION_1_3);

    of_packet_out_buffer_id_set(tempPacketOut_OF_1_0, OF_BUFFER_ID_NO_BUFFER);
    of_packet_out_buffer_id_set(tempPacketOut_OF_1_3, OF_BUFFER_ID_NO_BUFFER);


    tempActionList_OF_1_0 = of_list_action_new(OF_VERSION_1_0);
    tempActionList_OF_1_3 = of_list_action_new(OF_VERSION_1_3);
    tempOutPut_OF_1_0 = of_action_output_new(OF_VERSION_1_0);
    tempOutPut_OF_1_3 = of_action_output_new(OF_VERSION_1_3);
    of_list_action_append_bind(tempActionList_OF_1_0, tempOutPut_OF_1_0);
    of_list_action_append_bind(tempActionList_OF_1_3, tempOutPut_OF_1_3);
    //of_port_no_t port_no = destinationDevice->switchPort;
    //of_action_output_port_set(output, port_no);
    of_packet_out_actions_set(tempPacketOut_OF_1_0, tempActionList_OF_1_0);
    of_packet_out_actions_set(tempPacketOut_OF_1_3, tempActionList_OF_1_3);
};

SimpleForwardingModule::~SimpleForwardingModule() {
   // of_action_output_delete(tempOutPut_OF_1_0);
   // of_action_output_delete(tempOutPut_OF_1_3);
    of_list_action_delete(tempActionList_OF_1_0);
    of_list_action_delete(tempActionList_OF_1_3);
    of_packet_out_delete(tempPacketOut_OF_1_0);
    of_packet_out_delete(tempPacketOut_OF_1_3);
};


bool SimpleForwardingModule::setupDatabase() {
    bool result = false;

    DeviceModule_Proto::Request reqContainerMsg; // container message
    DeviceModule_Proto::Request_GetAllDevicesRequest* reqMsg =
            new DeviceModule_Proto::Request_GetAllDevicesRequest(); // getAllDevices message
    reqContainerMsg.set_allocated_get_all_devices_request(reqMsg);


    DeviceModule_Proto::Reply replyMsg;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            reqContainerMsg,
                                            replyMsg,
                                            getAllDevicesMsgTopic_,
                                            zsdn::MODULE_TYPE_ID_DeviceModule,
                                            deviceMgrDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {
            // clean up all device inside the map
            this->devices_.clear();
            int containedDevices = replyMsg.get_all_devices_reply().devices().size();
            // iterate over all devices inside the message and insert them to the devices map.
            for (int counter = 0; counter < containedDevices; counter++) {
                common::topology::Device msgDevice = replyMsg.get_all_devices_reply().devices().Get(counter);
                this->insertNewDevice(msgDevice.mac_address(), msgDevice.attachment_point().switch_dpid(),
                                      msgDevice.attachment_point().switch_port());
            }
            result = true; // successful setup the devices database
        }
            break;
        case zsdn::RequestUtils::TIMEOUT:
            getLogger().information("request getAllDevices timed out.");
            break;
        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning("No module with type zsdn::MODULE_TYPE_ID_DeviceModule found (DeviceModule).");
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            getLogger().warning("Serialization of the Request failed.");
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            getLogger().warning("Parsing of the Response failed.");
            break;
    }

    return result;
};

bool SimpleForwardingModule::setupSubscriptions() {

    // Check config file to set the topics to subscribe to
    std::vector<uint8_t> multicastGroupsToSubscribeTo;
    int multicastGroups;

    std::string moduleSpecificMulticastGroupsKey =
            std::string(zsdn::ZSDN_SIMPLE_FORWARDING_MODULE_MULTICAST_GROUPS_FOR_INSTANCE_ID_) +
            std::to_string(getUniqueId().InstanceId);

    std::string moduleSpecificEthertypesIngoreKey =
            std::string(zsdn::ZSDN_SIMPLE_FORWARDING_MODULE_IGNORE_ETHERTYPES_FOR_INSTANCE_ID_) +
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
                            std::string(zsdn::ZSDN_SIMPLE_FORWARDING_MODULE_MULTICAST_GROUPS_FOR_INSTANCE_ID_) +
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
                                  std::string(zsdn::ZSDN_SIMPLE_FORWARDING_MODULE_MULTICAST_GROUPS_FOR_INSTANCE_ID_) +
                                  ", malformed values");
                return false;
            }
        }


    } catch (Poco::Exception pe) {
        getLogger().error("failed to load configs: " + pe.message());
        return false;
    }



    // setup subscriptions to OpenFlow PacketIn events. Delegate to the handlePacketIn function
    for (uint8_t multicastGroup : multicastGroupsToSubscribeTo) {
        zmf::data::MessageType packetInTopic = switchAdapterTopics_.from().switch_adapter().openflow().packet_in().multicast_group(
                multicastGroup).build();
        this->getZmf()->subscribe(packetInTopic,
                                  [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                                      this->handlePacketIn(msg);
                                  });
    }

    // setup subscriptions to DeviceManager Change events. Delegate to the handleDeviceStateChanged function
    zmf::data::MessageType deviceStateChangeTopic = deviceModuleTopics_.from().device_module().device_event().build();
    this->getZmf()->subscribe(deviceStateChangeTopic,
                              [this](const zmf::data::ZmfMessage& msg,
                                     const zmf::data::ModuleUniqueId& sender) { this->handleDeviceStateChanged(msg); });
    return true;
};

void SimpleForwardingModule::handlePacketIn(const zmf::data::ZmfMessage& packetInMsg) {
    Device* destinationDevice = nullptr;
    // Unpack ZmfMessage which contains OpenFlow packet
    of_packet_in_t* ofPacketIn = zsdn::of_object_new_from_data_string_copy(packetInMsg.getData());
    of_octets_t ofPayload;
    of_packet_in_data_get(ofPacketIn, &ofPayload);
    // extracted message data
    uint8_t* payloadData = ofPayload.data;
    uint16_t payloadLength = ofPayload.bytes;
    // get the ethernet message frame
    Tins::EthernetII ethPacket(payloadData, payloadLength);
    of_version_t ofVersion = ofPacketIn->version;


    // some ethertype may be excluded from beeing handled by this module. we check if the received packet is one of them.
    bool ignoreEtherType = false;
    for (uint16_t ignoredType : ignoreEthertypes_) {
        if (ethPacket.payload_type() == ignoredType) {
            ignoreEtherType = true;
            break;
        }
    }
    if (ignoreEtherType) {
        getLogger().trace("Ignoring packet due to ethertype");
    }

    else {

        Tins::EthernetII::address_type dstAdress = ethPacket.dst_addr(); // extract the destination address

        uint64_t destinationMac = zsdn::NetUtils::mac_address_tins_to_uint64(dstAdress);
        // check for broadcast if == TRUE -> skip the message
        if (dstAdress.is_broadcast()) {
            this->getLogger().trace("Received Broadcast -> Message ignored");
        }

        else {

            this->getLogger().trace("Received Packet in");
            // check if the device is known if == FALSE -> request the device by a DeviceManager module
            if ((this->devices_.count(destinationMac) > 0)) {
                destinationDevice = &this->devices_.find(destinationMac)->second; // get the device out of the map
            } else {
                destinationDevice = this->requestDevice(destinationMac); // request the device by DeviceManager module
            }

            if (destinationDevice != nullptr) {
                // send the initial message payload to the destination switch port
                if (ofVersion == OF_VERSION_UNKNOWN) {
                    getLogger().warning("Received Packet with OF_VERSION_UNKNOWN. Ignoring Packet.");
                    return;
                }


                of_port_no_t port_no = destinationDevice->switchPort;



                of_object_t* resultObject = nullptr;
                switch (ofVersion) {

                    case OF_VERSION_UNKNOWN:
                        break;
                    case OF_VERSION_1_0:
                        of_packet_out_data_set(tempPacketOut_OF_1_0, &ofPayload);
                        of_action_output_port_set(tempOutPut_OF_1_0, port_no);
                        of_packet_out_actions_set(tempPacketOut_OF_1_0, tempActionList_OF_1_0);
                        resultObject = tempPacketOut_OF_1_0;
                        break;

                    case OF_VERSION_1_1:
                        break;
                    case OF_VERSION_1_2:
                        break;
                    case OF_VERSION_1_3:
                        of_packet_out_data_set(tempPacketOut_OF_1_3, &ofPayload);
                        of_action_output_port_set(tempOutPut_OF_1_3, port_no);
                        of_packet_out_actions_set(tempPacketOut_OF_1_3, tempActionList_OF_1_3);
                        resultObject = tempPacketOut_OF_1_3;
                        break;

                    case OF_VERSION_1_4:
                        break;
                }




                if (resultObject != nullptr) {
                    std::string messageBytes = zsdn::of_object_serialize_to_data_string(resultObject);

                    std::map<uint64_t, zmf::data::MessageType>::iterator msgType = this->linkDevicePacketOutMessageTypeMap_.find(
                            destinationDevice->macAddress);

                    this->getZmf()->publish(zmf::data::ZmfMessage(
                            msgType->second,
                            messageBytes));
                    if (this->getLogger().trace()) {
                        this->getLogger().trace(
                                "Forwarded packet to " + std::to_string(unsigned(destinationDevice->switchDpid)) + ":" +
                                std::to_string(unsigned(destinationDevice->switchPort)));
                    }
                }


            } else {
                this->getLogger().trace(
                        "Not able to forwared packet, host " + dstAdress.to_string() + " unknown");
            }
        }
    }
    of_packet_in_delete(ofPacketIn);
};


void SimpleForwardingModule::handleDeviceStateChanged(const zmf::data::ZmfMessage& changeMsg) {

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

            } else {
                // check if the "added device" is already known to update it's infomations
                if (this->devices_.count(changeProtoMsg.device_added().mac_address()) > 0) {
                    this->getLogger().information(
                            "Received add message of already existing Device / MAC combination -> solved as changed event.");
                    this->modifyDevice(changeProtoMsg.device_added().mac_address(),
                                       changeProtoMsg.device_added().attachment_point().switch_dpid(),
                                       changeProtoMsg.device_added().attachment_point().switch_port());
                } else {
                    // received invalid message
                    this->getLogger().warning("Received invalid message.");
                }
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
            // check if message contains change message and existence of the changed device
            if (this->devices_.count(changeProtoMsg.device_changed().device_now().mac_address()) > 0) {
                // modify the device
                this->modifyDevice(changeProtoMsg.device_changed().device_now().mac_address(),
                                   changeProtoMsg.device_changed().device_now().attachment_point().switch_dpid(),
                                   changeProtoMsg.device_changed().device_now().attachment_point().switch_port());
            } else {
                // check if the changed device is unknown
                if (this->devices_.count(changeProtoMsg.device_changed().device_now().mac_address()) == 0) {
                    this->getLogger().information(
                            "Received change message of not existing Device / MAC combination -> solved as add event.");
                    // insert the unknown device
                    this->insertNewDevice(changeProtoMsg.device_changed().device_now().mac_address(),
                                          changeProtoMsg.device_changed().device_now().attachment_point().switch_dpid(),
                                          changeProtoMsg.device_changed().device_now().attachment_point().switch_port());
                } else {
                    // received invalid message
                    this->getLogger().warning("Received invalid message.");
                }
            }
            break;
        case DeviceModule_Proto::From_DeviceEvent::DEVICEEVENTTYPE_NOT_SET:
            this->getLogger().warning(//
// Created by  Matthias Strljc, Andre Kutzleb on 6/28/15.
//

                    "Received invalid message where DEVICEEVENTTYPE_NOT_SET!");
            break;
    }

};

SimpleForwardingModule::Device* SimpleForwardingModule::insertNewDevice(std::uint64_t mac, std::uint64_t switch_dpid,
                                                                        uint32_t switch_port) {
    Device* result = nullptr;
    Device newDevice;
    newDevice.macAddress = mac;
    newDevice.switchDpid = switch_dpid;
    zmf::data::MessageType packetOutTopic = switchAdapterTopics_.to().switch_adapter()
            .switch_instance(switch_dpid).openflow().build();
    linkDevicePacketOutMessageTypeMap_.emplace(mac, packetOutTopic);
    newDevice.switchPort = switch_port;
    this->devices_.insert(std::pair<std::uint64_t, Device>(newDevice.macAddress, newDevice));
    result = &devices_.find(mac)->second;
    return result;
};

void SimpleForwardingModule::modifyDevice(std::uint64_t mac, std::uint64_t switch_dpid, uint32_t switch_port) {
    if (this->devices_.count(mac) > 0) {
        Device* device = &this->devices_.find(mac)->second;
        device->switchPort = switch_port;
        device->switchDpid = switch_dpid;
    } else {
        this->getLogger().warning("Tried to modify Device which not exist. Device MAC: " + std::to_string(mac));
    }
};

void SimpleForwardingModule::deleteDevice(std::uint64_t mac) {
    if (this->devices_.count(mac) > 0) {
        this->linkDevicePacketOutMessageTypeMap_.erase(mac);
        this->devices_.erase(mac);
    } else {
        this->getLogger().warning("Tried to remove Device which  not exist. Device MAC: " + std::to_string(mac));
    }
};

SimpleForwardingModule::Device* SimpleForwardingModule::requestDevice(std::uint64_t mac) {
    Device* result = nullptr;
    // get all DeviceManager modules

    DeviceModule_Proto::Request containerMsg;

    // create GetDeviceByMac request message
    DeviceModule_Proto::Request_GetDeviceByMACaddressRequest* reqMsg =
            new DeviceModule_Proto::Request_GetDeviceByMACaddressRequest();
    reqMsg->set_mac_address_of_device(mac);
    containerMsg.set_allocated_get_device_by_mac_address_request(reqMsg);

    // get the response message
    DeviceModule_Proto::Reply replyMsg;

    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            containerMsg,
                                            replyMsg,
                                            requestDeviceByMacTopic_,
                                            zsdn::MODULE_TYPE_ID_DeviceModule,
                                            deviceMgrDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {

            getLogger().trace("received reply for getDeviceByMacAddress with correct type.");

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
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            // response is wrong
            getLogger().warning("Parsing of the Response failed.");
            break;
        case zsdn::RequestUtils::TIMEOUT:
            // message timed out
            getLogger().information("request getDeviceByMacAddress timed out.");
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            // dependencies violated -> noe DeviceManager found
            getLogger().warning("Cannot request device, no DeviceModule found.");
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            getLogger().warning("Serialization of the Request failed.");
            break;
    }
    return result;
};

bool SimpleForwardingModule::enable() {
    bool result;
    // start setup the database and the subscriptions
    result = this->setupDatabase() && this->setupSubscriptions();
    return result;
};


void SimpleForwardingModule::disable() {
    this->devices_.clear(); // delete all devices
    this->linkDevicePacketOutMessageTypeMap_.clear();
};
