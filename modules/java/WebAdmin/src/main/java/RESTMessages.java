package RESTAdmin;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Response;
 
/**
 * @author Maksim Pahlberg
 * REST Interface for logging messages
 * access at /RESTAdmin/rest/messages
 */
@Path("/messages")
public class RESTMessages {
 
	
	/**
	 * Is requested at /RESTAdmin/rest/messages and provides all logging messages
	 * @return	All logging messages in JSON format
	 */
	@GET
	public Response getMessages() {
		
		String output = "all logging messages from cassandraDB";
		//Replace output string with logging messages in JSON format
		output = ZMFManager.adminModuleInstance.getAllMessages();
		return Response.status(200).entity(output).build();
		
 
	}
	
	/**
	 * Is requested at /RESTAdmin/rest/messages/filtered? and requires 
	 * the  starttime and endtime parameter
	 * @param time
	 * @param starttime
	 * @param endtime
	 * @return Logging messages filtered by  starttime and endtime
	 */
	@GET
	@Path("/filtered")
	public Response getMessagesByTime(@QueryParam("time") String time, @QueryParam("starttime") String starttime, @QueryParam("endtime") String endtime) {
		
		String output = "logging messages by time:"+time;
		
		if(starttime != null && endtime != null){
			output = ZMFManager.adminModuleInstance.getMessagesByTime(starttime,endtime);
		}
		
		return Response.status(200).entity(output).build();
		
 
	}
}