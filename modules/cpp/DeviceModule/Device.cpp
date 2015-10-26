#include <zsdn/proto/DeviceModule.pb.h>
#include <iostream>
#include "Device.h"
#include "DeviceModule.hpp"

Device::Device(uint64_t mac_address, std::vector<uint32_t> ipv4_addresses,
               std::vector<std::array<uint8_t, 16>> ipv6_addresses, uint64_t switch_dpid, uint32_t switch_port)
        : macAddress_(mac_address), ipv4Addresses_(ipv4_addresses), ipv6Addresses_(ipv6_addresses),
          attachmentPoint_(switch_dpid, switch_port) {
    this->SetTimestampToNow();
}

Device::Device(const common::topology::Device protoDevice) : macAddress_(protoDevice.mac_address()),
                                                             millisSinceLastSeen_(protoDevice.millis_since_last_seen()),
                                                             attachmentPoint_(
                                                                     protoDevice.attachment_point().switch_dpid(),
                                                                     protoDevice.attachment_point().switch_port()) {
    ipv4Addresses_.reserve(protoDevice.ipv4_address().size());
    for (int i = 0; i < protoDevice.ipv4_address().size(); i++) {
        ipv4Addresses_.insert(ipv4Addresses_.end(), protoDevice.ipv4_address(i));
    }

    ipv6Addresses_.reserve(protoDevice.ipv6_address_size());
    for (int i = 0; i < protoDevice.ipv6_address_size(); i++) {
        // Convert
        Tins::IPv6::address_type tinsIPv6Address = Tins::IPv6::address_type(
                protoDevice.ipv6_address(i));
        std::array<uint8_t, 16> ipv6Address{};
        std::copy(tinsIPv6Address.begin(), tinsIPv6Address.end(), ipv6Address.begin());

        ipv6Addresses_.insert(ipv6Addresses_.end(), ipv6Address);

    }
}

common::topology::Device* Device::convertToProtoDevice(const Device& device) {
    common::topology::Device* protoDevice = new common::topology::Device();
    protoDevice->set_mac_address(device.macAddress_);
    for (int i = 0; i < device.ipv4Addresses_.size(); i++) {
        protoDevice->add_ipv4_address(device.ipv4Addresses_[i]);

    }

    for (int i = 0; i < device.ipv6Addresses_.size(); i++) {
        // Convert
        Tins::IPv6::address_type tinsIPv6Address;
        std::copy(device.ipv6Addresses_[i].begin(), device.ipv6Addresses_[i].end(), tinsIPv6Address.begin());

        protoDevice->add_ipv6_address(tinsIPv6Address.to_string());
    }

    protoDevice->set_millis_since_last_seen(device.millisSinceLastSeen_);
    common::topology::AttachmentPoint* attP = new common::topology::AttachmentPoint();
    attP->set_switch_dpid(device.attachmentPoint_.switchDpid);
    attP->set_switch_port(device.attachmentPoint_.switchPort);
    protoDevice->set_allocated_attachment_point(attP);

    return protoDevice;
}

void Device::SetTimestampToNow() {
    this->timestampMs_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    );
}

long Device::GetMillisSinceLastSeen() {

    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    );

    return now.count() - this->timestampMs_.count();
}


