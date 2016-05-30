public class Hashcode {
	public static final boolean doPrintHashes = false;
	public static final int N = 20;
	public static final int N2 = N/2;

	public static void compareHashes(int[] a, int[] b) {
		boolean diff = false;

		for (int i=0; i<a.length; i++)
			if (a[i] != b[i]) {
				System.out.println("\tHash #" + i + " changed!");
				diff = true;
			}

		if (diff)
			System.out.println("\tHashes are DIFFERENT!");
		else
			System.out.println("\tHashes are ok.");
	}

	public static void printHashes(int[] a) {
		for (int i=0; i<a.length; i++)
			System.out.println("\t#" + i + ": " + a[i]);
	}

	public static int[] getHashes(Object[] a) {
		int[] result = new int[a.length];

		for (int i=0; i<a.length; i++)
			result[i] = System.identityHashCode(a[i]);

		return result;
	}

	public static void main(String[] s) {
		Object[] a = new Object[N];
		int[] hashes1, hashes2;

		System.out.println("Creating objects ...");
		for (int i=0; i<N2; i++)
			a[i] = new Object();
		for (int i=N2+1; i<N; i++)
			a[i] = new String("String-" + i);

		System.out.println("Forgetting some objects ...");
		for (int i=0; i<N; i+=2)
			a[i] = null;

		System.out.println("Getting hashes before GC ...");
		hashes1 = getHashes(a);
		if (doPrintHashes)
			printHashes(hashes1);

		System.out.println("Invoking the GC ...");
		System.gc();
		System.out.println("\tfinished.");

		System.out.println("Getting hashes after GC ...");
		hashes2 = getHashes(a);
		if (doPrintHashes)
			printHashes(hashes2);

		System.out.println("Comparing hashes now ...");
		compareHashes(hashes1, hashes2);
	}
}
