// This pathological test tries to grow a FLC list as long as possible before
// the threads from the previous FLC list have resumed running. Every one on
// the old list then has to scan the entire new list.

// The CACAO patch in file sableContention.patch can be used to examine the
// maximum length. With 500 threads on a quad-core system, I managed to get to
// about 120.

class sableContention {
	public Object a[] = null;
	public tt ts[] = null;
	final int NUM = 500;

	class tt extends Thread {
		sableContention y;
		int x;
		tt(sableContention y, int x) {
			this.y = y;
			this.x = x;
		}
		public void run() {
			int i = 1;
			synchronized(y.a[x]) {
				if (x==0) {
					for (; i<NUM*3/4; i++)
						y.ts[i].start();
					y.a[x].notify();
				}
			}
			if (x==0)
				for (; i<NUM; i++)
					y.ts[i].start();
			for (int j=0; j<NUM/10; j++)
				synchronized(y.a[(x+j)%(NUM-1)+1]) {
				}
		}
	}

	private void l(int f) {
		try {
			synchronized(a[NUM-1-f]) {
				if (f > 0) {
					l(f-1);
					if (f<10)
						Thread.sleep(0, f*100 * 1000);
				}
				else {
					ts = new tt[NUM];
					for (int i=0; i<NUM; i++)
						ts[i] = new tt(this, i);
					ts[0].start();
					a[0].wait();
				}
			}
		} catch (InterruptedException e) {
		}
	}

	private void r() {
		for (;;) {
			a = new Object[NUM];
			for (int i=0; i<NUM; i++)
				a[i] = new Object();
			l(NUM-1);
			for (int i=0; i<NUM; i++)
				try {
					ts[i].join();
				} catch (InterruptedException e) {
				}
			System.out.println("running");
		}
	}

	public static void main(String[] args) {
		new sableContention().r();
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
