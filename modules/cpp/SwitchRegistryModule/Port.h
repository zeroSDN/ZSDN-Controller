//
// Created by zsdn on 7/17/15.
//

#include <stdint.h>
#include <string>

#ifndef SWITCHREGISTRYMODULE_PORT_H
#define SWITCHREGISTRYMODULE_PORT_H

#endif //SWITCHREGISTRYMODULE_PORT_H

/**
 * @brief A class, that contains data for a Port.
 *
 * @details This Class only Purpose is to contain the information for a Port.
 * A Port always belongs to a Switch.
 *
 * @author Sebastian Vogel
 */
class Port{
public:


    Port(uint32_t switch_port);
    uint32_t switch_port;

    uint64_t mac_address; /* mac address of port */
    std::string port_name; /* Name of port */
    uint32_t config; /* Bitmap of OFPPC_* flags. */
    uint32_t state; /* Bitmap of OFPPS_* flags. */
    uint32_t curr; /* Current features. */
    uint32_t advertised; /* Features being advertised by the port. */
    uint32_t supported; /* Features supported by the port. */
    uint32_t peer; /* Features advertised by peer. */
    uint32_t curr_speed; /* Current port bitrate in kbps. */
    uint32_t max_speed; /* Max port bitrate in kbps */

};