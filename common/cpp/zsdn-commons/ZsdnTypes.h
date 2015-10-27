//
// Created by zsdn on 7/24/15.
//

#ifndef ZSDN_COMMONS_ZSDNTYPES_H
#define ZSDN_COMMONS_ZSDNTYPES_H

#include <stdint.h>
#include <iostream>
#include "NetUtils.h"

namespace zsdn {
    typedef uint64_t MAC;
    typedef uint64_t DPID;
    typedef uint32_t Port;

    struct AttachmentPoint {
        DPID switchDpid;
        Port switchPort;

        const std::string toString() const {
            return "[sw="
                   + zsdn::NetUtils::uint64_to_switch_dpid_string(switchDpid)
                   + ",p=" + std::to_string(switchPort)
                   + "]";
        }
    };

    struct Device {
        AttachmentPoint attachmentPoint;
        MAC macAddress;

        const std::string toString() const {
            return NetUtils::uint64_to_mac_address_string(macAddress) + " @" + attachmentPoint.toString();
        }
    };


    inline bool operator<(const AttachmentPoint& l, const AttachmentPoint& r) {
        return l.switchDpid < r.switchDpid && l.switchPort < r.switchPort;
    }

    inline bool operator==(const AttachmentPoint& l, const AttachmentPoint& r) {
        return l.switchDpid == r.switchDpid && l.switchPort == r.switchPort;
    }


    inline bool operator<(const Device& l, const Device& r) {
        return l.macAddress < r.macAddress;
    }

    inline bool operator==(const Device& l, const Device& r) {
        return l.macAddress == r.macAddress;
    }

}

#endif //ZSDN_COMMONS_ZSDNTYPES_H
