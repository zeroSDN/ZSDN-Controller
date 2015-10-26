#include "StatisticsModule.hpp"
#include <LociExtensions.h>
#include <unistd.h>

StatisticsModule::StatisticsModule(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId), MODULE_VERSION, "StatisticsModule",
                       {}), instanceId_(instanceId) {

}


StatisticsModule::~StatisticsModule() {
}


bool StatisticsModule::enable() {
    getLogger().trace("Start enable of the Module.");
    //  subscribe to modules you want to get information about
    bool success = prepareSubscriptions();
    //  if subscription went well, start thread
    if (success) {
        startThread();
    }
    getLogger().information("Successfull enable of Module: " + std::to_string(success));
    return success;
}


void StatisticsModule::disable() {
    getLogger().information("Start disabling Statistics Module");
    if (stopThread()) {
        getLogger().information("Statistics Module sucessfully disabled");
    }
    else {
        getLogger().warning("Statistics Module failed to disable");
    }

}


void StatisticsModule::handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                               zmf::data::ModuleState newState, zmf::data::ModuleState lastState) {
    // Optional: Handle state changes of modules
}

void StatisticsModule::startThread() {
    keepPolling = true;
    pollingThread = new std::thread([this]() {
        std::list<std::shared_ptr<zmf::data::ModuleHandle>> activeSwitches;
        while (keepPolling) {
            getLogger().information("Starting to Request GetAllSwitches from a SwitchRegistryModule.");
            activeSwitches = getZmf()->getPeerRegistry()->getPeersWithType(SWITCH_ADAPTER_MODULE_TYPE_ID, true);
            for (std::shared_ptr<zmf::data::ModuleHandle> switchAdapter : activeSwitches) {

                std::vector<uint8_t> addStateInfo = getZmf()->getPeerRegistry()->getPeerAdditionalState(switchAdapter);
                if (addStateInfo.size() != 0) {
                    of_version_t version = static_cast<of_version_t>(addStateInfo[0]);
                    sendDemandStatisticsRequest(switchAdapter->UniqueId.InstanceId, version);

                }
            }
            activeSwitches.clear();
            sleep(POLLING_INTERVAL_IN_SECONDS);
        }
    });
}

bool StatisticsModule::stopThread() {
    try {
        keepPolling = false;
        pollingThread->join();
        delete pollingThread;
        return true;
    }
    catch (std::exception e) {
        return false;
    }

};

bool StatisticsModule::prepareSubscriptions() {
    getZmf()->subscribe(StatisticSubscribeTopicOF13_,
                        [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                            onStatisticsOF13Reply(msg, sender);
                        });
    getZmf()->subscribe(StatisticSubscribeTopicOF10_,
                        [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                            onStatisticsOF10Reply(msg, sender);
                        });
    return true;
}

void StatisticsModule::onStatisticsOF13Reply(const zmf::data::ZmfMessage& message,
                                             const zmf::data::ModuleUniqueId& sender) {
    //parse message into a statistic-reply
    of_object_t* ofObjectT = zsdn::of_object_new_from_data_string_copy(message.getData());
    if (ofObjectT->version == OF_VERSION_1_3) {

        completeStatistics statistics;
        int statsIndex = -1;
        for (int i = 0; i < allTemporaryStatistics.size(); i++) {
            if (allTemporaryStatistics[i].moduleInstanceID == sender.InstanceId) {
                statsIndex = i;
                statistics = allTemporaryStatistics[i];
                break;
            }
        }
        if (statsIndex == -1) {
            statistics.moduleInstanceID = sender.InstanceId;
        }

        if (ofObjectT->object_id == OF_TABLE_STATS_REPLY) {
            getLogger().information(
                    "received OF13 Table Statistics Reply from Switch: " + std::to_string(sender.InstanceId));
            of_list_table_stats_entry_t* table_stats_entry_t = of_table_stats_reply_entries_get(ofObjectT);
            //of_table_stats_entry_t* statsEntry = of_table_stats_entry_new(ofObjectT->version);
            of_table_stats_entry_t* statsEntry = (of_table_stats_entry_t*) malloc(64);
            int error = of_list_table_stats_entry_first(table_stats_entry_t, statsEntry);
            while (error != OF_ERROR_RANGE) {
                // get information out of each entry
                tableStatistics tempTableStats;
                of_table_stats_entry_active_count_get(statsEntry, &tempTableStats.activeEntries);
                of_table_stats_entry_lookup_count_get(statsEntry, &tempTableStats.PackagesLookedUp);
                of_table_stats_entry_matched_count_get(statsEntry, &tempTableStats.PackagesMatchingTheTable);
                of_table_stats_entry_table_id_get(statsEntry, &tempTableStats.tableID);
                if (ofObjectT->version == OF_VERSION_1_0) {
                    of_table_stats_entry_max_entries_get(statsEntry, &tempTableStats.maxEntries);
                }
                statistics.tableStatsList.push_back(tempTableStats);
                // get next element
                error = of_list_table_stats_entry_next(table_stats_entry_t, statsEntry);
            }
            of_table_stats_entry_delete(statsEntry);
            of_list_table_stats_entry_delete(table_stats_entry_t);
        }
        else if (ofObjectT->object_id == OF_PORT_STATS_REPLY) {
            getLogger().information(
                    "received OF10 Port Statistics Reply from Switch: " + std::to_string(sender.InstanceId));
            of_list_port_stats_entry_t* port_stats_entry_t = of_port_stats_reply_entries_get(ofObjectT);
            //of_port_stats_entry_t* statsEntry = of_port_stats_entry_new(ofObjectT->version);
            of_port_stats_entry_t* statsEntry = (of_port_stats_entry_t*) malloc(64);


            int error = of_list_port_stats_entry_first(port_stats_entry_t, statsEntry);
            while (error != OF_ERROR_RANGE) {
                // get informations out of each entry
                portStatistics tempPort;
                of_port_stats_entry_port_no_get(statsEntry, &tempPort.portNr);
                of_port_stats_entry_rx_packets_get(statsEntry, &tempPort.numberOfPackagesReceived);
                of_port_stats_entry_tx_packets_get(statsEntry, &tempPort.numberOfPackagesSent);
                of_port_stats_entry_rx_dropped_get(statsEntry, &tempPort.numberOfPackagesDropedByReceiver);
                of_port_stats_entry_tx_dropped_get(statsEntry, &tempPort.numberOfPackagesDropedBySender);
                of_port_stats_entry_collisions_get(statsEntry, &tempPort.numberOfCollisions);
                of_port_stats_entry_tx_errors_get(statsEntry, &tempPort.numberOfErrorsFromSender);
                of_port_stats_entry_rx_errors_get(statsEntry, &tempPort.numberOfErrorsFromReceiver);
                of_port_stats_entry_rx_bytes_get(statsEntry, &tempPort.numberOfBytesReceived);
                of_port_stats_entry_tx_bytes_get(statsEntry, &tempPort.numberOfBytesSent);
                statistics.portStatsList.push_back(tempPort);
                // get next element
                error = of_list_port_stats_entry_next(port_stats_entry_t, statsEntry);
            }
            of_list_port_stats_entry_delete(port_stats_entry_t);
            of_port_stats_entry_delete(statsEntry);
        }
        else {
            of_object_delete(ofObjectT);
            return;
        }
        if (statsIndex == -1) {
            allTemporaryStatistics.push_back(statistics);
        }
        else if (statistics.portStatsList.size() >= 1 && statistics.tableStatsList.size() >= 1) {
            sendStatistics(statistics, ofObjectT->version);
            allTemporaryStatistics.erase(allTemporaryStatistics.begin() + statsIndex);
        }
    }
    of_object_delete(ofObjectT);
}


void StatisticsModule::onStatisticsOF10Reply(const zmf::data::ZmfMessage& message,
                                             const zmf::data::ModuleUniqueId& sender) {
    //parse message into a statistic-reply
    of_object_t* ofObjectT = zsdn::of_object_new_from_data_string_copy(message.getData());
    if (ofObjectT->version == OF_VERSION_1_0) {

        completeStatistics statistics;
        int statsIndex = -1;
        for (int i = 0; i < allTemporaryStatistics.size(); i++) {
            if (allTemporaryStatistics[i].moduleInstanceID == sender.InstanceId) {
                statsIndex = i;
                statistics = allTemporaryStatistics[i];
                break;
            }
        }
        if (statsIndex == -1) {
            statistics.moduleInstanceID = sender.InstanceId;
        }

        if (ofObjectT->object_id == OF_TABLE_STATS_REPLY) {
            getLogger().information(
                    "received OF10 Table Statistics Reply from Switch: " + std::to_string(sender.InstanceId));
            of_list_table_stats_entry_t* table_stats_entry_t = of_table_stats_reply_entries_get(ofObjectT);
            //of_table_stats_entry_t* statsEntry = of_table_stats_entry_new(ofObjectT->version);
            of_table_stats_entry_t* statsEntry = (of_table_stats_entry_t*) malloc(64);
            int error = of_list_table_stats_entry_first(table_stats_entry_t, statsEntry);
            while (error != OF_ERROR_RANGE) {
                // get information out of each entry
                tableStatistics tempTableStats;
                of_table_stats_entry_active_count_get(statsEntry, &tempTableStats.activeEntries);
                of_table_stats_entry_lookup_count_get(statsEntry, &tempTableStats.PackagesLookedUp);
                of_table_stats_entry_matched_count_get(statsEntry, &tempTableStats.PackagesMatchingTheTable);
                of_table_stats_entry_table_id_get(statsEntry, &tempTableStats.tableID);
                if (ofObjectT->version == OF_VERSION_1_0) {
                    of_table_stats_entry_max_entries_get(statsEntry, &tempTableStats.maxEntries);
                }
                statistics.tableStatsList.push_back(tempTableStats);
                // get next element
                error = of_list_table_stats_entry_next(table_stats_entry_t, statsEntry);
            }
            of_table_stats_entry_delete(statsEntry);
            of_list_table_stats_entry_delete(table_stats_entry_t);
        }
        else if (ofObjectT->object_id == OF_PORT_STATS_REPLY) {
            getLogger().information(
                    "received OF10 Port Statistics Reply from Switch: " + std::to_string(sender.InstanceId));
            of_list_port_stats_entry_t* port_stats_entry_t = of_port_stats_reply_entries_get(ofObjectT);
            //of_port_stats_entry_t* statsEntry = of_port_stats_entry_new(ofObjectT->version);
            of_port_stats_entry_t* statsEntry = (of_port_stats_entry_t*) malloc(64);


            int error = of_list_port_stats_entry_first(port_stats_entry_t, statsEntry);
            while (error != OF_ERROR_RANGE) {
                // get informations out of each entry
                portStatistics tempPort;
                of_port_stats_entry_port_no_get(statsEntry, &tempPort.portNr);
                of_port_stats_entry_rx_packets_get(statsEntry, &tempPort.numberOfPackagesReceived);
                of_port_stats_entry_tx_packets_get(statsEntry, &tempPort.numberOfPackagesSent);
                of_port_stats_entry_rx_dropped_get(statsEntry, &tempPort.numberOfPackagesDropedByReceiver);
                of_port_stats_entry_tx_dropped_get(statsEntry, &tempPort.numberOfPackagesDropedBySender);
                of_port_stats_entry_collisions_get(statsEntry, &tempPort.numberOfCollisions);
                of_port_stats_entry_tx_errors_get(statsEntry, &tempPort.numberOfErrorsFromSender);
                of_port_stats_entry_rx_errors_get(statsEntry, &tempPort.numberOfErrorsFromReceiver);
                of_port_stats_entry_rx_bytes_get(statsEntry, &tempPort.numberOfBytesReceived);
                of_port_stats_entry_tx_bytes_get(statsEntry, &tempPort.numberOfBytesSent);
                statistics.portStatsList.push_back(tempPort);
                // get next element
                error = of_list_port_stats_entry_next(port_stats_entry_t, statsEntry);
            }
            of_list_port_stats_entry_delete(port_stats_entry_t);
            of_port_stats_entry_delete(statsEntry);
        }
        else {
            of_object_delete(ofObjectT);
            return;
        }
        if (statsIndex == -1) {
            allTemporaryStatistics.push_back(statistics);
        }
        else if (statistics.portStatsList.size() >= 1 && statistics.tableStatsList.size() >= 1) {
            sendStatistics(statistics, ofObjectT->version);
            allTemporaryStatistics.erase(allTemporaryStatistics.begin() + statsIndex);
        }
    }
    of_object_delete(ofObjectT);
}

void StatisticsModule::sendDemandStatisticsRequest(uint64_t switchID, of_version_t& ofVersion) {

    if (ofVersion == OF_VERSION_1_0) {
        of_table_stats_request_t* tableStatReq = of_table_stats_request_new(ofVersion);
        std::string tableStatReq_serialized = zsdn::of_object_serialize_to_data_string(tableStatReq);

        zmf::data::MessageType tableStatMsgType = switchadapter_topics::TO().switch_adapter().switch_instance(
                switchID).openflow().of_1_0_stats_request_of_1_3_port_mod().build();
        getZmf()->publish(zmf::data::ZmfMessage(tableStatMsgType, tableStatReq_serialized));
        of_object_delete(tableStatReq);
        getLogger().information("sent OF10 Table Statistics Request to Switch: " + std::to_string(switchID));

        of_port_stats_request_t* portStatReq = of_port_stats_request_new(ofVersion);
        // 0xffff is OFPP_NONE in of10 which means "all ports"
        of_port_stats_request_port_no_set(portStatReq, 0xffff);
        std::string portStatReq_serialized = zsdn::of_object_serialize_to_data_string(portStatReq);
        zmf::data::MessageType portStatMsgType = switchadapter_topics::TO().switch_adapter().switch_instance(
                switchID).openflow().of_1_0_stats_request_of_1_3_port_mod().build();
        getZmf()->publish(zmf::data::ZmfMessage(portStatMsgType, portStatReq_serialized));
        of_object_delete(portStatReq);
        getLogger().information("sent OF10 Port Statistics Request to Switch: " + std::to_string(switchID));
    }

    if (ofVersion == OF_VERSION_1_3) {
        of_table_stats_request_t* tableStatReq = of_table_stats_request_new(ofVersion);
        std::string tableStatReq_serialized = zsdn::of_object_serialize_to_data_string(tableStatReq);

        zmf::data::MessageType tableStatMsgType = switchadapter_topics::TO().switch_adapter().switch_instance(
                switchID).openflow().of_1_0_barrier_request_of_1_3_multipart_request().build();
        getZmf()->publish(zmf::data::ZmfMessage(tableStatMsgType, tableStatReq_serialized));
        of_object_delete(tableStatReq);
        getLogger().information("sent OF13 Table Statistics Request to Switch: " + std::to_string(switchID));

        of_port_stats_request_t* portStatReq = of_port_stats_request_new(ofVersion);
        // 0xffffffff is OFPP_ANY in of13 which means "all ports"
        of_port_stats_request_port_no_set(portStatReq, 0xffffffff);
        std::string portStatReq_serialized = zsdn::of_object_serialize_to_data_string(portStatReq);
        zmf::data::MessageType portStatMsgType = switchadapter_topics::TO().switch_adapter().switch_instance(
                switchID).openflow().of_1_0_barrier_request_of_1_3_multipart_request().build();
        getZmf()->publish(zmf::data::ZmfMessage(portStatMsgType, portStatReq_serialized));
        of_object_delete(portStatReq);
        getLogger().information("sent OF13 Port Statistics Request to Switch: " + std::to_string(switchID));
    }

}

void StatisticsModule::sendStatistics(StatisticsModule::completeStatistics statistics, of_version_t ofVersion) {
    StatisticsModule_Proto::From fromMessage;
    StatisticsModule_Proto::From_SwitchStats* switchStats = new StatisticsModule_Proto::From_SwitchStats();
    switchStats->set_uniqueid(statistics.moduleInstanceID);
    StatisticsModule_Proto::From_PortStatslist* portStatslist = new StatisticsModule_Proto::From_PortStatslist();
    for (portStatistics portStats : statistics.portStatsList) {
        StatisticsModule_Proto::From_SinglePortStatistics* singlePortStatistics = portStatslist->add_portstats();
        singlePortStatistics->set_numberofcollisions(portStats.numberOfCollisions);
        singlePortStatistics->set_numberoferrorsfromreceiver(portStats.numberOfErrorsFromReceiver);
        singlePortStatistics->set_numberoferrorsfromsender(portStats.numberOfErrorsFromSender);
        singlePortStatistics->set_numberofpackagesdropedbyreceiver(portStats.numberOfPackagesDropedByReceiver);
        singlePortStatistics->set_numberofpackagesdropedbysender(portStats.numberOfPackagesDropedBySender);
        singlePortStatistics->set_numberofpackagesreceived(portStats.numberOfPackagesReceived);
        singlePortStatistics->set_numberofpackagessent(portStats.numberOfPackagesSent);
        singlePortStatistics->set_numberofbytesreceived(portStats.numberOfBytesReceived);
        singlePortStatistics->set_numberofbytessent(portStats.numberOfBytesSent);
        singlePortStatistics->set_portnumber(portStats.portNr);
    }
    switchStats->set_allocated_portstats(portStatslist);
    StatisticsModule_Proto::From_TableStatslist* tableStatslist = new StatisticsModule_Proto::From_TableStatslist();
    for (tableStatistics tableStats : statistics.tableStatsList) {
        StatisticsModule_Proto::From_SingleTableStatistics* singleTableStatistics = tableStatslist->add_tablestats();
        singleTableStatistics->set_activeentries(tableStats.activeEntries);
        singleTableStatistics->set_packageshittingthetable(tableStats.PackagesMatchingTheTable);
        singleTableStatistics->set_packageslookedup(tableStats.PackagesLookedUp);
        singleTableStatistics->set_tableid(tableStats.tableID);
        // maxEntries == 0 if there is no MaxEntries set. thats the case in of_version 1.3
        if (tableStats.maxEntries != 0) {
            singleTableStatistics->set_maxentries(tableStats.maxEntries);
        }
    }
    switchStats->set_allocated_tablestats(tableStatslist);
    fromMessage.set_allocated_stats(switchStats);
    getZmf()->publish(zmf::data::ZmfMessage(PublishNewStatisticReceivedTopic_, fromMessage.SerializeAsString()));

}
