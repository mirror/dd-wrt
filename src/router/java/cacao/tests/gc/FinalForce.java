public class FinalForce {
	public static final int N=8;

	public int id;
	
	protected void finalize() throws Throwable {
		System.out.println("\tFinalizing object #" + id);

		for (long i=0; i<40000000; i++);

		super.finalize();
	}

	public static void main(String args[]) {
		FinalForce f = null;

		System.out.println("Creating objects ...");
		for (int i=0; i<N; i++) {
			f = new FinalForce();
			f.id = i;
		}
		f = null;

		System.out.println("Forcing finalization ...");
		System.runFinalization();

		System.out.println("Forcing collection ...");
		System.gc();

		System.out.println("Forcing finalization ...");
		System.runFinalization();

		System.out.println("Shutting down ...");
		System.exit(0);
	}
}
