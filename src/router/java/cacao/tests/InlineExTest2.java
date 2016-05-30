public class InlineExTest2 {
	public static void internal() throws Exception {
		throw new Exception("*");
	}

	public static void main(String args[]) {
		try {
			internal();
			System.out.println("ERROR EXCEPTION EXPECTED");
		} catch (Exception e) {
			System.out.println(e);
			System.exit(0);
		}
		System.out.println("SHOULD NOT BE REACHED");
	}
}
