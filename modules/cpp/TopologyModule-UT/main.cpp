#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include "TopologyModuleTests.h"
#include <UnittestConfigUtil.hpp>

using namespace CppUnit;

std::string UT_CONFIG_FILE;

CPPUNIT_TEST_SUITE_REGISTRATION(TopologyModuleTests);


int main (int argc, char* argv[]) {

    // Try to read configuration parameter, exit if fails (no given config, could not load default)
    if (setUtConfigFile(argc, argv) != 0) {
        return 1;
    }
    TextUi::TestRunner runner;
    TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    bool failed = runner.run();
    google::protobuf::ShutdownProtobufLibrary();
    return !failed;

}