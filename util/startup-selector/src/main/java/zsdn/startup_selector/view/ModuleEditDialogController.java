package zsdn.startup_selector.view;

import java.io.File;

import javafx.fxml.FXML;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.TextField;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import zsdn.startup_selector.model.Module;

/**
 * Dialog to edit details of a module.
 * 
 * @author Mc
 */
public class ModuleEditDialogController {

    @FXML
    private TextField ModuleNameField;
    @FXML
    private TextField DescriptionField;
    @FXML
    private TextField PathField;
    @FXML
    private TextField ConfigFileField;
    @FXML
    private TextField ParametersField;
    
    private Stage dialogStage;
    private Module module;
    private boolean okClicked = false;

    /**
     * Initializes the controller class. This method is automatically called
     * after the fxml file has been loaded.
     */
    @FXML
    private void initialize() {
    }

    /**
     * Sets the stage of this dialog.
     * 
     * @param dialogStage
     */
    public void setDialogStage(Stage dialogStage) {
        this.dialogStage = dialogStage;
    }

    /**
     * Sets the module to be edited in the dialog.
     * 
     * @param module
     */
    public void setModule(Module module) {
        this.module = module;
        
        ModuleNameField.setText(module.getmoduleName());
        DescriptionField.setText(module.getmoduleDescription());
        PathField.setText(module.getPath());
        if(module.getConfigFilePath()==null){
        	ConfigFileField.setText("not selected");
        }else{        	
        	ConfigFileField.setText(module.getConfigFilePath());
        }
        if(module.getParameters()==null){
        	ParametersField.setText("not value");
        }else{
        	ParametersField.setText(module.getParameters());
        }
        //ConfigFileField.setText(module.getConfigFilePath());
       // ParametersField.setText(module.getParameters());
    }

    /**
     * Returns true if the user clicked OK, false otherwise.
     * 
     * @return
     */
    public boolean isOkClicked() {
        return okClicked;
    }

    /**
     * Called when the user clicks ok.
     */
    @FXML
    private void handleOk() {
        if (isInputValid()) {
            module.setmoduleName(ModuleNameField.getText());
            module.setmoduleDescription(DescriptionField.getText());
            module.setPath(PathField.getText());
            module.setConfigFilePath(ConfigFileField.getText());
            module.setParameters(ParametersField.getText());
      

            okClicked = true;
            dialogStage.close();
        }
    }

    /**
     * Called when the user clicks cancel.
     */
    @FXML
    private void handleCancel() {
        dialogStage.close();
    }


	@FXML
	private void handleOpenModule(){

		FileChooser fileChooser = new FileChooser();
		File selectedFile = fileChooser.showOpenDialog(null);
		
		PathField.setText(selectedFile.toString());
		//sets Module Name according to the selected file
		String[] bits = selectedFile.toString().split("/");
		String lastOne = bits[bits.length-1];
		ModuleNameField.setText(lastOne);
	
	/*	if (selectedFile != null) {		 
		    actionStatus.setText("File selected: " + selectedFile.getName());
		}
		else {
		    actionStatus.setText("File selection cancelled.");
		}*/
	}

    
	@FXML
	private void handleOpenConfig(){

		FileChooser fileChooser = new FileChooser();
		File selectedFile = fileChooser.showOpenDialog(null);
	//	module.setConfigFilePath(selectedFile.toString());
		ConfigFileField.setText(selectedFile.toString());
	
	/*	if (selectedFile != null) {		 
		    actionStatus.setText("File selected: " + selectedFile.getName());
		}
		else {
		    actionStatus.setText("File selection cancelled.");
		}*/
	}

    /**
     * Validates the user input in the text fields.
     * 
     * @return true if the input is valid
     */
    private boolean isInputValid() {
        String errorMessage = "";

        if (ModuleNameField.getText() == null || ModuleNameField.getText().length() == 0) {
            errorMessage += "No valid module name!\n"; 
        }
        if (DescriptionField.getText() == null || DescriptionField.getText().length() == 0) {
            errorMessage += "No valid description!\n"; 
        }
        if (PathField.getText() == null || PathField.getText().length() == 0) {
            errorMessage += "No valid path!\n"; 
        }
        if (ConfigFileField.getText() == null || ConfigFileField.getText().length() == 0) {
            errorMessage += "No valid path!\n"; 
        }
     /*   if (ParametersField.getText() == null || ParametersField.getText().length() == 0) {
            errorMessage += "No valid parameters!\n"; 
        }*/

        if (errorMessage.length() == 0) {
            return true;
        } else {
            // Show the error message.
            Alert alert = new Alert(AlertType.ERROR);
            alert.initOwner(dialogStage);
            alert.setTitle("Invalid Fields");
            alert.setHeaderText("Please correct invalid fields");
            alert.setContentText(errorMessage);

            alert.showAndWait();

            return false;
        }
    }
}