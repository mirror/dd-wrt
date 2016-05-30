public class InlineExTest4 {
	public static void internal() throws Exception {
		try {
			throw new Exception("*");
		} catch (Exception e) {
			throw new Exception("*"+e.getMessage());
		}
	}

	public static void main(String args[]) {
		try {
			internal();
			System.out.println("ERROR EXCEPTION EXPECTED");
		} catch (Throwable  e) {
			System.out.println(e);
			System.out.println("End of outer exception handler");
			System.exit(0);
		}
		System.out.println("SHOULD NOT BE REACHED");
	}
}
