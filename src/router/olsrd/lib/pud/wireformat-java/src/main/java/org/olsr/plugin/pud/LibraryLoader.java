package org.olsr.plugin.pud;

public class LibraryLoader {
	static private boolean loaded = false;

	/* Load uplink message handling library */
	static void load() {
		if (!loaded) {
			try {
				System.loadLibrary(WireFormatConstants.LibraryName);
				loaded = true;
			} catch (Throwable e) {
				throw new ExceptionInInitializerError(e);
			}
		}
	}
}
