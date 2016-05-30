public class hello2 {
	static int f() {
		int i=100;
		i=i+20;
		return i;
	}
	public static void main(String[] s) {
		for (int i=0;i<f(); i++) {
			System.out.println(i);
		}
		
		int i=0;
		while (i<f()) {
			System.out.println("i+1");
		}

	}
}
