//
// Created by Andre Kutzleb on 8/07/15.
//

#ifndef DEMO_MODULE_UT_DEMOMODULETESTS_H
#define DEMO_MODULE_UT_DEMOMODULETESTS_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include "NetUtils.h"

using namespace CppUnit;

class zsdnCommonsTests : public CppUnit::TestCase {
CPPUNIT_TEST_SUITE(zsdnCommonsTests);

        CPPUNIT_TEST(testLociExtensions);
        CPPUNIT_TEST(testRequestUtils);
        CPPUNIT_TEST(testNetworkGraph);
        CPPUNIT_TEST(testMacConversions);
        CPPUNIT_TEST(testTopicConstruction);


    CPPUNIT_TEST_SUITE_END();

public:

    zsdnCommonsTests();

    void testMacConversions();

    void testNetworkGraph();

    void testRequestUtils();

    void testLociExtensions();

    void testTopicConstruction();

};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(zsdnCommonsTests, zsdnCommonsTests);

#endif //DEMO_MODULE_UT_DEMOMODULETESTS_H
