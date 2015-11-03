#ifndef StatisticsModule_H
#define StatisticsModule_H

#include <zmf/AbstractModule.hpp>
#include <thread>
#include <zsdn/proto/CommonTopology.pb.h>
#include <zsdn/topics/SwitchRegistryModuleTopics.hpp>
#include <zsdn/topics/SwitchAdapterTopics.hpp>
#include <zsdn/topics/StatisticsModuleTopics.hpp>
#include "zsdn/proto/StatisticsModule.pb.h"
#include <loci/loci_base.h>
#include <loci/of_match.h>
#include <ModuleTypeIDs.hpp>

/**
 * @brief   A Module that gathers informations of all Switches.
 *
 * @details The StatisticsModule sends out pulling requests every few seconds. On Response, it extracts important
 * statistics and publishes them to everyone that is intressted in them
 *
 * @author  Tobias Korb
 *
 * @date    28.06.2015
*/
class StatisticsModule : public zmf::AbstractModule {

public:
    /**
     * Constructs (but does not start) A StatisticsModule.
     *
     * @param instanceId
     *        the switchId of the switch represented by this StatisticsModule.
     */
    StatisticsModule(uint64_t instanceId);

    /**
     * Destructor in case the Module isnt disabled correctly
     */
    ~StatisticsModule();


    /**
     * Optional: Handle state changes of other modules
     */
    virtual void handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                         zmf::data::ModuleState newState, zmf::data::ModuleState lastState) override;

protected:
    /**
     * CALLED, BY CORE, DONT CALL IT FROM MODULE. MODULE CAN NOT INFLUENCE ENABLING
     * Called when the module should enable itself. Must initialize and start the module.
     * @return True if enable successful, False if enable failed or rejected
     */
    virtual bool enable();

    /**
     * CALLED, BY CORE, DONT CALL IT FROM MODULE. USE getZmf()->requestDisable
     * Called when the module should disable itself. Must stop the module (threads etc.).
     */
    virtual void disable();


private:
    static const uint16_t SWITCH_REGISTRY_MODULE_DEP_VERSION = 0;
    static const uint16_t MODULE_VERSION = 0;
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_StatisticsModule;
    static const uint16_t SWITCH_ADAPTER_MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_SwitchAdapter;
    static const uint16_t POLLING_INTERVAL_IN_SECONDS = 5;

    /// Unique instance id fo the module inside the ZMF system.
    uint64_t instanceId_;

    std::thread* pollingThread;

    std::atomic<bool> keepPolling;

    // FIXME ONLY WORKS WITH OF_13
    // Builder for topic creation
    zsdn::modules::StatisticsModuleTopics<zmf::data::MessageType> statisticsModuleTopics_;
    // Builder for topic creation. used by multiple threads, however not concurrently.
    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType> switchAdapterTopics_;

    zmf::data::MessageType StatisticSubscribeTopicOF13_ = switchAdapterTopics_.from().switch_adapter().openflow().of_1_0_barrier_reply_of_1_3_multipart_reply().build();
    zmf::data::MessageType StatisticSubscribeTopicOF10_ = switchAdapterTopics_.from().switch_adapter().openflow().of_1_0_stats_reply_of_1_3_table_mod().build();
    zmf::data::MessageType PublishNewStatisticReceivedTopic_ = statisticsModuleTopics_.from().statistics_module().statistic_event().added().build();


    /**
     * This Method starts the polling Thread for statistics
     */
    void startThread();

    /**
     * This method prepares subscruptions needed for this module to work.
     *
     * @return true if all subscriptions are created successfully.
     */
    bool prepareSubscriptions();


    /**
     * This method is called when a statisticsReply from openflow version 1.3 is received.
     *
     * @param message the received message.
     * @param sender the unique id of the module that sent the message.
     *
     */
    void onStatisticsOF13Reply(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender);


    /**
    * This method is called when a statisticsReply from openflow version 1.0 is received.
    *
    * @param message the received message.
    * @param sender the unique id of the module that sent the message.
    *
    */
    void onStatisticsOF10Reply(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender);

    /**
     * This method sends a single statistics request to a specific switch
     *
     * @param switchID the id of the switch the request is send to
     * @param ofVersion the openflow version of the module that this request is send to
     *
     */
    void sendDemandStatisticsRequest(uint64_t switchID, of_version_t& ofVersion);


    /**
     * This method tries to stop the polling thread
     *
     * @return true if the polling thread was stoped succesfully
     *
     */
    bool stopThread();


    struct portStatistics {
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

    struct tableStatistics {
        uint8_t tableID;
        uint32_t activeEntries;
        uint64_t PackagesLookedUp;
        uint64_t PackagesMatchingTheTable;
        uint32_t maxEntries;

    };

    struct completeStatistics {
        uint64_t moduleInstanceID;
        std::vector<portStatistics> portStatsList;
        std::vector<tableStatistics> tableStatsList;
    };

    std::vector<completeStatistics> allTemporaryStatistics;


    /**
     * This method publishes received statistics
     *
     * @param statistics the statistics gathered that will be send
     *
     */
    void sendStatistics(completeStatistics statistics, of_version_t ofVersion);

};


#endif // StatisticsModule_H
