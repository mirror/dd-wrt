import java.util.Vector;

public class LockRecords {
	static final int N = 3;

	static Vector v;

	static class MyObject
	{
		public int id;
		public int counter;

		protected void finalize() throws Throwable
		{
			System.out.println("\tObject #" + id + " gets finalized");
			throw new Exception("Object #" + id + " is nasty!");
		}
	};

	static class MyThread extends Thread
	{
		public String name;
		public synchronized void run()
		{
			System.out.println("\t" + name + ": Starting ...");
			try {
				for (int i=0; i<N; i++) {
					MyObject o = (MyObject) v.get(i);
					System.out.println("\t" + name + ": Trying on #" + o.id);
					synchronized(o) {
						System.out.println("\t" + name + ": Locked on #" + o.id);
						this.wait(200);
						System.out.println("\t" + name + ": Releasing #" + o.id);
						o.counter--;
					}
					while (o.counter > 0) {
						this.wait(10);
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
			System.out.println("\t" + name + ": finished.");
		}
	};

	public synchronized void test()
	{
		System.out.println("Creating Objects ...");
		v = new Vector();
		for (int i=0; i<N; i++) {
			MyObject o = new MyObject();
			o.id = i;
			o.counter = 2;
			v.add(o);
		}

		System.out.println("Starting Blocking Threads A + B...");
		MyThread threadA = new MyThread();
		MyThread threadB = new MyThread();
		threadA.name = "A"; threadA.start();
		threadB.name = "B"; threadB.start();
		try {
			threadA.join();
			threadB.join();
		} catch (Exception e) {
			e.printStackTrace();
		}

		System.out.println("Cleaning up ...");
		v = null;
		System.gc();

		System.out.println("Waiting some seconds ...");
		try {
			wait(3000);
		} catch (Exception e) {
			e.printStackTrace();
		}

		System.out.println("Cleaning up again ...");
		System.gc();
	}

	public static void main(String args[])
	{
		LockRecords test = new LockRecords();
		test.test();
	}
}
