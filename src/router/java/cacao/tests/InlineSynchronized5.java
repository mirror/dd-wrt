public class InlineSynchronized5 extends Thread{

	private InlineSynchronized5 m_o;
	String m_name;
	public InlineSynchronized5(String name, InlineSynchronized5 o) {
		super(name);
		m_o=o;
		m_name=name;
	}

	public void run() {
		while (true) {
			try {
				try {
					testit(m_o);
				} catch (Exception e) {
					System.out.println(m_name+":First catch");
					testit(m_o);
				}
			} catch (Exception e) {
				System.out.println(m_name+":Second catch");
			}
			try {
				sleep(2000);
			} catch (Exception e) {}
		}
	}
	public static void testit(InlineSynchronized5 o) throws Exception {
		try {
			synchronized(o) {
				System.out.println("Within protected section, about to throw an exception");
				throw new Exception("");
			}
		} catch (Exception e) {
			System.out.println("Exception caught, rethrowing");
			throw e;
		}
	}
	public static void main(String args[]) {
		InlineSynchronized5 o=new InlineSynchronized5("dummy",null);
		InlineSynchronized5 o1=new InlineSynchronized5("1",o);
		InlineSynchronized5 o2=new InlineSynchronized5("2",o);
		InlineSynchronized5 o3=new InlineSynchronized5("3",o);
		o1.start();
		o2.start();
		o3.start();
	}
}
