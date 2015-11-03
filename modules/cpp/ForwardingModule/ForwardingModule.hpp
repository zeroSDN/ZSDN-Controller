//
// Created by Andre Kutzleb on 6/28/15.
//

#ifndef ForwardingModule_H
#define ForwardingModule_H

#include <zmf/AbstractModule.hpp>
#include <thread>
#include <ModuleTypeIDs.hpp>
#include <zsdn/topics/SwitchAdapterTopics.hpp>
#include <tins/ethernetII.h>
#include <zsdn/topics/DeviceModuleTopics.hpp>
#include <zsdn/topics/TopologyModuleTopics.hpp>
#include <zsdn/proto/CommonTopology.pb.h>
#include <NetworkGraph.h>
#include <loci/loci_base.h>
#include "Poco/ExpireCache.h"
#include "Poco/ExpirationDecorator.h"
#include "Poco/Void.h"
#include "ZsdnTypes.h"


class ForwardingModule : public zmf::AbstractModule {

    typedef Poco::ExpireCache<zsdn::Device, Poco::Void> ExpiringTargetSet;
public:
    ForwardingModule(uint64_t instanceId);

    ~ForwardingModule();

protected:
    /**
     * Called when the module should enable itself. Must initialize and start the module.
     * @return True if enable successful, False if enable failed or rejected
     */
    virtual bool enable();

public:

/**
     * Called when the module should disable itself. Must stop the module (threads etc.).
     */
    virtual void disable();


private:
    static const uint16_t MODULE_VERSION = 0;
    static const uint16_t topologyModuleDependencyType_ = zsdn::MODULE_TYPE_ID_TopologyModule;

    static const uint16_t topologyModuleDependencyVersion_ = 0;
    static const uint16_t deviceModuleDependencyType_ = zsdn::MODULE_TYPE_ID_DeviceModule;

    static const uint16_t deviceModuleDependencyVersion_ = 0;

    // Builders for topic creation
    zsdn::modules::DeviceModuleTopics<zmf::data::MessageType> deviceModuleTopics_;
    zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType> switchAdapterTopics_;
    zsdn::modules::TopologyModuleTopics<zmf::data::MessageType> topologyModuleTopics_;


    /// Used for subscribing to all packetsIns in order to install routes in the network.
    zmf::data::MessageType deviceStateChangeTopic_ = deviceModuleTopics_.from().device_module().device_event().build();

    zmf::data::MessageType requestDeviceByMacTopic_ = deviceModuleTopics_.request().device_module().get_device_by_mac_address().build();
    zmf::data::MessageType getAllDevicesMsgTopic_ = deviceModuleTopics_.request().device_module().get_all_devices().build();
    zmf::data::MessageType getTopologyTopic_ = topologyModuleTopics_.request().topology_module().get_topology().build();
    zmf::data::MessageType topologyChangedTopic_ = topologyModuleTopics_.from().topology_module().topology_changed_event().build();
    std::shared_ptr<zsdn::NetworkGraph> currentGraph;


    int cacheExpireMillis_;

    /// All the ethertypes in this list will be excluded from forwarding
    std::vector<uint16_t> ignoreEthertypes_;

    /// Map to store all known devices
    std::map<zsdn::MAC, zsdn::Device> devices_;

    std::map<zsdn::Device, std::shared_ptr<ExpiringTargetSet>> routeCache;


    void handlePacketIn(const zmf::data::ZmfMessage& packetInMsg);

    zsdn::Device* requestDevice(zsdn::MAC mac);

    zsdn::Device* insertNewDevice(zsdn::MAC mac, zsdn::DPID switch_dpid, zsdn::Port switch_port);


    void handleDeviceStateChanged(const zmf::data::ZmfMessage& changeMsg);

    void deleteDevice(zsdn::MAC mac);

    bool initiallyRequestAllDevices();

    void handleTopologyChanged(const zmf::data::ZmfMessage& message);

    void applyTopology(const common::topology::Topology& topology);

    bool initiallyRequestTopology();

    void handleMessageForKnownTarget(const of_version_t ofVersion,
                                     const Tins::EthernetII& eth,
                                     const zsdn::Device& sourceDevice,
                                     const zsdn::Device& destinationDevice);

    zsdn::Device* lookupDevice(zsdn::MAC mac);


    void installFlowRulesAlongPath(const of_version_t ofVersion,
                                   const std::vector<zsdn::AttachmentPoint>& vector,
                                   const zsdn::Device& destinationDevice);

    std::string printReachabilityCheck(const common::topology::Topology& topology);
};


#endif // ForwardingModule_H
