package zsdn.modulestarter;

import java.io.IOException;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


/**
 *
 * 
 * @author Jonas Grunert
 *
 */
public class Main {
	public static void main(String[] args) throws IOException, InterruptedException {

		Pattern pattern = Pattern.compile("\\s*((\".*?\")|\\S+)\\s*");

		List<Process> started = new ArrayList<>();

		for (String line : args) {
			System.out.println("Starting " + line);
			Matcher matcher = pattern.matcher(line);

			List<String> commands = new ArrayList<>();
			while (matcher.find()) {
				commands.add(matcher.group().trim());
			}
			ProcessBuilder pb = new ProcessBuilder();
			pb.redirectOutput(ProcessBuilder.Redirect.INHERIT);
			pb.redirectError(ProcessBuilder.Redirect.INHERIT);
			pb.command(commands);
			started.add(pb.start());
		}

		final Thread shutdownThread = new Thread(() -> started.forEach(Process::destroy));
		Runtime.getRuntime().addShutdownHook(shutdownThread);

		try (Scanner scanner = new Scanner(System.in)) {
			do {
				System.out.println("Type \"stop\" to terminate the controller.");
			} while(!scanner.next().equals("stop"));

		}

		System.out.println("Shutting down the controller");
		started.forEach(Process::destroy);
		for(Process p : started) {
			p.waitFor();
		}
	}
}
