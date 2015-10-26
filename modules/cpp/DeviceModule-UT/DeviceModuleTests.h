#ifndef DeviceModule_UT_H
#define DeviceModule_UT_H

#include <cppunit/extensions/HelperMacros.h>
#include <zsdn/topics/SwitchAdapter_topics.hpp>
#include <zmf/AbstractModule.hpp>
#include "Device.h"
#include "DeviceModule.hpp"
#include <zmf/IZmfInstanceController.hpp>
#include "tins/tins.h"

using namespace CppUnit;

/**
 * @details Functional Tests (Unit-Tests) for the DeviceModule implementation.
 * @author Tobias Freundorfer
 */
class DeviceModuleTests : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(DeviceModuleTests);
        // method addDevice()
        CPPUNIT_TEST(testAddDevice);

        // method getDeviceByMacAddress()
        CPPUNIT_TEST(testGetDeviceByMacAddressRegistered);
        CPPUNIT_TEST(testGetDeviceByMacAddressNotRegistered);

        // method handlePacketIn() with special cases.
        CPPUNIT_TEST(testHandlePacketInEthernetIINoInnerPDU);
        CPPUNIT_TEST(testHandlePacketInIPv4BroadcastAddress);
        CPPUNIT_TEST(testHandlePacketInIPv6BroadcastAddress);
        CPPUNIT_TEST(testHandlePacketInSTSLinkPacket);
        CPPUNIT_TEST(testHandlePacketInUnknownPacket);

        // method handlePacketIn() with ARP
        CPPUNIT_TEST(testHandleARPNewDevice);
        CPPUNIT_TEST(testHandleARPExistingDeviceNoUpdate);
        CPPUNIT_TEST(testHandleARPExistingDeviceUpdate);
        CPPUNIT_TEST(testHandleARPForBroadcast);

        // method handlePacketIn() with IPv4
        CPPUNIT_TEST(testHandleIPv4NewDevice);
        CPPUNIT_TEST(testHandleIPv4ExistingDeviceNoUpdate);
        CPPUNIT_TEST(testHandleIPv4ExistingDeviceUpdate);

        // method handlePacketIn() with IPv6
        CPPUNIT_TEST(testHandleIPv6NewDevice);
        CPPUNIT_TEST(testHandleIPv6ExistingDeviceNoUpdate);
        CPPUNIT_TEST(testHandleIPv6ExistingDeviceUpdate);

        // method handleRequest()
        CPPUNIT_TEST(testHandleRequestGetAllDevices);
        CPPUNIT_TEST(testHandleRequestGetDeviceByMacAddress);
        CPPUNIT_TEST(testHandleRequestGetDeviceByFilter);
        CPPUNIT_TEST(testHandleRequestGetDeviceByFilterWithMacFilter);
        CPPUNIT_TEST(testHandleRequestInvalidProtobuf);
        CPPUNIT_TEST(testHandleRequestUnknownReqType);

        // method isDevicePassingFurtherFilters()
        CPPUNIT_TEST(testIsDevicePassingFurtherFilters);
        CPPUNIT_TEST(testIsDevicePassingFurtherFiltersNegative);

        // method handleSwitchLinkEvent()
        CPPUNIT_TEST(testSwitchToSwitchLinkAdded);
        CPPUNIT_TEST(testSwitchToSwitchLinkRemoved);
        CPPUNIT_TEST(testSwitchLinkEventInvalidProtobuf);
        CPPUNIT_TEST(testSwitchLinkEventUnknownEventType);

        // method requestAllSwitchToSwitchLinks()
        CPPUNIT_TEST(testRequestAllSTSLinks);
        // method validateDevices()
        CPPUNIT_TEST(testValidateDevicesAllValid);
        CPPUNIT_TEST(testValidateDevicesInvalid);
    CPPUNIT_TEST_SUITE_END();

public:

    /*
     * Override setup method to set up the testing objects.
     */
    void setUp();

    /*
     * Override tearDown method to release any permanent ressources.
     */
    void tearDown();

    /**
     * A test for the DeviceModule method addDevice().
     * Tests if a given device gets added correctly including all of its properties.
     */
    void testAddDevice();

    /**
     * A test for the DeviceModule method getDeviceByMacAddress().
     * Tests whether for a valid (resp. registered Device at DeviceModule) MAC-Address the correct Device is returned.
     */
    void testGetDeviceByMacAddressRegistered();

    /**
     * A test for the DeviceModule method getDeviceByMacAddress().
     * Tests whether for a valid (resp. not registered Device at DeviceModule) MAC-Address no Device is returned.
     */
    void testGetDeviceByMacAddressNotRegistered();

    /**
     * A test for the DeviceModule method handlePacketIn() with an invalid EthernetII packet containing no inner pdu.
     * Tests if this case sepcial case gets catched.
     */
    void testHandlePacketInEthernetIINoInnerPDU();

    /**
     * A test for DeviceModule method handlePacketIn().
     * Tests the handling of a packet in with a broadcast IPv4 Address (255.255.255.255).
     */
    void testHandlePacketInIPv4BroadcastAddress();

    /**
     * A test for DeviceModule method handlePacketIn().
     * Tests the handling of a packet in with a "broadcast" IPv6 Address (::).
     */
    void testHandlePacketInIPv6BroadcastAddress();

    /**
     * A test for DeviceModule method handlePacketIn().
     * Tests the handling of a packet that was sent from a Switch.
     */
    void testHandlePacketInSTSLinkPacket();

    /**
     * A test for the DeviceModule method handlePacketIn().
     * Tests the handling of a packet that is unknown.
     */
    void testHandlePacketInUnknownPacket();

    /**
     * A test for the DeviceModule method handlePacketIn() with ARP packet.
     * Tests the handling for a new Device that is not yet registered at the DeviceModule.
     */
    void testHandleARPNewDevice();

    /**
     * A test for the DeviceModule method handlePacketIn() with ARP packet.
     * Tests the handling for an existing Device that is already registered at the DeviceModule with
     * the same properties like the registered one.
     */
    void testHandleARPExistingDeviceNoUpdate();

    /**
     * A test for the DeviceModule method handlePacketIn() with ARP packet.
     * Tests the handling for an existing Device that is already registered at the DeviceModule but
     * with other properties like the registered one.
     */
    void testHandleARPExistingDeviceUpdate();

    /**
     * A test for the DeviceModule method handlePacketIn() with ARP packet.
     * Tests the handling for an ARP packet that is a broadcast.
     */
    void testHandleARPForBroadcast();

    /**
     * A test for the DeviceModule method handlePacketIn() with IPv4 packet.
     * Tests the handling for a new Device that is not yet registered at the DeviceModule.
     */
    void testHandleIPv4NewDevice();

    /**
     * A test for the DeviceModule method handlePacketIn() with IPv4 packet.
     * Tests the handling for an existing Device that is already registered at the DeviceModule with
     * the same properties like the registered one.
     */
    void testHandleIPv4ExistingDeviceNoUpdate();

    /**
     * A test for the DeviceModule method handlePacketIn() with IPv4 packet.
     * Tests the handling for an existing Device that is already registered at the DeviceModule but
     * with other properties like the registered one.
     */
    void testHandleIPv4ExistingDeviceUpdate();

    /**
     * A test for the DeviceModule method handlePacketIn() with IPv6 packet.
     * Tests the handling for a new Device that is not yet registered at the DeviceModule.
     */
    void testHandleIPv6NewDevice();

    /**
     * A test for the DeviceModule method handlePacketIn() with IPv6 packet.
     * Tests the handling for an existing Device that is already registered at the DeviceModule with
     * the same properties like the registered one.
     */
    void testHandleIPv6ExistingDeviceNoUpdate();

    /**
     * A test for the DeviceModule method handlePacketIn() with IPv6 packet.
     * Tests the handling for an existing Device that is already registered at the DeviceModule but
     * with other properties like the registered one.
     */
    void testHandleIPv6ExistingDeviceUpdate();

    /**
     * A test for the DeviceModule method handleRequest().
     * Tests the handling for requests of type kGetAllDevicesRequest.
     */
    void testHandleRequestGetAllDevices();

    /**
     * A test for the DeviceModule method handleRequest().
     * Tests the handling for requests of type kGetDeviceByMacAddressRequest.
     */
    void testHandleRequestGetDeviceByMacAddress();

    /**
     * A test for the DeviceModule method handleRequest().
     * Tests the handling for requests of type kGetDevicesByFilterRequest with multiple filtering.
     */
    void testHandleRequestGetDeviceByFilter();

    /**
     * A test for the DeviceModule method handleRequest().
     * Tests the handling for requests of type kGetDevicesByFilterRequest with MAC filtering.
     */
    void testHandleRequestGetDeviceByFilterWithMacFilter();

    /**
     * A test for the DeviceModule method handleRequest().
     * Tests the handling for requests with an invalid ProtoBuffer format.
     */
    void testHandleRequestInvalidProtobuf();

    /**
     * A test for the DeviceModule method handleRequest().
     * Tests the handling for request with an unknwon Request type.
     */
    void testHandleRequestUnknownReqType();

    /**
     * A test for the DeviceModule method isDevicePassingFurtherFilters().
     * Tests if for a given filter request the positive devices pass the filtering.
     */
    void testIsDevicePassingFurtherFilters();

    /**
     * A test for the DeviceModule method isDevicePassingFurtherFilters().
     * Tests if for a given filter request the negative devices fail the filtering.
     */
    void testIsDevicePassingFurtherFiltersNegative();

    /**
    * A Test for the SwitchLinkEvent Added from the LinkDiscoveryModule.
    * Tests the adding for a new SwitchToSwitchLink published by the mentioned event.
    */
    void testSwitchToSwitchLinkAdded();

    /**
     * A test for the DeviceModule method handleSwitchLinkEvent().
     * Tests the handling for link event with an invalid ProtoBuffer format.
     */
    void testSwitchLinkEventInvalidProtobuf();

    /**
     * A test for the DeviceModule method handleSwitchLinkEvent().
     * Tests the handling for link events with an unknwon Request type
     */
    void testSwitchLinkEventUnknownEventType();

    /**
    * A Test for the SwitchLinkEvent Removed from the LinkDiscoveryModule.
    * Tests the removing of a SwitchToSwitchLink published by the mentioned event.
    */
    void testSwitchToSwitchLinkRemoved();

    /**
     * A Test for the Request to the LinkDiscoveryModule for all Switch-to-Switch Links.
     * Tests if the request to a Mock is successfull and if the returned Links are added correctly.
     */
    void testRequestAllSTSLinks();

    /**
     * A Test for the method validateDevices() of DeviceModule.
     * Tests if valid devices are detected as valid.
     */
    void testValidateDevicesAllValid();

    /**
     * A Test for the method validateDevices() of DeviceModule.
     * Tests if an invalid devices gets detected as invalid and is deleted from devices map.
     */
    void testValidateDevicesInvalid();

private:
    /// The ZmfInstance that holds the Module under test.
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstance_;

    /// The DeviceModule under test.
    std::shared_ptr<DeviceModule> module_;

    /// The ZmfInstance that holds the LinkDiscoveryModule.
    std::shared_ptr<zmf::IZmfInstanceController> linkDiscoveryModuleZmfInstance_;

    /// The ZmfInstance that holds the LinkDiscoveryModuleMock.
    std::shared_ptr<zmf::AbstractModule> linkDiscoveryModuleMock_;

    /// Vector holding some demo Devices for testing.
    std::vector<Device> demoDevices_;

    /// Vector holding some demo Switches for testing.
    std::vector<common::topology::Switch> demoSwitches_;

    /// Vector holding the demo Ports for testing.
    std::vector<common::topology::SwitchPort> demoPorts_;

    /// Vector holding the demo SwitchToSwitchLinks for testing.
    std::vector<common::topology::SwitchToSwitchLink> demoSwitchLinks_;


    /// Demo Switch type id.
    uint16_t demoSwitchTypeId_ = 999;

    /// Demo Switch instance id. Equals to the switch_dpid of the AttachmentPoint.
    uint64_t demoSwitchInstanceId_ = 666;

    zmf::data::ModuleUniqueId demoModuleUniqueId_ = zmf::data::ModuleUniqueId(demoSwitchTypeId_, demoSwitchInstanceId_);

    /// Vector holding some demo Switch Ports for testing.
    std::vector<uint32_t> demoSwitchPorts_;

    /// Topic for ARP packet_in's
    zmf::data::MessageType topicsArp_ = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().multicast_group_default().arp().build();
    /// Topic for IPv4 packet_in's
    zmf::data::MessageType topicsIpv4_ = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().multicast_group_default().ipv4().build();
    /// Topic for IPv6 packet_in's
    zmf::data::MessageType topicsIpv6_ = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().multicast_group_default().ipv6().build();

    /// Topic for LinkDiscoveryModule SwitchLink_Event
    zmf::data::MessageType topicsSwitchLinkEvent_ = linkdiscoverymodule_topics::FROM().link_discovery_module().switch_link_event().build();

    /**
     * Builds a demo ARP packet for testing.
     * @param device The Device for which the ARP packet should be built.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoARP_ZmfMessage(Device device);

    /**
     * Builds a demo IPv4 packet for testing.
     * @param device The Device for which the IPv4 packet should be built.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoIPv4_ZmfMessage(Device device);

    /**
     * Builds a demo IPv6 packet for testing.
     * @param device The Device for which the IPv6 packet should be built.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoIPv6_ZmfMessage(Device device);

    /**
     * Builds a demo Request for testing with the given type.
     * @param requestType The type of request.
     * @param macAddress The MAC-Address to filter. If 0 no filter!
     * @param ipv4Address The IPv4-Address to filter. If 0 no filter!
     * @param switch_dpid The Switch id to filter. If 0 no filter!
     * @param switch_port The Switch port to filter. If 0 no filter!
     * @param max_millis_since_last_seen The time to filter. If 0 no filter!
     * @param ipv6Address The IPv6-Address to filter. If 0 no filter!
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoRequest_ZmfMessage(const int requestType, uint64_t macAddress,
                                                                        uint32_t ipv4Address,
                                                                        uint64_t switch_dpid,
                                                                        uint32_t switch_port,
                                                                        uint64_t max_millis_since_last_seen,
                                                                        std::array<uint8_t, 16> ipv6Address);

    /**
    * Builds a demo ZmfMessage for the SwitchLinkEvent Added from the LinkDiscoveryModule for a given Switch.
    * @param aSTSLink The SwitchToSwitchLink for which the demo ZmfMessage should be built.
    */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoSwitchLinkEventAdded_ZmfMessage(
            common::topology::SwitchToSwitchLink aSTSLink);

    /**
    * Builds a demo ZmfMessage for the SwitchLinkEvent Removed from the LinkDiscoveryModule for a given Switch.
    * @param aSTSLink The SwitchToSwitchLink for which the demo ZmfMessage should be built.
    */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoSwitchLinkEventRemoved_ZmfMessage(
            common::topology::SwitchToSwitchLink aSTSLink);

    /**
     * Compares if the properties of two given Devices match.
     * @device1 The first Device.
     * @device2 The second Device.
     * @return True if the properties of the two given Devices match, else FALSE.
     */
    bool hasDeviceEqualProperties(const Device& device1, const Device& device2);
};

#endif //DeviceModule_UT_H
