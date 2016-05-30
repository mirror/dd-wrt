class checkcast	{
	public static void main(String[] args)	{
		Object o1 = new String();
		Object o2 = new Integer(1);
		String s1 = (String)o1;
		String s2 = (String)o2;
	}
}
