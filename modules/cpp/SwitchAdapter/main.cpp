#include <iostream>
#include <zmf/AbstractModule.hpp>
#include "zsdn/StartupHelper.h"
#include "SwitchAdapterRunner.h"

/**
 * Starts a standalone server waiting for incoming connections from SDN-switches
 */
int main(int argc, char* argv[]) {

    if (zsdn::StartupHelper::paramsOkay(argc, argv, "Usage: configFile openflowVersion(e.g.\"1.0\") openflowPort", 4)) {

        zmf::logging::ZmfLogging::initializeLogging("SwitchAdapterRunner", argv[1]);

        Poco::ConsoleChannel* c = new Poco::ConsoleChannel;
        Poco::Logger::root().setChannel(c);

        of_version_t version;

        std::string ofVersionString = std::string(argv[2]);

        if (ofVersionString == "1.0") {
            version = OF_VERSION_1_0;
        } else if (ofVersionString == "1.1") {
            version = OF_VERSION_1_1;
        } else if (ofVersionString == "1.2") {
            version = OF_VERSION_1_2;
        } else if (ofVersionString == "1.3") {
            version = OF_VERSION_1_3;
        } else if (ofVersionString == "1.4") {
            version = OF_VERSION_1_4;
        } else {
            std::cerr << "Unknown openflow version, stopping execution (allowed: 1.0, 1.1, etc) " << &std::endl;
            return 1;
        }
        std::cout << "Using OpenFlow version " << ofVersionString << &std::endl;


        int ofPort;
        try {
            ofPort = std::atoi(argv[3]);

            if (ofPort < 0 || ofPort > 65535) {
                std::cerr << "Port must be in range [0,65535]" << "\n";
                return 1;
            }
        }
        catch (std::invalid_argument ia) {
            std::cerr << "Could not parse port as a number" << "\n";
            return 1;
        }

        SwitchAdapterRunner switchAdapterServer(version, ofPort, argv[1]);
        bool started = switchAdapterServer.start();
        if (started) {
            std::cout << "Press enter to stop the SwitchAdapterRunner.";
            std::cin.ignore();
            switchAdapterServer.stop();
            return 0;
        } else {
            return 1;
        }

    } else {
        return 1;
    }

}

