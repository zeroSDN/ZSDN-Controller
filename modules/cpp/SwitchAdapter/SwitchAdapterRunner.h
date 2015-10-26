#ifndef SWITCHADAPTER_SWITCHADAPTERRUNNER_H
#define SWITCHADAPTER_SWITCHADAPTERRUNNER_H

#include <thread>
#include <Poco/Net/ServerSocket.h>
#include "SwitchConnectionUtil.h"
#include "zmf/IZmfInstanceController.hpp"
#include "ZsdnTypes.h"

/**
 * @brief   Class Managing a ServerSocket for incoming switch connections.
 *
 * @details For each incoming connection, an OpenFlow handshake is attempted.
 *          If the handshake is successful, a SwitchAdapter is started for the connection.
 *
 * @author  Andre Kutzleb
 * @author  Alexander Kicherer
 * @author  Matthias Hoppe
 *
 * @date    13.07.2015
 *
 * @remark  Refactored / rewritten by Andre Kutzleb on 18.07.2015
 */
class SwitchAdapterRunner {

public:

    /**
     * Constructs (but does not start) a SwitchAdapterRunner.
     *
     * @param openFlowVersion
     *        the OpenFlow version this runner and the spawned SwitchAdapters will speak with the switches.
     * @param serverPort
     *        the port of the address on which a ServerSocket will be listening for switches.
     * @param configFile (optional)
     *        a path to the config file to use (instead of the default location)
     *
     *
     */
    SwitchAdapterRunner(const of_version_t& openFlowVersion,
                        const uint16_t serverPort,
                        const std::string& configFile);


    /**
     * Destructor. Stops all SwitchAdapters and closes the ServerSocket.
     */
    ~SwitchAdapterRunner();


    /**
     * starts a ServerSocket listening for incoming switchConnections. For each incoming connection, an OpenFlow
     * handshake is attempted. If the handshake is successful, a SwitchAdapter(Module) is started.
     *
     * @return true if started successfully, false else (could not bind ServerSocket)
     */
    bool start();

    /**
     * closes the serverSocket, shuts down all child SwitchAdapter modules.
     */
    void stop();

private:
    /// Hidden empty constructor
    SwitchAdapterRunner() { }

    /// path to a custom config, or empty string if none was provided in the constructor
    std::string configFile_;

    /// Logger of this SwitchAdapterRunner
    Poco::Logger& logger_ = zmf::logging::ZmfLogging::getLogger("SwitchAdapterRunner");

    /// determines if the ServerSocket should keep accepting new connections. if false, shutdown.
    std::atomic<bool> running_ = ATOMIC_VAR_INIT(false);

    /// OpenFlow version the SwitchAdapters will use for communication with the switches
    of_version_t openFlowVersion_;

    /// Address the SwitchAdapterRunner will listen on.
    Poco::Net::SocketAddress serverAddress_;

    /// Thread accepting connections from switches.
    std::shared_ptr<std::thread> serverThread_;


    /**
     * The serverThread will accept new connections in a loop in this function until
     * the SwitchAdapterRunner is stopped.
     *
     * @param serverSocket the socket on which incoming connections will be accepted.
     *
     */
    void acceptConnectionLoop(const std::shared_ptr<Poco::Net::ServerSocket> serverSocket);

    /**
     * After a successful handshake, the serverThread will attempt to start a SwitchAdapter
     * for the switchConnection.
     *
     * @param socket
     *        socket connected to a switch.
     * @param switchConnection
     *        stream connected to the socket to the switch.
     * @param switchId
     *        switchId of the switch.
     * @param ofVersion
     *        OpenFlow version of the switch.
     *
     * @return shared_ptr to the ZmfInstance if the SwitchAdapter start succeeded, else shared_ptr to nullptr.
     */
    std::shared_ptr<zmf::IZmfInstanceController> startSwitchAdapter(
            std::shared_ptr<Poco::Net::StreamSocket> socket,
            std::shared_ptr<Poco::Net::SocketStream> switchConnection,
            const uint64_t switchId,
            const of_version_t ofVersion);

    /**
     * The serverThread locally has a vector of all active SwitchAdapters. after handling an incoming connection,
     * it will also go through this vector and remove all SwitchAdapters which are not running anymore. this
     * avoids the need to have an extra thread just for sanitizing the list of modules.
     *
     * @param switchAdapters
     *        all currently active SwitchAdapters.
     */
    void removeTerminatedSwitchAdapters(
            std::map<zsdn::DPID, std::shared_ptr<zmf::IZmfInstanceController>>& allSwitchAdapters);


    /**
     * Stops the given ZMF instance (a ZMF module)
     * Returns after the ZMF instance is stopped.
     *
     * @param shared_ptr a shared pointer to the ZMF instance (ZMF module) to be stopped
     */
    void stop(std::shared_ptr<zmf::IZmfInstanceController>& shared_ptr);
};


#endif //SWITCHADAPTER_SWITCHADAPTERRUNNER_H

