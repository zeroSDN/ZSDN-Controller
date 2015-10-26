#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "ARPModule.hpp"

int main(int argc, char* argv[]) {

    if(zsdn::StartupHelper::paramsOkay(argc,argv)) {
        zmf::logging::ZmfLogging::initializeLogging("ARPModule", argv[1]);
        return zsdn::StartupHelper::startInConsole(new ARPModule(0), argv[1]);
    } else {
        return 1;
    }
}