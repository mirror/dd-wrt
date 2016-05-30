public class InlineSynchronized3 extends Thread{

	private InlineSynchronized3 m_o;
	String m_name;
	public InlineSynchronized3(String name, InlineSynchronized3 o) {
		super(name);
		m_o=o;
		m_name=name;
	}

	public void run() {
		while (true) {
			testit(m_o);
			System.out.println("("+m_name+")");
			try {
				sleep(2000);
			} catch (Exception e) {}
		}
	}
	public static void testit(InlineSynchronized3 o) {
		synchronized(o) {
			System.out.println("Within protected section, about to throw an exception");
		}
	}
	public static void main(String args[]) {
		InlineSynchronized3 o=new InlineSynchronized3("dummy",null);
		InlineSynchronized3 o1=new InlineSynchronized3("1",o);
		InlineSynchronized3 o2=new InlineSynchronized3("2",o);
		InlineSynchronized3 o3=new InlineSynchronized3("3",o);
		o1.start();
		o2.start();
		o3.start();
	}
}
