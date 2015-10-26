#ifndef LinkDiscoveryModule_H
#define LinkDiscoveryModule_H

#include <zmf/AbstractModule.hpp>
#include <thread>
#include <ModuleTypeIDs.hpp>
#include <mutex>
#include <atomic>
#include <chrono>
#include <zsdn/proto/CommonTopology.pb.h>
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include <LociExtensions.h>
#include <zsdn/topics/SwitchRegistryModule_topics.hpp>
#include <zsdn/topics/LinkDiscoveryModule_topics.hpp>

/**
 * @details The LinkDiscoveryModule knows about all links between switches in the SDN-network.
 * These links are directed, meaning that if all switches have working bidirectional links between them,
 * there will be two directed links for each connection.
 * @author Matthias Strljic
 */
class LinkDiscoveryModule : public zmf::AbstractModule {

public:
    LinkDiscoveryModule(uint64_t instanceId);

    ~LinkDiscoveryModule();

    /**
     * Handles the incoming request messages from the zmf-network and handle accordingly to the received topic.
     * The replies always represent the actual state of discovered links between switch instances.
     * REQUEST.LINK_DISCOVERY_MODULE.GET_ALL_SWITCH_LINKS:
     *      Replies all links.
     * REQUEST.LINK_DISCOVERY_MODULE.GET_LINKS_FROM_SWITCH:
     *      Replies links where the given switch id acts as source.
     * REQUEST.LINK_DISCOVERY_MODULE.GET_LINKS_TO_SWITCH:
     *      Replies links where the given switch id acts as endpoint.
     * REQUEST.LINK_DISCOVERY_MODULE.GET_LINKS_OF_SWITCH:
     *      Replies all links where the given switch id act as source or endpoint.
     * REQUEST.LINK_DISCOVERY_MODULE.GET_LINKS_BETWEEN_TWO_SWITCHES
     *      Replies all links which contains both id's, one act as source and the other as endpoint.
     *
     * @param message The incoming ZmfMessage which contains information to the correlating Requests
     * @param sender is a tuple to identify the instance inside the zmf network
     * @return True if we want to respond on the request, false otherwise
     */
    virtual zmf::data::ZmfOutReply handleRequest(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender) override;

    // predefinition for the LinkDeviceTimestampTupel
    struct LinkDeviceTimestampTupel;

    // inner data stucture class to represent a endpoint device of a link
    class LinkDevice {
    private:
        uint64_t switchDpid_;
        std::vector<uint32_t> availablePorts_;
        std::map<uint32_t, LinkDeviceTimestampTupel> activeLinks_;
        of_version_t ofVersion_;
        std::mutex dataMutex;
    public:

        // enum to represent the result of a Insert of a link
        enum InsertResult {
            NO_VALID_PORT, SUCCESS_NEW, SUCCESS_MODIFY, SUCCESS_NO_CHANGE
        };

        /**
         * Constructor of a LinkDevice to initialize it. Requires the available network ports an the network id.
         *
         * @param availablePorts a inital list of available port of the represented LinkdDevice
         * @param switchDpid the device id
         */
        LinkDevice(std::vector<uint32_t> availablePorts, uint64_t switchDpid, of_version_t ofVersion) {
            this->switchDpid_ = switchDpid;
            this->availablePorts_ = availablePorts;
            this->ofVersion_ = ofVersion;
        };

        ~LinkDevice() {
            availablePorts_.clear();
            activeLinks_.clear();
        }

        /**
         * Threadsave function to insert a new link to the device. This requires a available port.
         *
         * @param port the port of de device where the link is setup
         * @param linkDevice the endpoint device which is connected to the port
         * @param endpointPort the port at the endpoint device where the link is setup
         * @param updateTime the timestamp of the link discovery.
         * @return InsertResult type to specify the result of the insert action. This is used to represent the different
         *         situations PORT_NOT_AVAILABLE / NEW_INSERT / MODIFY_ENTRY / NO_CHANGES
         */
        InsertResult insertLink(uint32_t port,
                                std::shared_ptr<LinkDevice> linkedDevice,
                                uint32_t endpointPort,
                                uint64_t updateTime);

        /**
         * Threadsave function to erase out dated linkes. The function returns
         * over the out param outDatedLinks the removed links.
         * @param borderData the time border to flush
         * @param outDatedLinks Out param to access the removed links of the flush. The result contains
         *        the erased Link with its endpoint port maped to the flushed device port.
         *
         */
        void flushOutDatedLinks(uint64_t borderDate,
                                std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>& outDatedLinks);

        /**
         * Threadsave function to erase links to a endpoint with the deviceId
         *
         * @param deviceId the id of the link endpoint device which have to be erased.
         * @return the erased links where the endpoint device his port of the link are mapped to the
         *         port of the source link device
         */
        std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>> eraseLinkToDevice(uint64_t deviceId);

        /**
         * Returns a copy of the available ports of the device
         * @param copy of the available ports
         */
        std::vector<uint32_t> getAvailablePorts() {
            return this->availablePorts_;
        }

        /**
         * Get a copy of the activeLinks inside the device.
         * WARNING: the map is a copy but the shared pointers of the LinkDevices points to the original Objects
         *
         * @return copy of the activeLinks, which maps the endpoint device with the endpoint port
         *         (contained inside the Tupel struct) to the port of the source device
         */
        std::map<uint32_t, LinkDeviceTimestampTupel> getActiveLinks() {
            return this->activeLinks_;
        }

        /**
         * Access the Id of the device
         * @return A copy of the device Id
         */
        uint64_t getId() {
            return this->switchDpid_;
        }

        void clearAll() {
            this->activeLinks_.clear();
        }

        of_version_t getVersion() {
            return this->ofVersion_;
        }

    };

    /**
     *
     * @details Data struct to bundle the 3 correlating informatons endpoint of a link,
     *          the endpoint port of the link, and the timestamp of the creation of the link
     *
     * @author Matthias Strljic
     */
    struct LinkDeviceTimestampTupel {
        /// Shared pointer to the endpoint device
        std::shared_ptr<LinkDevice> device;
        /// Port at the endpoint device where the link is established
        uint32_t connectionPort;
        /// Time of the last link keep alive
        uint64_t timestamp;
    };

    /**
     * @details Data struct to represent a single link discover request contains the information:
     *          deviceId: the id of the discover source device
     *          hashCode: A unique hash to identify the message
     *          timestamp: Timespamp of the send time
     *          discoverPort: The discovered port of the source device
     *
     * @author Matthias Strljic
     */
    struct DiscoverDevice {
        /// The instance id of the zmf-network instance as source point
        uint64_t deviceId_;
        /// Hash to identify the discover request (unique)
        uint64_t hashCode_;
        /// Timestamp of the sended request
        uint64_t timestamp_;
        /// Port at which the discovered message was send
        uint32_t discoverPort_;
        /// OpenFlow version of the instance
        of_version_t ofVersion_;

        DiscoverDevice(uint64_t deviceId, uint64_t hashCode, uint64_t timestamp, uint32_t discoverPort,
                       of_version_t ofVersion)
                : deviceId_(deviceId), hashCode_(hashCode), timestamp_(timestamp), discoverPort_(discoverPort),
                  ofVersion_(ofVersion) {

        }

        DiscoverDevice() {

        }
    };

protected:
    /**
     * Enables the module out of a disabled state. This will setup the needed subscriptions
     * (OpenFlow::PacketIn + SwitchRegistry::SwitchStateChanged), the database with a call of all existing switches
     * from the SwitchRegistryModule and starts the background Thread for continouse link discovery
     *
     * @return True if all needed elements could be setup correctly (subscriptions + database + background thread)
     */
    virtual bool enable();


public:
    /// The own specified Ethernet message type
    static const uint16_t LINK_DISCOVERY_MESSAGE_PAYLOAD_TYPE = 0x511F;

    /**
     * Deactivates the loop condition of the background thread
     */
    virtual void disable();


private:
    //static constexpr zmf::ModuleDependency SWITCH_REGISTRY_MODULE_DEP  = {zsdn::MODULE_TYPE_ID_SwitchRegistryModule,0};
    static constexpr const char* LINK_DISCOVERY_CONFIG_UPDATE_CYCLE = "ZSDN_LINK_DISCOVERY_MODULE_UPDATE_CYCLE";
    static const uint16_t SWITCH_REGISTRY_MODULE_DEP_TYPE = zsdn::MODULE_TYPE_ID_SwitchRegistryModule;
    static const uint16_t SWITCH_REGISTRY_MODULE_DEP_VERSION = 0;
    /// Current version of the Module (should be increased by release changes)
    static const uint16_t MODULE_VERSION = 0;
    /// Representation of the module type
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_LinkDiscoveryModule;

    /// Unique instance id fo the module inside the ZMF system.
    uint64_t instanceId_;
    /// Member variable of the background thread
    Poco::Thread backgroundThread_;
    /// True if the background thread is initialzed and is currently running
    bool backgroundThreadInit_ = false;
    /// Contition of the background loop True if he loop should continue
    bool updateBackground_ = false;
    /// The sleep thime of the background thread -> the break between each update cycle
    std::chrono::milliseconds updateCycle;
    /// Counter for the creation of unique hashes (Threadsafe)
    std::atomic<uint64_t> hashCounter_;
    /// Variable to store the timeout border of discover messages (Threadsafe)
    std::atomic<uint64_t> timeOutBorder_;
    /// Map to store all current active LinkDevices
    std::map<uint64_t, std::shared_ptr<LinkDevice>> activeDevices_; // Map<DeviceId, LinkDevice>
    /// List to store the current active request id's ordered by send date.
    std::vector<uint64_t> activeRequestOrder_; // List to save performance for timeouted messages
    /// Map of the current active link discover messages, they are mapped to their hash code.
    std::map<uint64_t, DiscoverDevice> activeRequest_; // Map<HASH, DiscoverDevice>
    /// Mutex to controle the access tho the activeDevices data
    std::mutex activeDevicesMutex_;
    /// Mutex to controle the access of the activeRequestOrder list and the activeRequest map
    std::mutex activeRequestMutex_;
    /// Message topics of the used message types.
    zmf::data::MessageType subscribeSwitchStateChangeTopic_ =
            switchregistrymodule_topics::FROM().switch_registry_module().switch_event().build();
    zmf::data::MessageType sendGetAllSwitchesRequestTopic_ =
            switchregistrymodule_topics::REQUEST().switch_registry_module().get_all_switches().build();
    zmf::data::MessageType publishSwitchLinkChangeAddTopic_ =
            linkdiscoverymodule_topics::FROM().link_discovery_module().switch_link_event().added().build();
    zmf::data::MessageType publishSwitchLinkChangeRemovedTopic_ =
            linkdiscoverymodule_topics::FROM().link_discovery_module().switch_link_event().removed().build();
    zmf::data::MessageType sendGetLinksFromSwitchReply_ =
            linkdiscoverymodule_topics::REPLY().link_discovery_module().get_links_from_switch().build();
    zmf::data::MessageType sendGetLinksToSwitchReply_ =
            linkdiscoverymodule_topics::REPLY().link_discovery_module().get_links_to_switch().build();
    zmf::data::MessageType sendGetLinksOfSwitchReply_ =
            linkdiscoverymodule_topics::REPLY().link_discovery_module().get_all_links_of_switch().build();
    zmf::data::MessageType sendGetLinksBetweenSwitchesReply_ =
            linkdiscoverymodule_topics::REPLY().link_discovery_module().get_links_between_two_switches().build();
    zmf::data::MessageType sendGetAllSwitchLinksReply_ =
            linkdiscoverymodule_topics::REPLY().link_discovery_module().get_all_switch_links().build();

    /**
     * This starts the background thread if it is not currently running.
     */
    void backgroundLinkDiscoveryRun();

    /**
     * Function to generate controled unique hashes
     */
    uint64_t getNewHash();

    /**
     * encapsulate the action of a switch changed event of the SwitchRegistryModule.
     * This will react to SWITCH_ADD / SWITCH_REMOVED / SWITCH_CHANGED events
     * @param changeMsg The published ZMF message
     */
    virtual void handleSwitchStateChanged(const zmf::data::ZmfMessage& changeMsg);

    /**
     * Encapsulate the action of a OpenFlow::PacketIn message. It will filterout the discover message by the
     * own Ethernet type and updates the links.
     *
     * @param message The ZMG message from the subscription (should contain a OpenFlow::PacketIn)
     * @param sender ModuleID of the ZMF module, which send the message.
     */
    virtual void handlePacketIn(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender);

    /**
     * Handles the data of a received link discovery message and will insert the resulting link to the source device
     * if both devices are known.
     * @param deviceId the id of the endpoint device
     * @param hashCode the hash of the link discovery message
     * @param timestamp the timestamp of the link discovery message
     * @param inPort the endpoint link port
     */
    virtual void handleDiscoverPacketIn(uint64_t deviceId, uint64_t hashCode, uint64_t timestamp, uint32_t inPort);

    /**
     * Publishes events for the given removed links
     *
     * @param deviceId Id of the source device of the removed links
     * @param removedLinks Map of the removed links.
     */
    virtual void handleLinkRemoved(uint64_t deviceId,
                                   std::map<uint32_t, std::pair<std::shared_ptr<LinkDevice>, uint32_t>>& removedLinks);

    /**
     * Publishes a  single event for a added link
     * @param deviceId The id of the link source device
     * @param devicePort port of the link source device
     * @param endpointId The id of the endpoint device
     * @param endpointPort Port of the link endpoint device
     */
    virtual void handleLinkAdded(uint64_t deviceId, uint32_t devicePort, uint64_t endpointId, uint32_t endpointPort);

    /**
     * Threadsave method to request a LinkDevice
     * @param deviceId Id of the requested LinkDevice
     * @param resultDevice Out param, the shared_ptr to the requested device it will not be filled if no device was found
     * @result True if a device was found
     */
    virtual bool tryToGetLinkDevice(uint64_t deviceId, std::shared_ptr<LinkDevice>& resultDevice);

    /**
     * Threadsave method to request a DiscoverDevice (which represents a link discover message)
     * @param requestHash Unique hash of the DiscoverDevice
     * @param resultDevice Out param which will contain the result of the request.
     * @result True if a  matchin request was found.
     */
    virtual bool tryToGetActiveRequest(uint64_t requestHash, DiscoverDevice& resultDevice);

    /**
     * Threadsave erase of all outdated discover request. It will use the ordered list of requests to iterate over the hashes to
     * save performance.
     * @param timeBorder The border underneath a request will be erased
     */
    virtual void flushOutdatedRequests(uint64_t timeBorder);

    /**
     * Threadsave insert function for new DiscoverDevice (discover requests). This will store the new request inside
     * the map and at the right position of the ordered list.
     * @param resultDevice new request that should be inserted
     */
    virtual void insertActiveRequest(DiscoverDevice resultDevice);

    /**
     * Function to setup all needed subscriptions (SwitchRegistry::SwitchChanged +  OpenFlow::PacketIn)
     * @return True if there occurred no errors
     */
    virtual bool setupSubscription();

    /**
     * Setup the initial data set. Requests the available switches from the SwitchRegistry.
     * @return True if during the request occurred no errors
     */
    virtual bool setupDatabase();

    /**
     * Threadsave function to insert a new LinkDevice to the activeDevices map.
     * @param availablePorts the available ports of the switch
     * @param switchDpid Id of the LinkDevice
     * @param ofVersion Vesion of the supported OpenFlow of the Switch.
     * @result True if the device is inserted to the activeDevices map
     */
    virtual bool addLinkDevice(std::vector<uint32_t> availablePorts, uint64_t switchDpid, of_version_t ofVersion);

    /**
     * Threadsave function to remove a LinkDevice from the activeDevices map.
     * @param switchDpid Id of the device
     * @resutl True if the device exists and is afterwards removed from the activeDevices map.
     *
     */
    virtual bool removeLinkDevice(uint64_t switchDpid);

    /**
     * Encapsulated function to fill a empty Protp_type SwitchToSwitchLink
     * @param container The empty SwitchToSwitchLink, which should be filled by this function
     * @param sourceId Id of the source device
     * @param sourcePort Link port at the source device
     * @param targetId Endpoint device id
     * @param targetPort Link endpoint port
     */
    virtual void fillSwitchToSwitchLink(common::topology::SwitchToSwitchLink* container,
                                        uint64_t sourceId,
                                        uint32_t sourcePort,
                                        uint64_t targetId,
                                        uint32_t targetPort);


};


#endif // LinkDiscoveryModule_H
