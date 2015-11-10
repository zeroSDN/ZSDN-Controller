/**
 * The StartUpSelector is a GUI tool that helps to try out the ZSDN.
 * It allows to list modules contained by a folder by selecting it.
 * Single modules can be edited and therefore allow settings of parameters and config files.
 * @author Matthias Hoppe
 */
// Based on a Tutorial by Marco Jakob
// http://code.makery.ch/library/javafx-8-tutorial/

package zsdn.startup_selector;

import java.io.File;
import java.io.IOException;
import java.util.prefs.Preferences;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import javafx.application.Application;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.layout.AnchorPane;
import javafx.scene.layout.BorderPane;
import javafx.stage.Modality;
import javafx.stage.Stage;
import zsdn.startup_selector.model.Module;
import zsdn.startup_selector.model.ModuleListWrapper;
import zsdn.startup_selector.view.ModuleEditDialogController;
import zsdn.startup_selector.view.ModuleOverviewController;
import zsdn.startup_selector.view.RootLayoutController;

public class MainApp extends Application {

	private Stage primaryStage;
	private BorderPane rootLayout;

	/**
	 * The data as an observable list of Modules.
	 */
	private ObservableList<Module> ModuleData = FXCollections
			.observableArrayList();

	/**
	 * Constructor
	 */
	public MainApp() {

	}

	/**
	 * Returns the data as an observable list of Module.
	 * 
	 * @return
	 */
	public ObservableList<Module> getModuleData() {
		return ModuleData;
	}

	@Override
	public void start(Stage primaryStage) {
		this.primaryStage = primaryStage;
		this.primaryStage.setTitle("Module Startup");

		initRootLayout();

		showModuleOverview();
	}

	/**
	 * Initializes the root layout and tries to load the last opened module
	 * file.
	 */
	public void initRootLayout() {
		try {
			// Load root layout from fxml file.
			FXMLLoader loader = new FXMLLoader();
            loader.setLocation(MainApp.class.getClassLoader()
					.getResource("view/RootLayout.fxml"));
			rootLayout = loader.load();

			// Show the scene containing the root layout.
			Scene scene = new Scene(rootLayout);
			primaryStage.setScene(scene);

			// Give the controller access to the main app.
			RootLayoutController controller = loader.getController();
			controller.setMainApp(this);

			primaryStage.show();
		} catch (IOException e) {
			e.printStackTrace();
		}

		// Try to load last opened person file.
		File file = getModuleFilePath();
		if (file != null) {
			loadModuleDataFromFile(file);
		}
	}

	/**
	 * Shows the module overview inside the root layout.
	 */
	public void showModuleOverview() {
		try {
			// Load module overview.
			FXMLLoader loader = new FXMLLoader();
			loader.setLocation(MainApp.class.getClassLoader()
					.getResource("view/ModuleOverview.fxml"));
			AnchorPane moduleOverview = loader.load();

			// Set module overview into the center of root layout.
			rootLayout.setCenter(moduleOverview);

			// Give the controller access to the main app.
			ModuleOverviewController controller = loader.getController();
			controller.setMainApp(this);

		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Returns the main stage.
	 * 
	 * @return
	 */
	public Stage getPrimaryStage() {
		return primaryStage;
	}

	/**
	 * Opens a dialog to edit details for the specified module. If the user
	 * clicks OK, the changes are saved into the provided module object and true
	 * is returned.
	 * 
	 * @param module
	 *            the module object to be edited
	 * @return true if the user clicked OK, false otherwise.
	 */
	public boolean showModuleEditDialog(Module module) {
		try {
			// Load the fxml file and create a new stage for the popup dialog.
			FXMLLoader loader = new FXMLLoader();
			loader.setLocation(MainApp.class.getClassLoader()
					.getResource("view/ModuleEditDialog.fxml"));
			AnchorPane page = loader.load();

			// Create the dialog Stage.
			Stage dialogStage = new Stage();
			dialogStage.setTitle("Edit Module");
			dialogStage.initModality(Modality.WINDOW_MODAL);
			dialogStage.initOwner(primaryStage);
			Scene scene = new Scene(page);
			dialogStage.setScene(scene);

			// Set the module into the controller.
			ModuleEditDialogController controller = loader.getController();
			controller.setDialogStage(dialogStage);
			controller.setModule(module);

			// Show the dialog and wait until the user closes it
			dialogStage.showAndWait();

			return controller.isOkClicked();
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
	}

	/**
	 * Returns the module file preference, i.e. the file that was last opened.
	 * The preference is read from the OS specific registry. If no such
	 * preference can be found, null is returned.
	 * 
	 * @return
	 */
	public File getModuleFilePath() {
		Preferences prefs = Preferences.userNodeForPackage(MainApp.class);
		String filePath = prefs.get("filePath", null);
		if (filePath != null) {
			return new File(filePath);
		} else {
			return null;
		}
	}

	/**
	 * Sets the file path of the currently loaded file. The path is persisted in
	 * the OS specific registry.
	 * 
	 * @param file
	 *            the file or null to remove the path
	 */
	public void setModuleFilePath(File file) {
		Preferences prefs = Preferences.userNodeForPackage(MainApp.class);
		if (file != null) {
			prefs.put("filePath", file.getPath());

			// Update the stage title.
			primaryStage.setTitle("AddressApp - " + file.getName());
		} else {
			prefs.remove("filePath");

			// Update the stage title.
			primaryStage.setTitle("AddressApp");
		}
	}

	/**
	 * Loads module data from the specified file. The current module data will
	 * be replaced.
	 * 
	 * @param file
	 */
	public void loadModuleDataFromFile(File file) {
		try {

			try {
				JAXBContext context = JAXBContext
						.newInstance(ModuleListWrapper.class);
				Unmarshaller um = context.createUnmarshaller();

				// Reading XML from the file and unmarshalling.
				ModuleListWrapper wrapper = (ModuleListWrapper) um
						.unmarshal(file);
				ModuleData.clear();
				ModuleData.addAll(wrapper.getModules());
				// Save the file path to the registry.
				setModuleFilePath(file);
			} catch (Exception e) {
				System.out.println("No previous modules available.");
			}

		} catch (NullPointerException np) {
			if (file == null) {
				Alert alert = new Alert(AlertType.ERROR);
				alert.setTitle("Error");
				alert.setHeaderText("Last used file not found");
				alert.setContentText("Could not load data from file.");
						

				alert.showAndWait();
			}

		} catch (Exception e) { // catches ANY exception

			Alert alert = new Alert(AlertType.ERROR);
			alert.setTitle("Error");
			alert.setHeaderText("Unknown error.");
			// alert.setContentText(""+e);
			// alert.setContentText("Could not load data from file:\n" +
			// file.getPath());

			alert.showAndWait();
		}

	}

	/**
	 * Saves the current person data to the specified file.
	 * 
	 * @param file
	 */
	public void saveModuleDataToFile(File file) {
		try {
			JAXBContext context = JAXBContext
					.newInstance(ModuleListWrapper.class);
			Marshaller m = context.createMarshaller();
			m.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);

			// Wrapping our module data.
			ModuleListWrapper wrapper = new ModuleListWrapper();
			wrapper.setModules(ModuleData);

			// Marshalling and saving XML to the file.
			m.marshal(wrapper, file);

			// Save the file path to the registry.
			setModuleFilePath(file);
		} catch (Exception e) { // catches ANY exception
			Alert alert = new Alert(AlertType.ERROR);
			alert.setTitle("Error");
			alert.setHeaderText("Could not save data");
			alert.setContentText("Could not save data to file:\n"
					+ file.getPath());

			alert.showAndWait();
		}
	}

	public static void main(String[] args) {
		launch(args);

	}

}