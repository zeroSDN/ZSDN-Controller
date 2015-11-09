package RESTAdmin;
import com.jayway.restassured.RestAssured;
import com.jayway.restassured.parsing.Parser;

import org.hamcrest.Matchers;
import org.junit.Test;
import static com.jayway.restassured.RestAssured.expect;


/**
 * @author Maksim Pahlberg
 * JUnit Tests for RESTAdmin module control
 * tests the REST interface of the modules
 * Requires a locally running zeroZDN setup and a module with the id 0:1
 * Will actually attempt to enable then disable the stop the module
 * Only recommended for testing if you are ok with the test disabling a module
 */
public class RESTModulesControlTest {
	

	/** 
	 * Junit test method for module enabling
	 * enables the module with the id 0:1
	 * Tests if /RESTAdmin/rest/modules/control?id=0:1&action=enable returns status code OK (200)
	 * Uses put so there is no further return
	 * 
	 */
	@Test 
	public void testModuleEnable() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().put("/RESTAdmin/rest/modules/control?id=0:1&action=enable").then().statusCode(200);
	}
	
	/** 
	 * Junit test method for module disabling
	 * disables the module with the id 0:1
	 * Tests if /RESTAdmin/rest/modules/control?id=0:1&action=disable returns status code OK (200)
	 * Uses put so there is no further return
	 * 
	 */
	@Test 
	public void testModuleDisable() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().put("/RESTAdmin/rest/modules/control?id=0:1&action=disable").then().statusCode(200);
	}
	
	/** 
	 * Junit test method for module stopping
	 * stopps the module with the id 0:1
	 * Tests if /RESTAdmin/rest/modules/control?id=0:1&action=stop returns status code OK (200)
	 * Uses put so there is no further return
	 * 
	 */
	@Test 
	public void testModuleStop() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().put("/RESTAdmin/rest/modules/control?id=0:1&action=stop").then().statusCode(200);
	}

}
