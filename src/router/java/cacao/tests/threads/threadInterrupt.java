// This test has been added because of a bug in CACAO that allowed threads
// blocked inside monitorenter to be interrupted. In the presence of the bug,
// the program would not exit.
//
// The bug has been fixed as part of the sable lock implementation.
// hg revision 2988182011bb ff (Wed Feb 06 18:46:34 2008 +0100)

public class threadInterrupt {
	public static class firstthread implements Runnable {
		private threadInterrupt s;

		public firstthread(threadInterrupt s_) {
			s = s_;
		}
		public void run() {
			try {
				synchronized (s.o1) {
					System.out.println("first thread!");
					Thread.sleep(500);
					System.out.println("interrupting");
					s.t2.interrupt();
					System.out.println("leaving");
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public static class secondthread implements Runnable {
		private threadInterrupt s;

		public secondthread(threadInterrupt s_) {
			s = s_;
		}
		public void run() {
			try {
				Thread.sleep(250);
				synchronized (s.o1) {
					System.out.println("second thread!");
					s.o1.wait();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public Object o1 = new Object();
	public Thread t1 = null;
	public Thread t2 = null;

	public static void main(String args[]) {
		System.out.println("should exit with java.lang.InterruptedException");
		threadInterrupt s = new threadInterrupt();
		firstthread r1 = new firstthread(s);
		secondthread r2 = new secondthread(s);

		s.t1 = new Thread(r1, "a");
		s.t2 = new Thread(r2, "b");
		s.t1.start();
		s.t2.start();
	}
}

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: java
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
