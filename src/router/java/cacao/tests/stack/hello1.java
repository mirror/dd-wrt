public class hello1 {
	static int f() {
		return 100;
	}
	public static void main(String[] s) {
		for (int i=0;i<f(); i++) {
			System.out.println(i);
		}
		
		int i=0;
		while (i<f()) {
			System.out.println("i+1");
			i=i+1;
		}

	}
}
