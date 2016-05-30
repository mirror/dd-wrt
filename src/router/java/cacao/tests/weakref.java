import java.lang.ref.*;
import java.util.HashSet;

class weakref {
	class OtherThread extends Thread {
		public volatile boolean quitNow = false;
		private final ReferenceQueue q;
		private final HashSet h;
		public OtherThread(ReferenceQueue q, HashSet h) {
			this.q = q;
			this.h = h;
		}
		public void run() {
			while (!quitNow) {
				try {
					MyRef r = (MyRef) q.remove();
					h.remove(r);
					System.out.println("Integer: " + Integer.toString(r.val));
				} catch (InterruptedException e) {
				}
			}
		}
	}

	class MyRef extends WeakReference {
		public final int val;
		MyRef(Object o, ReferenceQueue q, int val) {
			super(o, q);
			this.val = val;
		}
	}

	private void test() {
		System.out.println("This should print a long list of Integers if weak references are working.");
		ReferenceQueue q = new ReferenceQueue();
		HashSet h = new HashSet();
		OtherThread t = new OtherThread(q, h);
		t.start();
		for (int i=0; i<1000000; i++) {
			Object o = new Integer(i);
			Reference r = new MyRef(o, q, i);
			h.add(r);
		}
		Runtime.getRuntime().gc();
		try {
			Thread.sleep(1000);
			t.quitNow = true;
			t.interrupt();
			t.join();
		} catch (InterruptedException e) {
		}
	}

	public static void main(String[] args) {
		new weakref().test();
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
