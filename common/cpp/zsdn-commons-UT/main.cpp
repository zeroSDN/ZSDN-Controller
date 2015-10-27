#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include "zsdnCommonsTests.h"
#include <cppunit/TestResult.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextOutputter.h>
#include <google/protobuf/stubs/common.h>


using namespace CppUnit;

CPPUNIT_TEST_SUITE_REGISTRATION(zsdnCommonsTests);


int main (int argc, char* argv[]) {

// informs test-listener about testresults
    CppUnit::TestResult testresult;

    // register listener for collecting the test-results
    CppUnit::TestResultCollector collectedresults;
    testresult.addListener(&collectedresults);

    // register listener for per-test progress output
    CppUnit::BriefTestProgressListener progress;
    testresult.addListener(&progress);

    TextUi::TestRunner runner;
    TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    runner.run(testresult);

    // output results in compiler-format
    CppUnit::TextOutputter textOutputter(&collectedresults, std::cerr);
    textOutputter.write();

    google::protobuf::ShutdownProtobufLibrary();

    // return 0 if tests were successful
    return collectedresults.wasSuccessful() ? 0 : 1;
}