package zsdn.startup_selector.model;

import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

/**
 * Model class for a Module.
 *
 * @author Mc
 */
public class Module {

	private StringProperty status;
    private StringProperty moduleName;
    private StringProperty moduleDescription;
    private StringProperty path;
    private StringProperty configFile;
    private StringProperty parameters;
    //private final IntegerProperty postalCode;
    //private final StringProperty city;
    //private final ObjectProperty<LocalDate> birthday;

    /**
     * Default constructor.
     */
    public Module() {
        this(null, null);
    }

    /**
     * Constructor with some initial data.
     * 
     * @param moduleName
     * @param moduleDescription
     */
    public Module(String moduleName, String moduleDescription) {
        this.moduleName = new SimpleStringProperty(moduleName);
        this.moduleDescription = new SimpleStringProperty(moduleDescription);
        
        this.status = new SimpleStringProperty("Stopped");
        //this.moduleName = new SimpleStringProperty("Testmodule1");
        //this.moduleDescription = new SimpleStringProperty("This is a test module.");
        this.path = new SimpleStringProperty("Module is stored here");
        this.moduleDescription = new SimpleStringProperty("Module Description");
        this.configFile = new SimpleStringProperty("Config File Path");
        this.parameters = new SimpleStringProperty("Seperate Parameters with a blank");

    }

    public String getstatusProperty() {
		
		return status.get();
	}
    
    public void setstatusProperty(String status) {
    	this.status.set(status);
	}
    
	public StringProperty statusProperty() {
		
		return status;
	}
	
    
    public String getmoduleName() {
        return moduleName.get();
    }

    public void setmoduleName(String moduleName) {
        this.moduleName.set(moduleName);
    }

    public StringProperty moduleNameProperty() {
        return moduleName;
    }

    public String getmoduleDescription() {
        return moduleDescription.get();
    }

    public void setmoduleDescription(String moduleDescription) {
        this.moduleDescription.set(moduleDescription);
    }

    public StringProperty moduleDescriptionProperty() {
        return moduleDescription;
    }

    public String getPath() {
        return path.get();
    }
    
    
    public String getPathWithoutModule(){
    	return path.get().substring(0, this.getPath().length()-this.getmoduleName().length()-1);
    }

    public void setPath(String path) {
        this.path.set(path);
    }
    
    public String getConfigFilePath() {
        return configFile.get();
    }
    
    public void setConfigFilePath(String configFilePath){
    	this.configFile.set(configFilePath);
    }
    
    public String getParameters() {
        return parameters.get();
    }
    
    public void setParameters(String parameters){
    	this.parameters.set(parameters);
    }

    public StringProperty ParametersProperty() {
        return path;
    }
    
 /*   public String getStreet() {
        return street.get();
    }

    public void setStreet(String street) {
        this.street.set(street);
    }

    public StringProperty streetProperty() {
        return street;
    }

    public int getPostalCode() {
        return postalCode.get();
    }

    public void setPostalCode(int postalCode) {
        this.postalCode.set(postalCode);
    }

    public IntegerProperty postalCodeProperty() {
        return postalCode;
    }

    public String getCity() {
        return city.get();
    }

    public void setCity(String city) {
        this.city.set(city);
    }

    public StringProperty cityProperty() {
        return city;
    }

    public LocalDate getBirthday() {
        return birthday.get();
    }

    public void setBirthday(LocalDate birthday) {
        this.birthday.set(birthday);
    }

    public ObjectProperty<LocalDate> birthdayProperty() {
        return birthday;
    }
 */


}