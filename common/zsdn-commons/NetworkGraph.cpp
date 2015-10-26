#include "NetworkGraph.h"
#include "ZsdnTypes.h"
#include <lemon/dijkstra.h>
#include <loci/loci_base.h>
#include <Poco/Exception.h>

namespace zsdn {

    NetworkGraph::NetworkGraph(
            const common::topology::Topology& topology)
            :
            attachmentPoints(this->graph_),
            edgeWeights(this->graph_) {

        validate(topology);

        // create an empty map of Nodes (representing ports) for each switch
        for (int i = 0; i < topology.switches_size(); i++) {
            nodes[topology.switches(i).switch_dpid()] = std::map<uint32_t, lemon::ListDigraph::Node>();
        }


        // add all AttachmentPoints of the topology as SimpleSwitchPorts to the attachmentPoint map,
        // and add a Node for each of them in the graph.
        for (const common::topology::Switch& sw : topology.switches()) {
            for (const common::topology::SwitchPort& port : sw.switch_ports()) {

                zsdn::AttachmentPoint attachmentPoint = {sw.switch_dpid(), port.attachment_point().switch_port()};
                const lemon::ListDigraph::Node& node = graph_.addNode();
                // add to Map of all nodes
                nodes[attachmentPoint.switchDpid][attachmentPoint.switchPort] = node;
                // add to map of all attachmentPoints
                attachmentPoints[node] = attachmentPoint;
            }
        }

        // add bidirectional links between attachmentPoints of the same switch.
        for (const common::topology::Switch& sw : topology.switches()) {
            for (int allAttachmentPoints = 0; allAttachmentPoints < sw.switch_ports_size(); allAttachmentPoints++) {
                for (int onSameSwitch = allAttachmentPoints + 1;
                     onSameSwitch < sw.switch_ports_size(); onSameSwitch++) {

                    const lemon::ListDigraph::Node& from = nodes[sw.switch_ports(
                            allAttachmentPoints).attachment_point().switch_dpid()][sw.switch_ports(
                            allAttachmentPoints).attachment_point().switch_port()];

                    const lemon::ListDigraph::Node& to = nodes[sw.switch_ports(
                            onSameSwitch).attachment_point().switch_dpid()][sw.switch_ports(
                            onSameSwitch).attachment_point().switch_port()];

                    const lemon::ListDigraph::Arc& toArc = graph_.addArc(from, to);
                    const lemon::ListDigraph::Arc& fromArc = graph_.addArc(to, from);
                    edgeWeights[toArc] = EDGEWEIGHT_SWITCH_INTERNAL;
                    edgeWeights[fromArc] = EDGEWEIGHT_SWITCH_INTERNAL;

                }
            }
        }

        // add directed links between nodes according to switchToSwitchLinks in the topology.
        for (const common::topology::SwitchToSwitchLink& link : topology.switch_to_switch_links()) {

            const lemon::ListDigraph::Node& from = nodes[link.source().switch_dpid()][link.source().switch_port()];
            const lemon::ListDigraph::Node& to = nodes[link.target().switch_dpid()][link.target().switch_port()];
            const lemon::ListDigraph::Arc& arc = graph_.addArc(from, to);
            edgeWeights[arc] = EDGEWEIGHT_BETWEEN_SWITCHES;
        }
    }

    std::vector<zsdn::AttachmentPoint> NetworkGraph::getShortestPath(
            const common::topology::AttachmentPoint& from,
            const common::topology::AttachmentPoint& to) {

        return getShortestPath(from.switch_dpid(), from.switch_port(), to.switch_dpid(), to.switch_port());
    }

    std::vector<zsdn::AttachmentPoint> NetworkGraph::getShortestPath(
            const zsdn::AttachmentPoint& from,
            const zsdn::AttachmentPoint& to) {

        return getShortestPath(from.switchDpid,
                               from.switchPort,
                               to.switchDpid,
                               to.switchPort);
    }

    std::vector<zsdn::AttachmentPoint> NetworkGraph::getShortestPath(
            const zsdn::Device& from,
            const zsdn::Device& to) {

        return getShortestPath(from.attachmentPoint.switchDpid,
                               from.attachmentPoint.switchPort,
                               to.attachmentPoint.switchDpid,
                               to.attachmentPoint.switchPort);
    }

    std::vector<zsdn::AttachmentPoint> NetworkGraph::getShortestPath(
            const uint64_t fromSwitchDpid,
            const uint32_t fromSwitchPort,
            const uint64_t toSwitchDpid,
            const uint32_t toSwitchPort) {

        // path to self -> unnecessary
        if (fromSwitchDpid == toSwitchDpid && fromSwitchPort == toSwitchPort) { return {}; }

        // Check if source switch is contained in map of all switches
        SwitchPortNodeMap::iterator fromSwitchIter = nodes.find(fromSwitchDpid);
        if (fromSwitchIter == nodes.end()) { return {}; }

        // Check if port is contained in source switch
        PortNodeMap::iterator fromSwitchPortIter = fromSwitchIter->second.find(fromSwitchPort);
        if (fromSwitchPortIter == fromSwitchIter->second.end()) { return {}; }

        // Check if target switch is contained in map of all switches
        SwitchPortNodeMap::iterator toSwitchIter = nodes.find(toSwitchDpid);
        if (toSwitchIter == nodes.end()) { return {}; }

        // Check if port is contained in target switch
        PortNodeMap::iterator toSwitchPortIter = toSwitchIter->second.find(toSwitchPort);
        if (toSwitchPortIter == toSwitchIter->second.end()) { return {}; }

        const lemon::ListDigraph::Node fromNode = fromSwitchPortIter->second;
        const lemon::ListDigraph::Node toNode = toSwitchPortIter->second;

        // execute dijkstra
        lemon::Dijkstra<lemon::ListDigraph> dijkstra_runner(this->graph_, this->edgeWeights);

        bool reached = dijkstra_runner.run(fromNode, toNode);
        if (!reached) { return {}; }


        // convert from path to vector
        std::vector<zsdn::AttachmentPoint> path;
        for (lemon::ListDigraph::Node current = toNode;
             current != lemon::INVALID; current = dijkstra_runner.predNode(current)) {
            path.insert(path.begin(), attachmentPoints[current]);
        }
        return path;
    }

    // Graph:
    //
    //      2-------7
    //      |       |
    //  1---4---5---6       10
    //      |   |
    //      3   8===9
    //
    std::vector<uint64_t> switches = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<uint32_t> switchPorts = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::vector<std::vector<zsdn::AttachmentPoint>> att =
            {
                    {{1, 2}, {4, 1}},
                    {{2, 2}, {4, 2}},
                    {{3, 2}, {4, 3}},
                    {{4, 4}, {5, 1}},
                    {{5, 2}, {6, 1}},
                    {{6, 2}, {7, 1}},
                    {{7, 2}, {2, 1}},
                    {{5, 3}, {8, 1}},
                    {{8, 2}, {9, 1}},
                    {{8, 3}, {9, 2}}
            };

    common::topology::Topology* NetworkGraph::createTestTopology() {




        common::topology::Topology* topo = new common::topology::Topology;

        std::map<uint64_t, std::map<uint32_t, common::topology::AttachmentPoint>> attachmentPoints;


            for (uint64_t sw : switches) {
                common::topology::Switch* s = topo->add_switches();
                s->set_switch_dpid(sw);
                s->set_openflow_version(OF_VERSION_1_3);
                for (uint32_t swPort : switchPorts) {
                    common::topology::SwitchPort* port = s->add_switch_ports();
                    common::topology::AttachmentPoint* const attachmentPoint = new common::topology::AttachmentPoint();
                    attachmentPoint->set_switch_dpid(sw);
                    attachmentPoint->set_switch_port(swPort);
                    port->set_allocated_attachment_point(attachmentPoint);
                    attachmentPoints[sw][swPort] = common::topology::AttachmentPoint(*attachmentPoint);
                }
            }
            for (std::vector<zsdn::AttachmentPoint> vec : att) {
                zsdn::AttachmentPoint a = vec[0];
                zsdn::AttachmentPoint b = vec[1];

                common::topology::SwitchToSwitchLink* stsl = topo->add_switch_to_switch_links();
                common::topology::AttachmentPoint* aAtt = new common::topology::AttachmentPoint();
                aAtt->set_switch_dpid(a.switchDpid);
                aAtt->set_switch_port(a.switchPort);
                common::topology::AttachmentPoint* bAtt = new common::topology::AttachmentPoint();
                bAtt->set_switch_dpid(b.switchDpid);
                bAtt->set_switch_port(b.switchPort);
                stsl->set_allocated_source(aAtt);
                stsl->set_allocated_target(bAtt);

                common::topology::SwitchToSwitchLink* stslr = topo->add_switch_to_switch_links();
                common::topology::AttachmentPoint* aAttr = new common::topology::AttachmentPoint(*aAtt);
                common::topology::AttachmentPoint* bAttr = new common::topology::AttachmentPoint(*bAtt);
                stslr->set_allocated_source(aAttr);
                stslr->set_allocated_target(bAttr);
            }

        return topo;

        }

    std::vector<common::topology::Device> NetworkGraph::createDevicesConnectedToTestTopology() {
        std::vector<common::topology::Device> devices;
        zsdn::MAC mac = 1;
        for(uint64_t switchId: switches) {
            for (uint32_t switchport : switchPorts) {
                zsdn::AttachmentPoint asAttachmentPoint = {switchId,switchport};
                bool portCanBeUsed = true;
                // make sure its not a port connected to a Switch
                for (const std::vector<zsdn::AttachmentPoint>& pair : att) {
                    if (pair[0] == asAttachmentPoint || pair[1] == asAttachmentPoint) {
                        portCanBeUsed = false;
                        break;
                    }
                }
                if (portCanBeUsed) {
                    common::topology::Device device;
                    common::topology::AttachmentPoint* attachmentPoint = new common::topology::AttachmentPoint;
                    attachmentPoint->set_switch_dpid(switchId);
                    attachmentPoint->set_switch_port(switchport);
                    device.set_mac_address(mac++);
                    device.set_millis_since_last_seen(1);
                    device.set_allocated_attachment_point(attachmentPoint);

                    devices.push_back(device);
                }

            }
        }

        return devices;
    }

    void NetworkGraph::validate(const common::topology::Topology& topology) {
        std::map<zsdn::DPID,std::set<zsdn::Port>> switchesAndPorts;

        for(const common::topology::Switch& sw : topology.switches()) {
            for(const common::topology::SwitchPort& swPort : sw.switch_ports()) {
                switchesAndPorts[sw.switch_dpid()].insert(swPort.attachment_point().switch_port());
            }
        }

        for(const common::topology::SwitchToSwitchLink swToSw : topology.switch_to_switch_links()) {
            bool sourceContained = switchesAndPorts.count(swToSw.source().switch_dpid()) > 0
                                   &&  switchesAndPorts[swToSw.source().switch_dpid()].count(swToSw.source().switch_port()) > 0;

            bool targetContained = switchesAndPorts.count(swToSw.target().switch_dpid()) > 0
                                   &&  switchesAndPorts[swToSw.target().switch_dpid()].count(swToSw.target().switch_port()) > 0;

            if (!(sourceContained && targetContained)) {
                throw Poco::InvalidArgumentException("Topology is missing switch information for link");
            }
        }
    }
}
