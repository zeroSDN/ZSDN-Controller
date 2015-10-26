//
// Created by Andre Kutzleb on 6/28/15.
//

#include "SwitchRegistryModule.hpp"
#include <NetUtils.h>
#include "zsdn/proto/SwitchRegistryModule.pb.h"
#include "LociExtensions.h"

SwitchRegistryModule::SwitchRegistryModule(uint64_t instanceId) : AbstractModule(zmf::data::ModuleUniqueId
                                                                                         (MODULE_TYPE_ID, instanceId),
                                                                                 MODULE_VERSION, "SwitchRegistryModule",
                                                                                 std::vector<zmf::ModuleDependency>()) { }

SwitchRegistryModule::~SwitchRegistryModule() {
}


bool SwitchRegistryModule::enable() {
    //subscribe to Multipart-Reply and Feature-Reply OpenFlow messages
    getZmf()->subscribe(topicsFeatureReply_,
                        [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                            processFeatureReply(msg, sender);
                        });
    getZmf()->subscribe(topicsMultipartReply_,
                        [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                            processMultipartReply(msg, sender);
                        });

    getZmf()->subscribe(topicsEchoRequest_,
                        [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                            processEcho(msg, sender);
                        });

    getZmf()->subscribe(topicsPortStatus_,
                        [this](const ZmfMessage& msg, const ModuleUniqueId& sender) {
                            processPortStatus(msg, sender);
                        });

    //get a list with existing SwitchAdapterModules from ZMF
    std::list<std::shared_ptr<zmf::data::ModuleHandle>> moduleList = getZmf()->getPeerRegistry()->getPeersWithType(
            zsdn::MODULE_TYPE_ID_SwitchAdapter, true);
    getLogger().trace("requested existing SwitchAdapterModules from ZMF");
    //iterate through the existing SwitchAdapter and add them to our data
    for (std::shared_ptr<zmf::data::ModuleHandle> module : moduleList) {
        std::vector<uint8_t> addStateInfo = getZmf()->getPeerRegistry()->getPeerAdditionalState(module);
        if (addStateInfo.size() > 0) {
            addNewSwitch(module->UniqueId.InstanceId, static_cast<of_version_t>(addStateInfo[0]));
        } else {
            getLogger().error(
                    "OpenFlow-Version for Switch " + std::to_string(module->UniqueId.InstanceId) + " not available");
        }
    }
    return true;
}


void SwitchRegistryModule::disable() {
    switches.clear();
}


void SwitchRegistryModule::handleModuleStateChange(std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                                   zmf::data::ModuleState newState,
                                                   zmf::data::ModuleState lastState) {
    //only interested in SwitcheAdapterModules
    if (changedModule.get()->UniqueId.TypeId == zsdn::MODULE_TYPE_ID_SwitchAdapter) {
        getLogger().trace("received ModuleStateChange for a SwitchAdapter");
        switch (newState) {
            case zmf::data::ModuleState::Active : // when a SwitchAdapter becomes Active, it means he was added
            {
                //look wether the SwitchAdapter exists already or not
                uint64_t switchID = changedModule->UniqueId.InstanceId;
                SwitchMapIter iter = switches.find(switchID);
                if (iter == switches.end()) {
                    //SwitchAdapter is unknown-->add it to our list
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    std::vector<uint8_t> addStateInfo = getZmf()->getPeerRegistry()->getPeerAdditionalState(
                            changedModule);
                    if (addStateInfo.size() > 0) {
                        addNewSwitch(changedModule->UniqueId.InstanceId, static_cast<of_version_t>(addStateInfo[0]));
                    } else {
                        getLogger().error(
                                "OpenFlow-Version for Switch " + std::to_string(changedModule->UniqueId.InstanceId) +
                                " not available");
                    }

                } else {
                    //SwitchAdapter is kown-->should not happen, log it
                    getLogger().warning("Known Switch became ModulState::Active: " + std::to_string(switchID));

                }
                break;
            }
            case zmf::data::ModuleState::Dead : //when a SwitchAdapter becomes Dead, it means he disappeared
            {
                //look wether the SwitchAdapter exists already or not
                uint64_t switchID = changedModule->UniqueId.InstanceId;
                SwitchMapIter iter = switches.find(switchID);
                if (iter == switches.end()) {
                    //SwitchAdapter is unknown-->should not happen, log it
                    getLogger().warning("Unknown Switch became dead: " + std::to_string(switchID));
                } else {
                    //SwitchAdapter is known-->remove it from data
                    if (switches[switchID].active) { //if SwitchAdapter is active, we gotta send a Removed-Event for the Switch

                        // convert Switch
                        common::topology::Switch* pSwitch = convertToProto(switchID);
                        //create the Removed message
                        SwitchRegistryModule_Proto::From messageContainer;
                        SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();
                        switchEvent->set_allocated_switch_removed(pSwitch);
                        messageContainer.set_allocated_switch_event(switchEvent);
                        //send Removed-message
                        getZmf()->publish(
                                zmf::data::ZmfMessage(topicsSwitchRemoved_, messageContainer.SerializeAsString()));
                        getLogger().information("Switch deleted: " + std::to_string(switchID));
                    } else { //if SwitchAdapter is not active just remove it from local data
                        getLogger().information(
                                "Switch deleted: " + std::to_string(switchID) + ", which was not public yet");
                    }
                    //remove the Switch
                    switches.erase(switchID);
                }
                break;
            }
        }
    }
}


zmf::data::ZmfOutReply SwitchRegistryModule::handleRequest(const zmf::data::ZmfMessage& message,
                                                           const zmf::data::ModuleUniqueId& sender) {
    SwitchRegistryModule_Proto::Request request;
    bool parseSuccess = request.ParseFromArray(message.getDataRaw(), message.getDataLength());
    if (!parseSuccess) {
        getLogger().warning("Received invalid ProtoBuff request format.");
        return zmf::data::ZmfOutReply::createNoReply();
    }

    getLogger().information("Replying at " + getNameInstanceString() + " to request from " + sender.getString());

    switch (request.RequestMsg_case()) {
        case SwitchRegistryModule_Proto::Request::kGetAllSwitchesRequest: {
            // TODO: Evaluate


            // Build protobuf reply
            SwitchRegistryModule_Proto::Reply_GetAllSwitchesReply* getAllSwitchesReply = new SwitchRegistryModule_Proto::Reply_GetAllSwitchesReply();

            for (auto& element : switches) {
                if (element.second.active) {
                    common::topology::Switch* switchElement = convertToProto(element.first);
                    getAllSwitchesReply->add_switches()->CopyFrom(*switchElement);
                    delete switchElement;
                }
            }

            // Serialize and build ZmfMessage
            SwitchRegistryModule_Proto::Reply reply;
            reply.set_allocated_get_all_switches_reply(getAllSwitchesReply);
            getLogger().information(
                    "Request for get_all_switches_request from " + sender.getString() + " will answer with " +
                    std::to_string(getAllSwitchesReply->switches_size()) + " Switches");
            return zmf::data::ZmfOutReply::createImmediateReply(
                    zmf::data::ZmfMessage(topicsGetAllSwitchesReply_, reply.SerializeAsString()));
        }

        case SwitchRegistryModule_Proto::Request::kGetSwitchByIdRequest: {
            // TODO: Evaluate
            getLogger().information("Request for get_switch_by_id_request from " + sender.getString());

            // Build protobuf reply
            SwitchRegistryModule_Proto::Reply_GetSwitchByIdReply* getSwitchByIdReply = new SwitchRegistryModule_Proto::Reply_GetSwitchByIdReply();

            // Add switch if exists in map
            SwitchMapIter switchIterator = switches.find(request.get_switch_by_id_request().switch_dpid());
            if (switchIterator != switches.end()) {
                if (switchIterator->second.active) {
                    getSwitchByIdReply->set_allocated_switch_(convertToProto(switchIterator->first));
                }
            }

            // Serialize and build ZmfMessage
            SwitchRegistryModule_Proto::Reply reply;
            reply.set_allocated_get_switch_by_id_reply(getSwitchByIdReply);

            return zmf::data::ZmfOutReply::createImmediateReply(
                    zmf::data::ZmfMessage(topicsGetSwitchByIdReply_, reply.SerializeAsString()));
        }
        default:
            getLogger().information("Received unknown Request");
            break;
    }

    return zmf::data::ZmfOutReply::createNoReply();
}

void SwitchRegistryModule::processFeatureReply(const zmf::data::ZmfMessage& message,
                                               const zmf::data::ModuleUniqueId& sender) {
    //check if Switch available
    uint64_t switchID = sender.InstanceId;
    SwitchMapIter iter = switches.find(switchID);
    if (iter == switches.end()) {
        getLogger().warning("received Feature-reply from unknown Switch. (probably dead before reply reached)");
        return;
    }
    //parse message into a feature-reply
    of_features_reply_t* featRep = zsdn::of_object_new_from_data_string_copy(message.getData());
    //get SwitchAdapter from data, which sent the message
    Switch& aSwitch = switches[switchID];

    //set Switch information in data
    if(featRep->version>=OF_VERSION_1_3) {
        of_features_reply_auxiliary_id_get(featRep, &aSwitch.auxiliary_id);
    }
    of_features_reply_n_buffers_get(featRep, &aSwitch.n_buffers);
    of_features_reply_n_tables_get(featRep, &aSwitch.n_tables);
    of_features_reply_capabilities_get(featRep, &aSwitch.capabilities);
    if(featRep->version>= OF_VERSION_1_1){
        of_features_reply_reserved_get(featRep, &aSwitch.reserved);
    } else {//in Version 1.0 there is a active field instead of a reserved field. will be stored there(same size)
        of_features_reply_actions_get(featRep, &aSwitch.reserved);
    }


    if(featRep->version< OF_VERSION_1_3){
        //delete portlist(so it is updated from the message
        aSwitch.ports.clear();

        of_list_port_desc_t* listPortDescT = of_features_reply_ports_get(featRep);

        of_port_desc_t* portDescT = (of_port_desc_t*) malloc(64);
        //go to the first spot in the list
        int error = of_list_port_desc_first(listPortDescT, portDescT);
        if (error != OF_ERROR_RANGE) {//if its no out-of-range-error continue
            do {
                //Portinfo auslesen
                uint32_t portnr;
                of_port_desc_port_no_get(portDescT, &portnr);
                Port port = Port(portnr);
                of_mac_addr_t macAddrT;
                of_port_desc_hw_addr_get(portDescT, &macAddrT);
                port.mac_address = zsdn::NetUtils::mac_address_array_to_uint64(macAddrT.addr);
                of_port_name_t portName;
                of_port_desc_name_get(portDescT, &portName);
                port.port_name = std::string(portName);
                of_port_desc_config_get(portDescT, &port.config);
                of_port_desc_state_get(portDescT, &port.state);
                of_port_desc_curr_get(portDescT, &port.curr);
                of_port_desc_advertised_get(portDescT, &port.advertised);
                of_port_desc_supported_get(portDescT, &port.supported);
                of_port_desc_peer_get(portDescT, &port.peer);
                if(featRep->version>=OF_VERSION_1_1) {
                    of_port_desc_curr_speed_get(portDescT, &port.curr_speed);
                    of_port_desc_max_speed_get(portDescT, &port.max_speed);
                }

                aSwitch.ports.insert(aSwitch.ports.end(), port);

                //go to the next Item in the list
                error = of_list_port_desc_next(listPortDescT, portDescT);
            } while (error != OF_ERROR_RANGE);//repeat till error is an out-of-range-error
        }
        of_port_desc_delete(portDescT);
        of_list_port_desc_delete(listPortDescT);

        aSwitch.got_ports = true;
        getLogger().trace("received Feature-Reply for Switch: " + std::to_string(switchID) + " with Version below 1.3, so Ports were added too");
    }


    //free the memory
    of_object_delete(featRep);

    //set SwitchInfoAvailable true
    aSwitch.switch_info_available = true;
    getLogger().trace("received Feature-Reply for Switch: " + std::to_string(switchID));
    //if Switch is active it changed
    if(aSwitch.active){
        sendSwitchChanged(aSwitch);
    }else {
        //if Ports are available already, Switch becomes active
        if (aSwitch.got_ports) {
            aSwitch.active = true;
            switchBecameActive(aSwitch);
        }
    }
}

void SwitchRegistryModule::processMultipartReply(const zmf::data::ZmfMessage& message,
                                                 const zmf::data::ModuleUniqueId& sender) {
    //check if Switch available
    uint64_t switchID = sender.InstanceId;
    SwitchMapIter iter = switches.find(switchID);
    if (iter == switches.end()) {
        getLogger().warning("received Feature-reply from unknown Switch. (probably dead before reply reached)");
        return;
    }
    //parse message into an OpenFlow-Object
    of_object_t* multRep = zsdn::of_object_new_from_data_string_copy(message.getData());
    //check wether its a Port-Description or not
    if (multRep->object_id != OF_PORT_DESC_STATS_REPLY) {
        of_object_delete(multRep);
        return;
    } //TODO on change: implement other types
    //get SwitchAdapter from data, which sent the message
    Switch& aSwitch = switches[switchID];

    //delete portlist(so it is updated from the message
    aSwitch.ports.clear();
    //get List with ports from the Open-Flow-Object
    of_list_port_desc_t* listPortDescT = of_port_desc_stats_reply_entries_get(multRep);
    //create a port-desc-object as iterator for the list
    of_port_desc_t* portDescT = (of_port_desc_t*) malloc(64);
    //go to the first spot in the list
    int error = of_list_port_desc_first(listPortDescT, portDescT);
    if (error != OF_ERROR_RANGE) {//if its no out-of-range-error continue
        do {
            //Portinfo auslesen
            uint32_t portnr;
            of_port_desc_port_no_get(portDescT, &portnr);
            Port port = Port(portnr);
            of_mac_addr_t macAddrT;
            of_port_desc_hw_addr_get(portDescT, &macAddrT);
            port.mac_address = zsdn::NetUtils::mac_address_array_to_uint64(macAddrT.addr);
            of_port_name_t portName;
            of_port_desc_name_get(portDescT, &portName);
            port.port_name = std::string(portName);
            of_port_desc_config_get(portDescT, &port.config);
            of_port_desc_state_get(portDescT, &port.state);
            of_port_desc_curr_get(portDescT, &port.curr);
            of_port_desc_advertised_get(portDescT, &port.advertised);
            of_port_desc_supported_get(portDescT, &port.supported);
            of_port_desc_peer_get(portDescT, &port.peer);
            of_port_desc_curr_speed_get(portDescT, &port.curr_speed);
            of_port_desc_max_speed_get(portDescT, &port.max_speed);
            aSwitch.ports.insert(aSwitch.ports.end(), port);

            //go to the next Item in the list
            error = of_list_port_desc_next(listPortDescT, portDescT);
        } while (error != OF_ERROR_RANGE);//repeat till error is an out-of-range-error
    }
    getLogger().trace("received PortDesc-Reply for Switch: " + std::to_string(switchID));
    of_port_desc_delete(portDescT);
    of_list_port_desc_delete(listPortDescT);
    of_object_delete(multRep);
    //set gotPorts true
    aSwitch.got_ports = true;
    //if Switch is active it changed
    if(aSwitch.active){
        sendSwitchChanged(aSwitch);
    }else{
        //if SwitchInfo is available already, Switch becomes active
        if (aSwitch.switch_info_available) {
            aSwitch.active = true;
            switchBecameActive(aSwitch);
        }
    }
}

void SwitchRegistryModule::processEcho(const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) {
    //check if Switch available
    uint64_t switchID = sender.InstanceId;
    SwitchMapIter iter = switches.find(switchID);
    if (iter == switches.end()) {
        getLogger().warning(
                "received Echo from unknown Switch: " + std::to_string(switchID) + ", add him and send requests");
        addNewSwitch(switchID, of_message_version_get((of_message_t) message.getDataRaw()));
    } else {
        if (!switches[switchID].active) {
            if (!switches[switchID].switch_info_available) {
                getLogger().trace(
                        "received Echo from inactive Switch without Switch-Info: " + std::to_string(switchID) +
                        ", sending feature-request");
                sendFeatureRequest(switchID, of_message_version_get((of_message_t) message.getDataRaw()));
            }
            if (switches[switchID].of_version >= OF_VERSION_1_3) {
                if (!switches[switchID].got_ports) {

                    getLogger().trace(
                            "received Echo from inactive Switch without Port-Info: " + std::to_string(switchID) +
                            ", sending port-desc-request");
                    sendMultPortDescRequest(switchID, of_message_version_get((of_message_t) message.getDataRaw()));
                }
            }
        }
    }
}

void SwitchRegistryModule::processPortStatus(const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) {
    //check if Switch available
    uint64_t switchID = sender.InstanceId;
    SwitchMapIter iter = switches.find(switchID);
    if (iter == switches.end()) {
        getLogger().warning(
                "received Port-Status from unknown Switch: " + std::to_string(switchID));
        return;
    }
    Switch& aSwitch = switches[switchID];
    of_port_status_t* portStatusT = zsdn::of_object_new_from_data_string_copy(message.getData());
    uint8_t reason;
    of_port_status_reason_get(portStatusT, &reason);
    switch (reason){
        case OF_PORT_CHANGE_REASON_ADD:{
            //read the portDesc and add the new Port
            of_port_desc_t* portDescT = of_port_status_desc_get(portStatusT);
            uint32_t portnr;
            of_port_desc_port_no_get(portDescT, &portnr);
            Port port = Port(portnr);

            of_mac_addr_t macAddrT;
            of_port_desc_hw_addr_get(portDescT, &macAddrT);
            port.mac_address = zsdn::NetUtils::mac_address_array_to_uint64(macAddrT.addr);
            of_port_name_t portName;
            of_port_desc_name_get(portDescT, &portName);
            port.port_name = std::string(portName);
            of_port_desc_config_get(portDescT, &port.config);
            of_port_desc_state_get(portDescT, &port.state);
            of_port_desc_curr_get(portDescT, &port.curr);
            of_port_desc_advertised_get(portDescT, &port.advertised);
            of_port_desc_supported_get(portDescT, &port.supported);
            of_port_desc_peer_get(portDescT, &port.peer);
            if(portStatusT->version>=OF_VERSION_1_1) {
                of_port_desc_curr_speed_get(portDescT, &port.curr_speed);
                of_port_desc_max_speed_get(portDescT, &port.max_speed);
            }

            aSwitch.ports.insert(aSwitch.ports.end(), port);

            of_port_desc_delete(portDescT);
            of_port_status_delete(portStatusT);
            getLogger().trace("Port-Status-message added new Port " + std::to_string(portnr) +" for Switch:" + std::to_string(switchID));
            sendSwitchChanged(aSwitch);
            return;
        }
        case OF_PORT_CHANGE_REASON_DELETE:{
            //read which Port was deleted
            of_port_desc_t* portDescT = of_port_status_desc_get(portStatusT);
            uint32_t portnr;
            of_port_desc_port_no_get(portDescT, &portnr);

            //find that port in data and delete it
            for(int i=0;i<aSwitch.ports.size();i++){
                if(aSwitch.ports[i].switch_port==portnr){
                    aSwitch.ports.erase(aSwitch.ports.begin()+i);
                    break;
                }
            }

            of_port_desc_delete(portDescT);
            of_port_status_delete(portStatusT);
            getLogger().trace("Port-Status-message deleted Port " + std::to_string(portnr) +" for Switch:" + std::to_string(switchID));
            sendSwitchChanged(aSwitch);
            return;
        }
        case OF_PORT_CHANGE_REASON_MODIFY:{
            //read PortDesc
            of_port_desc_t* portDescT = of_port_status_desc_get(portStatusT);
            uint32_t portnr;
            of_port_desc_port_no_get(portDescT, &portnr);

            //find the Port that was updated
            for(int i=0;i<aSwitch.ports.size();i++){
                if(aSwitch.ports[i].switch_port==portnr){
                    //update the Port
                    of_mac_addr_t macAddrT;
                    of_port_desc_hw_addr_get(portDescT, &macAddrT);
                    aSwitch.ports[i].mac_address = zsdn::NetUtils::mac_address_array_to_uint64(macAddrT.addr);
                    of_port_name_t portName;
                    of_port_desc_name_get(portDescT, &portName);
                    aSwitch.ports[i].port_name = std::string(portName);
                    of_port_desc_config_get(portDescT, &aSwitch.ports[i].config);
                    of_port_desc_state_get(portDescT, &aSwitch.ports[i].state);
                    of_port_desc_curr_get(portDescT, &aSwitch.ports[i].curr);
                    of_port_desc_advertised_get(portDescT, &aSwitch.ports[i].advertised);
                    of_port_desc_supported_get(portDescT, &aSwitch.ports[i].supported);
                    of_port_desc_peer_get(portDescT, &aSwitch.ports[i].peer);
                    if(portStatusT->version>=OF_VERSION_1_1) {
                        of_port_desc_curr_speed_get(portDescT, &aSwitch.ports[i].curr_speed);
                        of_port_desc_max_speed_get(portDescT, &aSwitch.ports[i].max_speed);
                    }
                    break;
                }
            }

            of_port_desc_delete(portDescT);
            of_port_status_delete(portStatusT);
            getLogger().trace("Port-Status-message updated Port " + std::to_string(portnr) +" for Switch:" + std::to_string(switchID));
            sendSwitchChanged(aSwitch);
            return;
        }
        default:{
            of_port_status_delete(portStatusT);
            getLogger().warning(
                    "couldnt resolve reason for Port-Status-message (should be add(0), delete(1) or modify(2) for Switch: " + std::to_string(switchID));
            return;
        }
    }
}

common::topology::Switch* SwitchRegistryModule::convertToProto(uint64_t switchID) {
    //get Switch from data
    const Switch& aSwitch = switches[switchID];
    //create a new common::topology::Switch
    common::topology::Switch* pSwitch = new common::topology::Switch();
    //set SwitchInfo in common::topology::Switch
    pSwitch->set_switch_dpid(switchID);
    pSwitch->set_openflow_version(aSwitch.of_version);
    common::topology::SwitchSpecs* switchSpecs = new common::topology::SwitchSpecs();
    pSwitch->set_allocated_switch_specs(switchSpecs);
    switchSpecs->set_n_buffers(aSwitch.n_buffers);
    switchSpecs->set_n_tables(aSwitch.n_tables);
    switchSpecs->set_auxiliary_id(aSwitch.auxiliary_id);
    switchSpecs->set_capabilities(aSwitch.capabilities);
    switchSpecs->set_reserved(aSwitch.reserved);
    //add Ports and Portinfo to common::topology::Switch
    for (const Port& port : aSwitch.ports) {
        common::topology::SwitchPort* switchPort = pSwitch->add_switch_ports();
        common::topology::AttachmentPoint* attachmentPoint = new common::topology::AttachmentPoint();
        switchPort->set_allocated_attachment_point(attachmentPoint);
        attachmentPoint->set_switch_dpid(switchID);
        attachmentPoint->set_switch_port(port.switch_port);
        common::topology::PortSpecs* portSpecs = new common::topology::PortSpecs();
        switchPort->set_allocated_port_specs(portSpecs);
        portSpecs->set_mac_address(port.mac_address);
        portSpecs->set_port_name(port.port_name);
        portSpecs->set_config(port.config);
        portSpecs->set_state(port.state);
        portSpecs->set_curr(port.curr);
        portSpecs->set_advertised(port.advertised);
        portSpecs->set_supported(port.supported);
        portSpecs->set_peer(port.peer);
        portSpecs->set_curr_speed(port.curr_speed);
        portSpecs->set_max_speed(port.max_speed);
    }
    //return the complete common::topology::Switch
    return pSwitch;
}

void SwitchRegistryModule::switchBecameActive(Switch aSwitch) {
    //create common::topology::Switch
    common::topology::Switch* pSwitch = convertToProto(aSwitch.getSwitchID());
    //create message
    SwitchRegistryModule_Proto::From messageContainer;
    SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();
    switchEvent->set_allocated_switch_added(pSwitch);
    messageContainer.set_allocated_switch_event(switchEvent);

    getZmf()->publish(
            zmf::data::ZmfMessage(topicsSwitchAdded_, messageContainer.SerializeAsString()));
    getLogger().information("Switch became active: " + std::to_string(aSwitch.getSwitchID()));
}

void SwitchRegistryModule::sendSwitchChanged(Switch aSwitch) {
    //create common::topology::Switch
    common::topology::Switch* pSwitch = convertToProto(aSwitch.getSwitchID());
//create message
    SwitchRegistryModule_Proto::From messageContainer;
    SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();
    SwitchRegistryModule_Proto::From_SwitchEvent_SwitchChanged* switchChanged = new SwitchRegistryModule_Proto::From_SwitchEvent_SwitchChanged();
    switchChanged->set_allocated_switch_now(pSwitch);
    switchEvent->set_allocated_switch_changed(switchChanged);
    messageContainer.set_allocated_switch_event(switchEvent);

    getZmf()->publish(
            zmf::data::ZmfMessage(topicsSwitchChanged_, messageContainer.SerializeAsString()));
    getLogger().information("Switch changed: " + std::to_string(aSwitch.getSwitchID()));
}

void SwitchRegistryModule::addNewSwitch(uint64_t switchID, of_version_t ofVersion) {
    //add switch to map;
    switches.emplace(switchID, Switch(switchID, ofVersion));
    getLogger().information("Added new Switch: " + std::to_string(switchID));

    //send feature-request
    sendFeatureRequest(switchID, ofVersion);

    if(ofVersion>= OF_VERSION_1_3) {
        sendMultPortDescRequest(switchID, ofVersion);
    }
}

void SwitchRegistryModule::sendMultPortDescRequest(uint64_t switchID, of_version_t ofVersion) {
    //create and send port_desc-Request
    of_port_desc_stats_request_t* portReq = of_port_desc_stats_request_new(ofVersion);
    std::string portReq_serialized = zsdn::of_object_serialize_to_data_string(portReq);
    zmf::data::MessageType msgType = switchadapter_topics::TO().switch_adapter().switch_instance(
            switchID).openflow().of_1_0_barrier_request_of_1_3_multipart_request().build();
    getZmf()->publish(zmf::data::ZmfMessage(msgType, portReq_serialized));
    of_object_delete(portReq);
    getLogger().information("sent Port-Desc-Request for Switch: " + std::to_string(switchID));
}

void SwitchRegistryModule::sendFeatureRequest(uint64_t switchID, of_version_t ofVersion) {
    // create and send feature-request
    of_features_request_t* featReq = of_features_request_new(ofVersion);
    std::string featReq_serialized = zsdn::of_object_serialize_to_data_string(featReq);
    zmf::data::MessageType msgType = switchadapter_topics::TO().switch_adapter().switch_instance(
            switchID).openflow().features_request().build();
    getZmf()->publish(zmf::data::ZmfMessage(msgType, featReq_serialized));
    of_object_delete(featReq);
    getLogger().information("sent Feature-Request for Switch: " + std::to_string(switchID));
}
