//
// Created by zsdn on 6/25/15.
// Last edited by Tobias Freundorfer on 14.08.2015
//

#include <cppunit/config/SourcePrefix.h>
#include <DeviceModule.hpp>
#include <zmf/IZmfInstanceController.hpp>
#include <zmf/ZmfInstance.hpp>
#include <NetUtils.h>
#include <LociExtensions.h>
#include "DeviceModuleTests.h"
#include "dummyModules/DummyModule.hpp"
#include <zsdn/proto/LinkDiscoveryModule.pb.h>
#include <zsdn/topics/DeviceModuleTopics.hpp>
#include <UnittestConfigUtil.hpp>


extern "C" {
}

CPPUNIT_TEST_SUITE_REGISTRATION(DeviceModuleTests);      //  Registers the fixture in the 'registry'


std::shared_ptr<zmf::data::ZmfMessage> DeviceModuleTests::build_DemoARP_ZmfMessage(Device device) {
    // Build EthernetII containing ARP packet with given IP- and MAC-Address
    Tins::EthernetII ethPacket = Tins::EthernetII() / Tins::ARP();
    // Because of subscription to ARP we are sure that the inner PDU (IP) has to be an ARP packet
    Tins::ARP* arpPacket = (Tins::ARP*) ethPacket.inner_pdu();

    arpPacket->sender_ip_addr(Tins::ARP::ipaddress_type(device.ipv4Addresses_[0]));
    arpPacket->sender_hw_addr(
            Tins::ARP::hwaddress_type(zsdn::NetUtils::uint64_to_mac_address_string(device.macAddress_)));


    arpPacket->opcode(Tins::ARP::REQUEST);

    std::vector<uint8_t> serializedEth = ethPacket.serialize();
    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_in_t* pIn = of_packet_in_new(OF_VERSION_1_0);
    of_packet_in_in_port_set(pIn, device.attachmentPoint_.switchPort);
    int i = of_packet_in_data_set(pIn, &of_oc);

    std::string data = zsdn::of_object_serialize_to_data_string(pIn);
    of_packet_in_delete(pIn);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsArp_, data));
}

std::shared_ptr<zmf::data::ZmfMessage> DeviceModuleTests::build_DemoIPv4_ZmfMessage(Device device) {
    // Build EthernetII containing IPv4 packet with given IP- and MAC-Address
    Tins::EthernetII ethPacket = Tins::EthernetII() / Tins::IP();

    Tins::IP* ipv4Packet = (Tins::IP*) ethPacket.inner_pdu();

    ethPacket.src_addr(
            Tins::EthernetII::address_type(zsdn::NetUtils::uint64_to_mac_address_string(device.macAddress_)));
    ipv4Packet->src_addr(Tins::IP::address_type(device.ipv4Addresses_[0]));

    std::vector<uint8_t> serializedEth = ethPacket.serialize();
    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_version_e versionUnderTest = OF_VERSION_1_3;
    of_packet_in_t* pIn = of_packet_in_new(versionUnderTest);

    of_port_no_t port = device.attachmentPoint_.switchPort;

    of_packet_in_buffer_id_set(pIn, OF_BUFFER_ID_NO_BUFFER);
    of_match_v3_t* matchv3 = of_match_v3_new(of_version_e::OF_VERSION_1_3);
    of_list_oxm_t* oxm = of_list_oxm_new(of_version_e::OF_VERSION_1_3);
    of_oxm_in_port_t* oxmPort = of_oxm_in_port_new(of_version_e::OF_VERSION_1_3);
    of_oxm_in_port_value_set(oxmPort, port);
    of_list_oxm_append(oxm, oxmPort);
    int x = of_match_v3_oxm_list_set(matchv3, oxm);
    of_match_t match;
    of_match_v3_to_match(matchv3, &match);
    int xx = of_packet_in_match_set(pIn, &match);

    int y = of_packet_in_match_set(pIn, &match);
    int yy = of_packet_in_data_set(pIn, &of_oc);

    std::string data = zsdn::of_object_serialize_to_data_string(pIn);
    of_packet_in_delete(pIn);
    of_match_v3_delete(matchv3);
    of_oxm_delete(oxm);
    of_oxm_in_port_delete(oxmPort);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsIpv4_, data));
}

std::shared_ptr<zmf::data::ZmfMessage> DeviceModuleTests::build_DemoIPv6_ZmfMessage(Device device) {


    Tins::EthernetII ethPacket = Tins::EthernetII() / Tins::IPv6();

    Tins::IPv6* ipv6Packet = (Tins::IPv6*) ethPacket.inner_pdu();

    ethPacket.src_addr(
            Tins::EthernetII::address_type(zsdn::NetUtils::uint64_to_mac_address_string(device.macAddress_)));
    ethPacket.dst_addr(Tins::EthernetII::address_type("00:01:fa:9e:1a:c1"));

    uint8_t ipv6Tins[16];
    std::copy(device.ipv6Addresses_[0].begin(), device.ipv6Addresses_[0].end(), ipv6Tins);

    ipv6Packet->src_addr(Tins::IPv6::address_type(ipv6Tins));
    ipv6Packet->dst_addr(Tins::IPv6::address_type("fe80::caf7:33ff:fe15:76f2"));

    Tins::UDP* x = new Tins::UDP();
    x->dport(12);
    x->sport(444);
    Tins::RawPDU* c = new Tins::RawPDU("MEMES");
    x->inner_pdu(c);
    ipv6Packet->inner_pdu(x);

    std::vector<uint8_t> serializedEth = ethPacket.serialize();
    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_in_t* pIn = of_packet_in_new(OF_VERSION_1_0);

    of_packet_in_in_port_set(pIn, device.attachmentPoint_.switchPort);
    int i = of_packet_in_data_set(pIn, &of_oc);

    std::string data = zsdn::of_object_serialize_to_data_string(pIn);
    of_packet_in_delete(pIn);


    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsIpv6_, data));
}

std::shared_ptr<zmf::data::ZmfMessage> DeviceModuleTests::build_DemoRequest_ZmfMessage(const int requestType,
                                                                                       uint64_t macAddress,
                                                                                       uint32_t ipv4Address,
                                                                                       uint64_t switch_dpid,
                                                                                       uint32_t switch_port,
                                                                                       uint64_t max_millis_since_last_seen,
                                                                                       std::array<uint8_t, 16> ipv6Address) {
    DeviceModule_Proto::Request request;

    switch (requestType) {
        case DeviceModule_Proto::Request::kGetAllDevicesRequestFieldNumber: {
            DeviceModule_Proto::Request_GetAllDevicesRequest* allDevicesRequest = new DeviceModule_Proto::Request_GetAllDevicesRequest();
            request.set_allocated_get_all_devices_request(allDevicesRequest);

            break;
        }
        case DeviceModule_Proto::Request::kGetDeviceByMacAddressRequestFieldNumber: {
            DeviceModule_Proto::Request_GetDeviceByMACaddressRequest* deviceByMACaddressRequest = new DeviceModule_Proto::Request_GetDeviceByMACaddressRequest();
            deviceByMACaddressRequest->set_mac_address_of_device(macAddress);
            request.set_allocated_get_device_by_mac_address_request(deviceByMACaddressRequest);

            break;
        }
        case DeviceModule_Proto::Request::kGetDevicesByFilterRequestFieldNumber: {
            DeviceModule_Proto::Request_GetDevicesByFilterRequest* devicesByFilterRequest = new DeviceModule_Proto::Request_GetDevicesByFilterRequest();
            if (macAddress != 0) {
                devicesByFilterRequest->set_mac_address_filter(macAddress);
            }
            if (ipv4Address != 0) {
                devicesByFilterRequest->set_ipv4_address_filter(ipv4Address);
            }
            // If switch_port filter is set filter for whole AttachmentPoint
            if (switch_port != 0) {
                common::topology::AttachmentPoint* attP = new common::topology::AttachmentPoint();
                attP->set_switch_dpid(switch_dpid);
                attP->set_switch_port(switch_port);
                devicesByFilterRequest->set_allocated_attachment_point_filter(attP);
            }
            else if (switch_dpid != 0) {
                devicesByFilterRequest->set_switch_dpid_filter(switch_dpid);
            }
            if (max_millis_since_last_seen != 0) {
                devicesByFilterRequest->set_max_millis_since_last_seen_filter(max_millis_since_last_seen);
            }
            // Check if a valid IPv6 address is set. If not all values are 0.
            bool ipv6IsNull = true;
            int counter = 0;
            while (ipv6Address.size() > counter) {
                if (ipv6Address[counter] != 0) {
                    ipv6IsNull = false;
                    break;
                }
                counter++;
            }
            if (!ipv6IsNull) {
                Tins::IPv6::address_type tinsIPv6Address;
                std::copy(ipv6Address.begin(), ipv6Address.end(), tinsIPv6Address.begin());
                devicesByFilterRequest->set_ipv6_address_filter(tinsIPv6Address.to_string());
            }


            request.set_allocated_get_devices_by_filter_request(devicesByFilterRequest);

            break;
        }
        default:
            break;
    }

    zmf::data::MessageType topics_allDevicesRequest = zsdn::modules::DeviceModuleTopics<zmf::data::MessageType>().request().device_module().get_all_devices().build();
    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topics_allDevicesRequest, request.SerializeAsString()));
}

std::shared_ptr<zmf::data::ZmfMessage> DeviceModuleTests::build_DemoSwitchLinkEventAdded_ZmfMessage(
        common::topology::SwitchToSwitchLink aSTSLink) {
    LinkDiscoveryModule_Proto::From_SwitchLinkEvent* switchLinkEvent = new LinkDiscoveryModule_Proto::From_SwitchLinkEvent();

    common::topology::SwitchToSwitchLink* aSTSLinkPtr = new common::topology::SwitchToSwitchLink();
    aSTSLinkPtr->CopyFrom(aSTSLink);
    switchLinkEvent->set_allocated_switch_link_added(aSTSLinkPtr);

    LinkDiscoveryModule_Proto::From fromMessage;
    fromMessage.set_allocated_switch_link_event(switchLinkEvent);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsSwitchLinkEvent_, fromMessage.SerializeAsString()));
}

std::shared_ptr<zmf::data::ZmfMessage> DeviceModuleTests::build_DemoSwitchLinkEventRemoved_ZmfMessage(
        common::topology::SwitchToSwitchLink aSTSLink) {
    LinkDiscoveryModule_Proto::From_SwitchLinkEvent* switchLinkEvent = new LinkDiscoveryModule_Proto::From_SwitchLinkEvent();

    common::topology::SwitchToSwitchLink* aSTSLinkPtr = new common::topology::SwitchToSwitchLink();
    aSTSLinkPtr->CopyFrom(aSTSLink);
    switchLinkEvent->set_allocated_switch_link_removed(aSTSLinkPtr);

    LinkDiscoveryModule_Proto::From fromMessage;
    fromMessage.set_allocated_switch_link_event(switchLinkEvent);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsSwitchLinkEvent_, fromMessage.SerializeAsString()));
}

bool DeviceModuleTests::hasDeviceEqualProperties(const Device& device1, const Device& device2) {
    if (device1.macAddress_ != device2.macAddress_) return false;

    if (device1.ipv4Addresses_.size() != device2.ipv4Addresses_.size()) return false;
    for (int i = 0; i < device1.ipv4Addresses_.size(); i++) {
        if (device1.ipv4Addresses_[i] != device2.ipv4Addresses_[i]) return false;
    }

    if (device1.ipv6Addresses_.size() != device2.ipv6Addresses_.size()) return false;
    for (int i = 0; i < device1.ipv6Addresses_.size(); i++) {
        for (int j = 0; j < device1.ipv6Addresses_[i].size(); j++) {
            if (device1.ipv6Addresses_[i][j] != device2.ipv6Addresses_[i][j]) return false;
        }
    }

    if (device1.attachmentPoint_.switchDpid != device2.attachmentPoint_.switchDpid) return false;
    if (device1.attachmentPoint_.switchPort != device2.attachmentPoint_.switchPort) return false;

    return true;
}

void DeviceModuleTests::setUp() {

    // Create some demo Swicht Ports
    demoSwitchPorts_.insert(demoSwitchPorts_.end(), 80);
    demoSwitchPorts_.insert(demoSwitchPorts_.end(), 8080);
    demoSwitchPorts_.insert(demoSwitchPorts_.end(), 8181);

    // Create some demo Devices for testing
    uint64_t macAddress1 = zsdn::NetUtils::mac_address_string_to_uint64("ab:ab:ab:ab:ab:ab");
    std::vector<uint32_t> ipv4Addresses1{Tins::ARP::ipaddress_type("1.1.0.1")};
    demoDevices_.insert(demoDevices_.end(),
                        Device(macAddress1, ipv4Addresses1, {}, demoSwitchInstanceId_, demoSwitchPorts_[0]));

    uint64_t macAddress2 = zsdn::NetUtils::mac_address_string_to_uint64("00:80:41:ae:fd:7e");
    std::vector<uint32_t> ipv4Addresses2{Tins::ARP::ipaddress_type("1.1.0.2")};
    demoDevices_.insert(demoDevices_.end(),
                        Device(macAddress2, ipv4Addresses2, {}, demoSwitchInstanceId_, demoSwitchPorts_[1]));

    uint64_t macAddress3 = zsdn::NetUtils::mac_address_string_to_uint64("00:41:ae:7e:ae:ae");
    std::vector<uint32_t> ipv4Addresses3{Tins::ARP::ipaddress_type("1.1.0.3")};
    demoDevices_.insert(demoDevices_.end(),
                        Device(macAddress3, ipv4Addresses3, {}, demoSwitchInstanceId_, demoSwitchPorts_[2]));

    uint64_t macAddressBroadcast = zsdn::NetUtils::mac_address_string_to_uint64("ff:ff:ff:ff:ff:ff");
    std::vector<uint32_t> ipv4Addresses4{Tins::ARP::ipaddress_type("0.0.0.0")};
    demoDevices_.insert(demoDevices_.end(),
                        Device(macAddressBroadcast, ipv4Addresses4, {}, demoSwitchInstanceId_, demoSwitchPorts_[0]));


    uint64_t macAddress4 = zsdn::NetUtils::mac_address_string_to_uint64("cd:cd:cd:cd:cd:cd");
    Tins::IPv6::address_type tinsIpv6Address4 = Tins::IPv6::address_type("2001:0db8:85a3:08d3:1319:8a2e:0370:7344");
    std::array<uint8_t, 16> ipv6Address4;
    std::copy(tinsIpv6Address4.begin(), tinsIpv6Address4.end(), ipv6Address4.begin());
    demoDevices_.insert(demoDevices_.end(),
                        Device(macAddress4, {}, {ipv6Address4}, demoSwitchInstanceId_, demoSwitchPorts_[0]));

    uint64_t macAddress5 = zsdn::NetUtils::mac_address_string_to_uint64("ab:cd:ab:cd:ab:cd");
    Tins::IPv6::address_type tinsIpv6Address5 = Tins::IPv6::address_type("FE80:0000:0000:0000:0202:B3FF:FE1E:8329");
    std::array<uint8_t, 16> ipv6Address5;
    std::copy(tinsIpv6Address5.begin(), tinsIpv6Address5.end(), ipv6Address5.begin());
    demoDevices_.insert(demoDevices_.end(),
                        Device(macAddress5, {}, {ipv6Address5}, demoSwitchInstanceId_, demoSwitchPorts_[0]));


    // Build the demo Ports
    for (int i = 0; i < 5; i++) {
        common::topology::SwitchPort port;

        common::topology::AttachmentPoint* attP = new common::topology::AttachmentPoint();
        attP->set_switch_dpid(i + 1);
        attP->set_switch_port(i + 2);
        port.set_allocated_attachment_point(attP);

        common::topology::PortSpecs* portSpecs = new common::topology::PortSpecs();
        portSpecs->set_advertised(i + 3);
        std::string* portName = new std::string("portNr: " + std::to_string(i + 2));
        portSpecs->set_allocated_port_name(portName);
        portSpecs->set_config(i + 4);
        portSpecs->set_curr(i + 5);
        portSpecs->set_curr_speed(i + 6);
        portSpecs->set_mac_address(i + 7);
        portSpecs->set_peer(i + 8);
        portSpecs->set_state(i + 9);
        portSpecs->set_supported(i + 10);
        portSpecs->set_max_speed(i + 11);
        port.set_allocated_port_specs(portSpecs);

        demoPorts_.insert(demoPorts_.end(), port);
    }

    // Build the demo Switches
    for (int i = 0; i < 5; i++) {
        common::topology::Switch aSwitch;

        aSwitch.set_switch_dpid(demoPorts_[i].attachment_point().switch_dpid());
        aSwitch.set_openflow_version(OF_VERSION_1_3);
        aSwitch.add_switch_ports()->CopyFrom(demoPorts_[i]);

        common::topology::SwitchSpecs* switchSpecs = new common::topology::SwitchSpecs();
        switchSpecs->set_auxiliary_id(i + 12);
        switchSpecs->set_capabilities(i + 13);
        switchSpecs->set_n_buffers(i + 14);
        switchSpecs->set_n_tables(i + 15);
        switchSpecs->set_reserved(i + 16);
        aSwitch.set_allocated_switch_specs(switchSpecs);


        demoSwitches_.insert(demoSwitches_.end(), aSwitch);
    }

    // Build the demo SwitchToSwitchLinks (are unidirectional --> 2 directed links for bidirectional
    /*
     * Linking Switches
     * (Link1) [0] <--> [1]                         0 ***** 1 ***** 2
     * (Link2) [1] <--> [2]                                  *     *
     * (Link3) [2] <--> [4]                                    *  *
     * (Link4) [4] <--> [1]                                      4
     * (Link5) [4] <--> [3]                                      *
     *                                                           *
     *                                                           3
     */
    // Link1 0-->1
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->set_switch_dpid(demoSwitches_[0].switch_dpid());
        attPSource->CopyFrom(demoSwitches_[0].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[1].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link1 1-->0
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[1].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[0].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link2 1-->2
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[1].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[2].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link2 2-->1
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[2].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[1].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link3 2-->4
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[2].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[4].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link3 4-->2
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[4].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[2].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link4 4-->1
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[4].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[1].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link4 1-->4
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[1].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[4].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link5 4-->3
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[4].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[3].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }
    // Link5 3-->4
    {
        common::topology::SwitchToSwitchLink stsLink;
        common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
        attPSource->CopyFrom(demoSwitches_[3].switch_ports(0).attachment_point());
        stsLink.set_allocated_source(attPSource);
        common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
        attPTarget->CopyFrom(demoSwitches_[4].switch_ports(0).attachment_point());
        stsLink.set_allocated_target(attPTarget);
        demoSwitchLinks_.insert(demoSwitchLinks_.end(), stsLink);
    }

    try {
        // Create module

        linkDiscoveryModuleMock_ = std::shared_ptr<zmf::AbstractModule>(
                new DummyModule(0, 0, zsdn::MODULE_TYPE_ID_LinkDiscoveryModule, "LinkDiscoveryModule",
                                [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                   zmf::data::ModuleState lastState) { }, [this](const zmf::data::ZmfMessage& message,
                                                                                 const zmf::data::ModuleUniqueId& sender) {
                            LinkDiscoveryModule_Proto::Request request;

                            bool parseSuccess = request.ParseFromString(message.getData());
                            if (!parseSuccess) {
                                std::string err = "Could not parse Protobuffer format.";
                                throw Poco::Exception(err, 1);
                            }

                            switch (request.RequestMsg_case()) {
                                case LinkDiscoveryModule_Proto::Request::kGetAllSwitchLinksRequest: {

                                    LinkDiscoveryModule_Proto::Reply reply;

                                    LinkDiscoveryModule_Proto::Reply_GetAllSwitchLinksReply* slReply = new LinkDiscoveryModule_Proto::Reply_GetAllSwitchLinksReply();
                                    for (int i = 0; i < demoSwitchLinks_.size(); i++) {
                                        slReply->add_switch_links()->CopyFrom(demoSwitchLinks_[i]);
                                    }
                                    reply.set_allocated_get_all_switch_links_reply(slReply);

                                    zmf::data::ZmfMessage msg(
                                    zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().reply().link_discovery_module().get_all_switch_links().build(),
                                            reply.SerializeAsString());

                                    return zmf::data::ZmfOutReply::createImmediateReply(msg);
                                }
                                case LinkDiscoveryModule_Proto::Request::kGetLinksFromSwitchRequest:
                                case LinkDiscoveryModule_Proto::Request::kGetLinksToSwitchRequest:
                                case LinkDiscoveryModule_Proto::Request::kGetAllLinksOfSwitchRequest:
                                case LinkDiscoveryModule_Proto::Request::kGetLinksBetweenTwoSwitchesRequest:
                                case LinkDiscoveryModule_Proto::Request::REQUESTMSG_NOT_SET:
                                default:

                                    return zmf::data::ZmfOutReply::createNoReply();
                            }
                        }));

        // Create and start ZMF instance with module
        linkDiscoveryModuleZmfInstance_ = zmf::instance::ZmfInstance::startInstance(linkDiscoveryModuleMock_,
                                                                                    {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                                                    UT_CONFIG_FILE);


        while (!linkDiscoveryModuleMock_->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
    }

    try {
// Create module
        module_ = std::shared_ptr<DeviceModule>(
                new DeviceModule(1));
// Create and start ZMF instance with module
        zmfInstance_ = zmf::instance::ZmfInstance::startInstance(module_,
                                                                 {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                                 UT_CONFIG_FILE);
// ensures that the module is enabled! do NOT delete!
        while (!module_->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch (Poco::Exception exc) {
        // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
    }
}

void DeviceModuleTests::tearDown() {
    zmfInstance_->requestStopInstance();
    linkDiscoveryModuleZmfInstance_->requestStopInstance();
    linkDiscoveryModuleZmfInstance_->joinExecution();
    zmfInstance_->joinExecution();
}

void DeviceModuleTests::testAddDevice() {
    module_->UTAccessor_addDevice(demoDevices_[0]);

    // Check if only one device has been added to the map
    int mapSize = (int) module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(1, mapSize);

    // Check if the MAC-Address as key is correct
    CPPUNIT_ASSERT(module_->UTAccessor_getAllDevicesMap().find(demoDevices_[0].macAddress_) !=
                   module_->UTAccessor_getAllDevicesMap().end());

    // Check if the device properties are correct
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[0],
                                                        module_->UTAccessor_getAllDevicesMap().find(
                                                                demoDevices_[0].macAddress_)->second));

}

void DeviceModuleTests::testGetDeviceByMacAddressRegistered() {
    module_->UTAccessor_addDevice(demoDevices_[0]);
    module_->UTAccessor_addDevice(demoDevices_[1]);
    module_->UTAccessor_addDevice(demoDevices_[2]);

    Device* returnedDevice = module_->UTAccessor_getDeviceByMACAddress(demoDevices_[1].macAddress_);

    CPPUNIT_ASSERT(returnedDevice != nullptr);
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[1], *returnedDevice));
}

void DeviceModuleTests::testGetDeviceByMacAddressNotRegistered() {
    // Pointer to DeviceModule


    module_->UTAccessor_addDevice(demoDevices_[0]);
    module_->UTAccessor_addDevice(demoDevices_[1]);

    // Try to get Device by MAC Address of a not registered device
    Device* returnedDevice = module_->UTAccessor_getDeviceByMACAddress(demoDevices_[2].macAddress_);

    CPPUNIT_ASSERT(returnedDevice == nullptr);
}

void DeviceModuleTests::testHandlePacketInEthernetIINoInnerPDU() {
    // Build EthernetII containing ARP packet with given IP- and MAC-Address
    Tins::EthernetII ethPacket = Tins::EthernetII();

    std::vector<uint8_t> serializedEth = ethPacket.serialize();
    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_in_t* pIn = of_packet_in_new(OF_VERSION_1_0);

    //of_packet_in_in_port_set(pIn, device.attachmentPoint_.switchPort);
    int i = of_packet_in_data_set(pIn, &of_oc);

    std::shared_ptr<zmf::data::ZmfMessage> message = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsArp_, OF_OBJECT_TO_MESSAGE(pIn), pIn->length));

    module_->UTAccessor_handlePacketIn(*message,
                                       demoModuleUniqueId_);
    of_packet_in_delete(pIn);

}

void DeviceModuleTests::testHandlePacketInIPv4BroadcastAddress() {
    Tins::IPv4Address iPv4Address("255.255.255.255");
    Device device(0, {iPv4Address}, {}, 0, 0);
    // Call handle IPv4
    module_->UTAccessor_handlePacketIn(
            *build_DemoIPv4_ZmfMessage(device),
            demoModuleUniqueId_);

    // Check if the right amount of devices have been added
    int mapSize = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(0, mapSize);
}

void DeviceModuleTests::testHandlePacketInIPv6BroadcastAddress() {
    // Pointer to DeviceModule


    Tins::IPv6Address tinsIPv6Address("::");
    std::array<uint8_t, 16> ipv6Address;
    std::copy(tinsIPv6Address.begin(), tinsIPv6Address.end(), ipv6Address.begin());
    Device device(0, {}, {ipv6Address}, 0, 0);

    module_->UTAccessor_handlePacketIn(*build_DemoIPv6_ZmfMessage(device),
                                       demoModuleUniqueId_);

    // Check if the right amount of devices have been added
    int mapSize = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(0, mapSize);
}

void DeviceModuleTests::testHandlePacketInSTSLinkPacket() {
    // Build EthernetII containing ARP packet with given IP- and MAC-Address
    Tins::EthernetII ethPacket = Tins::EthernetII() / Tins::ARP();
    // Because of subscription to ARP we are sure that the inner PDU (IP) has to be an ARP packet
    Tins::ARP* arpPacket = (Tins::ARP*) ethPacket.inner_pdu();

    arpPacket->sender_ip_addr(Tins::ARP::ipaddress_type(demoDevices_[0].ipv4Addresses_[0]));
    arpPacket->sender_hw_addr(
            Tins::ARP::hwaddress_type(zsdn::NetUtils::uint64_to_mac_address_string(demoDevices_[0].macAddress_)));


    arpPacket->opcode(Tins::ARP::REQUEST);

    std::vector<uint8_t> serializedEth = ethPacket.serialize();
    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_in_t* pIn = of_packet_in_new(OF_VERSION_1_0);
    of_packet_in_in_port_set(pIn, demoSwitches_[1].switch_ports(0).attachment_point().switch_port());
    int i = of_packet_in_data_set(pIn, &of_oc);

    std::string data = zsdn::of_object_serialize_to_data_string(pIn);
    of_packet_in_delete(pIn);

    int sizeBefore = module_->UTAccessor_getAllDevicesMap().size();
    module_->UTAccessor_handlePacketIn(zmf::data::ZmfMessage(topicsArp_, data),
                                       zmf::data::ModuleUniqueId(0, demoSwitches_[1].switch_dpid()));
    int sizeAfter = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(sizeBefore, sizeAfter);
}

void DeviceModuleTests::testHandlePacketInUnknownPacket() {
    Tins::EthernetII ethPacket = Tins::EthernetII() / Tins::DHCP();

    std::vector<uint8_t> serializedEth = ethPacket.serialize();
    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_in_t* pIn = of_packet_in_new(OF_VERSION_1_0);
    of_packet_in_in_port_set(pIn, demoDevices_[0].attachmentPoint_.switchPort);
    int i = of_packet_in_data_set(pIn, &of_oc);

    std::string data = zsdn::of_object_serialize_to_data_string(pIn);
    of_packet_in_delete(pIn);

    int sizeBefore = module_->UTAccessor_getAllDevicesMap().size();
    module_->UTAccessor_handlePacketIn(zmf::data::ZmfMessage(topicsArp_, data), demoModuleUniqueId_);
    int sizeAfter = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(sizeBefore, sizeAfter);

}

void DeviceModuleTests::testHandleARPNewDevice() {
    // Call handle ARP for new Device
    module_->UTAccessor_handlePacketIn(
            *build_DemoARP_ZmfMessage(demoDevices_[0]),
            demoModuleUniqueId_);

    // Check if the right amount of devices have been added
    int mapSize = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(1, mapSize);

    // Check if MAC Address key was set correct
    CPPUNIT_ASSERT_EQUAL(demoDevices_[0].macAddress_,
                         module_->UTAccessor_getAllDevicesMap().find(demoDevices_[0].macAddress_)->first);

    // Check if Device with properties in value was set correct
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[0],
                                                        module_->UTAccessor_getAllDevicesMap().find(
                                                                demoDevices_[0].macAddress_)->second));
}

void DeviceModuleTests::testHandleARPExistingDeviceNoUpdate() {
    module_->UTAccessor_handlePacketIn(
            *build_DemoARP_ZmfMessage(demoDevices_[0]),
            demoModuleUniqueId_);
    int mapSizeBeforeUpdate = module_->UTAccessor_getAllDevicesMap().size();

    // New incoming ARP packet of same Device
    module_->UTAccessor_handlePacketIn(
            *build_DemoARP_ZmfMessage(demoDevices_[0]),
            demoModuleUniqueId_);

    // Check if for the incoming ARP packet of the same Device no new Device was created
    int mapSizeAfterUpdate = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(mapSizeBeforeUpdate, mapSizeAfterUpdate);

    // Check if Device with properties in value was set correct
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[0],
                                                        module_->UTAccessor_getAllDevicesMap().find(
                                                                demoDevices_[0].macAddress_)->second));
}

void DeviceModuleTests::testHandleARPExistingDeviceUpdate() {
    // Add a new Device via ARP
    module_->UTAccessor_handlePacketIn(*build_DemoARP_ZmfMessage(demoDevices_[0]),
                                       demoModuleUniqueId_);
    int mapSizeBeforeUpdate = module_->UTAccessor_getAllDevicesMap().size();

    // New incoming ARP packet of same Device with different/new ipv4-Address
    uint32_t newIpv4Address = Tins::ARP::ipaddress_type("10.10.0.1");

    // Copy device and set new ipv4
    Device editedDevice = demoDevices_[0];
    editedDevice.ipv4Addresses_.insert(editedDevice.ipv4Addresses_.begin(), newIpv4Address);

    module_->UTAccessor_handlePacketIn(*build_DemoARP_ZmfMessage(editedDevice),
                                       demoModuleUniqueId_);

    // Check if for the incoming ARP packet of the same Device no new Device was created
    int mapSizeAfterUpdate = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(mapSizeBeforeUpdate, mapSizeAfterUpdate);

    // Check if Device was updated
    CPPUNIT_ASSERT_EQUAL(false, hasDeviceEqualProperties(demoDevices_[0],
                                                         module_->UTAccessor_getAllDevicesMap().find(
                                                                 editedDevice.macAddress_)->second));
}

void DeviceModuleTests::testHandleARPForBroadcast() {
    // Demo Device at index 3 is broadcast
    module_->UTAccessor_handlePacketIn(
            *build_DemoARP_ZmfMessage(demoDevices_[3]),
            demoModuleUniqueId_);

    // Assert that no Device was added --> Broadcast gets ignored by DeviceModule
    int mapSize = module_->UTAccessor_getAllDevicesMap().size();

    CPPUNIT_ASSERT_EQUAL(0, mapSize);

}

void DeviceModuleTests::testHandleIPv4NewDevice() {
    // Pointer to DeviceModule


    module_->UTAccessor_handlePacketIn(*build_DemoIPv4_ZmfMessage(demoDevices_[0]),
                                       demoModuleUniqueId_);

    // Check if the right amount of devices have been added
    int mapSize = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(1, mapSize);

    // Check if MAC Address key was set correct
    CPPUNIT_ASSERT_EQUAL(demoDevices_[0].macAddress_,
                         module_->UTAccessor_getAllDevicesMap().find(demoDevices_[0].macAddress_)->first);

    // Check if Device with properties in value was set correct
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[0],
                                                        module_->UTAccessor_getAllDevicesMap().find(
                                                                demoDevices_[0].macAddress_)->second));
}

void DeviceModuleTests::testHandleIPv4ExistingDeviceNoUpdate() {
    // Add a new Device via ARP
    module_->UTAccessor_handlePacketIn(
            *build_DemoIPv4_ZmfMessage(demoDevices_[0]),
            demoModuleUniqueId_);
    int mapSizeBeforeUpdate = module_->UTAccessor_getAllDevicesMap().size();

    // New incoming ARP packet of same Device
    module_->UTAccessor_handlePacketIn(
            *build_DemoIPv4_ZmfMessage(demoDevices_[0]),
            demoModuleUniqueId_);

    // Check if for the incoming ARP packet of the same Device no new Device was created
    int mapSizeAfterUpdate = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(mapSizeBeforeUpdate, mapSizeAfterUpdate);

    // Check if Device with properties in value was set correct
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[0],
                                                        module_->UTAccessor_getAllDevicesMap().find(
                                                                demoDevices_[0].macAddress_)->second));
}

void DeviceModuleTests::testHandleIPv4ExistingDeviceUpdate() {
    // Add a new Device via ARP
    module_->UTAccessor_handlePacketIn(*build_DemoIPv4_ZmfMessage(demoDevices_[0]),
                                       demoModuleUniqueId_);
    int mapSizeBeforeUpdate = module_->UTAccessor_getAllDevicesMap().size();

    // New incoming IPv4 packet of same Device with different/new ipv4-Address
    uint32_t newIpv4Address = Tins::ARP::ipaddress_type("10.10.0.1");

    // Copy device and set new ipv4
    Device editedDevice = demoDevices_[0];
    editedDevice.ipv4Addresses_.insert(editedDevice.ipv4Addresses_.begin(), newIpv4Address);

    module_->UTAccessor_handlePacketIn(*build_DemoIPv4_ZmfMessage(editedDevice),
                                       demoModuleUniqueId_);

    // Check if for the incoming ARP packet of the same Device no new Device was created
    int mapSizeAfterUpdate = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(mapSizeBeforeUpdate, mapSizeAfterUpdate);

    // Check if Device was updated
    CPPUNIT_ASSERT_EQUAL(false, hasDeviceEqualProperties(demoDevices_[0],
                                                         module_->UTAccessor_getAllDevicesMap().find(
                                                                 editedDevice.macAddress_)->second));
}

void DeviceModuleTests::testHandleIPv6NewDevice() {
    module_->UTAccessor_handlePacketIn(*build_DemoIPv6_ZmfMessage(demoDevices_[4]),
                                       demoModuleUniqueId_);

    // Check if the right amount of devices have been added
    int mapSize = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(1, mapSize);

    // Check if MAC Address key was set correct
    CPPUNIT_ASSERT_EQUAL(demoDevices_[4].macAddress_,
                         module_->UTAccessor_getAllDevicesMap().find(demoDevices_[4].macAddress_)->first);

    // Check if Device with properties in value was set correct
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[4],
                                                        module_->UTAccessor_getAllDevicesMap().find(
                                                                demoDevices_[4].macAddress_)->second));
}

void DeviceModuleTests::testHandleIPv6ExistingDeviceNoUpdate() {
    // Add a new Device via ARP
    module_->UTAccessor_handlePacketIn(
            *build_DemoIPv6_ZmfMessage(demoDevices_[5]),
            demoModuleUniqueId_);
    int mapSizeBeforeUpdate = module_->UTAccessor_getAllDevicesMap().size();

    // New incoming ARP packet of same Device
    module_->UTAccessor_handlePacketIn(
            *build_DemoIPv6_ZmfMessage(demoDevices_[5]),
            demoModuleUniqueId_);

    // Check if for the incoming ARP packet of the same Device no new Device was created
    int mapSizeAfterUpdate = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(mapSizeBeforeUpdate, mapSizeAfterUpdate);

    // Check if Device with properties in value was set correct
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[5],
                                                        module_->UTAccessor_getAllDevicesMap().find(
                                                                demoDevices_[5].macAddress_)->second));
}

void DeviceModuleTests::testHandleIPv6ExistingDeviceUpdate() {
    // Add a new Device via ARP
    module_->UTAccessor_handlePacketIn(*build_DemoIPv6_ZmfMessage(demoDevices_[5]),
                                       demoModuleUniqueId_);
    int mapSizeBeforeUpdate = module_->UTAccessor_getAllDevicesMap().size();

    // New incoming IPv6 packet of same Device with different/new ipv6-Address

    Tins::IPv6::address_type tinsNewIpv6Address = Tins::IPv6::address_type("FE80:ABAB:0000:0000:0202:B3FF:FE1E:8329");
    std::array<uint8_t, 16> newIpv6Address;
    std::copy(tinsNewIpv6Address.begin(), tinsNewIpv6Address.end(), tinsNewIpv6Address.begin());

    // Copy device and set new ipv6
    Device editedDevice = demoDevices_[5];
    editedDevice.ipv6Addresses_.insert(editedDevice.ipv6Addresses_.begin(), newIpv6Address);

    // Additionally edit the AttachmentPoint
    editedDevice.attachmentPoint_.switchPort = 9;


    module_->UTAccessor_handlePacketIn(*build_DemoIPv6_ZmfMessage(editedDevice),
                                       demoModuleUniqueId_);

    // Check if for the incoming ARP packet of the same Device no new Device was created
    int mapSizeAfterUpdate = module_->UTAccessor_getAllDevicesMap().size();
    CPPUNIT_ASSERT_EQUAL(mapSizeBeforeUpdate, mapSizeAfterUpdate);

    // Check if Device was updated
    CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(editedDevice,
                                                        module_->UTAccessor_getAllDevicesMap().find(
                                                                editedDevice.macAddress_)->second));
}

void DeviceModuleTests::testHandleRequestGetAllDevices() {
    // Add devices
    module_->UTAccessor_addDevice(demoDevices_[0]);
    module_->UTAccessor_addDevice(demoDevices_[1]);
    module_->UTAccessor_addDevice(demoDevices_[2]);

    zmf::data::ZmfOutReply zmfOutReply = module_->UTAccessor_handleRequest(*build_DemoRequest_ZmfMessage(
            DeviceModule_Proto::Request::kGetAllDevicesRequestFieldNumber,
            0, 0, 0, 0, 0, std::array<uint8_t, 16>()), demoModuleUniqueId_);

    // Unpack ZmfMessage
    DeviceModule_Proto::Reply reply;
    reply.ParseFromArray(zmfOutReply.reply_immediate.getDataRaw(), zmfOutReply.reply_immediate.getDataLength());

    DeviceModule_Proto::Reply_GetAllDevicesReply allDevicesReply = reply.get_all_devices_reply();

    // Get replied Devices
    std::vector<Device> repliedDevices;
    for (int i = 0; i < allDevicesReply.devices_size(); i++) {
        repliedDevices.insert(repliedDevices.end(), Device(allDevicesReply.devices(i)));
    }

    // Compare the replied Devices with the actual ones WATCH OUT: Devices are listed reverse
    for (int i = 0; i < repliedDevices.size(); i++) {

        CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(repliedDevices[i],
                                                            module_->UTAccessor_getAllDevicesMap().find(
                                                                    repliedDevices[i].macAddress_)->second));
    }
}

void DeviceModuleTests::testHandleRequestGetDeviceByMacAddress() {
    // Add devices
    module_->UTAccessor_addDevice(demoDevices_[0]);
    module_->UTAccessor_addDevice(demoDevices_[1]);
    module_->UTAccessor_addDevice(demoDevices_[2]);
    // Ipv6 device
    module_->UTAccessor_addDevice(demoDevices_[4]);

    {
        uint64_t seekedMacAddress1 = demoDevices_[0].macAddress_;
        zmf::data::ZmfOutReply zmfOutReply1 = module_->UTAccessor_handleRequest(
                *build_DemoRequest_ZmfMessage(DeviceModule_Proto::Request::kGetDeviceByMacAddressRequestFieldNumber,
                                              seekedMacAddress1, 0,
                                              0, 0, 0, std::array<uint8_t, 16>()),
                demoModuleUniqueId_);

        // Unpack ZmfMessage
        DeviceModule_Proto::Reply reply1;
        reply1.ParseFromArray(zmfOutReply1.reply_immediate.getDataRaw(), zmfOutReply1.reply_immediate.getDataLength());

        DeviceModule_Proto::Reply_GetDeviceByMACaddressReply deviceByMACaddressReply1 = reply1.get_device_by_mac_address_reply();

        // Compare the replied Device with the actual one
        CPPUNIT_ASSERT_EQUAL(true, deviceByMACaddressReply1.has_device());
        CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[0], deviceByMACaddressReply1.device()));
    }
    {
        uint64_t seekedMacAddress2 = demoDevices_[2].macAddress_;
        zmf::data::ZmfOutReply zmfOutReply2 = module_->UTAccessor_handleRequest(
                *build_DemoRequest_ZmfMessage(DeviceModule_Proto::Request::kGetDeviceByMacAddressRequestFieldNumber,
                                              seekedMacAddress2, 0,
                                              0, 0, 0, std::array<uint8_t, 16>()),
                demoModuleUniqueId_);

        // Unpack ZmfMessage
        DeviceModule_Proto::Reply reply2;
        reply2.ParseFromArray(zmfOutReply2.reply_immediate.getDataRaw(), zmfOutReply2.reply_immediate.getDataLength());

        DeviceModule_Proto::Reply_GetDeviceByMACaddressReply deviceByMACaddressReply2 = reply2.get_device_by_mac_address_reply();

        // Compare the replied Device with the actual one
        CPPUNIT_ASSERT_EQUAL(true, deviceByMACaddressReply2.has_device());
        CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[2], deviceByMACaddressReply2.device()));
    }
    // Test for IPv6 device
    {
        uint64_t seekedMacAddress = demoDevices_[4].macAddress_;
        zmf::data::ZmfOutReply zmfOutReply = module_->UTAccessor_handleRequest(
                *build_DemoRequest_ZmfMessage(DeviceModule_Proto::Request::kGetDeviceByMacAddressRequest,
                                              demoDevices_[4].macAddress_, 0, 0, 0, 0,
                                              demoDevices_[4].ipv6Addresses_[0]), demoModuleUniqueId_);

        // Unpack ZmfMessage
        DeviceModule_Proto::Reply reply;
        reply.ParseFromArray(zmfOutReply.reply_immediate.getDataRaw(), zmfOutReply.reply_immediate.getDataLength());

        DeviceModule_Proto::Reply_GetDeviceByMACaddressReply deviceByMACaddressReply = reply.get_device_by_mac_address_reply();

        // Compare the replied Device with the actual one
        CPPUNIT_ASSERT_EQUAL(true, deviceByMACaddressReply.has_device());
        CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(demoDevices_[4], deviceByMACaddressReply.device()));
    }
    // Negative case for MAC-Address that is not registered at the DeviceModule
    uint64_t seekedInvalidMacAddress = zsdn::NetUtils::mac_address_string_to_uint64("ab:cd:cd:ab:ab:ab");
    zmf::data::ZmfOutReply zmfOutReplyInvalid = module_->UTAccessor_handleRequest(
            *build_DemoRequest_ZmfMessage(DeviceModule_Proto::Request::kGetDeviceByMacAddressRequestFieldNumber,
                                          seekedInvalidMacAddress, 0, 0, 0, 0, std::array<uint8_t, 16>()),
            demoModuleUniqueId_);

    // Unpack ZmfMessage
    DeviceModule_Proto::Reply replyInvalid;
    replyInvalid.ParseFromArray(zmfOutReplyInvalid.reply_immediate.getDataRaw(),
                                zmfOutReplyInvalid.reply_immediate.getDataLength());

    DeviceModule_Proto::Reply_GetDeviceByMACaddressReply deviceByMACaddressReplyInvalid = replyInvalid.get_device_by_mac_address_reply();

    // Compare the replied Device with the actual one
    CPPUNIT_ASSERT_EQUAL(false, deviceByMACaddressReplyInvalid.has_device());
}

void DeviceModuleTests::testHandleRequestGetDeviceByFilter() {
    // Add devices
    module_->UTAccessor_addDevice(demoDevices_[0]);
    module_->UTAccessor_addDevice(demoDevices_[1]);
    module_->UTAccessor_addDevice(demoDevices_[2]);
    // IPv6 device
    module_->UTAccessor_addDevice(demoDevices_[4]);
    // Filter device for IPv4 Address without MAC
    {

        zmf::data::ZmfOutReply zmfOutReply1 = module_->UTAccessor_handleRequest(*build_DemoRequest_ZmfMessage(
                                                                                        DeviceModule_Proto::Request::kGetDevicesByFilterRequestFieldNumber,
                                                                                        0,
                                                                                        demoDevices_[1].ipv4Addresses_[0],
                                                                                        0,
                                                                                        0,
                                                                                        0,
                                                                                        std::array<uint8_t, 16>{}),
                                                                                demoModuleUniqueId_);

        // Unpack ZmfMessage
        DeviceModule_Proto::Reply reply1;
        reply1.ParseFromArray(zmfOutReply1.reply_immediate.getDataRaw(), zmfOutReply1.reply_immediate.getDataLength());

        DeviceModule_Proto::Reply_GetDevicesByFilterReply filterReply1 = reply1.get_devices_by_filter_reply();

        // Get replied Devices
        std::vector<Device> repliedDevices1;
        for (int i = 0; i < filterReply1.device_size(); i++) {
            repliedDevices1.insert(repliedDevices1.end(), Device(filterReply1.device(i)));
        }

        // Compare the replied Devices with the actual ones WATCH OUT: Devices are listed reverse
        ulong expectedSize1 = 1;
        CPPUNIT_ASSERT_EQUAL(expectedSize1, repliedDevices1.size());
        for (int i = 0; i < repliedDevices1.size(); i++) {

            CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(repliedDevices1[i],
                                                                module_->UTAccessor_getAllDevicesMap().find(
                                                                        repliedDevices1[i].macAddress_)->second));
        }
    }
    // Filter device for IPv6 Address without MAC
    {
        zmf::data::ZmfOutReply zmfOutReply2 = module_->UTAccessor_handleRequest(
                *build_DemoRequest_ZmfMessage(DeviceModule_Proto::Request::kGetDevicesByFilterRequestFieldNumber, 0, 0,
                                              0,
                                              0, 0, demoDevices_[4].ipv6Addresses_[0]),
                demoModuleUniqueId_);



        // Unpack ZmfMessage
        DeviceModule_Proto::Reply reply2;
        reply2.ParseFromArray(zmfOutReply2.reply_immediate.getDataRaw(), zmfOutReply2.reply_immediate.getDataLength());

        DeviceModule_Proto::Reply_GetDevicesByFilterReply filterReply2 = reply2.get_devices_by_filter_reply();

        // Get replied Devices
        std::vector<Device> repliedDevices2;
        for (int i = 0; i < filterReply2.device_size(); i++) {
            repliedDevices2.insert(repliedDevices2.end(), Device(filterReply2.device(i)));
        }

        // Compare the replied Devices with the actual ones WATCH OUT: Devices are listed reverse
        ulong expectedSize2 = 1;
        CPPUNIT_ASSERT_EQUAL(expectedSize2, repliedDevices2.size());
        for (int i = 0; i < repliedDevices2.size(); i++) {
            CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(repliedDevices2[i],
                                                                module_->UTAccessor_getAllDevicesMap().find(
                                                                        repliedDevices2[i].macAddress_)->second));
        }
    }
    // Filter device for IPv6 Address with MAC
    {
        zmf::data::ZmfOutReply zmfOutReply2 = module_->UTAccessor_handleRequest(
                *build_DemoRequest_ZmfMessage(DeviceModule_Proto::Request::kGetDevicesByFilterRequestFieldNumber,
                                              demoDevices_[4].macAddress_, 0,
                                              0,
                                              0, 0, demoDevices_[4].ipv6Addresses_[0]),
                demoModuleUniqueId_);



        // Unpack ZmfMessage
        DeviceModule_Proto::Reply reply2;
        reply2.ParseFromArray(zmfOutReply2.reply_immediate.getDataRaw(), zmfOutReply2.reply_immediate.getDataLength());

        DeviceModule_Proto::Reply_GetDevicesByFilterReply filterReply2 = reply2.get_devices_by_filter_reply();

        // Get replied Devices
        std::vector<Device> repliedDevices2;
        for (int i = 0; i < filterReply2.device_size(); i++) {
            repliedDevices2.insert(repliedDevices2.end(), Device(filterReply2.device(i)));
        }

        // Compare the replied Devices with the actual ones WATCH OUT: Devices are listed reverse
        ulong expectedSize2 = 1;
        CPPUNIT_ASSERT_EQUAL(expectedSize2, repliedDevices2.size());
        for (int i = 0; i < repliedDevices2.size(); i++) {
            CPPUNIT_ASSERT_EQUAL(true, hasDeviceEqualProperties(repliedDevices2[i],
                                                                module_->UTAccessor_getAllDevicesMap().find(
                                                                        repliedDevices2[i].macAddress_)->second));
        }
    }
}

void DeviceModuleTests::testHandleRequestGetDeviceByFilterWithMacFilter() {
    // Add devices
    module_->UTAccessor_addDevice(demoDevices_[0]);
    module_->UTAccessor_addDevice(demoDevices_[1]);
    module_->UTAccessor_addDevice(demoDevices_[2]);

    zmf::data::ZmfOutReply zmfOutReply = module_->UTAccessor_handleRequest(*build_DemoRequest_ZmfMessage(
                                                                                   DeviceModule_Proto::Request::kGetDevicesByFilterRequestFieldNumber,
                                                                                   demoDevices_[1].macAddress_,
                                                                                   demoDevices_[1].ipv4Addresses_[0],
                                                                                   0,
                                                                                   0,
                                                                                   0,
                                                                                   std::array<uint8_t, 16>{}),
                                                                           demoModuleUniqueId_);

    // Unpack ZmfMessage
    DeviceModule_Proto::Reply reply;
    reply.ParseFromArray(zmfOutReply.reply_immediate.getDataRaw(), zmfOutReply.reply_immediate.getDataLength());

    DeviceModule_Proto::Reply_GetDevicesByFilterReply filterReply = reply.get_devices_by_filter_reply();

    CPPUNIT_ASSERT_EQUAL(1, filterReply.device_size());

    // Only one because we filtered with MAC
    common::topology::Device* protoDevice = Device::convertToProtoDevice(demoDevices_[1]);
    std::string serializedExpected = protoDevice->SerializeAsString();
    delete protoDevice;
    std::string serializedActual = filterReply.device(0).SerializeAsString();

    CPPUNIT_ASSERT_EQUAL(serializedExpected, serializedActual);

}

void DeviceModuleTests::testHandleRequestInvalidProtobuf() {
    std::string invalidData = "qjweiqohbaosbnfas";
    zmf::data::ZmfOutReply reply = module_->UTAccessor_handleRequest(
            zmf::data::ZmfMessage(zsdn::modules::DeviceModuleTopics<zmf::data::MessageType>().request().device_module().get_all_devices().build(),
                                  invalidData), demoModuleUniqueId_);

    CPPUNIT_ASSERT_EQUAL(zmf::data::ZmfOutReply::NO_REPLY, reply.type);
}

void DeviceModuleTests::testHandleRequestUnknownReqType() {
    DeviceModule_Proto::Request request;

    zmf::data::ZmfOutReply reply = module_->UTAccessor_handleRequest(
            zmf::data::ZmfMessage(topicsSwitchLinkEvent_, request.SerializeAsString()),
            demoModuleUniqueId_);
    CPPUNIT_ASSERT_EQUAL(zmf::data::ZmfOutReply::NO_REPLY, reply.type);
}

void DeviceModuleTests::testIsDevicePassingFurtherFilters() {
    // Add 2 IPv4 Devices
    module_->UTAccessor_addDevice(demoDevices_[0]);
    module_->UTAccessor_addDevice(demoDevices_[1]);

    // Add 1 IPv6 Device
    module_->UTAccessor_addDevice(demoDevices_[4]);

    // Test to get device filtered by IPv4 Address
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestIPv4;
    filterRequestIPv4.set_ipv4_address_filter(demoDevices_[0].ipv4Addresses_[0]);

    bool isPassingIPv4FilterRequest = module_->UTAccessor_isDevicePassingFurtherFilters(filterRequestIPv4,
                                                                                        demoDevices_[0]);
    CPPUNIT_ASSERT_EQUAL(true, isPassingIPv4FilterRequest);

    // Test to get device filtered by IPv6 Address
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestIPv6;
    filterRequestIPv6.set_ipv6_address_filter("2001:0db8:85a3:08d3:1319:8a2e:0370:7344");

    bool isPassingIPv6FilterRequest = module_->UTAccessor_isDevicePassingFurtherFilters(filterRequestIPv6,
                                                                                        demoDevices_[4]);
    CPPUNIT_ASSERT_EQUAL(true, isPassingIPv6FilterRequest);

    // Test to get device filtered by Switch ID
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestSwitchId;
    filterRequestSwitchId.set_switch_dpid_filter(demoDevices_[0].attachmentPoint_.switchDpid);

    bool isPassingSwitchIdFilterRequest1 = module_->UTAccessor_isDevicePassingFurtherFilters(
            filterRequestSwitchId, demoDevices_[0]);
    bool isPassingSwitchIdFilterRequest2 = module_->UTAccessor_isDevicePassingFurtherFilters(
            filterRequestSwitchId, demoDevices_[1]);

    CPPUNIT_ASSERT_EQUAL(true, isPassingSwitchIdFilterRequest1);
    CPPUNIT_ASSERT_EQUAL(true, isPassingSwitchIdFilterRequest2);

    // Test to get device filtered by AttachmentPoint
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestAttachmentPoint;
    common::topology::AttachmentPoint* attP = new common::topology::AttachmentPoint();
    attP->set_switch_dpid(demoDevices_[0].attachmentPoint_.switchDpid);
    attP->set_switch_port(demoDevices_[0].attachmentPoint_.switchPort);
    filterRequestAttachmentPoint.set_allocated_attachment_point_filter(attP);

    bool isPassingAttachmentPointFilterRequest = module_->UTAccessor_isDevicePassingFurtherFilters(
            filterRequestAttachmentPoint, demoDevices_[0]);
    CPPUNIT_ASSERT_EQUAL(true, isPassingAttachmentPointFilterRequest);


    // Test to get device filtered by maxmillissincelastseen
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestMaxMillis1;
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestMaxMillis2;

    filterRequestMaxMillis1.set_max_millis_since_last_seen_filter(100000);
    filterRequestMaxMillis2.set_max_millis_since_last_seen_filter(0);
    // New incoming ARP packet of same Device to force an update --> setting new timestamp
    module_->UTAccessor_handlePacketIn(
            *build_DemoARP_ZmfMessage(demoDevices_[0]),
            demoModuleUniqueId_);

    bool isPassingMaxMillisFilterRequest1 = module_->UTAccessor_isDevicePassingFurtherFilters(
            filterRequestMaxMillis1, demoDevices_[0]);
    bool isPassingMaxMillisFilterRequest2 = module_->UTAccessor_isDevicePassingFurtherFilters(
            filterRequestMaxMillis2, demoDevices_[0]);
    CPPUNIT_ASSERT_EQUAL(true, isPassingMaxMillisFilterRequest1);
    CPPUNIT_ASSERT_EQUAL(false, isPassingMaxMillisFilterRequest2);


    // Test to get device with random multiple filters (e.g. ipv4 and swtich id + port)
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestMultiple;
    common::topology::AttachmentPoint* attP2 = new common::topology::AttachmentPoint();
    attP2->set_switch_dpid(demoDevices_[1].attachmentPoint_.switchDpid);
    attP2->set_switch_port(demoDevices_[1].attachmentPoint_.switchPort);
    filterRequestMultiple.set_ipv4_address_filter(demoDevices_[1].ipv4Addresses_[0]);
    filterRequestMultiple.set_allocated_attachment_point_filter(attP2);

    bool isPassingMultipleFilterRequest1 = module_->UTAccessor_isDevicePassingFurtherFilters(
            filterRequestMultiple, demoDevices_[1]);
    bool isPassingMultipleFilterRequest2 = module_->UTAccessor_isDevicePassingFurtherFilters(
            filterRequestMultiple, demoDevices_[0]);

    CPPUNIT_ASSERT_EQUAL(true, isPassingMultipleFilterRequest1);
    CPPUNIT_ASSERT_EQUAL(false, isPassingMultipleFilterRequest2);
}

void DeviceModuleTests::testIsDevicePassingFurtherFiltersNegative() {
    // Add 1 IPv6 Device
    module_->UTAccessor_addDevice(demoDevices_[4]);

    // Test to get IPv6 device not contained in cache
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestIPv6;
    filterRequestIPv6.set_ipv6_address_filter("FE80:0000:0000:0000:0202:B3FF:FE1E:8329");

    bool isPassingIPv6FilterRequest = module_->UTAccessor_isDevicePassingFurtherFilters(filterRequestIPv6,
                                                                                        demoDevices_[4]);
    CPPUNIT_ASSERT_EQUAL(false, isPassingIPv6FilterRequest);

    // Test to get device with false AttachmentPoint
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestAttP;
    common::topology::AttachmentPoint* attpPtr = new common::topology::AttachmentPoint();
    attpPtr->set_switch_dpid(demoDevices_[1].attachmentPoint_.switchDpid);
    attpPtr->set_switch_port(demoDevices_[1].attachmentPoint_.switchPort);
    filterRequestAttP.set_allocated_attachment_point_filter(attpPtr);

    bool isPassingAttPRequest = module_->UTAccessor_isDevicePassingFurtherFilters(filterRequestAttP,
                                                                                  demoDevices_[4]);
    CPPUNIT_ASSERT_EQUAL(false, isPassingAttPRequest);

    // Test to get device with false SwitchID
    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequestSwitchId;
    // Switch instance id is always the same in those tests (demoSwitchInstanceId_ == 666)
    filterRequestSwitchId.set_switch_dpid_filter(123);

    bool isPassingSwitchIdRequest = module_->UTAccessor_isDevicePassingFurtherFilters(filterRequestSwitchId,
                                                                                      demoDevices_[4]);
    CPPUNIT_ASSERT_EQUAL(false, isPassingSwitchIdRequest);
}

void DeviceModuleTests::testSwitchToSwitchLinkAdded() {
    // Add SwitchToSwitchLinks between the Switches
    common::topology::SwitchToSwitchLink newStsLink;
    common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
    attPSource->set_switch_dpid(demoSwitches_[3].switch_dpid());
    attPSource->CopyFrom(demoSwitches_[3].switch_ports(0).attachment_point());
    newStsLink.set_allocated_source(attPSource);
    common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
    attPTarget->CopyFrom(demoSwitches_[0].switch_ports(0).attachment_point());
    newStsLink.set_allocated_target(attPTarget);
    demoSwitchLinks_.insert(demoSwitchLinks_.end(), newStsLink);

    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(newStsLink),
                                              demoModuleUniqueId_);


    int vectorSizeRegisteredCache = module_->UTAccessor_SwitchToSwitchLinkCache().size();
    CPPUNIT_ASSERT_EQUAL(11, vectorSizeRegisteredCache);

    // Check previously contained links
    for (int i = 0; i < demoSwitchLinks_.size(); i++) {
        std::string expected = demoSwitchLinks_[i].SerializeAsString();
        std::string actual = module_->UTAccessor_SwitchToSwitchLinkCache()[i].SerializeAsString();
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }
    // Check added link
    std::string expected = newStsLink.SerializeAsString();
    int indexOfNewLink = module_->UTAccessor_SwitchToSwitchLinkCache().size() - 1;
    std::string actual = module_->UTAccessor_SwitchToSwitchLinkCache()[indexOfNewLink].SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void DeviceModuleTests::testSwitchToSwitchLinkRemoved() {

    std::cout << "size = " << std::to_string(module_->UTAccessor_SwitchToSwitchLinkCache().size()) << &std::endl;
    CPPUNIT_ASSERT_EQUAL(10, (int) module_->UTAccessor_SwitchToSwitchLinkCache().size());

    // Remove first link
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventRemoved_ZmfMessage(demoSwitchLinks_[0]),
                                              demoModuleUniqueId_);

    std::cout << "size = " << std::to_string(module_->UTAccessor_SwitchToSwitchLinkCache().size()) << &std::endl;
    CPPUNIT_ASSERT_EQUAL(9, (int) module_->UTAccessor_SwitchToSwitchLinkCache().size());

    // Check serialized and start with index 1 (index 0 was deleted from cache)
    for (int i = 1; i < demoSwitchLinks_.size(); i++) {
        std::string expected = demoSwitchLinks_[i].SerializeAsString();
        std::string actual = module_->UTAccessor_SwitchToSwitchLinkCache()[i - 1].SerializeAsString();
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }

    // Remove last link
    module_->UTAccessor_handleSwitchLinkEvent(
            *build_DemoSwitchLinkEventRemoved_ZmfMessage(demoSwitchLinks_[demoSwitchLinks_.size() - 1]),
            demoModuleUniqueId_);

    // Check serialized and start with index 1 (index 0 was deleted from cache)
    // End with index size - 1
    for (int i = 1; i < demoSwitchLinks_.size() - 1; i++) {
        std::string expected = demoSwitchLinks_[i].SerializeAsString();
        std::string actual = module_->UTAccessor_SwitchToSwitchLinkCache()[i - 1].SerializeAsString();
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }

    // Try to remove last link again
    module_->UTAccessor_handleSwitchLinkEvent(
            *build_DemoSwitchLinkEventRemoved_ZmfMessage(demoSwitchLinks_[demoSwitchLinks_.size() - 1]),
            demoModuleUniqueId_);

    // Check serialized and start with index 1 (index 0 was deleted from cache)
    // End with index size - 1
    for (int i = 1; i < demoSwitchLinks_.size() - 1; i++) {
        std::string expected = demoSwitchLinks_[i].SerializeAsString();
        std::string actual = module_->UTAccessor_SwitchToSwitchLinkCache()[i - 1].SerializeAsString();
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }
}

void DeviceModuleTests::testSwitchLinkEventInvalidProtobuf() {
    std::string invalidData = "qjweiqohbaosbnfas";
    int sizeBefore = module_->UTAccessor_SwitchToSwitchLinkCache().size();
    module_->UTAccessor_handleSwitchLinkEvent(
            zmf::data::ZmfMessage(topicsSwitchLinkEvent_, invalidData), demoModuleUniqueId_);
    int sizeAfter = module_->UTAccessor_SwitchToSwitchLinkCache().size();

    CPPUNIT_ASSERT_EQUAL(sizeBefore, sizeAfter);
}

void DeviceModuleTests::testSwitchLinkEventUnknownEventType() {
    LinkDiscoveryModule_Proto::From unknownFromMsg;
    int sizeBefore = module_->UTAccessor_SwitchToSwitchLinkCache().size();
    module_->UTAccessor_handleSwitchLinkEvent(
            zmf::data::ZmfMessage(topicsSwitchLinkEvent_, unknownFromMsg.SerializeAsString()), demoModuleUniqueId_);
    int sizeAfter = module_->UTAccessor_SwitchToSwitchLinkCache().size();

    CPPUNIT_ASSERT_EQUAL(sizeBefore, sizeAfter);
}

void DeviceModuleTests::testRequestAllSTSLinks() {
    // Check if all links were added
    CPPUNIT_ASSERT_EQUAL(demoSwitchLinks_.size(), module_->UTAccessor_SwitchToSwitchLinkCache().size());

    // Check if all links were correctly added
    for (int i = 0; i < demoSwitchLinks_.size(); i++) {
        CPPUNIT_ASSERT_EQUAL(demoSwitchLinks_[i].SerializeAsString(),
                             module_->UTAccessor_SwitchToSwitchLinkCache()[i].SerializeAsString());
    }
}

void DeviceModuleTests::testValidateDevicesAllValid() {
    // Add some demo Devices
    // Add 1 IPv4 Devices
    module_->UTAccessor_addDevice(demoDevices_[0]);

    // Add 1 IPv6 Device
    module_->UTAccessor_addDevice(demoDevices_[4]);

    module_->UTAccessor_validateDevices();

    CPPUNIT_ASSERT_EQUAL(2, (int) module_->UTAccessor_getAllDevicesMap().size());
    CPPUNIT_ASSERT_EQUAL(10, (int) module_->UTAccessor_SwitchToSwitchLinkCache().size());
}

void DeviceModuleTests::testValidateDevicesInvalid() {
    // Add some demo Devices
    // Add 1 IPv4 Devices
    module_->UTAccessor_addDevice(demoDevices_[0]);

    // Add 1 IPv6 Device
    module_->UTAccessor_addDevice(demoDevices_[4]);

    // Create an invalid Device (Switch as Device IPv4)
    uint64_t switchID = 123123;
    uint32_t switchPort = 3333;
    common::topology::SwitchToSwitchLink stsLinkSwitch;
    common::topology::AttachmentPoint* attPSource = new common::topology::AttachmentPoint();
    attPSource->set_switch_dpid(456456);
    attPSource->set_switch_port(4444);
    common::topology::AttachmentPoint* attPTarget = new common::topology::AttachmentPoint();
    attPTarget->set_switch_dpid(switchID);
    attPTarget->set_switch_port(switchPort);
    stsLinkSwitch.set_allocated_source(attPSource);
    stsLinkSwitch.set_allocated_target(attPTarget);

    Device* switchAsDeviceIpv4 = new Device(zsdn::NetUtils::mac_address_string_to_uint64("cc:cc:cc:cc:cc:cc"),
                                            {demoDevices_[2].ipv4Addresses_[0]}, {},
                                            switchID, switchPort);
    // Now the Switch gets detected as Device because the link is not known by the DeviceModule yet
    module_->UTAccessor_addDevice(*switchAsDeviceIpv4);
    delete switchAsDeviceIpv4;
    CPPUNIT_ASSERT_EQUAL(3, (int) module_->UTAccessor_getAllDevicesMap().size());
    CPPUNIT_ASSERT_EQUAL(10, (int) module_->UTAccessor_SwitchToSwitchLinkCache().size());


    // Now add the STSLink with the information that the potential Device is a Switch
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(stsLinkSwitch),
                                              demoModuleUniqueId_);
    CPPUNIT_ASSERT_EQUAL(2, (int) module_->UTAccessor_getAllDevicesMap().size());
    CPPUNIT_ASSERT_EQUAL(11, (int) module_->UTAccessor_SwitchToSwitchLinkCache().size());
}