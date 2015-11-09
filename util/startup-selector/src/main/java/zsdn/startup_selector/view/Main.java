package zsdn.startup_selector.view;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.TextArea;
import javafx.scene.layout.BorderPane;
import javafx.stage.Stage;

/**
 * Created by zsdn on 10/27/15.
 */
public class Main extends Application {

	private static TextArea ta;
	private static ModuleProcessHandler handler;

	static private class ModuleProcessHandler implements Runnable {
		private boolean running = false;
		private Process process;
		private final String module;
		private final Thread thread;
		private final String[] launch;

		private ModuleProcessHandler(String module, String exec, String... params) {
			thread = new Thread(this);
			this.module = module;
			thread.setName("ProcessHandler Thread for " + module);

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
					updateTxt(line);
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
	}

	public static void main(String[] args) throws Exception {
		//handler = new ModuleProcessHandler("SwitchRegistry", "/home/zsdn/zsdn-git/build/modules/default/SwitchRegistryModule", "/home/zsdn/zsdn-git/examples/example-configs/zmf_module.config");
		//handler = new ModuleProcessHandler("DeviceModule", "/home/zsdn/zsdn-git/build/modules/default/DeviceModule", "/home/zsdn/zsdn-git/examples/example-configs/zmf_module.config");
		handler = new ModuleProcessHandler("SwitchAdapter", "/home/zsdn/zsdn-git/build/modules/default/SwitchAdapter", "/home/zsdn/zsdn-git/examples/example-configs/zmf_module.config", "1.0", "6633");
		launch();
		handler.destroy();
	}

	public static void updateTxt(final String line) {
		Platform.runLater(() -> ta.appendText("\n" + line));
	}

	@Override
	public void start(final Stage stage) throws Exception {
		ta = new TextArea();
		ta.setEditable(false);

		Button btn = new Button("Stop");
		btn.setOnAction(event -> {
			try {
				handler.destroy();
			} catch (InterruptedException | IOException e) {
				e.printStackTrace();
			}
		});

		BorderPane bp = new BorderPane(ta);
		bp.setBottom(btn);
		Scene scene = new Scene(bp, 800, 600);

		stage.setTitle(handler.module);
		stage.setScene(scene);
		stage.show();

		handler.start();
	}
}
