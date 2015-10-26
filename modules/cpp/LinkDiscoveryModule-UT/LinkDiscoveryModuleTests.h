//
// Created by Matthias Strljic on 6/25/15.
//

#ifndef DEMO_MODULE_UT_SWITCH_ADAPTER_TESTS_H
#define DEMO_MODULE_UT_SWITCH_ADAPTER_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <zmf/AbstractModule.hpp>
#include <thread>
#include <map>
#include <LinkDiscoveryModule.hpp>
#include <zsdn/topics/SwitchRegistryModule_topics.hpp>
#include <zsdn/topics/LinkDiscoveryModule_topics.hpp>
#include <zsdn/proto/LinkDiscoveryModule.pb.h>
#include "zsdn/topics/SwitchAdapter_topics.hpp"
#include <zmf/IZmfInstanceController.hpp>

using namespace CppUnit;

/**
 * @details Unit-Test class which contains all needed state behavior elements and function tests to ensure correct
 * behavior of the LinkDiscoveryModule.
 * Each test follows a similar sequence of action.
 * 1) Setup the state of the test environment (Setup)
 * 2) Start the LinkDiscoveryModule
 * 3) Make some changes to the environment
 * 4) Request the state of the LinkDiscoveryModule
 * 5) Validate the result
 * 6) Clean up the test environment (TearDown)
 *
 * @author Matthias Strljic
 */
class LinkDiscoveryModuleTests : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(LinkDiscoveryModuleTests);
        /*
         * Setup the Test cases for CPPUNIT
         */
        CPPUNIT_TEST(testRequestNormalGetAllLinks);
        CPPUNIT_TEST(testRequestNormalGetAllLinksPartialEvent);
        CPPUNIT_TEST(testRequestNormalGetAllLinksFullEvent);
        CPPUNIT_TEST(testRequestNormalWithChangesGetAllLinks);
        CPPUNIT_TEST(testRequestCorruptedProtoGetAllLinks);

        CPPUNIT_TEST(testRequestNormalGetLinksFromSwitch);
        CPPUNIT_TEST(testRequestNormalGetLinksFromSwitchPartialEvent);
        CPPUNIT_TEST(testRequestNormalGetLinksFromSwitchFullEvent);
        CPPUNIT_TEST(testRequestNormalWithChangesGetLinksFromSwitch);
        CPPUNIT_TEST(testRequestCorrptedGetLinksFromSwitch);
        CPPUNIT_TEST(testRequestWrongIdGetLinkgsFromSwitch);

        CPPUNIT_TEST(testRequestNormalGetLinksToSwitch);
        CPPUNIT_TEST(testRequestNormalGetLinksToSwitchPartialEvent);
        CPPUNIT_TEST(testRequestNormalGetLinksToSwitchFullEvent);
        CPPUNIT_TEST(testRequestNormalWithChangesGetLinksToSwitch);
        CPPUNIT_TEST(testRequestCorrptedGetLinksToSwitch);
        CPPUNIT_TEST(testRequestWrongIdGetLinksToSwitch);

        CPPUNIT_TEST(testRequestNormalAllLinksOfSwitch);
        CPPUNIT_TEST(testRequestNormalAllLinksOfSwitchPartialEvent);
        CPPUNIT_TEST(testRequestNormalAllLinksOfSwitchFullEvent);
        CPPUNIT_TEST(testRequestNormalWithChangesAllLinksOfSwitch);
        CPPUNIT_TEST(testRequestCorrptedAllLinksOfSwitch);
        CPPUNIT_TEST(testRequestWrongIdAllLinksOfSwitch);

        CPPUNIT_TEST(testRequestNormalLinksBetweenTwoSwitches);
        CPPUNIT_TEST(testRequestNormalLinksBetweenTwoSwitchesPartialEvent);
        CPPUNIT_TEST(testRequestNormalLinksBetweenTwoSwitchesFullEvent);
        CPPUNIT_TEST(testRequestNormalWithChangesLinksBetweenTwoSwitches);
        CPPUNIT_TEST(testRequestCorrptedLinksBetweenTwoSwitches);
        CPPUNIT_TEST(testRequestWrongIdLinksBetweenTwoSwitches);

        /*
         *
         */

    CPPUNIT_TEST_SUITE_END();

public:
    LinkDiscoveryModuleTests();

    /**
     * Setup at the beginning of each single unit test.
     * This will start all simulated switch adapters, switch registry, the tester and linkdiscovery modules.
     */
    void setUp();

    /**
     * Releases all resources of the test at it's end, to get a clean setup at the next test case.
     * This method will take some time, cause it will waite until all modules are disabled.
     */
    void tearDown();

    /**
     * Encapsulated setup method to initialize the switches and the registry at the beginning of a test,
     * so that the registry will hold all available switches for the test at it's beginning.
     * This is used to get scenario in which all switches are known at the "GetAllSwitches" request from the
     * LinkDiscoveryModule to the SwitchRegistryModule instance.
     */
    void fullSetupRegistryStart();

    /**
     * Encapsulated setup method to initialize the switches and the registry at the beginning of a test,
     * so that the registry will hold half of the available switches for the test at it's beginning and will
     * publish the other half after it receives the "GetAllSwitches" request from the LinkDiscoveryModule.
     * This is used to get scenario in which a part of the switches are not known at the "GetAllSwitches" request from the
     * LinkDiscoveryModule to the SwitchRegistryModule instance.
     */
    void partialSetupRegistryStart();

    /**
     * Encapsulated setup method like the fullSetupRegistryStart method with a known set of 0.
     */
    void eventOnlySetupRegistryStart();
    /**
     * Evaluate if the to given map contains the link described by the 4 params source, sPort, endpoint, ePort
     * @param toValid reference to a map which contains a representation of links between switches
     * @param source ID of the switch at the beginning of a link.
     * @param sPort the port of the source switch with which it is connected to the link.
     * @param endpoint ID of the switch ath the end of a link
     * @param ePort the port of the endpoint switch with which it is conntected to the link.
     */
    bool containsMapLink(std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>>& toValid,
                         uint64_t source, uint32_t sPort, uint64_t endpoint, uint32_t ePort);

    /**
     * Evalutaion method to check if  the toValid link map contains all existing links from the source switch with the
     * given switch ID.
     * @param toValid link map which should be evaluated.
     * @param source ID of the source switch.
     */
    bool validateContainsAllFrom(std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> toValid,
                                 std::uint64_t source);
    /**
     * Evalutaion method to check if  the toValid link map contains all existing links to the endpoint switch with the
     * given switch ID.
     * @param toValid link map which should be evaluated.
     * @param endpoint ID of the source switch.
     */
    bool validateContainsAllTo(std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> toValid,
                               std::uint64_t endpoint);

    /**
     * Evalutaion method to check if  the toValid link map contains all existing links between a source and a endpoint
     * switch.
     * @param toValid link map which should be evaluated.
     * @param source ID of the source switch.
     * @param endpoint ID of the source switch.
     */
    bool validateContainsAllBetwenn(std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> toValid,
                                    std::uint64_t source, std::uint64_t endpoint);

//############################################TestMethodDeclaration#####################################################
    /**
     * Normal request scenario to get all existing links, with full setup.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> start TestModule -> waite -> request all Links -> compare result -> tearDown
     */
    void testRequestNormalGetAllLinks();

    /**
     * Normal request scenario to get all existing links, with partial setup.
     * Sequence:
     * PartialSetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> request all Links -> compare result -> tearDown
     */
    void testRequestNormalGetAllLinksPartialEvent();

    /**
     * Normal request scenario to get all existing links, with event only setup.
     * Sequence:
     * EventOnlySetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> request all Links -> compare result -> tearDown
     */
    void testRequestNormalGetAllLinksFullEvent();

    /**
     * Normal request scenario to get all existing links, with full setup with additional remove and add switch events.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> waite -> start TestModule -> waite
     * -> remove switch with links -> add switch with links -> waite -> request all Links -> compare result -> tearDown
     */
    void testRequestNormalWithChangesGetAllLinks();

    /**
     * Send corrupted / invalid message to test the stability of the GetAllLinks request logic.
     */
    void testRequestCorruptedProtoGetAllLinks();

    /**
     * Normal request scenario to get all existing links from a particular switch, with full setup.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> start TestModule -> waite -> request all from links -> compare result -> tearDown
     */
    void testRequestNormalGetLinksFromSwitch();

    /**
     * Normal request scenario to get all existing links from a particular switch, with partial setup.
     * Sequence:
     * PartialSetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> request all from links -> compare result -> tearDown
     */
    void testRequestNormalGetLinksFromSwitchPartialEvent();

    /**
     * Normal request scenario to get all existing links from a particular switch, with event only setup.
     * Sequence:
     * EventOnlySetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> request all with links -> compare result -> tearDown
     */
    void testRequestNormalGetLinksFromSwitchFullEvent();

    /**
       * Normal request scenario to get all existing links from a particular switch,
       * with full setup with additional remove and add switch events to test the correct behavior belong to these changes.
       * Sequence:
       * FullSetup -> start LinkDiscovery -> waite -> start TestModule -> waite
       * -> remove switch with links -> add switch with links -> waite -> request all from links -> compare result -> tearDown
       */
    void testRequestNormalWithChangesGetLinksFromSwitch();

    /**
     * Send corrupted / invalid message to test the stability of the GetAllLinksFromSwitch request logic.
     */
    void testRequestCorrptedGetLinksFromSwitch();

    /**
     * Tests the simple scenario of requesting from links of a non existing switch.
     */
    void testRequestWrongIdGetLinkgsFromSwitch();

    /**
     * Normal request scenario to get all existing links to a particular switch, with full setup.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> start TestModule -> waite -> request all links to switch -> compare result -> tearDown
     */
    void testRequestNormalGetLinksToSwitch();

    /**
     * Normal request scenario to get all existing links to a particular switch, with partial setup.
     * Sequence:
     * PartialSetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> request all links to switch -> compare result -> tearDown
     */
    void testRequestNormalGetLinksToSwitchPartialEvent();

    /**
     * Normal request scenario to get all existing links to a particular switch, with event only setup.
     * Sequence:
     * EventOnlySetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> request all links to switch -> compare result -> tearDown
     */
    void testRequestNormalGetLinksToSwitchFullEvent();

    /**
     * Normal request scenario to get all existing links to a particular switch,
     * with full setup with additional remove and add switch events to test the correct behavior belong to these changes.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> waite -> start TestModule -> waite
     * -> remove switch with links -> add switch with links -> waite -> request all links to switch -> compare result -> tearDown
     */
    void testRequestNormalWithChangesGetLinksToSwitch();

    /**
     * Send corrupted / invalid message to test the stability of the GetAllLinksToSwitch request logic.
     */
    void testRequestCorrptedGetLinksToSwitch();

    /**
     * Tests the simple scenario of requesting "to" links of a non existing switch.
     */
    void testRequestWrongIdGetLinksToSwitch();

    /**
     * Normal request scenario to get all existing links of a particular switch, with full setup.
     * This inlcudes all link "from" and "to" a switch.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> start TestModule -> waite -> request all links of switch -> compare result -> tearDown
     */
    void testRequestNormalAllLinksOfSwitch();

    /**
     * Normal request scenario to get all existing links of a particular switch, with partial setup.
     * This inlcudes all link "from" and "to" a switch.
     * Sequence:
     * PartialSetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> request all links of switch -> compare result -> tearDown
     */
    void testRequestNormalAllLinksOfSwitchPartialEvent();

    /**
     * Normal request scenario to get all existing links of a particular switch, with event only setup.
     * This inlcudes all link "from" and "to" a switch.
     * Sequence:
     * EventOnlySetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> request all links of switch -> compare result -> tearDown
     */
    void testRequestNormalAllLinksOfSwitchFullEvent();

    /**
     * Normal request scenario to get all existing links of a particular switch,
     * with full setup with additional remove and add switch events to test the correct behavior belong to these changes.
     * This inlcudes all link "from" and "to" a switch.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> waite -> start TestModule -> waite
     * -> remove switch with links -> add switch with links -> waite -> request all links of switch -> compare result -> tearDown
     */
    void testRequestNormalWithChangesAllLinksOfSwitch();

    /**
     * Send corrupted / invalid message to test the stability of the GetAllLinksOfSwitch request logic.
     */
    void testRequestCorrptedAllLinksOfSwitch();

    /**
     * Tests the simple scenario of requesting "of" links of a non existing switch.
     * This inlcudes all link "from" and "to" a switch.
     */
    void testRequestWrongIdAllLinksOfSwitch();

    /**
     * Normal request scenario to get all existing links between two switches, with full setup.
     * This case will loop over all posible combinations to find false-positives and false-negative links.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> start TestModule -> waite -> LOOP[ALL_SWITCH_COMBINATIONS]{ request all links between two switches -> compare result } -> tearDown
     */
    void testRequestNormalLinksBetweenTwoSwitches();

    /**
     * Normal request scenario to get all existing links between two switches, with partial setup.
     * This case will loop over all posible combinations to find false-positives and false-negative links.
     * Sequence:
     * PartialSetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> LOOP[ALL_SWITCH_COMBINATIONS]{ request all links between two switches -> compare result }  -> tearDown
     */
    void testRequestNormalLinksBetweenTwoSwitchesPartialEvent();

    /**
     * Normal request scenario to get all existing links between two switches, with event only setup.
     * This case will loop over all posible combinations to find false-positives and false-negative links.
     * Sequence:
     * EventOnlySetup -> start LinkDiscovery -> waite -> start TestModule -> waite -> LOOP[ALL_SWITCH_COMBINATIONS]{ request all links between two switches -> compare result } -> tearDown
     */
    void testRequestNormalLinksBetweenTwoSwitchesFullEvent();

    /**
     * Normal request scenario to get all existing links between two switches,
     * with full setup with additional remove and add switch events to test the correct behavior belong to these changes.
     * This case will loop over all posible combinations to find false-positives and false-negative links.
     * Sequence:
     * FullSetup -> start LinkDiscovery -> waite -> start TestModule -> waite
     * -> remove switch with links -> add switch with links -> waite -> LOOP[ALL_SWITCH_COMBINATIONS]{ request all links between two switches -> compare result } -> tearDown
     */
    void testRequestNormalWithChangesLinksBetweenTwoSwitches();

    /**
     * Send corrupted / invalid message to test the stability of the GetAllLinksBetweenSwitches request logic.
     */
    void testRequestCorrptedLinksBetweenTwoSwitches();

    /**
     * Tests the simple scenario of requestin links between two switches in 3 cases.
     * 1) source switch does not exist
     * 2) endpoint switch does not exist
     * 3) both switches does not exist
     */
    void testRequestWrongIdLinksBetweenTwoSwitches();

};

/**
 * @details A emulation of a SwitchRegistryModule to controle the tests. This module controles the set of available
 * switches for the to testing LinkDiscoveryModule. The Module is has initialize modes. These modes are needed to construct
 * different use modes of the LinkDiscoveryModule
 * Modes:
 * FULL_START(default): returns all switches at the GetAllSwitches request
 * PARTIAL_REACT_START: returns a part of all switches and starts to publish the rest of the complete set as events.
 * CONTINUES_EVENT_START: returns a empty set of switches and publishes all switches after the request.
 * @author Matthias Strljic
 */
class SwitchRegistryModuleMok : public zmf::AbstractModule {

public:
    //Constructors
    SwitchRegistryModuleMok(uint64_t instanceId);

    ~SwitchRegistryModuleMok();

    virtual zmf::data::ZmfOutReply handleRequest(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender);

    enum ReactMode {
        UNSET, FULL_START, PARTIAL_REACT_START, CONTINUES_EVENT_START
    };

protected:
    virtual bool enable();

public:
    virtual void disable();

    virtual bool isSetup();

    virtual void startSetup(ReactMode mode);

    virtual zmf::data::ZmfOutReply generateReplyMessageForSwitchesUpTo(uint32_t c);

    virtual void startChanges();

    virtual void startEventFinishFrom(uint32_t s);

private:
    static const uint16_t MODULE_VERSION = 0;
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_SwitchRegistryModule;
    uint64_t instanceId_;
    ReactMode mode = ReactMode::UNSET;
    bool isSetup_ = false;
    std::thread* backgroundThread_ = nullptr;
    std::thread* changerThread_ = nullptr;

    zmf::data::MessageType topicsSwitchRemoved_ = switchregistrymodule_topics::FROM().switch_registry_module().switch_event().removed().build();
    zmf::data::MessageType topicsGetAllSwitchesReply_ = switchregistrymodule_topics::REPLY().switch_registry_module().get_all_switches().build();
    zmf::data::MessageType topicsSwitchAdded_ = switchregistrymodule_topics::FROM().switch_registry_module().switch_event().added().build();

};

/**
 * @details A emulated SwitchAdapterModule with the simple action to lead the LinkDiscoveryModule discover messages
 * to the correlating switch (if exists) so that this emulated one could send it back as event to the ZMF-Network
 * with the correct data like Port, instance ID.
 * @author Matthias Strljic
 */
class SwitchAdapterModuleMok : public zmf::AbstractModule {

public:
    //Constructors
    SwitchAdapterModuleMok(uint64_t instanceId);

    ~SwitchAdapterModuleMok();

    virtual void sendMessageWithData(uint32_t port, uint64_t hash, uint64_t timestamp);

protected:
    virtual bool enable();

public:
    virtual void disable();

    virtual void setProcessMessages(bool pM);


private:
    static const uint16_t MODULE_VERSION = 0;
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_SwitchAdapter;
    uint64_t instanceId_;
    bool processMessages = true;

};


//######################################################################################################################
/**
 * @details The Accesor object for the test cases to send messages from outside into the ZMF-Network and receive the
 * responses.
 *
 * @author Matthias Strljic
 */
class TesterObject : public zmf::AbstractModule {

public:
    static const uint16_t LINK_MODULE_DEP_TYPE = zsdn::MODULE_TYPE_ID_LinkDiscoveryModule;
    static const uint16_t LINK_MODULE_DEP_VERSION = 0;

    //Constructors
    TesterObject(uint64_t instanceId);

    ~TesterObject();

protected:
    virtual bool enable();

public:
    virtual void disable();

    /**
     * Sends a GetAllLinks request to the LinkDiscoveryModule.
     * @return Map that represents the uni directional links.
     */
    virtual std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> requestAllLinks();

    /**
     * Sends a GetAllLinksOfSwitch request to the LinkDiscoveryModule.
     * @return Map that represents the uni directional links.
     */
    virtual std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> requestOfLinks(uint64_t ofId);

    /**
     * Sends a GetAllLinksFromSwitch request to the LinkDiscoveryModule.
     * That includes all links with the given switch as source of a link.
     * @param fromid the id of the source switch.
     * @return Map that represents the uni directional links.
     */
    virtual std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> requestFromLinks(
            uint64_t fromid);

    /**
     * Sends a GetAllLinksToSwitch request to the LinkDiscoveryModule.
     * That includes all links with the given switch as endpoint of a link.
     * @param toId the id of the endpoint switch.
     * @return Map that represents the uni directional links.
     */
    virtual std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> requestToLinks(uint64_t toId);

    /**
     * Sends a GetAllLinksBetween request to the LinkDiscoveryModule.
     * @param aId id of first switch of a link.
     * @param bId id of the second switch of a link.
     * @return Map that represents the uni directional links.
     */
    virtual std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> requestBetweenLinks(uint64_t aId,
                                                                                                           uint64_t bId);

    virtual bool requestCorruptedForTopic(zmf::data::MessageType type, std::string message);

private:
    static const uint16_t MODULE_VERSION = 0;
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_DemoModule;
    uint64_t instanceId_;

};


#endif //DEMO_MODULE_UT_SWITCH_ADAPTER_H
