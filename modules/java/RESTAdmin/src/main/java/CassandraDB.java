package RESTAdmin;

import com.datastax.driver.core.Cluster;
import com.datastax.driver.core.ResultSet;
import com.datastax.driver.core.Session;
import com.datastax.driver.core.Statement;

/**
 * @author Maksim Pahlberg 
 * CassandraDB access class
 *
 */
public class CassandraDB {

	Cluster cluster;
	Session session;

	String contactPoint;
	String keyspace;

	/**
	 * Create an instace of CassandraDB with the contactpoint (ip) and keyspace
	 * 
	 * @param contactPoint
	 * @param keyspace
	 */
	public CassandraDB(String contactPoint, String keyspace) {
		this.contactPoint = contactPoint;
		this.keyspace = keyspace;

	}

	/**
	 * Open CassandraDB connection
	 * 
	 * @return boolean successful connection
	 */
	public boolean connect() {
		// Connect to the cluster
		cluster = Cluster.builder().addContactPoint(contactPoint).build();
		// connect to logging keyspace
		session = cluster.connect(keyspace);
		// if connection fails
		if (session.isClosed()) {
			System.out.println("CassandraDB Connection failed:");
			return false;
		}
		return true;
	}


	/**
	 * Run a query on the DB
	 * 
	 * @param query
	 * @return ResultSet of the run query
	 */
	public ResultSet execute(String query) {
		return session.execute(query);
	}
	/**
	 * Run a query on the DB with a prepared statement
	 * 
	 * @param query
	 * @return ResultSet of the run query
	 */
	public ResultSet execute(Statement query) {
		return session.execute(query);
	}


	/** 
	 * Close CassandraDB connection
	 */
	public void closeConnection() {
		cluster.close();
	}
}
