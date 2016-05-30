public class InlineSynchronized {

	public static void testit(InlineSynchronized o) throws Exception {
		synchronized(o) {
			System.out.println("Within protected section, about to throw an exception");
			throw new Exception("");
		}
	}
	public static void main(String args[]) {
		InlineSynchronized o=new InlineSynchronized();
		try {
			try {
				testit(o);
			} catch (Exception e) {
				System.out.println("First catch");
				testit(o);
			}
		} catch (Exception e) {
			System.out.println("Second catch");
		}
	}
}
