#ifndef SimpleForwardingModule_H
#define SimpleForwardingModule_H

#include <zmf/AbstractModule.hpp>
#include <thread>
#include <map>
#include <mutex>
#include <tins/ethernetII.h>
#include <zsdn/topics/DeviceModuleTopics.hpp>
#include "ModuleTypeIDs.hpp"
#include <RequestUtils.h>
#include <zsdn/topics/SwitchAdapterTopics.hpp>

/**
 * The SimpleForwardingModule is a very simple packet forwarding moulde. It receives
 * PacketIn messages and does layer 2 forwarding.
 * @author Matthias Strljic
 * @author Andre Kutzleb
 */
class SimpleForwardingModule : public zmf::AbstractModule {

public:
    /**
     * Default Module constructor
     * @param  instanceId unique id of the module instance.
     */
    SimpleForwardingModule(uint64_t instanceId);

    ~SimpleForwardingModule();

protected:
    /**
     * Setup the the needed subscriptions (OpenFlow PacketIn, DeviceManager Item changed)
     * and the database of existing devices in the network. This includes requests to the
     * DeviceManager module.
     * @return True if enable successful, False if enable failed or rejected
     */
    virtual bool enable();

public:

    /**
     * Called when the module should disable itself. Must stop the module (threads etc.).
     */
    virtual void disable();

    /**
     * A minimalised representation of a device inside the network with the relevant information about
     * it' MAC, connected switch and switch port.
     */
    struct Device {

        uint64_t switchDpid;
        uint32_t switchPort;

        uint64_t macAddress;

    };
private:
    // PERFORMANCE IMPROFEMENT
    of_packet_out_t* tempPacketOut_OF_1_0;
    of_packet_out_t* tempPacketOut_OF_1_3;
    of_list_action_t* tempActionList_OF_1_0;
    of_list_action_t* tempActionList_OF_1_3;
    of_action_output_t* tempOutPut_OF_1_0;
    of_action_output_t* tempOutPut_OF_1_3;


    /// represents the version of the module in it's current state
    static const uint16_t MODULE_VERSION = 0;
    /// module type id
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_SimpleForwardingModule;

    static const uint16_t deviceMgrDependencyType_ = zsdn::MODULE_TYPE_ID_DeviceModule;
    static const uint16_t deviceMgrDependencyVersion_ = 0;


    /// Map to store all known devices
    std::map<std::uint64_t, Device> devices_;
    /// Map to store all MessageTypes for the specific SwitchDpid
    std::map<std::uint64_t, zmf::data::MessageType> linkDevicePacketOutMessageTypeMap_;
    /// All the ethertypes in this list will be excluded from forwarding
    std::vector<uint16_t> ignoreEthertypes_;

    // Builder for topic creation
    zsdn::modules::DeviceModuleTopics<zmf::data::MessageType> deviceModuleTopics_;
    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType> switchAdapterTopics_;


    // Topics
    zmf::data::MessageType requestDeviceByMacTopic_ = deviceModuleTopics_.request().device_module().get_device_by_mac_address()
            .build();

    zmf::data::MessageType getAllDevicesMsgTopic_ = deviceModuleTopics_.request().device_module().get_all_devices().build();

    /**
     * Setup of the devices map to know all devices at the  current state of the system.
     * At the beginning call of all DeviceManager modules to request in the second step all known devices.
     * @return True if at least one DeviceManager replied on the getAllDevices() Request and the data is stored in the
     *         devices map.
     */
    virtual bool setupDatabase();

    /**
     * Setup of the needed subscriptions, to receive updates from the DeviceManager modules (about changes of devices)
     * and incoming OpenFlow PacketIn Messages
     * @return True if all subscriptions are setup correctly
     */
    virtual bool setupSubscriptions();

    /**
     * Handle function to process the incoming PacketIn messages of the correlation subscription of OpenFlow PacketIn
     * @param packetInMsg The received message of the OpenFlow PacketIn subscription
     */
    virtual void handlePacketIn(const zmf::data::ZmfMessage& packetInMsg);

    /**
     * Handle function to process the incoming device events.
     * @param changeMsg The received change message about device added/removed/state changed.
     */
    virtual void handleDeviceStateChanged(const zmf::data::ZmfMessage& changeMsg);

    /**
     * Thread safe function to insert a new device and returns it as result.
     * @param mac The MAC address of the new device, also used as map key.
     * @param switchDpid Switch id == to the module id of the correlating SwitchAdapter module
     * @param switchPort Correlating switch port of which the device is connected to the switch.
     * @return Pointer to the element inside of the map if the function failed -> return nullptr
     */
    virtual Device* insertNewDevice(std::uint64_t mac, std::uint64_t switch_dpid, uint32_t switch_port);

    /**
     * Thread safe function to modify a device inside the device map if properties of the device changed.
     * @param mac The MAC address of the device, also used as map key.
     * @param switchDpid Switch id == to the module id of the correlating SwitchAdapter module
     * @param switchPort Correlating switch port of which the device is connected to the switch.
     */
    virtual void modifyDevice(std::uint64_t mac, std::uint64_t switch_dpid, uint32_t switch_port);

    /**
     * Thread safe function to delete a device of the devices map (if the device exists)
     * @param mac The MAC address of the device. Used to find the device inside the map as key.
     */
    virtual void deleteDevice(std::uint64_t mac);

    /**
     * Sends a request to a DeviceManager module with a GetDeviceByMac Request to receive information
     * (connected switch id and switch port) about the device as result. The function will add
     * the received device to the devices map.
     * @param mac The MAC address of the device
     * @result The replied device pointer inside the map. Result == nullptr if no device found.
     */
    virtual Device* requestDevice(std::uint64_t mac);


};


#endif // SimpleForwardingModule_H
