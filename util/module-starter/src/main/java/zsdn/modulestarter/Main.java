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
			Matcher matcher = pattern.matcher(line);

			List<String> commands = new ArrayList<>();
			while (matcher.find()) {
				commands.add(matcher.group().trim());
			}
			ProcessBuilder b = new ProcessBuilder();
			b.inheritIO();
			b.command(commands);
			started.add(b.start());
		}

		final Thread shutdownThread = new Thread(() -> started.forEach(Process::destroy));
		Runtime.getRuntime().addShutdownHook(shutdownThread);

		for (Process p : started) {
			p.waitFor();
		}
	}
}
