
// Zahlen summieren, Java Version

public class sum {
	
			
	static public void main(String [] arg) {

		int s = 0, i = 0;
		int inc = 1;
		int n = Integer.parseInt (arg[0]);

		
		for (i=0; i<n; i+=inc) s+=inc;
		
		System.out.print (".... done, sum: ");
		System.out.println (s);
		}
		
		
	}
	