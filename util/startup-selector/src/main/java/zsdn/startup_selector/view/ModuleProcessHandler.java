package zsdn.startup_selector.view;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * Handler to start Module Processes.
 * @author Jan Strauss
 */
public class ModuleProcessHandler implements Runnable {
	private boolean running = false;
	private Process process;
	private final String module;
	private final Thread thread;
	private final String[] launch;
	private final  ModuleOverviewController controller;

	public ModuleProcessHandler(ModuleOverviewController controller, String module, String exec, String... params) {
		thread = new Thread(this);
 		this.module = module;
		thread.setName("ProcessHandler Thread for " + module);
		this.controller = controller;
		this.launch = new String[params.length + 1];
		this.launch[0] = exec;
		System.arraycopy(params, 0, launch, 1, params.length);
	}

	public void start() throws IOException {
		process = new ProcessBuilder(launch).directory(new File("/home/zsdn/zsdn-git/build/modules/default/")).redirectErrorStream(true).start();

		thread.start();

		running = true;
	}

	@Override
	public void run() {
		try {
			final BufferedReader br = new BufferedReader(new InputStreamReader(process.getInputStream()));

			String line;
			while ((line = br.readLine()) != null) {
				System.out.println("MODULE OUTPUT " + module + ": " + line);
				controller.updateTextArea(line);
			}

			process.waitFor();
			br.close();

			System.out.println("=== PROCESS ENDED ===");
		} catch (final InterruptedException | IOException e) {
			System.out.println("Exception handling module process" + e.getMessage());
		}

	}

	public void destroy() throws InterruptedException, IOException {
		if (running) {
			process.getOutputStream().write("stop\n".getBytes());
			process.getOutputStream().flush();
			running = false;
		}
	}

	public boolean isAlive() {
		return running;
	
	}
}