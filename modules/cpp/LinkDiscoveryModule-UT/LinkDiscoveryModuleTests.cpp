//
// Created by Matthias Strljic on 6/25/15.
//
#include <zsdn/proto/SwitchRegistryModule.pb.h>
#include <LociExtensions.h>
#include <RequestUtils.h>
#include <zmf//ZmfInstance.hpp>
#include <zsdn/proto/LinkDiscoveryModule.pb.h>
#include <LociExtensions.h>
#include "LinkDiscoveryModuleTests.h"
#include <UnittestConfigUtil.hpp>
#include <zmf/ZmfInstance.hpp>

// Test Params
const std::uint32_t switchCount = 44;
const std::uint32_t portsPerSwitch = 5;
const std::uint32_t registryId = switchCount + 1;
const std::uint32_t testModuleId = switchCount + 2;
const std::uint32_t linkModuleId = switchCount + 3;
//Data map
std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>> switchesObjectRef;
std::map<std::uint64_t, std::shared_ptr<zmf::IZmfInstanceController>> switchesObjectInstance;
std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> switchLinkMap;
std::shared_ptr<SwitchRegistryModuleMok> registryObjectRef;
std::shared_ptr<zmf::IZmfInstanceController> registryInstance;
std::shared_ptr<TesterObject> testerObjectRef;
std::shared_ptr<zmf::IZmfInstanceController> testerInstance;
std::shared_ptr<LinkDiscoveryModule> testLinkDiscoveryModuleObjectRef;
std::shared_ptr<zmf::IZmfInstanceController> testLinkDiscoveryModuleInstance;

LinkDiscoveryModuleTests::LinkDiscoveryModuleTests() {

}

bool LinkDiscoveryModuleTests::containsMapLink(
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>>& toValid, uint64_t source,
        uint32_t sPort, uint64_t endpoint, uint32_t ePort) {
    bool containsMapSource = false;
    bool containsSourcePort = false;
    bool isEndPointEqual = false;
    int size = toValid.size();
    containsMapSource = toValid.count(source) > 0;
    if (containsMapSource) {
        containsSourcePort = toValid.find(source)->second.count(sPort) > 0;
    }
    if (containsSourcePort) {
        bool portEqual = toValid.find(source)->second.find(sPort)->second.second == ePort;
        bool targetIdEqual = toValid.find(source)->second.find(sPort)->second.first == endpoint;
        isEndPointEqual = portEqual && targetIdEqual;
    }
    bool result = (containsMapSource && containsSourcePort && isEndPointEqual);
    return result;
}

bool LinkDiscoveryModuleTests::validateContainsAllFrom(
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> toValid, std::uint64_t source) {
    bool result = true;
    // check for all active links contained inside the map
    for (std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>>::iterator existingIterator = switchLinkMap.begin();
         existingIterator != switchLinkMap.end(); ++existingIterator) {
        for (std::map<uint32_t, std::pair<uint64_t, uint32_t>>::iterator existingLinkIterator = existingIterator->second.begin();
             existingLinkIterator != existingIterator->second.end(); ++existingLinkIterator) {
            if (existingIterator->first == source) {
                result = result && containsMapLink(toValid, existingIterator->first, existingLinkIterator->first,
                                                   existingLinkIterator->second.first,
                                                   existingLinkIterator->second.second);//((toValid.count(source) > 0) && toValid.find(source)->second.count(existingLinkIterator->first) > 0 && toValid.find(source)->second.find(existingLinkIterator->first)->second.first == existingLinkIterator->second.first&& toValid.find(source)->second.find(existingLinkIterator->first)->second.second == existingLinkIterator->second.second);
            }
        }
    }
    // check if the map contains more then all available links
    for (std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>>::iterator existingIterator = toValid.begin();
         existingIterator != toValid.end(); ++existingIterator) {
        for (std::map<uint32_t, std::pair<uint64_t, uint32_t>>::iterator existingLinkIterator = existingIterator->second.begin();
             existingLinkIterator != existingIterator->second.end(); ++existingLinkIterator) {
            if (existingIterator->first == source) {
                result = result && containsMapLink(switchLinkMap, existingIterator->first, existingLinkIterator->first,
                                                   existingLinkIterator->second.first,
                                                   existingLinkIterator->second.second);//((switchLinkMap.count(source) > 0) && switchLinkMap.find(source)->second.count(existingLinkIterator->first) > 0 && switchLinkMap.find(source)->second.find(existingLinkIterator->first)->second.first == existingLinkIterator->second.first&& switchLinkMap.find(source)->second.find(existingLinkIterator->first)->second.second == existingLinkIterator->second.second);
            }
        }
        result = result && switchLinkMap.count(existingIterator->first) > 0;
    }

    return result;

}

bool LinkDiscoveryModuleTests::validateContainsAllTo(
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> toValid, std::uint64_t endpoint) {
    bool result = true;
    // check for all active links contained inside the map
    for (std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>>::iterator existingIterator = switchLinkMap.begin();
         existingIterator != switchLinkMap.end(); ++existingIterator) {
        for (std::map<uint32_t, std::pair<uint64_t, uint32_t>>::iterator existingLinkIterator = existingIterator->second.begin();
             existingLinkIterator != existingIterator->second.end(); ++existingLinkIterator) {
            if (existingLinkIterator->second.first == endpoint) {
                result = result && containsMapLink(toValid, existingIterator->first, existingLinkIterator->first,
                                                   existingLinkIterator->second.first,
                                                   existingLinkIterator->second.second);//((toValid.count(endpoint) > 0) && toValid.find(endpoint)->second.count(existingLinkIterator->first) > 0 && toValid.find(endpoint)->second.find(existingLinkIterator->first)->second.first == existingLinkIterator->second.first&& toValid.find(endpoint)->second.find(existingLinkIterator->first)->second.second == existingLinkIterator->second.second);
            }
        }
    }
    // check if the map contains more then all available links
    for (std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>>::iterator existingIterator = toValid.begin();
         existingIterator != toValid.end(); ++existingIterator) {
        for (std::map<uint32_t, std::pair<uint64_t, uint32_t>>::iterator existingLinkIterator = existingIterator->second.begin();
             existingLinkIterator != existingIterator->second.end(); ++existingLinkIterator) {
            if (existingLinkIterator->second.first == endpoint) {
                result = result && containsMapLink(switchLinkMap, existingIterator->first, existingLinkIterator->first,
                                                   existingLinkIterator->second.first,
                                                   existingLinkIterator->second.second);// ((switchLinkMap.count(endpoint) > 0) && switchLinkMap.find(endpoint)->second.count(existingLinkIterator->first) > 0 && switchLinkMap.find(endpoint)->second.find(existingLinkIterator->first)->second.first == existingLinkIterator->second.first&& switchLinkMap.find(endpoint)->second.find(existingLinkIterator->first)->second.second == existingLinkIterator->second.second);
            }
        }
        result = result && switchLinkMap.count(existingIterator->first) > 0;
    }
    return result;
}

bool LinkDiscoveryModuleTests::validateContainsAllBetwenn(
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> toValid, std::uint64_t source,
        std::uint64_t endpoint) {
    bool result = true;
    // check for all active links contained inside the map
    for (std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>>::iterator existingIterator = switchLinkMap.begin();
         existingIterator != switchLinkMap.end(); ++existingIterator) {
        for (std::map<uint32_t, std::pair<uint64_t, uint32_t>>::iterator existingLinkIterator = existingIterator->second.begin();
             existingLinkIterator != existingIterator->second.end(); ++existingLinkIterator) {
            if (existingLinkIterator->second.first == endpoint && existingIterator->first == source) {
                result = result && containsMapLink(toValid, existingIterator->first, existingLinkIterator->first,
                                                   existingLinkIterator->second.first,
                                                   existingLinkIterator->second.second);//((toValid.count(endpoint) > 0) && toValid.find(endpoint)->second.count(existingLinkIterator->first) > 0 && toValid.find(endpoint)->second.find(existingLinkIterator->first)->second.first == existingLinkIterator->second.first&& toValid.find(endpoint)->second.find(existingLinkIterator->first)->second.second == existingLinkIterator->second.second);
            }
        }
    }
    // check if the map contains more then all available links
    for (std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>>::iterator existingIterator = toValid.begin();
         existingIterator != toValid.end(); ++existingIterator) {
        for (std::map<uint32_t, std::pair<uint64_t, uint32_t>>::iterator existingLinkIterator = existingIterator->second.begin();
             existingLinkIterator != existingIterator->second.end(); ++existingLinkIterator) {
            if (existingLinkIterator->second.first == endpoint && existingIterator->first == source) {
                result = result && containsMapLink(switchLinkMap, existingIterator->first, existingLinkIterator->first,
                                                   existingLinkIterator->second.first,
                                                   existingLinkIterator->second.second);// ((switchLinkMap.count(endpoint) > 0) && switchLinkMap.find(endpoint)->second.count(existingLinkIterator->first) > 0 && switchLinkMap.find(endpoint)->second.find(existingLinkIterator->first)->second.first == existingLinkIterator->second.first&& switchLinkMap.find(endpoint)->second.find(existingLinkIterator->first)->second.second == existingLinkIterator->second.second);
            }
        }
        result = result && switchLinkMap.count(existingIterator->first) > 0;
    }
    return result;
}


void LinkDiscoveryModuleTests::setUp() {
    // setup the links between the switches inside a map
    for (std::uint64_t count = 1; count <= switchCount; count++) {
        for (std::uint32_t portCount = 1; portCount <= portsPerSwitch; portCount++) {
            bool allTriedOrLinkFound = false;
            std::uint64_t actualSwitch = 1;
            while (!allTriedOrLinkFound) {
                for (std::uint32_t endPortCounter = 1; endPortCounter <= portsPerSwitch; endPortCounter++) {
                    if (!allTriedOrLinkFound && actualSwitch != count &&
                        switchLinkMap[actualSwitch].count(endPortCounter) == 0) {
                        allTriedOrLinkFound = true;
                        switchLinkMap[count][portCount] = {actualSwitch, endPortCounter};
                        switchLinkMap[actualSwitch][endPortCounter] = {count, portCount};
                    }
                }
                if (actualSwitch >= switchCount) {
                    allTriedOrLinkFound = true;
                }
                actualSwitch++;
            }
        }

    }
    try {
        // start the switch adapters
        for (unsigned int counter = 1; counter <= switchCount; counter++) {
            switchesObjectRef.insert(std::pair<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>(counter,
                                                                                                       std::shared_ptr<SwitchAdapterModuleMok>(
                                                                                                               new SwitchAdapterModuleMok(
                                                                                                                       counter))));
            switchesObjectInstance.insert(
                    std::pair<std::uint64_t, std::shared_ptr<zmf::IZmfInstanceController>>(counter,
                                                                                           zmf::instance::ZmfInstance::startInstance(
                                                                                                   switchesObjectRef.find(
                                                                                                           counter)->second,
                                                                                                   {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                                                                   UT_CONFIG_FILE)));
            // waite to enable
            while (!switchesObjectRef.find(counter)->second->isEnabled()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        // start the switch registry mok
        registryObjectRef = std::shared_ptr<SwitchRegistryModuleMok>(new SwitchRegistryModuleMok(registryId));
        registryInstance = zmf::instance::ZmfInstance::startInstance(registryObjectRef,
                                                                     {zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                                     UT_CONFIG_FILE);
        // waite for enable
        while (!registryObjectRef->isEnabled()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // create LinkDiscovery to start it later in the setup phase
        testLinkDiscoveryModuleObjectRef = std::shared_ptr<LinkDiscoveryModule>(new LinkDiscoveryModule(linkModuleId));
        testLinkDiscoveryModuleInstance = zmf::instance::ZmfInstance::startInstance(testLinkDiscoveryModuleObjectRef,
                                                                                    {zmf::instance::ZmfInstance::NO_AUTO_ENABLE},
                                                                                    UT_CONFIG_FILE);

        testerObjectRef = std::shared_ptr<TesterObject>(new TesterObject(testModuleId));
        testerInstance = zmf::instance::ZmfInstance::startInstance(testerObjectRef,
                                                                   {zmf::instance::ZmfInstance::NO_AUTO_ENABLE,
                                                                    zmf::instance::ZmfInstance::NO_PEER_DISCOVERY_WAIT},
                                                                   UT_CONFIG_FILE);

    } catch (...) {

    }

}

void LinkDiscoveryModuleTests::tearDown() {
    testerInstance->stopInstance();

    //testLinkDiscoveryModuleInstance->requestDisableModule();
    testLinkDiscoveryModuleInstance->stopInstance();

    registryInstance->stopInstance();

    for (std::map<std::uint64_t, std::shared_ptr<zmf::IZmfInstanceController>>::iterator it = switchesObjectInstance.begin();
         it != switchesObjectInstance.end(); it++) {
        it->second->stopInstance();
    }

    switchesObjectRef.clear();
    switchesObjectInstance.clear();
    switchLinkMap.clear();
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void LinkDiscoveryModuleTests::fullSetupRegistryStart() {
    registryObjectRef->startSetup(SwitchRegistryModuleMok::ReactMode::FULL_START);
    testLinkDiscoveryModuleInstance->requestEnableModule();
    while (!testLinkDiscoveryModuleObjectRef->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    testerInstance->requestEnableModule();
    while (!testerObjectRef->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void LinkDiscoveryModuleTests::partialSetupRegistryStart() {
    registryObjectRef->startSetup(SwitchRegistryModuleMok::ReactMode::PARTIAL_REACT_START);
    testLinkDiscoveryModuleInstance->requestEnableModule();
    while (!testLinkDiscoveryModuleObjectRef->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    testerInstance->requestEnableModule();
    while (!testerObjectRef->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Waiting for partial setup finishing by receiving events
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void LinkDiscoveryModuleTests::eventOnlySetupRegistryStart() {
    registryObjectRef->startSetup(SwitchRegistryModuleMok::ReactMode::CONTINUES_EVENT_START);
    testLinkDiscoveryModuleInstance->requestEnableModule();
    while (!testLinkDiscoveryModuleObjectRef->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    testerInstance->requestEnableModule();
    while (!testerObjectRef->isEnabled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Waiting for partial setup finishing by receiving events
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

//######################################################################################################################
void LinkDiscoveryModuleTests::testRequestNormalGetAllLinks() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestAllLinks();
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalGetAllLinksPartialEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    partialSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestAllLinks();
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalGetAllLinksFullEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    eventOnlySetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestAllLinks();
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalWithChangesGetAllLinks() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    registryObjectRef->startChanges();
    std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestAllLinks();
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestCorruptedProtoGetAllLinks() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LinkDiscoveryModule_Proto::Request request;
    std::string requestAsString = request.SerializeAsString();
    std::string modifiedOne = requestAsString.substr(0, requestAsString.length() - 6);
    bool result = testerObjectRef->requestCorruptedForTopic(
    zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_all_switch_links().build(), modifiedOne);
    CPPUNIT_ASSERT(result);
}

//######################################################################################################################
void LinkDiscoveryModuleTests::testRequestNormalGetLinksFromSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestFromLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
    }

}

void LinkDiscoveryModuleTests::testRequestNormalGetLinksFromSwitchPartialEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    partialSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestFromLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
    }

}

void LinkDiscoveryModuleTests::testRequestNormalGetLinksFromSwitchFullEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    eventOnlySetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestFromLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
    }

}

void LinkDiscoveryModuleTests::testRequestNormalWithChangesGetLinksFromSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    registryObjectRef->startChanges();
    std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestFromLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestCorrptedGetLinksFromSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LinkDiscoveryModule_Proto::Request request;
    CPPUNIT_ASSERT(testerObjectRef->requestCorruptedForTopic(
            zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_links_from_switch().build(),
            request.SerializeAsString()));
}

void LinkDiscoveryModuleTests::testRequestWrongIdGetLinkgsFromSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestFromLinks(
            switchCount + 1337);
    CPPUNIT_ASSERT(resultReq.empty());
}

//######################################################################################################################
void LinkDiscoveryModuleTests::testRequestNormalGetLinksToSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestToLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalGetLinksToSwitchPartialEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    partialSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestToLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalGetLinksToSwitchFullEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    eventOnlySetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestToLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalWithChangesGetLinksToSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    registryObjectRef->startChanges();
    std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestToLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestCorrptedGetLinksToSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LinkDiscoveryModule_Proto::Request request;
    CPPUNIT_ASSERT(testerObjectRef->requestCorruptedForTopic(
            zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_links_to_switch().build(),
            request.SerializeAsString()));
}

void LinkDiscoveryModuleTests::testRequestWrongIdGetLinksToSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestToLinks(
            switchCount + 1337);
    CPPUNIT_ASSERT(resultReq.empty());

}

//######################################################################################################################
void LinkDiscoveryModuleTests::testRequestNormalAllLinksOfSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestOfLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalAllLinksOfSwitchPartialEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    partialSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestOfLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalAllLinksOfSwitchFullEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    eventOnlySetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestOfLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestNormalWithChangesAllLinksOfSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    registryObjectRef->startChanges();
    std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestOfLinks(
                it->first);
        CPPUNIT_ASSERT(validateContainsAllFrom(resultReq, it->first));
        CPPUNIT_ASSERT(validateContainsAllTo(resultReq, it->first));
    }
}

void LinkDiscoveryModuleTests::testRequestCorrptedAllLinksOfSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LinkDiscoveryModule_Proto::Request request;
    CPPUNIT_ASSERT(testerObjectRef->requestCorruptedForTopic(
            zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_all_links_of_switch().build(),
            request.SerializeAsString()));
}

void LinkDiscoveryModuleTests::testRequestWrongIdAllLinksOfSwitch() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestOfLinks(
            switchCount + 1337);
    CPPUNIT_ASSERT(resultReq.empty());

}

//######################################################################################################################
// Tests corelating to the BetweenTwoSwitches request of the LinkDiscovrery
//######################################################################################################################
void LinkDiscoveryModuleTests::testRequestNormalLinksBetweenTwoSwitches() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it2 = switchesObjectRef.begin();
             it2 != switchesObjectRef.end(); ++it2) {
            std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestBetweenLinks(
                    it->first, it2->first);
            CPPUNIT_ASSERT(validateContainsAllBetwenn(resultReq, it->first, it2->first));
        }

    }
}

void LinkDiscoveryModuleTests::testRequestNormalLinksBetweenTwoSwitchesPartialEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    partialSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it2 = switchesObjectRef.begin();
             it2 != switchesObjectRef.end(); ++it2) {
            std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestBetweenLinks(
                    it->first, it2->first);
            CPPUNIT_ASSERT(validateContainsAllBetwenn(resultReq, it->first, it2->first));
        }

    }
}

void LinkDiscoveryModuleTests::testRequestNormalLinksBetweenTwoSwitchesFullEvent() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //fullSetupRegistryStart();
    eventOnlySetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it2 = switchesObjectRef.begin();
             it2 != switchesObjectRef.end(); ++it2) {
            std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestBetweenLinks(
                    it->first, it2->first);
            CPPUNIT_ASSERT(validateContainsAllBetwenn(resultReq, it->first, it2->first));
        }

    }
}

void LinkDiscoveryModuleTests::testRequestNormalWithChangesLinksBetweenTwoSwitches() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    registryObjectRef->startChanges();
    std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it = switchesObjectRef.begin();
         it != switchesObjectRef.end(); ++it) {
        for (std::map<std::uint64_t, std::shared_ptr<SwitchAdapterModuleMok>>::iterator it2 = switchesObjectRef.begin();
             it2 != switchesObjectRef.end(); ++it2) {
            std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestBetweenLinks(
                    it->first, it2->first);
            CPPUNIT_ASSERT(validateContainsAllBetwenn(resultReq, it->first, it2->first));
        }

    }
}

void LinkDiscoveryModuleTests::testRequestCorrptedLinksBetweenTwoSwitches() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LinkDiscoveryModule_Proto::Request request;
    CPPUNIT_ASSERT(testerObjectRef->requestCorruptedForTopic(
            zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_links_between_two_switches().build(),
            request.SerializeAsString()));
}

void LinkDiscoveryModuleTests::testRequestWrongIdLinksBetweenTwoSwitches() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    fullSetupRegistryStart();
    //sleep_for(std::chrono::milliseconds(3000));
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> resultReq = testerObjectRef->requestBetweenLinks(
            1, switchCount + 1337);
    CPPUNIT_ASSERT(resultReq.empty());

}


// Implementation of the facke RegistryModule
//######################################################################################################################
SwitchRegistryModuleMok::SwitchRegistryModuleMok(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
                       MODULE_VERSION, "SwitchRegistryMok", {}),
        instanceId_(instanceId) {
}

SwitchRegistryModuleMok::~SwitchRegistryModuleMok() {
}

zmf::data::ZmfOutReply SwitchRegistryModuleMok::handleRequest(const zmf::data::ZmfMessage& message,
                                                              const zmf::data::ModuleUniqueId& sender) {

    SwitchRegistryModule_Proto::Request request;
    bool parseSuccess = request.ParseFromArray(message.getDataRaw(), message.getDataLength());
    if (!parseSuccess) {
        assert(false);
        return zmf::data::ZmfOutReply::createNoReply();
    }

    switch (mode) {

        case UNSET:
            this->isSetup_ = true;
            break;
        case FULL_START:

            this->isSetup_ = true;
            return generateReplyMessageForSwitchesUpTo(switchCount);
            break;
        case PARTIAL_REACT_START:
            backgroundThread_ = new std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(11));
                this->startEventFinishFrom(switchCount / 2);
            });
            return generateReplyMessageForSwitchesUpTo(switchCount / 2);
            break;
        case CONTINUES_EVENT_START:
            backgroundThread_ = new std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(11));
                this->startEventFinishFrom(1);
            });

            return generateReplyMessageForSwitchesUpTo(1);
            break;
    }


}

zmf::data::ZmfOutReply SwitchRegistryModuleMok::generateReplyMessageForSwitchesUpTo(uint32_t c) {
    SwitchRegistryModule_Proto::Reply_GetAllSwitchesReply* getAllSwitchesReply = new SwitchRegistryModule_Proto::Reply_GetAllSwitchesReply();

    for (unsigned int counter = 1; counter <= c; counter++) {
        common::topology::Switch* switchElement = getAllSwitchesReply->add_switches();
        switchElement->set_openflow_version(1);
        switchElement->set_switch_dpid(counter);
        for (unsigned int portCounter = 1; portCounter <= portsPerSwitch; portCounter++) {
            common::topology::SwitchPort* portElement = switchElement->add_switch_ports();
            common::topology::AttachmentPoint* attachPoint = new common::topology::AttachmentPoint();
            attachPoint->set_switch_dpid(counter);
            attachPoint->set_switch_port(portCounter);
            portElement->set_allocated_attachment_point(attachPoint);
        }
    }

    SwitchRegistryModule_Proto::Reply reply;
    reply.set_allocated_get_all_switches_reply(getAllSwitchesReply);
    std::string asString = reply.SerializeAsString();
    return zmf::data::ZmfOutReply::createImmediateReply(
            zmf::data::ZmfMessage(topicsGetAllSwitchesReply_, asString));
}

void SwitchRegistryModuleMok::startEventFinishFrom(uint32_t s) {
    for (unsigned int counter = s + 1; counter <= switchCount; counter++) {
        //create common::topology::Switch
        common::topology::Switch* gelberElf = new common::topology::Switch();
        gelberElf->set_switch_dpid(counter);
        gelberElf->set_openflow_version(2);
        for (unsigned int portCounter = 1; portCounter <= portsPerSwitch; portCounter++) {
            common::topology::SwitchPort* portElement = gelberElf->add_switch_ports();
            common::topology::AttachmentPoint* attachPoint = new common::topology::AttachmentPoint();
            attachPoint->set_switch_dpid(counter);
            attachPoint->set_switch_port(portCounter);
            portElement->set_allocated_attachment_point(attachPoint);
        }
        SwitchRegistryModule_Proto::From messageContainer;
        SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();
        switchEvent->set_allocated_switch_added(gelberElf);
        messageContainer.set_allocated_switch_event(switchEvent);
        std::string letzteNachrichtVomElf = messageContainer.SerializeAsString();
        getZmf()->publish(
                zmf::data::ZmfMessage(topicsSwitchAdded_, letzteNachrichtVomElf));
    }
};

bool SwitchRegistryModuleMok::enable() {
    return true;
}

void SwitchRegistryModuleMok::startChanges() {
    changerThread_ = new std::thread([this]() {
        {

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            SwitchRegistryModule_Proto::From messageContainerRemove;
            SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();
            common::topology::Switch* gelberElf = new common::topology::Switch();
            gelberElf->set_switch_dpid(2);
            gelberElf->set_openflow_version(2);
            switchEvent->set_allocated_switch_removed(gelberElf);
            std::string removeMsgString;
            std::string emptyOne = "";
            getZmf()->publish(
                    zmf::data::ZmfMessage(topicsSwitchRemoved_, emptyOne));

            messageContainerRemove.set_allocated_switch_event(switchEvent);
            removeMsgString = messageContainerRemove.SerializeAsString();
            getZmf()->publish(
                    zmf::data::ZmfMessage(topicsSwitchRemoved_, removeMsgString));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        {
            common::topology::Switch* gelberElf = new common::topology::Switch();
            gelberElf->set_switch_dpid(2);
            gelberElf->set_openflow_version(2);
            for (unsigned int portCounter = 1; portCounter <= portsPerSwitch; portCounter++) {
                common::topology::SwitchPort* portElement = gelberElf->add_switch_ports();
                common::topology::AttachmentPoint* attachPoint = new common::topology::AttachmentPoint();
                attachPoint->set_switch_dpid(1);
                attachPoint->set_switch_port(portCounter);
                portElement->set_allocated_attachment_point(attachPoint);
            }
            SwitchRegistryModule_Proto::From messageContainer;
            SwitchRegistryModule_Proto::From_SwitchEvent* switchEvent = new SwitchRegistryModule_Proto::From_SwitchEvent();
            switchEvent->set_allocated_switch_added(gelberElf);
            messageContainer.set_allocated_switch_event(switchEvent);
            std::string letzteNachrichtVomElf = messageContainer.SerializeAsString();
            getZmf()->publish(
                    zmf::data::ZmfMessage(topicsSwitchAdded_, letzteNachrichtVomElf));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        switchesObjectRef[1]->setProcessMessages(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        switchesObjectRef[1]->setProcessMessages(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    });

};

bool SwitchRegistryModuleMok::isSetup() {
    return this->isSetup_;
}


void SwitchRegistryModuleMok::disable() {
    if (backgroundThread_ != nullptr) {
        if (backgroundThread_->joinable()) {
            try {
                backgroundThread_->join();
            } catch (...) { }
        }
        delete backgroundThread_;
    }
    if (changerThread_ != nullptr) {
        if (changerThread_->joinable()) {
            try {
                changerThread_->join();
            } catch (...) { }
        }
        delete changerThread_;
    }

}

void SwitchRegistryModuleMok::startSetup(ReactMode mode) {
    this->mode = mode;
}


// Implementation of the facke SwitchAdapter
//######################################################################################################################
SwitchAdapterModuleMok::SwitchAdapterModuleMok(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
                       MODULE_VERSION, "SwitchAdapterMok", {}),
        instanceId_(instanceId) {
}

SwitchAdapterModuleMok::~SwitchAdapterModuleMok() {
}

void SwitchAdapterModuleMok::setProcessMessages(bool pM) {
    processMessages = pM;
}

bool SwitchAdapterModuleMok::enable() {
    this->getZmf()->subscribe(
            zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().to().switch_adapter().switch_instance(this->instanceId_).openflow().build(),
            [this](const zmf::data::ZmfMessage& msg, const zmf::data::ModuleUniqueId& sender) {
                if (processMessages) {
                    uint64_t hash = 0;
                    uint64_t timestamp = 0;
                    uint32_t outPort = 0;
                    uint32_t newInPort = 0;
                    of_object_t* ofPacketout = zsdn::of_object_new_from_data_string_copy(msg.getData());
                    of_octets_t ofOutPayload;
                    of_packet_out_data_get(ofPacketout, &ofOutPayload);
                    of_action_output_t ofOutPort;
                    of_list_action_t* listAction = of_packet_out_actions_get(ofPacketout);
                    of_list_action_first(listAction, &ofOutPort);
                    of_action_output_port_get(&ofOutPort, &outPort);
                    //of_packet_out_in_port_get(ofPacketout ,&outPort);
                    uint8_t* outPayloadData = ofOutPayload.data;
                    uint16_t outPayloadLength = ofOutPayload.bytes;
                    Tins::EthernetII outEthPacket(outPayloadData, outPayloadLength);
                    LinkDiscoveryModule_Proto::LinkDiscoveryMessage linkMessage;
                    linkMessage.ParseFromArray(outEthPacket.inner_pdu()->serialize().data(),
                                               outEthPacket.inner_pdu()->serialize().size());
                    hash = linkMessage.uniqueid();
                    timestamp = linkMessage.sentimestamp();

                    of_list_action_delete(listAction);
                    of_object_delete(ofPacketout);
                    //std::map<std::uint64_t, std::map<uint32_t, std::map<uint64_t, uint32_t>>> switchLinkMap;
                    if (switchLinkMap.count(this->instanceId_) &&
                        switchLinkMap.find(this->instanceId_)->second.count(outPort) > 0) {
                        newInPort = switchLinkMap.find(this->instanceId_)->second.find(outPort)->second.second;
                        switchesObjectRef.find(switchLinkMap.find(this->instanceId_)->second.find(
                                outPort)->second.first)->second->sendMessageWithData(newInPort, hash, timestamp);
                    }
                }

                //sendMessageWithData(newInPort, hash,timestamp);

            });
    return true;
}

void SwitchAdapterModuleMok::sendMessageWithData(uint32_t port, uint64_t hash, uint64_t timestamp) {
    // send reply
    LinkDiscoveryModule_Proto::LinkDiscoveryMessage msgL;
    msgL.set_uniqueid(hash);
    msgL.set_sentimestamp(timestamp);

    Tins::RawPDU* pdu = new Tins::RawPDU(msgL.SerializeAsString());
    Tins::EthernetII eth;
    eth.inner_pdu(pdu);
    eth.payload_type(LinkDiscoveryModule::LINK_DISCOVERY_MESSAGE_PAYLOAD_TYPE);

    of_object_t* ofPacketIn = of_packet_in_new(of_version_e::OF_VERSION_1_3);
    of_octets_t ofPayload;
    //of_packet_in_data_get(ofPacketIn, ofPayload);

    Tins::PDU::serialization_type seri = eth.serialize();
    ofPayload.data = seri.data();
    ofPayload.bytes = seri.size();

    of_packet_in_buffer_id_set(ofPacketIn, OF_BUFFER_ID_NO_BUFFER);
    of_match_v3_t* ofM = of_match_v3_new(of_version_e::OF_VERSION_1_3);
    of_list_oxm_t* ofX = of_list_oxm_new(of_version_e::OF_VERSION_1_3);
    of_oxm_in_port_t* ofXE = of_oxm_in_port_new(of_version_e::OF_VERSION_1_3);
    of_oxm_in_port_value_set(ofXE, port);
    of_list_oxm_append(ofX, ofXE);

    of_match_v3_oxm_list_set(ofM, ofX);
    of_match_t theMatch;
    //theMatch.fields.in_port = portNo;
    of_match_v3_to_match(ofM, &theMatch);
    of_packet_in_match_set(ofPacketIn, &theMatch);
    //of_packet_in_in_phy_port_set(ofPacketIn, portNo);
    of_packet_in_data_set(ofPacketIn, &ofPayload);

    //of_packet_in_in_port_set(ofPacketIn, portNo);
    std::string messageStr = zsdn::of_object_serialize_to_data_string(ofPacketIn);
    of_packet_in_delete(ofPacketIn);
    of_list_oxm_delete(ofX);
    of_oxm_in_port_delete(ofXE);
    of_match_v3_delete(ofM);
    //zmf::data::MessageType packetInTopic = switchadapter_topics::FROM().switch_adapter().openflow().packet_in().build();

    zmf::data::MessageType packetInTopic = zsdn::modules::SwitchAdapterTopics<zmf::data::MessageType>().from().switch_adapter().openflow().packet_in().multicast_group_default().custom_protocol(
            0x511F).build();

    if (this->isEnabled()) {
        this->getZmf()->publish(zmf::data::ZmfMessage(packetInTopic, messageStr));
    }
}

void SwitchAdapterModuleMok::disable() {

}

// Implementation of the TesterObject
//######################################################################################################################
TesterObject::TesterObject(uint64_t instanceId) :
        AbstractModule(zmf::data::ModuleUniqueId(MODULE_TYPE_ID, instanceId),
                       MODULE_VERSION, "TestObjectMok", {{LINK_MODULE_DEP_TYPE, LINK_MODULE_DEP_VERSION}}) {
    this->instanceId_ = instanceId;
}

TesterObject::~TesterObject() {
}


bool TesterObject::enable() {

    return true;
}

void TesterObject::disable() {

}

std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> TesterObject::requestAllLinks() {
    // Result map to store the replies links
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> result;
    // Protobuffer request container
    LinkDiscoveryModule_Proto::Request request;
    // Create GetAllSwitchLink request
    LinkDiscoveryModule_Proto::Request_GetAllSwitchLinksRequest* getAllSwitchesRequest =
            new LinkDiscoveryModule_Proto::Request_GetAllSwitchLinksRequest();
    // insert the request into the container
    request.set_allocated_get_all_switch_links_request(getAllSwitchesRequest);
    // Create the reply object to store the result.
    LinkDiscoveryModule_Proto::Reply reply;
    // Start a GetAllSwitchLinks request with the utility funciton
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            request,
                                            reply,
    zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_all_switch_links().build(),
                                            zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                            testLinkDiscoveryModuleObjectRef->getVersion());
    // Process the reply
    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {
            // loop over the relied links
            for (int counter = 0; counter < reply.get_all_switch_links_reply().switch_links_size(); counter++) {
                // extract the id of the link source device
                uint64_t sourceID = reply.get_all_switch_links_reply().switch_links(counter).source().switch_dpid();
                // extract the port of the link at the source device
                uint32_t sourcePort = reply.get_all_switch_links_reply().switch_links(counter).source().switch_port();
                // extract the id of the link endpoint device
                uint64_t targetID = reply.get_all_switch_links_reply().switch_links(counter).target().switch_dpid();
                // extract the port of the link at the endpoint device
                uint32_t targetPort = reply.get_all_switch_links_reply().switch_links(counter).target().switch_port();
                // encapsulater tuple of a endpoint device < endpointId, endpointPort>
                std::pair<uint64_t, uint32_t> innerPair(targetID, targetPort);
                // create tuple of source link port and the endpoint device <sourcePort, < endpointId, endpointPort > >
                std::pair<uint32_t, std::pair<uint64_t, uint32_t>> middlePair(sourcePort, innerPair);
                // check if the source device already exists if == true -> insert else create holder map
                if (result.count(sourceID)) {
                    // check if the link is two times in the reply -> should be an error, else insert.
                    if (result.find(sourceID)->second.count(sourcePort)) {
                        // ERROR?
                    } else {
                        result.find(sourceID)->second.insert(middlePair);
                    }
                } else {
                    // create the holder map and insert the tuple
                    std::map<uint32_t, std::pair<uint64_t, uint32_t>> innerMap;
                    innerMap.insert(middlePair);
                    std::pair<uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> outerPair(sourceID,
                                                                                                     innerMap);
                    result.insert(outerPair);
                }

            }
        }
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND: {
        }
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::TIMEOUT: {
        }
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            CPPUNIT_ASSERT(false);
            break;
        default :
            CPPUNIT_ASSERT(false);
            break;

    }
    return result;
};

std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> TesterObject::requestOfLinks(uint64_t ofId) {
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> result;
    LinkDiscoveryModule_Proto::Request request;

    LinkDiscoveryModule_Proto::Request_GetAllLinksOfSwitchRequest* getAllSwitchesRequest =
            new LinkDiscoveryModule_Proto::Request_GetAllLinksOfSwitchRequest();
    getAllSwitchesRequest->set_switch_dpid(ofId);
    request.set_allocated_get_all_links_of_switch_request(getAllSwitchesRequest);
    LinkDiscoveryModule_Proto::Reply reply;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            request,
                                            reply,
    zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_all_links_of_switch().build(),
                                            zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                            testLinkDiscoveryModuleObjectRef->getVersion());

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {
            for (int counter = 0; counter < reply.get_all_links_of_switch_reply().links_to_switch_size(); counter++) {
                uint64_t sourceID = reply.get_all_links_of_switch_reply().links_to_switch(
                        counter).source().switch_dpid();
                uint32_t sourcePort = reply.get_all_links_of_switch_reply().links_to_switch(
                        counter).source().switch_port();
                uint64_t targetID = reply.get_all_links_of_switch_reply().links_to_switch(
                        counter).target().switch_dpid();
                uint32_t targetPort = reply.get_all_links_of_switch_reply().links_to_switch(
                        counter).target().switch_port();
                std::pair<uint64_t, uint32_t> innerPair(targetID, targetPort);
                std::pair<uint32_t, std::pair<uint64_t, uint32_t>> middlePair(sourcePort, innerPair);

                if (result.count(sourceID)) {
                    if (result.find(sourceID)->second.count(sourcePort)) {
                        // ERROR?
                    } else {
                        result.find(sourceID)->second.insert(middlePair);
                    }
                } else {
                    std::map<uint32_t, std::pair<uint64_t, uint32_t>> innerMap;
                    innerMap.insert(middlePair);
                    std::pair<uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> outerPair(sourceID,
                                                                                                     innerMap);
                    result.insert(outerPair);
                }

            }

            for (int counter = 0; counter < reply.get_all_links_of_switch_reply().links_from_switch_size(); counter++) {
                uint64_t sourceID = reply.get_all_links_of_switch_reply().links_from_switch(
                        counter).source().switch_dpid();
                uint32_t sourcePort = reply.get_all_links_of_switch_reply().links_from_switch(
                        counter).source().switch_port();
                uint64_t targetID = reply.get_all_links_of_switch_reply().links_from_switch(
                        counter).target().switch_dpid();
                uint32_t targetPort = reply.get_all_links_of_switch_reply().links_from_switch(
                        counter).target().switch_port();
                std::pair<uint64_t, uint32_t> innerPair(targetID, targetPort);
                std::pair<uint32_t, std::pair<uint64_t, uint32_t>> middlePair(sourcePort, innerPair);

                if (result.count(sourceID)) {
                    if (result.find(sourceID)->second.count(sourcePort)) {
                        // ERROR?
                    } else {
                        result.find(sourceID)->second.insert(middlePair);
                    }
                } else {
                    std::map<uint32_t, std::pair<uint64_t, uint32_t>> innerMap;
                    innerMap.insert(middlePair);
                    std::pair<uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> outerPair(sourceID,
                                                                                                     innerMap);
                    result.insert(outerPair);
                }

            }

        }
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND: {
        }
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::TIMEOUT: {
        }
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            CPPUNIT_ASSERT(false);
            break;
        default :
            CPPUNIT_ASSERT(false);
            break;

    }
    return result;
};

std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> TesterObject::requestFromLinks(
        uint64_t fromid) {
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> result;
    LinkDiscoveryModule_Proto::Request request;

    LinkDiscoveryModule_Proto::Request_GetLinksFromSwitchRequest* getAllSwitchesRequest =
            new LinkDiscoveryModule_Proto::Request_GetLinksFromSwitchRequest();
    getAllSwitchesRequest->set_switch_dpid(fromid);
    request.set_allocated_get_links_from_switch_request(getAllSwitchesRequest);
    LinkDiscoveryModule_Proto::Reply reply;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            request,
                                            reply,
                                            zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_links_from_switch().build(),
                                            zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                            testLinkDiscoveryModuleObjectRef->getVersion());

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {
            for (int counter = 0; counter < reply.get_links_from_switch_reply().links_from_switch_size(); counter++) {
                uint64_t sourceID = reply.get_links_from_switch_reply().links_from_switch(
                        counter).source().switch_dpid();
                uint32_t sourcePort = reply.get_links_from_switch_reply().links_from_switch(
                        counter).source().switch_port();
                uint64_t targetID = reply.get_links_from_switch_reply().links_from_switch(
                        counter).target().switch_dpid();
                uint32_t targetPort = reply.get_links_from_switch_reply().links_from_switch(
                        counter).target().switch_port();
                std::pair<uint64_t, uint32_t> innerPair(targetID, targetPort);
                std::pair<uint32_t, std::pair<uint64_t, uint32_t>> middlePair(sourcePort, innerPair);

                if (result.count(sourceID)) {
                    if (result.find(sourceID)->second.count(sourcePort)) {
                        // ERROR?
                    } else {
                        result.find(sourceID)->second.insert(middlePair);
                    }
                } else {
                    std::map<uint32_t, std::pair<uint64_t, uint32_t>> innerMap;
                    innerMap.insert(middlePair);
                    std::pair<uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> outerPair(sourceID,
                                                                                                     innerMap);
                    result.insert(outerPair);
                }

            }
        }
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND: {
        }
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::TIMEOUT: {
        }
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            CPPUNIT_ASSERT(false);
            break;
        default :
            CPPUNIT_ASSERT(false);
            break;

    }
    return result;
};

std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> TesterObject::requestToLinks(uint64_t toId) {
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> result;
    LinkDiscoveryModule_Proto::Request request;

    LinkDiscoveryModule_Proto::Request_GetLinksToSwitchRequest* getAllSwitchesRequest =
            new LinkDiscoveryModule_Proto::Request_GetLinksToSwitchRequest();
    getAllSwitchesRequest->set_switch_dpid(toId);
    request.set_allocated_get_links_to_switch_request(getAllSwitchesRequest);

    LinkDiscoveryModule_Proto::Reply reply;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            request,
                                            reply,
                                            zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_links_to_switch().build(),
                                            zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                            testLinkDiscoveryModuleObjectRef->getVersion());

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {
            for (int counter = 0; counter < reply.get_links_to_switch_reply().links_to_switch_size(); counter++) {
                uint64_t sourceID = reply.get_links_to_switch_reply().links_to_switch(counter).source().switch_dpid();
                uint32_t sourcePort = reply.get_links_to_switch_reply().links_to_switch(counter).source().switch_port();
                uint64_t targetID = reply.get_links_to_switch_reply().links_to_switch(counter).target().switch_dpid();
                uint32_t targetPort = reply.get_links_to_switch_reply().links_to_switch(counter).target().switch_port();
                std::pair<uint64_t, uint32_t> innerPair(targetID, targetPort);
                std::pair<uint32_t, std::pair<uint64_t, uint32_t>> middlePair(sourcePort, innerPair);

                if (result.count(sourceID)) {
                    if (result.find(sourceID)->second.count(sourcePort)) {
                        // ERROR?
                    } else {
                        result.find(sourceID)->second.insert(middlePair);
                    }
                } else {
                    std::map<uint32_t, std::pair<uint64_t, uint32_t>> innerMap;
                    innerMap.insert(middlePair);
                    std::pair<uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> outerPair(sourceID,
                                                                                                     innerMap);
                    result.insert(outerPair);
                }

            }
        }
            break;

        case zsdn::RequestUtils::NO_MODULE_INSTANCE_FOUND: {
        }
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::TIMEOUT: {
        }
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::REQUEST_SERIALIZATION_FAILED:
            CPPUNIT_ASSERT(false);
            break;
        case zsdn::RequestUtils::RESPONSE_PARSE_FAILED:
            CPPUNIT_ASSERT(false);
            break;
        default :
            CPPUNIT_ASSERT(false);
            break;

    }
    return result;
};

std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> TesterObject::requestBetweenLinks(
        uint64_t aId, uint64_t bId) {
    std::map<std::uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> result;
    LinkDiscoveryModule_Proto::Request request;

    LinkDiscoveryModule_Proto::Request_GetLinksBetweenTwoSwitchesRequest* getAllSwitchesRequest =
            new LinkDiscoveryModule_Proto::Request_GetLinksBetweenTwoSwitchesRequest();
    getAllSwitchesRequest->set_switch_a_dpid(aId);
    getAllSwitchesRequest->set_switch_b_dpid(bId);
    request.set_allocated_get_links_between_two_switches_request(getAllSwitchesRequest);
    LinkDiscoveryModule_Proto::Reply reply;
    zsdn::RequestUtils::RequestResult requestResult =
            zsdn::RequestUtils::sendRequest(*getZmf(),
                                            request,
                                            reply,
                                            zsdn::modules::LinkDiscoveryModuleTopics<zmf::data::MessageType>().request().link_discovery_module().get_links_between_two_switches().build(),
                                            zsdn::MODULE_TYPE_ID_LinkDiscoveryModule,
                                            testLinkDiscoveryModuleObjectRef->getVersion());

    switch (requestResult) {
        case zsdn::RequestUtils::SUCCESS: {
            for (int counter = 0; counter <
                                  reply.get_links_between_two_switches_reply().links_from_switch_a_to_switch_b_size(); counter++) {
                uint64_t sourceID = reply.get_links_between_two_switches_reply().links_from_switch_a_to_switch_b(
                        counter).source().switch_dpid();
                uint32_t sourcePort = reply.get_links_between_two_switches_reply().links_from_switch_a_to_switch_b(
                        counter).source().switch_port();
                uint64_t targetID = reply.get_links_between_two_switches_reply().links_from_switch_a_to_switch_b(
                        counter).target().switch_dpid();
                uint32_t targetPort = reply.get_links_between_two_switches_reply().links_from_switch_a_to_switch_b(
                        counter).target().switch_port();
                std::pair<uint64_t, uint32_t> innerPair(targetID, targetPort);
                std::pair<uint32_t, std::pair<uint64_t, uint32_t>> middlePair(sourcePort, innerPair);

                if (result.count(sourceID)) {
                    if (result.find(sourceID)->second.count(sourcePort)) {
                        // ERROR?
                    } else {
                        result.find(sourceID)->second.insert(middlePair);
                    }
                } else {
                    std::map<uint32_t, std::pair<uint64_t, uint32_t>> innerMap;
                    innerMap.insert(middlePair);
                    std::pair<uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> outerPair(sourceID,
                                                                                                     innerMap);
                    result.insert(outerPair);
                }

            }

            for (int counter = 0; counter <
                                  reply.get_links_between_two_switches_reply().links_from_switch_b_to_switch_a_size(); counter++) {
                uint64_t sourceID = reply.get_links_between_two_switches_reply().links_from_switch_b_to_switch_a(
                        counter).source().switch_dpid();
                uint32_t sourcePort = reply.get_links_between_two_switches_reply().links_from_switch_b_to_switch_a(
                        counter).source().switch_port();
                uint64_t targetID = reply.get_links_between_two_switches_reply().links_from_switch_b_to_switch_a(
                        counter).target().switch_dpid();
                uint32_t targetPort = reply.get_links_between_two_switches_reply().links_from_switch_b_to_switch_a(
                        counter).target().switch_port();
                std::pair<uint64_t, uint32_t> innerPair(targetID, targetPort);
                std::pair<uint32_t, std::pair<uint64_t, uint32_t>> middlePair(sourcePort, innerPair);

                if (result.count(sourceID)) {
                    if (result.find(sourceID)->second.count(sourcePort)) {
                        // ERROR?
                    } else {
                        result.find(sourceID)->second.insert(middlePair);
                    }
                } else {
                    std::map<uint32_t, std::pair<uint64_t, uint32_t>> innerMap;
                    innerMap.insert(middlePair);
                    std::pair<uint64_t, std::map<uint32_t, std::pair<uint64_t, uint32_t>>> outerPair(sourceID,
                                                                                                     innerMap);
                    result.insert(outerPair);
                }

            }
        }
            break;


        default :
            CPPUNIT_ASSERT(false);
            break;

    }
    return result;
};

bool TesterObject::requestCorruptedForTopic(zmf::data::MessageType type, std::string message) {
    bool result = false;
    try {
        zmf::data::ZmfInReply reply = this->getZmf()->sendRequest(
                zmf::data::ModuleUniqueId(zsdn::MODULE_TYPE_ID_LinkDiscoveryModule, 0),
                zmf::data::ZmfMessage(type, message));
        std::future_status status = reply.wait_for(std::chrono::milliseconds(2000));

        switch (status) {


            case std::future_status::ready:
                result = true;
                break;
            default :
                result = false;
                break;

        }
    } catch (...) {
        result = false;
    }

    return result;

}

