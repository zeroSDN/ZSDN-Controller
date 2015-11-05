/**
 * @brief   A Module that tests the StatisticsModule
 *
 * @details This Module sends statistics to a StatisticsModule after receiving the pollrequest for statistics.
 * Additionaly it subscribes to the published answer from the Statistics Module.
 * Then it checks if both statistics are the sames
 *
 * @author  Tobias Korb
 *
 * @date    28.06.2015
*/

#ifndef DEMO_MODULE_UT_SWITCH_ADAPTER_TESTS_H
#define DEMO_MODULE_UT_SWITCH_ADAPTER_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <zmf/AbstractModule.hpp>
#include <thread>
#include <map>
#include <StatisticsModule.hpp>
#include <zsdn/topics/SwitchRegistryModuleTopics.hpp>
#include "zsdn/topics/SwitchAdapterTopics.hpp"
#include <zsdn/topics/StatisticsModuleTopics.hpp>
#include <zsdn/proto/StatisticsModule.pb.h>
#include <ModuleTypeIDs.hpp>
#include <zmf/IZmfInstanceController.hpp>

using namespace CppUnit;

const uint32_t StatisticModuleId = 1;
const uint32_t SwitchAdapterModuleMockId = 2;
const uint32_t SwitchAdapterModuleMock2Id = 3;
const uint32_t TestingModuleID = 3;

class StatisticsModuleTests : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(StatisticsModuleTests);
        CPPUNIT_TEST(checkValues);
    CPPUNIT_TEST_SUITE_END();

public:

    /**
 * Constructs StatisticsModuleTests.
 */
    StatisticsModuleTests();

/**
     * checks if statistics sent and recevied are the same
     */
    void checkValues();

    /*
   * Override setup method to set up the testing objects.
   */
    void setUp();

    /*
     * Override tearDown method to release any permanent ressources.
     */
    void tearDown();


    class SwitchAdapterModuleMock : public zmf::AbstractModule {

    public:
        /**
     * Constructs (but does not start) a switchadapter mock.
     *
     * @param instanceId
     *        the switchId of the switch represented by this switchadapter mock.
     */
        SwitchAdapterModuleMock(uint64_t instanceId);

        /**
         * Destructor in case the Module isnt disabled correctly
         */
        ~SwitchAdapterModuleMock();

    protected:
        /**
     * Enables the ModuleMock.
     *
     * @return true if enable was successful.
     */
        virtual bool enable();

    public:
        /**
  * Disables the ModuleMock.
  *
  * @return true if disable was successful.
  */
        virtual void disable();


    private:
        static const uint16_t MODULE_VERSION = 0;
        static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_SwitchAdapter;
        uint64_t instanceId_;

        zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType> switchAdapterTopics_;

        zmf::data::MessageType StatReplyOF13MsgType = switchAdapterTopics_.from().switch_adapter().openflow().of_1_0_barrier_reply_of_1_3_multipart_reply().build();
        zmf::data::MessageType StatReplyOF10MsgType = switchAdapterTopics_.from().switch_adapter().openflow().of_1_0_stats_reply_of_1_3_table_mod().build();
    };


    class TestStatisticsModule : public zmf::AbstractModule {

    public:
        static const uint16_t STATISTICS_MODULE_DEP_TYPE = zsdn::MODULE_TYPE_ID_StatisticsModule;
        static const uint16_t STATISTICS_MODULE_DEP_VERSION = 0;

        /**
          * Constructs (but does not start) a switchadapter mock.
          *
          * @param instanceId
          *        the switchId of the switch represented by this switchadapter mock.
          */
        TestStatisticsModule(uint64_t instanceId);


        /**
         * Destructor in case the Module isnt disabled correctly
         */
        ~TestStatisticsModule();

    protected:
        /**
  * Enables the TestStatisticsModule.
  *
  * @return true if enable was successful.
  */
        virtual bool enable();

    public:
        /**
* Disables the TestStatisticsModule.
*
* @return true if disable was successful.
*/
        virtual void disable();

        struct PortStatistics {
            uint32_t portNr;
            uint64_t numberOfPackagesReceived;
            uint64_t numberOfPackagesSent;
            uint64_t numberOfPackagesDropedByReceiver;
            uint64_t numberOfPackagesDropedBySender;
            uint64_t numberOfCollisions;
            uint64_t numberOfErrorsFromSender;
            uint64_t numberOfErrorsFromReceiver;
            uint64_t numberOfBytesSent;
            uint64_t numberOfBytesReceived;
        };

        struct TableStatistics {
            uint8_t tableID;
            uint32_t activeEntries;
            uint64_t PackagesLookedUp;
            uint64_t PackagesMatchingTheTable;
            uint32_t maxEntries;

        };

        struct CompleteStatistics {
            uint64_t moduleID;
            std::vector<PortStatistics> portStatsList;
            std::vector<TableStatistics> tableStatsList;
        };
        std::vector<CompleteStatistics> completeStatisticsList = std::vector<CompleteStatistics>();


        /**
     * checks if statistics sent and recevied are the same
         *
         * @return true if values sent are equal to values received
     */
        bool checkValues() {
            if (completeStatisticsList.size() == 0) {
                return false;
            }

            for (int i = 0; i < completeStatisticsList.size(); i++) {
                bool idCheck;
                idCheck = completeStatisticsList[i].moduleID == SwitchAdapterModuleMockId ||
                          completeStatisticsList[i].moduleID == SwitchAdapterModuleMock2Id;
                if (!idCheck) {
                    return false;
                }

                bool portCheck;
                for (int j = 0; j < completeStatisticsList[i].portStatsList.size(); j++) {
                    portCheck = (completeStatisticsList[i].portStatsList[j].numberOfPackagesReceived == 12 &&
                                 completeStatisticsList[i].portStatsList[j].numberOfCollisions == 16 &&
                                 completeStatisticsList[i].portStatsList[j].numberOfErrorsFromReceiver == 18 &&
                                 completeStatisticsList[i].portStatsList[j].numberOfErrorsFromSender == 17 &&
                                 completeStatisticsList[i].portStatsList[j].numberOfPackagesDropedByReceiver == 14 &&
                                 completeStatisticsList[i].portStatsList[j].numberOfPackagesDropedBySender == 15 &&
                                 completeStatisticsList[i].portStatsList[j].numberOfPackagesSent == 13 &&
                                 completeStatisticsList[i].portStatsList[j].portNr == 11 &&
                                 completeStatisticsList[i].portStatsList[j].numberOfBytesSent == 20 &&
                                 completeStatisticsList[i].portStatsList[j].numberOfBytesReceived == 19);
                    if (!portCheck) {
                        return false;
                    }
                }
                bool tableCheck;
                for (int j = 0; j < completeStatisticsList[i].portStatsList.size(); j++) {
                    tableCheck = (completeStatisticsList[i].tableStatsList[j].activeEntries == 01 &&
                                  completeStatisticsList[i].tableStatsList[j].PackagesLookedUp == 02 &&
                                  completeStatisticsList[i].tableStatsList[j].PackagesMatchingTheTable == 03 &&
                                  completeStatisticsList[i].tableStatsList[j].tableID == 04 &&
                                  (completeStatisticsList[i].tableStatsList[j].maxEntries == 05 ||
                                   completeStatisticsList[i].moduleID == SwitchAdapterModuleMockId));
                    //  SwitchAdapterModuleMockId supports of13 which has no maxEntries
                    if (!tableCheck) {
                        return false;
                    }
                }
            }
            return true;

        }

    private:
        static const uint16_t MODULE_VERSION = 0;
        static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_DemoModule;
        uint64_t instanceId_;


    };


};

#endif //DEMO_MODULE_UT_SWITCH_ADAPTER_H
