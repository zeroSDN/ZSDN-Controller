#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <google/protobuf/stubs/common.h>
#include "SwitchAdapterTests.h"

#include <UnittestConfigUtil.hpp>

std::string UT_CONFIG_FILE;

using namespace CppUnit;

CPPUNIT_TEST_SUITE_REGISTRATION(SwitchAdapterTests);

int main(int argc, char* argv[]) {

    // Try to read configuration parameter, exit if fails (no given config, could not load default)
    if (setUtConfigFile(argc, argv) != 0) {
        return 1;
    }

    TextUi::TestRunner runner;
    TestFactoryRegistry& registry = TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    bool failed = runner.run();
    google::protobuf::ShutdownProtobufLibrary();
    return !failed;

}