package zsdn.startup_selector;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

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

  public void run(File dir) throws FileNotFoundException, UnsupportedEncodingException {

	Crawler fileSearch = new Crawler();
  
	//fileSearch.searchDirectory(new File(""+dir),"asdasd.txt");
	fileSearch.searchDirectory(new File(""+dir));

	int count = fileSearch.getResult().size();
	if(count ==0){
	    System.out.println("\nNo result found!");
	}else{
	    System.out.println("\nFound " + count + " result!\n");
	    
	    //save results to file
	    ///StartUpSelector/bin/ch/makery/address/view/
	    PrintWriter writer = new PrintWriter("moduleTemp.config", "UTF-8");
	    for (String matched : fileSearch.getResult()){
		System.out.println("Found : " + matched);
		writer.println(matched);
		
	
		
		
	    }
	    writer.close();
	}
  }
 // public void searchDirectory(File directory, String fileNameToSearch) {
  public void searchDirectory(File directory) {

	setFileNameToSearch(fileNameToSearch);

	if (directory.isDirectory()) {
	    search(directory);
	} else {
	    System.out.println(directory.getAbsoluteFile() + " is not a directory!");
	}

  }

  private void search(File file) {

	if (file.isDirectory()) {
	  System.out.println("Searching directory ... " + file.getAbsoluteFile());
		
            //do you have permission to read this directory?	
	    if (file.canRead()) {
		for (File temp : file.listFiles()) {
		    if (temp.isDirectory()) {
			search(temp);
		    } else {
		    if (temp.canExecute()&&!(temp.getName().contains("."))){
		    	result.add(temp.getAbsoluteFile().toString());
		    }
			//if (getFileNameToSearch().equals(temp.getName().toLowerCase())) {			
			//    result.add(temp.getAbsoluteFile().toString());
		    //}

		}
	    }

	 } else {
		System.out.println(file.getAbsoluteFile() + "Permission Denied");
	 }
      }

  }

}
