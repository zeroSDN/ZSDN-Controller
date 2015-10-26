#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "DeviceModule.hpp"

int main(int argc, char* argv[]) {

    if (zsdn::StartupHelper::paramsOkay(argc, argv)) {
        zmf::logging::ZmfLogging::initializeLogging("DeviceModule", argv[1]);
        return zsdn::StartupHelper::startInConsole(new DeviceModule(0), argv[1]);
    } else {
        return 1;
    }
}