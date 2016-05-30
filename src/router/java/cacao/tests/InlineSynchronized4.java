public class InlineSynchronized4 extends Thread{

	private InlineSynchronized4 m_o;
	String m_name;
	public InlineSynchronized4(String name, InlineSynchronized4 o) {
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
	public static void testit(InlineSynchronized4 o){
		try {
			synchronized(o) {
				System.out.println("Within protected section, about to throw an exception");
				throw new Exception("");
			}
		}
		catch (Exception e) {
			System.out.println("Exception caught");
		}
	}
	public static void main(String args[]) {
		InlineSynchronized4 o=new InlineSynchronized4("dummy",null);
		InlineSynchronized4 o1=new InlineSynchronized4("1",o);
		InlineSynchronized4 o2=new InlineSynchronized4("2",o);
		InlineSynchronized4 o3=new InlineSynchronized4("3",o);
		o1.start();
		o2.start();
		o3.start();
	}
}
