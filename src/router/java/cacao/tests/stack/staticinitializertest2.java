class staticinitializertest2_1 {
	public static void a() {
		System.out.println("***");
	}
	public static void b() {
		System.out.println("***");
	}
	static {
		String s=null;
		s.length();
	}
}

public class staticinitializertest2 {
	public static void a() {
		staticinitializertest2_1.a();
	}
	public static void b() {
		staticinitializertest2_1.b();
	}
	public static void main(String args[]) {
		try {
			a();
		} catch (Throwable t) {
			System.out.println("Ok:Caught an exception");
			t.printStackTrace();
			System.out.println("Test2:");
			b();
		}
		System.out.println("Error:Should not be reached");
	}
}
