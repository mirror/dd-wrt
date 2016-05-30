
public class main {

	public static void main(String [] s) {
		System.out.println ("Ausgabe startet");
		int i;
		
		float fa[] = new float[5];
		for (i=0; i<5; i++) fa[i]=47.20F + (float)i;
		for (i=0; i<5; i++) System.out.println(fa[i]);
		
		double da[] = new double[5];
		for (i=0; i<5; i++) da[i]=99.40F + (double)i;
		for (i=0; i<5; i++) System.out.println(da[i]);
		
		long la[] = new long[5];
		for (i=0; i<5; i++) la[i]=11 + i;
		for (i=0; i<5; i++) System.out.println(la[i]);
		
		int ia[] = new int[5];
		for (i=0; i<5; i++) ia[i]=99 + i;
		for (i=0; i<5; i++) System.out.println(ia[i]);

		short sa[] = new short[5];
		for (i=0; i<5; i++) sa[i]=(short)(77 + i);
		for (i=0; i<5; i++) System.out.println(sa[i]);

		byte ba[] = new byte[5];
		for (i=0; i<5; i++) ba[i]=(byte)(66 + i);
		for (i=0; i<5; i++) System.out.println(ba[i]);
		
		boolean boa[] = new boolean[5];
		for (i=0; i<5; i++) boa[i]=(i<2);
		for (i=0; i<5; i++) System.out.println(boa[i]);
		
		char ca[] = new char[5];
		for (i=0; i<5; i++) ca[i]=(char)('A' + i);
		for (i=0; i<5; i++) System.out.println(ca[i]);
		
		
		}
	 }

	 	