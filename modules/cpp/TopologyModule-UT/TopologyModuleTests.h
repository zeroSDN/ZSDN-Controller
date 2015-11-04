#ifndef DEMO_MODULE_UT_SWITCH_ADAPTER_TESTS_H
#define DEMO_MODULE_UT_SWITCH_ADAPTER_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <zmf/ZmfMessage.hpp>
#include <TopologyModule.hpp>
#include <zmf/IZmfInstanceController.hpp>

using namespace CppUnit;

/**
 * @details Functional Tests (Unit-Tests) for the TopologyModule implementation.
 * @author Tobias Freundorfer
 */
class TopologyModuleTests : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(TopologyModuleTests);
        // methods for handleSwitchEvent()
        CPPUNIT_TEST(testSwitchEventAdded);
        CPPUNIT_TEST(testSwitchEventChanged);
        CPPUNIT_TEST(testSwitchEventChangedUnknownSwitch);
        CPPUNIT_TEST(testSwitchEventRemoved);
        CPPUNIT_TEST(testSwitchEventRemovedUnknownSwitch);
        CPPUNIT_TEST(testSwitchEventInvalidProtobuf);
        CPPUNIT_TEST(testSwitchEventUnknownEvent);

        // methods for handleSwitchLinkEvent()
        CPPUNIT_TEST(testSwitchToSwitchLinkAddedRegistered);
        CPPUNIT_TEST(testSwitchToSwitchLinkAddedUnegistered);
        CPPUNIT_TEST(testSwitchToSwitchLinkRemoved);
        CPPUNIT_TEST(testSwitchLinkEventInvalidProtobuf);
        CPPUNIT_TEST(testSwitchLinkEventUnknownEvent);

        // methods for special cases
        CPPUNIT_TEST(testSwitchToSwitchLinkFromUnregisteredToRegistered);
        CPPUNIT_TEST(testIsSwitchRegisteredCorrectIDIncorrectPort);

        // methods for handleRequest()
        CPPUNIT_TEST(testHandleRequestGetTopologyRequest);
        CPPUNIT_TEST(testHandleRequestInvalidProtobuf);
        CPPUNIT_TEST(testHandleRequestUnknownReqType);
    CPPUNIT_TEST_SUITE_END();

public:
    TopologyModuleTests();

    /*
     * Override setup method to set up the testing objects.
     */
    void setUp();

    /*
     * Override tearDown method to release any permanent ressources.
     */
    void tearDown();

    /**
     * A Test for the SwitchEvent Added from the SwitchRegistryModule.
     * Tests the adding for a new Switch published by the mentioned event.
     */
    void testSwitchEventAdded();

    /**
     * A Test for the SwitchEvent Changed from the SwitchRegistryModule.
     * Tests the changing for a Switch published by the mentioned event.
     */
    void testSwitchEventChanged();

    /**
     * A Test for the SwitchEvent Changed from the SwitchRegistryModule.
     * Tests if an update for a yet unknown Switch in published, a new switch shoud be added.
     */
    void testSwitchEventChangedUnknownSwitch();

    /**
     * A Test for the SwitchEvent Removed from the LinkDiscoveryModule.
     * Tests the removing of a Switch published by the mentioned event.
     */
    void testSwitchEventRemoved();

    /**
     * A Test for the SwitchEvent Removed from the LinkDiscoveryModule.
     * Tests if a remove for a yet unknown Switch is published.
     */
    void testSwitchEventRemovedUnknownSwitch();

    /**
     * A test for the SwitchEvent.
     * Tests the handling for SwitchEvents with an invalid ProtoBuffer format.
     */
    void testSwitchEventInvalidProtobuf();

    /**
    * A test for the TopologyModule method handleSwitchEvent().
    * Tests the handling for SwitchEvents with an unknwon Request type.
    */
    void testSwitchEventUnknownEvent();

    /**
     * A Test for the SwitchLinkEvent Added from the LinkDiscoveryModule for already registered Switches.
     * Tests the adding for a new SwitchToSwitchLink published by the mentioned event.
     */
    void testSwitchToSwitchLinkAddedRegistered();

    /**
     * A Test for the SwitchLinkEvent Added from the LinkDiscoveryModule for unregistered Switches.
     * Tests the adding for a new SwitchToSwitchLink published by the mentioned event.
     */
    void testSwitchToSwitchLinkAddedUnegistered();

    /**
     * A Test for the SwitchEvent Added where a primarily unregistered SwitchToSwitchLink gets set to registered because the
     * according Switch was added.
     */
    void testSwitchToSwitchLinkFromUnregisteredToRegistered();

    /**
     * A Test for the TopologyModule method isSwitchRegistered().
     * Input is a valid switchID but with an invalid/unknown port.
     */
    void testIsSwitchRegisteredCorrectIDIncorrectPort();

    /**
     * A Test for the SwitchLinkEvent Removed from the LinkDiscoveryModule.
     * Tests the removing of a SwitchToSwitchLink published by the mentioned event.
     */
    void testSwitchToSwitchLinkRemoved();

    /**
     * A test for the TopologyModule method handleSwitchLinkEvent().
     * Tests the handling for SwitchLinkEvents with an invalid ProtoBuffer format.
     */
    void testSwitchLinkEventInvalidProtobuf();

    /**
     * A test for the TopologyModule method handleSwitchLinkEvent().
     * Tests the handling for SwitchLinkEvents with an unknwon type.
     */
    void testSwitchLinkEventUnknownEvent();

    /**
     * A test for the handleRequest method.
     */
    void testHandleRequestGetTopologyRequest();

    /**
     * A test for the TopologyModule method handleRequest().
     * Tests the handling for requests with an invalid ProtoBuffer format.
     */
    void testHandleRequestInvalidProtobuf();

    /**
     * A test for the TopologyModule method handleRequest().
     * Tests the handling for request with an unknwon Request type.
     */
    void testHandleRequestUnknownReqType();

private:
    /// The ZmfInstance that holds the Module under test.
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstance_;

    /// The ZmfInstance that holds the SwitchRegistryModule.
    std::shared_ptr<zmf::IZmfInstanceController> switchRegistryModuleZmfInstance_;

    /// The ZmfInstance that holds the LinkDiscoveryModule.
    std::shared_ptr<zmf::IZmfInstanceController> linkDiscoveryModuleZmfInstance_;

    /// The TopologyModule under test.
    std::shared_ptr<TopologyModule> module_;

    /// The SwitchRegistryModule Mock
    std::shared_ptr<zmf::AbstractModule> switchRegistryModuleMock_;

    /// The LinkDiscoveryModule Mock
    std::shared_ptr<zmf::AbstractModule> linkDiscoveryModuleMock_;

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



    /// Topic for SwitchRegistryModule Switch_Event
    zmf::data::MessageType topicsSwitchEvent_ = zsdn::modules::SwitchRegistryModuleTopics<zmf::data::MessageType>().from().switch_registry_module().switch_event().build();

    /// Topic for LinkDiscoveryModule SwitchLink_Event
    zmf::data::MessageType topicsSwitchLinkEvent_ = zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().from().link_discovery_module().switch_link_event().build();

    /// Topic for TopologyModule with GetTopologyRequest
    zmf::data::MessageType topicsGetTopologyRequest_ = zsdn::modules::TopologyModuleTopics<zmf::data::MessageType>().request().topology_module().get_topology().build();

    /**
     * Builds a demo ZmfMessage for the SwitchEvent Added from the SwitchRegistryModule for a given Switch.
     * @param aSwitch The Switch for which the demo ZmfMessage should be built.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoSwitchEventAdded_ZmfMessage(common::topology::Switch aSwitch);

    /**
     * Builds a demo ZmfMessage for the SwitchEvent Changed from the SwitchRegistryModule for a given Switch.
     * @param switchBefore The Switch before the Changed event for which the demo ZmfMessage should be built.
     * @param switchNow The Switch after the Changed event for which the demo ZmfMessage should be built.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoSwitchEventChanged_ZmfMessage(
            common::topology::Switch switchBefore, common::topology::Switch switchNow);

    /**
     * Builds a demo ZmfMessage for the SwitchEvent Removed from the SwitchRegistryModule for a given Switch.
     * @param aSwitch The Switch for which the demo ZmfMessage should be built.
     */
    std::shared_ptr<zmf::data::ZmfMessage> build_DemoSwitchEventRemoved_ZmfMessage(common::topology::Switch aSwitch);

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

};

#endif //DEMO_MODULE_UT_SWITCH_ADAPTER_H
