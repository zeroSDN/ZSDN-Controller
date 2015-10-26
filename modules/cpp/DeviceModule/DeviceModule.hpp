#ifndef DeviceModule_H
#define DeviceModule_H

#include <zmf/AbstractModule.hpp>
#include <thread>
#include <zsdn/proto/DeviceModule.pb.h>
#include <zsdn/topics/DeviceModule_topics.hpp>
#include <zsdn/topics/SwitchAdapter_topics.hpp>
#include <zsdn/topics/LinkDiscoveryModule_topics.hpp>
#include "Device.h"
#include "tins/tins.h"
#include "ModuleTypeIDs.hpp"

/**
 * @details The DeviceModule collects information about the Devices connected to the SDN network.
 * A Device in this context is an entity with a MAC-Address. Switches are not clarified as Devices. Therefore the DeviceModule uses
 * information about the Links between switches to determine whether a Device is a Switch or not.
 * @author Tobias Freundorfer
 */
class DeviceModule : public zmf::AbstractModule {

public:
    DeviceModule(uint64_t instanceId);

    ~DeviceModule();

    /**
     * Will handle requests of types...:
     * 1.) REQUEST.DEVICE_MODULE.GET_ALL_DEVICES: Query all Devices known to the DeviceModule.
     * 2.) REQUEST.DEVICE_MODULE.GET_DEVICE_BY_MAC_ADDRESS: Query for a single device. Request must contain MAC address of target device.
     * 3.) REQUEST.DEVICE_MODULE.GET_DEVICES_BY_FILTER: Queries all devices matching the filters provided in the request.
     * A device has to satisfy all provided filters in order to be present in the reply.
     *
     * ...and answers according to the request type replies of type:
     * 1.) REPLY.DEVICE_MODULE.GET_ALL_DEVICES:  Reply can contain 0 or more devices.
     * 2.) REPLY.DEVICE_MODULE.GET_DEVICE_BY_MAC_ADDRESS: Reply contains the device with that MAC address, or nothing, if the DeviceModule does not know any device with that MAC address.
     * 3.) REPLY.DEVICE_MODULE.GET_DEVICES_BY_FILTER: The reply may contain 0 or more devices.
     * @param message The Zmf message.
     * @param sender The sender of the according Zmf message.
     */
    virtual zmf::data::ZmfOutReply handleRequest(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender) override;


protected:
    /**
     * Enables this DeviceModule and subscribes itself to the specific topics.
     * @return True if enable successful, False if enable failed or rejected
     */
    virtual bool enable();

public:

    /**
     * Called when the module should disable itself. Must stop the module and clear all caches.
     */
    virtual void disable();

    /* Begin: section for UnitTest-Accessor methods for testing internal behaviour */

    const std::map<uint64_t, Device>& UTAccessor_getAllDevicesMap() { return devicesCache_; };

    void UTAccessor_addDevice(const Device& device) { addDevice(device); };

    void UTAccessor_handlePacketIn(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& id) {
        handlePacketIn(message, id);
    };

    zmf::data::ZmfOutReply UTAccessor_handleRequest(const zmf::data::ZmfMessage& message,
                                                    const zmf::data::ModuleUniqueId& sender) {
        return handleRequest(message, sender);
    };

    Device* UTAccessor_getDeviceByMACAddress(uint64_t macAddress) { return getDeviceByMACAddress(macAddress); };

    bool UTAccessor_isDevicePassingFurtherFilters(
            const DeviceModule_Proto::Request_GetDevicesByFilterRequest& filterRequest,
            Device& device) { return isDevicePassingFurtherFilters(filterRequest, device); };

    void UTAccessor_handleSwitchLinkEvent(const ZmfMessage& message, const ModuleUniqueId& id) {
        handleSwitchLinkEvent(message, id);
    };

    std::vector<common::topology::SwitchToSwitchLink> UTAccessor_SwitchToSwitchLinkCache() { return switchToSwitchLinkCache_; };

    void UTAccessor_RequestAllSwitchToSwitchLinksFromLinkDiscoveryModule() { requestAllSwitchToSwitchLinksFromLinkDiscoveryModule(); };

    void UTAccessor_validateDevices() { validateDevices(); };

    /* End: section */

private:
    /// The version of this module.
    static const uint16_t MODULE_VERSION = 0;
    /// The type-id of this module.
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_DeviceModule;
    uint64_t instanceId_;

    /// The array size for IPv6 Addresses
    static const int IPV6ADDRESS_SIZE = 16;
    /// The version of the LinkDiscovery module dependency
    static const uint16_t linkDiscoveryModuleDependencyVersion_ = 0;

    /// The pattern containing the valid packet_in types to subscribe to.
    std::string SUBSCRIPTIONS_VALIDTYPES_ARP = "arp";
    std::string SUBSCRIPTIONS_VALIDTYPES_IPV4 = "ipv4";
    std::string SUBSCRIPTIONS_VALIDTYPES_IPV6 = "ipv6";

    /// Map holding all the Devices that are registered in the DeviceModule.
    std::map<uint64_t, Device> devicesCache_;

    /// Vector holding SwitchToSwitchLinks.
    std::vector<common::topology::SwitchToSwitchLink> switchToSwitchLinkCache_;

    /// Topic for get_all_devices Reply
    zmf::data::MessageType topicsAllDevicesReply_ = devicemodule_topics::REPLY().device_module().get_all_devices().build();
    /// Topic for get_device_by_mac_address Reply
    zmf::data::MessageType topicsDeviceByMacReply_ = devicemodule_topics::REPLY().device_module().get_device_by_mac_address().build();
    /// Topic for get_devices_by_filter Reply
    zmf::data::MessageType topicsDevicesByFilterReply_ = devicemodule_topics::REPLY().device_module().get_devices_by_filter().build();
    /// Topic for DEVICE_EVENT CHANGED Reply
    zmf::data::MessageType topicsDeviceEventChanged_ = devicemodule_topics::FROM().device_module().device_event().changed().build();
    /// Topic for DEVICE_EVENT ADDED Reply
    zmf::data::MessageType topicsDeviceEventAdded_ = devicemodule_topics::FROM().device_module().device_event().added().build();
    /// Topic for DEVICE_EVENT REMOVED Reply
    zmf::data::MessageType topicsDeviceEventRemoved_ = devicemodule_topics::FROM().device_module().device_event().removed().build();

    /// Topic for LinkDiscoveryModule Switch_Link_Event
    zmf::data::MessageType topicsSwitchLinkEvent_ = linkdiscoverymodule_topics::FROM().link_discovery_module().switch_link_event().build();
    /// Topic for requesting all SwitchToSwitchLinks from LinkDiscoveryModule
    zmf::data::MessageType topicsGetAllSwitchLinks_ = linkdiscoverymodule_topics::REQUEST().link_discovery_module().get_all_switch_links().build();

    /**
     * Handles incoming packets.
     * This method will extract information of the devices available in the network.
     * @param message The ZmfMessage that was submitted.
     * @param id The id of the Module that has submitted this ZmfMessage.
     */
    void handlePacketIn(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& id);

    /**
     * Adds the given Device to the map of all devices.
     * @param device The device to be added.
     */
    void addDevice(const Device& device);

    /**
     * Decides whether the given device has to be updated or not. If necessary the device gets updated.
     * @param iterator The iterator pointing to the Device in the map of all devices.
     * @param device The Device with the possibly newer information that should be updated.
     * @return Whether an update was necessary or not.
     */
    bool updateDeviceIfNecessary(const std::map<uint64_t, Device>::iterator& existingDeviceIterator,
                                 const Device& newIncomingDevice);

    /**
     * Returns the Device with the given MAC-Address.
     * @param macAddress The MAC-Address for the seeked Device.
     * @return The specific Device with the given MAC-Address or nullptr if Device is not registered.
     */
    Device* getDeviceByMACAddress(uint64_t macAddress);

    /**
     * Checks the given Device against the given filters (excluding MAC-Address Filter).
     * @param filterRequest The filtering request for the devices.
     * @param device The specific device for which the filters should be checked.
     */
    bool isDevicePassingFurtherFilters(const DeviceModule_Proto::Request_GetDevicesByFilterRequest& filterRequest,
                                       Device& device);

    /**
     * Checks if the given IPv6 Address is a broadcast (::).
     * @param ipv6Address The IPv6 Address that should be checked.
     * @return Whether the given IPv6 Address is a broadcast or not.
     */
    bool isIPv6Broadcast(std::array<uint8_t, IPV6ADDRESS_SIZE> ipv6Address);

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
     * Sends a request to the LinkDiscoveryModule for all currently registered SwitchToSwitchLinks and adds all the returned SwitchToSwitchLinks to the SwitchToSwitchLinkCache.
     */
    void requestAllSwitchToSwitchLinksFromLinkDiscoveryModule();

    /**
     * Validates the currently knwon Devices and removes Devices if they were added
     * but invalid (e.g. Switches that were added when SwitchToSwitchLinks were not
     * known already)
     */
    void validateDevices();
};


#endif // DeviceModule_H
