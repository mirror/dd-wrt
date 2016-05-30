class if_tests	{
	static public void main(String args[])	{
		int i = 5;
		long l = 690000000000L;
		int mi = -5;
		long ml = -690000000000L;

		if (i<10)	{
			System.out.println("OK i<10");
		} else	{
			System.out.println("FAIL i<10");
		}

		if (i>4)	{
			System.out.println("OK i>4");
		} else	{
			System.out.println("FAIL i>4");
		}

		if (i<=5)	{
			System.out.println("OK i<=5");
		} else	{
			System.out.println("FAIL i<=5");
		}
		if (i>=5)	{
			System.out.println("OK i>=5");
		} else	{
			System.out.println("FAIL i>=5");
		}

		if (l<690000000001L)	{
			System.out.println("OK l<690000000001L");
		} else	{
			System.out.println("FAIL l<690000000001L");
		}

		if (l>689999999999L)	{
			System.out.println("OK l>689999999999L");
		} else	{
			System.out.println("FAIL l>689999999999L");
		}

		if (l<=690000000000L)	{
			System.out.println("OK l<=690000000000L");
		} else	{
			System.out.println("FAIL l<=690000000000L");
		}
		if (l>=690000000000L)	{
			System.out.println("OK l>=690000000000L");
		} else	{
			System.out.println("FAIL l>=690000000000L");
		}

		// nagtive i and l

		if (mi<10)	{
			System.out.println("OK mi<10");
		} else	{
			System.out.println("FAIL mi<10");
		}

		if (mi>4)	{
			System.out.println("FAIL mi>4");
		} else	{
			System.out.println("OK mi>4");
		}

		if (mi<=5)	{
			System.out.println("OK mi<=5");
		} else	{
			System.out.println("FAIL mi<=5");
		}
		if (mi>=5)	{
			System.out.println("FAIL mi>=5");
		} else	{
			System.out.println("OK mi>=5");
		}

		if (ml<690000000001L)	{
			System.out.println("OK ml<690000000001L");
		} else	{
			System.out.println("FAIL ml<690000000001L");
		}

		if (ml>689999999999L)	{
			System.out.println("FAIL ml>689999999999L");
		} else	{
			System.out.println("OK ml>689999999999L");
		}

		if (ml<=690000000000L)	{
			System.out.println("OK ml<=690000000000L");
		} else	{
			System.out.println("FAIL ml<=690000000000L");
		}
		if (ml>=690000000000L)	{
			System.out.println("FAIL ml>=690000000000L");
		} else	{
			System.out.println("OK ml>=690000000000L");
		}


	}
};
