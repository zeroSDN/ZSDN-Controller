#ifndef TopologyModule_H
#define TopologyModule_H

#include <zmf/AbstractModule.hpp>
#include <zsdn/topics/SwitchRegistryModule_topics.hpp>
#include <zsdn/topics/LinkDiscoveryModule_topics.hpp>
#include <zsdn/topics/TopologyModule_topics.hpp>
#include <thread>
#include <zsdn/proto/CommonTopology.pb.h>
#include <ModuleTypeIDs.hpp>

extern "C" {
#include <loci/loci.h>
}

/**
 * @details The TopologyModule collects information about Switches connected to the SDN network and the Links between those Switches.
 * @author Tobias Freundorfer
 */
class TopologyModule : public zmf::AbstractModule {

public:
    TopologyModule(uint64_t instanceId);

    ~TopologyModule();

    /**
     * Handles incoming requests of types
     * 1.) kGetTopologyRequest
     * and replies with the current stable topology.
     * @param message The Zmf message that was submitted.
     * @param sender The sender that sent the message.
     * @return True if we want to respond on the request, false otherwise
     */
    virtual zmf::data::ZmfOutReply handleRequest(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender) override;

    /* Begin: section for UnitTest-Accessor methods for testing internal behaviour */


    void UTAccessor_requestAllSwitchesFromSwitchRegistryModule() { requestAllSwitchesFromSwitchRegistryModule(); };

    void UTAccessor_requestAllSwitchToSwitchLinksFromLinkDiscoveryModule() { requestAllSwitchToSwitchLinksFromLinkDiscoveryModule(); };

    const std::map<uint64_t, common::topology::Switch>& UTAccessor_getSwitchesCache() { return switchesCache_; };

    const std::vector<common::topology::SwitchToSwitchLink>& UTAccessor_getSwitchToSwitchLinkCacheRegistered() { return switchToSwitchLinkCacheRegistered_; };

    const std::vector<common::topology::SwitchToSwitchLink>& UTAccessor_getSwitchToSwitchLinkCacheUnregistered() { return switchToSwitchLinkCacheUnregistered_; };

    zmf::data::ZmfOutReply UTAccessor_handleRequest(const zmf::data::ZmfMessage& message,
                                                    const zmf::data::ModuleUniqueId& sender) {
        return handleRequest(message, sender);
    };

    const common::topology::Topology UTAccessor_getCurrentStableTopology() { return *currentStableTopology_; };

    void UTAccessor_handleSwitchEvent(const ZmfMessage& message, const ModuleUniqueId& id) {
        handleSwitchEvent(message, id);
    };

    void UTAccessor_handleSwitchLinkEvent(const ZmfMessage& message, const ModuleUniqueId& id) {
        handleSwitchLinkEvent(message, id);
    };

    bool UTAccessor_isSwitchRegistered(uint64_t switchID, uint32_t switchPort) {
        return isSwitchRegistered(switchID, switchPort);
    };

    /* End: section */

    /**
     * Called when the module should disable itself. This will clear all caches of this module.
     */
    virtual void disable();

protected:
    /**
     * Called when the module should enable itself. Requests all Switches and SwitchToSwitchLinks and subscribes
     * itself to the specific topics.
     * @return True if enable successful, False if enable failed or rejected
    */
    virtual bool enable();

private:
    /// The version of this module.
    static const uint16_t MODULE_VERSION = 0;
    /// The type id of this module.
    static const uint16_t MODULE_TYPE_ID = 0x0006;
    /// The instance id of this module.
    uint64_t instanceId_;

    /// The module dependency version of the SwitchRegistryModule.
    const uint16_t switchRegistryModuleDependencyVersion_ = 0;
    /// The module dependency version of the LinkDiscoveryModule.
    const uint16_t linkDiscoveryModuleDependencyVersion_ = 0;


    /// Map holding all Switches known in the network.
    std::map<uint64_t, common::topology::Switch> switchesCache_;

    /// Vector holding SwitchToSwitchLinks where both the source and the target AttachmentPoint is registered in the SwitchCache.
    std::vector<common::topology::SwitchToSwitchLink> switchToSwitchLinkCacheRegistered_;
    /// Vector holding SwitchToSwitchLinks where the source and/or the target AttachmentPoint is NOT (already) registered in the SwitchCache.
    std::vector<common::topology::SwitchToSwitchLink> switchToSwitchLinkCacheUnregistered_;

    /// The newest stable Topology.
    /// (Stable means in this context that only links between Swichtes that are also registered in the SwitchesCache are part of the Topology)
    common::topology::Topology* currentStableTopology_ = nullptr;

    /// Topic for LinkDiscoveryModule Switch_Link_Event
    zmf::data::MessageType topicsSwitchLinkEvent_ = linkdiscoverymodule_topics::FROM().link_discovery_module().switch_link_event().build();

    /// Topic for SwitchRegistryModule Switch_Event
    zmf::data::MessageType topicsSwitchEvent_ = switchregistrymodule_topics::FROM().switch_registry_module().switch_event().build();

    /// Topic for get_topology Reply
    zmf::data::MessageType topicsGetTopologyReply_ = topologymodule_topics::REPLY().topology_module().get_topology().build();

    /// Topic for TopologyModule TopologyChanged Event
    zmf::data::MessageType topicsTopologyChangedEvent_ = topologymodule_topics::FROM().topology_module().topology_changed_event().build();

    /// Topic for requesting all Switches from SwitchRegistryModule
    zmf::data::MessageType topicsGetAllSwitches_ = switchregistrymodule_topics::REQUEST().switch_registry_module().get_all_switches().build();

    /// Topic for requesting all SwitchToSwitchLinks from LinkDiscoveryModule
    zmf::data::MessageType topicsGetAllSwitchLinks_ = linkdiscoverymodule_topics::REQUEST().link_discovery_module().get_all_switch_links().build();

    /**
     * Handles incoming packets of the SwitchRegistryModule for changing Switches.
     * Possible event types are:
     * 1.) switch_added
     * 2.) switch_removed
     * 3.) switch_changed
     * @param message The ZmfMessage that was submitted.
     * @param id The id of the Module that has submitted this ZmfMessage.
     */
    void handleSwitchEvent(const ZmfMessage& message, const ModuleUniqueId& id);

    /**
     * Sends a request to the SwitchRegistryModule for all currently registered Switches and adds all the returned Switches to the SwitchCache.
     */
    void requestAllSwitchesFromSwitchRegistryModule();

    /**
     * Adds a new Switch to the SwitchCache.
     * @param newSwitch The Switch to be added.
     */
    void addNewSwitch(const common::topology::Switch& newSwitch);

    /**
     * Updates the given Switch in the SwitchCache if necessary or not.
     * If the Switch is not already registered in the SwitchCache it gets added.
     * @param updatedSwitch The Switch which has possibly updated values.
     */
    void updateSwitch(const common::topology::Switch& updatedSwitch);

    /**
     * Removes the given Switch from the SwitchCache.
     * If the Switch to remove is not already registered it removes nothing.
     * @param removedSwitch The Switch that should be removed.
     */
    void removeSwitch(const common::topology::Switch& removedSwitch);

    /**
     * Handles incoming packets of the LinkDiscoveryModule for changing Links.
     * Possible event type are:
     * 1.) switch_link_added
     * 2.) switch_link_removed
     * @param message The ZmfMessage that was submitted.
     * @param id The id of the Module that has submitted this ZmfMessage.
     */
    void handleSwitchLinkEvent(const ZmfMessage& message, const ModuleUniqueId& id);

    /**
     * Sends a request to the LinkDiscoveryModule for all currently registered SwitchToSwitchLinks and adds all the returned SwitchToSwitchLinks to the SwitchToSwitchLinkCache.
     */
    void requestAllSwitchToSwitchLinksFromLinkDiscoveryModule();

    /**
     * Adds a new Switch to Switch Link to the SwitchToSwitchLinkCache.
     * @param newLink The new Link that should be added.
     */
    void addNewSwitchToSwitchLink(const common::topology::SwitchToSwitchLink& newLink);

    /**
     * Removes the given SwitchToSwitchLink from the SwitchToSwitchLinkCache.
     * If the SwitchToSwitchLink is not already registered it removes nothing.
     * @param removedLink The SwitchToSwitchLink that should be removed.
     */
    void removeSwitchToSwitchLink(const common::topology::SwitchToSwitchLink& removedLink);

    /**
     * Checks for the currently unregistered SwitchToSwichtLinks if one can now be set from Unregistered to Registered.
     */
    void checkSwitchToSwitchLinkStateChanged();

    /**
     * Initializes the current stable Topology according to the entries in the specific Caches.
     * It will delete the currently set Topology if set.
     */
    void initializeCurrentStableTopology();

    /**
     * Checks whether the given Switch is registered in the SwitchCache or not.
     * @param switchID The Switch ID.
     * @param switchPort The Switch port.
     * @return Whether the Switch is registered or not.
     */
    bool isSwitchRegistered(uint64_t switchID, uint32_t switchPort);

    /**
     * Publishs the Event for TopologyChanged.
     */
    void publishTopologyChangedEvent();

    /**
     * Validates the given Topology and returns false if it is not valid.
     * @return True if Topology is valid, else False.
     */
    bool validate(const common::topology::Topology& topology);
};


#endif // TopologyModule_H
