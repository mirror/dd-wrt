public class InlineSynchronized2 extends Thread{

	private InlineSynchronized2 m_o;
	String m_name;
	public InlineSynchronized2(String name, InlineSynchronized2 o) {
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
	public static void testit(InlineSynchronized2 o) throws Exception {
		synchronized(o) {
			System.out.println("Within protected section, about to throw an exception");
			throw new Exception("");
		}
	}
	public static void main(String args[]) {
		InlineSynchronized2 o=new InlineSynchronized2("dummy",null);
		InlineSynchronized2 o1=new InlineSynchronized2("1",o);
		InlineSynchronized2 o2=new InlineSynchronized2("2",o);
		InlineSynchronized2 o3=new InlineSynchronized2("3",o);
		o1.start();
		o2.start();
		o3.start();
	}
}
