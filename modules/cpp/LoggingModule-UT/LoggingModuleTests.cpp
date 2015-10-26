//
// Created by zsdn on 6/25/15.
//

#include "LoggingModuleTests.h"
LoggingModuleTests::LoggingModuleTests()
{

}

void LoggingModuleTests::testMethod1()
{
    CPPUNIT_ASSERT(true);
}

void LoggingModuleTests::testMethod2()
{
    CPPUNIT_ASSERT(true);
}

void LoggingModuleTests::testMethod3()
{
    CPPUNIT_ASSERT_EQUAL(i,5);
    CPPUNIT_ASSERT(true);

}