#include <zmf/ZmfInstance.hpp>
#include <LociExtensions.h>
#include "StatisticsModuleTests.h"
#include <UnittestConfigUtil.hpp>


std::shared_ptr<StatisticsModuleTests::SwitchAdapterModuleMock> switchAdapterMock;
std::shared_ptr<zmf::IZmfInstanceController> switchAdapterMockInstance;

std::shared_ptr<StatisticsModuleTests::SwitchAdapterModuleMock> switchAdapterMock2;
std::shared_ptr<zmf::IZmfInstanceController> switchAdapterMock2Instance;

std::shared_ptr<StatisticsModuleTests::TestStatisticsModule> testingModule;
std::shared_ptr<zmf::IZmfInstanceController> testingModuleInstance;

std::shared_ptr<StatisticsModule> statisticsModule;
std::shared_ptr<zmf::IZmfInstanceController> statisticsModuleInstance;

StatisticsModuleTests::StatisticsModuleTests() {

}

void StatisticsModuleTests::checkValues() {

    CPPUNIT_ASSERT(testingModule->checkValues());
}

void StatisticsModuleTests::setUp() {
    testingModule = std::shared_ptr<StatisticsModuleTests::TestStatisticsModule>(
            new StatisticsModuleTests::TestStatisticsModule(TestingModuleID));
    testingModuleInstance = zmf::instance::ZmfInstance::startInstance(testingModule, {}, UT_CONFIG_FILE);
    while (!testingModule->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    switchAdapterMock = std::shared_ptr<StatisticsModuleTests::SwitchAdapterModuleMock>(
            new StatisticsModuleTests::SwitchAdapterModuleMock(SwitchAdapterModuleMockId));
    switchAdapterMockInstance = zmf::instance::ZmfInstance::startInstance(switchAdapterMock, {}, UT_CONFIG_FILE);

    while (!switchAdapterMock->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::vector<uint8_t> version;
    version.push_back(OF_VERSION_1_3);
    switchAdapterMock->getZmf()->onModuleAdditionalStateChanged(version);


    switchAdapterMock2 = std::shared_ptr<StatisticsModuleTests::SwitchAdapterModuleMock>(
            new StatisticsModuleTests::SwitchAdapterModuleMock(SwitchAdapterModuleMock2Id));
    switchAdapterMock2Instance = zmf::instance::ZmfInstance::startInstance(switchAdapterMock2, {}, UT_CONFIG_FILE);

    while (!switchAdapterMock2->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::vector<uint8_t> version2;
    version2.push_back(OF_VERSION_1_0);
    switchAdapterMock2->getZmf()->onModuleAdditionalStateChanged(version2);


    statisticsModule = std::shared_ptr<StatisticsModule>(new StatisticsModule(StatisticModuleId));
    statisticsModuleInstance = zmf::instance::ZmfInstance::startInstance(statisticsModule, {}, UT_CONFIG_FILE);
    while (!statisticsModule->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

void StatisticsModuleTests::tearDown() {
    testingModuleInstance->stopInstance();
    switchAdapterMockInstance->stopInstance();
    switchAdapterMock2Instance->stopInstance();
    statisticsModuleInstance->stopInstance();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

bool StatisticsModuleTests::SwitchAdapterModuleMock::enable() {
    //OF13
    this->getZmf()->subscribe(

            switchadapter_topics::TO().switch_adapter().switch_instance(
                    getUniqueId().InstanceId).openflow().of_1_0_barrier_request_of_1_3_multipart_request().build(),
            [this](const zmf::data::ZmfMessage& msg, const ModuleUniqueId& sender) {
                of_object_t* ofObjectT = zsdn::of_object_new_from_data_string_copy(msg.getData());
                if (ofObjectT->version == OF_VERSION_1_3) {
                    if (ofObjectT->object_id == OF_TABLE_STATS_REQUEST) {
                        of_table_stats_reply_t* statsReply;
                        statsReply = of_table_stats_reply_new(OF_VERSION_1_3);
                        of_list_table_stats_entry_t* table_stats_entry_List_t;
                        table_stats_entry_List_t = of_list_table_stats_entry_new(OF_VERSION_1_3);
                        for (int i = 0; i < 5; i++) {
                            of_table_stats_entry_t* statsEntry;
                            statsEntry = of_table_stats_entry_new(OF_VERSION_1_3);
                            //fill Entry
                            of_table_stats_entry_active_count_set(statsEntry, 01);
                            of_table_stats_entry_lookup_count_set(statsEntry, 02);
                            of_table_stats_entry_matched_count_set(statsEntry, 03);
                            of_table_stats_entry_table_id_set(statsEntry, 04);
                            of_list_table_stats_entry_append(table_stats_entry_List_t, statsEntry);
                            of_table_stats_entry_delete(statsEntry);
                        }
                        of_table_stats_reply_entries_set(statsReply, table_stats_entry_List_t);
                        std::string tableStatsReply_serialized = zsdn::of_object_serialize_to_data_string(statsReply);
                        of_list_table_stats_entry_delete(table_stats_entry_List_t);
                        of_table_stats_reply_delete(statsReply);
                        getZmf()->publish(zmf::data::ZmfMessage(StatReplyOF13MsgType, tableStatsReply_serialized));
                    }
                    else if (ofObjectT->object_id == OF_PORT_STATS_REQUEST) {
                        // publish wrong messages to check behavier of statisticsModule
                        // 1.: wrong topic
                        of_port_desc_stats_reply_t* desc_statsReply;
                        desc_statsReply = of_port_desc_stats_reply_new(OF_VERSION_1_3);
                        std::string wrongReply_ser = zsdn::of_object_serialize_to_data_string(desc_statsReply);
                        of_port_desc_stats_reply_delete(desc_statsReply);
                        getZmf()->publish(zmf::data::ZmfMessage(StatReplyOF13MsgType, wrongReply_ser));
                        // 2.: wrong openflow version
                        of_port_stats_reply_t* desc_statsReply2;
                        desc_statsReply2 = of_port_stats_reply_new(OF_VERSION_1_1);
                        std::string wrongReply_ser2 = zsdn::of_object_serialize_to_data_string(desc_statsReply2);
                        of_port_stats_reply_delete(desc_statsReply2);
                        getZmf()->publish(zmf::data::ZmfMessage(StatReplyOF13MsgType, wrongReply_ser2));
                        // start building right response
                        of_port_stats_reply_t* statsReply;
                        statsReply = of_port_stats_reply_new(OF_VERSION_1_3);
                        of_list_port_stats_entry_t* port_stats_entry_List_t;
                        port_stats_entry_List_t = of_list_port_stats_entry_new(OF_VERSION_1_3);
                        for (int i = 0; i < 5; i++) {
                            of_port_stats_entry_t* statsEntry;
                            statsEntry = of_port_stats_entry_new(OF_VERSION_1_3);
                            //fill Entry
                            of_port_stats_entry_port_no_set(statsEntry, 11);
                            of_port_stats_entry_rx_packets_set(statsEntry, 12);
                            of_port_stats_entry_tx_packets_set(statsEntry, 13);
                            of_port_stats_entry_rx_dropped_set(statsEntry, 14);
                            of_port_stats_entry_tx_dropped_set(statsEntry, 15);
                            of_port_stats_entry_collisions_set(statsEntry, 16);
                            of_port_stats_entry_tx_errors_set(statsEntry, 17);
                            of_port_stats_entry_rx_errors_set(statsEntry, 18);
                            of_port_stats_entry_rx_bytes_set(statsEntry, 19);
                            of_port_stats_entry_tx_bytes_set(statsEntry, 20);
                            of_list_port_stats_entry_append(port_stats_entry_List_t, statsEntry);
                            of_port_stats_entry_delete(statsEntry);
                        }
                        of_port_stats_reply_entries_set(statsReply, port_stats_entry_List_t);
                        std::string portStatsReply_serialized = zsdn::of_object_serialize_to_data_string(statsReply);
                        of_list_port_stats_entry_delete(port_stats_entry_List_t);
                        of_port_stats_reply_delete(statsReply);
                        getZmf()->publish(zmf::data::ZmfMessage(StatReplyOF13MsgType, portStatsReply_serialized));
                    }
                }
                of_object_delete(ofObjectT);
            }

    );

    //OF10
    this->getZmf()->subscribe(

            switchadapter_topics::TO().switch_adapter().switch_instance(
                    getUniqueId().InstanceId).openflow().of_1_0_stats_request_of_1_3_port_mod().build(),
            [this](const zmf::data::ZmfMessage& msg, const ModuleUniqueId& sender) {
                of_object_t* ofObjectT = zsdn::of_object_new_from_data_string_copy(msg.getData());
                if (ofObjectT->version == OF_VERSION_1_0) {
                    if (ofObjectT->object_id == OF_TABLE_STATS_REQUEST) {
                        of_table_stats_reply_t* statsReply;
                        statsReply = of_table_stats_reply_new(OF_VERSION_1_0);
                        of_list_table_stats_entry_t* table_stats_entry_List_t;
                        table_stats_entry_List_t = of_list_table_stats_entry_new(OF_VERSION_1_0);
                        for (int i = 0; i < 5; i++) {
                            of_table_stats_entry_t* statsEntry;
                            statsEntry = of_table_stats_entry_new(OF_VERSION_1_0);
                            //fill Entry
                            of_table_stats_entry_active_count_set(statsEntry, 01);
                            of_table_stats_entry_lookup_count_set(statsEntry, 02);
                            of_table_stats_entry_matched_count_set(statsEntry, 03);
                            of_table_stats_entry_table_id_set(statsEntry, 04);
                            of_table_stats_entry_max_entries_set(statsEntry, 05);
                            of_list_table_stats_entry_append(table_stats_entry_List_t, statsEntry);
                            of_table_stats_entry_delete(statsEntry);
                        }
                        of_table_stats_reply_entries_set(statsReply, table_stats_entry_List_t);
                        std::string tableStatsReply_serialized = zsdn::of_object_serialize_to_data_string(statsReply);
                        of_list_table_stats_entry_delete(table_stats_entry_List_t);
                        of_table_stats_reply_delete(statsReply);
                        getZmf()->publish(zmf::data::ZmfMessage(StatReplyOF10MsgType, tableStatsReply_serialized));
                    }
                    else if (ofObjectT->object_id == OF_PORT_STATS_REQUEST) {
                        // publish wrong messages to check behavier of statisticsModule
                        // 1.: wrong topic
                        /*of_port_desc_stats_reply_t* desc_statsReply;
                        desc_statsReply = of_port_desc_stats_reply_new(OF_VERSION_1_0);
                        std::string wrongReply_ser = zsdn::of_object_serialize_to_data_string(desc_statsReply);
                        of_port_desc_stats_reply_delete(desc_statsReply);
                        getZmf()->publish(zmf::data::ZmfMessage(StatReplyOF10MsgType, wrongReply_ser));*/
                        // 2.: wrong openflow version
                        of_port_stats_reply_t* desc_statsReply2;
                        desc_statsReply2 = of_port_stats_reply_new(OF_VERSION_1_1);
                        std::string wrongReply_ser2 = zsdn::of_object_serialize_to_data_string(desc_statsReply2);
                        of_port_stats_reply_delete(desc_statsReply2);
                        getZmf()->publish(zmf::data::ZmfMessage(StatReplyOF10MsgType, wrongReply_ser2));
                        // start building right response
                        of_port_stats_reply_t* statsReply;
                        statsReply = of_port_stats_reply_new(OF_VERSION_1_0);
                        of_list_port_stats_entry_t* port_stats_entry_List_t;
                        port_stats_entry_List_t = of_list_port_stats_entry_new(OF_VERSION_1_0);
                        for (int i = 0; i < 5; i++) {
                            of_port_stats_entry_t* statsEntry;
                            statsEntry = of_port_stats_entry_new(OF_VERSION_1_0);
                            //fill Entry
                            of_port_stats_entry_port_no_set(statsEntry, 11);
                            of_port_stats_entry_rx_packets_set(statsEntry, 12);
                            of_port_stats_entry_tx_packets_set(statsEntry, 13);
                            of_port_stats_entry_rx_dropped_set(statsEntry, 14);
                            of_port_stats_entry_tx_dropped_set(statsEntry, 15);
                            of_port_stats_entry_collisions_set(statsEntry, 16);
                            of_port_stats_entry_tx_errors_set(statsEntry, 17);
                            of_port_stats_entry_rx_errors_set(statsEntry, 18);
                            of_port_stats_entry_rx_bytes_set(statsEntry, 19);
                            of_port_stats_entry_tx_bytes_set(statsEntry, 20);
                            of_list_port_stats_entry_append(port_stats_entry_List_t, statsEntry);
                            of_port_stats_entry_delete(statsEntry);
                        }
                        of_port_stats_reply_entries_set(statsReply, port_stats_entry_List_t);
                        std::string portStatsReply_serialized = zsdn::of_object_serialize_to_data_string(statsReply);
                        of_list_port_stats_entry_delete(port_stats_entry_List_t);
                        of_port_stats_reply_delete(statsReply);
                        getZmf()->publish(zmf::data::ZmfMessage(StatReplyOF10MsgType, portStatsReply_serialized));
                    }
                }
                of_object_delete(ofObjectT);
            }

    );

    return true;
}

void StatisticsModuleTests::SwitchAdapterModuleMock::disable() {

}

StatisticsModuleTests::SwitchAdapterModuleMock::SwitchAdapterModuleMock(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
                       MODULE_VERSION, "SwitchAdapterModuleMock", {}),
        instanceId_(instanceId) {
}

StatisticsModuleTests::SwitchAdapterModuleMock::~SwitchAdapterModuleMock() {

}

StatisticsModuleTests::TestStatisticsModule::TestStatisticsModule(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
                       MODULE_VERSION, "StatisticsTesterModule", {}),
        instanceId_(instanceId) {
}

StatisticsModuleTests::TestStatisticsModule::~TestStatisticsModule() {

}

void StatisticsModuleTests::TestStatisticsModule::disable() {

}

bool StatisticsModuleTests::TestStatisticsModule::enable() {
    this->getZmf()->subscribe(
            statisticsmodule_topics::FROM().statistics_module().statistic_event().added().build(),
            [this](const zmf::data::ZmfMessage& msg, const ModuleUniqueId& sender) {
                StatisticsModule_Proto::From statsProto;
                bool sucess = statsProto.ParseFromString(msg.getData());
                if (sucess) {
                    CompleteStatistics completeStatistics = CompleteStatistics();
                    StatisticsModule_Proto::From_SwitchStats switchStats = statsProto.stats();
                    completeStatistics.moduleID = switchStats.uniqueid();
                    StatisticsModule_Proto::From_PortStatslist portStatslist = switchStats.portstats();
                    StatisticsModule_Proto::From_TableStatslist tableStatslist = switchStats.tablestats();
                    for (int i = 0; i < portStatslist.portstats_size(); i++) {
                        StatisticsModule_Proto::From_SinglePortStatistics singlePortStatistics = portStatslist.portstats(
                                i);
                        PortStatistics portStatistics = PortStatistics();
                        portStatistics.numberOfCollisions = singlePortStatistics.numberofcollisions();
                        portStatistics.numberOfErrorsFromReceiver = singlePortStatistics.numberoferrorsfromreceiver();
                        portStatistics.numberOfErrorsFromSender = singlePortStatistics.numberoferrorsfromsender();
                        portStatistics.numberOfPackagesDropedByReceiver = singlePortStatistics.numberofpackagesdropedbyreceiver();
                        portStatistics.numberOfPackagesDropedBySender = singlePortStatistics.numberofpackagesdropedbysender();
                        portStatistics.numberOfPackagesReceived = singlePortStatistics.numberofpackagesreceived();
                        portStatistics.numberOfPackagesSent = singlePortStatistics.numberofpackagessent();
                        portStatistics.portNr = singlePortStatistics.portnumber();
                        portStatistics.numberOfBytesSent = singlePortStatistics.numberofbytessent();
                        portStatistics.numberOfBytesReceived = singlePortStatistics.numberofbytesreceived();
                        completeStatistics.portStatsList.push_back(portStatistics);
                    }
                    for (int i = 0; i < tableStatslist.tablestats_size(); i++) {
                        StatisticsModule_Proto::From_SingleTableStatistics singleTableStatistics = tableStatslist.tablestats(
                                i);
                        TableStatistics tableStatistics = TableStatistics();
                        tableStatistics.activeEntries = singleTableStatistics.activeentries();
                        tableStatistics.PackagesLookedUp = singleTableStatistics.packageslookedup();
                        tableStatistics.PackagesMatchingTheTable = singleTableStatistics.packageshittingthetable();
                        tableStatistics.tableID = singleTableStatistics.tableid();
                        tableStatistics.maxEntries = singleTableStatistics.maxentries();
                        completeStatistics.tableStatsList.push_back(tableStatistics);
                    }
                    completeStatisticsList.push_back(completeStatistics);
                }
            });
    return true;
}

