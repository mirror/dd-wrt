public class fintest {
	public int num;
	
	public fintest (int n) { num = n; }

	public void finalize () {
		System.out.print ("finalized ");
		System.out.print (num);
		System.out.println (". object");
		}

	public static void main (String[] s) {
		int i;
		fintest f=null;
		
		for (i=0; i<100; i++) {
			System.out.print (i);
			System.out.println (". Objekt wird angelegt");
			f = new fintest(i);
			byte[] b = new byte[1000000];
			b[0] = 0;
			}

		Runtime.getRuntime().exit(0);
	
		}
		
	}

	