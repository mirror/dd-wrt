/* Copyright (C) 2005, Joseph Wenninger <jowenn@kde.org> 
GNU General Public License
*/


public class cond1 {

	private static int test(int i) {
		return i==0 ? 0: 1<< (i % 10);
	}

	public static void main (String [] args) {
		test(20);
	}

}
