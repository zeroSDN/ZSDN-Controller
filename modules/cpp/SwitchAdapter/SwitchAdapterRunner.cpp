#include <zmf/AbstractModule.hpp>
#include <zmf/ZmfInstance.hpp>
#include <ZsdnTypes.h>
#include "SwitchAdapterRunner.h"
#include "SwitchAdapter.hpp"
#include "Poco/Net/NetException.h"


SwitchAdapterRunner::SwitchAdapterRunner(
        const of_version_t& openFlowVersion,
        const uint16_t serverPort,
        const std::string& configFile)
        :
        openFlowVersion_(openFlowVersion),
        serverAddress_(Poco::Net::SocketAddress(serverPort)),
        configFile_(configFile) { }


SwitchAdapterRunner::~SwitchAdapterRunner() {
    stop();
}

bool SwitchAdapterRunner::start() {
    if (running_) {
        return false;
    }
    else {
        try {
            // attempt to bind the socket
            std::shared_ptr<Poco::Net::ServerSocket> serverSocket(new Poco::Net::ServerSocket(serverAddress_));
            // bind was successful. start serverThread. return true.
            running_ = true;
            serverThread_ = std::shared_ptr<std::thread>(
                    new std::thread(&SwitchAdapterRunner::acceptConnectionLoop, this, serverSocket));
            logger_.information("SwitchAdapterRunner started.");
            return true;
        }
        catch (Poco::Net::NetException netException) {
            // bind was unsuccessful. return false.
            logger_.fatal("Could not start SwitchAdapterRunner: " + netException.displayText());
            return false;
        }
    }
}


void SwitchAdapterRunner::stop() {
    if (running_) {
        running_ = false;
        // Stop ServerSocket. The serverThread may be currently blocking on the ServerSocket.
        // Connect to it so it, which will wake it up. Then it has a chance to react to the
        // running flag and can shutdown gracefully.
        try {
            // Opens and immediately close a connection to the ServerSocket to wake the serverThread up.
            Poco::Net::StreamSocket killsocket(serverAddress_);
            killsocket.shutdown();
        }
        catch (Poco::Net::NetException exception) {
            logger_.warning("Exception when stopping server: " + exception.displayText());
        }
        logger_.information("Attempting to join serverThread");
        // wait for the serverThread to shut down before we return from this function
        // to make sure everything shuts down correctly.
        if (serverThread_.get() != nullptr && serverThread_.get()->joinable()) {
            logger_.information("joining serverThread_ now");
            try {
                serverThread_.get()->join();
                serverThread_.reset();
                logger_.information("joined serverThread_");
            } catch (...) {  // Catch all
                logger_.warning("Failed to join serverThread_");
            }
        }
        logger_.information("SwitchAdapterRunner stopped.");
    }
}


void SwitchAdapterRunner::acceptConnectionLoop(const std::shared_ptr<Poco::Net::ServerSocket> serverSocket) {

    // reference to all currently running SwitchAdapters
    std::map<zsdn::DPID, std::shared_ptr<zmf::IZmfInstanceController>> activeSwitchAdapters;

    while (running_) {
        logger_.information("Waiting for connection of a Switch on socket " + serverAddress_.toString());
        std::shared_ptr<Poco::Net::StreamSocket> socket(new Poco::Net::StreamSocket(serverSocket->acceptConnection()));

        std::shared_ptr<Poco::Net::SocketStream> switchConnection(new Poco::Net::SocketStream(*socket));

        logger_.information("Woken up from blocking acceptConnection on ServerSocket.");

        // In case the SwitchAdapterRunner is supposed to shut down, and the serverThread was just woken up by a
        // connection (e.g. the killConnection in the stop function),
        // we do not actually handle the incoming connection, but instead will escape the while loop when running_ is
        // evaluated during the next loop iteration.
        if (running_) {

            logger_.information("Incoming connection from " + socket->peerAddress().toString() +
                                ", trying to remove any existing but stopped adapters first.");
            // the serverThread will also clean up SwitchAdapters so we don't keep piling them up in the vector forever.
            removeTerminatedSwitchAdapters(activeSwitchAdapters);

            try {
                logger_.information("Conduction handshake with " + socket->peerAddress().toString());
                SwitchConnectionUtil::HandshakeResult result = SwitchConnectionUtil::attemptHandshake(switchConnection,
                                                                                                      openFlowVersion_);

                // Only handle the connection if the handshake was successful.
                if (result.status == SwitchConnectionUtil::HandshakeResult::SUCCESS) {
                    logger_.information(
                            "Handshake with " + socket->peerAddress().toString() + " successful.");

                    if (activeSwitchAdapters.count(result.switchId) != 0) {
                        logger_.information("SwitchAdapter with id " + std::to_string(result.switchId) +
                                            " still running. stopping it.");
                        stop(activeSwitchAdapters[result.switchId]);
                        logger_.information("SwitchAdapter with id " + std::to_string(result.switchId) + " stopped.");
                    }

                    // attempt to start the switchAdapter
                    std::shared_ptr<zmf::IZmfInstanceController> switchAdapter =
                            startSwitchAdapter(socket,
                                               switchConnection,
                                               result.switchId,
                                               result.switchOpenFlowVersion);

                    // starting the SwitchAdapter may have failed, in this case we ignore the connection, it will time out
                    if (switchAdapter.get() != nullptr) {
                        // put the new SwitchAdapter to the list of running instances
                        activeSwitchAdapters[result.switchId] = switchAdapter;
                    } else {
                        logger_.warning("Failed to start SwitchAdapter, ignoring connection.");
                    }
                }
                else {
                    // log the failed connection attempt.
                    logger_.warning("Handshake with "
                                    + socket->peerAddress().toString()
                                    + "  was not successful: "
                                    + result.toString());
                }
            }
            catch (Poco::Net::NetException netException) {
                // bind was unsuccessful. return false.
                logger_.warning("Exception when trying to handshake with switch: " + netException.displayText());
            }

            logger_.information(
                    "Current count of switchAdapter instances:" + std::to_string(activeSwitchAdapters.size()));

        } else {
            logger_.information("Woken up for shutdown (running set to false).");
        }
    }

    // finally stop all SwitchAdapters (SwitchAdapterRunner is shutting down).
    for (std::map<zsdn::DPID, ::std::shared_ptr<::zmf::IZmfInstanceController>>::iterator it = activeSwitchAdapters.begin();
         it != activeSwitchAdapters.end();) {
        it = activeSwitchAdapters.erase(it);
    }
}

std::shared_ptr<zmf::IZmfInstanceController> SwitchAdapterRunner::startSwitchAdapter(
        std::shared_ptr<Poco::Net::StreamSocket> socket,
        std::shared_ptr<Poco::Net::SocketStream> switchConnection,
        const uint64_t switchId,
        const of_version_t ofVersion) {

    try {
        // try to create the SwitchAdapter
        std::shared_ptr<zmf::AbstractModule> module = std::shared_ptr<zmf::AbstractModule>(
                new SwitchAdapter(switchId, switchConnection, socket, ofVersion));


        // return the successfully started instance.
        return configFile_.empty() ?
               zmf::instance::ZmfInstance::startInstance(module, {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT})
                                   :
               zmf::instance::ZmfInstance::startInstance(module, {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                         configFile_);


    }
    catch (Poco::Exception exc) {
        logger_.critical("Failed to run SwitchAdapter: " + exc.message());
    }
    catch (...) {
        logger_.critical("Failed to run SwitchAdapter. Unknown reason (catchall)");
    }

    // return failure.
    return std::shared_ptr<zmf::IZmfInstanceController>(nullptr);

}

void SwitchAdapterRunner::removeTerminatedSwitchAdapters(
        std::map<zsdn::DPID, std::shared_ptr<zmf::IZmfInstanceController>>& allSwitchAdapters) {

    // iterate through all known SwitchAdapter instances and erase them in case they are stopped
    for (std::map<zsdn::DPID, ::std::shared_ptr<::zmf::IZmfInstanceController>>::iterator it = allSwitchAdapters.begin();
         it != allSwitchAdapters.end();) {
        if (it->second->isStopped()) {
            it = allSwitchAdapters.erase(it);
        } else {
            ++it;
        }
    }
}

void SwitchAdapterRunner::stop(std::shared_ptr<zmf::IZmfInstanceController>& shared_ptr) {
    logger_.information("Requesting to stop SwitchAdapter and joining execution");
    // initiate stopping of the given ZMF instance (ZMF module)
    shared_ptr->requestStopInstance();
    // wait until the instance is terminated
    shared_ptr->joinExecution();
    logger_.information("SwitchAdapter stopped");

}


