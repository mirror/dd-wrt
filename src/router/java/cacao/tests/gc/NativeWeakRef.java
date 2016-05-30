public class NativeWeakRef {
	public native static void setWeakReference(Object o);
	public native static Object getWeakReference();
	public native static void delWeakReference();

	public static void main(String args[]) {
		Object o;
		System.loadLibrary("native");

		/* create the object we want to deal with */
		o = new String("I am not important, you can forget me!");

		/* pass the object to the native world */
		setWeakReference(o);

		/* is the object still there? */
		o = getWeakReference();
		System.out.println(o);

		/* now forget about it and see if it gets collected */
		o = null;
		System.gc();

		/* is the object still there? */
		o = getWeakReference();
		System.out.println(o);

		/* delete the reference inside the native world */
		delWeakReference();
	}
}
