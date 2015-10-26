#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include "LoggingModuleTests.h"

using namespace CppUnit;

CPPUNIT_TEST_SUITE_REGISTRATION(LoggingModuleTests);
//CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( LoggingModuleTests, LoggingModuleTests );


int main (int argc, char* argv[]) {
    TextUi::TestRunner runner;
    TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    bool failed = runner.run();
    return !failed;

}