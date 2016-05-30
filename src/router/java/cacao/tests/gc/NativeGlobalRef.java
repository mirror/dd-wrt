public class NativeGlobalRef {
	public native static void setReference(Object o);
	public native static Object getReference();
	public native static void delReference();

	public static void main(String args[]) {
		Object o;
		System.loadLibrary("native");

		/* create the object we want to deal with */
		o = new String("I am an important object, don't forget me!");
		//String s = "";
		//for (int i=0; i<100; i++)
		//	s = s + "I am an important object, don't forget me!";
		//o = s;
		//s = null;

		/* pass the object to the native world */
		setReference(o);

		/* now forget about it and see if it gets collected */
		o = null;
		System.gc();

		/* fill up the heap */
		//for (long i=0; i<100000000l; i++)
		//	o = new String("I am simply an heap filler!");

		/* is the object still there? */
		o = getReference();
		System.out.println(o);

		/* delete the reference inside the native world */
		delReference();
	}
}
