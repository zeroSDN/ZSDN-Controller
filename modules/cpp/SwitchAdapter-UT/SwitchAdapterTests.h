//
// Created by zsdn on 6/25/15.
//

#ifndef DEMO_MODULE_UT_SWITCH_ADAPTER_TESTS_H
#define DEMO_MODULE_UT_SWITCH_ADAPTER_TESTS_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <Poco/Net/SocketStream.h>

extern "C" {
#include <loci/loci.h>
}

using namespace CppUnit;


class SwitchAdapterTests : public CppUnit::TestCase {
CPPUNIT_TEST_SUITE(SwitchAdapterTests);
        CPPUNIT_TEST(testSwitchAdapter);

        //   CPPUNIT_TEST(testMethod2);
        //  CPPUNIT_TEST(testMethod3);
    CPPUNIT_TEST_SUITE_END();

public:
    SwitchAdapterTests();

    void testSwitchAdapter();

    /*
     * Override setup method to set up the testing objects.
     */
    void setUp();

    /*
     * Override tearDown method to release any permanent ressources.
     */
    void tearDown();

    void testHandshake(Poco::Net::SocketStream& str, of_version_t OFVersion);

    void testPacketInOut(Poco::Net::StreamSocket& strSock, Poco::Net::SocketStream& stream);

    void testSpecialCases(Poco::Net::StreamSocket& strSock, Poco::Net::SocketStream& stream);
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(SwitchAdapterTests, SwitchAdapterTests);

#endif //DEMO_MODULE_UT_SWITCH_ADAPTER_H
