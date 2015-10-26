//
// Created by Andre Kutzleb on 7/7/15.
//

#include "NetUtils.h"
#include <algorithm>

namespace zsdn {


    uint8_t* zsdn::NetUtils::uint64_to_mac_address_array(uint64_t address) {
        return new uint8_t[6]{
                (uint8_t) (address >> 40),
                (uint8_t) (address >> 32),
                (uint8_t) (address >> 24),
                (uint8_t) (address >> 16),
                (uint8_t) (address >> 8),
                (uint8_t) (address)
        };
    }

    uint8_t* zsdn::NetUtils::uint64_to_switch_dpid_array(uint64_t switchDpid) {
        return new uint8_t[8]{
                (uint8_t) (switchDpid >> 56),
                (uint8_t) (switchDpid >> 48),
                (uint8_t) (switchDpid >> 40),
                (uint8_t) (switchDpid >> 32),
                (uint8_t) (switchDpid >> 24),
                (uint8_t) (switchDpid >> 16),
                (uint8_t) (switchDpid >> 8),
                (uint8_t) (switchDpid)
        };
    }

    uint64_t zsdn::NetUtils::mac_address_array_to_uint64(const uint8_t* const address) {
        uint64_t converted =
                (((uint64_t) (address[0])) << 40) |
                (((uint64_t) (address[1])) << 32) |
                (((uint64_t) (address[2])) << 24) |
                (((uint64_t) (address[3])) << 16) |
                (((uint64_t) (address[4])) << 8) |
                (((uint64_t) (address[5])));

        return converted;
    }

    uint64_t zsdn::NetUtils::mac_address_tins_to_uint64(const Tins::EthernetII::address_type& address) {
        uint64_t converted =
                (((uint64_t) (address[0])) << 40) |
                (((uint64_t) (address[1])) << 32) |
                (((uint64_t) (address[2])) << 24) |
                (((uint64_t) (address[3])) << 16) |
                (((uint64_t) (address[4])) << 8) |
                (((uint64_t) (address[5])));

        return converted;
    }

      uint64_t zsdn::NetUtils::mac_address_string_to_uint64 (const std::string& macAddress) {
          std::string temp (macAddress);
          temp.erase (std::remove(temp.begin(), temp.end(), ':'), temp.end());

          std::istringstream buffer(temp);
          uint64_t result;
          buffer >> std::hex >> result;

          return result;
       }

    std::string zsdn::NetUtils::uint64_to_mac_address_string(uint64_t macAddress) {
        uint8_t* asArray = zsdn::NetUtils::uint64_to_mac_address_array(macAddress);

        std::stringstream stream;
        for (int i = 0; i < 6; i++) {
            stream
            << std::setfill('0')
            << std::setw(2)
               << std::hex << unsigned(asArray[i]);

            // dont append ":" at the end
            if (i < 5) {
                stream << ":";
            }
        }

        delete[] asArray;
        return stream.str();
    }

    std::string zsdn::NetUtils::uint64_to_switch_dpid_string(uint64_t switchDpid) {
        uint8_t* asArray = zsdn::NetUtils::uint64_to_switch_dpid_array(switchDpid);

        std::stringstream stream;
        for (int i = 0; i < 8; i++) {
            stream
            << std::setfill('0')
            << std::setw(2)
            << std::hex << unsigned(asArray[i]);

            // dont append ":" at the end
            if (i < 7) {
                stream << ":";
            }
        }

        delete[] asArray;
        return stream.str();
    }

    std::string zsdn::NetUtils::data_string_data_to_hex_string(const std::string& dataString) {

        std::stringstream stream;
        for (int i = 0; i < dataString.size(); i++) {
            stream
            << std::setfill('0')
            << std::setw(2)
            << std::hex << unsigned(dataString[i]);

            // dont append ":" at the end
            if (i < (dataString.size() -1)) {
                stream << ":";
            }
        }

        return stream.str();
    }

}