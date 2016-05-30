public class New {
	public static final int N=1000000;
	public static final String TEMPLATE="I am simply a space filler";

	public static void main(String args[]) {
		String s;

		System.out.println("Creating lots of Strings ...");
		s = null;
		for (int i=0; i<N; i++) {
			s = new String(TEMPLATE);
		}

		System.out.println("The String was: \'" + s + "\'");

		System.out.println("done.");
	}
}
