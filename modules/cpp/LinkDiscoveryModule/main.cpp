#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "LinkDiscoveryModule.hpp"
#include <google/protobuf/stubs/common.h>

int main(int argc, char* argv[]) {
    int returnCode;
    if (zsdn::StartupHelper::paramsOkay(argc, argv)) {
        zmf::logging::ZmfLogging::initializeLogging("LinkDiscoveryModule", argv[1]);
        returnCode = zsdn::StartupHelper::startInConsole(new LinkDiscoveryModule(0), argv[1]);
    } else {
        returnCode = 1;
    }
    google::protobuf::ShutdownProtobufLibrary();
    return returnCode;
}