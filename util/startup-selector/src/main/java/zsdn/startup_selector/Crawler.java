package zsdn.startup_selector;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;

/**
 * The Crawler lets you search executables contained in directories and
 * sub-directories
 */

public class Crawler {

	private String fileNameToSearch;
	private List<String> result = new ArrayList<String>();

	public String getFileNameToSearch() {
		return fileNameToSearch;
	}

	public void setFileNameToSearch(String fileNameToSearch) {
		this.fileNameToSearch = fileNameToSearch;
	}

	public List<String> getResult() {
		return result;
	}

	public void run(File dir) throws FileNotFoundException,
			UnsupportedEncodingException {

		Crawler fileSearch = new Crawler();

		// fileSearch.searchDirectory(new File(""+dir),"asdasd.txt");
		fileSearch.searchDirectory(new File("" + dir));

		int count = fileSearch.getResult().size();

		// no executable in this directory
		if (count == 0) {
			System.out.println("No executable found in directory.");

			Alert alert = new Alert(AlertType.WARNING);
			// alert.initOwner(mainApp.getPrimaryStage());
			alert.setTitle("No executable found.");
			alert.setHeaderText("No executable found in directory.");
			alert.setContentText("Please select directory containing modules.");

			alert.showAndWait();
			System.out.println("\nNo result found!");
		} else {
			System.out.println("\nFound " + count + " result!\n");

			// save results to file
			PrintWriter writer = new PrintWriter("moduleTemp.config", "UTF-8");
			for (String matched : fileSearch.getResult()) {
				System.out.println("Found : " + matched);
				writer.println(matched);

			}
			writer.close();
		}
	}

	public void searchDirectory(File directory) {

		setFileNameToSearch(fileNameToSearch);

		if (directory.isDirectory()) {
			search(directory);
		} else {
			System.out.println(directory.getAbsoluteFile()
					+ " is not a directory!");
		}

	}

	private void search(File file) {

		if (file.isDirectory()) {
			System.out.println("Searching directory ... "
					+ file.getAbsoluteFile());

			// do you have permission to read this directory?
			if (file.canRead()) {
				for (File temp : file.listFiles()) {
					if (temp.isDirectory()) {
						search(temp);
					} else {
						if (temp.canExecute()
								&& !(temp.getName().contains("."))) {
							result.add(temp.getAbsoluteFile().toString());
						}

					}
				}

			} else {
				System.out
						.println(file.getAbsoluteFile() + "Permission Denied");
			}
		}

	}

}
