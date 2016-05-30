public class fintest3a {
	public int num;
	
	public fintest3a (int n) { num = n; }

	public void finalize () {
		System.out.print ("finalized ");
		System.out.print (num);
		System.out.println (". object");
		}

	public static void main (String[] s) {
		int i;
		fintest3a f=null;

		for (i=0; i<100; i++) {
			System.out.print (i);
			System.out.println (". Objekt wird angelegt");
			f = new fintest3a(i);
			}

	
		}
		
	}

	
