#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "SwitchRegistryModule.hpp"

int main(int argc, char* argv[]) {

    if (zsdn::StartupHelper::paramsOkay(argc, argv)) {
        zmf::logging::ZmfLogging::initializeLogging("SwitchRegistryModule", argv[1]);
        return zsdn::StartupHelper::startInConsole(new SwitchRegistryModule(0), argv[1]);
    } else {
        return 1;
    }
}