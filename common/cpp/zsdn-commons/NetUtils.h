//
// Created by Andre Kutzleb on 7/7/15.
//

#ifndef ZSDN_COMMONS_NETUTILS_H
#define ZSDN_COMMONS_NETUTILS_H

#include <stdint.h>
#include <tins/tins.h>

namespace zsdn {

    class NetUtils {
    public:

        static uint8_t* uint64_to_mac_address_array(uint64_t address);

        static uint8_t* uint64_to_switch_dpid_array(uint64_t switchDpid);

        static uint64_t mac_address_array_to_uint64(const uint8_t* const address);

        static uint64_t mac_address_tins_to_uint64(const Tins::EthernetII::address_type& address);

        static std::string uint64_to_mac_address_string(uint64_t macAddress);

        static std::string uint64_to_switch_dpid_string(uint64_t switchDpid);

        static uint64_t mac_address_string_to_uint64(const std::string& macAddress);

        static std::string data_string_data_to_hex_string(const std::string& dataString);
    };
}

#endif //ZSDN_COMMONS_NETUTILS_H
