class inheritancetest_2 {
	inheritancetest_2() {
		System.out.println("inerhitancetest_2");
	}
}

class inheritancetest_1 extends inheritancetest_2 {
	inheritancetest_1() {
		System.out.println("inerhitancetest_1");
	}
}

public class inheritancetest {
	public static void main(String args[]) {
		try {
			inheritancetest_1 x=new inheritancetest_1();
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}
}
