package zsdn.webadmin;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.core.Response;
 
/**
 * @author Maksim Pahlberg
 * REST Interface for logging messages
 * access at /rest/topo
 */
@Path("/topo")
public class RESTTopology {
 
	
	/**
	 * Is requested at /rest/topo and provides topology JSON
	 * @return	topology in JSON format
	 */
	@GET
	public Response getTopo() {
		
		String output = "all logging messages from cassandraDB";
		//Replace output string with logging messages in JSON format
		output = ZMFManager.adminModuleInstance.getTopology();
		return Response.status(200).entity(output).build();
		
	}
	
}