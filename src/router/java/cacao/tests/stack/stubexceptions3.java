class MyTestClass3 {


	public static void test() {
		System.out.println("Test1");

		for (int i=0;i<4;i++) {
			MyTestClass3 x=null;
			try {
				x=new MyTestClass3(i);
				x.doSomething();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	public MyTestClass3(int i) throws Exception {
		switch (i) {
			case 0:
				throw new Exception("Exception in constructor");
			case 1:
				int iarray[]=new int[10];
				iarray[11]=0;
				break;
			case 2:
				MyTestClass3 x=null;
				x.doSomething();
				break;
			case 3:
				System.out.println("Create new class");
				MyTestClass3 y=new MyTestClass3(3);
				break;
		}
	}

	public void doSomething() {
		System.out.println("Something done");
	}
}

public class stubexceptions3 {

	public static void main(String args[]) {
			MyTestClass3.test();
	}
}
