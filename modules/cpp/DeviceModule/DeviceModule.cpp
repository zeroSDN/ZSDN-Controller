#include <zsdn/topics/DeviceModule_topics.hpp>
#include <LociExtensions.h>
#include "DeviceModule.hpp"
#include <zsdn/proto/LinkDiscoveryModule.pb.h>
#include <RequestUtils.h>
#include <zsdn/Configs.h>
#include "Poco/RegularExpression.h"
#include "Poco/StringTokenizer.h"

extern "C" {
}

DeviceModule::DeviceModule(uint64_t instanceId) : AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
                                                                 MODULE_VERSION, "DeviceModule",
                                                                 {{zsdn::MODULE_TYPE_ID_LinkDiscoveryModule, linkDiscoveryModuleDependencyVersion_}}) {
}

DeviceModule::~DeviceModule() {
}

bool DeviceModule::enable() {

    // Check config file to set the topics to subscribe to
    std::string subscriptionConfig;
    int multicastGroups;

    try {
        bool configProtocolsRead = getZmf()->getConfigurationProvider()->getAsString(zsdn::ZSDN_DEVICE_MODULE_PROTOCOLS_TO_USE,
                                                                                     subscriptionConfig);
        bool configMulticastGroupsRead = getZmf()->getConfigurationProvider()->getAsInt(
                zsdn::ZSDN_SWITCH_ADAPTER_PACKET_IN_MULTICAST_GROUPS, multicastGroups);

        if (!configProtocolsRead) {
            getLogger().error("Could not read config value " + std::string(zsdn::ZSDN_DEVICE_MODULE_PROTOCOLS_TO_USE));
            return false;
        }
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


    // Split the comma seperated string to retreive the single topics
    Poco::StringTokenizer st(subscriptionConfig, ",", Poco::StringTokenizer::TOK_TRIM);

    bool usingARP = false;
    bool usingIPv4 = false;
    bool usingIPv6 = false;

    for (int i = 0; i < st.count(); i++) {
        std::string input(st[i]);
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);

        usingARP = usingARP || (input == SUBSCRIPTIONS_VALIDTYPES_ARP);
        usingIPv4 = usingIPv4 || (input == SUBSCRIPTIONS_VALIDTYPES_IPV4);
        usingIPv6 = usingIPv6 || (input == SUBSCRIPTIONS_VALIDTYPES_IPV6);

    }

    /// Topic for IPv4 packet_in's
    /// Topic for IPv6 packet_in's

    if (usingARP) {
        getLogger().information("Using ARP protocol to find Hosts");

        for (int i = 0; i < multicastGroups; i++) {
            zmf::data::MessageType topicsARP_ = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().multicast_group(
                    i).arp().build();
            getZmf()->subscribe(topicsARP_,
                                [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                                    handlePacketIn(msg, sender);
                                });
        }

    }

    if (usingIPv4) {
        getLogger().information("Using IPv4 protocol to find Hosts");

        for (int i = 0; i < multicastGroups; i++) {
            zmf::data::MessageType topicsIpv4_ = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().multicast_group(
                    i).ipv4().build();
            getZmf()->subscribe(topicsIpv4_,
                                [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                                    handlePacketIn(msg, sender);
                                });
        }

    }

    if (usingIPv6) {
        getLogger().information("Using IPv6 protocol to find Hosts");

        for (int i = 0; i < multicastGroups; i++) {
            zmf::data::MessageType topicsIpv6_ = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().multicast_group(i).ipv6().build();
            getZmf()->subscribe(topicsIpv6_,
                                [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                                    handlePacketIn(msg, sender);
                                });
        }
    }

    //  Request all SwitchToSwitchLinks and subscribe to Events.
    requestAllSwitchToSwitchLinksFromLinkDiscoveryModule();
    getZmf()->subscribe(topicsSwitchLinkEvent_, [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
        handleSwitchLinkEvent(msg, sender);
    });

    return true;
}

void DeviceModule::disable() {
    devicesCache_.clear();
}


zmf::data::ZmfOutReply DeviceModule::handleRequest(const zmf::data::ZmfMessage& message,
                                                   const zmf::data::ModuleUniqueId& sender) {
    DeviceModule_Proto::Request request;

    bool parseSuccess = request.ParseFromString(message.getData());
    if (!parseSuccess) {
        getLogger().warning(
                "For Request from " + sender.getString() + " received invalid ProtoBuffer request format.");
        return zmf::data::ZmfOutReply::createNoReply();
    }

    switch (request.RequestMsg_case()) {
        case DeviceModule_Proto::Request::kGetAllDevicesRequest: {
            // Build Proto-Message
            DeviceModule_Proto::Reply_GetAllDevicesReply* msgAllDevices = new DeviceModule_Proto::Reply_GetAllDevicesReply();

            for (auto& element : devicesCache_) {
                common::topology::Device* protoDevice = Device::convertToProtoDevice(element.second);
                msgAllDevices->add_devices()->CopyFrom(*protoDevice);
                delete protoDevice;
            }

            getLogger().information("Replying to GetAllDevicesRequest from " + sender.getString() + " with " +
                                    std::to_string(msgAllDevices->devices_size()) + " Devices.");

            // Serialize and build ZmfMessage
            DeviceModule_Proto::Reply reply;
            reply.set_allocated_get_all_devices_reply(msgAllDevices);

            return zmf::data::ZmfOutReply::createImmediateReply(
                    zmf::data::ZmfMessage(topicsAllDevicesReply_, reply.SerializeAsString()));
        }
        case DeviceModule_Proto::Request::kGetDeviceByMacAddressRequest: {

            // Build Proto-Message
            DeviceModule_Proto::Reply_GetDeviceByMACaddressReply* msgDeviceByMAC = new DeviceModule_Proto::Reply_GetDeviceByMACaddressReply();

            Device* device = getDeviceByMACAddress(request.get_device_by_mac_address_request().mac_address_of_device());

            if (device != nullptr) {
                common::topology::Device* protoDevice = Device::convertToProtoDevice(*device);
                msgDeviceByMAC->set_allocated_device(protoDevice);

                std::string ipv4AddressAsString = "-";
                if (protoDevice->ipv4_address_size() > 0) {
                    // Using index 0 for getting the first (and only) MAC Address requested
                    ipv4AddressAsString = Tins::IPv4Address(protoDevice->ipv4_address(0)).to_string();
                }

                std::string ipv6AddressAsString = "-";
                if (protoDevice->ipv6_address_size() > 0) {
                    ipv6AddressAsString = Tins::IPv6Address(protoDevice->ipv6_address(0)).to_string();
                }

                getLogger().information(
                        "Replying to GetDeviceByMacAddressRequest from " + sender.getString() + " for MAC Address "
                        + zsdn::NetUtils::uint64_to_mac_address_string(
                                request.get_device_by_mac_address_request().mac_address_of_device()) +
                        " with Device: \n \t " +
                        " MAC Address: " + zsdn::NetUtils::uint64_to_mac_address_string(protoDevice->mac_address())
                        + " IPv4 Address: " + ipv4AddressAsString
                        + " IPv6 Address: " + ipv6AddressAsString
                        + " Switch ID: " + std::to_string(protoDevice->attachment_point().switch_dpid())
                        + " Switch Port: " + std::to_string(protoDevice->attachment_point().switch_port()));
            } else {
                getLogger().trace("Requested device not found, sending empty reply back to requester");
            }

            // Serialize and build ZmfMessage
            DeviceModule_Proto::Reply reply;
            reply.set_allocated_get_device_by_mac_address_reply(msgDeviceByMAC);

            return zmf::data::ZmfOutReply::createImmediateReply(
                    zmf::data::ZmfMessage(topicsDeviceByMacReply_, reply.SerializeAsString()));
        }
        case DeviceModule_Proto::Request::kGetDevicesByFilterRequest: {

            // Extract the filter request
            DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequest = request.get_devices_by_filter_request();

            DeviceModule_Proto::Reply_GetDevicesByFilterReply* msgDevicesByFilter = new DeviceModule_Proto::Reply_GetDevicesByFilterReply();


            if (filterRequest.has_mac_address_filter()) {
                std::map<uint64_t, Device>::iterator foundDevice = devicesCache_.find(
                        filterRequest.mac_address_filter());
                if (foundDevice != devicesCache_.end()) {
                    // We only have to check this single device for further filters because the MAC is unique
                    Device macFilteredDevice(0, {}, {}, 0, 0);
                    macFilteredDevice = foundDevice->second;
                    if (isDevicePassingFurtherFilters(filterRequest, macFilteredDevice)) {
                        // Device has also passed other filters --> add
                        common::topology::Device* protoDevice = Device::convertToProtoDevice(macFilteredDevice);

                        // Logging only one device because it was filtered by MAC Address (unique)
                        std::string ipv4AddressAsString = "-";
                        if (protoDevice->ipv4_address_size() > 0) {
                            ipv4AddressAsString = Tins::IPv4Address(protoDevice->ipv4_address(0)).to_string();
                        }

                        std::string ipv6AddressAsString = "-";
                        if (protoDevice->ipv6_address_size() > 0) {
                            ipv6AddressAsString = Tins::IPv6Address(protoDevice->ipv6_address(0)).to_string();
                        }

                        getLogger().information(
                                "Replying to GetDevicesByFilterRequest from " + sender.getString() +
                                " for MAC Address "
                                + zsdn::NetUtils::uint64_to_mac_address_string(
                                        request.get_device_by_mac_address_request().mac_address_of_device()) +
                                " with Device: \n \t " +
                                " MAC Address: " +
                                zsdn::NetUtils::uint64_to_mac_address_string(protoDevice->mac_address())
                                + " IPv4 Address: " + ipv4AddressAsString
                                + " IPv6 Address: " + ipv6AddressAsString
                                + " Switch ID: " + std::to_string(protoDevice->attachment_point().switch_dpid())
                                + " Switch Port: " + std::to_string(protoDevice->attachment_point().switch_port()));

                        msgDevicesByFilter->add_device()->CopyFrom(*protoDevice);
                        delete protoDevice;
                    }
                }
                // If Device with given MAC address was not found --> There can't be any device satisfying the other filters also --> let reply have 0 devices
            }
                // If MAC Address wasn't filtered, we have to check every Device the DeviceModule knows
            else {
                for (std::map<uint64_t, Device>::iterator iterator = devicesCache_.begin();
                     iterator != devicesCache_.end(); ++iterator) {
                    if (isDevicePassingFurtherFilters(filterRequest, iterator->second)) {
                        common::topology::Device* protoDevice = Device::convertToProtoDevice(iterator->second);

                        std::string ipv4AddressAsString = "-";
                        if (protoDevice->ipv4_address_size() > 0) {
                            ipv4AddressAsString = Tins::IPv4Address(protoDevice->ipv4_address(0)).to_string();
                        }

                        std::string ipv6AddressAsString = "-";
                        if (protoDevice->ipv6_address_size() > 0) {
                            ipv6AddressAsString = Tins::IPv6Address(protoDevice->ipv6_address(0)).to_string();
                        }

                        getLogger().information(
                                "Found matching Device for Filter \n \t MAC Address: "
                                + zsdn::NetUtils::uint64_to_mac_address_string(protoDevice->mac_address())
                                + " IPv4 Address: " + ipv4AddressAsString
                                + " IPv6 Address: " + ipv6AddressAsString
                                + " Switch ID: " + std::to_string(protoDevice->attachment_point().switch_dpid())
                                + " Switch Port: " + std::to_string(protoDevice->attachment_point().switch_port()));

                        msgDevicesByFilter->add_device()->CopyFrom(*protoDevice);
                        delete protoDevice;
                    }
                }

                getLogger().information(
                        "Replying to GetDevicesByFilterRequest from " + sender.getString() + " with " +
                        std::to_string(msgDevicesByFilter->device_size()) + " Devices.");
            }

            // Serialize and build ZmfMessage
            DeviceModule_Proto::Reply reply;
            reply.set_allocated_get_devices_by_filter_reply(msgDevicesByFilter);

            return zmf::data::ZmfOutReply::createImmediateReply(
                    zmf::data::ZmfMessage(topicsDevicesByFilterReply_, reply.SerializeAsString()));
        }
        default:
            getLogger().information("Received unknown Request");
            break;
    }
    // Default case: Unknown request
    return zmf::data::ZmfOutReply::createNoReply();
}

void DeviceModule::handlePacketIn(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& id) {
    // Unpack ZmfMessage which contains OpenFlow packet
    of_object_t* ofPacketIn = zsdn::of_object_new_from_data_string_copy(message.getData());

    of_octets_t ofPayload;
    of_packet_in_data_get(ofPacketIn, &ofPayload);

    uint32_t port;
    switch (ofPacketIn->version) {
        case OF_VERSION_1_0:
        case OF_VERSION_1_1:

            of_packet_in_in_port_get(ofPacketIn, &port);
            break;

        case OF_VERSION_1_2:
        case OF_VERSION_1_3:
        case OF_VERSION_1_4: {
            of_match_t match;
            // Assign unused local variable x to suppress compiler warning for ignoring return value
            int x = of_packet_in_match_get(ofPacketIn, &match);
            port = match.fields.in_port;
            break;
        }
        default:
            of_packet_in_delete(ofPacketIn);
            getLogger().warning("Unsupported OF version. Ignoring packet.");
            return;
    }

    // Check if the packet_in comes from a Switch
    for (int i = 0; i < switchToSwitchLinkCache_.size(); i++) {
        if (
                ((switchToSwitchLinkCache_[i].source().switch_dpid() == id.InstanceId) &&
                 (switchToSwitchLinkCache_[i].source().switch_port() == port)) ||
                ((switchToSwitchLinkCache_[i].target().switch_dpid() == id.InstanceId) &&
                 (switchToSwitchLinkCache_[i].target().switch_port() == port))

                ) {
            getLogger().trace("Received packet_in from Switch " + std::to_string(id.InstanceId) + " Port " +
                              std::to_string(port) +
                              " is no Device but a SwitchToSwitchLink. Ignoring packet.");
            of_packet_in_delete(ofPacketIn);
            return;
        }
    }

    uint8_t* payloadData = ofPayload.data;
    uint16_t payloadLength = ofPayload.bytes;

    Tins::EthernetII ethPacket(payloadData, payloadLength);
    of_packet_in_delete(ofPacketIn);

    uint64_t senderMacAddress = 0;
    uint32_t senderIpv4Address = 0;
    Tins::IPv6Address ipv6AddressTins;
    std::array<uint8_t, IPV6ADDRESS_SIZE> senderIpv6Address;

    // Extract IP and MAC-Addresses according to the specific type
    Tins::PDU* innerPdu = ethPacket.inner_pdu();
    if (innerPdu == nullptr) {
        getLogger().warning("Received EthernetII Packet with invalid inner PDU.");
        return;
    }
    Tins::PDU::PDUType pduType = ethPacket.inner_pdu()->pdu_type();

    switch (pduType) {
        case Tins::PDU::PDUType::ARP: {
            getLogger().information("Received ARP Packet.");
            Tins::ARP* arpPacket = (Tins::ARP*) ethPacket.inner_pdu();

            if (arpPacket->sender_hw_addr().is_broadcast()) {
                // ignore
                getLogger().information("ARP packet_in is broadcast. Ignoring packet.");
                return;
            }
            else {
                senderMacAddress = zsdn::NetUtils::mac_address_tins_to_uint64(arpPacket->sender_hw_addr());
                senderIpv4Address = arpPacket->sender_ip_addr();
            }
            break;
        }
        case Tins::PDU::PDUType::IP: {
            getLogger().trace("Received IPv4 Packet.");
            Tins::IP* ipv4Packet = (Tins::IP*) ethPacket.inner_pdu();
            senderIpv4Address = ipv4Packet->src_addr();

            // Check if IPv4 address is broadcast (255.255.255.255)
            if (senderIpv4Address == 0xFFFFFFFF) {
                getLogger().information("IPv4 packet_in is broadcast. Ignoring packet.");
                return;
            }
            senderMacAddress = zsdn::NetUtils::mac_address_tins_to_uint64(ethPacket.src_addr());
            break;
        }
        case Tins::PDU::PDUType::IPv6: {
            getLogger().trace("Received IPv6 Packet.");
            Tins::IPv6* ipv6Packet = (Tins::IPv6*) ethPacket.inner_pdu();

            senderMacAddress = zsdn::NetUtils::mac_address_tins_to_uint64(ethPacket.src_addr());
            ipv6AddressTins = ipv6Packet->src_addr();
            std::copy(ipv6AddressTins.begin(), ipv6AddressTins.end(), senderIpv6Address.begin());

            if (isIPv6Broadcast(senderIpv6Address)) {
                getLogger().trace("IPv6 packet_in is broadcast. Ignoring packet.");
                return;
            }

            break;
        }
        default:
            getLogger().trace("Received unknown Packet.");
            return;
    }


    std::map<uint64_t, Device>::iterator senderDevice = devicesCache_.find(senderMacAddress);

    // Add or update if necessary
    if (senderDevice == devicesCache_.end()) {
        // Key not found --> add
        if (pduType == Tins::PDU::PDUType::IP || pduType == Tins::PDU::PDUType::ARP) {
            Device device(senderMacAddress, {senderIpv4Address}, {}, id.InstanceId, port);

            getLogger().information(
                    "Found new Device \n \t "
                            " MAC Address: " + zsdn::NetUtils::uint64_to_mac_address_string(device.macAddress_)
                    + " IPv4 Address: " + Tins::IPv4Address(device.ipv4Addresses_[0]).to_string()
                    + " IPv6 Address: -"
                    + " Switch ID: " + std::to_string(device.attachmentPoint_.switchDpid)
                    + " Switch Port: " + std::to_string(device.attachmentPoint_.switchPort));

            addDevice(device);
        }
        else if (pduType == Tins::PDU::PDUType::IPv6) {
            Device device(senderMacAddress, {}, {senderIpv6Address}, id.InstanceId, port);

            getLogger().information(
                    "Found new Device \n \t "
                            " MAC Address: " + zsdn::NetUtils::uint64_to_mac_address_string(device.macAddress_)
                    + " IPv4 Address: -"
                    + " IPv6 Address: " + ipv6AddressTins.to_string()
                    + " Switch ID: " + std::to_string(device.attachmentPoint_.switchDpid)
                    + " Switch Port: " + std::to_string(device.attachmentPoint_.switchPort));

            addDevice(device);
        }
    }
    else {
        // Key found --> update
        bool updateNeeded = false;
        if (pduType == Tins::PDU::PDUType::ARP || pduType == Tins::PDU::PDUType::IP) {
            updateNeeded = updateDeviceIfNecessary(senderDevice,
                                                   Device(senderMacAddress, {senderIpv4Address}, {}, id.InstanceId,
                                                          port));
        }
        else if (pduType == Tins::PDU::PDUType::IPv6) {
            updateNeeded = updateDeviceIfNecessary(senderDevice,
                                                   Device(senderMacAddress, {}, {senderIpv6Address}, id.InstanceId,
                                                          port));
        }

        std::stringstream ss;
        ss << "Found existing Device " << zsdn::NetUtils::uint64_to_mac_address_string(senderMacAddress) << "\n \t" <<
        "Update necessary: "
        << std::boolalpha << updateNeeded;

        getLogger().trace(ss.str());
    }

    validateDevices();
}

bool DeviceModule::updateDeviceIfNecessary(const std::map<uint64_t, Device>::iterator& existingDeviceIterator,
                                           const Device& newIncomingDevice) {
    bool updateNeeded = false;

    // Update timestamp
    existingDeviceIterator->second.SetTimestampToNow();

    if (newIncomingDevice.ipv4Addresses_.size() > 0) {
        // Same MAC and therefore check if there's another / newer IPv4 and add at beginning
        int ipv4AddressesSize = existingDeviceIterator->second.ipv4Addresses_.size(); // Use extracted size because else it would end up in endless loop
        bool ipv4AlreadyContained = false;
        for (int i = 0; i < ipv4AddressesSize; i++) {
            // Incoming device only contains 1 IP-Address given by ARP packet
            if (existingDeviceIterator->second.ipv4Addresses_[i] == newIncomingDevice.ipv4Addresses_[0]) {
                ipv4AlreadyContained = true;
                break;
            }
        }
        if (!ipv4AlreadyContained) {
            existingDeviceIterator->second.ipv4Addresses_.insert(existingDeviceIterator->second.ipv4Addresses_.begin(),
                                                                 newIncomingDevice.ipv4Addresses_[0]);
            updateNeeded = true;
        }
    }

    if (newIncomingDevice.ipv6Addresses_.size() > 0) {
        bool ipv6AlreadyContained = false;
        for (int i = 0; existingDeviceIterator->second.ipv6Addresses_.size(); i++) {
            if (!(existingDeviceIterator->second.ipv6Addresses_[i] == newIncomingDevice.ipv6Addresses_[0])) {
                break;
            }
            ipv6AlreadyContained = true;
        }
        if (!ipv6AlreadyContained) {
            existingDeviceIterator->second.ipv6Addresses_.insert(existingDeviceIterator->second.ipv6Addresses_.begin(),
                                                                 newIncomingDevice.ipv6Addresses_[0]);

            updateNeeded = true;
        }
    }

    if ((newIncomingDevice.attachmentPoint_.switchDpid != existingDeviceIterator->second.attachmentPoint_.switchDpid) ||
        (newIncomingDevice.attachmentPoint_.switchPort != existingDeviceIterator->second.attachmentPoint_.switchPort)) {

        existingDeviceIterator->second.attachmentPoint_.switchDpid = newIncomingDevice.attachmentPoint_.switchDpid;
        existingDeviceIterator->second.attachmentPoint_.switchPort = newIncomingDevice.attachmentPoint_.switchPort;

        updateNeeded = true;
    }

    if (updateNeeded) {
        common::topology::Device* protoDeviceBeforeUpdate = Device::convertToProtoDevice(
                existingDeviceIterator->second);
        DeviceModule_Proto::From_DeviceEvent::DeviceChanged* msgDeviceEventChanged = new DeviceModule_Proto::From_DeviceEvent::DeviceChanged();

        msgDeviceEventChanged->set_allocated_device_before(protoDeviceBeforeUpdate);

        common::topology::Device* currentDeviceAfterUpdate = Device::convertToProtoDevice(
                existingDeviceIterator->second);
        msgDeviceEventChanged->set_allocated_device_now(currentDeviceAfterUpdate);


        DeviceModule_Proto::From_DeviceEvent* deviceEvent = new DeviceModule_Proto::From_DeviceEvent();
        deviceEvent->set_allocated_device_changed(msgDeviceEventChanged);

        DeviceModule_Proto::From fromMsg;
        fromMsg.set_allocated_device_event(deviceEvent);

        // Serialize and publish
        getZmf()->publish(zmf::data::ZmfMessage(topicsDeviceEventChanged_,
                                                fromMsg.SerializeAsString()));
    }

    return updateNeeded;
}

Device* DeviceModule::getDeviceByMACAddress(uint64_t macAddress) {
    std::map<uint64_t, Device>::iterator deviceElement = devicesCache_.find(macAddress);
    if (deviceElement == devicesCache_.end()) {
        // not found
        return nullptr;
    }
    else {
        Device* devPtr = &deviceElement->second;
        return devPtr;
    }
}

void DeviceModule::addDevice(const Device& device) {
    devicesCache_.emplace(device.macAddress_, device);

    common::topology::Device* newProtoDevice = Device::convertToProtoDevice(device);

    DeviceModule_Proto::From_DeviceEvent* deviceEvent = new DeviceModule_Proto::From_DeviceEvent();
    deviceEvent->set_allocated_device_added(newProtoDevice);

    DeviceModule_Proto::From fromMsg;
    fromMsg.set_allocated_device_event(deviceEvent);

    getZmf()->publish(
            zmf::data::ZmfMessage(topicsDeviceEventAdded_, fromMsg.SerializeAsString()));
}

bool DeviceModule::isDevicePassingFurtherFilters(
        const DeviceModule_Proto::Request_GetDevicesByFilterRequest& filterRequest,
        Device& device) {

    if (filterRequest.has_ipv4_address_filter()) {
        bool ipv4FilterIsSatisfied = false;
        for (int i = 0; i < device.ipv4Addresses_.size(); i++) {
            if (device.ipv4Addresses_[i] == filterRequest.ipv4_address_filter()) {
                // Filter is satisfied
                ipv4FilterIsSatisfied = true;
                break;
            }
        }
        // If filter was not satisfied the device can be rejected
        if (!ipv4FilterIsSatisfied) return false;
    }
    if (filterRequest.has_ipv6_address_filter()) {

        bool ipv6FilterIsSatisfied = false;
        // Convert Proto IPv6 string to array
        Tins::IPv6::address_type tinsIPv6AddressFromFilter = Tins::IPv6::address_type(
                filterRequest.ipv6_address_filter());
        std::array<uint8_t, 16> ipv6AddressFromFilter{};
        std::copy(tinsIPv6AddressFromFilter.begin(), tinsIPv6AddressFromFilter.end(), ipv6AddressFromFilter.begin());

        // Iterate over all IPv6 Addresses of the Device
        for (int i = 0; i < device.ipv6Addresses_.size(); i++) {
            if (device.ipv6Addresses_[i] == ipv6AddressFromFilter) {
                ipv6FilterIsSatisfied = true;
                break;
            }
            else {
                ipv6FilterIsSatisfied = false;
            }
        }
        if (!ipv6FilterIsSatisfied) return false;
    }
    if (filterRequest.has_max_millis_since_last_seen_filter()) {
        if (device.GetMillisSinceLastSeen() > filterRequest.max_millis_since_last_seen_filter()) {
            return false;
        }
    }
    if (filterRequest.has_attachment_point_filter()) {

        if ((device.attachmentPoint_.switchDpid != filterRequest.attachment_point_filter().switch_dpid()) ||
            (device.attachmentPoint_.switchPort != filterRequest.attachment_point_filter().switch_port())) {
            return false;
        }
    }
    if (filterRequest.has_switch_dpid_filter()) {

        if (device.attachmentPoint_.switchDpid != filterRequest.switch_dpid_filter()) {
            return false;
        }
    }

    return true;
}

bool DeviceModule::isIPv6Broadcast(std::array<uint8_t, 16> ipv6Address) {
    for (int i = 0; i < ipv6Address.size(); i++) {
        if (ipv6Address[i] != 0) {
            return false;
        }
    }
    // Passed all filter
    return true;
}

void DeviceModule::handleSwitchLinkEvent(const ZmfMessage& message, const ModuleUniqueId& id) {
    // Container of the Zmf message
    LinkDiscoveryModule_Proto::From msgContainer;

    bool parseSuccess = msgContainer.ParseFromArray(message.getDataRaw(), message.getDataLength());
    if (!parseSuccess) {
        getLogger().warning(
                "For FROM message from LinkDiscoveryModule " + std::to_string(id.TypeId) + ":" +
                std::to_string(id.InstanceId) +
                " received invalid ProtoBuffer request format.");
        return;
    }
    // Get the specific event message out of the message container
    const LinkDiscoveryModule_Proto::From_SwitchLinkEvent& switchLinkEvent = msgContainer.switch_link_event();

    switch (switchLinkEvent.SwitchLinkEventType_case()) {
        case LinkDiscoveryModule_Proto::From_SwitchLinkEvent::kSwitchLinkAdded:
            getLogger().information(
                    "Received SwitchLinkEvent Added from LinkDiscoveryModule " + std::to_string(id.TypeId) + ":" +
                    std::to_string(id.InstanceId));

            addNewSwitchToSwitchLink(switchLinkEvent.switch_link_added());
            break;
        case LinkDiscoveryModule_Proto::From_SwitchLinkEvent::kSwitchLinkRemoved:
            getLogger().information(
                    "Received SwitchLinkEvent Removed from LinkDiscoveryModule " + std::to_string(id.TypeId) + ":" +
                    std::to_string(id.InstanceId));

            removeSwitchToSwitchLink(switchLinkEvent.switch_link_removed());
            break;
        default:
            getLogger().warning("Received unkown SwitchLinkEvent from LinkDiscoveryModule");
            break;
    }
}

void DeviceModule::addNewSwitchToSwitchLink(const common::topology::SwitchToSwitchLink& newLink) {
    switchToSwitchLinkCache_.insert(switchToSwitchLinkCache_.end(), newLink);
    getLogger().information("Added new SwitchToSwitchLink for Switches from Source-Switch " +
                            std::to_string(newLink.source().switch_dpid()) + " at Port " +
                            std::to_string(newLink.source().switch_port()) + " to Target-Switch " +
                            std::to_string(newLink.target().switch_dpid()) + " at Port " +
                            std::to_string(newLink.target().switch_port()));
    validateDevices();
}

void DeviceModule::removeSwitchToSwitchLink(const common::topology::SwitchToSwitchLink& removedLink) {
    std::string serializedRemovedLink = removedLink.SerializeAsString();

    // For all SwitchToSwitchLink find the one that should be removed by comparing the bytes.
    int indexToRemove = -1;
    for (int i = 0; i < switchToSwitchLinkCache_.size(); i++) {
        std::string serializedSTSLink = switchToSwitchLinkCache_[i].SerializeAsString();
        if (serializedRemovedLink == serializedSTSLink) {
            indexToRemove = i;
            break;
        }
    }

    if (indexToRemove != -1) {
        // Remove the found Tupel.
        getLogger().information("Removed SwitchToSwitchLink from Source-Switch " +
                                std::to_string(
                                        switchToSwitchLinkCache_[indexToRemove].source().switch_dpid()) +
                                " at Port " +
                                std::to_string(
                                        switchToSwitchLinkCache_[indexToRemove].source().switch_port()) +
                                " to Target-Switch " +
                                std::to_string(
                                        switchToSwitchLinkCache_[indexToRemove].target().switch_dpid()) +
                                " at Port " +
                                std::to_string(
                                        switchToSwitchLinkCache_[indexToRemove].target().switch_port()));

        switchToSwitchLinkCache_.erase(
                switchToSwitchLinkCache_.begin() +
                indexToRemove);
    }
    else {
        getLogger().information(
                "SwitchToSwitchLink to remove wasn't already registered in SwitchToSwitchLinkCache. Removing nothing.");
    }

}

void DeviceModule::requestAllSwitchToSwitchLinksFromLinkDiscoveryModule() {
    getLogger().information("Trying to Request GetAllSwitchLinks from a LinkDiscoveryModule.");

    LinkDiscoveryModule_Proto::Request request;

    LinkDiscoveryModule_Proto::Request_GetAllSwitchLinksRequest* getAllSwitchLinksRequest = new LinkDiscoveryModule_Proto::Request_GetAllSwitchLinksRequest();
    request.set_allocated_get_all_switch_links_request(getAllSwitchLinksRequest);

    LinkDiscoveryModule_Proto::Reply reply;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(), request, reply, topicsGetAllSwitchLinks_,
                                            zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                            linkDiscoveryModuleDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {

            int counter = 0;
            for (int i = 0; i < reply.get_all_switch_links_reply().switch_links_size(); i++) {
                addNewSwitchToSwitchLink(reply.get_all_switch_links_reply().switch_links(i));
                counter++;
            }
            getLogger().information("Added " + std::to_string(counter) +
                                    " SwitchToSwitchLinks to the SwitchToSwitchLinksCache.");
        }
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning(
                    "Request aborted. Currently no modules with the type MODULE_TYPE_ID_LinkDiscoveryModule are available.");
            break;

        case zsdn::RequestUtils::TIMEOUT:
            getLogger().warning("Timeout when waiting for all SwitchToSwitchLinks.");
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            getLogger().warning("Serialization of the Request failed.");
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            getLogger().warning("Parsing of the Response failed.");
            break;
    }
}

void DeviceModule::validateDevices() {

//     Iterate over all Devices
    getLogger().information("Validating " + std::to_string(devicesCache_.size()) + " Devices "
                            + "against " + std::to_string(switchToSwitchLinkCache_.size()) + " SwitchToSwitchLinks.");
    // Increment not in loop header but internally
    for (auto it = devicesCache_.begin(); it != devicesCache_.end();) {
        // Check if one Device is possibly a Switch; If so --> remove
        bool hasErasedDevice = false;
        for (int i = 0; i < switchToSwitchLinkCache_.size(); i++) {
            if (((switchToSwitchLinkCache_[i].source().switch_dpid() == it->second.attachmentPoint_.switchDpid) &&
                 (switchToSwitchLinkCache_[i].source().switch_port() == it->second.attachmentPoint_.switchPort)) ||
                ((switchToSwitchLinkCache_[i].target().switch_dpid() == it->second.attachmentPoint_.switchDpid) &&
                 (switchToSwitchLinkCache_[i].target().switch_port() == it->second.attachmentPoint_.switchPort))) {
                std::string invalidMacAddress = zsdn::NetUtils::uint64_to_mac_address_string(it->second.macAddress_);
                // Erase Device and increment Iterator accordingly
                common::topology::Device* deletedProtoDevice = Device::convertToProtoDevice(it->second);

                devicesCache_.erase(it++);
                hasErasedDevice = true;
                getLogger().information("Validated Device with MAC-Address: "
                                        + invalidMacAddress
                                        + " as a Switch. Removed invalid Device. Currently known Devices = "
                                        + std::to_string(devicesCache_.size()));

                DeviceModule_Proto::From_DeviceEvent* deviceEvent = new DeviceModule_Proto::From_DeviceEvent();
                deviceEvent->set_allocated_device_removed(deletedProtoDevice);

                DeviceModule_Proto::From fromMsg;
                fromMsg.set_allocated_device_event(deviceEvent);

                getZmf()->publish(
                        zmf::data::ZmfMessage(topicsDeviceEventRemoved_, fromMsg.SerializeAsString()));

            }
        }
        if (!hasErasedDevice) {
            ++it;
        }
    }
}