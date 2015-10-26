//
// Created by zsdn on 7/15/15.
//

#include "Switch.h"


uint64_t Switch::getSwitchID() {
    return switchID;
}

Switch::Switch(uint64_t switchID, const of_version_t& of_version)
        :
        switchID(switchID),
        of_version(of_version) { }
