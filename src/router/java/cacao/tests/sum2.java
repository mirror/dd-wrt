
// Zahlen summieren, Java Version

public class sum2 {
	
			
	static public void main(String [] arg) {

		int s1 = 0, s2 = 0, i = 0;
		int inc = 1;
		int n = Integer.parseInt (arg[0]);

		
		for (i = n; i > 0; i -= inc) {
			s1 += inc;
			s2 -= inc;
			}
		
		System.out.print (".... done, sum: ");
		System.out.println (s1);
		System.out.println (s2);
		}
		
		
	}
	