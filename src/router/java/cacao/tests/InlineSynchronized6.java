public class InlineSynchronized6 extends Thread{

	private InlineSynchronized6 m_o;
	String m_name;
	public InlineSynchronized6(String name, InlineSynchronized6 o) {
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
	public static synchronized void testit(InlineSynchronized6 o) throws Exception {
			System.out.println("Within protected section, about to throw an exception");
			throw new Exception("");
	}
	public static void main(String args[]) {
		InlineSynchronized6 o=new InlineSynchronized6("dummy",null);
		InlineSynchronized6 o1=new InlineSynchronized6("1",o);
		InlineSynchronized6 o2=new InlineSynchronized6("2",o);
		InlineSynchronized6 o3=new InlineSynchronized6("3",o);
		o1.start();
		o2.start();
		o3.start();
	}
}
