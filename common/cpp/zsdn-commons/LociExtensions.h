//
// Created by zsdn on 7/20/15.
//

#ifndef ZSDN_COMMONS_LOCIEXTENSIONS_H
#define ZSDN_COMMONS_LOCIEXTENSIONS_H

#include <Poco/Exception.h>
#include "ZsdnTypes.h"

extern "C" {
#include "loci/loci.h"
}

namespace zsdn {
    inline void of_object_delete_ignoring_buffer(of_object_t* obj) {
        obj->wbuf = nullptr;
        of_object_delete(obj);
    }

    inline std::string of_object_serialize_to_data_string(of_object_t* obj) {
        uint8_t* tempBuffer = OF_OBJECT_TO_MESSAGE(obj);
        return std::string((char*) tempBuffer, obj->length);
    }


    inline of_object_t* of_object_new_from_message_copy(of_message_t msg, int len) {
        of_message_t buffer = new uint8_t[len];
        memcpy(buffer, msg, len);
        return of_object_new_from_message(buffer, len);
    }

    inline of_object_t* of_object_new_from_data_string_copy(const std::string& dataString) {
        return of_object_new_from_message_copy((of_message_t) dataString.data(), dataString.size());
    }


    inline of_flow_add_t* create_layer2_forwarding_flow(of_version_t openFlowVersion,
                                                        uint8_t tableId,
                                                        uint16_t idleTimeout,
                                                        const zsdn::Port inPort,
                                                        const zsdn::Port outPort,
                                                        const zsdn::MAC destinationMac) {


        if (openFlowVersion != OF_VERSION_1_3 && openFlowVersion != OF_VERSION_1_0) {
            throw new Poco::NotImplementedException("Only OF10 and OF13 supported");
        }

        // create main object (flow_add)
        of_flow_add_t* flowToAdd = of_flow_add_new(openFlowVersion);

        // has to be set as default:
        of_flow_add_buffer_id_set(flowToAdd, OF_BUFFER_ID_NO_BUFFER);

        if (openFlowVersion == OF_VERSION_1_3) {
            // which table the flow should b e installed in. table 0 seems to exist and work for default 1.3 OpenVSwitches
            of_flow_add_table_id_set(flowToAdd, tableId);
        }

        of_flow_add_idle_timeout_set(flowToAdd, idleTimeout);

        // what we are going to match on
        of_match_t match;
        // set all bits in the match to 0
        memset(&match, 0, sizeof(match));

        // example mac - address which will be used as match
        uint8_t* targetMac = zsdn::NetUtils::uint64_to_mac_address_array(destinationMac);
        // copy our mac into the match data structure
        memcpy(match.fields.eth_dst.addr, targetMac, 6);

        // let the match know that we will actually match on targetMAC
        OF_MATCH_MASK_ETH_DST_EXACT_SET(&match);

        // we will also match on the inPort. here we set the value
        match.fields.in_port = inPort;
        // and let the match know that we will actually match on it
        OF_MATCH_MASK_IN_PORT_EXACT_SET(&match);

        // we set the match into our flow_add object
        of_flow_add_match_set(flowToAdd, &match);

        // the apply action contains a list of actions that will be executed in order
        of_list_action_t* list = of_list_action_new(openFlowVersion);

        // the only action inside is an output-on-port action.
        of_action_output_t* output = of_action_output_new(openFlowVersion);

        // set the port of the out action. output on port 2
        of_action_output_port_set(output, outPort);

        // bind the outout action to the list of actions
        of_list_action_append(list, output);

        if (openFlowVersion == OF_VERSION_1_3) {

            // a flow will execute a set of instructions when a packet matches.
            of_list_instruction_t* instructions = of_list_instruction_new(openFlowVersion);

            // our only action in the instruction set is an apply action.
            of_instruction_apply_actions_t* applyActions = of_instruction_apply_actions_new(openFlowVersion);


            // add list of actions to apply-actions instruction.
            of_instruction_apply_actions_actions_set(applyActions, list);

            // add apply-actions instruction to set of instructions.
            of_list_instruction_append(instructions, applyActions);

            // add list of instructions to set of all instructions.
            of_flow_add_instructions_set(flowToAdd, instructions);

            of_list_instruction_delete(instructions);
            of_instruction_apply_actions_delete(applyActions);

        } else if (openFlowVersion == OF_VERSION_1_0) {

            of_flow_add_actions_set(flowToAdd, list);
        }


        // cleanup
        delete[] targetMac;
        of_list_action_delete(list);
        of_action_output_delete(output);

        return flowToAdd;
    }
}

#endif //ZSDN_COMMONS_LOCIEXTENSIONS_H
