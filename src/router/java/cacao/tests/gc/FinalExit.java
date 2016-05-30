public class FinalExit {
	public static final int N=8;

	public int id;
	
	protected void finalize() throws Throwable {
		System.out.println("\tFinalized object #" + id);

		super.finalize();
	}

	public static void main(String args[]) {
		FinalExit f = null;

		System.out.println("Enabling runFinalizersOnExit ...");
		System.runFinalizersOnExit(true);

		System.out.println("Creating objects ...");
		for (int i=0; i<N; i++) {
			f = new FinalExit();
			f.id = i;
		}
		f = null;

		System.out.println("Shutting down ...");
		System.exit(0);
	}
}
