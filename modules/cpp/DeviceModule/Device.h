#ifndef DEVICEMODULE_DEVICE_H
#define DEVICEMODULE_DEVICE_H


#include <stdint.h>
#include <zsdn/proto/DeviceModule.pb.h>
#include <chrono>

/**
 * @details This class represents a Device that is connected to the SDN network.
 * A Device is an entity with a MAC-Address that is not a Switch.
 * @author Tobias Freundorfer
 */
class Device {
private:

    Device() : attachmentPoint_(0, 0) { }

    /// The time since the Device was seen the last time.
    uint64_t millisSinceLastSeen_ = 0;

    /// The Timestamp when the Device was seen last time.
    std::chrono::milliseconds timestampMs_;

public:
    /**
     * Constructor for a new Device.
     * @param mac_address The MAC-Address of this Device.
     * @param ipv4_address The vector of IPv4-Addresses of this Device.
     * @param ipv6_address The array of IPv6-Addresses of this Device.
     * @param switch_dpid The id of the switch which represents the attachment point.
     * @param switch_port The port of the switch which represents the attachment point.
     */
    Device(uint64_t mac_address, std::vector<uint32_t> ipv4_addresses,
           std::vector<std::array<uint8_t, 16>> ipv6_addresses, uint64_t switch_dpid, uint32_t switch_port);

    /**
     * Constructor for a new Device.
     * @param protoDevice The Protobuffer-Device that this Device should be built of.
     */
    Device(common::topology::Device protoDevice);

    /// The MAC-Address of this Device.
    uint64_t macAddress_;
    /// The vector of IPv4-Addresses of this Device.
    std::vector<uint32_t> ipv4Addresses_;
    /// The vector of IPv6-Addresses of this Device.
    std::vector<std::array<uint8_t, 16>> ipv6Addresses_;

    /// The AttachmentPoint of this Device.
    struct AttachmentPoint {
        AttachmentPoint(uint64_t switch_dpid, uint32_t switch_port) : switchDpid(switch_dpid),
                                                                      switchPort(switch_port) { }

        /// The id of the switch which represents the attachment point.
        uint64_t switchDpid;
        /// The port of the switch which represents the attachment point.
        uint32_t switchPort;
    } attachmentPoint_;
    // REMARK: IMPROVEMENT Possibility to handle multiple AttachmentPoints per Device

    /**
     * Convert the given Device to a ProtoDevice.
     * @param device The Device that should be converted.
     * @return The converted ProtoDevice.
     */
    static common::topology::Device* convertToProtoDevice(const Device& device);

    /**
     * Sets the Timestamp of this Device to the current time.
     */
    void SetTimestampToNow();

    /**
     * Returns the milli-seconds since the last time the Device was seen.
     */
    long GetMillisSinceLastSeen();
};


#endif //DEVICEMODULE_DEVICE_H
