package zsdn.webadmin;


/**
 * @author Maksim Pahlberg
 * Container for topo link data
 * used with GSON
 *
 */
public class Link {

	String source;
	String target;
	int value = 123;
	public String getSource() {
		return source;
	}
	public void setSource(String ource) {
		this.source = ource;
	}
	public String getTarget() {
		return target;
	}
	public void setTarget(String target) {
		this.target = target;
	}
	public int getValue() {
		return value;
	}
	public void setValue(int value) {
		this.value = value;
	}
}
