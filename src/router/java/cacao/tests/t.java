
public class t {
	public static void main(String [] s) {
		ausgeber m1 = new m(4711);
		float a=(float)7.5,b=(float)3.2;
		double ad=5.3,bd=2.1;
		
		int i; for (i=0; i<s.length; i++) m1.print (s[i]);
		
		m1.print (System.getProperty("file.separator"));
		m1.print (System.getProperty("line.separator"));
		
		m1.print ();
		System.out.println ((int) ad);
		m1.print ( (int) (a) );
		m1.print ( (int) (ad % bd) );
		m1.print (17.3);
		m1.print ((float)9.4);
		m1.print ("bye");
		}
	 }

class m implements ausgeber { 
	public int value;
	
	public m(int v) { value = v; }
	
	public void print(String s) { System.out.println(s); }
	public void print(int i) { System.out.println(i); }
	public void print() { System.out.println(value); }
	public void print(float f) { System.out.println(f); }
	public void print(double d) { System.out.println(d); }
	}
	
interface ausgeber {
	public void print(String s);
	public void print(int i);
	public void print();
	public void print(float f);
	public void print(double d);
	}

	 	