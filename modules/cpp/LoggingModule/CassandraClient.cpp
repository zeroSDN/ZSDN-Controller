#include <cassandra.h>
#include <zmf/ZmfMessage.hpp>
#include <sstream>
#include <iomanip>
#include "CassandraClient.h"

CassError CassandraClient::connect(const std::string nodes) {
    CassError rc = CASS_OK;
    logger.information("Connecting to " + nodes);

    cluster = cass_cluster_new();
    session = cass_session_new();

    CassFuture* connect_future = NULL;
    // Optional: use multiple nodes for better load balancing. Just enter more IPs here /re
    // cass_cluster_set_contact_points(cluster, "127.0.0.1");
    cass_cluster_set_contact_points(cluster, nodes.c_str());
    connect_future = cass_session_connect(session, cluster);
    cass_future_wait(connect_future);

    rc = cass_future_error_code(connect_future);
    if (rc == CASS_OK) {
        logger.information("Connected!");
    } else {
        return printError(rc);
    }
    cass_future_free(connect_future);
    return rc;
}

void CassandraClient::close() {
    logger.information("Closing down cluster connection");
    cass_session_close(session);
    cass_cluster_free(cluster);
}

inline CassError CassandraClient::executeStatement(std::string cqlStatement) {

    CassError rc = CASS_OK;
    CassFuture* result_future = NULL;

    logger.information("Executing CQL Command");

    CassStatement* statement = cass_statement_new(cqlStatement.c_str(), 0);
    result_future = cass_session_execute(session, statement);
    cass_future_wait(result_future);

    rc = cass_future_error_code(result_future);
    if (rc == CASS_OK) {
        logger.information("Executing CQL Command: Success");
    } else {
        return printError(rc);
    }

    cass_statement_free(statement);
    cass_future_free(result_future);

    return rc;
}

CassError CassandraClient::createSchema() {
    CassError rc = CASS_OK;
    logger.information("Creating Schema");

    // Writing Schema logging
    // rc = executeStatement("SELECT zmfLogging FROM system.schema_keyspaces");
    rc = executeStatement(
            "CREATE KEYSPACE IF NOT EXISTS logging WITH REPLICATION = { 'class' : 'SimpleStrategy', 'replication_factor' : 1 };");
    // Create Table messages
    rc = executeStatement(
            "CREATE TABLE IF NOT EXISTS logging.messages (id uuid PRIMARY KEY, messageType blob, senderType varint, senderId varint, timeStamp timestamp);");
    return rc;
}


CassError CassandraClient::insertLogMsg(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender) {
    CassError rc = CASS_OK;

    std::stringstream stream;
    for (int i = 0; i < message.getType().getMatchLength(); i++) {
        stream
        << std::setfill('0')
        << std::setw(2)
        << std::hex << unsigned(message.getType().getMatchRaw()[i]);
    }
    std::string asString = stream.str();

    logger.information("New Log Message");
    std::string statement = std::string(
            "INSERT INTO logging.messages (id, messageType, senderType, senderId, timestamp) VALUES (uuid(), 0x") +
                            asString + ", " + std::to_string(sender.TypeId) + ", " +
                            std::to_string(sender.InstanceId) + ", dateOf(now()));";


    rc = executeStatement(statement);

    return rc;
}

inline CassError CassandraClient::printError(CassError error) {
    logger.warning(cass_error_desc(error));
    return error;
}