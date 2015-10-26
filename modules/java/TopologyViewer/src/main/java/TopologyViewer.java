import java.io.File;
import java.io.FileNotFoundException;
import java.util.Collections;
import java.util.List;
import java.util.Scanner;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.graphstream.graph.Edge;
import org.graphstream.graph.Graph;
import org.graphstream.graph.Node;
import org.graphstream.graph.implementations.SingleGraph;
import org.graphstream.ui.view.Viewer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedLong;
import com.google.protobuf.InvalidProtocolBufferException;

import jmf.data.InReply;
import jmf.data.Message;
import jmf.data.MessageType;
import jmf.data.ModuleHandle;
import jmf.data.ModuleUniqueId;
import jmf.module.AbstractModule;
import zsdn.protocol.DeviceModuleProtocol.From.DeviceEvent;
import zsdn.protocol.DeviceModuleProtocol.Request.GetAllDevicesRequest;
import zsdn.protocol.DeviceModuleProtocol;
import zsdn.protocol.TopologyModuleProtocol;
import zsdn.protocol.TopologyModuleProtocol.From.TopologyChangedEvent;
import zsdn.protocol.TopologyModuleProtocol.Request;
import zsdn.protocol.TopologyModuleProtocol.Request.GetTopologyRequest;
import zsdn.protocol.ZsdnCommonProtocol.AttachmentPoint;
import zsdn.protocol.ZsdnCommonProtocol.Device;
import zsdn.protocol.ZsdnCommonProtocol.Switch;
import zsdn.protocol.ZsdnCommonProtocol.SwitchToSwitchLink;
import zsdn.protocol.ZsdnCommonProtocol.Topology;
import zsdn.topics.DeviceModuleTopics;
import zsdn.topics.TopologyModuleTopics;

public class TopologyViewer extends AbstractModule {
	
	private static final String STYLE;
	static {
		String style = null;
		try {
			style = new Scanner(new File("style.css")).useDelimiter("\\Z").next();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		STYLE = style;
	}
	
	private static final Logger LOG = LoggerFactory.getLogger(TopologyViewer.class);
	private static final short MODULE_TYPE_ID = 0x00ff;
	private static final UnsignedInteger VERSION = UnsignedInteger.valueOf(0);
	private static final String NAME = "TopologyViewer";
	private static final MessageType TOPOLOGY_TOPIC = new TopologyModuleTopics.FROM().topology_module().topology_changed_event()
			.build();
	private static final MessageType TOPOLOGY_REQUEST = new TopologyModuleTopics.REQUEST().topology_module().get_topology()
			.build();
	private static final MessageType DEVICE_TOPIC = new DeviceModuleTopics.FROM().device_module().device_event().added().build();
	private static final MessageType DEVICES_REQUEST = new DeviceModuleTopics.REQUEST().device_module().get_all_devices().build();

	private final Graph graph = new SingleGraph("Network");
	private Viewer display;

	public TopologyViewer(long instanceId) {
		super(new ModuleUniqueId(UnsignedInteger.valueOf(MODULE_TYPE_ID), UnsignedLong.valueOf(instanceId)),
				VERSION,
				NAME,
				Collections.emptyList());
		resetGraph();
	}

	@Override
	public boolean enable() {
		getFramework().subscribe(TOPOLOGY_TOPIC, (msg, sender) -> {
			handleTopoEvent(msg);
		});
		getFramework().subscribe(DEVICE_TOPIC, (msg, sender) -> {
			handleDeviceEvent(msg);
		});

		initializeGraph();
		display = graph.display();

		return true;
	}

	private void initializeGraph() {

		new Thread(
				() -> {
					try {
						Thread.sleep(1000l);
						ModuleHandle topoModule = getFramework().getPeerRegistry().getAnyPeerWithTypeVersion(
								UnsignedInteger.valueOf(6),
								UnsignedInteger.valueOf(0),
								true);
						if (topoModule != null) {
							Request request = TopologyModuleProtocol.Request.newBuilder()
									.setGetTopologyRequest(GetTopologyRequest.getDefaultInstance())
									.build();
							InReply reply = getFramework().sendRequest(topoModule.getUniqueId(),
									new Message(TOPOLOGY_REQUEST, request.toByteArray()));
							Message message;
							try {
								message = reply.get(1, TimeUnit.SECONDS);

								Topology topology = TopologyModuleProtocol.Reply.parseFrom(message.getData()).getGetTopologyReply()
										.getTopology();
								updateTopo(topology);
							} catch (InterruptedException | TimeoutException | ExecutionException
									| InvalidProtocolBufferException e) {
								e.printStackTrace();
							}
						}

						ModuleHandle deviceModule = getFramework().getPeerRegistry().getAnyPeerWithTypeVersion(
								UnsignedInteger.valueOf(2),
								UnsignedInteger.valueOf(0),
								true);
						if (deviceModule != null) {
							DeviceModuleProtocol.Request request = DeviceModuleProtocol.Request.newBuilder()
									.setGetAllDevicesRequest(GetAllDevicesRequest.getDefaultInstance()).build();
							InReply reply = getFramework().sendRequest(deviceModule.getUniqueId(),
									new Message(DEVICES_REQUEST, request.toByteArray()));
							Message message;
							try {
								message = reply.get(1, TimeUnit.SECONDS);

								List<Device> devices = DeviceModuleProtocol.Reply.parseFrom(message.getData()).getGetAllDevicesReply()
										.getDevicesList();
								devices.forEach(d -> updateDevice(d));
							} catch (InterruptedException | TimeoutException | ExecutionException
									| InvalidProtocolBufferException e) {
								e.printStackTrace();
							}

						}

					} catch (Exception e) {
						e.printStackTrace();
					}
				}).start();

	}

	@Override
	public void disable() {
		graph.clear();
	
	}

	private void handleTopoEvent(Message topoMsg) {
		try {
			TopologyChangedEvent topo = TopologyModuleProtocol.From.parseFrom(topoMsg.getData()).getTopologyChangedEvent();
			LOG.info("Received new Topology, Updating Graph.");
			updateTopo(topo.getTopology());
		} catch (InvalidProtocolBufferException e) {
			LOG.error("Failed to parse topology", e);
		}
	}

	private void handleDeviceEvent(Message msg) {
		try {
			DeviceEvent device = DeviceModuleProtocol.From.parseFrom(msg.getData()).getDeviceEvent();
			LOG.info("Received new Device, Updating Graph.");
			updateDevice(device.getDeviceAdded());

		} catch (InvalidProtocolBufferException e) {
			LOG.error("Failed to parse DeviceEvent", e);

		}
	}
	
	private void resetGraph() {
		graph.clear();
		graph.setAutoCreate(true);
		graph.setAttribute("ui.stylesheet", STYLE);
		graph.addAttribute("ui.quality");
		graph.addAttribute("ui.antialias");
	}

	private void updateTopo(Topology topo) {
		resetGraph();
		
		for (Switch sw : topo.getSwitchesList()) {
			Node addNode = graph.addNode(Long.toHexString(sw.getSwitchDpid()));
			addNode.addAttribute("ui.style", "size: 30px,20px;");
			addNode.addAttribute("ui.style", "text-size: 15px;");
			addNode.addAttribute("ui.style", "text-style: bold;");
			addNode.addAttribute("ui.label", "S"+Long.toHexString(sw.getSwitchDpid())+"");
		}

		for (SwitchToSwitchLink swToSw : topo.getSwitchToSwitchLinksList()) {
			AttachmentPoint source = swToSw.getSource();
			AttachmentPoint target = swToSw.getTarget();

			String linkName = swToString(source) + " <-> " + swToString(target);
			String inverseName = swToString(target) + " <-> " + swToString(source);
			if (graph.getEdge(inverseName) != null || graph.getEdge(linkName) != null) {
				continue;
			}

			graph.addEdge(
					linkName,
					Long.toHexString(swToSw.getSource().getSwitchDpid()),
					Long.toHexString(swToSw.getTarget().getSwitchDpid()));
		}
	}

	private void updateDevice(Device deviceAdded) {

		if(graph.getNode(swToString(deviceAdded.getAttachmentPoint())) != null) {
			return;
		}
		String nodeName = "H[" + Long.toHexString(deviceAdded.getMACAddress())+"]";

		String linkName = nodeName + " <-> " + swToString(deviceAdded.getAttachmentPoint());
		Node addNode = graph.addNode(nodeName);
		addNode.addAttribute("ui.style", "size: 15px,15px;");
		addNode.addAttribute("ui.style", "shape: rounded-box;");
		addNode.addAttribute("ui.style", "fill-color: rgb(0,100,255);");
		addNode.addAttribute("ui.style", "text-size: 15px;");
		addNode.addAttribute("ui.style", "text-style: bold;");
		addNode.addAttribute("ui.label", nodeName);


		Edge addEdge = graph.addEdge(linkName, nodeName, Long.toHexString(deviceAdded.getAttachmentPoint().getSwitchDpid()));
		display.enableAutoLayout();
	}

	private String swToString(AttachmentPoint attPoint) {
		return Long.toHexString(attPoint.getSwitchDpid()) + ":" + attPoint.getSwitchPort();
	}
}
