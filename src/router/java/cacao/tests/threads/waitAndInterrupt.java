// This should run forever. If it stops, that's a good indication for a bug in
// the VM.
//
// This test grew a bit more elaborate than anticipated...
// It verifies that the JVM handles properly the case of a thread being
// interrupted and notified at the same time.

public class waitAndInterrupt {
	private class semaphore {
		private int v;
		public semaphore(int v) {
			this.v = v;
		}
		public synchronized void semwait() {
			while (v == 0)
				try {
					wait();
				} catch (InterruptedException e) {
				}
			v--;
		}
		public synchronized void sempost() {
			if (v == 0)
				notify();
			v++;
		}
	}

	public static class firstthread implements Runnable {
		private waitAndInterrupt s;

		public firstthread(waitAndInterrupt s_) {
			s = s_;
		}
		public void run() {
			boolean iAmFirst = Thread.currentThread() == s.t1;
			try {
				int i = 0;
				int count_not = 0;
				int count_int = 0;
				for (;;) {
					if (iAmFirst) {
						if (++i == 100) {
							i = 0;
							System.out.println(Thread.currentThread().getName() + " still running, notified " + Integer.toString(count_not) + ", interrupted " + Integer.toString(count_int));
						}
						synchronized (s) {
							s.sem1.sempost();
							try {
								while (!s.notified)
									s.wait();
								try {
									s.wait();
								} catch (InterruptedException e) {
									s.notify(); // wake t2
								}
								count_not++;
							} catch (InterruptedException e) {
								count_int++;
							}
						}

						s.sem5.sempost();
						s.sem8.semwait();
					} else {
						s.sem1.semwait();
						if (++i == 100) {
							i = 0;
							System.out.println(Thread.currentThread().getName() + " still running");
						}
						synchronized (s) {
							s.sem2.sempost();
							try {
								while (!s.notified)
									s.wait();
								s.notified = false;
								count_not++;
							} catch (InterruptedException e) {
								count_int++;
							}
						}

						s.sem6.sempost();
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public static class otherthread implements Runnable {
		private waitAndInterrupt s;

		public otherthread(waitAndInterrupt s_) {
			s = s_;
		}
		public void run() {
			boolean iAmFirst = Thread.currentThread() == s.t3;
			try {
				int i = 0;
				for (;;) {
					if (iAmFirst) {
						s.sem3.semwait();
						if (++i == 100) {
							i = 0;
							System.out.println(Thread.currentThread().getName() + " still running");
						}
						synchronized (s) {
							s.sem4.sempost();
						}
						s.t1.interrupt();
						s.sem5.semwait();
					} else {
						s.sem4.semwait();
						if (++i == 100) {
							i = 0;
							System.out.println(Thread.currentThread().getName() + " still running");
						}
						synchronized (s) {
							if (s.notified)
								System.out.println("shouldn't happen (1)");
							s.notified = true;
							s.notify();
						}
						s.sem6.semwait();
					}
					s.sem7.sempost();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public static class controlthread implements Runnable {
		private waitAndInterrupt s;

		public controlthread(waitAndInterrupt s_) {
			s = s_;
		}
		public void run() {
			try {
				for (;;) {
					s.sem2.semwait();
					synchronized (s) {
					}
					s.sem3.sempost();
					s.sem7.semwait();
					s.sem7.semwait();
					s.sem8.sempost(); // wake first
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public Thread t1 = null;
	public Thread t2 = null;
	public Thread t3 = null;
	public Thread t4 = null;
	public semaphore sem1 = new semaphore(0);
	public semaphore sem2 = new semaphore(0);
	public semaphore sem3 = new semaphore(0);
	public semaphore sem4 = new semaphore(0);
	public semaphore sem5 = new semaphore(0);
	public semaphore sem6 = new semaphore(0);
	public semaphore sem7 = new semaphore(0);
	public semaphore sem8 = new semaphore(0);
	public boolean notified = false;

	public static void main(String args[]) {
		waitAndInterrupt s = new waitAndInterrupt();
		firstthread r1 = new firstthread(s);
		firstthread r2 = new firstthread(s);
		otherthread r3 = new otherthread(s);
		controlthread r5 = new controlthread(s);

		s.t1 = new Thread(r1, "a");
		s.t2 = new Thread(r2, "b");
		s.t3 = new Thread(r3, "c");
		s.t4 = new Thread(r3, "d");
		Thread t5 = new Thread(r5, "e");
		s.t1.start();
		s.t2.start();
		s.t3.start();
		s.t4.start();
		t5.start();
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
