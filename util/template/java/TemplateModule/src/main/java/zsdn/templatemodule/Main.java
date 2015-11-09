package zsdn.templatemodule;

import java.util.Optional;

import jmf.Launcher;
import jmf.module.IFrameworkController;

public class Main {

	public static void main(String[] args) {

		long instanceId = 0;

		IFrameworkController instance = Launcher.createInstance(new TemplateModule(instanceId), Optional.empty());
		instance.joinExecution();
	}

}
