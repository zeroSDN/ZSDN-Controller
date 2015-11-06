#include <iostream>

#include <memory>
#include <Poco/Exception.h>
#include <zmf/AbstractModule.hpp>
#include <zmf/ZmfConsole.hpp>
#include "zmf/ZmfInstance.hpp"
#include "TemplateModule.hpp"


int main() {

    std::cout << "Starting TemplateModule. Please enter instance ID: ";
    uint64_t instanceId;
    std::cin >> instanceId;

    std::vector<zmf::instance::ZmfInstance::StartOption> startOptions = {
            // zmf::instance::ZmfInstance::StartOption::NO_AUTO_ENABLE, // module won't be enabled automatically. If left out, module will be enabled as soon as possible (dependencies satisfied)
            //zmf::instance::ZmfInstance::StartOption::NO_EXIT_WHEN_ENABLE_FAILED, // No shutdown when module enabling failed
            //zmf::instance::ZmfInstance::StartOption::NO_PEER_DISCOVERY_WAIT // if left out, starting module will wait until all peers are discovered
    };

    try {
        // Create module
        std::shared_ptr<zmf::AbstractModule> module = std::shared_ptr<zmf::AbstractModule>(
                new TemplateModule(instanceId));
        // Create and start ZMF instance with module
        std::shared_ptr<zmf::IZmfInstanceController> zmfInstance = zmf::instance::ZmfInstance::startInstance(module,
                                                                                                    startOptions);
        // Start console
        zmf::ZmfConsole console(zmfInstance);
        console.startConsole();
    }
    catch (Poco::Exception exc) { // Catch Exceptions
        std::cerr << "Failed to run module: " << exc.message() << &std::endl;
        return 1;
    }
    catch (...) {  // Catch all
        std::cerr << "Failed to run module: Unknown reason" << &std::endl;
        return 1;
    }

    return 0;
}