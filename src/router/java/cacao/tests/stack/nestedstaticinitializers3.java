class nestedstaticinitializers3_clinit1 extends nestedstaticinitializers3_clinit2 {

	public nestedstaticinitializers3_clinit1() {
		System.out.println("Never reached");
	}

	public static int x1;
	static {
		x1=1;
	}

}

class nestedstaticinitializers3_clinit2 {
	public static int x;
	static {
		nestedstaticinitializers2_clinit3.x=3;
	}

}

class nestedstaticinitializers3_clinit3 {
	public static int x;
	static {
		int y[]=new int[-1];
	}

}

class nestedstaticinitializers3 {
	public static void main (String args[]) {
		try {
			nestedstaticinitializers2_clinit1  x[]=new nestedstaticinitializers2_clinit1[10];
			x[0]=new nestedstaticinitializers2_clinit1();
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}
}
