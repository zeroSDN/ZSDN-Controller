#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "ARPModule.hpp"
#include <google/protobuf/stubs/common.h>

int main(int argc, char* argv[]) {
    int returnCode;
    if(zsdn::StartupHelper::paramsOkay(argc,argv)) {
        zmf::logging::ZmfLogging::initializeLogging("ARPModule", argv[1]);
        returnCode = zsdn::StartupHelper::startInConsole(new ARPModule(0), argv[1]);
    } else {
        returnCode = 1;
    }
    google::protobuf::ShutdownProtobufLibrary();
    return returnCode;
}