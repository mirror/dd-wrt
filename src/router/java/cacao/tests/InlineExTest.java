public class InlineExTest {
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
		} catch (Exception e) {
			System.out.println(e);
			e.printStackTrace();
			System.out.println("End of outer exception handler");
			System.exit(0);
		}
		System.out.println("SHOULD NOT BE REACHED");
	}
}
