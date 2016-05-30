import java.util.Random;

public class ThreadSleep extends Thread {
	public static final int N=5;
	public static final int MOD_GC=2;
	public static final int SLEEP_FIX=1000;
	public static final int SLEEP_RND=500;

	public static Random rnd;

	public int id;
	public int cnt;
	public Object o;

	public void run() {
		String myString;

		myString = new String("LocObj#" + this.id);

		while (true) {
			try {
				sleep(SLEEP_FIX + rnd.nextInt(SLEEP_RND));
			} catch(Exception e) {
				System.out.println("Thread #" + this.id + " had an Exception");
			}

			this.cnt++;

			System.out.println("(" + this.id + ") Thread woke up:");
			System.out.println("(" + this.id + ")\t cnt=" + this.cnt + ", o=\'" + this.o + "\'");
			System.out.println("(" + this.id + ")\t local=\'" + myString + "\'");

			if (this.cnt % MOD_GC == 0) {
				System.out.println("(" + this.id + ") Starting the GC now!...");
				System.gc();
			}
		}
	}

	public static void main(String args[]) {
		ThreadSleep t = null;

		rnd = new Random();

		System.out.println("Creating and starting threads ...");
		for (int i=0; i<N; i++) {
			t = new ThreadSleep();
			t.id = i;
			t.cnt = 0;
			t.o = new String("GlobObj#" + i);
			t.start();
		}
		t = null;

		System.out.println("Finished.");
	}
}
