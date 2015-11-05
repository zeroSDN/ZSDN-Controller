#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "StatisticsModule.hpp"

int main(int argc, char* argv[]) {
    int returnCode;
    if (zsdn::StartupHelper::paramsOkay(argc, argv)) {
        zmf::logging::ZmfLogging::initializeLogging("StatisticsModule", argv[1]);
        returnCode = zsdn::StartupHelper::startInConsole(new StatisticsModule(0), argv[1]);
    } else {
        returnCode = 1;
    }
    google::protobuf::ShutdownProtobufLibrary();
    return returnCode;
}