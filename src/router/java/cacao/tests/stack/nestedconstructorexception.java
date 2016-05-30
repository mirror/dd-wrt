public class nestedconstructorexception {

	public nestedconstructorexception(int val) throws Throwable {
		switch (val) {
			case 1:
				nestedconstructorexception y=new nestedconstructorexception(2);
				break;
			case 2:
				nestedconstructorexception y1=new nestedconstructorexception(3);
				break;
			case 3:
				String a[]=new String[10];
				Object o=new nestedconstructorexception(5);
				a[0]=(String)o;
				break;
			default:

		}
	}


	public static void main (String args[]) {
		try {
			nestedconstructorexception x=new nestedconstructorexception(1);
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}
}
