package zsdn.startup_selector.view;


import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;

import javafx.application.Platform;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.Label;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.TextArea;
import javafx.stage.DirectoryChooser;
import javafx.stage.Window;
import zsdn.startup_selector.Crawler;
import zsdn.startup_selector.MainApp;
import zsdn.startup_selector.model.Module;



public class ModuleOverviewController {
	@FXML
	private TextArea outputBox;
	@FXML
	private TableView<Module> moduleTable;
	@FXML
	private TableColumn<Module, String> statusColumn;
	@FXML
	private TableColumn<Module, String> modulesColumn;

	@FXML
	private Label moduleNameLabel;
	@FXML
	private Label moduleDescriptionLabel;
	@FXML
	private Label pathLabel;
	@FXML
	private Label pathConfigFileLabel;
	@FXML
	private Label parametersLabel;
	// Reference to the main application.
	private MainApp mainApp;



	Map<String, ModuleProcessHandler> ModuleProcesses = new HashMap<String, ModuleProcessHandler>();

	/**
	 * The constructor. The constructor is called before the initialize()
	 * method.
	 */
	public ModuleOverviewController() {
	}

	/**
	 * Is called by the main application to give a reference back to itself.
	 * 
	 * @param mainApp
	 */
	public void setMainApp(MainApp mainApp) {
		this.mainApp = mainApp;

		// Add observable list data to the table
		moduleTable.setItems(mainApp.getModuleData());
	}

	public void setOutputBox(String output) {
		this.outputBox.setText(output);
	}

	/**
	 * Fills all text fields to show details about the module. If the specified
	 * module is null, all text fields are cleared.
	 * 
	 * @param module
	 *            the module or null
	 */
	private void showModuleDetails(Module module) {
		if (module != null) {
			// Fill the labels with info from the module object.
			moduleNameLabel.setText(module.getmoduleName());
			moduleDescriptionLabel.setText(module.getmoduleDescription());
			pathLabel.setText(module.getPath());
			pathConfigFileLabel.setText(module.getConfigFilePath());
			parametersLabel.setText(module.getParameters());

			// TODO: We need a way to convert the birthday into a String!
			// birthdayLabel.setText(...);
		} else {
			// module is null, remove all the text.
			moduleNameLabel.setText("");
			moduleDescriptionLabel.setText("");
			pathLabel.setText("");
			pathConfigFileLabel.setText("");
			parametersLabel.setText("");

			// parametersLabel.setText("");
		}
	}

	@FXML
	private void initialize() {
		// Initialize the module table with the two columns.
		statusColumn.setCellValueFactory(cellData -> cellData.getValue()
				.statusProperty());
		modulesColumn.setCellValueFactory(cellData -> cellData.getValue()
				.moduleNameProperty());

		// Clear module details.
		showModuleDetails(null);
		
		outputBox.setEditable(false);

		// Listen for selection changes and show the module details when
		// changed.
		moduleTable
				.getSelectionModel()
				.selectedItemProperty()
				.addListener(
						(observable, oldValue, newValue) -> showModuleDetails(newValue));
	}

	// dirty, dirty
	// called to destroy selected modules process
	@FXML
	private void handleStopModule() {
		Module selectedModule = moduleTable.getSelectionModel()
				.getSelectedItem();
		stopModule(selectedModule);

	}
	
	@FXML
	private void handleStopAllModules() {

		ObservableList<Module> selectedModules = moduleTable.getItems();
		for (Module currentModule : selectedModules) {
			stopModule(currentModule);
		}
	}
	
	private void stopModule(Module selectedModule){
		if (selectedModule != null) {
			ModuleProcessHandler pb = ModuleProcesses.get(selectedModule.getmoduleName());
			try {
				pb.destroy();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		
			if (!pb.isAlive()){
				selectedModule.setstatusProperty("Stopped");
				System.out.println("Module Stopped");
			}
			else
				System.out.println("couldn't stop module"); // TODO
			// System.out.println(selectedModule.getmoduleName() +" is alive: "
			// + p.isAlive());
			 
			 

		}
	}

	/**
	 * Called when the user clicks on the delete button.
	 */
	@FXML
	private void handleDeleteModule() {
		int selectedIndex = moduleTable.getSelectionModel().getSelectedIndex();
		if (selectedIndex >= 0) {
			moduleTable.getItems().remove(selectedIndex);
		} else {
			// Nothing selected.
			Alert alert = new Alert(AlertType.WARNING);
			alert.initOwner(mainApp.getPrimaryStage());
			alert.setTitle("No Selection");
			alert.setHeaderText("No Module Selected");
			alert.setContentText("Please select a module in the table.");

			alert.showAndWait();
		}
	}

	/*
	 * adds found modules to modules list in the GUI. called by handleOpenDir()
	 */
	public void addNewFoundModules() {

		java.net.URL url = getClass().getResource("moduleTemp.config");
		File file = new File(url.getPath());

		try (BufferedReader br = new BufferedReader(new FileReader(file))) {
			for (String line; (line = br.readLine()) != null;) {
				// process the line.
				// adding found executable to module list
				Module tempModule = new Module();
				if (tempModule != null) {
					// Fill the labels with info from the module object.
					tempModule.setPath(line);
					String[] name = line.split("/");
					String lastOne = name[name.length - 1];

					tempModule.setmoduleName(lastOne);
					moduleNameLabel.setText(tempModule.getmoduleName());
					moduleDescriptionLabel.setText(tempModule
							.getmoduleDescription());
					pathLabel.setText(tempModule.getPath());
					pathConfigFileLabel.setText(tempModule.getConfigFilePath());
					parametersLabel.setText(tempModule.getParameters());

				} else {
					// module is null, remove all the text.
					moduleNameLabel.setText("");
					moduleDescriptionLabel.setText("");
					pathLabel.setText("");
					pathConfigFileLabel.setText("");
					parametersLabel.setText("");

					// parametersLabel.setText("");
				} 
				mainApp.getModuleData().add(tempModule);
				mainApp.setModuleFilePath(file);

			}
			// line is not visible here.
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	/**
	 * Called when the user clicks the new button. Opens a dialog to edit
	 * details for a new module.
	 */
	@FXML
	private void handleNewModule() {
		Module tempModule = new Module();
		boolean okClicked = mainApp.showModuleEditDialog(tempModule);
		if (okClicked) {
			mainApp.getModuleData().add(tempModule);
		}
	}

	/**
	 * Called when the user clicks the edit button. Opens a dialog to edit
	 * details for the selected module.
	 */
	@FXML
	private void handleEditModule() {
		Module selectedModule = moduleTable.getSelectionModel()
				.getSelectedItem();
		if (selectedModule != null) {
			boolean okClicked = mainApp.showModuleEditDialog(selectedModule);
			if (okClicked) {
				showModuleDetails(selectedModule);
			}

		} else {
			// Nothing selected.
			Alert alert = new Alert(AlertType.WARNING);
			alert.initOwner(mainApp.getPrimaryStage());
			alert.setTitle("No Selection");
			alert.setHeaderText("No module Selected");
			alert.setContentText("Please select a module in the table.");

			alert.showAndWait();
		}
	}

	/*
	 * searches selected directories for executables (all files that don't
	 * contain a "." and are executable) then calls addNewFoundModules() to add
	 * them to modules list in the GUI
	 */
	@FXML
	private void handleOpenDir() throws FileNotFoundException,
			UnsupportedEncodingException {
		DirectoryChooser directoryChooser = new DirectoryChooser();
		directoryChooser.setTitle("Open Resource File");
		Window stage = null;
		// directoryChooser.showDialog(stage);
		File selectedDirectory = directoryChooser.showDialog(stage);
		if (selectedDirectory.getAbsolutePath() != null) {
			System.out.println(selectedDirectory.getAbsolutePath());
		} else {
			System.out.println("No directory selected.");
		}
		Crawler crawler = new Crawler();
		crawler.run(selectedDirectory);
		addNewFoundModules();

	}

	/**
	 * Called when the user clicks the edit button. Opens a dialog to edit
	 * details for the selected module.
	 */
	@FXML
	private void handleStartSelectedModules() {

		Module selectedModule = moduleTable.getSelectionModel()
				.getSelectedItem();
		startModule(selectedModule);

	}
	
	private void startModule(Module selectedModule){
		

		if (selectedModule != null) {


			// builds string array from file, configpath and parameters
			String paraTemp =""
					+ selectedModule.getConfigFilePath() + " "
					+ selectedModule.getParameters();
			String[] para = paraTemp.split("\\ ", -1);

			//ProcessBuilder pb = new ProcessBuilder(para);
			
			//ProcessBuilder pb = new ProcessBuilder(para);
			ModuleProcessHandler pb = new ModuleProcessHandler(this,"","./" + selectedModule.getmoduleName(),para);
			
			try {
				// is a process of this module already running?
				if (ModuleProcesses.get(selectedModule.getmoduleName()) == null||!(ModuleProcesses.get(selectedModule.getmoduleName())
						.isAlive())) {

					//Process p = pb.start();
					
					pb.start();
					selectedModule.setstatusProperty("Running");
					ModuleProcesses.put(selectedModule.getmoduleName(), pb);
					System.out.println("outputBOX: ");
				
							}
				else{ System.out.println("Module is already running");}

			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		
	}

	@FXML
	private void handleStartAllModules() {

		ObservableList<Module> selectedModules = moduleTable.getItems();
		for (Module currentModule : selectedModules) {
			startModule(currentModule);

			/*if (selectedModules != null) {

				// builds string array from file, configpath and parameters
				String paraTemp = "./" + currentModule.getmoduleName() + " "
						+ currentModule.getConfigFilePath() + " "
						+ currentModule.getParameters();
				String[] para = paraTemp.split("\\ ", -1);

				//ProcessBuilder pb = new ProcessBuilder(para);
				ModuleProcessHandler pb = new ModuleProcessHandler(this,"","./" + currentModule.getmoduleName(), currentModule.getConfigFilePath(),
						currentModule.getParameters());
				try {
					pb.start();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

			}*/

		}

	}
	
	
	
	public void updateTextArea(final String text) {
		Platform.runLater(()-> {outputBox.appendText("\n"+text);});
  	}
}