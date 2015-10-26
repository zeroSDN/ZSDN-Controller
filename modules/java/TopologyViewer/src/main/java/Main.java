import java.io.IOException;

import jmf.Launcher;


public class Main {

	public static void main(String[] args) throws IOException {
		TopologyViewer topologyViewer = new TopologyViewer(0);
		Launcher.createInstance(topologyViewer, false, true, true, true);
	}
}
