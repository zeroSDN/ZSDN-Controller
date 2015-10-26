#include <fstream>
#include <iostream>
#include <zmf/IZmfInstanceController.hpp>
#include <zmf/ZmfConsole.hpp>
#include "StartupHelper.h"
#include <zmf/ZmfInstance.hpp>


namespace zsdn {

    bool StartupHelper::paramsOkay(int argc, char* argv[], const std::string& usage,
                                   const uint32_t expectedParamCount) {

        if (argc < expectedParamCount) {
            std::cerr << usage << &std::endl;
            return false;
        }

        std::ifstream ifile(argv[1]);
        if (ifile) {
            // The file exists, and is open for input
            return true;
        } else {
            std::cerr << "config file does not exist: " << std::string(argv[1]) << &std::endl;
            return false;
        }

    }

    int StartupHelper::startInConsole(
            zmf::AbstractModule* module,
            const std::string& config,
            std::vector<zmf::instance::ZmfInstance::StartOption> startOptions) {

        try {
            std::shared_ptr<zmf::AbstractModule> modulePtr = std::shared_ptr<zmf::AbstractModule>(module);
            // Create and start ZMF instance with module
            std::shared_ptr<zmf::IZmfInstanceController> zmfInstance
                    = zmf::instance::ZmfInstance::startInstance(modulePtr, {}, config);
            // Start console
            zmf::ZmfConsole console(zmfInstance);
            console.startConsole();
            return 0;
        }
        catch (Poco::Exception exc) { // Catch Exceptions
            std::cerr << "Failed to run module: " << exc.message() << &std::endl;
            return 1;
        }
        catch (...) {  // Catch all
            std::cerr << "Failed to run module: Unknown reason" << &std::endl;
            return 1;
        }

    }
}