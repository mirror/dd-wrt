import java.lang.reflect.*;

public class BasicToStrings {


	private static void check(String res, String expected) {
		if (res.equals(expected)) System.out.println("OK");
		else System.out.println("got "+res+", expected "+expected);
	}


	public static void main(String [] args) {

		int [][] i_a=new int[10][10];
		Object [][] o_a=new Object[10][10];

		check(i_a.getClass().getName(),"[[I");
		check(i_a.getClass().toString(),"class [[I");
		check(o_a.getClass().getName(),"[[Ljava.lang.Object;");
		check(o_a.getClass().toString(),"class [[Ljava.lang.Object;");
		check(""+i_a.getClass(),"class [[I");
		check(""+o_a.getClass(),"class [[Ljava.lang.Object;");
		try {
			Method m=BasicToStrings.class.getDeclaredMethod("main",new Class[]{(new String[10]).getClass()});
			check (m.toString(),"public static void BasicToStrings.main(java.lang.String[])");
			check (m.getName(),"main");
			check (""+m,"public static void BasicToStrings.main(java.lang.String[])");
		} catch (Exception e) {
			System.out.println("Exception:"+e);
		}
	}

}
