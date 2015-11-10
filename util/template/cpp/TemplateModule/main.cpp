#include <iostream>

#include <memory>
#include <zmf/AbstractModule.hpp>
#include "TemplateModule.hpp"
#include "zsdn/StartupHelper.h"


int main(int argc, char* argv[]) {

	// Default instance id = 0, if multiple modules of same type should run in paralell, 
	// each one has to start with a unique one.
	uint64_t instanceId = 0;

	std::vector<zmf::instance::ZmfInstance::StartOption> startOptions = {
            // zmf::instance::ZmfInstance::StartOption::NO_AUTO_ENABLE, 		// module won't be enabled automatically. If not used, module will be enabled as soon as possible (dependencies satisfied)
            // zmf::instance::ZmfInstance::StartOption::NO_EXIT_WHEN_ENABLE_FAILED, 	// If not used, module will simply remain disabled instead of shutting down when enable fails.
            // zmf::instance::ZmfInstance::StartOption::NO_PEER_DISCOVERY_WAIT 		// if not used, modules will start quicker, but may not know about all other modules when enabled yet.
	    // zmf::instance::ZmfInstance::StartOption::NO_EQUAL_MODULE_INTERCONNECT 	// if not used, modules of the same type will connect to each other. for modules that have many instances, this causes a lot of unecessary interconnections (unless they are actually needed).
    };

   	int returnCode;
    if(zsdn::StartupHelper::paramsOkay(argc,argv)) {
    	// Load the logger configuration before anyone instanciates a logger
        zmf::logging::ZmfLogging::initializeLogging("TemplateModule", argv[1]); 
        // Start the module
        returnCode = zsdn::StartupHelper::startInConsole(new TemplateModule(instanceId), argv[1],startOptions);
    } else {
        returnCode = 1;
    }
    return returnCode;
}