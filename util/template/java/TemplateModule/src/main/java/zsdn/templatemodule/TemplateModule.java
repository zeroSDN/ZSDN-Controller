package zsdn.templatemodule;

import java.util.Collection;
import java.util.Collections;

import jmf.data.ModuleDependency;
import jmf.data.ModuleUniqueId;
import jmf.module.AbstractModule;

/**
 * [TODO]
 * 
 * @author [TODO]
 *
 */
public class TemplateModule extends AbstractModule {

	private static final short MODULE_TYPE_ID = 0x0000;
	private static final short VERSION = 0;
	private static final String MODULE_NAME = "TemplateModule";
	private static final Collection<ModuleDependency> DEPENDENCIES = Collections.emptyList();

	public TemplateModule(long instanceId) {
		super(new ModuleUniqueId(MODULE_TYPE_ID, instanceId), VERSION, MODULE_NAME, DEPENDENCIES);
	}

	@Override
	public boolean enable() {
		// TODO enable code
		return true;
	}

	@Override
	public void disable() {
		// TODO disable code
	}
}
