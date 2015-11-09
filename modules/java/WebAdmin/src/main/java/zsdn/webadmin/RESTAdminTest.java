package zsdn.webadmin;
import static com.jayway.restassured.RestAssured.expect;

import org.hamcrest.Matchers;
import org.junit.Test;

import com.jayway.restassured.RestAssured;
import com.jayway.restassured.parsing.Parser;

/**
 * @author Maksim Pahlberg
 * JUnit Test Cases for the RESTAdmin 
 * tests the REST interface of modules,messages and topology.
 * Requires a locally running zeroZDN setup and a module with 
 * the id 0:1 (for the module control testing)
 */
public class RESTAdminTest {

	@Test 
	public void testModulesAccess() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		
		expect().get("/rest/modules").then().statusCode(200).assertThat().body("[0].name", Matchers.notNullValue());
		expect().get("/rest/modules/filtered?id=0:1").then().statusCode(200).assertThat().body("[0].name", Matchers.notNullValue());
		
	}
	@Test 
	public void testModulesControl() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().put("/rest/modules/control?id=0:1&action=enable").then().statusCode(200);
		expect().put("/rest/modules/control?id=0:1&action=disable").then().statusCode(200);
		expect().put("/rest/modules/control?id=0:1&action=stop").then().statusCode(200);
	}
	@Test 
	public void testMessages() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/rest/messages").then().statusCode(200).assertThat().body("[0].id", Matchers.notNullValue());
		
		long unixTime = System.currentTimeMillis();
		expect().get("/rest/messages/filtered?starttime=128946816000&endtime="+unixTime).then().statusCode(200).assertThat().body("[0].id", Matchers.notNullValue());		
		
	}
	@Test 
	public void testTopology() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/rest/topo").then().statusCode(200).assertThat().body("nodes", Matchers.notNullValue());
		expect().get("/rest/topo").then().statusCode(200).assertThat().body("links", Matchers.notNullValue());
		
	}

}
