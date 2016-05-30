class staticinitializertest_1 {
	public static void a() {
		System.out.println("***");
	}
	static {
		String s=null;
		s.length();
	}
}

public class staticinitializertest {
	public static void a() {
		staticinitializertest_1.a();
	}
	public static void main(String args[]) {
		try {
			a();
		} catch (Throwable t) {
			System.out.println("Ok:Caught an exception");
			t.printStackTrace();
			System.out.println("Test2:");
			a();
		}
		System.out.println("Error:Should not be reached");
	}
}
