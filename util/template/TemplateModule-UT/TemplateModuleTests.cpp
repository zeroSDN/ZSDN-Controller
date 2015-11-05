//
// Created by zsdn on 6/25/15.
//

#include "TemplateModuleTests.h"
TemplateModuleTests::TemplateModuleTests()
{

}

void TemplateModuleTests::testMethod1()
{
    CPPUNIT_ASSERT(true);
}

void TemplateModuleTests::testMethod2()
{
    CPPUNIT_ASSERT(true);
}

void TemplateModuleTests::testMethod3()
{
    CPPUNIT_ASSERT_EQUAL(5,5);
    CPPUNIT_ASSERT(true);

}