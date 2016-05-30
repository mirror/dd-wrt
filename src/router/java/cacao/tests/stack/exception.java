public class exception {

	public exception() {
	}

	public static void b(int v) throws Exception{
		throw new Exception("Exception: value="+v);
	}

	public static void a() throws Exception {
		try {
			b(1);
		} catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
			System.out.println("Caught in a()");
		}

		b(2);
	}

	public static void c() throws Exception{

		try {
			d();
		} catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
			System.out.println("Caught in c(), rethrowing");
			throw e;
		}


	}

	public static void d() throws Exception{
		throw new Exception("Exception: value="+4);
	}

	public static void e() throws Exception{

		try {
			f();
		} catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
			System.out.println("Caught in e(), refilling stacktrace and  rethrowing");
			e.fillInStackTrace();
			throw e;
		}


	}

	public static void f() throws Exception{
		System.out.println("Entering f");
		throw new Exception("Exception: value="+5);
	}



	public static void main(String args[]){
		try {
			a();
		}
		catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
		}
		try {
			throw new Exception("3");
		}
		catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
			
		}

		try {
			c();
		}
		catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
			
		}

		try {
			e();
		}
		catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
			
		}

		try {
			throw new ClassCastException();
		} catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
		}

		throw new ClassCastException();
	}
}
