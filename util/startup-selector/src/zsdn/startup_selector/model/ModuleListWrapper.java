package zsdn.startup_selector.model;


import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * Helper class to wrap a list of Modules. This is used for saving the
 * list of persons to XML.
 * 
 * @author Matthias Hoppe
 */
@XmlRootElement(name = "modules")
public class ModuleListWrapper {

    private List<Module> modules;

    @XmlElement(name = "module")
    public List<Module> getModules() {
        return modules;
    }

    public void setModules(List<Module> modules) {
        this.modules = modules;
    }
}