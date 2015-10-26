#include "SwitchConnectionUtil.h"

SwitchConnectionUtil::HandshakeResult SwitchConnectionUtil::attemptHandshake(
        std::shared_ptr<Poco::Net::SocketStream> socket,
        const of_version_t openFlowVersion) {

    uint8_t* receiveBuffer = new uint8_t[65536];
    bool ioSuccess;

    // Do handshake - send an OpenFlow hello message to the switch
    of_hello_t* hello = of_hello_new(openFlowVersion);
    ioSuccess = SwitchConnectionUtil::writeOpenFlowToSocket(*socket, hello);
    of_hello_delete(hello);

    if (ioSuccess) {
        socket->flush();
        ioSuccess = socket->good();
    }

    if (!ioSuccess) {
        delete[]  receiveBuffer;
        return HandshakeResult(HandshakeResult::HELLO_FAILED, OF_VERSION_UNKNOWN, 0);
    }

    ioSuccess = SwitchConnectionUtil::parseOpenFlowFromSocketToBuffer(receiveBuffer, *socket);

    if (!ioSuccess) {
        delete[]  receiveBuffer;
        return HandshakeResult(HandshakeResult::HELLO_FAILED, OF_VERSION_UNKNOWN, 0);
    }

    if (of_message_type_get(receiveBuffer) != OF_OBJ_TYPE_HELLO) {
        delete[]  receiveBuffer;
        return HandshakeResult(HandshakeResult::HELLO_FAILED, OF_VERSION_UNKNOWN, 0);
    }

    of_version_t switchVersion = of_message_version_get(receiveBuffer);

    if (switchVersion != openFlowVersion) {
        // send ERROR message to switch
        of_hello_failed_error_msg_t* errorMsg = of_hello_failed_error_msg_new(openFlowVersion);
        SwitchConnectionUtil::writeOpenFlowToSocket(*socket, errorMsg);
        of_hello_failed_error_msg_delete(errorMsg);

        delete[]  receiveBuffer;
        return HandshakeResult(HandshakeResult::WRONG_OPENFLOW_VERSION, switchVersion, 0);
    }

    // now we send a feature request so we know the Switch id (DPID) of the switch that just connected.
    of_features_request_t* featReq = of_features_request_new(openFlowVersion);
    ioSuccess = SwitchConnectionUtil::writeOpenFlowToSocket(*socket, featReq);
    of_features_request_delete(featReq);

    if (ioSuccess) {
        socket->flush();
        ioSuccess = socket->good();
    }

    if (!ioSuccess) {
        delete[]  receiveBuffer;
        return HandshakeResult(HandshakeResult::FEATURE_REQUEST_FAILED, switchVersion, 0);
    }


    // receive featureReply
    ioSuccess = SwitchConnectionUtil::parseOpenFlowFromSocketToBuffer(receiveBuffer, *socket);

    if (!ioSuccess) {
        delete[]  receiveBuffer;
        return HandshakeResult(HandshakeResult::FEATURE_REQUEST_FAILED, switchVersion, 0);
    }


    if (of_message_type_get(receiveBuffer) != OF_OBJ_TYPE_FEATURES_REPLY) {
        delete[]  receiveBuffer;
        return HandshakeResult(HandshakeResult::FEATURE_REQUEST_FAILED, switchVersion, 0);
    }

    uint64_t switchId;
    of_features_reply_t* featRep = of_object_new_from_message(receiveBuffer, of_message_length_get(receiveBuffer));
    of_features_reply_datapath_id_get(featRep, &switchId);
    of_object_wire_buffer_steal(featRep, &receiveBuffer);
    of_features_reply_delete(featRep);

    delete[]  receiveBuffer;
    return HandshakeResult(HandshakeResult::SUCCESS, switchVersion, switchId);
}





