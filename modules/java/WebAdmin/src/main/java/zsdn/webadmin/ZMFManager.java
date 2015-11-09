package zsdn.webadmin;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;

import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedLong;

import jmf.Launcher;
import jmf.data.ModuleUniqueId;
import jmf.module.IFrameworkController;

/**
 * Manages Java Module
 * @author Maksim Pahlberg
 *
 */
public class ZMFManager extends HttpServlet {
	
	private static final int MODULE_ENABLE_WAIT_TIME = 5;
	
	public static AdminModule adminModuleInstance;
	private static IFrameworkController adminModuleController;
	
	//TODO (Maksim) remove dis and implement web.xml
	static {
		
		//ModuleUniqueId muID = new ModuleUniqueId(UnsignedInteger.fromIntBits(17),UnsignedLong.fromLongBits(1));
		//adminModuleInstance = new AdminModule(muID,UnsignedInteger.valueOf(1),"RestAdminModule",null);
		//adminModuleInstance.enable();
	}
	

	public void init() throws ServletException {
		System.out.println("STARTED!!");
		ModuleUniqueId muID = new ModuleUniqueId(UnsignedInteger.fromIntBits(17),UnsignedLong.fromLongBits(1));
		adminModuleInstance = new AdminModule(muID,UnsignedInteger.valueOf(1),"RestAdminModule",null);
		adminModuleController = Launcher.createInstance(adminModuleInstance, true, true, true, false);
		// Wait for up to MODULE_ENABLE_WAIT_TIME seconds for enable
		for(int i = 0; i < MODULE_ENABLE_WAIT_TIME; i++) {
			if(adminModuleInstance.isEnabled()) {
				break;
			}
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
				break;
			}
		}
		
		if(!adminModuleInstance.isEnabled()) {
			System.err.println("Failed to enable admin module");
			adminModuleController.stopInstance();
			System.out.println("Stopped JMF instance after fail");
			throw new RuntimeException("Failed to enable admin module");
		}
	}
	
}
