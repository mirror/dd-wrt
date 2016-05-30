public class memtest {
	public memtest next;
	
	public static memtest t;

	public memtest (memtest n) { next = n; }
	public memtest () { };

	public static void main (String[] s) {
		int i;

		t = new memtest(new memtest(new memtest()));
		char c[][] = new char[100][10];
		
		for (i=0; i<100; i++) {
			System.out.print (i);
			System.out.println (". Objekt angelegt");
			memtest m = new memtest();
			}	
	
		}

	}

	