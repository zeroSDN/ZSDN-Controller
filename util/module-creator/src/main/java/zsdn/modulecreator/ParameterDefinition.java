package zsdn.modulecreator;

import com.beust.jcommander.Parameter;

public class ParameterDefinition {

	@Parameter(names = {"--help","-h"}, help = true)
	private boolean help;

	@Parameter(names = {"--template_folder", "-t"}, description = "folder of template module.", required = true)
	protected String templateFolder;
	
	@Parameter(names = {"--template_ut_folder","-u"}, description = "folder of template module unittest.")
	protected String templateUnitestFolder;
	
	@Parameter(names = {"--target_folder", "-d"}, description = "folder where module should be created.",required = true)
	protected String targetFolder;
	
	@Parameter(names = {"--module_name", "-n"}, description = "name of new Module.",required = true)
	protected String moduleName;

	@Parameter(names = {"--module_id", "-i"}, description = "id of module, as 4 hex digits (e.g. \"00af\").",required = true)
	protected String moduleId;
}
