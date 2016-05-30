import java.lang.reflect.*;

public class ReflectEx {
	public ReflectEx() {
		throw new ClassCastException();
	}



	public static void main(String [] args) {
		try {
			ReflectEx o=new ReflectEx();
			System.out.println("Test 1 failed (should have gotten exception)");
		} catch (Exception e) {
			if (! (e instanceof ClassCastException)) {
				System.out.println("Test 1 failed (wrong exception)");
			} else {
				System.out.println("Test 1 OK");
			}
		}


		try {
			Class c= Class.forName("ReflectEx");
			Constructor con=c.getConstructors()[0];

			Object o=con.newInstance(new Object[0]);
			System.out.println("Test 2 failed (should have gotten exception)");
		} catch (Exception e) {
			if (! (e instanceof InvocationTargetException)) {
				System.out.println("Test 2 failed (wrong exception)");
			} else {
				System.out.println("Test 2 OK");
			}
		}


		System.out.println("End of test");
	}
}
