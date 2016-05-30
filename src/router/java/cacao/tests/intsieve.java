
// Primzahlen sieben,  Java-Version

public class intsieve {
	
    static void sievenumber(int n, int[] no_prime, int p) {
	int i;
	for (i = p*2; i <= n; i += p)
	    no_prime[i] = 1;
    }
				
    static void sieving(int n, int[] no_prime) {
	int p;
	for (p = 2; p <= n; p++) {
	    if (no_prime[p] == 0) sievenumber(n, no_prime, p);
	}
    }
			
    static public void main(String [] s) {
	int count=0;
	int p;
		
	int n = Integer.parseInt (s[0]);
	int times = Integer.parseInt (s[1]);
				
	int no_prime[] = new int[n+1];
	
	System.out.print ("Start sieving primes from 2 to ");
	System.out.print (n);
	System.out.print (" for ");
	System.out.print (times);
	System.out.println (" times");
		
	for (; times > 0; times--) {	
	    for (p = 0; p < n+1; p++)
		no_prime[p] = 0;
			
	    sieving(n, no_prime);
		
	    count = 0;
	    for (p = 2; p <= n; p++)
		if (no_prime[p] == 0) {
		    count++;
		}
	}
					
	System.out.print (".... done, number of primes: ");
	System.out.println (count);
    }
}
