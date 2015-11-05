#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "LoggingModule.hpp"


int main(int argc, char* argv[]) {
    int returnCode;
    if (zsdn::StartupHelper::paramsOkay(argc, argv)) {
        zmf::logging::ZmfLogging::initializeLogging("LoggingModule", argv[1]);
        returnCode = zsdn::StartupHelper::startInConsole(new LoggingModule(0), argv[1]);
    } else {
        returnCode = 1;
    }
    google::protobuf::ShutdownProtobufLibrary();
    return returnCode;
}