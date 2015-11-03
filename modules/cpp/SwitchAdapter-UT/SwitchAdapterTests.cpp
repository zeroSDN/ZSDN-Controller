//
// Created by zsdn on 6/25/15.
//

#include <SwitchAdapter.hpp>
#include <SwitchAdapterRunner.h>
#include <LociExtensions.h>
#include "SwitchAdapterTests.h"
#include "SwitchConnectionUtil.h"
#include "zsdn/topics/SwitchAdapterTopics.hpp"
#include "MemUtils.h"
#include "dummyModules/DummyModule.hpp"
#include "zmf/ZmfInstance.hpp"

#include "UnittestConfigUtil.hpp"

extern "C" {
#include <loci/loci.h>
}


SwitchAdapterTests::SwitchAdapterTests() {

}

uint8_t helloMessage[8] = {0x04, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};

uint8_t featRepMessage[32] = {0x04, 0x06, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47,
                              0x00, 0x00, 0x00, 0x00};

uint8_t packetInMessage[0x0400] = {0x04, 0x0a, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x60, 0x00,
                                   0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                                   0x00, 0x0c, 0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x01, 0x08, 0x00, 0x45, 0x00, 0x00, 0x33, 0xbb, 0xad, 0x40, 0x00, 0x40,
                                   0x11, 0x6b, 0x0a, 0x0a, 0x00, 0x00, 0x01, 0x0a, 0x00, 0x00, 0x02, 0xd5, 0x41,
                                   0x05, 0x39, 0x00, 0x1f, 0x7c, 0x3c, 0x4a, 0x61, 0x76, 0x61, 0x20, 0x53, 0x6f,
                                   0x75, 0x72, 0x63, 0x65, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x53, 0x75, 0x70, 0x70,
                                   0x6f, 0x72, 0x74};

uint8_t packetInARPRequest[0x0054] = {0x04, 0x0a, 0x00, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00,
                                      0x2a,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
                                      0x0c,
                                      0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00,
                                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x8e, 0xf7, 0x27, 0x1a, 0x78, 0x4b, 0x08,
                                      0x06,
                                      0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x8e, 0xf7, 0x27, 0x1a, 0x78,
                                      0x4b,
                                      0x0a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x02
};

uint8_t packetOutLinkDiscovery[0x003f] = {0x04, 0x0d, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x10, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x51, 0x1f, 0x08, 0x1f, 0x10, 0x80, 0xfa, 0xc4, 0xbf, 0xf1, 0x29};

/**
 * immitates a Switch (the part that happens on initialisation)
 */
void SwitchAdapterTests::testSwitchAdapter() {


    Poco::Net::SocketAddress address("127.0.0.1:6633");

    SwitchAdapterRunner runner_13(OF_VERSION_1_3, 6633,
                                  UT_CONFIG_FILE);

    // may fail when port blocked.
    CPPUNIT_ASSERT_EQUAL(true, runner_13.start());

    // wait for switchAdapterRunnerServer to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "Attempting to connect to a SwitchAdapter" << &std::endl;
    // try connect to "the controller"
    Poco::Net::StreamSocket strSock(address);
    Poco::Net::SocketStream str(strSock);
    std::cout << "Connection to SwitchAdapter successful" << &std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Testing Handshake" << &std::endl;
    testHandshake(str, OF_VERSION_1_3);
    std::cout << "Handshake tested succesfully" << &std::endl;
    std::cout << "TestPacketInOut" << &std::endl;
    testPacketInOut(strSock, str);
    std::cout << "TestPacketInOut done" << &std::endl;

    std::cout << "TestSpecialCases" << &std::endl;
    testSpecialCases(strSock, str);
    std::cout << "TestSpecialCases done" << &std::endl;


    std::cout << "START: attempting to stop Runner." << &std::endl;
    runner_13.stop();
    std::cout << "DONE: Stopped Runner." << &std::endl;

}


void SwitchAdapterTests::testHandshake(Poco::Net::SocketStream& str, of_version_t OFVersion) {

    uint8_t* receiveBuffer = new uint8_t[65536];
    ///simulates switch:
    ///expecting Hello(version x) from SwitchAdapter(Runner)
    ///answering with Hello(version x)
    // read first packet from stream

    std::cout << "reading hello" << &std::endl;
    CPPUNIT_ASSERT(SwitchConnectionUtil::parseOpenFlowFromSocketToBuffer(receiveBuffer, str));
    CPPUNIT_ASSERT_EQUAL(OF_OBJ_TYPE_HELLO, (int) of_message_type_get(receiveBuffer));
    CPPUNIT_ASSERT_EQUAL(8, (int) of_message_length_get(receiveBuffer));

    std::cout << "reading hello done, writing hello" << &std::endl;
    // answer with hello
    CPPUNIT_ASSERT(SwitchConnectionUtil::writeOpenFlowToSocket(str, helloMessage));
    str.flush();

    std::cout << "writing hello done, reading featureRequest" << &std::endl;
    //expect featureRequest.
    CPPUNIT_ASSERT(SwitchConnectionUtil::parseOpenFlowFromSocketToBuffer(receiveBuffer, str));
    CPPUNIT_ASSERT_EQUAL(OF_OBJ_TYPE_FEATURES_REQUEST, (int) of_message_type_get(receiveBuffer));
    CPPUNIT_ASSERT_EQUAL(8, (int) of_message_length_get(receiveBuffer));

    std::cout << "reading featureRequest done, writing featureReply" << &std::endl;
    // answer with feature Reply.
    CPPUNIT_ASSERT(SwitchConnectionUtil::writeOpenFlowToSocket(str, featRepMessage));
    str.flush();

    std::cout << "writing featureReply done, writing echoReq" << &std::endl;
    // send echo request
    of_echo_request_t* echoReq = of_echo_request_new(OFVersion);
    CPPUNIT_ASSERT(SwitchConnectionUtil::writeOpenFlowToSocket(str, echoReq));
    of_echo_request_delete(echoReq);
    str.flush();
    std::cout << "writing echoReq done, reading echoReply" << &std::endl;


    //expect echo reply
    CPPUNIT_ASSERT(SwitchConnectionUtil::parseOpenFlowFromSocketToBuffer(receiveBuffer, str));
    CPPUNIT_ASSERT_EQUAL(OF_OBJ_TYPE_ECHO_REPLY, (int) of_message_type_get(receiveBuffer));
    CPPUNIT_ASSERT_EQUAL(8, (int) of_message_length_get(receiveBuffer));
    std::cout << "reading echoReply done, handshake successful" << &std::endl;

    delete[] receiveBuffer;

}

void SwitchAdapterTests::testPacketInOut(Poco::Net::StreamSocket& strSock, Poco::Net::SocketStream& stream) {
    // Create module
    std::shared_ptr<DummyModule> module =
            std::shared_ptr<DummyModule>(
                    new DummyModule(0,
                                    0,
                                    zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                    "DUMMY",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));


    // Create and start ZMF instance with module
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstance =
            zmf::instance::ZmfInstance::startInstance(module, {}, UT_CONFIG_FILE);


    while (!module->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "DummyModule enabled" << &std::endl;



    // PACKET IN ########################

    zmf::data::MessageType packetIn = zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().from().switch_adapter().openflow().packet_in().multicast_group_default().build();
    zmf::data::MessageType udp = zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().from().switch_adapter().openflow().packet_in().multicast_group_default().ipv4().udp().build();
    zmf::data::MessageType arp = zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().from().switch_adapter().openflow().packet_in().multicast_group_default().arp().build();


    std::atomic_int counter_packet_in(0);
    std::atomic_int counter_udp(0);
    std::atomic_int counter_arp(0);

    int noOfMessages = 10;

    module.get()->getZmfForUnittests()->subscribe(
            packetIn,
            [this, &counter_packet_in](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_packet_in++;
            }
    );

    module.get()->getZmfForUnittests()->subscribe(
            udp,
            [this, &counter_udp](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_udp++;
            }
    );

    module.get()->getZmfForUnittests()->subscribe(
            arp,
            [this, &counter_arp](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_arp++;
            }
    );


    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Subscribed to 3 topics" << &std::endl;


    for (int i = 0; i < noOfMessages; i++) {
        SwitchConnectionUtil::writeOpenFlowToSocket(stream, packetInMessage);
        stream.flush();
    }
    std::cout << "Wrote packetIn to socket" << &std::endl;


    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    CPPUNIT_ASSERT_EQUAL(noOfMessages, (int) counter_packet_in);
    CPPUNIT_ASSERT_EQUAL(noOfMessages, (int) counter_udp);
    CPPUNIT_ASSERT_EQUAL(0, (int) counter_arp);

    for (int i = 0; i < noOfMessages; i++) {
        SwitchConnectionUtil::writeOpenFlowToSocket(stream, packetInARPRequest);
        stream.flush();
    }
    std::cout << "Wrote arpRequest to socket" << &std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(250));


    CPPUNIT_ASSERT_EQUAL(noOfMessages * 2, (int) counter_packet_in);
    CPPUNIT_ASSERT_EQUAL(noOfMessages, (int) counter_udp);
    CPPUNIT_ASSERT_EQUAL(noOfMessages, (int) counter_arp);


    std::cout << "switch -> adapter -> module tested successfully" << &std::endl;


    // PACKET OUT ########################

    zmf::data::MessageType packetOut = zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().to().switch_adapter().switch_instance(
            1).openflow().packet_out().build();

    std::cout << "START: testing module -> adapter -> switch" << &std::endl;
    uint8_t readBuffer[sizeof(packetOutLinkDiscovery)];
    for (int i = 0; i < noOfMessages; i++) {
        zmf::data::ZmfMessage msg(packetOut, packetOutLinkDiscovery, sizeof(packetOutLinkDiscovery));
        std::cout << "publishing to " << packetOut.toString() << &std::endl;

        module->getZmfForUnittests()->publish(msg);
        SwitchConnectionUtil::parseOpenFlowFromSocketToBuffer(readBuffer, stream);
        for (int j = 0; j < sizeof(packetOutLinkDiscovery); j++) {
            CPPUNIT_ASSERT_EQUAL(packetOutLinkDiscovery[j], readBuffer[j]);

        }

    }
    std::cout << "DONE: testing module -> adapter -> switch" << &std::endl;

}


void SwitchAdapterTests::setUp() {
}

void SwitchAdapterTests::tearDown() {
}

void SwitchAdapterTests::testSpecialCases(Poco::Net::StreamSocket& strSock, Poco::Net::SocketStream& stream) {
    std::shared_ptr<DummyModule> module =
            std::shared_ptr<DummyModule>(
                    new DummyModule(0,
                                    0,
                                    zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                    "DUMMY",
                                    [](std::shared_ptr<zmf::data::ModuleHandle> changedModule,
                                       zmf::data::ModuleState lastState) { },
                                    [](const zmf::data::ZmfMessage& message,
                                       const zmf::data::ModuleUniqueId& sender) { return zmf::data::ZmfOutReply::createNoReply(); }));


    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create and start ZMF instance with module
    std::shared_ptr<zmf::IZmfInstanceController> zmfInstance =
            zmf::instance::ZmfInstance::startInstance(module, {},
                                                      UT_CONFIG_FILE);


    while (!module->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::atomic_int counter_Events(0);
    zmf::data::MessageType of = zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().from().switch_adapter().openflow().build();

    module.get()->getZmfForUnittests()->subscribe(
            of,
            [this, &counter_Events](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                counter_Events++;
            }
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // test all of types that should be ignored
    std::vector<of_object_t*> noForward;
    noForward.push_back(of_hello_new(OF_VERSION_1_3));
    noForward.push_back(of_echo_reply_new(OF_VERSION_1_3));
    noForward.push_back(of_echo_request_new(OF_VERSION_1_3));
    noForward.push_back(of_role_request_new(OF_VERSION_1_3));

    for (of_object_t* obj : noForward) {

        std::string ofObjSer = zsdn::of_object_serialize_to_data_string(obj);
        of_object_delete(obj);
        zmf::data::MessageType ofTopic = zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().to().switch_adapter().switch_instance(1).openflow().build();
        zmf::data::ZmfMessage msg(ofTopic, ofObjSer);
        module->getZmfForUnittests()->publish(msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        CPPUNIT_ASSERT_EQUAL(0, strSock.available());
    }

    // test frame too short to be openFlow
    std::string test = "ABC";
    zmf::data::MessageType ofTopic = zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().to().switch_adapter().switch_instance(1).openflow().build();
    zmf::data::ZmfMessage msg(ofTopic, test);
    module->getZmfForUnittests()->publish(msg);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CPPUNIT_ASSERT_EQUAL(0, strSock.available());

    // check that we have not received any events. now we send an echoRequest, which will both be published to zmf
    // AND be relpied with a echoreply to the switch
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    {
        CPPUNIT_ASSERT_EQUAL(0, strSock.available());
        int counter = counter_Events;
        CPPUNIT_ASSERT_EQUAL(0, counter);
    }
    of_object_t* echoReq = of_echo_request_new(OF_VERSION_1_3);
    SwitchConnectionUtil::writeOpenFlowToSocket(stream, echoReq);
    of_echo_request_delete(echoReq);
    stream.flush();


    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    CPPUNIT_ASSERT_EQUAL(8, strSock.available());
    int counter2 = counter_Events;
    CPPUNIT_ASSERT_EQUAL(1, counter2);



    // send special cases from switch to switchAdapter

    of_object_t* echoReply = of_echo_reply_new(OF_VERSION_1_3);
    SwitchConnectionUtil::writeOpenFlowToSocket(stream, echoReply);
    of_echo_reply_delete(echoReply);
    stream.flush();


    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    CPPUNIT_ASSERT_EQUAL(8, strSock.available());
    int counter3 = counter_Events;
    CPPUNIT_ASSERT_EQUAL(1, counter3);


    of_object_t* hello = of_hello_new(OF_VERSION_1_3);
    SwitchConnectionUtil::writeOpenFlowToSocket(stream, hello);
    of_hello_delete(hello);
    stream.flush();


    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    CPPUNIT_ASSERT_EQUAL(8, strSock.available());
    int counter4 = counter_Events;
    CPPUNIT_ASSERT_EQUAL(1, counter4);


    SwitchConnectionUtil::writeOpenFlowToSocket(stream, featRepMessage);
    stream.flush();


    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    CPPUNIT_ASSERT_EQUAL(8, strSock.available());
    int counter5 = counter_Events;
    std::cout << counter5 << &std::endl;
    CPPUNIT_ASSERT_EQUAL(2, counter5);


    std::cout << "done with all tests" << &std::endl;
}
