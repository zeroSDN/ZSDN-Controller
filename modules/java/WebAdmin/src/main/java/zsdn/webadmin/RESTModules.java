package zsdn.webadmin;

import javax.ws.rs.GET;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Response;
 
/**
 * @author Maksim Pahlberg
 * REST interface for module management
 * access at Rest /rest/modules
 */

@Path("/modules")
public class RESTModules{
 
	
	/** 
	 * Return all modules
	 * @return all modules 
	 */
	@GET
	public Response getModules() {
		
		String output = "all modules with status";
		
		output = ZMFManager.adminModuleInstance.getAllModules();
		return Response.status(200).entity(output).build();
 
	}
	
	/**
	 * Return one or more modules by either id or type or status
	 * @param id
	 * @param type
	 * @param status
	 * @return module
	 */
	@GET
	@Path("/filtered")
	public Response getModuleByFilter(@QueryParam("id") String id, @QueryParam("type") String type,  @QueryParam("status") String status) {
		
		String output = "empty response";
		
		//REQUEST FOR ID
		if (id != null){
			output = ZMFManager.adminModuleInstance.getModuleByID(id);
		}
		//REQUEST FOR TYPE
		else if (type != null){
			//output = ZMFManager.adminModuleInstance.getAllModulesByType(type);
		}
		//REQUEST FOR STATUS
		else if (status != null){
			//output = ZMFManager.adminModuleInstance.getAllModulesByStatus(status);
		}
		
		
		return Response.status(200).entity(output).build();
 
	}
	/** Perform  action "enable", "disable" or "stop" on a module with id
	 * @param id
	 * @param action
	 * @return action successful 
	 */
	@PUT
	@Path("/control")
	public Response performModuleAction (@QueryParam("id") String id, @QueryParam("action") String action) {
		
		String output = "perform module action";
		if (id != null && action != null){
			
			output = "Performing "+action+" on module "+id;
			
			if (action.equals("enable")){
				ZMFManager.adminModuleInstance.enableModule(id);
			}
			else if (action.equals("disable")){
				ZMFManager.adminModuleInstance.disableModule(id);
			}
			else if (action.equals("stop")){
				ZMFManager.adminModuleInstance.stopModule(id);
			}
			else{
				output = "Valid actions are: enable,disable,stop";
			}
			
		}
		
		return Response.status(200).entity(output).build();
 
	}

}