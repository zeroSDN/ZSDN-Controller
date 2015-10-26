//
// Created by zsdn on 6/25/15.
// @author Tobias Freundorfer last edited: 15.08.2015
//

#include <zsdn/proto/CommonTopology.pb.h>
#include <zsdn/proto/SwitchRegistryModule.pb.h>
#include <zsdn/proto/LinkDiscoveryModule.pb.h>
#include <zsdn/proto/TopologyModule.pb.h>
#include <zmf/ZmfInstance.hpp>
#include <ModuleTypeIDs.hpp>
#include <RequestUtils.h>
#include "TopologyModuleTests.h"
#include "dummyModules/DummyModule.hpp"
#include <UnittestConfigUtil.hpp>


TopologyModuleTests::TopologyModuleTests() {

}

std::shared_ptr<zmf::data::ZmfMessage> TopologyModuleTests::build_DemoSwitchEventAdded_ZmfMessage(
        common::topology::Switch aSwitch) {
    SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();

    common::topology::Switch* aSwitchPtr = new common::topology::Switch();
    aSwitchPtr->CopyFrom(aSwitch);
    switchEvent->set_allocated_switch_added(aSwitchPtr);


    SwitchRegistryModule_Proto::From fromMessage;
    fromMessage.set_allocated_switch_event(switchEvent);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsSwitchEvent_, fromMessage.SerializeAsString()));
}

std::shared_ptr<zmf::data::ZmfMessage> TopologyModuleTests::build_DemoSwitchEventChanged_ZmfMessage(
        common::topology::Switch switchBefore, common::topology::Switch switchNow) {
    SwitchRegistryModule_Proto::From_SwitchEvent_SwitchChanged* switchEventChanged = new SwitchRegistryModule_Proto::From_SwitchEvent_SwitchChanged;

    common::topology::Switch* switchBeforePtr = new common::topology::Switch();
    switchBeforePtr->CopyFrom(switchBefore);
    common::topology::Switch* switchNowPtr = new common::topology::Switch();
    switchNowPtr->CopyFrom(switchNow);

    switchEventChanged->set_allocated_switch_before(switchBeforePtr);
    switchEventChanged->set_allocated_switch_now(switchNowPtr);


    SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();
    switchEvent->set_allocated_switch_changed(switchEventChanged);

    SwitchRegistryModule_Proto::From fromMessage;
    fromMessage.set_allocated_switch_event(switchEvent);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsSwitchEvent_, fromMessage.SerializeAsString()));
}


std::shared_ptr<zmf::data::ZmfMessage> TopologyModuleTests::build_DemoSwitchEventRemoved_ZmfMessage(
        common::topology::Switch aSwitch) {
    SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();

    common::topology::Switch* aSwitchPtr = new common::topology::Switch();
    aSwitchPtr->CopyFrom(aSwitch);
    switchEvent->set_allocated_switch_removed(aSwitchPtr);

    SwitchRegistryModule_Proto::From fromMessage;
    fromMessage.set_allocated_switch_event(switchEvent);

    return std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsSwitchEvent_, fromMessage.SerializeAsString()));
}

std::shared_ptr<zmf::data::ZmfMessage> TopologyModuleTests::build_DemoSwitchLinkEventAdded_ZmfMessage(
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

std::shared_ptr<zmf::data::ZmfMessage> TopologyModuleTests::build_DemoSwitchLinkEventRemoved_ZmfMessage(
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

void TopologyModuleTests::setUp() {

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

    /// Mock dependencies
    try {
        // Create module
        switchRegistryModuleMock_ = std::shared_ptr<zmf::AbstractModule>(
                new DummyModule(0, 0, zsdn::MODULE_TYPE_ID_SwitchRegistryModule, "SwitchRegistryModule",
                                [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                   zmf::data::ModuleState lastState) { }, [this](const zmf::data::ZmfMessage& message,
                                                                                 const zmf::data::ModuleUniqueId& sender) {
                            SwitchRegistryModule_Proto::Request request;

                            bool parseSuccess = request.ParseFromString(message.getData());
                            if (!parseSuccess) {
                                std::string err = "Could not parse Protobuffer format.";
                                throw Poco::Exception(err, 1);
                            }

                            switch (request.RequestMsg_case()) {
                                case SwitchRegistryModule_Proto::Request::kGetAllSwitchesRequest: {
                                    SwitchRegistryModule_Proto::Reply reply;

                                    SwitchRegistryModule_Proto::Reply_GetAllSwitchesReply* asReply = new SwitchRegistryModule_Proto::Reply_GetAllSwitchesReply();

                                    // Returning the first two switches
                                    asReply->add_switches()->CopyFrom(demoSwitches_[0]);
                                    asReply->add_switches()->CopyFrom(demoSwitches_[1]);
                                    reply.set_allocated_get_all_switches_reply(asReply);

                                    zmf::data::ZmfMessage msg(
                                            switchregistrymodule_topics::REPLY().switch_registry_module().get_all_switches().build(),
                                            reply.SerializeAsString());

                                    return zmf::data::ZmfOutReply::createImmediateReply(msg);
                                }
                                default:
                                    return zmf::data::ZmfOutReply::createNoReply();
                            }

                        }));
        // Create and start ZMF instance with module
        switchRegistryModuleZmfInstance_ = zmf::instance::ZmfInstance::startInstance(switchRegistryModuleMock_, {},
                                                                                     UT_CONFIG_FILE);
        // ensures that the module is enabled! do NOT delete!
        while (!switchRegistryModuleMock_->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
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

                                    // Returning the first two switchlinks
                                    slReply->add_switch_links()->CopyFrom(demoSwitchLinks_[0]);
                                    slReply->add_switch_links()->CopyFrom(demoSwitchLinks_[1]);

                                    reply.set_allocated_get_all_switch_links_reply(slReply);

                                    zmf::data::ZmfMessage msg(
                                            linkdiscoverymodule_topics::REPLY().link_discovery_module().get_all_switch_links().build(),
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
        linkDiscoveryModuleZmfInstance_ = zmf::instance::ZmfInstance::startInstance(linkDiscoveryModuleMock_, {},
                                                                                    UT_CONFIG_FILE);
        // ensures that the module is enabled! do NOT delete!
        while (!linkDiscoveryModuleMock_->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
    }

    // Start module under test: TopologyModule
    try {
        // Create module
        module_ = std::shared_ptr<TopologyModule>(
                new TopologyModule(0));
        // Create and start ZMF instance with module
        zmfInstance_ = zmf::instance::ZmfInstance::startInstance(module_, {}, UT_CONFIG_FILE);
        // ensures that the module is enabled! do NOT delete!
        while (!module_->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
    }
}

void TopologyModuleTests::tearDown() {
    zmfInstance_->requestDisableModule();
    zmfInstance_->requestStopInstance();
    switchRegistryModuleZmfInstance_->requestDisableModule();
    switchRegistryModuleZmfInstance_->requestStopInstance();
    linkDiscoveryModuleZmfInstance_->requestStopInstance();
    linkDiscoveryModuleZmfInstance_->requestDisableModule();
    zmfInstance_->joinExecution();
    switchRegistryModuleZmfInstance_->joinExecution();
    linkDiscoveryModuleZmfInstance_->joinExecution();
}

void TopologyModuleTests::testSwitchEventAdded() {
    // Add two unregistered STSLinks to simulate the checkSwitchToSwitchLinkStateChanged addtionally
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(demoSwitchLinks_[3]),
                                              demoModuleUniqueId_);
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(demoSwitchLinks_[4]),
                                              demoModuleUniqueId_);

    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventAdded_ZmfMessage(demoSwitches_[2]),
                                          demoModuleUniqueId_);


    int mapSize = module_->UTAccessor_getSwitchesCache().size();
    CPPUNIT_ASSERT_EQUAL(3, mapSize);

    std::string expected0 = demoSwitches_[0].SerializeAsString();
    std::string actual0 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[0].switch_dpid())->second.SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(expected0, actual0);

    std::string expected1 = demoSwitches_[1].SerializeAsString();
    std::string actual1 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[1].switch_dpid())->second.SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(expected1, actual1);

    std::string expected2 = demoSwitches_[2].SerializeAsString();
    std::string actual2 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[2].switch_dpid())->second.SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(expected2, actual2);
}

void TopologyModuleTests::testSwitchEventChanged() {
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventAdded_ZmfMessage(demoSwitches_[2]),
                                          demoModuleUniqueId_);
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventAdded_ZmfMessage(demoSwitches_[3]),
                                          demoModuleUniqueId_);
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventAdded_ZmfMessage(demoSwitches_[4]),
                                          demoModuleUniqueId_);

    // Use another switch from the demo Switches and manipulate the switchID so that alle fields are different except of the switchID itself
    common::topology::Switch modifiedSwitch = demoSwitches_[0];

    modifiedSwitch.set_switch_dpid(demoSwitches_[3].switch_dpid());
    modifiedSwitch.mutable_switch_ports(0)->mutable_attachment_point()->set_switch_dpid(demoSwitches_[3].switch_dpid());

    module_->UTAccessor_handleSwitchEvent(
            *build_DemoSwitchEventChanged_ZmfMessage(demoSwitches_[0], modifiedSwitch),
            zmf::data::ModuleUniqueId(999, demoSwitches_[3].switch_dpid()));

    int mapSize = module_->UTAccessor_getSwitchesCache().size();
    CPPUNIT_ASSERT_EQUAL(5, mapSize);

    std::string expected0 = demoSwitches_[2].SerializeAsString();
    std::string actual0 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[2].switch_dpid())->second.SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(expected0, actual0);

    std::string expected1 = modifiedSwitch.SerializeAsString();
    std::string actual1 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[3].switch_dpid())->second.SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(expected1, actual1);

    std::string expectedInvalid = demoSwitches_[3].SerializeAsString();
    std::string actualInvalid = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[3].switch_dpid())->second.SerializeAsString();
    CPPUNIT_ASSERT(expectedInvalid != actualInvalid);

    std::string expected2 = demoSwitches_[4].SerializeAsString();
    std::string actual2 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[4].switch_dpid())->second.SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(expected2, actual2);
}

void TopologyModuleTests::testSwitchEventChangedUnknownSwitch() {
    int switchCacheSizeBefore = module_->UTAccessor_getSwitchesCache().size();
    // Send a SwitchEvent Changed for an currently unknown Switch.
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventChanged_ZmfMessage(demoSwitches_[3], demoSwitches_[3]),
                                          demoModuleUniqueId_);
    int switchCacheSizeAfter = module_->UTAccessor_getSwitchesCache().size();
    CPPUNIT_ASSERT_EQUAL(switchCacheSizeBefore + 1, switchCacheSizeAfter);
}

void TopologyModuleTests::testSwitchEventRemoved() {
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventAdded_ZmfMessage(demoSwitches_[2]),
                                          demoModuleUniqueId_);

    std::string serializedSwitch1 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[1].switch_dpid())->second.SerializeAsString();
    std::string serializedSwitch2 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[2].switch_dpid())->second.SerializeAsString();

    // Insert event to remove a switch
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventRemoved_ZmfMessage(demoSwitches_[0]),
                                          demoModuleUniqueId_);

    int mapSize = module_->UTAccessor_getSwitchesCache().size();
    CPPUNIT_ASSERT_EQUAL(2, mapSize);

    std::map<uint64_t, common::topology::Switch>::const_iterator iterator1 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[1].switch_dpid());
    CPPUNIT_ASSERT(iterator1 != module_->UTAccessor_getSwitchesCache().end());
    std::string serializedMapElement1 = iterator1->second.SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(serializedSwitch1, serializedMapElement1);

    std::map<uint64_t, common::topology::Switch>::const_iterator iterator2 = module_->UTAccessor_getSwitchesCache().find(
            demoSwitches_[2].switch_dpid());
    CPPUNIT_ASSERT(iterator2 != module_->UTAccessor_getSwitchesCache().end());
    std::string serializedMapElement2 = iterator2->second.SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(serializedSwitch2, serializedMapElement2);
}

void TopologyModuleTests::testSwitchEventRemovedUnknownSwitch() {
    int switchCacheBefore = module_->UTAccessor_getSwitchesCache().size();
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventRemoved_ZmfMessage(demoSwitches_[3]),
                                          demoModuleUniqueId_);
    int switchCacheAfter = module_->UTAccessor_getSwitchesCache().size();

    CPPUNIT_ASSERT_EQUAL(switchCacheBefore, switchCacheAfter);
}

void TopologyModuleTests::testSwitchEventInvalidProtobuf() {
    // SwitchEvent with invalid data --> parseError
    std::string invalidData = "qjweiqohbaosbnfas";

    int switchCacheBefore = module_->UTAccessor_getSwitchesCache().size();
    module_->UTAccessor_handleSwitchEvent(
            zmf::data::ZmfMessage(topicsSwitchEvent_, invalidData), demoModuleUniqueId_);
    int switchCacheAfter = module_->UTAccessor_getSwitchesCache().size();
    CPPUNIT_ASSERT_EQUAL(switchCacheBefore, switchCacheAfter);

}

void TopologyModuleTests::testSwitchEventUnknownEvent() {
    // SwitchEvent with unset Switch! --> unknown SwitchEvent
    SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();

    SwitchRegistryModule_Proto::From fromMessage;
    fromMessage.set_allocated_switch_event(switchEvent);
    int switchCacheBefore = module_->UTAccessor_getSwitchesCache().size();
    module_->UTAccessor_handleSwitchEvent(
            zmf::data::ZmfMessage(topicsSwitchEvent_, fromMessage.SerializeAsString()), demoModuleUniqueId_);
    int switchCacheAfter = module_->UTAccessor_getSwitchesCache().size();

    CPPUNIT_ASSERT_EQUAL(switchCacheBefore, switchCacheAfter);
}

void TopologyModuleTests::testSwitchToSwitchLinkAddedRegistered() {
    // Add Switches for the SwitchToSwitchLinks
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventAdded_ZmfMessage(demoSwitches_[2]),
                                          demoModuleUniqueId_);

    // Add SwitchToSwitchLinks between the Switches
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(demoSwitchLinks_[2]),
                                              demoModuleUniqueId_);

    int vectorSizeRegisteredCache = module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size();
    CPPUNIT_ASSERT_EQUAL(3, vectorSizeRegisteredCache);
    int vectorSizeUnregisteredCache = module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size();
    CPPUNIT_ASSERT_EQUAL(0, vectorSizeUnregisteredCache);

    std::string exptextedSerializedLink0 = demoSwitchLinks_[0].SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(exptextedSerializedLink0,
                         module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered()[0].SerializeAsString());
    std::string exptextedSerializedLink1 = demoSwitchLinks_[1].SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(exptextedSerializedLink1,
                         module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered()[1].SerializeAsString());
    std::string exptextedSerializedLink2 = demoSwitchLinks_[2].SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(exptextedSerializedLink2,
                         module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered()[2].SerializeAsString());
}

void TopologyModuleTests::testSwitchToSwitchLinkAddedUnegistered() {
    // Add SwitchToSwitchLinks without adding the according Switches before --> unregistered SwitchToSwitchLinks
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(demoSwitchLinks_[2]),
                                              demoModuleUniqueId_);

    int vectorSizeRegisteredCache = module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size();
    CPPUNIT_ASSERT_EQUAL(2, vectorSizeRegisteredCache);
    int vectorSizeUnregisteredCache = module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size();
    CPPUNIT_ASSERT_EQUAL(1, vectorSizeUnregisteredCache);

    std::string exptextedSerializedLink2 = demoSwitchLinks_[2].SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(exptextedSerializedLink2,
                         module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered()[0].SerializeAsString());
}

void TopologyModuleTests::testSwitchToSwitchLinkRemoved() {
    // Add unregistered SwitchToSwitchLinks
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(demoSwitchLinks_[2]),
                                              demoModuleUniqueId_);

    std::string expectedSerializedLink0 = demoSwitchLinks_[0].SerializeAsString();
    std::string expectedSerializedLink2 = demoSwitchLinks_[2].SerializeAsString();

    // Remove one registered link
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventRemoved_ZmfMessage(demoSwitchLinks_[1]),
                                              demoModuleUniqueId_);
    {
        int vectorSizeUnregistered = module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size();
        CPPUNIT_ASSERT_EQUAL(1, vectorSizeUnregistered);
        int vectorSizeRegistered = module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size();
        CPPUNIT_ASSERT_EQUAL(1, vectorSizeRegistered);

        CPPUNIT_ASSERT_EQUAL(expectedSerializedLink2,
                             module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered()[0].SerializeAsString());
        CPPUNIT_ASSERT_EQUAL(expectedSerializedLink0,
                             module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered()[0].SerializeAsString());
    }
    // Remove one unregistered link
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventRemoved_ZmfMessage(demoSwitchLinks_[2]),
                                              demoModuleUniqueId_);
    {
        int vectorSizeUnregistered = module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size();
        CPPUNIT_ASSERT_EQUAL(0, vectorSizeUnregistered);
        int vectorSizeRegistered = module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size();
        CPPUNIT_ASSERT_EQUAL(1, vectorSizeRegistered);
    }

    // Try to remove a link that is not known
    {
        int vectorSizeUnregisteredBefore = module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size();
        int vectorSizeRegisteredBefore = module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size();
        module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventRemoved_ZmfMessage(demoSwitchLinks_[3]),
                                                  demoModuleUniqueId_);
        int vectorSizeUnregisteredAfter = module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size();
        int vectorSizeRegisteredAfter = module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size();
        CPPUNIT_ASSERT_EQUAL(vectorSizeUnregisteredBefore, vectorSizeUnregisteredAfter);
        CPPUNIT_ASSERT_EQUAL(vectorSizeRegisteredBefore, vectorSizeRegisteredAfter);
    }
}

void TopologyModuleTests::testSwitchLinkEventInvalidProtobuf() {
    // SwitchLinkEvent with invalid data --> parseError
    std::string invalidData = "qjweiqohbaosbnfas";

    int switchLinkCacheBefore = module_->UTAccessor_getSwitchesCache().size();
    module_->UTAccessor_handleSwitchLinkEvent(zmf::data::ZmfMessage(topicsSwitchLinkEvent_, invalidData),
                                              demoModuleUniqueId_);

    int switchLinkCacheAfter = module_->UTAccessor_getSwitchesCache().size();
    CPPUNIT_ASSERT_EQUAL(switchLinkCacheBefore, switchLinkCacheAfter);

}

void TopologyModuleTests::testSwitchLinkEventUnknownEvent() {
    // SwitchLinkEvent with unset SwitchLink! --> unknown SwitchEvent
    LinkDiscoveryModule_Proto::From_SwitchLinkEvent* switchLinkEvent = new LinkDiscoveryModule_Proto::From_SwitchLinkEvent();

    LinkDiscoveryModule_Proto::From fromMessage;
    fromMessage.set_allocated_switch_link_event(switchLinkEvent);

    int switchLinkCacheRegBefore = module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size();
    int switchLinkCacheUnregBefore = module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size();
    module_->UTAccessor_handleSwitchLinkEvent(
            zmf::data::ZmfMessage(topicsSwitchLinkEvent_, fromMessage.SerializeAsString()), demoModuleUniqueId_);
    int switchLinkCacheRegAfter = module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size();
    int switchLinkCacheUnregAfter = module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size();

    CPPUNIT_ASSERT_EQUAL(switchLinkCacheRegBefore, switchLinkCacheRegAfter);
    CPPUNIT_ASSERT_EQUAL(switchLinkCacheUnregBefore, switchLinkCacheUnregAfter);
}

void TopologyModuleTests::testSwitchToSwitchLinkFromUnregisteredToRegistered() {
    // Add unregistered SwitchToSwitchLinks
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(demoSwitchLinks_[2]),
                                              demoModuleUniqueId_);

    CPPUNIT_ASSERT_EQUAL(2, (int) module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size());
    CPPUNIT_ASSERT_EQUAL(1, (int) module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size());

    // Add the second Switch (should now be registered because all necessary Switches are registered)
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventAdded_ZmfMessage(demoSwitches_[2]),
                                          demoModuleUniqueId_);
    CPPUNIT_ASSERT_EQUAL(3, (int) module_->UTAccessor_getSwitchToSwitchLinkCacheRegistered().size());
    CPPUNIT_ASSERT_EQUAL(0, (int) module_->UTAccessor_getSwitchToSwitchLinkCacheUnregistered().size());
}

void TopologyModuleTests::testIsSwitchRegisteredCorrectIDIncorrectPort() {
    // Positive case for registered switch
    bool positiveResult = module_->UTAccessor_isSwitchRegistered(demoSwitches_[0].switch_dpid(),
                                                                 demoSwitches_[0].switch_ports(
                                                                         0).attachment_point().switch_port());
    CPPUNIT_ASSERT_EQUAL(true, positiveResult);
    bool negativeResult = module_->UTAccessor_isSwitchRegistered(demoSwitches_[0].switch_dpid(), 9090);
    CPPUNIT_ASSERT_EQUAL(false, negativeResult);
}

void TopologyModuleTests::testHandleRequestGetTopologyRequest() {
    // Add Switches for the SwitchToSwitchLinks
    module_->UTAccessor_handleSwitchEvent(*build_DemoSwitchEventAdded_ZmfMessage(demoSwitches_[2]),
                                          demoModuleUniqueId_);

    // Add SwitchToSwitchLinks between the Switches
    module_->UTAccessor_handleSwitchLinkEvent(*build_DemoSwitchLinkEventAdded_ZmfMessage(demoSwitchLinks_[2]),
                                              demoModuleUniqueId_);

    // Build demo request
    TopologyModule_Proto::Request request;
    TopologyModule_Proto::Request_GetTopologyRequest* getTopoRequest = new TopologyModule_Proto::Request_GetTopologyRequest();
    request.set_allocated_get_topology_request(getTopoRequest);

    std::shared_ptr<zmf::data::ZmfMessage> demoRequest = std::shared_ptr<zmf::data::ZmfMessage>(
            new zmf::data::ZmfMessage(topicsGetTopologyRequest_, request.SerializeAsString()));

    auto zmfOutReply = module_->UTAccessor_handleRequest(*demoRequest.get(), demoModuleUniqueId_);
    TopologyModule_Proto::Reply reply;
    reply.ParseFromArray(zmfOutReply.reply_immediate.getDataRaw(), zmfOutReply.reply_immediate.getDataLength());

    TopologyModule_Proto::Reply_GetTopologyReply topoReply = reply.get_topology_reply();

    std::string topoExpected = module_->UTAccessor_getCurrentStableTopology().SerializeAsString();
    std::string topoActual = topoReply.topology().SerializeAsString();
    CPPUNIT_ASSERT_EQUAL(topoExpected, topoActual);
}

void TopologyModuleTests::testHandleRequestInvalidProtobuf() {
    std::string invalidData = "qjweiqohbaosbnfas";
    zmf::data::ZmfOutReply reply = module_->UTAccessor_handleRequest(
            zmf::data::ZmfMessage(topicsGetTopologyRequest_, invalidData), demoModuleUniqueId_);

    CPPUNIT_ASSERT_EQUAL(zmf::data::ZmfOutReply::NO_REPLY, reply.type);
}

void TopologyModuleTests::testHandleRequestUnknownReqType() {
    TopologyModule_Proto::Request request;

    zmf::data::ZmfOutReply reply = module_->UTAccessor_handleRequest(
            zmf::data::ZmfMessage(topicsSwitchLinkEvent_, request.SerializeAsString()),
            demoModuleUniqueId_);
    CPPUNIT_ASSERT_EQUAL(zmf::data::ZmfOutReply::NO_REPLY, reply.type);
}