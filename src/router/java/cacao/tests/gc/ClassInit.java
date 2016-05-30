class ClassInitTest {
	static {
		System.out.println("Static Initializer will call the GC ...");
		System.gc();
	}

	public static int val;

	public static void test() {
		System.out.println("Static method fine.");
	}
}

public class ClassInit {
	public static void main(String[] s) {
		String t;

		System.out.println("Preparing a String ...");
		t = new String("Remember Me!");

		/*System.out.println("Static Test Method will be called ...");
		ClassInitTest.test();*/

		System.out.println("Static Field will be accessed ...");
		ClassInitTest.val = 123;

		System.out.println("String: " + t);
		System.out.println("Field:  " + ClassInitTest.val);
		System.out.println("Test fine.");
	}
}
