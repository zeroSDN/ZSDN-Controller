//
// Created by zsdn on 7/24/15.
//

#include <zmf/ModuleHandle.hpp>
#include <zmf/IZmfInstanceAccess.hpp>
#include "RequestUtils.h"
#include "NetUtils.h"

zsdn::RequestUtils::RequestResult zsdn::RequestUtils::sendRequest(zmf::IZmfInstanceAccess& zmfInstance,
                                                                  const google::protobuf::Message& request,
                                                                  google::protobuf::Message& response,
                                                                  const zmf::data::MessageType& requestType,
                                                                  const uint16_t toModuleType,
                                                                  const uint16_t requiredModuleVersion,
                                                                  const uint32_t waitForAnswerMs) {

    // first try if we can even serialize the request successfully
    std::string serializedRequest;
    bool successfullySerialized = request.SerializeToString(&serializedRequest);

    if (!successfullySerialized) {
        return REQUEST_SERIALIZATION_FAILED;
    }

    // request all handles for the requested module type
    std::shared_ptr<zmf::data::ModuleHandle> module = zmfInstance.getPeerRegistry()->getAnyPeerWithTypeVersion(
            toModuleType, requiredModuleVersion, true);

    // at least one has to be present.
    if (module.get() == nullptr) {
        return NO_MODULE_INSTANCE_FOUND;

    }

    zmf::data::ZmfMessage requestMsg(requestType, serializedRequest);

    // send the message inside the zmf system with the first DeviceManager as receiver
    zmf::data::ZmfInReply responseHandle = zmfInstance.sendRequest(module->UniqueId, requestMsg);
    // wait for the response
    std::future_status status = responseHandle.wait_for(std::chrono::milliseconds(waitForAnswerMs));
    if (status == std::future_status::ready) {
        // get response message
        try {
            zmf::data::ZmfMessage responseMsg = responseHandle.get();
            // fill in data
            bool successfulParse = response.ParseFromString(responseMsg.getData());

            if (successfulParse) {
                return SUCCESS;
            } else {
                return RESPONSE_PARSE_FAILED;
            }
        } catch (const std::exception& ex) {

            return NO_MODULE_INSTANCE_FOUND;
        }

    }
    else {
        return TIMEOUT;
    }
}

void zsdn::RequestUtils::OFCorrespondenceHandle::startCorrespondence(zmf::IZmfInstanceAccess& zmfInstance,
                                                                     zmf::data::MessageType requestTopic,
                                                                     uint32_t requesterID,
                                                                     of_object_t* request,
                                                                     zmf::data::MessageType answerTopic,
                                                                     std::function<void(CorespondStatus status,
                                                                                        of_object_t* result)> callback) {
    this->zmfInstance = &zmfInstance;
    this->requestTopic = requestTopic;
    this->request = request;
    this->status = CorespondStatus::CREATED;
    this->answerTopic = answerTopic;
    this->callback = callback;
    this->requesterID = requesterID;
    of_object_xid_set(request, requesterID);
    zmfInstance.publish(zmf::data::ZmfMessage(requestTopic, zsdn::of_object_serialize_to_data_string(request)));
    this->status = CorespondStatus::WAITING;
    subs.push_back(zmfInstance.subscribe(answerTopic,
                                         [this](const zmf::data::ZmfMessage& msg,
                                                const zmf::data::ModuleUniqueId& sender) {
                                             try {
                                                 of_object_t* ofPacketIn = zsdn::of_object_new_from_data_string_copy(
                                                         msg.getData());
                                                 uint32_t id;
                                                 of_object_xid_get(ofPacketIn, &id);
                                                 if (id == this->requesterID) {
                                                     this->status = CorespondStatus::FINISHED;
                                                     this->subs[0].unsubscribe();
                                                     this->callback(status, ofPacketIn);
                                                 } else {
                                                     of_object_delete(ofPacketIn);
                                                 }
                                             } catch (...) {
                                                 this->status = CorespondStatus::FAILURE_PARSING_ANSWER;
                                                 this->subs[0].unsubscribe();
                                                 this->callback(status, nullptr);
                                             }

                                         }));

}

std::shared_ptr<zsdn::RequestUtils::OFCorrespondenceHandle> zsdn::RequestUtils::ofCorrespondence(
        zmf::IZmfInstanceAccess& zmfInstance,
        zmf::data::MessageType requestTopic,
        uint32_t requesterID,
        of_object_t* request,
        zmf::data::MessageType answerTopic,
        std::function<void(CorespondStatus status,
                           of_object_t* result)> callback) {
    std::shared_ptr<OFCorrespondenceHandle> result(new OFCorrespondenceHandle());
    result->startCorrespondence(zmfInstance, requestTopic, requesterID, request, answerTopic, callback);


    return result;
}