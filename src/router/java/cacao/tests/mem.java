
public class mem {
	public int dummy;
	
	public static void main(String[] s) {
		int i;
		
		for (i=0; i<100000000; i++) {
			try {
				byte[] b = new byte[10000];
				b[0]=17;
				} 
			catch (java.lang.Throwable c) {
				System.out.print ("Out of mem after ");
				System.out.print (i);
				System.out.println (" allocations.");
				}
			finally {
				System.out.println ("OK");
				}
			}
		
		
		}
		
		
	}
	