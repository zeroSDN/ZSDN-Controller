//
// Created by zsdn on 6/25/15.
// Modified by Jose Marin

#include <cppunit/config/SourcePrefix.h>
#include <NetUtils.h>
#include "ARPModuleTests.h"
#include <RequestUtils.h>
#include "../DeviceModule/Device.h"
#include <LociExtensions.h>
#include "dummyModules/DummyModule.hpp"
#include <zmf/ZmfInstance.hpp>
#include "UnittestConfigUtil.hpp"


extern "C" {
}

CPPUNIT_TEST_SUITE_REGISTRATION(ARPModuleTests);

void ARPModuleTests::setUp(void) {
    // Initialize Module

    bool trackModuleStates = true;     // States of other modules will be tracked
    bool moduleAutoEnable = true;       // Module will be enabled as soon as possible (dependencies satisfied)
    bool exitWhenEnableFail = false;    // No shutdown when module enabling failed

    // Create device Module Mock
    try {
        deviceModuleMock_ = std::shared_ptr<zmf::AbstractModule>(
                new DummyModule(0, 0, zsdn::MODULE_TYPE_ID_DeviceModule, "DeviceModule",
                                [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                   zmf::data::ModuleState lastState) { }, [this](const zmf::data::ZmfMessage& message,
                                                                                 const zmf::data::ModuleUniqueId& sender) {
                            DeviceModule_Proto::Request request;

                            bool parseSuccess = request.ParseFromString(message.getData());
                            if (!parseSuccess) {
                                std::string err = "Could not parse Protobuffer format.";
                                throw Poco::Exception(err, 1);
                            }

                            switch (request.RequestMsg_case()) {
                                case DeviceModule_Proto::Request::kGetAllDevicesRequest: {
                                    Tins::ARP::ipaddress_type ipAddString = "1.1.0.10";
                                    std::string macAddString("ca:ca:ca:ca:ca:ca");

                                    uint64_t macAddress = zsdn::NetUtils::mac_address_string_to_uint64(macAddString);
                                    uint32_t ipAddress = (uint32_t) ipAddString;

                                    DeviceModule_Proto::Reply reply;
                                    DeviceModule_Proto::Reply_GetAllDevicesReply* gAllDevReply = new DeviceModule_Proto::Reply_GetAllDevicesReply();
                                    reply.set_allocated_get_all_devices_reply(gAllDevReply);

                                    common::topology::Device* newProtoDevice = gAllDevReply->add_devices();

                                    newProtoDevice->set_mac_address(macAddress);
                                    newProtoDevice->add_ipv4_address(ipAddress);
                                    newProtoDevice->set_millis_since_last_seen(300);
                                    common::topology::AttachmentPoint* attP = new common::topology::AttachmentPoint();
                                    attP->set_switch_dpid(3);
                                    attP->set_switch_port(2);
                                    newProtoDevice->set_allocated_attachment_point(attP);

                                    zmf::data::ZmfMessage msg(
                                    zsdn::modules::DeviceModuleTopics<zmf::data::MessageType>().reply().device_module().get_all_devices().build(),
                                            reply.SerializeAsString());

                                    return zmf::data::ZmfOutReply::createImmediateReply(msg);

                                }
                                case DeviceModule_Proto::Request::kGetDevicesByFilterRequest: {

                                    Tins::ARP::ipaddress_type ipAddString = "1.1.0.2";
                                    Tins::ARP::ipaddress_type ipAddStringForTimeout = "10.10.0.20"; // To siumulate timeout
                                    std::string macAddString("bc:bc:bc:bc:bc:bc");

                                    uint64_t macAddress = zsdn::NetUtils::mac_address_string_to_uint64(macAddString);
                                    uint32_t ipAddress = (uint32_t) ipAddString;
                                    uint32_t ipAddressForTimeout = (uint32_t) ipAddStringForTimeout;

                                    DeviceModule_Proto::Reply reply;
                                    DeviceModule_Proto::Reply_GetDevicesByFilterReply* gDevByFilterReply = new DeviceModule_Proto::Reply_GetDevicesByFilterReply();
                                    reply.set_allocated_get_devices_by_filter_reply(gDevByFilterReply);
                                    // Extract the filter request
                                    DeviceModule_Proto::Request_GetDevicesByFilterRequest filterRequest = request.get_devices_by_filter_request();

                                    if (filterRequest.ipv4_address_filter() == ipAddressForTimeout) {
                                        // wait until timeout
                                        return zmf::data::ZmfOutReply::createNoReply();
                                    }

                                    if (filterRequest.ipv4_address_filter() == ipAddress) {
                                        common::topology::Device* newProtoDevice = gDevByFilterReply->add_device();

                                        newProtoDevice->set_mac_address(macAddress);
                                        newProtoDevice->add_ipv4_address(ipAddress);
                                        newProtoDevice->set_millis_since_last_seen(300);
                                        common::topology::AttachmentPoint* attP = new common::topology::AttachmentPoint();
                                        attP->set_switch_dpid(3);
                                        attP->set_switch_port(1);
                                        newProtoDevice->set_allocated_attachment_point(attP);

                                    }

                                    zmf::data::ZmfMessage msg(
                                            zsdn::modules::DeviceModuleTopics<zmf::data::MessageType>().reply().device_module().get_devices_by_filter().build(),
                                            reply.SerializeAsString());

                                    return zmf::data::ZmfOutReply::createImmediateReply(msg);
                                }
                                case DeviceModule_Proto::Request::kGetDeviceByMacAddressRequest:
                                case DeviceModule_Proto::Request::REQUESTMSG_NOT_SET:
                                default:

                                    return zmf::data::ZmfOutReply::createNoReply();
                            }
                        }));

        // Create and start ZMF instance with module
        deviceModuleZmfInstance_ = zmf::instance::ZmfInstance::startInstance(deviceModuleMock_,
                                                                             {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                                             UT_CONFIG_FILE);

        while (!deviceModuleMock_->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
    }
    // Create Switch Adapter Mock
    try {
        switchAdapterMock_ = std::shared_ptr<zmf::AbstractModule>(
                new DummyModule(0, 0, zsdn::MODULE_TYPE_ID_SwitchAdapter, "SwitchAdapter",
                                [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                   zmf::data::ModuleState lastState) { }, [this](const zmf::data::ZmfMessage& message,
                                                                                 const zmf::data::ModuleUniqueId& sender) {
                            return zmf::data::ZmfOutReply::createNoReply();
                        }));

        // Create and start ZMF instance with module
        switchAdapterZmfInstance_ = zmf::instance::ZmfInstance::startInstance(switchAdapterMock_,
                                                                              {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                                              UT_CONFIG_FILE);

        while (!switchAdapterMock_->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
    }

    // Create ARP module
    try {
        arpModPtr_ = std::shared_ptr<ARPModule>(new ARPModule(1));
        // Create and start ZMF instance with module
        arpModuleInstance_ = zmf::instance::ZmfInstance::startInstance(arpModPtr_,
                                                                       {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                                       UT_CONFIG_FILE);

        while (!arpModPtr_->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
    }
}

void ARPModuleTests::tearDown(void) {
    // Delete all Objects
    deviceModuleZmfInstance_->requestStopInstance();
    switchAdapterZmfInstance_->requestStopInstance();
    arpModuleInstance_->requestStopInstance();
    deviceModuleZmfInstance_->joinExecution();
    switchAdapterZmfInstance_->joinExecution();
    arpModuleInstance_->joinExecution();
}

void ARPModuleTests::testHandleUpdateDevice(void) {
    uint64_t macAddress = zsdn::NetUtils::mac_address_string_to_uint64("ab:ab:ab:ab:ab:ab");
    Tins::ARP::ipaddress_type ipv4AddressTins = "1.1.0.1";
    std::uint32_t ipv4Address = ipv4AddressTins;
    Tins::ARP::ipaddress_type newIpv4AddressTins = "1.1.0.10";
    std::uint32_t newIpv4Address = newIpv4AddressTins;
    uint64_t switchInstanceId = 10;
    uint32_t switchPort = 1;
    uint32_t newSwitchPort = 3;

    arpModPtr_->updateArpCacheCache(ipv4Address, macAddress, switchInstanceId, switchPort);
    unsigned long sizeCacheBeforeDeviceUpdate = arpModPtr_->arpCache_.size();

    arpModPtr_->handleUpdateDevice(
            *build_DemoDeviceUpdate_ZmfMessage(macAddress, ipv4Address, switchInstanceId, switchPort,
                                               newIpv4Address, newSwitchPort));

    std::map<std::uint64_t, ARPModule::Device>::iterator itArpCache = arpModPtr_->arpCache_.find(macAddress);
    unsigned long sizeCacheAfterDeviceUpdate = arpModPtr_->arpCache_.size();

    CPPUNIT_ASSERT_EQUAL(sizeCacheBeforeDeviceUpdate, sizeCacheAfterDeviceUpdate);
    CPPUNIT_ASSERT(itArpCache != arpModPtr_->arpCache_.end());
    CPPUNIT_ASSERT_EQUAL(itArpCache->first, macAddress);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.ipAddress, newIpv4Address);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.switchDpid, switchInstanceId);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.switchPort, newSwitchPort);
}

void ARPModuleTests::testHandleAddDevice(void) {
    uint64_t macAddress = zsdn::NetUtils::mac_address_string_to_uint64("ab:ab:ab:ab:ab:ab");
    Tins::ARP::ipaddress_type ipv4AddressTins = "1.1.0.1";
    std::uint32_t ipv4Address = ipv4AddressTins;
    uint64_t switchInstanceId = 10;
    uint32_t switchPort = 1;

    unsigned long sizeCacheBeforeAddDevice = arpModPtr_->arpCache_.size();

    arpModPtr_->handleUpdateDevice(
            *build_DemoDeviceAdd_ZmfMessage(macAddress, ipv4Address, switchInstanceId, switchPort));

    std::map<std::uint64_t, ARPModule::Device>::iterator itArpCache = arpModPtr_->arpCache_.find(macAddress);
    unsigned long sizeCacheAfterAddDevice = arpModPtr_->arpCache_.size();

    CPPUNIT_ASSERT_EQUAL(sizeCacheBeforeAddDevice + 1, sizeCacheAfterAddDevice);
    CPPUNIT_ASSERT(itArpCache != arpModPtr_->arpCache_.end());
    CPPUNIT_ASSERT_EQUAL(itArpCache->first, macAddress);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.ipAddress, ipv4Address);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.switchDpid, switchInstanceId);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.switchPort, switchPort);
}

void ARPModuleTests::testHandleDeleteDevice(void) {

    // add first Device
    Tins::ARP::ipaddress_type ipAddFirst = "1.1.0.1";
    uint32_t switchPort = 1;
    uint64_t switchDpid = 5;
    std::uint32_t ipAddressFirst = ipAddFirst;
    std::uint64_t macAddressFirst = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    arpModPtr_->updateArpCacheCache(ipAddressFirst, macAddressFirst, switchDpid, switchPort);

    // add second device
    Tins::ARP::ipaddress_type ipAddFirstSecond = "1.1.0.1";
    uint32_t switchPortSecond = 1;
    std::uint32_t ipAddressSecond = ipAddFirstSecond;
    std::uint64_t macAddressSecond = zsdn::NetUtils::mac_address_string_to_uint64("da:da:da:da:da:da");
    arpModPtr_->updateArpCacheCache(ipAddressSecond, macAddressSecond, switchDpid, switchPortSecond);


    unsigned long sizeCacheBeforeDeleteDevice = arpModPtr_->arpCache_.size();

    arpModPtr_->handleUpdateDevice(*build_DemoDeviceDelete_ZmfMessage(macAddressFirst));

    unsigned long sizeCacheAfterDeleteDevice = arpModPtr_->arpCache_.size();

    std::map<std::uint64_t, ARPModule::Device>::iterator itArpCache = arpModPtr_->arpCache_.find(macAddressFirst);

    CPPUNIT_ASSERT_EQUAL(sizeCacheAfterDeleteDevice + 1, sizeCacheBeforeDeleteDevice);
    CPPUNIT_ASSERT(itArpCache == arpModPtr_->arpCache_.end());
}

void ARPModuleTests::testHandleDeleteInexistentDevice(void) {

    // add first Device
    Tins::ARP::ipaddress_type ipAddFirst = "1.1.0.1";
    uint32_t switchPort = 1;
    uint64_t switchDpid = 5;
    std::uint32_t ipAddressFirst = ipAddFirst;
    std::uint64_t macAddressFirst = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    arpModPtr_->updateArpCacheCache(ipAddressFirst, macAddressFirst, switchDpid, switchPort);

    // add second device
    Tins::ARP::ipaddress_type ipAddFirstSecond = "1.1.0.1";
    uint32_t switchPortSecond = 1;
    std::uint32_t ipAddressSecond = ipAddFirstSecond;
    std::uint64_t macAddressSecond = zsdn::NetUtils::mac_address_string_to_uint64("da:da:da:da:da:da");
    arpModPtr_->updateArpCacheCache(ipAddressSecond, macAddressSecond, switchDpid, switchPortSecond);

    std::uint64_t macAddressInexistentDevice = zsdn::NetUtils::mac_address_string_to_uint64("aa:aa:aa:aa:aa:aa");

    unsigned long sizeCacheBeforeDeleteDevice = arpModPtr_->arpCache_.size();

    // Try to delete an inexistent device
    arpModPtr_->handleUpdateDevice(*build_DemoDeviceDelete_ZmfMessage(macAddressInexistentDevice));

    unsigned long sizeCacheAfterDeleteDevice = arpModPtr_->arpCache_.size();

    std::map<std::uint64_t, ARPModule::Device>::iterator itArpCache = arpModPtr_->arpCache_.find(macAddressFirst);

    CPPUNIT_ASSERT_EQUAL(sizeCacheAfterDeleteDevice, sizeCacheBeforeDeleteDevice);
}

void ARPModuleTests::testUpdateArpCacheCacheNewDevice(void) {

    unsigned long sizeCacheBefore = arpModPtr_->arpCache_.size();
    Tins::ARP::ipaddress_type ipAddTargetPackIn = "1.1.0.1";
    uint32_t switchPort = 1;
    uint64_t switchDpid = 5;
    std::uint32_t ipAddressDevice = ipAddTargetPackIn;
    std::uint64_t macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpid, switchPort);

    std::map<std::uint64_t, ARPModule::Device>::iterator itArpCache = arpModPtr_->arpCache_.find(macAddressDevice);
    unsigned long sizeCacheAfter = arpModPtr_->arpCache_.size();

    CPPUNIT_ASSERT_EQUAL(sizeCacheBefore + 1, sizeCacheAfter);
    CPPUNIT_ASSERT(itArpCache != arpModPtr_->arpCache_.end());
    CPPUNIT_ASSERT_EQUAL(itArpCache->first, macAddressDevice);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.ipAddress, ipAddressDevice);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.switchDpid, switchDpid);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.switchPort, switchPort);
}

void ARPModuleTests::testUpdateArpCacheCacheUpdateDevice(void) {

    Tins::ARP::ipaddress_type ipAddTargetPackIn = "1.1.0.1";
    uint32_t switchPort = 1;
    uint64_t switchDpid = 5;
    std::uint32_t ipAddressDevice = ipAddTargetPackIn;
    std::uint64_t macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpid, switchPort);


    unsigned long sizeCacheBefore = arpModPtr_->arpCache_.size();
    ipAddTargetPackIn = "2.2.0.2";
    switchPort = 2;
    switchDpid = 10;
    ipAddressDevice = ipAddTargetPackIn;
    macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpid, switchPort);

    std::map<std::uint64_t, ARPModule::Device>::iterator itArpCache = arpModPtr_->arpCache_.find(macAddressDevice);
    unsigned long sizeCacheAfter = arpModPtr_->arpCache_.size();

    CPPUNIT_ASSERT_EQUAL(sizeCacheBefore, sizeCacheAfter);
    CPPUNIT_ASSERT(itArpCache != arpModPtr_->arpCache_.end());
    CPPUNIT_ASSERT_EQUAL(itArpCache->first, macAddressDevice);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.ipAddress, ipAddressDevice);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.switchDpid, switchDpid);
    CPPUNIT_ASSERT_EQUAL(itArpCache->second.switchPort, switchPort);
}

void ARPModuleTests::testARPReplyFromKnownDeviceToKnownDevice(void) {

    Tins::ARP::ipaddress_type ipAddSender = "1.1.0.1";
    uint32_t switchPortSender = 1;
    uint64_t switchDpidSender = 5;
    std::uint32_t ipAddressDevice = ipAddSender;
    const Tins::ARP::hwaddress_type macAddressSender("ba:ba:ba:ba:ba:ba");
    std::uint64_t macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    zmf::data::ModuleUniqueId moduleIdSender;
    moduleIdSender.TypeId = 20;
    moduleIdSender.InstanceId = switchDpidSender;

    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpidSender, switchPortSender);

    Tins::ARP::ipaddress_type ipAddTarget = "2.2.0.2";
    uint32_t switchPortTarget = 2;
    uint64_t switchDpidTarget = 10;
    ipAddressDevice = ipAddTarget;
    macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ab:ab:ab:ab:ab:ab");
    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpidTarget, switchPortTarget);


    arpModPtr_->handleIncomingArp(*build_DemoReqARP_ZmfMessage(macAddressSender, ipAddTarget,
                                                               ipAddSender, switchPortSender), moduleIdSender);
}

void ARPModuleTests::testARPReplyFromUnknownDeviceToKnownDevice(void) {

    Tins::ARP::ipaddress_type ipAddSender = "1.1.0.1";
    uint32_t switchPortSender = 1;
    uint64_t switchDpidSender = 5;
    std::uint32_t ipAddressDevice = ipAddSender;
    const Tins::ARP::hwaddress_type macAddressSender("ba:ba:ba:ba:ba:ba");
    std::uint64_t macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    zmf::data::ModuleUniqueId moduleIdSender;
    moduleIdSender.TypeId = 20;
    moduleIdSender.InstanceId = switchDpidSender;

    Tins::ARP::ipaddress_type ipAddTarget = "2.2.0.2";
    uint32_t switchPortTarget = 2;
    uint64_t switchDpidTarget = 10;
    ipAddressDevice = ipAddTarget;
    macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ab:ab:ab:ab:ab:ab");
    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpidTarget, switchPortTarget);


    arpModPtr_->handleIncomingArp(*build_DemoReqARP_ZmfMessage(macAddressSender, ipAddTarget,
                                                               ipAddSender, switchPortSender), moduleIdSender);
}

void ARPModuleTests::handleIncomingArpUnknownDevice(void) {
    const Tins::ARP::hwaddress_type macAddressSender("bb:bb:bb:bb:bb:bb");
    Tins::ARP::ipaddress_type ipAddTargetPackIn = "1.1.0.21";
    Tins::ARP::ipaddress_type ipAddSenderPackIn = "1.1.0.20";
    uint32_t switchPort = 1;
    zmf::data::ModuleUniqueId moduleId;
    moduleId.TypeId = 20;
    moduleId.InstanceId = 10;

    unsigned long sizeCacheBefore = arpModPtr_->arpCache_.size();

    arpModPtr_->handleIncomingArp(*build_DemoReqARP_ZmfMessage(macAddressSender, ipAddTargetPackIn,
                                                               ipAddSenderPackIn, switchPort), moduleId);

    unsigned long sizeCacheAfter = arpModPtr_->arpCache_.size();

    // No device added
    CPPUNIT_ASSERT_EQUAL(sizeCacheBefore, sizeCacheAfter);

}

void ARPModuleTests::handleIncomingArpKnownDeviceByDevModule(void) {
    const Tins::ARP::hwaddress_type macAddressSender("ab:ab:ab:ab:ab:ab");
    Tins::ARP::ipaddress_type ipAddTargetPackIn = "1.1.0.2";
    Tins::ARP::ipaddress_type ipAddSenderPackIn = "1.1.0.5";
    uint32_t switchPort = 1;
    zmf::data::ModuleUniqueId moduleId;
    moduleId.TypeId = 20;
    moduleId.InstanceId = 10;

    // Attributes of the device defined in kGetDevicesByFilterRequest --> "bc:bc:bc:bc:bc:bc"
    std::string macAddString("bc:bc:bc:bc:bc:bc");
    uint64_t macAddress = zsdn::NetUtils::mac_address_string_to_uint64(macAddString);
    uint32_t portTar = 1;
    uint64_t dpidTar = 3;
    uint32_t ipAddTar = (uint32_t) ipAddTargetPackIn;

    arpModPtr_->handleIncomingArp(*build_DemoReqARP_ZmfMessage(macAddressSender, ipAddTargetPackIn,
                                                               ipAddSenderPackIn, switchPort), moduleId);

    CPPUNIT_ASSERT(arpModPtr_->arpCache_.find(macAddress) != arpModPtr_->arpCache_.end());
    CPPUNIT_ASSERT_EQUAL(ipAddTar, arpModPtr_->arpCache_.at(macAddress).ipAddress);
    CPPUNIT_ASSERT_EQUAL(portTar, arpModPtr_->arpCache_.at(macAddress).switchPort);
    CPPUNIT_ASSERT_EQUAL(dpidTar, arpModPtr_->arpCache_.at(macAddress).switchDpid);

}

void ARPModuleTests::testGetDeviceTimeout(void) {
    const Tins::ARP::hwaddress_type macAddressSender("ab:ab:ab:ab:ab:ab");
    Tins::ARP::ipaddress_type ipAddTargetPackIn = "10.10.0.20";
    Tins::ARP::ipaddress_type ipAddSenderPackIn = "10.10.0.5";
    uint32_t switchPort = 1;
    zmf::data::ModuleUniqueId moduleId;
    moduleId.TypeId = 20;
    moduleId.InstanceId = 10;

    // Attributes of the device defined in kGetDevicesByFilterRequest --> "bc:bc:bc:bc:bc:bc"
    std::string macAddString("bc:bc:bc:bc:bc:bc");
    uint64_t macAddress = zsdn::NetUtils::mac_address_string_to_uint64(macAddString);
    uint32_t portTar = 1;
    uint64_t dpidTar = 3;
    uint32_t ipAddTar = (uint32_t) ipAddTargetPackIn;

    arpModPtr_->handleIncomingArp(*build_DemoReqARP_ZmfMessage(macAddressSender, ipAddTargetPackIn,
                                                               ipAddSenderPackIn, switchPort), moduleId);

}

void ARPModuleTests::testForwardARPReplay(void) {

    Tins::ARP::ipaddress_type ipAddSender = "1.1.0.1";
    uint32_t switchPortSender = 1;
    uint64_t switchDpidSender = 5;
    std::uint32_t ipAddressDevice = ipAddSender;
    const Tins::ARP::hwaddress_type macAddressSender("ba:ba:ba:ba:ba:ba");
    std::uint64_t macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    zmf::data::ModuleUniqueId moduleIdSender;
    moduleIdSender.TypeId = 20;
    moduleIdSender.InstanceId = switchDpidSender;

    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpidSender, switchPortSender);

    Tins::ARP::ipaddress_type ipAddTarget = "2.2.0.2";
    uint32_t switchPortTarget = 2;
    uint64_t switchDpidTarget = 10;
    ipAddressDevice = ipAddTarget;
    const Tins::ARP::hwaddress_type macAddressTarget("ab:ab:ab:ab:ab:ab");
    macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ab:ab:ab:ab:ab:ab");
    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpidTarget, switchPortTarget);


    arpModPtr_->handleIncomingArp(*build_DemoReplyARP_ZmfMessage(macAddressTarget, macAddressSender, ipAddTarget,
                                                                 ipAddSender, switchPortSender), moduleIdSender);
}


std::shared_ptr<zmf::data::ZmfMessage> ARPModuleTests::build_DemoReqARP_ZmfMessage(
        Tins::ARP::hwaddress_type macAddressSender,
        Tins::ARP::ipaddress_type ipv4AddressesTarget,
        Tins::ARP::ipaddress_type ipv4AddressesSender,
        uint32_t switchPortSender) {

    // Build EthernetII containing ARP packet with given IP- and MAC-Address
    Tins::EthernetII arpPacket = Tins::ARP::make_arp_request(ipv4AddressesTarget, ipv4AddressesSender,
                                                             macAddressSender);


    std::vector<uint8_t> serializedEth = arpPacket.serialize();


    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_in_t* pIn = of_packet_in_new(OF_VERSION_1_0);

    of_packet_in_in_port_set(pIn, switchPortSender);
    int i = of_packet_in_data_set(pIn, &of_oc);

    std::string data = zsdn::of_object_serialize_to_data_string(pIn);
    of_packet_in_delete(pIn);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(
                    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().from().switch_adapter().openflow().packet_in().multicast_group_default().arp().build(),
                    data));
}

std::shared_ptr<zmf::data::ZmfMessage> ARPModuleTests::build_DemoReqARP_ZmfMessageOF1_3(
        Tins::ARP::hwaddress_type macAddressSender,
        Tins::ARP::ipaddress_type ipv4AddressesTarget,
        Tins::ARP::ipaddress_type ipv4AddressesSender,
        uint32_t switchPortSender) {

    // Build EthernetII containing ARP packet with given IP- and MAC-Address
    Tins::EthernetII arpPacket = Tins::ARP::make_arp_request(ipv4AddressesTarget, ipv4AddressesSender,
                                                             macAddressSender);


    std::vector<uint8_t> serializedEth = arpPacket.serialize();


    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_in_t* pIn = of_packet_in_new(OF_VERSION_1_3);

    of_packet_in_buffer_id_set(pIn, OF_BUFFER_ID_NO_BUFFER);
    of_match_v3_t* matchv3 = of_match_v3_new(of_version_e::OF_VERSION_1_3);
    of_list_oxm_t* oxm = of_list_oxm_new(of_version_e::OF_VERSION_1_3);
    of_oxm_in_port_t* oxmPort = of_oxm_in_port_new(of_version_e::OF_VERSION_1_3);
    of_oxm_in_port_value_set(oxmPort, switchPortSender);
    of_list_oxm_append(oxm, oxmPort);
    int x = of_match_v3_oxm_list_set(matchv3, oxm);
    of_match_t match;
    of_match_v3_to_match(matchv3, &match);
    int xx = of_packet_in_match_set(pIn, &match);
    of_match_v3_delete(matchv3);
    of_oxm_in_port_delete(oxmPort);
    of_list_oxm_delete(oxm);

    int i = of_packet_in_data_set(pIn, &of_oc);

    std::string data = zsdn::of_object_serialize_to_data_string(pIn);
    of_packet_in_delete(pIn);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(
                    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().from().switch_adapter().openflow().packet_in().multicast_group_default().arp().build(),
                    data));
}


std::shared_ptr<zmf::data::ZmfMessage> ARPModuleTests::build_DemoReplyARP_ZmfMessage(
        Tins::ARP::hwaddress_type macAddressTarget,
        Tins::ARP::hwaddress_type macAddressSender,
        Tins::ARP::ipaddress_type ipv4AddressesTarget,
        Tins::ARP::ipaddress_type ipv4AddressesSender,
        uint32_t switchPortSender) {

    // Build EthernetII containing ARP packet with given IP- and MAC-Address
    Tins::EthernetII arpPacket = Tins::ARP::make_arp_reply(ipv4AddressesTarget, ipv4AddressesSender, macAddressTarget,
                                                           macAddressSender);

    std::vector<uint8_t> serializedEth = arpPacket.serialize();

    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_out_t* arpPacketOut = of_packet_in_new(OF_VERSION_1_0);

    of_packet_in_in_port_set(arpPacketOut, switchPortSender);
    int i = of_packet_in_data_set(arpPacketOut, &of_oc);

    std::string data = zsdn::of_object_serialize_to_data_string(arpPacketOut);
    of_packet_out_delete(arpPacketOut);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(
                    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().from().switch_adapter().openflow().packet_in().multicast_group_default().arp().build(),
                    data));

}


std::shared_ptr<zmf::data::ZmfMessage> ARPModuleTests::build_DemoDeviceUpdate_ZmfMessage(uint64_t macAddress,
                                                                                         uint32_t ipv4Address,
                                                                                         uint64_t switchInstanceId,
                                                                                         uint32_t switchPort,
                                                                                         uint32_t newIpv4Address,
                                                                                         uint32_t newSwitchPort) {
    // Before Update
    common::topology::Device* protoDeviceBeforeUpdate = new common::topology::Device();
    protoDeviceBeforeUpdate->set_mac_address(macAddress);
    protoDeviceBeforeUpdate->add_ipv4_address(ipv4Address);
    protoDeviceBeforeUpdate->set_millis_since_last_seen(300);
    common::topology::AttachmentPoint* beforeUpdateAttP = new common::topology::AttachmentPoint();
    beforeUpdateAttP->set_switch_dpid(switchInstanceId);
    beforeUpdateAttP->set_switch_port(switchPort);
    protoDeviceBeforeUpdate->set_allocated_attachment_point(beforeUpdateAttP);

    //After Update
    common::topology::Device* protoDeviceAfterUpdate = new common::topology::Device();
    protoDeviceAfterUpdate->set_mac_address(macAddress);
    protoDeviceAfterUpdate->add_ipv4_address(newIpv4Address);
    protoDeviceAfterUpdate->set_millis_since_last_seen(50);
    common::topology::AttachmentPoint* AfterUpdateAttP = new common::topology::AttachmentPoint();
    AfterUpdateAttP->set_switch_dpid(switchInstanceId);
    AfterUpdateAttP->set_switch_port(newSwitchPort);
    protoDeviceAfterUpdate->set_allocated_attachment_point(AfterUpdateAttP);

    DeviceModule_Proto::From_DeviceEvent::DeviceChanged* msgDeviceEventChanged = new DeviceModule_Proto::From_DeviceEvent::DeviceChanged();
    msgDeviceEventChanged->set_allocated_device_before(protoDeviceBeforeUpdate);
    msgDeviceEventChanged->set_allocated_device_now(protoDeviceAfterUpdate);

    DeviceModule_Proto::From_DeviceEvent* deviceEvent = new DeviceModule_Proto::From_DeviceEvent();
    deviceEvent->set_allocated_device_changed(msgDeviceEventChanged);

    DeviceModule_Proto::From fromMsg;
    fromMsg.set_allocated_device_event(deviceEvent);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(zsdn::modules::DeviceModuleTopics<zmf::data::MessageType>().from().device_module().device_event().changed().build(),
                                      fromMsg.SerializeAsString()));
}

std::shared_ptr<zmf::data::ZmfMessage> ARPModuleTests::build_DemoDeviceAdd_ZmfMessage(uint64_t macAddress,
                                                                                      uint32_t ipv4Address,
                                                                                      uint64_t switchInstanceId,
                                                                                      uint32_t switchPort) {
    common::topology::Device* newProtoDevice = new common::topology::Device();
    newProtoDevice->set_mac_address(macAddress);
    newProtoDevice->add_ipv4_address(ipv4Address);
    newProtoDevice->set_millis_since_last_seen(300);
    common::topology::AttachmentPoint* attP = new common::topology::AttachmentPoint();
    attP->set_switch_dpid(switchInstanceId);
    attP->set_switch_port(switchPort);
    newProtoDevice->set_allocated_attachment_point(attP);

    DeviceModule_Proto::From_DeviceEvent* deviceEvent = new DeviceModule_Proto::From_DeviceEvent();
    deviceEvent->set_allocated_device_added(newProtoDevice);

    DeviceModule_Proto::From fromMsg;
    fromMsg.set_allocated_device_event(deviceEvent);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(zsdn::modules::DeviceModuleTopics<zmf::data::MessageType>().from().device_module().device_event().added().build(),
                                      fromMsg.SerializeAsString()));
}

std::shared_ptr<zmf::data::ZmfMessage> ARPModuleTests::build_DemoDeviceDelete_ZmfMessage(uint64_t macAddress) {

    // Dont care about the values, relevant is just the MAC address
    Tins::ARP::ipaddress_type ipv4Address = "1.1.0.1";
    uint32_t switchPort = 1;
    uint64_t switchDpid = 5;
    std::uint32_t ipAddressDevice = ipv4Address;

    common::topology::Device* deletedProtoDevice = new common::topology::Device();
    deletedProtoDevice->set_mac_address(macAddress);
    deletedProtoDevice->add_ipv4_address(ipAddressDevice);
    deletedProtoDevice->set_millis_since_last_seen(300);
    common::topology::AttachmentPoint* attP = new common::topology::AttachmentPoint();
    attP->set_switch_dpid(switchDpid);
    attP->set_switch_port(switchPort);
    deletedProtoDevice->set_allocated_attachment_point(attP);

    DeviceModule_Proto::From_DeviceEvent* deviceEvent = new DeviceModule_Proto::From_DeviceEvent();
    deviceEvent->set_allocated_device_removed(deletedProtoDevice);

    DeviceModule_Proto::From fromMsg;
    fromMsg.set_allocated_device_event(deviceEvent);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(zsdn::modules::DeviceModuleTopics<zmf::data::MessageType>().from().device_module().device_event().removed().build(),
                                      fromMsg.SerializeAsString()));
}

void ARPModuleTests::testGetPortPacketInCameFrom(void) {

    const Tins::ARP::hwaddress_type macAddressSender("ab:ab:ab:ab:ab:ab");
    Tins::ARP::ipaddress_type ipAddTargetPackIn = "10.10.0.1";
    Tins::ARP::ipaddress_type ipAddSenderPackIn = "10.10.0.5";
    uint32_t switchPort = 1;
    uint32_t switchPortFromARPModule;

    // Build EthernetII containing ARP packet with given IP- and MAC-Address
    Tins::EthernetII arpPacket = Tins::ARP::make_arp_request(ipAddTargetPackIn, ipAddSenderPackIn, macAddressSender);

    std::vector<uint8_t> serializedEth = arpPacket.serialize();
    of_octets_t of_oc;
    of_oc.data = serializedEth.data();
    of_oc.bytes = serializedEth.size();

    of_packet_in_t* pIn = of_packet_in_new(OF_VERSION_1_3);

    of_packet_in_buffer_id_set(pIn, OF_BUFFER_ID_NO_BUFFER);
    of_match_v3_t* matchv3 = of_match_v3_new(of_version_e::OF_VERSION_1_3);
    of_list_oxm_t* oxm = of_list_oxm_new(of_version_e::OF_VERSION_1_3);
    of_oxm_in_port_t* oxmPort = of_oxm_in_port_new(of_version_e::OF_VERSION_1_3);
    of_oxm_in_port_value_set(oxmPort, switchPort);
    of_list_oxm_append(oxm, oxmPort);
    int x = of_match_v3_oxm_list_set(matchv3, oxm);
    of_match_t match;
    of_match_v3_to_match(matchv3, &match);
    int xx = of_packet_in_match_set(pIn, &match);

    int i = of_packet_in_data_set(pIn, &of_oc);
    of_match_v3_delete(matchv3);
    of_oxm_in_port_delete(oxmPort);
    of_list_oxm_delete(oxm);
    switchPortFromARPModule = arpModPtr_->getPortPacketInCameFrom(pIn);

    of_packet_in_delete(pIn);
    CPPUNIT_ASSERT_EQUAL(switchPortFromARPModule, switchPort);
}

void ARPModuleTests::testARPReplyWithUnknownOpenFlowVersion(void) {

    Tins::ARP::ipaddress_type ipAddSender = "1.1.0.1";
    uint32_t switchPortSender = 1;
    uint64_t switchDpidSender = 5;
    std::uint32_t ipAddressDevice = ipAddSender;
    const Tins::ARP::hwaddress_type macAddressSender("ba:ba:ba:ba:ba:ba");
    std::uint64_t macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ba:ba:ba:ba:ba:ba");
    zmf::data::ModuleUniqueId moduleIdSender;
    moduleIdSender.TypeId = 20;
    moduleIdSender.InstanceId = switchDpidSender;

    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpidSender, switchPortSender);

    Tins::ARP::ipaddress_type ipAddTarget = "2.2.0.2";
    uint32_t switchPortTarget = 2;
    uint64_t switchDpidTarget = 10;
    ipAddressDevice = ipAddTarget;
    macAddressDevice = zsdn::NetUtils::mac_address_string_to_uint64("ab:ab:ab:ab:ab:ab");
    arpModPtr_->updateArpCacheCache(ipAddressDevice, macAddressDevice, switchDpidTarget, switchPortTarget);


    arpModPtr_->handleIncomingArp(*build_DemoReqARP_ZmfMessageOF1_3(macAddressSender, ipAddTarget,
                                                                    ipAddSender, switchPortSender), moduleIdSender);
}
