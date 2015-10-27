/**
 * @details Utils to pass configuration into unittests
 * @author Jonas Grunert
 * @date created on 10/14/15
*/
#ifndef SWITCHADAPTER_UT_UNITTESTCONFIGUTIL_HPP
#define SWITCHADAPTER_UT_UNITTESTCONFIGUTIL_HPP

#include <string>
#include <Poco/Util/PropertyFileConfiguration.h>

/** Global variable to pass configuration path to unittest classes */
extern std::string UT_CONFIG_FILE;


/**
 * Tries to set the UT_CONFIG_FILE variable. Called from unittests main method.
 * Configuration file must be given as FIRST ARGUMENT of the unittest execution
 */
inline int setUtConfigFile(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout << "No configuration file arg given." << &std::endl;
        std::cout << "Trying to locate and test default unittest configfile" << &std::endl;

        try {
            Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> testConfig = new Poco::Util::PropertyFileConfiguration();
            UT_CONFIG_FILE = std::string("./module-ut.config");
            testConfig->load(UT_CONFIG_FILE);
            std::cout << "Succeeded testing default unittest configfile" << &std::endl;
        } catch (...) {
            std::cerr << "Failed to locate and test default unittest configfile" << &std::endl;
            std::cout << "Usage: <configFile>" << &std::endl;
            return 1;
        }
    }
    else {
        UT_CONFIG_FILE = argv[1];
    }

    std::cout << "Config file path: " << UT_CONFIG_FILE << &std::endl;
    return 0;
}

#endif //SWITCHADAPTER_UT_UNITTESTCONFIGUTIL_HPP
