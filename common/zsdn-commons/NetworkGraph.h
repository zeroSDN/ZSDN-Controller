#ifndef ZSDN_COMMONS_NETWORKGRAPH_H
#define ZSDN_COMMONS_NETWORKGRAPH_H


#include "zsdn/proto/CommonTopology.pb.h"
#include "ZsdnTypes.h"
#include <lemon/list_graph.h>


namespace zsdn {

    /**
     * @brief   Graph representation of the network.
     *
     * @details This class creates a graph data structure based on a topology Message.
     *          It can be used to find shortest paths between AttachmentPoints(SwitchPorts).
     *
     * @author  Andre Kutzleb
     *
     * @date    15.07.2015
     *
     * @remark  TODO this graph structure is very inefficient.(each SwitchPort is a node)
     *          if performance is too weak, consider optimizing it (Andre)
     */
    class NetworkGraph {

        typedef std::map<uint32_t, lemon::ListDigraph::Node> PortNodeMap;
        typedef std::map<uint64_t, PortNodeMap> SwitchPortNodeMap;

    public:

        /**
         *  Constructs a NetworkGraph from the given topology. On the resulting NetworkGraph instance shortest path
         *  algorithms can be executed.
         *
         * @param topology
         *        the topology from which the NetworkGraph will be built.
         */
        NetworkGraph(const common::topology::Topology& topology);

        /// Empty Destructor
        ~NetworkGraph() { }

        /**
         *  Finds the shortest directed Path from one AttachmentPoint to another in the topology.
         *
         * @param from
         *        source AttachmentPoint(switchId+port).
         * @param to
         *        target AttachmentPoint(switchId+port).
         *
         * @return  shortest path between "from" and "to" as vector, including source and target themselves.
         * If no path can be found, the returned vector will be empty. If source == target, the vector will also
         * be empty.
         */
        std::vector<zsdn::AttachmentPoint> getShortestPath(
                const common::topology::AttachmentPoint& from,
                const common::topology::AttachmentPoint& to);

        /**
         *  Finds the shortest directed Path from one AttachmentPoint to another in the topology.
         *
         * @param from
         *        source AttachmentPoint(switchId+port).
         * @param to
         *        target AttachmentPoint(switchId+port).
         *
         * @return  shortest path between "from" and "to" as vector, including source and target themselves.
         * If no path can be found, the returned vector will be empty. If source == target, the vector will also
         * be empty.
         */
        std::vector<zsdn::AttachmentPoint> getShortestPath(
                const zsdn::AttachmentPoint& from,
                const zsdn::AttachmentPoint& to);

        /**
         *  Finds the shortest directed Path from one AttachmentPoint of a Device to another in the topology.
         *
         * @param from
         *        source Device with AttachmentPoint(switchId+port).
         * @param to
         *        target Device with AttachmentPoint(switchId+port).
         *
         * @return  shortest path between "from" and "to" as vector, including source and target themselves.
         * If no path can be found, the returned vector will be empty. If source == target, the vector will also
         * be empty.
         */
        std::vector<zsdn::AttachmentPoint> getShortestPath(
                const zsdn::Device& from,
                const zsdn::Device& to);

        /**
         *  Finds the shortest directed Path from one AttachmentPoint to another in the topology.
         *
         * @param fromSwitchDpid
         *        switchId of source AttachmentPoint.
         * @param fromSwitchPort
         *        port of source AttachmentPoint.
         * @param toSwitchDpid
         *        switchId of target AttachmentPoint.
         * @param toSwitchPort
         *        port of target AttachmentPoint.
         *
         * @return  shortest path between "from" and "to" as vector, including source and target themselves.
         * If no path can be found, the returned vector will be empty. If source == target, the vector will also
         * be empty.
         */
        std::vector<zsdn::AttachmentPoint> getShortestPath(
                const uint64_t fromSwitchDpid,
                const uint32_t fromSwitchPort,
                const uint64_t toSwitchDpid,
                const uint32_t toSwitchPort);


        static const std::string pathToString(const std::vector<zsdn::AttachmentPoint>& path) {

            std::string arrow = "";

            std::stringstream builder;
            for(const zsdn::AttachmentPoint& hop : path) {
                builder << arrow << hop.toString();
                arrow = " -> ";
            }

            return builder.str();
        }

        static common::topology::Topology* createTestTopology();

        static std::vector<common::topology::Device> createDevicesConnectedToTestTopology();


    private:

        /// Used as weight between AttachmentPoints on the same switch
        const int EDGEWEIGHT_SWITCH_INTERNAL = 1;
        /// Used as weight between AttachmentPoints between switches.
        const int EDGEWEIGHT_BETWEEN_SWITCHES = 10;

        // Graph data structure representing the topology
        lemon::ListDigraph graph_;

        /// Maps a switchId to a Map of all ports of that switch.
        SwitchPortNodeMap nodes;

        /// Holds a SimpleSwitchPort for each node in the graph. this is the data of each node.
        lemon::ListDigraph::NodeMap <zsdn::AttachmentPoint> attachmentPoints;

        /// Edgeweights for each Arc (directed edge) in the graph
        lemon::ListDigraph::ArcMap <int> edgeWeights;

        void validate(const common::topology::Topology& topology);
    };
}

#endif //ZSDN_COMMONS_NETWORKGRAPH_H
