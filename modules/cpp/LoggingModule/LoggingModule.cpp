#include "LoggingModule.hpp"


LoggingModule::LoggingModule(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId), MODULE_VERSION, "LoggingModule",
                       std::vector<zmf::ModuleDependency>())
         { }

LoggingModule::~LoggingModule() {
}


bool LoggingModule::enable() {

    std::string cassandraURL;

    try{
        bool configCassandraURLRead = getZmf()->getConfigurationProvider()->getAsString(zsdn::ZSDN_LOGGING_MODULE_CASSANDRA_URL, cassandraURL);

        if(!configCassandraURLRead){
            getLogger().error("Could not read config value " + std::string(zsdn::ZSDN_LOGGING_MODULE_CASSANDRA_URL));
            return false;
        }

        cassandraURL = cassandraURL.substr(1, cassandraURL.size()-2);

    } catch (Poco::Exception pe) {
        getLogger().error("failed to load configs: " + pe.message());
        return false;
    }

    //cassandraClient.connect("127.0.0.1");
    cassandraClient.connect(cassandraURL);
    cassandraClient.createSchema();
    // Subscribe to any messages
    zmf::data::MessageType topic;

    getZmf()->subscribe(topic, [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
        handleEvent(msg, sender);
    });

    getLogger().information("startup complete");

    return true;
}

void LoggingModule::handleEvent(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender) {
    // Optional: filter messages, if performance issues occurring
    cassandraClient.insertLogMsg(message, sender);
}


void LoggingModule::disable() {
    cassandraClient.close();
}


void LoggingModule::handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                            zmf::data::ModuleState newState, zmf::data::ModuleState lastState) {
    // Optional: Handle state changes of modules
}

