public class InlineExTest3 {
	public static void internal() throws Exception {
		System.out.println("INTERNAL");
	}

	public static void main(String args[]) {
		try {
			internal();
			throw new Exception("ex");
			//System.out.println("ERROR EXCEPTION EXPECTED");
		} catch (Exception e) {
			System.out.println(e);
			System.exit(0);
		}
	//	System.out.println("SHOULD NOT BE REACHED");
	}
}
