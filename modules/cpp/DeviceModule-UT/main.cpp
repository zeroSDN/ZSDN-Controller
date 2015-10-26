#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include "DeviceModuleTests.h"
#include <UnittestConfigUtil.hpp>

std::string UT_CONFIG_FILE;


int main (int argc, char* argv[]) {

    // Try to read configuration parameter, exit if fails (no given config, could not load default)
    if (setUtConfigFile(argc, argv) != 0) {
        return 1;
    }
    CppUnit::TextUi::TestRunner runner;                                                        //  Create a runner-object
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();      //  Get a reference to the FactoryRegistry
    runner.addTest( registry.makeTest() );                                                     //  Getting every Test of every testsuite registered in the registry and add them to the runner
    bool wasSuccessful = runner.run( "", false );                                              //  Run every test and return if they run without errors or not
    google::protobuf::ShutdownProtobufLibrary();

    return wasSuccessful ? 0 : 1;
}