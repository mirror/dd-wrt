import java.security.AccessController;
import sun.security.action.*;
import java.io.File;

public class classloader {
    private static String[] initializePath(String propname) {
        String ldpath = System.getProperty(propname, "");
	String ps = File.pathSeparator;
	int ldlen = ldpath.length();
	int i, j, n;
	// Count the separators in the path
	i = ldpath.indexOf(ps);
	n = 0;
	while (i >= 0) {
	    n++;
	    i = ldpath.indexOf(ps, i+1);
	}

	// allocate the array of paths - n :'s = n + 1 path elements
	String[] paths = new String[n + 1];

	// Fill the array with paths from the ldpath
	n = i = 0;
	j = ldpath.indexOf(ps);
	while (j >= 0) {
	    if (j - i > 0) {
	        paths[n++] = ldpath.substring(i, j);
	    } else if (j - i == 0) { 
	        paths[n++] = ".";
	    }
	    i = j + 1;
	    j = ldpath.indexOf(ps, i);
	}
	paths[n] = ldpath.substring(i, ldlen);
	return paths;
    }

    public static void main (String args[]) {
	/*
	String dirs[] = initializePath("sun.boot.library.path");

	System.out.println("length: " + dirs.length);
	for (int i = 0; i < dirs.length; ++i) {
	    System.out.println(dirs[i]);
	}
	*/
	AccessController.doPrivileged(new LoadLibraryAction("net"));
    }
}
