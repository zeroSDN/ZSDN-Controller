#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "SimpleForwardingModule.hpp"


int main(int argc, char* argv[]) {

    if (zsdn::StartupHelper::paramsOkay(argc, argv, "Usage: configFile instanceId", 3)) {

        uint64_t instanceId;
        std::stringstream(std::string(argv[2])) >> instanceId;

        zmf::logging::ZmfLogging::initializeLogging("SimpleForwardingModule", argv[1]);
        return zsdn::StartupHelper::startInConsole(new SimpleForwardingModule(instanceId), argv[1]);
    } else {
        return 1;
    }
}