public class arrayNewInstance {
public static void main(String args[]) {
	Object[] oa1 = (Object[]) new Runnable[10]; 
	System.out.println("Test 1 succeeded");
	Object[] oa2 = (Object[])java.lang.reflect.Array.newInstance(String.class, 1);
	System.out.println("Test 2 succeeded");
	Runnable[] ra3 = (Runnable[]) java.lang.reflect.Array.newInstance(Runnable.class, 1);
	System.out.println("Test 3 succeeded");
	Object[] oa4=(Object[]) ra3;
	System.out.println("Test 4 succeeded");
	Object[] oa5 = (Object[]) java.lang.reflect.Array.newInstance(Runnable.class, 1);
	System.out.println("Test 5 succeeded");
}

}
