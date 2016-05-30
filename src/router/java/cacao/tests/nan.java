public class nan
{
	public static void main(String[] s) {
		double a,b,c;
		a = -1;
		b = 0;
		c = a/b;

		System.out.println (Long.toString(Double.doubleToLongBits(c)>>>4, 16) );
		}
	}
