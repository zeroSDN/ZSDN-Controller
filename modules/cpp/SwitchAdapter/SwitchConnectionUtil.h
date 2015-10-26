#ifndef SWITCHADAPTER_SWITCHCONNECTIONUTIL_H
#define SWITCHADAPTER_SWITCHCONNECTIONUTIL_H

#include <Poco/Net/SocketStream.h>
#include <memory>
#include <zmf/MessageType.hpp>
#include <zsdn/topics/SwitchAdapter_topics.hpp>
#include <tins/ethernetII.h>
#include <tins/ip.h>
#include <tins/ipv6.h>

extern "C" {
#include "loci/loci.h"
}

/**
 * @brief   This class contains helper functions for communicating with OpenFlow switches.
 *
 * @author  Andre Kutzleb
 *
 * @date    18.07.2015
 */
class SwitchConnectionUtil {

public:
    /// length of the OpenFlow message header in bytes
    static const uint32_t OF_HEADER_LENGTH = 8;

    /// constants used for experimental topic construction
    static const int ethPayloadOffset = 14;
    static const int etherTypePos = 12;
    static const int ipv4 = 0x0800;
    static const int ipv6 = 0x86DD;
    static const int protoPos4 = ethPayloadOffset + 9;
    static const int protoPos6 = ethPayloadOffset + 6;


    /**
     * Object representing the outcome of a handshake with an OpenFlow switch.
     */
    struct HandshakeResult {

        /// Possible outcomes of a handshake.
        enum Status {
            NONE,
            HELLO_FAILED,
            WRONG_OPENFLOW_VERSION,
            FEATURE_REQUEST_FAILED,
            SUCCESS
        };

        /**
         * Constructs a HandshakeResult.
         *
         * @param status
         *        the outcome of the handshake.
         * @param ofVersion
         *        the OpenFlow version of the switch
         *        (only if the status is >= FEATURE_REQUEST_FAILED. else OF_VERSION_UNKNOWN)
         * @param switchId
         *        the switchId of the switch (only if the status is SUCCESS. else 0)
         */
        HandshakeResult(
                const Status& status,
                const of_version_t& switchOpenFlowVersion,
                const uint64_t switchId)
                :
                status(status),
                switchOpenFlowVersion(switchOpenFlowVersion),
                switchId(switchId) { }

        /// Outcome of the handshake.
        Status status = NONE;
        /// version of the switch
        of_version_t switchOpenFlowVersion = OF_VERSION_UNKNOWN;
        /// switchId of the switch
        uint64_t switchId = 0;

        /**
         * Enum to string helper.
         *
         * @return the openFlow version of this HandshakeResult, as string.
         */
        std::string versionToString() {
            switch (switchOpenFlowVersion) {
                case OF_VERSION_1_0:
                    return "OF_VERSION_1_0";
                case OF_VERSION_1_1:
                    return "OF_VERSION_1_1";
                case OF_VERSION_1_2:
                    return "OF_VERSION_1_2";
                case OF_VERSION_1_3:
                    return "OF_VERSION_1_3";
                case OF_VERSION_1_4:
                    return "OF_VERSION_1_4";
                default :
                    return "OF_VERSION_UNKNOWN";
            }
        }

        /**
         * returns a textual representation of this HandshakeResult.
         *
         * @return a textual representation of this HandshakeResult.
         */
        std::string toString() {
            switch (status) {
                case NONE:
                    return "NONE";
                case HELLO_FAILED:
                    return "HELLO_FAILED";
                case WRONG_OPENFLOW_VERSION:
                    return "WRONG_OPENFLOW_VERSION (" + versionToString() + ")";
                case FEATURE_REQUEST_FAILED:
                    return "FEATURE_REQUEST_FAILED";
                case SUCCESS:
                    return "SUCCESS";
                default:
                    return "INVALID";
            }
        }
    };

    /**
     * Attempts to perform an OpenFlow handshake over the given socket and returns the outcome.
     *
     * Exchanges Hello, then a Feature-Request/Reply with the opposite side.
     *
     * @param socket
     *        the connection used for the handshake.
     * @param ofVersion
     *        the openFlow version used in the handshake. the handshake will fail if the switch does nto speak this
     *        version.
     * @return a HandshakeResult object detailing the result of the handshake.
     */
    static HandshakeResult attemptHandshake(std::shared_ptr<Poco::Net::SocketStream> socket,
                                            const of_version_t openFlowVersion);


    /**
     * Reads an OpenFlow message from the given socket into the given buffer.
     *
     * @param buffer
     *        the buffer that the function will write to.
     * @param fromSocket
     *        the socket from which the OpenFlow message will be read.
     * @return true if the read was sucessful, or false if the underlying connection failed while reading.
     */
    static inline bool parseOpenFlowFromSocketToBuffer(uint8_t* buffer, Poco::Net::SocketStream& fromSocket) {


        fromSocket.read((char*) buffer, OF_HEADER_LENGTH);

        if (!fromSocket.good()) {
            return false;
        }

        int toReceive = of_message_length_get(buffer) - OF_HEADER_LENGTH;

        if (toReceive <= 0) {
            return true;
        }

        fromSocket.read((char*) (buffer + OF_HEADER_LENGTH), toReceive);

        return fromSocket.good();
    }


    /**
     *  Writes the given OpenFlow message (contained in the buffer) to the given socket.
     *
     * @param toSocket
     *        the socket where the OpenFlow message will be written to.
     * @param buffer
     *        the buffer containing an OpenFlow message to be written to the socket.
     * @return true if the write was successful, or false if the underlying connection failed while writing.
     */
    static inline bool writeOpenFlowToSocket(Poco::Net::SocketStream& toSocket,
                                             uint8_t* buffer) {
        toSocket.write((const char*) buffer, of_message_length_get(buffer));
        return toSocket.good();
    }


    /**
     *  Writes the given OpenFlow message to the given socket.
     *
     * @param toSocket
     *        the socket where the OpenFlow message will be written to.
     * @param openFlowMsg
     *        the message to be written to the socket.
    * @return true if the write was scucessful, or false if the underlying connection failed while writing.
     */
    static inline bool writeOpenFlowToSocket(Poco::Net::SocketStream& toSocket,
                                             of_object_t* const openFlowMsg) {
        return writeOpenFlowToSocket(toSocket, OF_OBJECT_TO_MESSAGE(openFlowMsg));
    }


    /**
     * Extends to basic packetIn topic with ethertype / protocoltype (if ipv4/ipv6).
     * Uses fixed offsets and may produce wrong results. may be faster than the libtins based topic construction.
     *
     * USE AT YOUR OWN RISK.
     *
     * @param topic
     *        topic to append bytes (at least ethertype) to.
     * @param packetIn
     *        OpenFlow PACKET_IN message with an EthernetII frame as payload
     */
    static inline void buildSimplePacketInTopic(MessageType& topic, of_packet_in_t* const packetIn) {

        of_octets_t payloadOfPacketIn;
        of_packet_in_data_get(packetIn, &payloadOfPacketIn);


        const uint16_t ethertype =
                (payloadOfPacketIn.data[etherTypePos] << 8) | payloadOfPacketIn.data[etherTypePos + 1];
        topic.appendMatch16(ethertype);

        switch (ethertype) {
            case ipv4:
                topic.appendMatch8(payloadOfPacketIn.data[protoPos4]);
                break;
            case ipv6:
                topic.appendMatch8(payloadOfPacketIn.data[protoPos6]);
                break;
            default:
                break;
        }
    }


    /**
     *  Constructs a MessageType (topic) based on the payload of the given packetIn OpenFlow message using the libtins
     *  library to identify ethertype/protocol type.
     * @param topic
     *        topic to append bytes (at least ethertype) to.
     * @param packetIn
     *        OpenFlow PACKET_IN message with an EthernetII frame as payload
     * @param switchId
     *        for the HPMOM packet, UDP topics are appended with the id of the switch - this is not necessary normally.
     *
     * @return a MessageType depending on the payload according to the definitions in SwitchAdapter.topics
     */
    static inline void buildPacketInTopic(MessageType& topic, of_packet_in_t* const packetIn, const uint64_t switchId) {

        of_octets_t payloadOfPacketIn;
        of_packet_in_data_get(packetIn, &payloadOfPacketIn);

        // read/interpret data as ethII packet
        Tins::EthernetII ethPacket(payloadOfPacketIn.data, (uint32_t) payloadOfPacketIn.bytes);

        uint16_t payLoadType = ethPacket.payload_type();
        // add packet type to base topic (IPv4/IPv6/etc)
        topic.appendMatch16(payLoadType);

        //inner_pdu can contain TCP/UDP/ICMP/IGP etc.
        Tins::PDU* inner = ethPacket.inner_pdu();

        // The EthernetII frame may not contain a payload.
        if (inner != nullptr) {

            // handle IPv4 packet (its protocol)
            if (inner->pdu_type() == Tins::PDU::PDUType::IP) {

                Tins::IP* ipv4Packet = (Tins::IP*) inner;
                uint8_t ipv4ProtocolType = ipv4Packet->protocol();
                // add protocol type to message topic (TCP/UDP/etc)
                topic.appendMatch8(ipv4ProtocolType);

                //constant UDP- this helps hpMOM dispatcher to only receive udp from a specific switch
                if (ipv4ProtocolType == 0x11) {
                    topic.appendMatch64(switchId);
                }
            }

                // handle IPv6 packet (its protocol)
            else if (inner->pdu_type() == Tins::PDU::PDUType::IPv6) {

                Tins::IPv6* ipv6Packet = (Tins::IPv6*) inner;
                uint8_t ipv6ProtocolType = ipv6Packet->next_header();
                // add protocol type to message topic (TCP/UDP/etc)
                topic.appendMatch8(ipv6ProtocolType);
            }
        }
    }
};


#endif //SWITCHADAPTER_SWITCHCONNECTIONUTIL_H


