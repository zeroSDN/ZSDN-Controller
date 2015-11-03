#ifndef ARPModule_H
#define ARPModule_H

#include <zmf/AbstractModule.hpp>
#include <thread>
#include <zsdn/topics/DeviceModuleTopics.hpp>
#include <zsdn/topics/SwitchAdapterTopics.hpp>
#include "ModuleTypeIDs.hpp"

extern "C" {
#include <loci/loci.h>
}


/**
 * @brief   A Module representing one ARP handle
 * @details The module handles any ARP request and reply.
 *          For an ARP reply, just forward the ARP to the target device, and in
 *          case a request it replies by it self in case it knows the target device,
 *          try to get the information from the device manager or ignore it in other case.0
 * @author  Jose Marin
 * @date    23.07.2015
 *
 */
class ARPModule : public zmf::AbstractModule {

public:
    /**
     * Allow theclassARPModuleTests to have access to the private methods of this class
     */
    friend class ARPModuleTests;

    ARPModule(uint64_t instanceId);

    ~ARPModule();


protected:

    /**
     * Called when the module should enable itself. Must initialize and start the module.
     * Here the module subscribes to topic ARP from Switch Adapter and  subscribes to topic
     * deviceStateChange from Device Module, also gets the devices that the device Manager
     * knows at this point
     * @return True if enable successful, False if enable failed or rejected
     */
    virtual bool enable();

public:

    /**
     * Delete the ARP cache
     */
    virtual void disable();


private:

    static const uint16_t deviceModuleDependencyType_ = zsdn::MODULE_TYPE_ID_DeviceModule;
    static const uint16_t deviceModuleDependencyVersion_ = 0;

    /**
     * Structure with the IP address of the device and the port and
     * the switch the device is attached to
    */
    struct Device {
        uint64_t switchDpid;
        uint32_t switchPort;
        uint32_t ipAddress;
    };

    /**
     * Data type declaration of an iterator over a map
    */
    typedef std::map<std::uint64_t, Device>::iterator IteratorArpCache;

    /**
     * Representation of the ARP cache with the necessary information to
     * build an ARP request/reply communication between two devices.
     * Every entry of the ARP Cache represents a device, it is implemented
     * as a map with a key (MAC address of the device) associated
     * to a structure with the ip address of the device and the port and
     * the switch ID the device is attached to
    */
    std::map<std::uint64_t, Device> arpCache_; // Map to store all known devices
    static const uint16_t MODULE_VERSION = 0;
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_ARPModule;
    uint64_t instanceId_;


    // Builders for topic creation
    zsdn::modules::DeviceModuleTopics<zmf::data::MessageType> deviceModuleTopics_;
    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType> switchAdapterTopics_;

    zmf::data::MessageType requestTopic_ = deviceModuleTopics_.request().device_module().get_all_devices().build();


    /**
    * Handles subscription to topic ARP from Switch Adapter. the ARP will be
    * reply/forward in case this module can discover the mac address of the
    * target or drop it out otherwise
    * @param message: Received protobuf message from the Device Module
    * @param moduleId: Id of the Device Module which sent the message
    */

    void handleIncomingArp(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& moduleId);

    /**
    * Handle the Subscription to topic deviceStateChangeTopic from Device Module,
    * so that the ARP cache of the ARPModuel is up to date
    * @param message: Received protobuf message from the Device Module
    */
    void handleUpdateDevice(const zmf::data::ZmfMessage& message);

    /**
    * It starts a request/reply communication with the device module, to get all
    * devices present in his ARP cache
    * @return true: One or more devices got from device module
     *        false: Did not get devices from device module
    */
    bool getDevicesFromDeviceModule();

    /**
    * It Adds a new device to the ARPCache in case the device does not exist
    * or Update a Device in the ARPCache in case the device already exists
    * @param ipAddressDevice IP address of the new device
    * @param macAddressDevice: The MAC address of the new device, it is used as map key.
    * @param switchDpid: ID of the switch the device is attached to
    * @param switchPort: Switch port the device is attached to
    */
    void updateArpCacheCache(std::uint32_t ipAddressDevice, std::uint64_t macAddressDevice,
                             std::uint64_t switchDpid, std::uint32_t switchPort);

    /**
    * Add a new device to the ARP Cache
    * @param ipAddressDevice IP address of the new device
    * @param macAddressDevice: The MAC address of the new device, it is used as map key.
    * @param switchDpid: ID of the switch the device is attached to
    * @param switchPort: Switch port the device is attached to
    */
    void addNewDeviceToArpCache(std::uint32_t ipAddressDevice, std::uint64_t macAddressDevice,
                                std::uint64_t switchDpid, std::uint32_t switchPort);

    /**
    * Update the information of a device present in the ARP Cache
    * @param ARPCacheEntry: pointer to the desired entry of the ARP cache to update
    * @param macAddressDevice: The MAC address of the new device
    * @param switchDpid: ID of the switch the device is attached to
    * @param switchPort: Switch port the device is attached to
    */
    void modifyDeviceInArpCache(std::map<std::uint64_t, Device>::iterator& arpCacheEntry,
                                std::uint32_t ipAddressDevice, std::uint64_t switchDpid, std::uint32_t switchPort);

    /**
    * Get the port of the switch the open flow packet_in came from
    * @param ofPacketIn: pointer to the open flow packet_in
    * @param ofVersion: version of the open flow of the packet_in
    * @return uint32_t: number of the port the packet_in came from
    */
    std::uint32_t getPortPacketInCameFrom(of_object_s* ofPacketIn);

    /**
    * Packs the ethernet frame in an open flow packet_out
    * @param serializedEth: vector with an ethernet frame
    * @param switchPort: Port of the switch the message has to be sent to
    * @param arpPacketOut: pointer to an open flow packet_out in which the ethernet
    * frame hast to be packed
    */
    void packEthFrameInOfPacketOut(std::vector<std::uint8_t>& serializedEth, std::uint32_t switchPort,
                                   of_packet_out_t* arpPacketOut);

    /**
    * Searches in the ARP cache if there is an entry with a particular ip address and
    * return the MAC address of the device
    * @param ipAddressDevice: IP Address of the device to search
    * @param posDeviceInArpCache(call by reference): Position of the device in the ARP cache
    * in case the Device was found in the ARP Cache
    * @return true: if the device with the ip address was found
    *         false: otherwise
    */
    bool GetDevicePosInArpCacheByIp(std::uint32_t ipAddressDevice, std::uint32_t& posDeviceInArpCache);

    /**
    * Sends a request to the device Module to get an ARP entry with a particular ip address,
    * and in case to received a positive answer update the ARP cache with this information
    * @param ipAddressDevice: IP Address of the device to search
    * @param posDeviceInArpCache(call by reference): Position of the new added device in the ARP cache
    * in case the Device was found in the ARP Cache
    * @return true: if the device with the ip address was found
    *         false: otherwise
    */
    bool GetDeviceFromDeviceModule(std::uint32_t ipAddTargetPackIn, std::uint32_t& posNewDeviceInArpCache);
};


#endif // ARPModule_H
