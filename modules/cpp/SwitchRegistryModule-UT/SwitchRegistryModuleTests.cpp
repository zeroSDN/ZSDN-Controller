//
// Created by zsdn on 6/25/15.
//

#include <NetUtils.h>
#include <LociExtensions.h>
#include <zmf/ZmfInstance.hpp>
#include "SwitchRegistryModuleTests.h"
#include "zsdn/proto/SwitchRegistryModule.pb.cc"
#include "dummyModules/DummyModule.hpp"
#include <UnittestConfigUtil.hpp>


SwitchRegistryModuleTests::SwitchRegistryModuleTests() {

}

void SwitchRegistryModuleTests::setUp() {

    //add Switch1 with 1 Port
    Switch aSwitch1 = Switch(1, OF_VERSION_1_3);
    aSwitch1.auxiliary_id = 1;
    aSwitch1.capabilities = 1;
    aSwitch1.reserved = 1;
    aSwitch1.n_tables = 1;
    aSwitch1.n_buffers = 1;
    aSwitch1.switch_info_available = true;
    aSwitch1.got_ports = true;
    aSwitch1.active = true;
    Port port = Port(1);
    port.advertised = 1;
    port.config = 1;
    port.curr = 1;
    port.mac_address = 1;
    port.port_name = "1";
    port.state = 1;
    port.supported = 1;
    port.peer = 1;
    port.curr_speed = 1;
    port.max_speed = 1;
    aSwitch1.ports.insert(aSwitch1.ports.end(), port);
    switchVector.insert(switchVector.end(), aSwitch1);

    //add Switch2 with 2 Ports
    Switch aSwitch2 = Switch(2, OF_VERSION_1_0);
    aSwitch2.auxiliary_id = 2;
    aSwitch2.capabilities = 2;
    aSwitch2.reserved = 2;
    aSwitch2.n_tables = 2;
    aSwitch2.n_buffers = 2;
    aSwitch2.switch_info_available = true;
    aSwitch2.got_ports = true;
    aSwitch2.active = true;
    port.switch_port = 2;
    aSwitch2.ports.insert(aSwitch2.ports.end(), port);
    Port port2 = Port(2);
    port2.advertised = 2;
    port2.config = 2;
    port2.curr = 2;
    port2.mac_address = 2;
    port2.port_name = "2";
    port2.state = 2;
    port2.supported = 2;
    port2.peer = 2;
    port2.curr_speed = 2;
    port2.max_speed = 2;
    aSwitch2.ports.insert(aSwitch2.ports.end(), port2);
    switchVector.insert(switchVector.end(), aSwitch2);


    try {
        // Create module
        module_ = std::shared_ptr<SwitchRegistryModule>(
                new SwitchRegistryModule(1));
        // Create and start ZMF instance with module
        zmfInstance_ = zmf::instance::ZmfInstance::startInstance(module_, {}, UT_CONFIG_FILE);
        // ensures that the module is enabled! do NOT delete!
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
    }

    // Create  dummy module
    dummy =
            std::shared_ptr<DummyModule>(
                    new DummyModule(0,
                                    0,
                                    zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                    "DUMMY",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));


    // Create and start ZMF instance with module
    zmfInstanceDummy =
            zmf::instance::ZmfInstance::startInstance(dummy, {}, UT_CONFIG_FILE);


    while (!dummy->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    // subscribe and count

    zmf::data::MessageType deletedSwitch = switchRegistryModuleTopics_.from().switch_registry_module().switch_event().removed().build();
    zmf::data::MessageType newSwitch = switchRegistryModuleTopics_.from().switch_registry_module().switch_event().added().build();
    zmf::data::MessageType changedSwitch = switchRegistryModuleTopics_.from().switch_registry_module().switch_event().changed().build();
    zmf::data::MessageType featureRequest = switchAdapterTopics_.to().switch_adapter().switch_instance(
            switchVector[0].getSwitchID()).openflow().features_request().build();
    zmf::data::MessageType featureRequest2 = switchAdapterTopics_.to().switch_adapter().switch_instance(
            switchVector[1].getSwitchID()).openflow().features_request().build();
    zmf::data::MessageType featureRequest3 = switchAdapterTopics_.to().switch_adapter().switch_instance(
            3).openflow().features_request().build();
    zmf::data::MessageType portDescRequest = switchAdapterTopics_.to().switch_adapter().switch_instance(
            switchVector[0].getSwitchID()).openflow().of_1_0_barrier_request_of_1_3_multipart_request().build();
    zmf::data::MessageType portDescRequest2 = switchAdapterTopics_.to().switch_adapter().switch_instance(
            switchVector[1].getSwitchID()).openflow().of_1_0_barrier_request_of_1_3_multipart_request().build();
    zmf::data::MessageType portDescRequest3 = switchAdapterTopics_.to().switch_adapter().switch_instance(
            3).openflow().of_1_0_barrier_request_of_1_3_multipart_request().build();


    counter_deletedSwitch = 0;
    counter_featureRequest = 0;
    counter_newSwitch = 0;
    counter_changedSwitch = 0;
    counter_portDescRequest = 0;

    dummy.get()->getZmfForUnittests()->subscribe(
            deletedSwitch,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_deletedSwitch++;
            }
    );

    dummy.get()->getZmfForUnittests()->subscribe(
            newSwitch,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_newSwitch++;
            }
    );

    dummy.get()->getZmfForUnittests()->subscribe(
            changedSwitch,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_changedSwitch++;
            }
    );

    dummy.get()->getZmfForUnittests()->subscribe(
            featureRequest,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_featureRequest++;
            }
    );

    dummy.get()->getZmfForUnittests()->subscribe(
            featureRequest2,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_featureRequest++;
            }
    );

    dummy.get()->getZmfForUnittests()->subscribe(
            featureRequest3,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_featureRequest++;
            }
    );

    dummy.get()->getZmfForUnittests()->subscribe(
            portDescRequest,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_portDescRequest++;
            }
    );

    dummy.get()->getZmfForUnittests()->subscribe(
            portDescRequest2,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_portDescRequest++;
            }
    );

    dummy.get()->getZmfForUnittests()->subscribe(
            portDescRequest3,
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_portDescRequest++;
            }
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void SwitchRegistryModuleTests::tearDown() {
    zmfInstance_->requestDisableModule();
    zmfInstance_->requestStopInstance();
    zmfInstanceDummy->requestDisableModule();
    zmfInstanceDummy->requestStopInstance();
}

void SwitchRegistryModuleTests::testRequestAllSwitches() {
    //add Switches
    for (int i = 0; i < switchVector.size(); i++) {
        module_->UTAccessor_addSwitch(switchVector[i]);

    }
    //simulate the Request
    zmf::data::ZmfOutReply zmfOutReply = module_->handleRequest(*build_DemoAllSwitchRequest_ZmfMessage(),
                                                                zmf::data::ModuleUniqueId(123, 456));
    //Unpack Reply
    SwitchRegistryModule_Proto::Reply reply;
    reply.ParseFromArray(zmfOutReply.reply_immediate.getDataRaw(), zmfOutReply.reply_immediate.getDataLength());
    SwitchRegistryModule_Proto::Reply_GetAllSwitchesReply allSwitchesReply = reply.get_all_switches_reply();
    //get Switches
    std::vector<Switch> repliedSwitches;
    for (int i = 0; i < allSwitchesReply.switches_size(); i++) {
        repliedSwitches.insert(repliedSwitches.end(), protoToSwitch(allSwitchesReply.switches(i)));
    }
    bool allexist = true;
    for (int i = 0; i < repliedSwitches.size(); i++) {
        bool exists = false;
        for (int j = 0; j < switchVector.size(); j++) {
            bool same = (hasSameSwitchInfo(repliedSwitches[i], switchVector[j]) &&
                         hasSamePorts(repliedSwitches[i], switchVector[j]));
            if (same) {
                exists = true;
                break;
            }
        }
        if (!exists) allexist = false;
    }
    if (allexist) CPPUNIT_ASSERT(true);
    else
        CPPUNIT_ASSERT(false);

}

void SwitchRegistryModuleTests::testRequestSwitchByID() {
    for (int i = 0; i < switchVector.size(); i++) {
        module_->UTAccessor_addSwitch(switchVector[i]);

    }
    //simulate the Request
    zmf::data::ZmfOutReply zmfOutReply = module_->handleRequest(
            *build_DemoSwitchRequest_ZmfMessage(switchVector[0].getSwitchID()),
            zmf::data::ModuleUniqueId(123, 456));
    //Unpack Reply
    SwitchRegistryModule_Proto::Reply reply;
    reply.ParseFromArray(zmfOutReply.reply_immediate.getDataRaw(), zmfOutReply.reply_immediate.getDataLength());
    SwitchRegistryModule_Proto::Reply_GetSwitchByIdReply switchByIdReply = reply.get_switch_by_id_reply();
    //get Switches
    Switch repliedSwitch = protoToSwitch(switchByIdReply.switch_());
    if (hasSameSwitchInfo(repliedSwitch, switchVector[0]), hasSamePorts(repliedSwitch, switchVector[0]))
        CPPUNIT_ASSERT(true);
    else
        CPPUNIT_ASSERT(false);
}

void SwitchRegistryModuleTests::testRequestUnknownRequest() {
    SwitchRegistryModule_Proto::Request request;

    zmf::data::ZmfOutReply reply = module_->handleRequest(
            zmf::data::ZmfMessage(topicsFeatureReply_, request.SerializeAsString()),
            zmf::data::ModuleUniqueId(123, 456));
    CPPUNIT_ASSERT_EQUAL(zmf::data::ZmfOutReply::NO_REPLY, reply.type);
}

void SwitchRegistryModuleTests::testRequestTrollRequest() {

    zmf::data::ZmfOutReply reply = module_->handleRequest(
            zmf::data::ZmfMessage(topicsFeatureReply_, "123"),
            zmf::data::ModuleUniqueId(123, 456));
    CPPUNIT_ASSERT_EQUAL(zmf::data::ZmfOutReply::NO_REPLY, reply.type);
}

void SwitchRegistryModuleTests::testFeatureReplyUnknownSwitch() {

    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoFeatureReply(switchVector[0]);
    module_->UTAccessor_processFeatureReply(*message, zmf::data::ModuleUniqueId(123, 456));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testFeatureReplyKnownSwitchNoBecomeActive() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = false;
    aSwitch.switch_info_available = false;
    aSwitch.active = false;
    aSwitch.capabilities = 987;
    aSwitch.n_tables = 987;
    aSwitch.n_buffers = 987;
    aSwitch.auxiliary_id = 987;
    aSwitch.reserved = 987;
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoFeatureReply(switchVector[0]);
    module_->UTAccessor_processFeatureReply(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSameSwitchInfo(switchVector[0], module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testFeatureReplyKnownSwitchBecomeActive() {

    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = true;
    aSwitch.switch_info_available = false;
    aSwitch.active = false;
    aSwitch.capabilities = 987;
    aSwitch.n_tables = 987;
    aSwitch.n_buffers = 987;
    aSwitch.auxiliary_id = 987;
    aSwitch.reserved = 987;
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoFeatureReply(switchVector[0]);
    module_->UTAccessor_processFeatureReply(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSameSwitchInfo(switchVector[0], module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 1, 0, 0, 0));
}

void SwitchRegistryModuleTests::testFeatureReplyKnownSwitchActiveToBeginWith() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = true;
    aSwitch.switch_info_available = true;
    aSwitch.active = true;
    aSwitch.capabilities = 987;
    aSwitch.n_tables = 987;
    aSwitch.n_buffers = 987;
    aSwitch.auxiliary_id = 987;
    aSwitch.reserved = 987;
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoFeatureReply(switchVector[0]);
    module_->UTAccessor_processFeatureReply(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSameSwitchInfo(switchVector[0], module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 1, 0, 0));
}

void SwitchRegistryModuleTests::testFeatureReplyKnownSwitchOf1_0NotActive() {
    module_->UTAccessor_addSwitch(switchVector[1]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[1].getSwitchID()];
    aSwitch.got_ports = false;
    aSwitch.switch_info_available = false;
    aSwitch.active = false;
    aSwitch.capabilities = 987;
    aSwitch.n_tables = 987;
    aSwitch.n_buffers = 987;
    aSwitch.auxiliary_id = 987;
    aSwitch.reserved = 987;
    aSwitch.ports[0].advertised = 987;
    aSwitch.ports[0].switch_port = 987;
    aSwitch.ports[0].mac_address = 987;
    aSwitch.ports[0].port_name = 987;
    aSwitch.ports[0].config = 987;
    aSwitch.ports[0].state = 987;
    aSwitch.ports[0].curr = 987;
    aSwitch.ports[0].supported = 987;
    aSwitch.ports[0].peer = 987;
    aSwitch.ports[0].curr_speed = 987;
    aSwitch.ports[0].max_speed = 987;
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoFeatureReply(switchVector[1]);
    module_->UTAccessor_processFeatureReply(*message, zmf::data::ModuleUniqueId(123, switchVector[1].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSameSwitchInfo(switchVector[1], module_->UTAccessor_getAllSwitchesMap()[switchVector[1].getSwitchID()]));
    CPPUNIT_ASSERT(
            hasSamePorts(switchVector[1], module_->UTAccessor_getAllSwitchesMap()[switchVector[1].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 1, 0, 0, 0));
}

void SwitchRegistryModuleTests::testEchoKnownSwitch() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = false;
    aSwitch.switch_info_available = false;
    aSwitch.active = false;
    of_echo_request_t* echoRequestT = of_echo_request_new(OF_VERSION_1_3);
    std::shared_ptr<zmf::data::ZmfMessage> message = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsEchoRequest_, OF_OBJECT_TO_MESSAGE(echoRequestT), echoRequestT->length));
    module_->UTAccessor_processEcho(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    of_object_delete(echoRequestT);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 1, 1));
}

void SwitchRegistryModuleTests::testEchoUnknownSwitch() {
    of_echo_request_t* echoRequestT = of_echo_request_new(OF_VERSION_1_3);
    std::shared_ptr<zmf::data::ZmfMessage> message = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsEchoRequest_, OF_OBJECT_TO_MESSAGE(echoRequestT), echoRequestT->length));
    module_->UTAccessor_processEcho(*message, zmf::data::ModuleUniqueId(123, 3));
    of_object_delete(echoRequestT);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT_EQUAL(1, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 1, 1));
}

void SwitchRegistryModuleTests::testEchoKnownActiveSwitch() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = true;
    aSwitch.switch_info_available = true;
    aSwitch.active = true;
    of_echo_request_t* echoRequestT = of_echo_request_new(OF_VERSION_1_3);
    std::shared_ptr<zmf::data::ZmfMessage> message = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsEchoRequest_, OF_OBJECT_TO_MESSAGE(echoRequestT), echoRequestT->length));
    module_->UTAccessor_processEcho(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    of_object_delete(echoRequestT);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testPortStatusUnknownSwitch() {
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoPortStatus(switchVector[0].getSwitchID(),
                                                                          switchVector[0].of_version,
                                                                          switchVector[0].ports[0], 0);
    module_->UTAccessor_processPortStatus(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testPortStatusKnownSwitchReasonPortAdded() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = true;
    aSwitch.switch_info_available = true;
    aSwitch.active = true;
    aSwitch.ports.erase(aSwitch.ports.begin() + 0);
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoPortStatus(switchVector[0].getSwitchID(),
                                                                          switchVector[0].of_version,
                                                                          switchVector[0].ports[0], 0);
    module_->UTAccessor_processPortStatus(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSamePorts(switchVector[0], module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 1, 0, 0));
}

void SwitchRegistryModuleTests::testPortStatusKnownSwitchReasonPortDeleted() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoPortStatus(switchVector[0].getSwitchID(),
                                                                          switchVector[0].of_version,
                                                                          switchVector[0].ports[0], 1);
    module_->UTAccessor_processPortStatus(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT_EQUAL(0,
                         (const int) module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()].ports.size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 1, 0, 0));
}

void SwitchRegistryModuleTests::testPortStatusKnownSwitchReasonPortModified() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = true;
    aSwitch.switch_info_available = true;
    aSwitch.active = true;
    aSwitch.ports[0].advertised = 987;
    aSwitch.ports[0].mac_address = 987;
    aSwitch.ports[0].port_name = 987;
    aSwitch.ports[0].config = 987;
    aSwitch.ports[0].state = 987;
    aSwitch.ports[0].curr = 987;
    aSwitch.ports[0].supported = 987;
    aSwitch.ports[0].peer = 987;
    aSwitch.ports[0].curr_speed = 987;
    aSwitch.ports[0].max_speed = 987;
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoPortStatus(switchVector[0].getSwitchID(),
                                                                          switchVector[0].of_version,
                                                                          switchVector[0].ports[0], 2);
    module_->UTAccessor_processPortStatus(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSamePorts(switchVector[0], module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 1, 0, 0));
}

void SwitchRegistryModuleTests::testPortStatusKnownSwitchReasonInvalid() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoPortStatus(switchVector[0].getSwitchID(),
                                                                          switchVector[0].of_version,
                                                                          switchVector[0].ports[0], 4);
    module_->UTAccessor_processPortStatus(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testMultiPartAnyUnknownSwitch() {
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoMultipartReply(switchVector[0]);
    module_->UTAccessor_processMultipartReply(*message, zmf::data::ModuleUniqueId(123, 456));
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testMultiPartPortDescReplyKnownSwitchNoBecomeActive() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = false;
    aSwitch.switch_info_available = false;
    aSwitch.active = false;
    aSwitch.ports[0].advertised = 987;
    aSwitch.ports[0].switch_port = 987;
    aSwitch.ports[0].mac_address = 987;
    aSwitch.ports[0].port_name = 987;
    aSwitch.ports[0].config = 987;
    aSwitch.ports[0].state = 987;
    aSwitch.ports[0].curr = 987;
    aSwitch.ports[0].supported = 987;
    aSwitch.ports[0].peer = 987;
    aSwitch.ports[0].curr_speed = 987;
    aSwitch.ports[0].max_speed = 987;
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoMultipartReply(switchVector[0]);
    module_->UTAccessor_processMultipartReply(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSamePorts(switchVector[0], module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testMultiPartPortDescReplyKnownSwitchBecomeActive() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = false;
    aSwitch.switch_info_available = true;
    aSwitch.active = false;
    aSwitch.ports[0].advertised = 987;
    aSwitch.ports[0].switch_port = 987;
    aSwitch.ports[0].mac_address = 987;
    aSwitch.ports[0].port_name = 987;
    aSwitch.ports[0].config = 987;
    aSwitch.ports[0].state = 987;
    aSwitch.ports[0].curr = 987;
    aSwitch.ports[0].supported = 987;
    aSwitch.ports[0].peer = 987;
    aSwitch.ports[0].curr_speed = 987;
    aSwitch.ports[0].max_speed = 987;
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoMultipartReply(switchVector[0]);
    module_->UTAccessor_processMultipartReply(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSamePorts(switchVector[0], module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 1, 0, 0, 0));
}

void SwitchRegistryModuleTests::testMultiPartPortDescReplyKnownSwitchActiveToBeginWith() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = true;
    aSwitch.switch_info_available = true;
    aSwitch.active = true;
    aSwitch.ports[0].advertised = 987;
    aSwitch.ports[0].switch_port = 987;
    aSwitch.ports[0].mac_address = 987;
    aSwitch.ports[0].port_name = 987;
    aSwitch.ports[0].config = 987;
    aSwitch.ports[0].state = 987;
    aSwitch.ports[0].curr = 987;
    aSwitch.ports[0].supported = 987;
    aSwitch.ports[0].peer = 987;
    aSwitch.ports[0].curr_speed = 987;
    aSwitch.ports[0].max_speed = 987;
    std::shared_ptr<zmf::data::ZmfMessage> message = build_DemoMultipartReply(switchVector[0]);
    module_->UTAccessor_processMultipartReply(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(
            hasSamePorts(switchVector[0], module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()]));
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 1, 0, 0));
}

void SwitchRegistryModuleTests::testMultiPartPortNoDescReply() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    of_table_stats_reply_t* tableStatsReplyT = of_table_stats_reply_new(OF_VERSION_1_3);
    std::shared_ptr<zmf::data::ZmfMessage> message = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsMultipartReply_, OF_OBJECT_TO_MESSAGE(tableStatsReplyT),
                                      tableStatsReplyT->length));
    module_->UTAccessor_processMultipartReply(*message, zmf::data::ModuleUniqueId(123, switchVector[0].getSwitchID()));
    of_object_delete(tableStatsReplyT);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT_EQUAL(1, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testModuleStateChangeNoSwitchAdapter() {
    std::shared_ptr<DummyModule> dummy2 =
            std::shared_ptr<DummyModule>(
                    new DummyModule(0,
                                    0,
                                    zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                    "DUMMY2",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));


    // Create and start ZMF instance with module
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstanceDummy2 =
            zmf::instance::ZmfInstance::startInstance(dummy2, {}, UT_CONFIG_FILE);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());

}

void SwitchRegistryModuleTests::testModuleStateChangeAddNew1_3SwitchAdapterAndDeleteNoActiveOne() {
    std::shared_ptr<DummyModule> dummy2 =
            std::shared_ptr<DummyModule>(
                    new DummyModule(switchVector[0].getSwitchID(),
                                    0,
                                    zsdn::MODULE_TYPE_ID_SwitchAdapter,
                                    "DUMMY2",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));

    // Create and start ZMF instance with module
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstanceDummy2 =
            zmf::instance::ZmfInstance::startInstance(dummy2, {}, UT_CONFIG_FILE);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //add one without version-->nothing should happen
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
    dummy2->getZmfForUnittests()->onModuleAdditionalStateChanged({OF_VERSION_1_3});
    zmfInstanceDummy2->requestDisableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    zmfInstanceDummy2->requestEnableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //-->1.3 Switch should be added
    CPPUNIT_ASSERT_EQUAL(1, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 1, 1));
    zmfInstanceDummy2->requestStopInstance();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //swtich deleted but no deleted message(since not active yet)
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 1, 1));//00011 cause we received 2 for the test before
}

void SwitchRegistryModuleTests::testModuleStateChangeAddKnownSwitchAdapterAndDeleteActiveOne() {
    module_->UTAccessor_addSwitch(switchVector[0]);
    Switch& aSwitch = module_->UTAccessor_getAllSwitchesMap()[switchVector[0].getSwitchID()];
    aSwitch.got_ports = true;
    aSwitch.switch_info_available = true;
    aSwitch.active = true;
    std::shared_ptr<DummyModule> dummy2 =
            std::shared_ptr<DummyModule>(
                    new DummyModule(switchVector[0].getSwitchID(),
                                    0,
                                    zsdn::MODULE_TYPE_ID_SwitchAdapter,
                                    "DUMMY2",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));

    // Create and start ZMF instance with module
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstanceDummy2 =
            zmf::instance::ZmfInstance::startInstance(dummy2, {}, UT_CONFIG_FILE);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CPPUNIT_ASSERT_EQUAL(1, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
    zmfInstanceDummy2->requestStopInstance();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(1, 0, 0, 0, 0));
}

void SwitchRegistryModuleTests::testModuleStateChangeAddNew1_0SwitchAdapter() {
    std::shared_ptr<DummyModule> dummy2 =
            std::shared_ptr<DummyModule>(
                    new DummyModule(switchVector[0].getSwitchID(),
                                    0,
                                    zsdn::MODULE_TYPE_ID_SwitchAdapter,
                                    "DUMMY2",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));

    // Create and start ZMF instance with module
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstanceDummy2 =
            zmf::instance::ZmfInstance::startInstance(dummy2, {}, UT_CONFIG_FILE);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //add one without version-->nothing should happen
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));
    dummy2->getZmfForUnittests()->onModuleAdditionalStateChanged({OF_VERSION_1_0});
    zmfInstanceDummy2->requestDisableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    zmfInstanceDummy2->requestEnableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //-->1.3 Switch should be added
    CPPUNIT_ASSERT_EQUAL(1, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 1, 0));
}

void SwitchRegistryModuleTests::testDisableAndEnableModuleWithValidSwitchInZMF() {
    std::shared_ptr<DummyModule> dummy2 =
            std::shared_ptr<DummyModule>(
                    new DummyModule(switchVector[0].getSwitchID(),
                                    0,
                                    zsdn::MODULE_TYPE_ID_SwitchAdapter,
                                    "DUMMY2",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));

    // Create and start ZMF instance with module
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstanceDummy2 =
            zmf::instance::ZmfInstance::startInstance(dummy2, {}, UT_CONFIG_FILE);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    dummy2->getZmfForUnittests()->onModuleAdditionalStateChanged({OF_VERSION_1_3});
    zmfInstanceDummy2->requestDisableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    zmfInstanceDummy2->requestEnableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CPPUNIT_ASSERT_EQUAL(1, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 1, 1));
    zmfInstance_->requestDisableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    zmfInstance_->requestEnableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CPPUNIT_ASSERT_EQUAL(1, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 2, 2));
}

void SwitchRegistryModuleTests::testDisableAndEnableModuleWithInvalidSwitchInZMF() {
    std::shared_ptr<DummyModule> dummy2 =
            std::shared_ptr<DummyModule>(
                    new DummyModule(switchVector[0].getSwitchID(),
                                    0,
                                    zsdn::MODULE_TYPE_ID_SwitchAdapter,
                                    "DUMMY2",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));

    // Create and start ZMF instance with module
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstanceDummy2 =
            zmf::instance::ZmfInstance::startInstance(dummy2, {}, UT_CONFIG_FILE);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    zmfInstance_->requestDisableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    zmfInstance_->requestEnableModule();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CPPUNIT_ASSERT_EQUAL(0, (const int) module_->UTAccessor_getAllSwitchesMap().size());
    CPPUNIT_ASSERT(compareMessageCounters(0, 0, 0, 0, 0));

}


//------------------------------------- Helper Methods -------------------------------------



std::shared_ptr<zmf::data::ZmfMessage> SwitchRegistryModuleTests::build_DemoAllSwitchRequest_ZmfMessage() {
    SwitchRegistryModule_Proto::Request request;
    SwitchRegistryModule_Proto::Request_GetAllSwitchesRequest* allSwitchesRequest = new SwitchRegistryModule_Proto::Request_GetAllSwitchesRequest();
    request.set_allocated_get_all_switches_request(allSwitchesRequest);
    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicAllSwitches_, request.SerializeAsString()));
}

std::shared_ptr<zmf::data::ZmfMessage> SwitchRegistryModuleTests::build_DemoSwitchRequest_ZmfMessage(
        uint64_t switchID) {
    SwitchRegistryModule_Proto::Request request;
    SwitchRegistryModule_Proto::Request_GetSwitchByIdRequest* switchRequest = new SwitchRegistryModule_Proto::Request_GetSwitchByIdRequest();
    switchRequest->set_switch_dpid(switchID);
    request.set_allocated_get_switch_by_id_request(switchRequest);
    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicSwitch_, request.SerializeAsString()));
}

std::shared_ptr<zmf::data::ZmfMessage> SwitchRegistryModuleTests::build_DemoMultipartReply(Switch aSwitch) {
    of_port_desc_stats_reply_t* portDescReply = of_port_desc_stats_reply_new(aSwitch.of_version);
    of_list_port_desc_t* entries = of_list_port_desc_new(aSwitch.of_version);
    for (Port& port : aSwitch.ports) {

        of_port_desc_t* portDescT = of_port_desc_new(aSwitch.of_version);

        of_mac_addr_t* macAddrT = (of_mac_addr_t*) zsdn::NetUtils::uint64_to_mac_address_array(port.mac_address);
        of_port_desc_hw_addr_set(portDescT, *macAddrT);
        delete[] macAddrT;
        of_port_name_t* portName = (of_port_name_t*) port.port_name.c_str();

        of_port_desc_name_set(portDescT, *portName);
        of_port_desc_port_no_set(portDescT, port.switch_port);
        of_port_desc_config_set(portDescT, port.config);
        of_port_desc_state_set(portDescT, port.state);
        of_port_desc_curr_set(portDescT, port.curr);
        of_port_desc_advertised_set(portDescT, port.advertised);
        of_port_desc_supported_set(portDescT, port.supported);
        of_port_desc_peer_set(portDescT, port.peer);
        of_port_desc_curr_speed_set(portDescT, port.curr_speed);
        of_port_desc_max_speed_set(portDescT, port.max_speed);

        of_list_port_desc_append(entries, portDescT);
        of_port_desc_delete(portDescT);
    }
    of_port_desc_stats_reply_entries_set(portDescReply, entries);
    of_list_port_desc_delete(entries);

    std::shared_ptr<zmf::data::ZmfMessage> message = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsMultipartReply_, OF_OBJECT_TO_MESSAGE(portDescReply),
                                      portDescReply->length));
    of_object_delete(portDescReply);

    return message;
}

std::shared_ptr<zmf::data::ZmfMessage> SwitchRegistryModuleTests::build_DemoFeatureReply(Switch aSwitch) {
    of_features_reply_t* featuresReplyT = of_features_reply_new(aSwitch.of_version);
    printf("check 0");
    if (aSwitch.of_version >= OF_VERSION_1_3) {
        of_features_reply_auxiliary_id_set(featuresReplyT, aSwitch.auxiliary_id);
    }

    of_features_reply_n_buffers_set(featuresReplyT, aSwitch.n_buffers);
    of_features_reply_n_tables_set(featuresReplyT, aSwitch.n_tables);
    of_features_reply_capabilities_set(featuresReplyT, aSwitch.capabilities);

    if (aSwitch.of_version > OF_VERSION_1_0) {
        of_features_reply_reserved_set(featuresReplyT, aSwitch.reserved);
    } else {
        of_features_reply_actions_set(featuresReplyT, aSwitch.reserved);
    }

    if (aSwitch.of_version < OF_VERSION_1_3) {
        of_list_port_desc_t* entries = of_list_port_desc_new(aSwitch.of_version);
        for (Port& port : aSwitch.ports) {

            of_port_desc_t* portDescT = of_port_desc_new(aSwitch.of_version);

            of_mac_addr_t* macAddrT = (of_mac_addr_t*) zsdn::NetUtils::uint64_to_mac_address_array(port.mac_address);
            of_port_desc_hw_addr_set(portDescT, *macAddrT);
            delete[] macAddrT;
            of_port_name_t* portName = (of_port_name_t*) port.port_name.c_str();

            of_port_desc_name_set(portDescT, *portName);
            of_port_desc_port_no_set(portDescT, port.switch_port);
            of_port_desc_config_set(portDescT, port.config);
            of_port_desc_state_set(portDescT, port.state);
            of_port_desc_curr_set(portDescT, port.curr);
            of_port_desc_advertised_set(portDescT, port.advertised);
            of_port_desc_supported_set(portDescT, port.supported);
            of_port_desc_peer_set(portDescT, port.peer);
            if (aSwitch.of_version >= OF_VERSION_1_1) {
                of_port_desc_curr_speed_set(portDescT, port.curr_speed);
                of_port_desc_max_speed_set(portDescT, port.max_speed);
            }

            of_list_port_desc_append(entries, portDescT);
            of_port_desc_delete(portDescT);
        }
        of_features_reply_ports_set(featuresReplyT, entries);
        of_list_port_desc_delete(entries);
    }

    std::shared_ptr<zmf::data::ZmfMessage> message = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsFeatureReply_, OF_OBJECT_TO_MESSAGE(featuresReplyT),
                                      featuresReplyT->length));
    of_object_delete(featuresReplyT);

    return message;
}

std::shared_ptr<zmf::data::ZmfMessage> SwitchRegistryModuleTests::build_DemoPortStatus(uint64_t switchID,
                                                                                       of_version_t of_version,
                                                                                       Port port, uint8_t reason) {

    of_port_status_t* portStatusT = of_port_status_new(of_version);
    of_port_status_reason_set(portStatusT, reason);
    of_port_desc_t* portDescT = of_port_desc_new(of_version);
    //of_port_status_desc_bind(portStatusT, portDescT);
    of_mac_addr_t* macAddrT = (of_mac_addr_t*) zsdn::NetUtils::uint64_to_mac_address_array(port.mac_address);
    of_port_desc_hw_addr_set(portDescT, *macAddrT);
    delete[] macAddrT;
    of_port_name_t* portName = (of_port_name_t*) port.port_name.c_str();

    of_port_desc_name_set(portDescT, *portName);
    of_port_desc_port_no_set(portDescT, port.switch_port);
    of_port_desc_config_set(portDescT, port.config);
    of_port_desc_state_set(portDescT, port.state);
    of_port_desc_curr_set(portDescT, port.curr);
    of_port_desc_advertised_set(portDescT, port.advertised);
    of_port_desc_supported_set(portDescT, port.supported);
    of_port_desc_peer_set(portDescT, port.peer);
    if (of_version >= OF_VERSION_1_1) {
        of_port_desc_curr_speed_set(portDescT, port.curr_speed);
        of_port_desc_max_speed_set(portDescT, port.max_speed);
    }

    of_port_status_desc_set(portStatusT, portDescT);
    of_port_desc_delete(portDescT);


    std::shared_ptr<zmf::data::ZmfMessage> message = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsPortStatus_, OF_OBJECT_TO_MESSAGE(portStatusT),
                                      portStatusT->length));
    of_port_status_delete(portStatusT);

    return message;
}

bool SwitchRegistryModuleTests::hasSameSwitchInfo(Switch aSwitch1, Switch aSwitch2) {
    if (aSwitch1.of_version >= OF_VERSION_1_3) {
        if (aSwitch1.auxiliary_id != aSwitch2.auxiliary_id) return false;
    }
    if (aSwitch1.capabilities != aSwitch2.capabilities) return false;
    if (aSwitch1.n_buffers != aSwitch2.n_buffers) return false;
    if (aSwitch1.n_tables != aSwitch2.n_tables) return false;
    if (aSwitch1.of_version != aSwitch2.of_version) return false;
    if (aSwitch1.getSwitchID() != aSwitch2.getSwitchID()) return false;
    if (aSwitch1.reserved != aSwitch2.reserved) return false;

    return true;
}

bool SwitchRegistryModuleTests::hasSamePorts(Switch aSwitch1, Switch aSwitch2) {
    if (aSwitch1.ports.size() != aSwitch2.ports.size()) return false;
    for (int i = 0; i < aSwitch1.ports.size(); i++) {
        bool exists = false;
        for (int j = 0; j < aSwitch2.ports.size(); j++) {
            bool same = true;
            if (aSwitch1.ports[i].switch_port != aSwitch2.ports[j].switch_port) same = false;
            if (aSwitch1.ports[i].advertised != aSwitch2.ports[j].advertised) same = false;
            if (aSwitch1.ports[i].config != aSwitch2.ports[j].config) same = false;
            if (aSwitch1.ports[i].curr != aSwitch2.ports[j].curr) same = false;
            if (aSwitch1.ports[i].mac_address != aSwitch2.ports[j].mac_address) same = false;
            if (aSwitch1.ports[i].peer != aSwitch2.ports[j].peer) same = false;
            if (aSwitch1.ports[i].supported != aSwitch2.ports[j].supported) same = false;
            if (aSwitch1.ports[i].state != aSwitch2.ports[j].state) same = false;
            if (aSwitch1.ports[i].port_name != aSwitch2.ports[j].port_name) same = false;
            if (aSwitch1.of_version >= OF_VERSION_1_1) {
                if (aSwitch1.ports[i].max_speed != aSwitch2.ports[j].max_speed) same = false;
                if (aSwitch1.ports[i].curr_speed != aSwitch2.ports[j].curr_speed) same = false;
            }
            if (same) {
                exists = true;
                break;
            }
        }
        if (!exists) return false;
    }
    return true;
}

Switch SwitchRegistryModuleTests::protoToSwitch(common::topology::Switch aSwitch) {
    Switch result = Switch(aSwitch.switch_dpid(), (of_version_t) aSwitch.openflow_version());
    result.n_buffers = aSwitch.switch_specs().n_buffers();
    result.n_tables = (uint8_t) aSwitch.switch_specs().n_tables();
    result.auxiliary_id = (uint8_t) aSwitch.switch_specs().auxiliary_id();
    result.capabilities = aSwitch.switch_specs().capabilities();
    result.reserved = aSwitch.switch_specs().reserved();

    for (const common::topology::SwitchPort port : aSwitch.switch_ports()) {
        Port aPort = Port(port.attachment_point().switch_port());
        aPort.mac_address = port.port_specs().mac_address();
        aPort.port_name = port.port_specs().port_name();
        aPort.config = port.port_specs().config();
        aPort.state = port.port_specs().state();
        aPort.curr = port.port_specs().curr();
        aPort.advertised = port.port_specs().advertised();
        aPort.supported = port.port_specs().supported();
        aPort.peer = port.port_specs().peer();
        aPort.curr_speed = port.port_specs().curr_speed();
        aPort.max_speed = port.port_specs().max_speed();
        result.ports.insert(result.ports.end(), aPort);
    }
    return result;

}

bool SwitchRegistryModuleTests::compareMessageCounters(int expected_deletedSwitch, int expected_newSwitch,
                                                       int expected_changedSwitch,
                                                       int expected_featureRequest, int expected_portDescRequest) {
    return (expected_deletedSwitch == counter_deletedSwitch
            && expected_newSwitch == counter_newSwitch
            && expected_changedSwitch == counter_changedSwitch
            && expected_featureRequest == counter_featureRequest
            && expected_portDescRequest == counter_portDescRequest);
}
