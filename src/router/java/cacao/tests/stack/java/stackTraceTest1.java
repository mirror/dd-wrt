/*GPL Licene Joseph Wenninger 2004*/
public class stackTraceTest1 {

	public static void a() throws Exception {
		b();
	}

	public static native void  b() throws Exception;

	public static void c() throws Exception {
		try {
			throw new Exception("ABCD");
		} catch (Exception e) {	
			e.printStackTrace();
			throw e;
		}
	}
	public static void main(String args[]) {
		System.loadLibrary("stackTraceTest1");

		System.out.println("Reference output:\nJava_stackTraceTest1_b\njava.lang.Exception: ABCD\n\tat stackTraceTest1.c(stackTraceTest1.java:12)\n\tat stackTraceTest1.b(Native Method)");
	        System.out.println("\tat stackTraceTest1.a(stackTraceTest1.java:5)\n\tat stackTraceTest1.main(stackTraceTest1.java:29)");
		System.out.println("java.lang.Exception: ABCD\n\tat stackTraceTest1.c(stackTraceTest1.java:12)\n\tat stackTraceTest1.b(Native Method)");
	        System.out.println("\tat stackTraceTest1.a(stackTraceTest1.java:5)\n\t at stackTraceTest1.main(stackTraceTest1.java:29");
		System.out.println("=======================================");
		System.out.println("Output of testcase:");

		try {
			a();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

}
