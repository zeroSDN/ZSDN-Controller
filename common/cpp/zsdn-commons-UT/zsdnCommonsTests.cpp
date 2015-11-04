//
// Created by Andre Kutzleb on 8/07/15.
//

#include "zsdnCommonsTests.h"
#include <NetUtils.h>
#include "zsdn/proto/CommonTopology.pb.h"
#include "NetworkGraph.h"
#include <RequestUtils.h>
#include <LociExtensions.h>
#include <Poco/Exception.h>
#include <MemUtils.h>
#include "zsdn/topics/TopologyModuleTopics.hpp"

zsdnCommonsTests::zsdnCommonsTests() {

}


void zsdnCommonsTests::testMacConversions() {

    // Testdata

    std::vector<uint64_t> testMacs = {
            0x268a599f01c9,
            0x7d96a562b119,
            0xee03f3efe023,
            0xddd3a137617f,
            0x4c70b98be263,
            0x15ecf4e64f53,
            0x648b59d810ba,
            0x322fc4241bc9,
            0x978664a53843,
            0x0a8ad9dcba72
    };

    std::vector<std::vector<uint8_t>> testMacsAsArray = {
            {0x26, 0x8a, 0x59, 0x9f, 0x01, 0xc9},
            {0x7d, 0x96, 0xa5, 0x62, 0xb1, 0x19},
            {0xee, 0x03, 0xf3, 0xef, 0xe0, 0x23},
            {0xdd, 0xd3, 0xa1, 0x37, 0x61, 0x7f},
            {0x4c, 0x70, 0xb9, 0x8b, 0xe2, 0x63},
            {0x15, 0xec, 0xf4, 0xe6, 0x4f, 0x53},
            {0x64, 0x8b, 0x59, 0xd8, 0x10, 0xba},
            {0x32, 0x2f, 0xc4, 0x24, 0x1b, 0xc9},
            {0x97, 0x86, 0x64, 0xa5, 0x38, 0x43},
            {0x0a, 0x8a, 0xd9, 0xdc, 0xba, 0x72}
    };

    std::vector<std::string> testMacsAsStrings = {
            "26:8a:59:9f:01:c9",
            "7d:96:a5:62:b1:19",
            "ee:03:f3:ef:e0:23",
            "dd:d3:a1:37:61:7f",
            "4c:70:b9:8b:e2:63",
            "15:ec:f4:e6:4f:53",
            "64:8b:59:d8:10:ba",
            "32:2f:c4:24:1b:c9",
            "97:86:64:a5:38:43",
            "0a:8a:d9:dc:ba:72"
    };

    for (int i = 0; i < testMacs.size(); i++) {
        const uint64_t mac = testMacs[i];
        uint8_t* macAsArr = testMacsAsArray[i].data();
        Tins::EthernetII::address_type address_type(macAsArr);

        // test uint64_to_mac_address_array
        const uint8_t* const asArray = zsdn::NetUtils::uint64_to_mac_address_array(mac);
        for (int i = 0; i < 6; i++) {
            CPPUNIT_ASSERT_EQUAL(macAsArr[i], asArray[i]);
        }


        // test mac_address_array_to_uint64
        CPPUNIT_ASSERT_EQUAL(mac, zsdn::NetUtils::mac_address_array_to_uint64(macAsArr));

        // test mac_address_array_to_uint64
        CPPUNIT_ASSERT_EQUAL(mac, zsdn::NetUtils::mac_address_tins_to_uint64(Tins::EthernetII::address_type(macAsArr)));

        // test mac_address_string_to_uint64
        CPPUNIT_ASSERT_EQUAL(mac, zsdn::NetUtils::mac_address_string_to_uint64(testMacsAsStrings[i]));

        // test uint64_to_mac_address_string
        CPPUNIT_ASSERT_EQUAL(testMacsAsStrings[i], zsdn::NetUtils::uint64_to_mac_address_string(mac));

        delete[] asArray;
    }
}

void zsdnCommonsTests::testNetworkGraph() {
    using namespace common::topology;

    Topology* topo = zsdn::NetworkGraph::createTestTopology();

    zsdn::NetworkGraph graph(*topo);

    delete topo;

    {
        // "edge"-attachmentPoints (not used in switch link)
        std::vector<zsdn::AttachmentPoint> path = graph.getShortestPath(1, 1, 9, 3);
        CPPUNIT_ASSERT_EQUAL(uint32_t(10),uint32_t(path.size()));
    }

    {
        // "edge"-attachmentPoint + switch connection point
        std::vector<zsdn::AttachmentPoint> path = graph.getShortestPath(1, 1, 9, 2);
        CPPUNIT_ASSERT_EQUAL(uint32_t(9),uint32_t(path.size()));
    }

    {
        // impossible path
        std::vector<zsdn::AttachmentPoint> path = graph.getShortestPath(1, 1, 10, 2);
        CPPUNIT_ASSERT_EQUAL(uint32_t(0),uint32_t(path.size()));
    }

    {
        // path to self
        std::vector<zsdn::AttachmentPoint> path = graph.getShortestPath(1, 1, 1, 1);
        CPPUNIT_ASSERT_EQUAL(uint32_t(0),uint32_t(path.size()));
    }

    {
        // single hop
        std::vector<zsdn::AttachmentPoint> path = graph.getShortestPath(5, 2, 6, 1);
        CPPUNIT_ASSERT_EQUAL(uint32_t(2),uint32_t(path.size()));
    }
}

void zsdnCommonsTests::testRequestUtils() {
  /*  zmf::IZmfInstanceAccess* z = nullptr;
    google::protobuf::Message* m = nullptr;
    zmf::data::MessageType* mt = nullptr;
   zsdn::RequestUtils::sendRequest(*z, *m,*m , *mt,0,0);*/

}

uint8_t packetInMessage[0x0400] = {0x04, 0x0a, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x60, 0x00,
                                   0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                                   0x00, 0x0c, 0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x01, 0x08, 0x00, 0x45, 0x00, 0x00, 0x33, 0xbb, 0xad, 0x40, 0x00, 0x40,
                                   0x11, 0x6b, 0x0a, 0x0a, 0x00, 0x00, 0x01, 0x0a, 0x00, 0x00, 0x02, 0xd5, 0x41,
                                   0x05, 0x39, 0x00, 0x1f, 0x7c, 0x3c, 0x4a, 0x61, 0x76, 0x61, 0x20, 0x53, 0x6f,
                                   0x75, 0x72, 0x63, 0x65, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x53, 0x75, 0x70, 0x70,
                                   0x6f, 0x72, 0x74};

void zsdnCommonsTests::testLociExtensions() {

    of_object_t* of10 = nullptr;
    of_object_t* of13 = nullptr;

    CPPUNIT_ASSERT_NO_THROW(of10 = zsdn::create_layer2_forwarding_flow(OF_VERSION_1_0, 0, 10, 1, 1, 1));
    CPPUNIT_ASSERT_NO_THROW(of13 = zsdn::create_layer2_forwarding_flow(OF_VERSION_1_3, 0, 10, 1, 1, 1));

    of_object_delete(of10);
    of_object_delete(of13);

    of_object_t* packetIn = zsdn::of_object_new_from_message_copy(packetInMessage,sizeof(packetInMessage));

    CPPUNIT_ASSERT(packetIn != nullptr);
    of_object_delete(packetIn);

}

void zsdnCommonsTests::testTopicConstruction() {
    zmf::data::MessageType t = zsdn::modules::TopologyModuleTopics<zmf::data::MessageType>().from().topology_module().topology_changed_event().build();
}
