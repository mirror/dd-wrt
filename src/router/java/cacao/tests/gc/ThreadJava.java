public class ThreadJava extends Thread {
	public static final int N=5;
	public static final int SLEEP=200;
	public static final int LEN=20;

	public void run() {
		while (true) {
			double d = Math.sqrt(Math.PI);
			String s = Double.toString(d);
			double e = Double.parseDouble(s);
			double f = Math.pow(e, 2);
			long  l1 = Math.round(f);
			long  l2 = Math.round(Math.PI);
			if (l1 != l2)
				System.out.println("What is going on???");
		}
	}

	public static void main(String args[]) {
		ThreadJava t = null;

		System.out.println("Creating and starting threads ...");
		for (int i=0; i<N; i++) {
			t = new ThreadJava();
			t.start();
		}
		t = null;

		System.out.println("Sending Main-Thread to sleep ...");
		try {
			Thread.sleep(SLEEP);
		} catch (Exception e) {
			System.out.println("Exception while sleeping!");
		}

		System.out.println("Forcing a collection ...");
		System.gc();

		System.out.println("Finished.");
	}
}
