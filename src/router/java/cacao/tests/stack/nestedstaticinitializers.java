class nestedstaticinitializers_clinit1 {
	public static int x;
	static {
		nestedstaticinitializers_clinit2.x=3;
	}

}

class nestedstaticinitializers_clinit2 {
	public static int x;
	static {
		nestedstaticinitializers_clinit3.x=3;
	}

}

class nestedstaticinitializers_clinit3 {
	public static int x;
	static {
		int y[]=new int[-1];
	}

}

class nestedstaticinitializers {
	public static void main (String args[]) {
		try {
			nestedstaticinitializers_clinit1.x=3;
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}
}
