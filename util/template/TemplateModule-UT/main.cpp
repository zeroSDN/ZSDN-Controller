#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include "TemplateModuleTests.h"
#include <google/protobuf/stubs/common.h>

using namespace CppUnit;

CPPUNIT_TEST_SUITE_REGISTRATION(TemplateModuleTests);


int main (int argc, char* argv[]) {
    TextUi::TestRunner runner;
    TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    bool failed = runner.run();
    google::protobuf::ShutdownProtobufLibrary();
    return !failed;

}