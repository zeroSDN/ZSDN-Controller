//
// Created by zsdn on 7/15/15.
//

#include <stdint.h>
#include <vector>
#include <Poco/Exception.h>
#include "Port.h"

extern "C" {
#include "loci/loci.h"
}

#ifndef SWITCHREGISTRYMODULE_SWITCH_H
#define SWITCHREGISTRYMODULE_SWITCH_H

#endif //SWITCHREGISTRYMODULE_SWITCH_H
/**
 * @brief A class, that contains data for a Switch.
 *
 * @details This Class only Purpose is to contain the information for a Switch.
 *
 * @author Sebastian Vogel
 */
class Switch {
private:


    uint64_t switchID; //ID of the switch

public:

    Switch() {
        throw Poco::InvalidAccessException("invalid empty constructor call of Switch(), probably illegal map access");
    };

    Switch(uint64_t switchID, const of_version_t& of_version);

    bool active = false;
    of_version_t of_version;
    //OF-Version
    bool switch_info_available = false;
    uint32_t n_buffers;
    /* Max packets buffered at once. */
    uint8_t n_tables;
    /* Number of tables supported by datapath.(8 bit) */
    uint8_t auxiliary_id; /* Identify auxiliary connections (8 bit)*/
    /* Features. */
    uint32_t capabilities;
    /* Bitmap of support "ofp_capabilities". */
    //in Version 1.0 there is a active field instead of a reserved field. will be stored there(same size)
    uint32_t reserved;
    bool got_ports = false;
    std::vector<Port> ports;

    /**
     * getter-Method to access the private Switch-ID, which should not change
     */
    uint64_t getSwitchID();

};