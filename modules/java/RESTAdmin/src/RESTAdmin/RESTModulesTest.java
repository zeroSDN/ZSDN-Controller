package RESTAdmin;
import com.jayway.restassured.RestAssured;
import com.jayway.restassured.parsing.Parser;

import org.hamcrest.Matchers;
import org.junit.Test;
import static com.jayway.restassured.RestAssured.expect;


/**
 * @author Maksim Pahlberg
 * JUnit Tests for the modules 
 * tests the REST interface of modules
 * Requires a locally running zeroZDN setup and a module with the id 0:1
 */
public class RESTModulesTest {

	/** 
	 * Junit test method for general module access
	 * Tests if /rest/modules returns status code OK (200)
	 * Tests if the returned JSON contains a field "name"
	 * 
	 */
	@Test 
	public void testModulesAccess() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/RESTAdmin/rest/modules").then().statusCode(200).assertThat().body("[0].name", Matchers.notNullValue());
	}
	
	/** 
	 * Junit test method for filtered module access
	 * Requests the module with the id 0:1
	 * Tests if /rest/modules/filtered?id=0:1 returns status code OK (200)
	 * Tests if the returned JSON contains a field "name"
	 * 
	 */
	@Test 
	public void testModulesFilteredAccess() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/RESTAdmin/rest/modules/filtered?id=0:1").then().statusCode(200).assertThat().body("[0].name", Matchers.notNullValue());		
	}
	
	

	
}
