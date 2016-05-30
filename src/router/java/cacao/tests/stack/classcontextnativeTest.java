public class classcontextnativeTest {

	public classcontextnativeTest() {}

	private static void z() throws Exception{
//		try {
		Object o=Class.forName("blup",true,null).newInstance();
//		} catch (Throwable e) {
//			System.out.println("TEST: "+e.toString());
//		}
	}

	private static native void y() throws Exception;

	private static void x() throws Exception {
		y();
	}
	private static void w() throws Exception{
		x();
	}
	private static void v() throws Exception{
		w();
	}
	private static void u() throws Exception{
		v();
	}
	private static void t() throws Exception{
		u();
	}
	private static void s() throws Exception{
		t();
	}
	private static void r() throws Exception{
		s();
	}
	private static void q() throws Exception{
		r();
	}
	private static void p() throws Exception{
		q();
	}
	private static void o() throws Exception{
		p();
	}
	private static void n() throws Exception{
		o();
	}
	private static void m() throws Exception{
		n();
	}
	private static void l() throws Exception{
		m();
	}
	private native static void k() throws Exception;

	private static void j() throws Exception{
		k();
	}
	private static void i() throws Exception{
		j();
	}
	private static void h() throws Exception{
		i();
	}
	private static void g() throws Exception{
		h();
	}
	private static void f() throws Exception{
		g();
	}
	private static void e() throws Exception{
		f();
	}
	private static void d() throws Exception{
		e();
	}
	private static void c() throws Exception{
		d();
	}
	private static void b() throws Exception{
		c();
	}

	private native static void a() throws Exception;

	public static void main(String args[]) {
		System.setSecurityManager(new SecurityManager());
		try {
			a();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
