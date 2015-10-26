#ifndef LOGGINGMODULE_CASSANDRACLIENT_H
#define LOGGINGMODULE_CASSANDRACLIENT_H

#include <cassandra.h>
#include <zmf/AbstractModule.hpp>
#include <string>
#include <Poco/Logger.h>

/**
 * @brief This is the Cassandra Client which is used to create a connection to cassandra db.
 * @details All ZMF Events (only the pub/sub ones) will be stored in the database. We use the Apache Cassandra DB in latest version (2.1.x). To create a connection to cassandra db, you have to install the cassandra db driver on your system!
 * @author Rainer Erban
 */
class CassandraClient {

private:
    /** The cluster of Cassandra db. For testing purposes there is just 1 node. For a better load balancing there should be more than just 1 node in the productive system */
    CassCluster* cluster;
    /** The session is the active session of the connection to cassandra db*/
    CassSession* session;

    /**
     * This <code>private</code> function executes given statements on cassandra db
     * @param cqlStatement - the statement which sould be executeted
     * @param results - <code>optional</code>
     * @return A <code>CassError</code> which should be <code>CASS_OK</code> statement was executed successfully. If not, the <code>CassError</code> helps you to find the problem.
     */
    inline CassError executeStatement(std::string cqlStatement);

    /**
     * A helper method, which logs possible failures. Forwards the <code>CassError</code>
     * @see PocoLogger
     * @return the <code>CassError</code>
     */
    CassError printError(CassError error);

protected:
    /** @return reference to poco logger */
    Poco::Logger& logger = zmf::logging::ZmfLogging::getLogger("Cassandra Client");

public:
    /** @return the current session, necessary for a connection to cassandra db*/
    inline CassSession* getSession() { return session; }

    /**
     * Connects client to cassandra db
     * @param nodes a list of nodes to connect with
     * @return A <code>CassError</code> which should be <code>CASS_OK</code> if the client is connected.
     */
    CassError connect(const std::string nodes);

    /**
     * Creates the Schema on cassandra db, if not exits
     * @return A <code>CassError</code> which should be <code>CASS_OK</code> if schema is created successfully.
     */
    CassError createSchema();

    /**
     * Inserts a @link{ZmfMessage} to the cassandra db. Attention: the payload will be ignored.
     * @param message the @link{ZmfMessage}
     * @param sender the @link{ModuleUniqueId} which is the module type id and module instance id
     */
    CassError insertLogMsg(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender);

    /**
     * Close the connection to cassandra db.
     */
    void close();

    /**
     * The reference to poco logger
     */
    Poco::Logger* loggerRef_;
};


#endif //LOGGINGMODULE_CASSANDRACLIENT_H
