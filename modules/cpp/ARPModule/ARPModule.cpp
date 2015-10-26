#include "ARPModule.hpp"
#include <zsdn/topics/SwitchAdapter_topics.hpp>
#include <zsdn/proto/DeviceModule.pb.h>
#include "zsdn/Configs.h"
#include <RequestUtils.h>
#include <LociExtensions.h>


#define ARP_MODULE_ENABLE_DEBUG_INFO 0


ARPModule::ARPModule(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId), MODULE_VERSION, "ARPModule",
                       {{deviceModuleDependencyType_,deviceModuleDependencyVersion_}}) { }

ARPModule::~ARPModule() {
    disable();
}


bool ARPModule::enable() {

    int multicastGroups;

    try {
        bool configMulticastGroupsRead = getZmf()->getConfigurationProvider()->getAsInt(
                zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS, multicastGroups);

        if (!configMulticastGroupsRead) {
            getLogger().error(
                    "Could not read config value " + std::string(zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS));
            return false;
        }

        if(multicastGroups < 1 || multicastGroups > 256) {
            getLogger().error("ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS has to be in range [1,256)");
            return false;
        }

    } catch (Poco::Exception pe) {
        getLogger().error("failed to load configs: " + pe.message());
        return false;
    }


    // Get the current list of devices from Device Module
    getDevicesFromDeviceModule();

    // Subscribe to topic ARP from Switch Adapter
    // With this subscription the module gets the ARP packets in
    for (int i = 0; i < multicastGroups; i++) {
        zmf::data::MessageType arpTopic = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().multicast_group(i).arp().build();
        getZmf()->subscribe(arpTopic,
                            [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                                handleIncomingArp(msg, sender);
                            });
    }
    // Subscribe to topic deviceStateChangeTopic from Device Module
    // With this subscription the module gets the update of the known devices from Device Module
    zmf::data::MessageType deviceStateChangeTopic = devicemodule_topics::FROM().device_module().device_event().build();
    getZmf()->subscribe(deviceStateChangeTopic,
                        [this](const ZmfMessage& msg, const ModuleUniqueId& sender) { handleUpdateDevice(msg); });
    return true;
}


void ARPModule::disable() {
    arpCache_.clear();
}


void ARPModule::handleIncomingArp(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& moduleId) {

    of_octets_t ofPayload;               // Structure to unpack the data from the open flow message
    Tins::ARP::ipaddress_type ipAddSenderPackOut; // Ip address of the sender device for the message to send
    Tins::ARP::ipaddress_type ipAddTargetPackOut; // Ip address of the target device for the message to send
    Tins::EthernetII arpReply;                    // Ethernet frame for the packet out
    std::uint32_t destinationSwitchPort; // Switch port the device is attached to
    std::uint64_t destinationSwitchDpid; // ID of the switch the device is attached to
    std::uint32_t portPacketOut;         // Port an dem the packet out will be sent out
    of_version_t versionOfPacketIn;     // OFVersion of incoming packetIn


    // Get general information of the received open flow message
    // Structure to unpack open flow message
    of_packet_in_t* ofPacketIn = zsdn::of_object_new_from_data_string_copy(message.getData());
    versionOfPacketIn = ofPacketIn->version;
    of_packet_in_data_get(ofPacketIn, &ofPayload);

    try {
        // Send the reply to the port the request came from
        portPacketOut = getPortPacketInCameFrom(ofPacketIn);
    }
    catch (...) {
        getLogger().warning("Not possible to identify the port the ARP-Request came from");
        of_object_delete(ofPacketIn);
        return;
    }
    of_object_delete(ofPacketIn); // TODO: Evaluate



    //Get the ethernet frame from the received open flow message
    Tins::EthernetII ethFrameIn(ofPayload.data, ofPayload.bytes);
    //Get the ARP packet from the ethernet frame
    //the ethernet frame contains just ARP packets, because this module subscribes just to ARP events
    Tins::ARP* arpPacketIn = (Tins::ARP*) ethFrameIn.inner_pdu();

    if (arpPacketIn != nullptr) {
        // Read the ip of the target and the sender device from the ARP packet in
        Tins::ARP::ipaddress_type ipAddTargetPackIn = arpPacketIn->target_ip_addr();
        Tins::ARP::ipaddress_type ipAddSenderPackIn = arpPacketIn->sender_ip_addr();
        // Search if there is an entry in the ARP cache with the ip address
        std::uint32_t devicePosInArpCache;
        // Iterator to search over the ARP Cache
        ARPModule::IteratorArpCache itArpCache;
        bool isIpInArpCache = false;

        isIpInArpCache = GetDevicePosInArpCacheByIp(ipAddTargetPackIn, devicePosInArpCache);

        if (!isIpInArpCache) {
            // Target Device unknown, try to get the info from Device Module
            getLogger().information("Send request to the device module");
            isIpInArpCache = GetDeviceFromDeviceModule(ipAddTargetPackIn, devicePosInArpCache);
        }

        //Answer the ARP request only in case the destination is known, in other case just drop out the packet_in
        if (isIpInArpCache) {
            itArpCache = arpCache_.begin();
            advance(itArpCache, devicePosInArpCache);
            destinationSwitchPort = itArpCache->second.switchPort;
            destinationSwitchDpid = itArpCache->second.switchDpid;

            Tins::ARP::Flags opCodePacketIn = (Tins::ARP::Flags) arpPacketIn->opcode();

            switch (opCodePacketIn) {
                case Tins::ARP::REQUEST: {
                    // Send the reply to the switch the request came from
                    destinationSwitchDpid = moduleId.InstanceId;
                    // Exchange destination and target
                    ipAddSenderPackOut = ipAddTargetPackIn;
                    ipAddTargetPackOut = ipAddSenderPackIn;

                    std::string macAddSenderAsString = zsdn::NetUtils::uint64_to_mac_address_string(itArpCache->first);
                    const Tins::ARP::hwaddress_type macAddSenderPackOut(macAddSenderAsString);

                    // Build a Ethernet frame with the reply to the ARP request
                    arpReply = Tins::ARP::make_arp_reply(ipAddTargetPackOut, ipAddSenderPackOut,
                                                         arpPacketIn->sender_hw_addr(), macAddSenderPackOut);
                    #if ARP_MODULE_ENABLE_DEBUG_INFO
                    // Show some debug information
                    getLogger().information("Reply ARP-Request from device "
                                            + ipAddSenderPackOut.to_string()
                                            + " to device " + ipAddTargetPackOut.to_string()
                                            + " attached to port: " + std::to_string(portPacketOut)
                                            + " of switch with ID: " + std::to_string(destinationSwitchDpid));
                    #endif
                }
                    break;
                case Tins::ARP::REPLY:
                    // Send the reply to the port the request came from
                    portPacketOut = destinationSwitchPort;
                    ipAddSenderPackOut = ipAddSenderPackIn;
                    ipAddTargetPackOut = ipAddTargetPackIn;
                    // Packet out is a just the same as packet_in
                    arpReply = Tins::ARP::make_arp_reply(ipAddTargetPackOut, ipAddSenderPackOut,
                                                         arpPacketIn->target_hw_addr(), arpPacketIn->sender_hw_addr());
                    #if ARP_MODULE_ENABLE_DEBUG_INFO
                    // Show some debug information
                    getLogger().information("Forward ARP-Reply from device "
                                            + ipAddSenderPackOut.to_string()
                                            + " to device " + ipAddTargetPackOut.to_string()
                                            + " attached to port: " + std::to_string(portPacketOut)
                                            + " of switch with ID: " + std::to_string(destinationSwitchDpid));
                    #endif
                    break;
                default:
                    getLogger().warning("Operation in ARP packet unknown");
                    return;
            }

            // Vector to serialized the ethernet frame to send the open flow packet out
            std::vector<std::uint8_t> serializedEthFrameOut = arpReply.serialize();

            // pack the ethernet packet in a open flow message, which can be sent over Zmf
            of_packet_out_t* arpPacketOut = of_packet_out_new(versionOfPacketIn);
            try {
                packEthFrameInOfPacketOut(serializedEthFrameOut, portPacketOut, arpPacketOut);
                // send out the open flow message
                std::string messageBytes = zsdn::of_object_serialize_to_data_string(arpPacketOut);
                of_packet_out_delete(arpPacketOut);

                zmf::data::MessageType toSpecificSwitchTopic = switchadapter_topics::TO().switch_adapter().switch_instance(
                        destinationSwitchDpid).openflow().build();

                this->getZmf()->publish(zmf::data::ZmfMessage(toSpecificSwitchTopic, messageBytes));
                //show some debug information.
                getLogger().information("Publish message to Zmf...");
            }
            catch (...) {
                of_packet_out_delete(arpPacketOut);
                getLogger().warning("Operation in ARP packet unknown");
            }


        }
        else {
            //of_object_delete((of_object_t*)ofPacketIn);
            getLogger().warning("No device with IP " + ipAddTargetPackIn.to_string() + " found. Cannot handle ARP.");
        }

    }
}

void ARPModule::handleUpdateDevice(const zmf::data::ZmfMessage& message) {
    bool isDeviceInArpCache = false;
    std::uint64_t macAddressDevice;
    std::uint32_t ipAddressDevice;
    ARPModule::IteratorArpCache itArpCache; // Iterator to search a device in the ARP cache
    DeviceModule_Proto::From msgContainer;  // Proto buffer with the message from Device Module

    bool isParseSuccess = msgContainer.ParseFromString(message.getData());

    if (isParseSuccess) {

        const DeviceModule_Proto::From_DeviceEvent& updateDeviceProtoMsg = msgContainer.device_event();

        switch (updateDeviceProtoMsg.DeviceEventType_case()) {

            // Process Add device events from Device Manager
            case DeviceModule_Proto::From_DeviceEvent::kDeviceAdded:
                if (updateDeviceProtoMsg.device_added().ipv4_address_size() > 0) {
                    macAddressDevice = updateDeviceProtoMsg.device_added().mac_address();
                    // Get the newest IPv4 address attached to the device saved in the index 0
                    ipAddressDevice = updateDeviceProtoMsg.device_added().ipv4_address().Get(0);

                    updateArpCacheCache(ipAddressDevice, macAddressDevice,
                                        updateDeviceProtoMsg.device_added().attachment_point().switch_dpid(),
                                        updateDeviceProtoMsg.device_added().attachment_point().switch_port());
                }
                break;
                // Process Device Change events from Device Manager
            case DeviceModule_Proto::From_DeviceEvent::kDeviceChanged:
                if (updateDeviceProtoMsg.device_changed().device_now().ipv4_address_size() > 0) {
                    macAddressDevice = updateDeviceProtoMsg.device_changed().device_now().mac_address();
                    // Get the newest IPv4 address attached to the device saved in the index 0
                    ipAddressDevice = updateDeviceProtoMsg.device_changed().device_now().ipv4_address().Get(
                            0);

                    updateArpCacheCache(ipAddressDevice, macAddressDevice,
                                        updateDeviceProtoMsg.device_changed().device_now().attachment_point().switch_dpid(),
                                        updateDeviceProtoMsg.device_changed().device_now().attachment_point().switch_port());
                }
                break;
                // Process delete device events from Device Manager
            case DeviceModule_Proto::From_DeviceEvent::kDeviceRemoved:
                // Get the MAC address of the device to delete from the ARP Cache
                macAddressDevice = updateDeviceProtoMsg.device_removed().mac_address();
                // If the device is found in the ARP cache delete it
                itArpCache = arpCache_.find(macAddressDevice);
                if (itArpCache != arpCache_.end()) {
                    arpCache_.erase(itArpCache);
                } else {
                    this->getLogger().information("Remove device received for a not existing ARP cache entry.");
                }
                break;
            default:
                this->getLogger().warning(
                        "Received protobuffer from Device Manager deviceStateChangeTopic with an unknown event");
                break;
        }
    }

    else {
        this->

                        getLogger()

                .warning("The parse from array msgContainer was not possible!");
    }
}

bool ARPModule::getDevicesFromDeviceModule() {
    bool result = false;
    int msgSize;

    DeviceModule_Proto::Request reqContainerMsg;
    DeviceModule_Proto::Request_GetAllDevicesRequest* reqMsg = new DeviceModule_Proto::Request_GetAllDevicesRequest();
    reqContainerMsg.set_allocated_get_all_devices_request(reqMsg);

    // Send a request to the DeviceModule to get all his known devices
    DeviceModule_Proto::Reply replyMsg;
    zsdn::RequestUtils::RequestResult requestResult = zsdn::RequestUtils::sendRequest(
            *getZmf(),
            reqContainerMsg,
            replyMsg,
            requestTopic_,
            zsdn::MODULE_TYPE_ID_DeviceModule,
            deviceModuleDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {
            getLogger().information("received reply for getAllDevices request.");

            // Add all devices send in the message to the ARP Map
            int counter;
            for (counter = 0; counter < replyMsg.get_all_devices_reply().devices().size(); counter++) {
                common::topology::Device msgDevice = replyMsg.get_all_devices_reply().devices().Get(counter);
                if (msgDevice.ipv4_address_size() > 0) {
                    addNewDeviceToArpCache(msgDevice.ipv4_address().Get(0), msgDevice.mac_address(),
                                           msgDevice.attachment_point().switch_dpid(),
                                           msgDevice.attachment_point().switch_port());
                }
            }
            getLogger().information(std::to_string(counter) + " Devices added");
            result = true;
        }
            break;
        case zsdn::RequestUtils::TIMEOUT:
            getLogger().information(
                    "Timeout - No response getAllDevices message received within expected timeframe.");
            break;
        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning("Module with type MODULE_TYPE_ID_DeviceModule not found.");
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            getLogger().warning("Failed to serialize request.");
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            getLogger().warning("Failed to parse the response-message.");
            break;
    }


    return result;
}

void ARPModule::updateArpCacheCache(std::uint32_t ipAddressDevice, std::uint64_t macAddressDevice,
                                    std::uint64_t switchDpid, std::uint32_t switchPort) {

    ARPModule::IteratorArpCache itArpCache;

    // Key found modify the device in the ARP cache map
    itArpCache = arpCache_.find(macAddressDevice);
    if (itArpCache != arpCache_.end()) {
        modifyDeviceInArpCache(itArpCache, ipAddressDevice, switchDpid, switchPort);
        std::string macAddDesAsString = zsdn::NetUtils::uint64_to_mac_address_string(macAddressDevice);
        #if ARP_MODULE_ENABLE_DEBUG_INFO
        //show some debug info
        this->getLogger().information("Updated ARP cache Entry: Device with MAC " + macAddDesAsString
                                      + " and ip address: " + Tins::IPv4Address(ipAddressDevice).to_string()
                                      + " attached to port " + std::to_string(switchPort)
                                      + " of switch Id " + std::to_string(switchDpid));
        #endif
    }
        // Key not found add the device to the ARP cache map
    else {
        addNewDeviceToArpCache(ipAddressDevice, macAddressDevice, switchDpid, switchPort);


        std::string macAddDesAsString = zsdn::NetUtils::uint64_to_mac_address_string(macAddressDevice);
        #if ARP_MODULE_ENABLE_DEBUG_INFO
        this->getLogger().information("Added ARP cache Entry: Device with MAC " + macAddDesAsString
                                      + " and ip address: " + Tins::IPv4Address(ipAddressDevice).to_string()
                                      + " attached to port " + std::to_string(switchPort)
                                      + " of switch Id " + std::to_string(switchDpid));
        #endif
    }
}

void ARPModule::addNewDeviceToArpCache(std::uint32_t ipAddressDevice, std::uint64_t macAddressDevice,
                                       std::uint64_t switchDpid, std::uint32_t switchPort) {
    Device newDevice;
    newDevice.ipAddress = ipAddressDevice;
    newDevice.switchDpid = switchDpid;
    newDevice.switchPort = switchPort;
    arpCache_.emplace(macAddressDevice, newDevice);
};

void ARPModule::modifyDeviceInArpCache(ARPModule::IteratorArpCache& arpCacheEntry,
                                       std::uint32_t ipAddressDevice, std::uint64_t switchDpid,
                                       std::uint32_t switchPort) {
    arpCacheEntry->second.ipAddress = ipAddressDevice;
    arpCacheEntry->second.switchDpid = switchDpid;
    arpCacheEntry->second.switchPort = switchPort;
}

std::uint32_t ARPModule::getPortPacketInCameFrom(of_packet_in_t* ofPacketIn) {
    std::uint32_t port;
    switch (ofPacketIn->version) {
        case OF_VERSION_1_0:
        case OF_VERSION_1_1:
            of_packet_in_in_port_get(ofPacketIn, &port);
            break;

        case OF_VERSION_1_2:
        case OF_VERSION_1_3:
        case OF_VERSION_1_4:
            of_match_t match;
            if (of_packet_in_match_get(ofPacketIn, &match) < 0) {
                throw Poco::Exception("match get failed");
            }
            port = match.fields.in_port;
            break;

        case OF_VERSION_UNKNOWN:
        default:
            throw Poco::Exception("unsupported OF version");
    }

    return port;
}

void ARPModule::packEthFrameInOfPacketOut(std::vector<std::uint8_t>& serializedEth, std::uint32_t switchPort,
                                          of_packet_out_t* arpPacketOut) {
    if (arpPacketOut != nullptr) {
        // Pack the Ethernet packet in an open flow messageß
        of_octets_t arpPayload;
        arpPayload.data = serializedEth.data();
        arpPayload.bytes = serializedEth.size();

        arpPacketOut->version;
        if (of_packet_out_data_set(arpPacketOut, &arpPayload) < 0) {
            throw Poco::Exception("Failed to set data on packet out");
        }
        // Use of_object_t instead of of_action_output_t* --> elsewise resulting in memleak when deleting
        of_object_t output;
        // set the list of actions the Switch does with the packet --> for ARP just one send out the packet
        of_list_action_t* list = of_list_action_new(arpPacketOut->version);
        of_action_output_init(&output, arpPacketOut->version, -1, 1);
        of_list_action_append_bind(list, &output);

        // Set the port of the switch the message has to be sent to
        of_action_output_port_set(&output, switchPort);

        // assign the actions to the ARP packet to be sent
        if (of_packet_out_actions_set(arpPacketOut, list) < 0) {
            of_list_action_delete(list);
            throw Poco::Exception("Failed to set action on packet out");
        }
        of_packet_out_buffer_id_set(arpPacketOut, OF_BUFFER_ID_NO_BUFFER);

        // Release memory: of_list_action_new and of_action_output_new make use of dynamic storage duration
        of_list_action_delete(list);
    }
    else {
        throw Poco::Exception("No arpPacketOut created");
    }
}

bool ARPModule::GetDevicePosInArpCacheByIp(std::uint32_t ipAddressDevice, std::uint32_t& posDeviceInArpCache) {
    bool isDeviceFound = false;

    ARPModule::IteratorArpCache itArpCache = arpCache_.begin();
    while (itArpCache != arpCache_.end() && (!isDeviceFound)) {
        if (ipAddressDevice == itArpCache->second.ipAddress) {
            isDeviceFound = true;
            posDeviceInArpCache = distance(arpCache_.begin(), itArpCache);
            std::string destinationMac = zsdn::NetUtils::uint64_to_mac_address_string(itArpCache->first);
            #if ARP_MODULE_ENABLE_DEBUG_INFO
            //show some debug info
            this->getLogger().information("Found device in the ARP Cache with MAC " + destinationMac
                                          + " attached to port " + std::to_string(itArpCache->second.switchPort)
                                          + " of switch Id " + std::to_string(itArpCache->second.switchDpid)
                                          + " with ip address: " + Tins::IPv4Address(ipAddressDevice).to_string());
            #endif
        }
        else {
            ++itArpCache;
        }
    }

    if (!isDeviceFound) {
        this->getLogger().information("Device with the Ip Address " + Tins::IPv4Address(ipAddressDevice).to_string()
                                      + " not found in map of size " + std::to_string(arpCache_.size()));
    }

    return isDeviceFound;
}

bool ARPModule::GetDeviceFromDeviceModule(std::uint32_t ipAddTargetPackIn, std::uint32_t& posNewDeviceInArpCache) {
    bool isDeviceFound = false;
    DeviceModule_Proto::Request containerMsg;
    DeviceModule_Proto::Request_GetDevicesByFilterRequest* reqMsgByIp =
            new DeviceModule_Proto::Request_GetDevicesByFilterRequest();
    reqMsgByIp->set_ipv4_address_filter(ipAddTargetPackIn);
    containerMsg.set_allocated_get_devices_by_filter_request(reqMsgByIp);

    MessageType requestDeviceByIpTopic = devicemodule_topics::REQUEST().device_module().get_devices_by_filter()
            .build();

    DeviceModule_Proto::Reply replyMsg;

    zsdn::RequestUtils::RequestResult requestResult = zsdn::RequestUtils::sendRequest(
            *getZmf(),
            containerMsg,
            replyMsg,
            requestDeviceByIpTopic,
            zsdn::MODULE_TYPE_ID_DeviceModule,
            deviceModuleDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {

            getLogger().information("received reply for getDeviceByFilter(IP) request.");

            // Check if the message contains a device
            if (replyMsg.get_devices_by_filter_reply().device_size() > 0) {
                // Add the device inside the messge into the ARP Cache
                const common::topology::Device& msgDevice = replyMsg.get_devices_by_filter_reply().device(0);
                #if ARP_MODULE_ENABLE_DEBUG_INFO
                getLogger().information("## ProtoDevice ## Result: MAC " + std::to_string(msgDevice.mac_address())
                                        + " IP " + std::to_string(msgDevice.ipv4_address(0))
                                        + " Switch ID " + std::to_string(msgDevice.attachment_point().switch_dpid())
                                        + " port " + std::to_string(msgDevice.attachment_point().switch_port()));
                #endif

                updateArpCacheCache(ipAddTargetPackIn, msgDevice.mac_address(),
                                    msgDevice.attachment_point().switch_dpid(),
                                    msgDevice.attachment_point().switch_port());
                #if ARP_MODULE_ENABLE_DEBUG_INFO
                getLogger().information("This was the information IP: " + std::to_string(ipAddTargetPackIn)
                                        + " port: " + std::to_string(msgDevice.attachment_point().switch_port())
                                        + " switch id: " +
                                        std::to_string(msgDevice.attachment_point().switch_dpid())
                                        + " MAC: " + std::to_string(msgDevice.mac_address()));
                #endif
                ARPModule::IteratorArpCache itArpCache = arpCache_.find(msgDevice.mac_address());
                posNewDeviceInArpCache = distance(arpCache_.begin(), itArpCache);
                isDeviceFound = true;
            } else {
                // no device inside the message
                getLogger().information("requested Device with IP: "
                                        + Tins::IPv4Address(ipAddTargetPackIn).to_string()
                                        + " is unknown by the Devicemanager.");
            }

        }
            break;

        case zsdn::RequestUtils::TIMEOUT:
            getLogger().information(
                    "Timeout - No response getDeviceByFilter message received within expected timeframe.");
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning("Module with type MODULE_TYPE_ID_DeviceModule not found.");
            break;

        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            getLogger().warning("Failed to serialize request.");
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            getLogger().warning("Failed to parse the response-message.");
            break;
    }

    return isDeviceFound;
}