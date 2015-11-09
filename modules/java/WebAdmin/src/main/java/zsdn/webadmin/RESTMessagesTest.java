package zsdn.webadmin;

import static com.jayway.restassured.RestAssured.expect;

import org.hamcrest.Matchers;
import org.junit.Test;

import com.jayway.restassured.RestAssured;
import com.jayway.restassured.parsing.Parser;

/**
 * @author Maksim Pahlberg
 * JUnit tests for the REST Messages 
 * tests the REST interface of the messages.
 * Requires a locally running CassandraDB with messages
 */

public class RESTMessagesTest {
	

	/** 
	 * Junit test method for general messages access
	 * Tests if /rest/messages returns status code OK (200)
	 * Tests if the returned JSON contains a field "id"
	 * 
	 */
	@Test 
	public void testMessages() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		expect().get("/rest/messages").then().statusCode(200).assertThat().body("[0].id", Matchers.notNullValue());
	}
	
	/** 
	 * Junit test method for filtered messages access
	 * Request messages between starttime 128946816000 and now (current unix time)
	 * Tests if it returns status code OK (200)
	 * Tests if the returned JSON contains a field "id"
	 * 
	 */
	@Test 
	public void testMessagesFiltered() { 
		RestAssured.registerParser("text/plain", Parser.JSON);
		long unixTime = System.currentTimeMillis();
		expect().get("/rest/messages/filtered?starttime=128946816000&endtime="+unixTime).then().statusCode(200).assertThat().body("[0].id", Matchers.notNullValue());		
		
	}

}
