package RESTAdmin;
import com.jayway.restassured.RestAssured;
import com.jayway.restassured.RestAssured.*;
import com.jayway.restassured.http.ContentType;
import com.jayway.restassured.matcher.RestAssuredMatchers.*;
import com.jayway.restassured.parsing.Parser;

import org.hamcrest.Matchers;
import org.hamcrest.Matchers.*;
import org.hamcrest.core.Is;
import org.json.JSONException;
import org.junit.Test;
import org.junit.Before;
import static com.jayway.restassured.RestAssured.expect;
import static org.hamcrest.Matchers.equalTo;

import java.io.InputStream;

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
		
		expect().get("/RESTAdmin/rest/modules").then().statusCode(200).assertThat().body("[0].name", Matchers.notNullValue());
		expect().get("/RESTAdmin/rest/modules/filtered?id=0:1").then().statusCode(200).assertThat().body("[0].name", Matchers.notNullValue());
		
	}
	@Test 
	public void testModulesControl() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().put("/RESTAdmin/rest/modules/control?id=0:1&action=enable").then().statusCode(200);
		expect().put("/RESTAdmin/rest/modules/control?id=0:1&action=disable").then().statusCode(200);
		expect().put("/RESTAdmin/rest/modules/control?id=0:1&action=stop").then().statusCode(200);
	}
	@Test 
	public void testMessages() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/RESTAdmin/rest/messages").then().statusCode(200).assertThat().body("[0].id", Matchers.notNullValue());
		
		long unixTime = System.currentTimeMillis();
		expect().get("/RESTAdmin/rest/messages/filtered?starttime=128946816000&endtime="+unixTime).then().statusCode(200).assertThat().body("[0].id", Matchers.notNullValue());		
		
	}
	@Test 
	public void testTopology() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/RESTAdmin/rest/topo").then().statusCode(200).assertThat().body("nodes", Matchers.notNullValue());
		expect().get("/RESTAdmin/rest/topo").then().statusCode(200).assertThat().body("links", Matchers.notNullValue());
		
	}

}
