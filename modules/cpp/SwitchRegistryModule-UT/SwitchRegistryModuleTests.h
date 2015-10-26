//
// Created by zsdn on 6/25/15.
//

#ifndef DEMO_MODULE_UT_SWITCH_ADAPTER_TESTS_H
#define DEMO_MODULE_UT_SWITCH_ADAPTER_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <zmf/MessageType.hpp>
#include <zmf/ZmfMessage.hpp>
#include <zsdn/topics/SwitchRegistryModule_topics.hpp>
#include <dummyModules/DummyModule.hpp>
#include <zmf/IZmfInstanceController.hpp>
#include "SwitchRegistryModule.hpp"

using namespace CppUnit;

/**
 * @brief This class contains and runs Unit-Tests for SwitchRegistryModule.
 *
 * @details The class contains Unit-Tests for SwitchRegistryModule.
 * Each test starts by setting up an environment for the test, with the startup()-method.
 * Then the test itself is run, and it is evaluated, whether the test is successful.
 * At last the test-environment is cleaned up again.
 * If not stated different, the Switch-Version should be OpenFlow 1.3
 *
 * @author Sebastian Vogel
 */
class SwitchRegistryModuleTests : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(SwitchRegistryModuleTests);
        CPPUNIT_TEST(testRequestAllSwitches);
        CPPUNIT_TEST(testRequestSwitchByID);
        CPPUNIT_TEST(testRequestUnknownRequest);
        CPPUNIT_TEST(testRequestTrollRequest);
        CPPUNIT_TEST(testFeatureReplyUnknownSwitch);
        CPPUNIT_TEST(testFeatureReplyKnownSwitchNoBecomeActive);
        CPPUNIT_TEST(testFeatureReplyKnownSwitchBecomeActive);
        CPPUNIT_TEST(testFeatureReplyKnownSwitchActiveToBeginWith);
        CPPUNIT_TEST(testFeatureReplyKnownSwitchOf1_0NotActive);
        CPPUNIT_TEST(testEchoKnownSwitch);
        CPPUNIT_TEST(testEchoUnknownSwitch);
        CPPUNIT_TEST(testEchoKnownActiveSwitch);
        CPPUNIT_TEST(testPortStatusUnknownSwitch);
        CPPUNIT_TEST(testPortStatusKnownSwitchReasonPortAdded);
        CPPUNIT_TEST(testPortStatusKnownSwitchReasonPortDeleted);
        CPPUNIT_TEST(testPortStatusKnownSwitchReasonPortModified);
        CPPUNIT_TEST(testPortStatusKnownSwitchReasonInvalid);
        CPPUNIT_TEST(testMultiPartAnyUnknownSwitch);
        CPPUNIT_TEST(testMultiPartPortDescReplyKnownSwitchNoBecomeActive);
        CPPUNIT_TEST(testMultiPartPortDescReplyKnownSwitchBecomeActive);
        CPPUNIT_TEST(testMultiPartPortDescReplyKnownSwitchActiveToBeginWith);
        CPPUNIT_TEST(testMultiPartPortNoDescReply);
        CPPUNIT_TEST(testModuleStateChangeNoSwitchAdapter);
        CPPUNIT_TEST(testModuleStateChangeAddNew1_3SwitchAdapterAndDeleteNoActiveOne);
        CPPUNIT_TEST(testModuleStateChangeAddKnownSwitchAdapterAndDeleteActiveOne);
        CPPUNIT_TEST(testModuleStateChangeAddNew1_0SwitchAdapter);
        CPPUNIT_TEST(testDisableAndEnableModuleWithValidSwitchInZMF);
        CPPUNIT_TEST(testDisableAndEnableModuleWithInvalidSwitchInZMF);
    CPPUNIT_TEST_SUITE_END();

    typedef std::map<uint64_t, Switch> SwitchMap;
    typedef std::pair<uint64_t, Switch> SwitchMapPair;
    typedef std::map<uint64_t, Switch>::iterator SwitchMapIter;

public:
    /**
     * Constructor for the Class
     */
    SwitchRegistryModuleTests();

    /**
     * This test sends an All-Switches-Request to the module per Request-Reply. Answer should be all added Switches.
     */
    void testRequestAllSwitches();

    /**
     * This test sends a Switch-Request to the module per Request-Reply. Answer should be the Switch with the requested ID.
     */
    void testRequestSwitchByID();

    /**
     * This test sends a valid Request of unkonwn type to the Switch. Answer should be a NO-REPLY.
     */
    void testRequestUnknownRequest();

    /**
     * This test sends a serialized String to the Switch, which should not be parseable to a request. Answer should be NO-REPLY.
     */
    void testRequestTrollRequest();

    /**
     * This test emulates a Feature-Reply from an unknown switch. Result should be nothing happens.
     */
    void testFeatureReplyUnknownSwitch();

    /**
     * This test emulates a Feature-Reply From a known, as inactive marked Switch. Result should be Information is added, but no messages sent.
     */
    void testFeatureReplyKnownSwitchNoBecomeActive();

    /**
    * This test emulates a Feature-Reply From a known, as inactive marked Switch. Result should be Information is added and a ADDED-message is sent.
    */
    void testFeatureReplyKnownSwitchBecomeActive();

    /**
    * This test emulates a Feature-Reply From a known, as active marked Switch. Result should be Information is updated and a CHANGED-message is sent.
    */
    void testFeatureReplyKnownSwitchActiveToBeginWith();

    /**
    * This test emulates a Feature-Reply From a known, as inactive marked Switch with OF-Version 1.0. Result should be Information is added and a ADDED-messages is sent.
    */
    void testFeatureReplyKnownSwitchOf1_0NotActive();

    /**
    * This test emulates a Echo-Request from a known, but as inactive marked switch. Result should be Feature-Request and Multipart-Request are sent.
    */
    void testEchoKnownSwitch();

    /**
    * This test emulates a Echo-Request from an unknown switch. Result should be Switch is added and Feature-Request and Multipart-Request are sent.
    */
    void testEchoUnknownSwitch();

    /**
    * This test emulates a Echo-Request from a known and active switch. Result should be nothing happens.
    */
    void testEchoKnownActiveSwitch();

    /**
    * This test emulates a Port-Status-message from an unknown switch. Result should be nothing happens.
    */
    void testPortStatusUnknownSwitch();

    /**
    * This test emulates a Port-Status-message from a known switch with a new port added. Result should be the new port is added to Switch-Info and CHANGED-messge is sent.
    */
    void testPortStatusKnownSwitchReasonPortAdded();

    /**
    * This test emulates a Port-Status-message from a known switch with a deleted port. Result should be the port is deleted from Switch-Info and CHANGED-messge is sent.
    */
    void testPortStatusKnownSwitchReasonPortDeleted();

    /**
    * This test emulates a Port-Status-message from a known switch with a modified port. Result should be the portinfo is updated and CHANGED-messge is sent.
    */
    void testPortStatusKnownSwitchReasonPortModified();

    /**
    * This test emulates a Port-Status-message from a known switch with a invalid reason. Result should be nothing happens.
    */
    void testPortStatusKnownSwitchReasonInvalid();

    /**
     * This test emulates a Multipart-Reply from an unknown Switch. Result should be nothing happens.
     */
    void testMultiPartAnyUnknownSwitch();

    /**
     * This test emulates a Port-Desc-Multipart-Reply from a known, as inactive marked Switch. Result should be Information is added, but no messages sent.
     */
    void testMultiPartPortDescReplyKnownSwitchNoBecomeActive();

    /**
     * This test emulates a Port-Desc-Multipart-Reply from a known, as inactive marked Switch. Result should be Information is added and a ADDED-message is sent.
     */
    void testMultiPartPortDescReplyKnownSwitchBecomeActive();

    /**
     * This test emulates a Port-Desc-Multipart-Reply from a known, as active marked Switch. Result should be Information is updated and a CHANGED-message is sent.
     */
    void testMultiPartPortDescReplyKnownSwitchActiveToBeginWith();

    /**
     * This test emulates a Multipart-Reply of a different type from a known, as active marked Switch. Result should be nothing happens.
     */
    void testMultiPartPortNoDescReply();

    /**
     * This test emulates a non-SwitchAdapterModule to become active. Result should be nothing happens.
     */
    void testModuleStateChangeNoSwitchAdapter();

    /**
     * This test emulates a SwitchAdapterModule with OF 1.3 to become active and an as inactive marked SwitchAdapter to become dead.
     * Result should be 1 Switch added and Multipart/Feature-Request sent for first part and 1 Switch removed for 2nd part.
     */
    void testModuleStateChangeAddNew1_3SwitchAdapterAndDeleteNoActiveOne();

    /**
     * This test emulates a known SwitchAdapter become active and an as active marked SwitchAdapter to become dead.
     * Result should be nothing happens for the first part and remove 1 Switch and REMOVED-message sent for 2nd part.
     */
    void testModuleStateChangeAddKnownSwitchAdapterAndDeleteActiveOne();

    /**
     * This test emulates a SwitchAdapterModule with OF 1.0 to become active.
     * Result should be 1 Switch added and Feature-Request sent.
     */
    void testModuleStateChangeAddNew1_0SwitchAdapter();

    /**
     * This test emulates a disable() and enable() of the SwitchRegistryModule with one valid Switch in ZMF.
     * Result should be after disable() Swich-list is empty, after enable() 1 Switch added to list and feature/multipart-request sent.
     */
    void testDisableAndEnableModuleWithValidSwitchInZMF();

    /**
    * This test emulates a disable() and enable() of the SwitchRegistryModule with one invalid Switch in ZMF.
     * Result should be after disable() Switch-list is empty, after enable() nothing happens
    */
    void testDisableAndEnableModuleWithInvalidSwitchInZMF();

    /**
     * This method sets the environment for tests up. It initializes the module and adds some dummy-Switches to test with.
     * Additionally there will be a second ZMF-Instance initialized, that listens to messages from our module, to see wether messages are actually sent.
     */
    void setUp();

    /**
     * This method shuts down and cleans the environment for each Unit-Test. It is called after each test is done.
     */
    void tearDown();

private:
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstance_;
    std::shared_ptr<SwitchRegistryModule> module_;
    //dummy module and its counters
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstanceDummy;
    std::shared_ptr<DummyModule> dummy;
    int counter_newSwitch;
    int counter_deletedSwitch;
    int counter_changedSwitch;
    int counter_featureRequest;
    int counter_portDescRequest;

    std::vector<Switch> switchVector;

    //topics requests
    zmf::data::MessageType topicAllSwitches_ = switchregistrymodule_topics::REQUEST().switch_registry_module().get_all_switches().build();
    zmf::data::MessageType topicSwitch_ = switchregistrymodule_topics::REQUEST().switch_registry_module().get_switch_by_id().build();
    //topics from Switch
    zmf::data::MessageType topicsMultipartReply_ = switchadapter_topics::FROM().switch_adapter().openflow().of_1_0_barrier_reply_of_1_3_multipart_reply().build();
    zmf::data::MessageType topicsFeatureReply_ = switchadapter_topics::FROM().switch_adapter().openflow().features_reply().build();
    zmf::data::MessageType topicsEchoRequest_ = switchadapter_topics::FROM().switch_adapter().openflow().echo_request().build();
    zmf::data::MessageType topicsPortStatus_ = switchadapter_topics::FROM().switch_adapter().openflow().port_status().build();

    std::shared_ptr<zmf::data::ZmfMessage> build_DemoAllSwitchRequest_ZmfMessage();

    std::shared_ptr<zmf::data::ZmfMessage> build_DemoSwitchRequest_ZmfMessage(uint64_t switchID);

    std::shared_ptr<zmf::data::ZmfMessage> build_DemoMultipartReply(Switch aSwitch);

    std::shared_ptr<zmf::data::ZmfMessage> build_DemoFeatureReply(Switch aSwitch);

    std::shared_ptr<zmf::data::ZmfMessage> build_DemoPortStatus(uint64_t switchID, of_version_t of_version, Port port,
                                                                uint8_t reason);
    /**
     * compares Info of 2 Switches
     * @return true if equal, else false
     */
    bool hasSameSwitchInfo(Switch aSwitch1, Switch aSwitch2);
    /**
     * compares Ports of 2 Switches
     * @return true if equal, else false
     */
    bool hasSamePorts(Switch aSwitch1, Switch aSwitch2);
    /**
     * compares expected to actual messagecounter-values
     * @return true if equal, else false
     */
    bool compareMessageCounters(int expected_deletedSwitch, int expected_newSwitch, int expected_changedSwitch,
                                int expected_featureRequest, int expected_portDescRequest);

    /**
     * cast Topology:Switch to the SwitchRegistry Switch-dataclass
     * @return a Switch Dataclass
     */
    Switch protoToSwitch(common::topology::Switch aSwitch);

};

#endif //DEMO_MODULE_UT_SWITCH_ADAPTER_H
