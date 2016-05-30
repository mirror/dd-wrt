public class ThreadStress extends Thread {
	public static final int N=10;

	public void run() {
		while (true) {
			System.gc();
		}
	}

	public static void main(String args[]) {
		ThreadStress t = null;

		System.out.println("Creating and starting threads ...");
		for (int i=0; i<N; i++) {
			t = new ThreadStress();
			t.start();
		}

		System.out.println("Finished.");
	}
}
