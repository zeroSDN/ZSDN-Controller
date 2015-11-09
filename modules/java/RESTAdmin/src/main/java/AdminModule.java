package RESTAdmin;

import java.util.Collection;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.datastax.driver.core.ResultSet;
import com.datastax.driver.core.Row;
import com.google.common.primitives.UnsignedInteger;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.protobuf.InvalidProtocolBufferException;

import jmf.data.InReply;
import jmf.data.Message;
import jmf.data.MessageType;
import jmf.data.ModuleDependency;
import jmf.data.ModuleHandle;
import jmf.data.ModuleUniqueId;
import zsdn.protocol.TopologyModuleProtocol;
import zsdn.protocol.TopologyModuleProtocol.Request;
import zsdn.protocol.TopologyModuleProtocol.Request.GetTopologyRequest;
import zsdn.protocol.ZsdnCommonProtocol.Switch;
import zsdn.protocol.ZsdnCommonProtocol.SwitchToSwitchLink;
import zsdn.protocol.ZsdnCommonProtocol.Topology;
import zsdn.topics.TopologyModuleTopics;


/**
 * @author Maksim Pahlberg 
 * Admin Module Class
 *
 */
public class AdminModule extends jmf.module.AbstractModule {

	private static final Logger LOGGER = LoggerFactory.getLogger(AdminModule.class);

	private CassandraDB cassandraDB;
	private static final MessageType TOPOLOGY_REQUEST = TopologyModuleTopics.newCustomBuilderInstance(MessageType::new).request().topology_module().get_topology()
			.build();
	

	public AdminModule(ModuleUniqueId uniqueId, UnsignedInteger version, String name,
			Collection<ModuleDependency> dependencies) {
		super(uniqueId, version, name, dependencies);
		System.out.println("Admin Module started");	
	}

	
	/**
	 * Request Enable other module
	 * 
	 */
	public void enableModule(String id){
		getFramework().requestEnableRemoteInstance(new ModuleUniqueId(id), 1000);
	}
	/**
	 * Request Disable other module
	 * 
	 */
	public void disableModule(String id){
		getFramework().requestDisableRemoteInstance(new ModuleUniqueId(id), 1000);
	}
	
	/**
	 * Request Stop other module
	 * 
	 */
	public void stopModule(String id){
		getFramework().requestStopRemoteInstance(new ModuleUniqueId(id), 1000);
	}

	/**
	 * Get all modules
	 * @return all modules in JSON format
	 */
	public String getAllModules(){
		ConcurrentMap<ModuleUniqueId, ModuleHandle> modules = getFramework().getPeerRegistry().getAllPeersCopy(false);
		return buildModules(modules);
		//return null;
	}
	/**
	 * Get module by ID
	 * @return single module in JSON format
	 */
	public String getModuleByID(String id){
		ModuleUniqueId mID = new ModuleUniqueId(id);
		ModuleHandle mh = getFramework().getPeerRegistry().getPeerWithId(mID, false);
		return buildModule(mh);
		//return null;
	}
	
	/**
	 * Build JSON object from ConcurrentMap of modules
	 * 
	 * @param results
	 * @return Messages in JSON format
	 */
	String buildModules(ConcurrentMap<ModuleUniqueId, ModuleHandle> map) {
		Gson gson = new GsonBuilder().create();
		String str = "[";

		for (ConcurrentMap.Entry<ModuleUniqueId, ModuleHandle> entry : map.entrySet()) {

			//Access Module through Modulehandle
			Module module = new Module();
			ModuleHandle mHandle = entry.getValue();
			if(mHandle == null)
				return "Module handle is null!";
			//Fill Module object with values from Concurrent map
			module.setVersion(mHandle.getVersion());
			module.setName(mHandle.getName());
			module.setUniqueId(mHandle.getUniqueId().toString());
			module.setCurrentState(getFramework().getPeerRegistry().getPeerState(mHandle).toString());

			//Convert Module object to JSON string
			str += gson.toJson(module) + ",";
			

		}
		// Remove last comma hack
		if(str.charAt(str.length()-1) == ',')
			str = str.substring(0, str.length() - 1);

		return str + "]";

	}
	

	/**
	 * Build JSON object from ModuleHandle of a single Module
	 * 
	 * @param results
	 * @return Messages in JSON format
	 */
	String buildModule(ModuleHandle mHandle) {
		if(mHandle == null)
			return "Module handle is null!";
		
		Gson gson = new GsonBuilder().create();
		String str = "[";

			//Fill Module object with values from ModuleHandle
			Module module = new Module();
			module.setVersion(mHandle.getVersion());
			module.setName(mHandle.getName());
			module.setUniqueId(mHandle.getUniqueId().toString());
			module.setCurrentState(getFramework().getPeerRegistry().getPeerState(mHandle).toString());

			//Convert Module object to JSON string
			str += gson.toJson(module) + ",";
			

		return str + "]";

	}
	
	
	
	

	/**
	 * Get all logging messages from cassandraDB
	 * 
	 * @return All logging messages in JSON format
	 */
	public String getAllMessages() {
		String str = "";

		if (cassandraDB.connect()) {

			//Fill resultset with all messages
			ResultSet results = cassandraDB.execute("SELECT * FROM messages");
			//Make JSON string out of resultset
			str = buildMessage(results);
			cassandraDB.closeConnection();
		}

		return str;
	}

	/**
	 * Get logging messages by starttime and endtime from cassandradb
	 * 
	 * @param time
	 * @return logging messages filtered by starttime and endtime in JSON format
	 */
	public String getMessagesByTime(String startTime, String endTime) {
		String str = "";

		if (cassandraDB.connect()) {
			
			//Fill resultset with messages filtered by startTime and endTime
			ResultSet results = cassandraDB.execute("SELECT * FROM messages " + "WHERE timestamp > '" + startTime
					+ "' AND timestamp < '" + endTime + "' ALLOW FILTERING");
			//Make JSON String out of resultset
			str = buildMessage(results);
			cassandraDB.closeConnection();
		}

		return str;
	}

	/**
	 * Build JSON objects from ResultSet
	 * 
	 * @param results
	 * @return Messages in JSON format
	 */
	String buildMessage(ResultSet results) {
		Gson gson = new GsonBuilder().create();
		String str = "[";

		for (Row row : results) {

			RESTAdmin.Message msg = new RESTAdmin.Message();
			byte[] asByteArray = row.getBytes("messageType").array();
			int[] asIntArray = new int[asByteArray.length];
			for(int i = 0; i < asIntArray.length;i++) {
				asIntArray[i] = Byte.toUnsignedInt(asByteArray[i]);
			}
			String asString = TopicToString.toString(asIntArray);

			//Fill Message Object with values
			msg.setId(row.getUUID("id").toString());
			msg.setMessageType(asString);
			msg.setSenderId(row.getVarint("senderId"));
			msg.setSenderType(row.getVarint("senderType"));
			msg.setTimeStamp(row.getDate("timeStamp").getTime());
			
			//Convert Message object to JSON string
			str += gson.toJson(msg) + ",";
			

		}
		// Remove last comma hack
		if(str.charAt(str.length()-1) == ',')
			str = str.substring(0, str.length() - 1);

		return str + "]";

	}
	
	/**
	 * Get all logging messages from cassandraDB
	 * 
	 * @return All logging messages in JSON format
	 */
	public String getTopology() {
		
		return generateTopoJSON();
	}
	
	/**
	 * Build JSON string of all nodes and links
	 * 
	 * @return JSON string of the topology
	 */
	public String generateTopoJSON(){
		
		Topology topo = connectToTopo();
		
		//Check if topo is connected 
		if(topo == null)
			return "accessing topo module failed!";
		//Create new GSON builder
		Gson gson = new GsonBuilder().create();
		String str = "{ \"nodes\":[";
		//Go through all switches
		for (Switch sw : topo.getSwitchesList()) {
			
			//Fill Module object with values from Concurrent map
			Node node = new Node();
			
			node.setId(Long.toHexString(sw.getSwitchDpid()));
			node.setName("S"+Long.toHexString(sw.getSwitchDpid()));
			
			//Convert Node object to JSON string
			str += gson.toJson(node) + ",";
			
		}
		// Remove last comma hack
		if(str.charAt(str.length()-1) == ',')
			str = str.substring(0, str.length() - 1);
				
		str += "], \"links\":[";
		//Go through all switchlinks
		for (SwitchToSwitchLink swToSw : topo.getSwitchToSwitchLinksList()) {
			//Fill objects with values from List
			Link link = new Link();
			link.source = String.valueOf(swToSw.getSource().getSwitchDpid());
			link.target = String.valueOf(swToSw.getTarget().getSwitchDpid());
			link.value = 123;
			
			//Convert Link object to JSON string
			str += gson.toJson(link) + ",";
			
		}
		// Remove last comma hack
		if(str.charAt(str.length()-1) == ',')
			str = str.substring(0, str.length() - 1);
		str += "]";
		


		return str + "}";
		
	}
	
	/**
	 * Get the Topology Object
	 * 
	 * Access the TopologyModule through the jmf and and return the Topology Object 
	 * @return Topology object or null 
	 */
	private Topology connectToTopo(){
		
		//Get the handle of the TopologyModule
		ModuleHandle topoModule = getFramework().getPeerRegistry().getAnyPeerWithTypeVersion(
				(short)6,
				(short)0,
				true);
		//Check if handle is valid
		if (topoModule != null) {
			//Create a new request for the TopologyModuleProtocol
			Request request = TopologyModuleProtocol.Request.newBuilder()
					.setGetTopologyRequest(GetTopologyRequest.getDefaultInstance())
					.build();
			//get the Topology data
			InReply reply = getFramework().sendRequest(topoModule.getUniqueId(),
					new Message(TOPOLOGY_REQUEST, request.toByteArray()));
			Message message;
			try {
				message = reply.get(1, TimeUnit.SECONDS);
				//Covert the topology data into Topology object
				Topology topology = TopologyModuleProtocol.Reply.parseFrom(message.getData()).getGetTopologyReply()
						.getTopology();
				//Give back the Topology object
				return topology;
			} catch (InterruptedException | TimeoutException | ExecutionException
					| InvalidProtocolBufferException e) {
				e.printStackTrace();
			}
		}
		
		//if failed
		return null;
	}

	/**
	 * Connect to cassandra DB
	 * 
	 * @param cluster the IPs of the cluster
	 * @param keyspace the name of the keyspace
	 */
	private void connectToCassandra(String cluster, String keyspace) {
		cassandraDB = new CassandraDB(cluster, keyspace);
	}
	
	/** 
	 * Close the connection to cassandra
	 */
	private void disconnectCassandra() {
		cassandraDB.closeConnection();
	}

	/**
	 * This function will be called when this module should be enabled
	 * Creates a Connection to Cassandra DB
	 * @see {@link jmf.module.AbstractModule}
	 */
	@Override
	public boolean enable() {
		this.connectToCassandra("127.0.0.1", "logging");
		LOGGER.info("Admin Module enabled");

		return true;
	}

	/**
	 * This function will be called when this module should be disabled.
	 * The connection to Cassandra DB will be closed
	 * @see {@link jmf.module.AbstractModule}
	 */
	@Override
	public void disable() {
		//this.disconnectCassandra();
		LOGGER.info("Admin Module disabled");
	}

}