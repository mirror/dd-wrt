import java.util.Vector;
import java.io.FileInputStream;

class ClassUnloadTest {
	public static Vector classEater;

	public static void classTest() {
		System.out.println("TC: Testclass eats up memory ...");
		classEater = new Vector();
		for (int i=0; i<1000000; i++)
			classEater.add(new String("I am a memory eater!"));

		System.out.println("TC: Testclass fine.");
	}

	static {
		System.out.println("TC: Static Initializer ...");
	}
}

class ClassUnloadLoader extends ClassLoader {
	public Vector myEater;

	public ClassUnloadLoader() {
		super();
		System.out.println("CL: Initializer ...");
	}

	public Class myLoad() {
		Class c;

		System.out.println("CL: Classloader loads testing class ...");
		try {
			c = loadClass("ClassUnloadTest");
			ClassUnloadTest.classTest();
		} catch (Exception e) {
			System.out.println("EXCEPTION: " + e);
			c = null;
		}

		System.out.println("CL: Classloader eats up memory ...");
		myEater = new Vector();
		for (int i=0; i<1000000; i++)
			myEater.add(new String("I am a memory eater!"));

		return c;
	}

	public Class loadClass(String name) throws ClassNotFoundException {
		Class c;
		FileInputStream s;
		byte[] buf;
		int size;

		if (name != "ClassUnloadTest") {
			c = super.loadClass(name);
			return c;
		}

		System.out.println("CL: Now loading \'" + name + "\' ...");
		try {
			s = new FileInputStream(name + ".class");
			size = s.available();
			buf = new byte[size];
			s.read(buf);
			System.out.println("\tloaded a total of " + size + " bytes.");
			c = super.defineClass(name, buf, 0, size);
		} catch (Exception e) {
			System.out.println("EXCEPTION: " + e);
			throw new ClassNotFoundException();
		}

		return c;
	}

	protected void finalize() {
		System.out.println("CL: Classloader will be unloaded ...");
	}
}

public class ClassUnload {
	public static void printMemInfo() {
		Runtime r = Runtime.getRuntime();
		/*System.out.println("\tfree : " + r.freeMemory());*/
		/*System.out.println("\ttotal: " + r.totalMemory());*/
		/*System.out.println("\tmax  : " + r.maxMemory());*/
		System.out.println("\tused : " + (r.totalMemory() - r.freeMemory()));
	}

	public static void main(String[] s) {
		ClassUnloadLoader l;
		Class c;

		System.out.println("--: Running program ...");
		printMemInfo();

		System.out.println("--: Create classloader ...");
		l = new ClassUnloadLoader();

		System.out.println("--: Load stuff ...");
		c = l.myLoad();
		System.out.println("\tclass = " + c);

		printMemInfo();

		/*System.out.println("\tDEBUG l=" + l);*/
		/*System.out.println("\tDEBUG c=" + c);*/
		/*System.out.println("\tDEBUG c.loader=" + c.getClassLoader());*/

		/* REMEMBER: if you comment out one of the following two lines, no class unloading should happen! */
		l = null;
		c = null;

		System.out.println("--: Collect stuff ...");
		System.gc();
		System.gc();
		System.gc();

		printMemInfo();

		/*System.out.println("\tDEBUG l=" + l);*/
		/*System.out.println("\tDEBUG c=" + c);*/
		/*System.out.println("\tDEBUG c.loader=" + (c==null ? null : c.getClassLoader()));*/

		System.out.println("done.");
	}
}
