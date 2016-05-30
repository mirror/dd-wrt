import java.util.Vector;
import java.util.Enumeration;

public class Final {
	public static final int N=20;
	public static final int SLEEP=1000;
	public static final int MOD_REMEMBERED =3;
	public static final int MOD_RESURRECTED=4;
	public static final int MOD_UGLY       =10;

	public static Vector<Final> resurrected;
	public static Vector<Final> remembered;

	public int id;
	
	protected void finalize() throws Throwable {
		System.out.println("\tFinalized object #" + id);

		if (id % MOD_RESURRECTED == 0) {
			System.out.println("\tFinalizer #" + id + " resurrected object");
			resurrected.add(this);
		}

		if (id % MOD_UGLY == 0) {
			System.out.println("\tFinalizer #" + id + " does an ugly thing!");
			System.gc();
		}

		super.finalize();
	}

	public static void main(String args[]) {
		Enumeration<Final> en;
		Final f = null;

		resurrected = new Vector<Final>(N);
		remembered  = new Vector<Final>(N);

		System.out.println("Creating objects ...");
		for (int i=0; i<N; i++) {
			f = new Final();
			f.id = i;

			if (i % MOD_REMEMBERED == 0) {
				System.out.println("\tRemembering object #" + i);
				remembered.add(f);
			}

		}
		f = null;

		System.out.println("Forcing collection 1 ...");
		System.gc();
		System.out.println("Forcing collection 2 ...");
		System.gc();
		System.out.println("Forcing collection 3 ...");
		System.gc();

		for (long i=0; i<400000000; i++);
		/*try {
			Thread.sleep(SLEEP);
		} catch (Exception e) {
		}*/

		en = remembered.elements();
		while (en.hasMoreElements()) {
			f = en.nextElement();
			System.out.println("Preserved object #" + f.id);
		}

		en = resurrected.elements();
		while (en.hasMoreElements()) {
			f = en.nextElement();
			System.out.println("Resurrected object #" + f.id);
		}

		f = null;
		remembered = null;
		resurrected = null;

		System.out.println("Forcing collection 4 ...");
		System.gc();
		System.out.println("Forcing collection 5 ...");
		System.gc();

		for (long i=0; i<400000000; i++);
		/*try {
			Thread.sleep(SLEEP);
		} catch (Exception e) {
		}*/

		System.out.println("Shutting down ...");
		System.exit(0);
	}
}
