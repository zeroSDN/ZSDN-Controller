//
// Created by Andre Kutzleb on 6/28/15.
//

#ifndef SwitchRegistryModule_H
#define SwitchRegistryModule_H

#include <zmf/AbstractModule.hpp>
#include <thread>
#include <zsdn/topics/SwitchRegistryModule_topics.hpp>
#include <zsdn/proto/CommonTopology.pb.h>
#include <ModuleTypeIDs.hpp>
#include "Switch.h"
#include "zsdn/topics/SwitchAdapter_topics.hpp"

/**
 * @brief This class contains information about available Switches.
 *
 * @details This class gathers Information about Switches in the ZMQ-network.
 * If there is a new Switch(SwitchAdapter), requests will be sent to that Switch to gather the necassary Information.
 * If all Information is gathered the Switch will be set to active and an ADDED-Message, a message to other modules will be sent, so they can react to it.
 * If the Information of an active Switch changes or the Switch is deleted, CHANGED- or DELETED-Messages will be sent.
 * Ther is an option for other modules to request the information one specific or all available Switches per Request-Reply.
 *
 * @author Sebastian Vogel
 *
 */
class SwitchRegistryModule : public zmf::AbstractModule {

    //define map for quick access
    typedef std::map<uint64_t, Switch> SwitchMap;
    typedef std::pair<uint64_t, Switch> SwitchMapPair;
    typedef std::map<uint64_t, Switch>::iterator SwitchMapIter;

public:
    //Constructors
    SwitchRegistryModule(uint64_t instanceId);

    ~SwitchRegistryModule();

    /**
     * Looks for Statechange of SwitchAdapterInstances.
     * If one becomes "alive" it will add it to the map and send OF-requests.
     * If one becomes "dead" it will delete it from the map and send a "Switch died"-message in the ZMF.
     */
    virtual void handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                         zmf::data::ModuleState newState,
                                         zmf::data::ModuleState lastState) override;

    /**
     * Handle incoming ZMQ requests from other modules
     * 1.) AllSwitches-Request
     * This will answer with a List of all existing Switches, whose Information is completly known
     * 2.) SwitchByID-Request
     * This will answer with the Switch, that has the same ID as the one in the Request, if it is available.
     * @return True if we want to respond on the request, false otherwise
     */
    virtual zmf::data::ZmfOutReply handleRequest(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender) override;


protected:
    /**
     * Called when the module should enable itself. Must initialize and start the module.
     * @return True if enable successful, False if enable failed or rejected
     */
    virtual bool enable();

public:
    /**
     * converts a Switch from the map into a common::topology::Switch.
     * This method assumes the Switch with switchID is actually in the map.
     * @param the ID of the Switch, which is to converted into the ZMF-Format
     */
    common::topology::Switch* convertToProto(uint64_t switchID);

/**
     * Called when the module should disable itself. Must stop the module (threads etc.).
     */
    virtual void disable();

    /* Begin: section for UnitTest accessor methods for testing internal behaviour */

    SwitchMap& UTAccessor_getAllSwitchesMap() { return switches; };

    void UTAccessor_addSwitch(Switch& aSwitch) { switches.insert(SwitchMapPair(aSwitch.getSwitchID(), aSwitch)); };

    void UTAccessor_processMultipartReply(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& id) {
        processMultipartReply(message, id);
    };

    void UTAccessor_processFeatureReply(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& id) {
        processFeatureReply(message, id);
    };

    void UTAccessor_processEcho(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& id) {
        processEcho(message, id);
    };

    void UTAccessor_processPortStatus(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& id) {
        processPortStatus(message, id);
    };


    zmf::data::ZmfOutReply UTAccessor_handleRequest(const zmf::data::ZmfMessage& message,
                                                    const zmf::data::ModuleUniqueId& sender) {
        return handleRequest(message, sender);
    };


    /* End: section */


private:
    static const uint16_t MODULE_VERSION = 0;
    static const uint16_t MODULE_TYPE_ID = zsdn::MODULE_TYPE_ID_SwitchRegistryModule;
    uint64_t instanceId_;

    //Map holding all the Switches
    SwitchMap switches;

    //Topic for Switch-Removed
    zmf::data::MessageType topicsSwitchRemoved_ = switchregistrymodule_topics::FROM().switch_registry_module().switch_event().removed().build();
    //Topic for AllSwitches-Reply
    zmf::data::MessageType topicsGetAllSwitchesReply_ = switchregistrymodule_topics::REPLY().switch_registry_module().get_all_switches().build();
    //Topic for SwitchById-Reply
    zmf::data::MessageType topicsGetSwitchByIdReply_ = switchregistrymodule_topics::REPLY().switch_registry_module().get_switch_by_id().build();
    //Topic for Switch-Added
    zmf::data::MessageType topicsSwitchAdded_ = switchregistrymodule_topics::FROM().switch_registry_module().switch_event().added().build();
    //Topic for Switch-Changed
    zmf::data::MessageType topicsSwitchChanged_ = switchregistrymodule_topics::FROM().switch_registry_module().switch_event().changed().build();
    //Topic for Multipart-Reply
    zmf::data::MessageType topicsMultipartReply_ = switchadapter_topics::FROM().switch_adapter().openflow().of_1_0_barrier_reply_of_1_3_multipart_reply().build();
    //Topic for Feature-Reply
    zmf::data::MessageType topicsFeatureReply_ = switchadapter_topics::FROM().switch_adapter().openflow().features_reply().build();
    //Topic for Echo-Request from SwitchAdapter
    zmf::data::MessageType topicsEchoRequest_ = switchadapter_topics::FROM().switch_adapter().openflow().echo_request().build();
    //Topic for Port-Status from SwitchAdapter
    zmf::data::MessageType topicsPortStatus_ = switchadapter_topics::FROM().switch_adapter().openflow().port_status().build();

    /**
     * Handles Incoming Multipart-Replys from the SwitchAdapter.
     * This method will extract the Information from the message and add it to a Switch.
     * @param message The ZmfMessage that was submitted.
     * @param id The id of the Module that has submitted this ZmfMessage.
     */
    void processMultipartReply(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender);

    /**
     * Handles Incoming Feature-Replys from the SwitchAdapter.
     * This method will extract the Information from the message and add it to a Switch.
     * @param message The ZmfMessage that was submitted.
     * @param id The id of the Module that has submitted this ZmfMessage.
     */
    void processFeatureReply(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender);

    /**
     * Handles Incoming Echo-Messages from the SwitchAdapter.
     * This method will check wether Switch is known/completely known and ask for the missing information if necassary.
     * @param message The ZmfMessage that was submitted.
     * @param id The id of the Module that has submitted this ZmfMessage.
     */
    void processEcho(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender);

    /**
     * Handles Incoming Port-Status-Messages from the SwitchAdapter.
     * This method will update the Ports of a Switch according to the OF-message
     * @param message The ZmfMessage that was submitted.
     * @param id The id of the Module that has submitted this ZmfMessage.
     */
    void processPortStatus(const zmf::data::ZmfMessage& message, const zmf::data::ModuleUniqueId& sender);

    /**
     * This method sends a "Switch-Active"-message into the ZMF.
     * To be used after all Information is available for a Switch.
     * @param The Switch that has all Information now
     */
    void switchBecameActive(Switch aSwitch);

    /**
     * This method sends a "Switch-Changed"-message into the ZMF.
     * To be used after active Switchs data changed.
     * @param The Switch that has different data now
     */
    void sendSwitchChanged(Switch aSwitch, Switch oldSwitch);


    /**
     * First adds a new Switch to the map.
     * Then sends the Feature/Multipart-Request messages to the Switch to get the neccassary information.
     *
     * @param switchID: the ID of the Switch u want to add
     * @param ofVersion: the OF-Version of the Switch u want to add
     */
    void addNewSwitch(uint64_t switchID, of_version_t ofVersion);

    /**
     * Sends a Multipart-Request of type Port-Descripition to a Switch
     *
     * @param switchID: the ID of the Switch u want to send to
     * @param ofVersion: the OF-Version of the Switch u want to send to
     */
    void sendMultPortDescRequest(uint64_t switchID, of_version_t ofVersion);

    /**
     * Sends a Feature-Request to a Switch
     *
     * @param switchID: the ID of the Switch u want to send to
     * @param ofVersion: the OF-Version of the Switch u want to send to
     */
    void sendFeatureRequest(uint64_t switchID, of_version_t ofVersion);

};


#endif // SwitchRegistryModule_H
