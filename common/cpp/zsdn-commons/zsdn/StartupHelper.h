//
// Created by zsdn on 10/14/15.
//

#include <string>
#include <bits/shared_ptr.h>
#include <zmf/AbstractModule.hpp>
#include <zmf/ZmfInstance.hpp>

#ifndef ZSDN_COMMONS_STARTUPHELPER_H
#define ZSDN_COMMONS_STARTUPHELPER_H


namespace zsdn {


    class StartupHelper {
    public:
        static bool paramsOkay(
                int argc, char* argv[],
                const std::string& config = "Usage: configFile",
                const uint32_t expectedParamCount = 2);

        static int startInConsole(
                zmf::AbstractModule* module,
                const std::string& config,
                std::vector<zmf::instance::ZmfInstance::StartOption> startOptions = {});

    };
}

#endif //ZSDN_COMMONS_STARTUPHELPER_H
