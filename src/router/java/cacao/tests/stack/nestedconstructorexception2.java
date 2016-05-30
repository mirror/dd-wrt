class nestedconstructorexception2_1 {
		nestedconstructorexception2_1() {
			String s=null;
			s.length();
		}
}

public class nestedconstructorexception2 {

	public nestedconstructorexception2(int val) throws Throwable {
		try {
			switch (val) {
				case 1:
					nestedconstructorexception2 y=new nestedconstructorexception2(2);
					break;
				case 2:
					nestedconstructorexception2 y1=new nestedconstructorexception2(3);
					break;
				case 3:
					Object o=new nestedconstructorexception2_1();
					break;
				default:

			}
		} catch (Throwable t) {
			System.out.println("Something caught in constructor");
			throw t;
		}
	}


	public static void main (String args[]) {
		try {
			nestedconstructorexception2 x=new nestedconstructorexception2(1);
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}
}
