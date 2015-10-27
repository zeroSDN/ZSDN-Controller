//
// Created by zsdn on 7/24/15.
//

#include <google/protobuf/message.h>

#ifndef ZSDN_COMMONS_REQUESTUTIL_H
#define ZSDN_COMMONS_REQUESTUTIL_H

#include "zmf/IZmfInstanceAccess.hpp"
#include "zmf/MessageType.hpp"
#include "LociExtensions.h"

namespace zsdn {


    class RequestUtils {

    public:

        enum RequestResult {
            REQUEST_SERIALIZATION_FAILED,
            NO_MODULE_INSTANCE_FOUND,
            TIMEOUT,
            RESPONSE_PARSE_FAILED,
            SUCCESS
        };

        static RequestResult sendRequest(zmf::IZmfInstanceAccess& zmfInstance,
                                         const google::protobuf::Message& request,
                                         google::protobuf::Message& response,
                                         const zmf::data::MessageType& requestType,
                                         const uint16_t toModuleType,
                                         const uint16_t requiredModuleVersion,
                                         const uint32_t waitForAnswerMs = 1000);

        enum CorespondStatus {
            CREATED,
            WAITING,
            FINISHED,
            FAILURE_UNKNOWN,
            FAILURE_PARSING_ANSWER
        };

        class OFCorrespondenceHandle {

        public:
            std::vector<zmf::IZmfInstanceAccess::SubscriptionHandle> subs;
            std::function<void(CorespondStatus status, of_object_t* result)> callback;
            zmf::IZmfInstanceAccess* zmfInstance;
            zmf::data::MessageType requestTopic, answerTopic;
            of_object_t* request = nullptr;
            CorespondStatus status;
            uint32_t requesterID;

            OFCorrespondenceHandle() { };

            ~OFCorrespondenceHandle() {
                if (request != nullptr) {
                    of_object_delete(request);
                }
            }

            void startCorrespondence(zmf::IZmfInstanceAccess& zmfInstance,
                                     zmf::data::MessageType requestTopic,
                                     uint32_t requesterID,
                                     of_object_t* request,
                                     zmf::data::MessageType answerTopic,
                                     std::function<void(CorespondStatus status,
                                                        of_object_t* result)> callback);

        };

        static std::shared_ptr<OFCorrespondenceHandle> ofCorrespondence(zmf::IZmfInstanceAccess& zmfInstance,
                                                                        zmf::data::MessageType requestTopic,
                                                                        uint32_t requesterID,
                                                                        of_object_t* request,
                                                                        zmf::data::MessageType answerTopic,
                                                                        std::function<void(CorespondStatus status,
                                                                                           of_object_t* result)> callback);

    };
}
#endif //ZSDN_COMMONS_REQUESTUTIL_H
