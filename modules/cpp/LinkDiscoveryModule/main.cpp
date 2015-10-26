#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "LinkDiscoveryModule.hpp"


int main(int argc, char* argv[]) {

    if (zsdn::StartupHelper::paramsOkay(argc, argv)) {
        zmf::logging::ZmfLogging::initializeLogging("LinkDiscoveryModule", argv[1]);
        return zsdn::StartupHelper::startInConsole(new LinkDiscoveryModule(0), argv[1]);
    } else {
        return 1;
    }

}