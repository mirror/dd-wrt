public class StackTraceBig {
	public static void thrower(int n) throws Exception {
		if (n > 0)
			thrower(n - 1);
		else
			throw new Exception("Deep Exception!");
	}

	public static void main(String[] s) {
		try {
			thrower(100);
			System.out.println("FAILED: Exception not caught!");
		} catch (Exception e) {
			System.out.println("OK");
			e.printStackTrace();
		}
	}
}
