public class Chain {
	public static final int N=500000;

	public Chain next;

	public static void main(String args[]) {
		Chain top = null;

		System.out.println("Building chain ...");
		for (int i=0; i<N; i++) {
			Chain c = new Chain();
			c.next = top;
			top = c;
		}

		System.out.println("Forcing collection ...");
		System.gc();
	}
}
