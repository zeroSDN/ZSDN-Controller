package zsdn.webadmin;

/**
 * @author Maksim Pahlberg
 * Container for module data
 * used with GSON
 * 
 */
public class Module {
	private String name;
	private short version;
	private String UniqueId;
	public String getUniqueId() {
		return UniqueId;
	}

	public void setUniqueId(String uniqueId) {
		UniqueId = uniqueId;
	}

	public String getCurrentState() {
		return currentState;
	}

	public void setCurrentState(String currentState) {
		this.currentState = currentState;
	}

	private String currentState;

	public short getVersion() {
		return version;
	}

	public void setVersion(short version) {
		this.version = version;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}
}
