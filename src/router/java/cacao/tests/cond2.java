/* Copyright (C) 2005, Joseph Wenninger <jowenn@kde.org> 
GNU General Public License
*/


public class cond2 {

	private static int test(int i) {
		return i==0 ? 0: 1;
	}

	private static int test2(int i) {
		i= i==0 ? 0: 1;
		i=i+10;
		return i;
	}


	private static int test3(int i) {
		i= i==0 ? 0: 1;
		try {
			i=i+10;
		} catch (Exception e) {}
		return i;

	}

	private static int test4(int i) {
		return test5(
			(i==0) ? 0: 1
		);
	}

	private static int test5(int i) {
		return i;
	}

	private static int test6(int i) {
		i= i==0 ? 0:1+test5(i);
		return i;
	}

	public static void main (String [] args) {
		test(20);
		test2(20);
		test3(20);
		test4(20);
		test6(20);
	}

}
