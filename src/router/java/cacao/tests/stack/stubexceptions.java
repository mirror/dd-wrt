class MyTestClass {
	public void doSomething() {
		System.out.println("Do something");
	}

	public void doSomethingWithException() {
		System.out.println("Do somethingWithException:");
			MyTestClass x=null;
			x.doSomething();

	}

}

public class stubexceptions {

	public static void main(String args[]) {
		System.out.println("Test1");
		try {
			MyTestClass x=null;
			x.doSomething();
		} catch (Exception e) {
			e.printStackTrace();
		}

		try {
			MyTestClass x=new MyTestClass();
			x.doSomethingWithException();
		} catch (Exception e) {
			e.printStackTrace();
		}

		
		
		System.out.println("ArrayIndexOutOfBounds_CHECK (1)");
		try {
			int iarray[]=new int[10];
			iarray[11]=20;
		} catch (Exception e) {
			e.printStackTrace();
		}

		System.out.println("ArrayIndexOutOfBounds_CHECK (2)");
		try {
			int iarray[]=new int[10];
			int i=iarray[11];
		} catch (Exception e) {
			e.printStackTrace();
		}

		System.out.println("NegativeArraySize_CHECK");
		try {
			int iarray[]=new int[-1];
		} catch (Exception e) {
			e.printStackTrace();
		}

		System.out.println("NegativeArrayIndex_CHECK");
		try {
			int iarray[]=new int[10];
			iarray[-1]=20;
		} catch (Exception e) {
			e.printStackTrace();
		}

		System.out.println("OOM_EXCEPTION_CHECK");
		try {
			int iarray[]=new int[1000000000];
		} catch (Exception e) {
			e.printStackTrace();
		}
	}


}
