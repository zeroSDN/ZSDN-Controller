#ifndef LoggingModule_H
#define LoggingModule_H

#include <zmf/AbstractModule.hpp>
#include <zsdn/Configs.h>
#include <thread>
#include <cassandra.h>
#include "CassandraClient.h"

/**
 * @brief This is the Logging Module.
 * @details All ZMF Events (only the pub/sub ones) will be stored in a database. The Logging Module will subscribe to all Events and write them to the database.
 * @see CassandraClient
 * @author Rainer Erban
 */
class LoggingModule : public zmf::AbstractModule {

public:
    /**
     * Constructor with instance id
     * @param instanceId the id of the instance
     */
    LoggingModule(uint64_t instanceId);

    /**
     * Default Constructor
     */
    ~LoggingModule();

    /**
     * Optional: Handle state changes of other modules
     * @param changedModule the module which change his state
     * @param newState the new state of the module
     * @param lastState the old state of the module
     */
    virtual void handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                         zmf::data::ModuleState newState, zmf::data::ModuleState lastState) override;


protected:
    /**
     * Called when the module should enable itself. Must initialize and start the module.
     * First, there will be a connection to the database initialized. Second, there will be a subscription to all kinds of events.
     * @return <code>True</code> if enable is successful, <code>False</code> if enable is failed or rejected
     */
    virtual bool enable();

public:
    /**
     * Handler for new events which are received via ZMF Bus. Forwards the event to the CassandraClient.
     * @see CassandraClient.insertLogMsg()
     * @param message the received event
     * @param sender the sender of the event
     */
    virtual void handleEvent(const zmf::data::ZmfMessage& message,
                             const zmf::data::ModuleUniqueId& sender);

    /**
     * Called when the module should disable itself. Must stop the module (threads etc.).
     */
    virtual void disable();


private:
    /** Version of the module */
    static const uint16_t MODULE_VERSION = 0;
    /** Module type id */
    static const uint16_t MODULE_TYPE_ID = 0x0010;
    /** Module instance id */
    uint64_t instanceId_;
    /** Instance of cassandra client
     * @see CassandraClient*/
    CassandraClient cassandraClient;


};


#endif // LoggingModule_H
