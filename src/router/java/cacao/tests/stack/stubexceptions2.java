class MyTestClass2 {

	public MyTestClass2(int i) throws Exception {
		switch (i) {
			case 0:
				throw new Exception("Exception in constructor");
			case 1:
				int iarray[]=new int[10];
				iarray[11]=0;
				break;
			case 2:
				MyTestClass2 x=null;
				x.doSomething();
				break;
			case 3:
				System.out.println("Create new class");
				MyTestClass2 y=new MyTestClass2(3);
				break;
		}
	}

	public void doSomething() {
		System.out.println("Something done");
	}
}

public class stubexceptions2 {

	public static void main(String args[]) {
		System.out.println("Test1");

		for (int i=0;i<4;i++) {
			MyTestClass2 x=null;
			try {
				x=new MyTestClass2(i);
				x.doSomething();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
}
