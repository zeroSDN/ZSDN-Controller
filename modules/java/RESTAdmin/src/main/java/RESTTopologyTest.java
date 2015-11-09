package RESTAdmin;

import static com.jayway.restassured.RestAssured.expect;

import org.hamcrest.Matchers;
import org.junit.Test;

import com.jayway.restassured.RestAssured;
import com.jayway.restassured.parsing.Parser;

/**
 * @author Maksim Pahlberg
 * JUnit Test Cases for the REST Topology 
 * tests the REST interface of the topology.
 * Requires a locally running zeroZDN setup with the TopologyModule
 */
public class RESTTopologyTest {
	
	
	/** 
	 * Junit test method for topology nodes
	 * Tests if /RESTAdmin/rest/topo returns status code OK (200)
	 * Tests if the returned JSON contains a field "nodes"
	 * 
	 */
	@Test 
	public void testTopologyNodes() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/RESTAdmin/rest/topo").then().statusCode(200).assertThat().body("nodes", Matchers.notNullValue());
	}
	
	/** 
	 * Junit test method for topology links
	 * Tests if the returned JSON from /RESTAdmin/rest/topo contains a field "links"
	 * 
	 */
	@Test 
	public void testTopologyLinks() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/RESTAdmin/rest/topo").then().assertThat().body("links", Matchers.notNullValue());
	}


}
