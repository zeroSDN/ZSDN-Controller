#ifndef ARPModule_UT_H
#define ARPModule_UT_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <zsdn/topics/SwitchAdapterTopics.hpp>
#include <zmf/AbstractModule.hpp>
#include <zmf/IZmfInstanceController.hpp>
#include "tins/tins.h"
#include "ARPModule.hpp"

/**
 * @details   Module for the unit test of the ARPModule
 * @author  Jose Marin
 * @date    23.07.2015
 *
 */

extern "C" {
#include <loci/loci.h>
}

using namespace CppUnit;

class ARPModuleTests : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(ARPModuleTests);
        // Test updates from device module
        CPPUNIT_TEST(testHandleUpdateDevice);
        CPPUNIT_TEST(testHandleAddDevice);
        CPPUNIT_TEST(testHandleDeleteDevice);
        CPPUNIT_TEST(testHandleDeleteInexistentDevice);

        // Test static methods
        CPPUNIT_TEST(testUpdateArpCacheCacheNewDevice);
        CPPUNIT_TEST(testUpdateArpCacheCacheUpdateDevice);
        CPPUNIT_TEST(testGetPortPacketInCameFrom);

        // Test ARPs handle
        CPPUNIT_TEST(handleIncomingArpKnownDeviceByDevModule);
        CPPUNIT_TEST(testARPReplyFromKnownDeviceToKnownDevice);
        CPPUNIT_TEST(testARPReplyFromUnknownDeviceToKnownDevice);
        CPPUNIT_TEST(handleIncomingArpUnknownDevice);
        CPPUNIT_TEST(testForwardARPReplay);
        CPPUNIT_TEST(testARPReplyWithUnknownOpenFlowVersion);
        CPPUNIT_TEST(testGetDeviceTimeout);
    CPPUNIT_TEST_SUITE_END();

public:
    /**
     * Override tearDown method to release any permanent ressources.
     */
    void tearDown(void);

    /*
     * Override setup method to set up the testing objects.
     */
    void setUp(void);

    /**
     * test the updateArpCache method, it tests if a new device is successfully added to the ARPCache
     */
    void testUpdateArpCacheCacheNewDevice(void);

    /**
     * test the updateArpCache method, it tests if an existing device is successfully updated in the ARPCache
     */
    void testUpdateArpCacheCacheUpdateDevice(void);

    /**
     * It tests, if the ARPModule creates and forward an ARP-Replay to a known device
     * according to the ARP-Request
     */
    void testForwardARPReplay(void);

    /**
     * It tests the private method to find out from which port of the switch the packet in came from
     */
    void testGetPortPacketInCameFrom(void);

    /**
     * Test if the response of an ARP-Request from a known to a known device was sent out successfully
     */
    void testARPReplyFromKnownDeviceToKnownDevice(void);

    /**
     * Test if the response of an ARP-Request from a unknown to a known device was sent out successfully
     */
    void testARPReplyFromUnknownDeviceToKnownDevice(void);

    /**
     * Test if the response of an ARP-Request to an unknown device was sent out successfully
     */
    void handleIncomingArpUnknownDevice(void);

    /**
     * Test if an exception is thrown by recpetion of an packet in with an unknown open flow version
     */
    void testARPReplyWithUnknownOpenFlowVersion(void);

    /**
     * Test an ARP-Reply to a device that does not existing in the ARPcache of the module ARP, but that
     * deviceMoudle know
     */
    void handleIncomingArpKnownDeviceByDevModule(void);

    /**
     * Test if a timeout warning comes after a request an device module without response
     */
    void testGetDeviceTimeout(void);

    //Test topics from device Module: Add/Update/Delete device

    /**
     * Test the private method update device. Update an existing device
     */
    void testHandleUpdateDevice(void);

    /**
     * Test the private method update device. Add a new device
     */
    void testHandleAddDevice(void);

    /**
     * Test the private method update device. Delete an existing device
     */
    void testHandleDeleteDevice(void);

    /**
     * Test the private method update device. Delete a device, that deoes not appear in the ARPCache
     */
    void testHandleDeleteInexistentDevice(void);

private:


    std::shared_ptr<zmf::IZmfInstanceController> arpModuleInstance_;
    std::shared_ptr<ARPModule> arpModPtr_;

    std::shared_ptr<zmf::IZmfInstanceController> deviceModuleZmfInstance_;
    std::shared_ptr<zmf::AbstractModule> deviceModuleMock_;

    std::shared_ptr<zmf::IZmfInstanceController> switchAdapterZmfInstance_;
    std::shared_ptr<zmf::AbstractModule> switchAdapterMock_;

    /**
     * Builds a demo Request ARP packet for testing.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoReqARP_ZmfMessage(Tins::ARP::hwaddress_type macAddressSender,
                                                                       Tins::ARP::ipaddress_type ipv4AddressesTarget,
                                                                       Tins::ARP::ipaddress_type ipv4AddressesSender,
                                                                       uint32_t switchPortSender);


    /**
     * Builds a demo Request ARP packet for testing.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoReqARP_ZmfMessageOF1_3(Tins::ARP::hwaddress_type macAddressSender,
                                                                            Tins::ARP::ipaddress_type ipv4AddressesTarget,
                                                                            Tins::ARP::ipaddress_type ipv4AddressesSender,
                                                                            uint32_t switchPortSender);


    /**
     * Builds a demo Reply ARP packet for testing.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoReplyARP_ZmfMessage(Tins::ARP::hwaddress_type macAddressTarget,
                                                                         Tins::ARP::hwaddress_type macAddressSender,
                                                                         Tins::ARP::ipaddress_type ipv4AddressesTarget,
                                                                         Tins::ARP::ipaddress_type ipv4AddressesSender,
                                                                         uint32_t switchPortSender);

    /**
     * Builds a demo Update device for testing.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoDeviceUpdate_ZmfMessage(uint64_t macAddress,
                                                                             uint32_t ipv4Addresses,
                                                                             uint64_t switchInstanceId,
                                                                             uint32_t switchPort,
                                                                             uint32_t newIpv4Addresses,
                                                                             uint32_t newSwitchPort);

    /**
     * Builds a demo Add Device for testing.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoDeviceAdd_ZmfMessage(uint64_t macAddress,
                                                                          uint32_t ipv4Address,
                                                                          uint64_t switchInstanceId,
                                                                          uint32_t switchPort);

    /**
     * Builds a demo delete Device for testing.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoDeviceDelete_ZmfMessage(uint64_t macAddress);
};

#endif //ARPModule_UT_H
