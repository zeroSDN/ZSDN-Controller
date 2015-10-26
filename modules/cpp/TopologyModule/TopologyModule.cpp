#include "TopologyModule.hpp"
#include <zsdn/proto/SwitchRegistryModule.pb.h>
#include <zsdn/proto/LinkDiscoveryModule.pb.h>
#include <zsdn/proto/TopologyModule.pb.h>
#include <RequestUtils.h>

TopologyModule::TopologyModule(uint64_t instanceId) : AbstractModule(
        zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
        MODULE_VERSION,
        "TopologyModule", {{zsdn::MODULE_TYPE_ID_SwitchRegistryModule, switchRegistryModuleDependencyVersion_},
                           {zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,  linkDiscoveryModuleDependencyVersion_}}) { }

TopologyModule::~TopologyModule() {
}

bool TopologyModule::enable() {

    currentStableTopology_ = new common::topology::Topology();

    requestAllSwitchesFromSwitchRegistryModule();
    requestAllSwitchToSwitchLinksFromLinkDiscoveryModule();

    // Subscribe at ZMF to Topics
    getZmf()->subscribe(topicsSwitchEvent_,
                        [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                            handleSwitchEvent(msg, sender);
                        });

    getZmf()->subscribe(topicsSwitchLinkEvent_, [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
        handleSwitchLinkEvent(msg, sender);
    });

    initializeCurrentStableTopology();

    return true;
}


void TopologyModule::disable() {
    switchesCache_.clear();
    switchToSwitchLinkCacheRegistered_.clear();
    switchToSwitchLinkCacheUnregistered_.clear();
    delete currentStableTopology_;
}

zmf::data::ZmfOutReply TopologyModule::handleRequest(const zmf::data::ZmfMessage& message,
                                                     const zmf::data::ModuleUniqueId& sender) {

    TopologyModule_Proto::Request request;

    bool parseSuccess = request.ParseFromArray(message.getDataRaw(), message.getDataLength());
    if (!parseSuccess) {
        getLogger().warning(
                "For Request from " + sender.getString() + " received invalid ProtoBuffer request format.");
        return zmf::data::ZmfOutReply::createNoReply();
    }

    switch (request.RequestMsg_case()) {
        case TopologyModule_Proto::Request::kGetTopologyRequest: {
            TopologyModule_Proto::Reply_GetTopologyReply* getTopologyReply = new TopologyModule_Proto::Reply_GetTopologyReply();

            getLogger().information(
                    "Replying to GetTopologyRequest from " + sender.getString() + "with Topology containing  " +
                    std::to_string(currentStableTopology_->switches_size()) + " Switches and " +
                    std::to_string(currentStableTopology_->switch_to_switch_links_size()) + " SwitchToSwitchLinks.");

            getTopologyReply->set_allocated_topology(new common::topology::Topology(*currentStableTopology_));
            // Serialize and build ZmfMessage
            TopologyModule_Proto::Reply reply;
            reply.set_allocated_get_topology_reply(getTopologyReply);

            return zmf::data::ZmfOutReply::createImmediateReply(
                    zmf::data::ZmfMessage(topicsGetTopologyReply_, reply.SerializeAsString()));
        }
        case TopologyModule_Proto::Request::REQUESTMSG_NOT_SET:
            getLogger().warning("Received Request where RequestMessage was not set.");
            return ZmfOutReply::createNoReply();
        default:
            getLogger().information("Received unknown Request");
            return ZmfOutReply::createNoReply();
    }
}

void TopologyModule::requestAllSwitchesFromSwitchRegistryModule() {
    getLogger().information("Trying to Request GetAllSwitches from a SwitchRegistryModule.");

    SwitchRegistryModule_Proto::Request request;

    SwitchRegistryModule_Proto::Request_GetAllSwitchesRequest* getAllSwitchesRequest = new SwitchRegistryModule_Proto::Request_GetAllSwitchesRequest();
    request.set_allocated_get_all_switches_request(getAllSwitchesRequest);

    SwitchRegistryModule_Proto::Reply reply;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            request,
                                            reply,
                                            topicsGetAllSwitches_,
                                            zsdn::MODULE_TYPE_ID_SwitchRegistryModule,
                                            switchRegistryModuleDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {

            int counter = 0;
            for (int i = 0; i < reply.get_all_switches_reply().switches_size(); i++) {
                addNewSwitch(reply.get_all_switches_reply().switches(i));
                counter++;
            }
            getLogger().information("Added " + std::to_string(counter) + " Switches to the SwitchesCache.");
        }
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning(
                    "Request aborted. Currently no modules with the type MODULE_TYPE_ID_SwitchRegistryModule are available.");
            break;

        case zsdn::RequestUtils::TIMEOUT:
            getLogger().warning("Timeout when waiting for all Switches.");
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            getLogger().warning("Serialization of the Request failed.");
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            getLogger().warning("Parsing of the Response failed.");
            break;
    }
}


void TopologyModule::requestAllSwitchToSwitchLinksFromLinkDiscoveryModule() {
    getLogger().information("Trying to Request GetAllSwitchLinks from a LinkDiscoveryModule.");

    LinkDiscoveryModule_Proto::Request request;

    LinkDiscoveryModule_Proto::Request_GetAllSwitchLinksRequest* getAllSwitchLinksRequest = new LinkDiscoveryModule_Proto::Request_GetAllSwitchLinksRequest();
    request.set_allocated_get_all_switch_links_request(getAllSwitchLinksRequest);

    LinkDiscoveryModule_Proto::Reply reply;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(), request, reply, topicsGetAllSwitchLinks_,
                                            zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                            linkDiscoveryModuleDependencyVersion_);

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {

            int counter = 0;
            for (int i = 0; i < reply.get_all_switch_links_reply().switch_links_size(); i++) {
                addNewSwitchToSwitchLink(reply.get_all_switch_links_reply().switch_links(i));
                counter++;
            }
            getLogger().information("Added " + std::to_string(counter) +
                                    " SwitchToSwitchLinks to the SwitchToSwitchLinksCache.");
        }
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND:
            getLogger().warning(
                    "Request aborted. Currently no modules with the type MODULE_TYPE_ID_LinkDiscoveryModule are available.");
            break;

        case zsdn::RequestUtils::TIMEOUT:
            getLogger().warning("Timeout when waiting for all SwitchToSwitchLinks.");
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            getLogger().warning("Serialization of the Request failed.");
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            getLogger().warning("Parsin of the Response failed.");
            break;
    }
}


void TopologyModule::handleSwitchEvent(const ZmfMessage& message, const ModuleUniqueId& id) {
    // Container of the Zmf message
    SwitchRegistryModule_Proto::From msgContainer;

    bool parseSuccess = msgContainer.ParseFromArray(message.getDataRaw(), message.getDataLength());
    if (!parseSuccess) {
        getLogger().warning(
                "For FROM message from SwitchRegistryModule " + std::to_string(id.TypeId) + ":" +
                std::to_string(id.InstanceId) +
                " received invalid ProtoBuffer request format.");
        return;
    }

    // Get the specific event message out of the message container
    const SwitchRegistryModule_Proto::From_SwitchEvent& switchEventMessage = msgContainer.switch_event();

    switch (switchEventMessage.SwitchEventType_case()) {
        case SwitchRegistryModule_Proto::From_SwitchEvent::kSwitchAdded:
            getLogger().information(
                    "Received SwitchEvent Added from SwitchRegistryModule " + std::to_string(id.TypeId) + ":" +
                    std::to_string(id.InstanceId));

            addNewSwitch(switchEventMessage.switch_added());
            break;

        case SwitchRegistryModule_Proto::From_SwitchEvent::kSwitchChanged:
            getLogger().information(
                    "Received SwitchEvent Changed from SwitchRegistryModule " + std::to_string(id.TypeId) + ":" +
                    std::to_string(id.InstanceId));

            updateSwitch(switchEventMessage.switch_changed().switch_now());
            break;
        case SwitchRegistryModule_Proto::From_SwitchEvent::kSwitchRemoved:
            getLogger().information(
                    "Received SwitchEvent Removed from SwitchRegistryModule " + std::to_string(id.TypeId) + ":" +
                    std::to_string(id.InstanceId));

            removeSwitch(switchEventMessage.switch_removed());
            break;
        default:
            getLogger().warning("Received unkown SwitchEvent from SwitchRegistryModule");
            break;
    }

}

void TopologyModule::handleSwitchLinkEvent(const ZmfMessage& message, const ModuleUniqueId& id) {
    // Container of the Zmf message
    LinkDiscoveryModule_Proto::From msgContainer;

    bool parseSuccess = msgContainer.ParseFromArray(message.getDataRaw(), message.getDataLength());
    if (!parseSuccess) {
        getLogger().warning(
                "For FROM message from LinkDiscoveryModule " + std::to_string(id.TypeId) + ":" +
                std::to_string(id.InstanceId) +
                " received invalid ProtoBuffer request format.");
        return;
    }
    // Get the specific event message out of the message container
    const LinkDiscoveryModule_Proto::From_SwitchLinkEvent& switchLinkEvent = msgContainer.switch_link_event();

    switch (switchLinkEvent.SwitchLinkEventType_case()) {
        case LinkDiscoveryModule_Proto::From_SwitchLinkEvent::kSwitchLinkAdded:
            getLogger().information(
                    "Received SwitchLinkEvent Added from LinkDiscoveryModule " + std::to_string(id.TypeId) + ":" +
                    std::to_string(id.InstanceId));

            addNewSwitchToSwitchLink(switchLinkEvent.switch_link_added());
            break;
        case LinkDiscoveryModule_Proto::From_SwitchLinkEvent::kSwitchLinkRemoved:
            getLogger().information(
                    "Received SwitchLinkEvent Removed from LinkDiscoveryModule " + std::to_string(id.TypeId) + ":" +
                    std::to_string(id.InstanceId));

            removeSwitchToSwitchLink(switchLinkEvent.switch_link_removed());
            break;
        default:
            getLogger().warning("Received unkown SwitchLinkEvent from LinkDiscoveryModule");
            break;
    }
}


void TopologyModule::addNewSwitch(const common::topology::Switch& newSwitch) {
    switchesCache_.emplace(newSwitch.switch_dpid(), newSwitch);
    getLogger().information("Added new Switch with ID: " + std::to_string(newSwitch.switch_dpid()));

    // Check if a SwitchToSwitchLink can be set to registered now
    checkSwitchToSwitchLinkStateChanged();

    currentStableTopology_->add_switches()->CopyFrom(newSwitch);

    publishTopologyChangedEvent();
}

void TopologyModule::updateSwitch(const common::topology::Switch& updatedSwitch) {
    std::map<uint64_t, common::topology::Switch>::iterator iterator = switchesCache_.find(updatedSwitch.switch_dpid());

    if (iterator == switchesCache_.end()) {
        // Switch to update was not found in Cache, therefore just add the Switch without a update.
        getLogger().information("Switch to update wasn't already registered in SwitchCache. Adding as new Switch.");
        // Do not call checkSwitchToSwitchLinkStateChanged() because it gets internally called in addNewSwitch()
        addNewSwitch(updatedSwitch);
    }
    else {
        // Switch was found in SwitchCache. Update this switch.
        iterator->second = updatedSwitch;

        getLogger().information("Updated Switch with ID: " + std::to_string(iterator->second.switch_dpid()));
        checkSwitchToSwitchLinkStateChanged();
        // REMARK: IMPROVEMENT Could use a better way than building the whole topology again for more performance.
        initializeCurrentStableTopology();
    }

}

void TopologyModule::removeSwitch(const common::topology::Switch& removedSwitch) {
    std::map<uint64_t, common::topology::Switch>::iterator iterator = switchesCache_.find(removedSwitch.switch_dpid());

    if (iterator == switchesCache_.end()) {
        // Switch to remove couldn't be found
        getLogger().information("Switch to remove wasn't already registered in SwitchCache. Removing nothing.");
    }
    else {
        // Found Switch to remove
        getLogger().information("Removed Switch with ID: " + std::to_string(iterator->second.switch_dpid()));
        switchesCache_.erase(iterator);
        // REMARK: IMPROVEMENT Could use a better way than building the whole topology again for more performance.
        initializeCurrentStableTopology();
    }
}

void TopologyModule::addNewSwitchToSwitchLink(const common::topology::SwitchToSwitchLink& newLink) {

    if (isSwitchRegistered(newLink.source().switch_dpid(), newLink.source().switch_port()) &&
        isSwitchRegistered(newLink.target().switch_dpid(), newLink.target().switch_port())) {

        switchToSwitchLinkCacheRegistered_.insert(switchToSwitchLinkCacheRegistered_.end(), newLink);

        // REMARK: IMPROVEMENT Could use a better way than building the whole topology again for more performance.
        initializeCurrentStableTopology();

        getLogger().information("Added new SwitchToSwitchLink for REGISTERED Switches from Source-Switch " +
                                std::to_string(newLink.source().switch_dpid()) + " at Port " +
                                std::to_string(newLink.source().switch_port()) + " to Target-Switch " +
                                std::to_string(newLink.target().switch_dpid()) + " at Port " +
                                std::to_string(newLink.target().switch_port()));
    }
    else {
        switchToSwitchLinkCacheUnregistered_.insert(switchToSwitchLinkCacheUnregistered_.end(), newLink);
        getLogger().information("Added new SwitchToSwitchLink for NOT REGISTERED Switches from Source-Switch " +
                                std::to_string(newLink.source().switch_dpid()) + " at Port " +
                                std::to_string(newLink.source().switch_port()) + " to Target-Switch " +
                                std::to_string(newLink.target().switch_dpid()) + " at Port " +
                                std::to_string(newLink.target().switch_port()));
    }

}

void TopologyModule::checkSwitchToSwitchLinkStateChanged() {
    std::vector<common::topology::SwitchToSwitchLink>::iterator iterator = switchToSwitchLinkCacheUnregistered_.begin();
    while (iterator != switchToSwitchLinkCacheUnregistered_.end()) {
        // Holds the indices of the unregistered SwitchToSwitchLinkCache that can be removed afterwards
        if (isSwitchRegistered(iterator->source().switch_dpid(), iterator->source().switch_port()) &&
            isSwitchRegistered(iterator->target().switch_dpid(), iterator->target().switch_port())) {

            // Found a SwitchToSwitchLink that now can be set to registered because the module now knows all necessary Switches
            switchToSwitchLinkCacheRegistered_.insert(switchToSwitchLinkCacheRegistered_.end(), *iterator);

            // Delete this SwitchToSwitchLink from the unregistered Cache and set the iterator to this index
            iterator = switchToSwitchLinkCacheUnregistered_.erase(iterator);

            getLogger().information("Moved SwitchToSwitchLink from Source-Switch " +
                                    std::to_string(switchToSwitchLinkCacheRegistered_[
                                                           switchToSwitchLinkCacheRegistered_.size() -
                                                           1].source().switch_dpid()) + " at Port " +
                                    std::to_string(switchToSwitchLinkCacheRegistered_[
                                                           switchToSwitchLinkCacheRegistered_.size() -
                                                           1].source().switch_port()) + " to Target-Switch " +
                                    std::to_string(switchToSwitchLinkCacheRegistered_[
                                                           switchToSwitchLinkCacheRegistered_.size() -
                                                           1].target().switch_dpid()) + " at Port " +
                                    std::to_string(switchToSwitchLinkCacheRegistered_[
                                                           switchToSwitchLinkCacheRegistered_.size() -
                                                           1].target().switch_port()) +
                                    " from NOT REGISTERED to REGISTERED.");
            // REMARK: IMPROVEMENT Could use a better way than building the whole topology again for more performance.
            initializeCurrentStableTopology();
        }
        else {
            // If not found and therefor nothing removed just increment the iterator.
            ++iterator;
        }
    }
}

void TopologyModule::removeSwitchToSwitchLink(const common::topology::SwitchToSwitchLink& removedLink) {
    std::string serializedRemovedLink = removedLink.SerializeAsString();

    // Boolean indicating whether the STSLink was found in registered. If so, the unregistered shouldn't be iterated.
    bool foundInRegistered = false;
    // For all SwitchToSwitchLink find the one that should be removed by comparing the bytes.
    int indexToRemoveRegistered = -1;
    for (int i = 0; i < switchToSwitchLinkCacheRegistered_.size(); i++) {
        std::string serializedSTSLink = switchToSwitchLinkCacheRegistered_[i].SerializeAsString();
        if (serializedRemovedLink == serializedSTSLink) {
            indexToRemoveRegistered = i;
            break;
        }
    }

    if (indexToRemoveRegistered != -1) {
        // Remove the found Tupel.
        foundInRegistered = true;
        getLogger().information("Removed SwitchToSwitchLink from Source-Switch " +
                                std::to_string(
                                        switchToSwitchLinkCacheRegistered_[indexToRemoveRegistered].source().switch_dpid()) +
                                " at Port " +
                                std::to_string(
                                        switchToSwitchLinkCacheRegistered_[indexToRemoveRegistered].source().switch_port()) +
                                " to Target-Switch " +
                                std::to_string(
                                        switchToSwitchLinkCacheRegistered_[indexToRemoveRegistered].target().switch_dpid()) +
                                " at Port " +
                                std::to_string(
                                        switchToSwitchLinkCacheRegistered_[indexToRemoveRegistered].target().switch_port()));

        switchToSwitchLinkCacheRegistered_.erase(
                switchToSwitchLinkCacheRegistered_.begin() +
                indexToRemoveRegistered);
    }

    int indexToRemoveUnregistered = -1;
    if (!foundInRegistered) {
        for (int i = 0; i < switchToSwitchLinkCacheUnregistered_.size(); i++) {
            std::string serializedSTSLink = switchToSwitchLinkCacheUnregistered_[i].SerializeAsString();
            if (serializedRemovedLink == serializedSTSLink) {
                indexToRemoveUnregistered = i;
                break;
            }
        }
    }

    if (indexToRemoveUnregistered != -1) {
        // Remove the found Tupel.
        getLogger().information("Removed SwitchToSwitchLink from Source-Switch " +
                                std::to_string(
                                        switchToSwitchLinkCacheUnregistered_[indexToRemoveUnregistered].source().switch_dpid()) +
                                " at Port " +
                                std::to_string(
                                        switchToSwitchLinkCacheUnregistered_[indexToRemoveUnregistered].source().switch_port()) +
                                " to Target-Switch " +
                                std::to_string(
                                        switchToSwitchLinkCacheUnregistered_[indexToRemoveUnregistered].target().switch_dpid()) +
                                " at Port " +
                                std::to_string(
                                        switchToSwitchLinkCacheUnregistered_[indexToRemoveUnregistered].target().switch_port()));

        switchToSwitchLinkCacheUnregistered_.erase(
                switchToSwitchLinkCacheUnregistered_.begin() +
                indexToRemoveUnregistered);
    }

    if (indexToRemoveRegistered == -1 && indexToRemoveUnregistered == -1) {
        getLogger().information(
                "SwitchToSwitchLink to remove wasn't already registered in SwitchToSwitchLinkCache. Removing nothing.");
    }
    else {
        initializeCurrentStableTopology();
    }
}

void TopologyModule::initializeCurrentStableTopology() {
    // Remove old data if set
    currentStableTopology_->clear_switches();
    currentStableTopology_->clear_switch_to_switch_links();
    getLogger().information("Initializing current stable Topology.");

    // Add all switches to the topology
    int counterSwitches = 0;
    for (auto& element : switchesCache_) {
        currentStableTopology_->add_switches()->CopyFrom(element.second);
        counterSwitches++;
    }

    getLogger().information("Added " + std::to_string(counterSwitches) + " Switches to the Topology.");

    // Only add SwitchToSwitchLinks if the matching Switches are registered
    int counterLinks = 0;
    for (int i = 0; i < switchToSwitchLinkCacheRegistered_.size(); i++) {

        currentStableTopology_->add_switch_to_switch_links()->CopyFrom(switchToSwitchLinkCacheRegistered_[i]);
        counterLinks++;
    }
    getLogger().information("Added " + std::to_string(counterLinks) + " SwitchToSwitchLinks to the Topology.");

    publishTopologyChangedEvent();
}

bool TopologyModule::isSwitchRegistered(uint64_t switchID, uint32_t switchPort) {
    std::map<uint64_t, common::topology::Switch>::iterator iterator = switchesCache_.find(switchID);

    // Haven't found switch with the given ID
    if (iterator == switchesCache_.end()) return false;

    // ID is correct, now check for the port
    for (int i = 0; i < iterator->second.switch_ports_size(); i++) {
        if (iterator->second.switch_ports(i).attachment_point().switch_dpid() == switchID &&
            iterator->second.switch_ports(i).attachment_point().switch_port() == switchPort)
            return true;
    }

    return false;
}

void TopologyModule::publishTopologyChangedEvent() {
    if (validate(*currentStableTopology_)) {
        TopologyModule_Proto::From_TopologyChangedEvent* msgTopologyChangedEvent = new TopologyModule_Proto::From_TopologyChangedEvent();

        // Need to copy the Topo here because ProtoBuffer wants to take care about deleting itself
        common::topology::Topology* topoCopy = new common::topology::Topology(*currentStableTopology_);
        msgTopologyChangedEvent->set_allocated_topology(topoCopy);

        TopologyModule_Proto::From fromMsg;
        fromMsg.set_allocated_topology_changed_event(msgTopologyChangedEvent);

        // Serialize and publish
        getZmf()->publish(zmf::data::ZmfMessage(topicsTopologyChangedEvent_, fromMsg.SerializeAsString()));

        getLogger().information("Published TopologyChanged Event.");
    }
}

bool TopologyModule::validate(const common::topology::Topology& topology) {
    std::map<zsdn::DPID, std::set<zsdn::Port>> switchesAndPorts;

    for (const common::topology::Switch& sw : topology.switches()) {
        for (const common::topology::SwitchPort& swPort : sw.switch_ports()) {
            switchesAndPorts[sw.switch_dpid()].insert(swPort.attachment_point().switch_port());
        }
    }

    for (const common::topology::SwitchToSwitchLink swToSw : topology.switch_to_switch_links()) {
        bool sourceContained = switchesAndPorts.count(swToSw.source().switch_dpid()) > 0
                               &&
                               switchesAndPorts[swToSw.source().switch_dpid()].count(swToSw.source().switch_port()) > 0;

        bool targetContained = switchesAndPorts.count(swToSw.target().switch_dpid()) > 0
                               &&
                               switchesAndPorts[swToSw.target().switch_dpid()].count(swToSw.target().switch_port()) > 0;

        if (!(sourceContained && targetContained)) {
            getLogger().warning("Validation failed.");
            return false;
        }

        return true;
    }
}