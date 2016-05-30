/*
 *	Tests if registers get restored correctly when exceptions are raised.
 *	This file is part of cacao.
 *	(c) Roland Lezuo, 2007
 */
public class exception_restore_registers	{
	public static void main(String[] args)	{
		int  	i1=-1, i2=-1, i3=-1, i4=-1, i5=-1, i6=-1, i7=-1, i8=-1;
		long 	l1=-1, l2=-1, l3=-1, l4=-1, l5=-1, l6=-1, l7=-1, l8=-1;
		float	f1=-1, f2=-1, f3=-1, f4=-1, f5=-1, f6=-1, f7=-1, f8=-1;
		double	d1=-1, d2=-1, d3=-1, d4=-1, d5=-1, d6=-1, d7=-1, d8=-1;

		try	{
			throw new Exception();
		} catch (Exception e)	{
			System.out.println("Integers: " + i1 + " " + i2 + " " + i3 + " " + i4 + " " + i5 + " " + i6 + " " + i7 + " " + i8);
			System.out.println("Longs:    " + l1 + " " + l2 + " " + l3 + " " + l4 + " " + l5 + " " + l6 + " " + l7 + " " + l8);
			System.out.println("Floats:   " + f1 + " " + f2 + " " + f3 + " " + f4 + " " + f5 + " " + f6 + " " + f7 + " " + f8);
			System.out.println("Doubles:  " + d1 + " " + d2 + " " + d3 + " " + d4 + " " + d5 + " " + d6 + " " + d7 + " " + d8);
		}

		try	{
			m1();
		} catch (Exception e)	{
			System.out.println("Integers: " + i1 + " " + i2 + " " + i3 + " " + i4 + " " + i5 + " " + i6 + " " + i7 + " " + i8);
			System.out.println("Longs:    " + l1 + " " + l2 + " " + l3 + " " + l4 + " " + l5 + " " + l6 + " " + l7 + " " + l8);
			System.out.println("Floats:   " + f1 + " " + f2 + " " + f3 + " " + f4 + " " + f5 + " " + f6 + " " + f7 + " " + f8);
			System.out.println("Doubles:  " + d1 + " " + d2 + " " + d3 + " " + d4 + " " + d5 + " " + d6 + " " + d7 + " " + d8);
		}

	}

	private static void m1() throws Exception	{
		int  	i1=0, i2=0, i3=0, i4=0, i5=0, i6=0, i7=0, i8=0;
		long 	l1=0, l2=0, l3=0, l4=0, l5=0, l6=0, l7=0, l8=0;
		float	f1=0, f2=0, f3=0, f4=0, f5=0, f6=0, f7=0, f8=0;
		double	d1=0, d2=0, d3=0, d4=0, d5=0, d6=0, d7=0, d8=0;

		System.out.println("Integers: " + i1 + " " + i2 + " " + i3 + " " + i4 + " " + i5 + " " + i6 + " " + i7 + " " + i8);
		System.out.println("Longs:    " + l1 + " " + l2 + " " + l3 + " " + l4 + " " + l5 + " " + l6 + " " + l7 + " " + l8);
		System.out.println("Floats:   " + f1 + " " + f2 + " " + f3 + " " + f4 + " " + f5 + " " + f6 + " " + f7 + " " + f8);
		System.out.println("Doubles:  " + d1 + " " + d2 + " " + d3 + " " + d4 + " " + d5 + " " + d6 + " " + d7 + " " + d8);

		throw new Exception();
	}
}
