public class InlineTableSwitchTest {

	private static void testit(int x) {
		char y;
		if (Math.min(x,5)==x) {
			System.out.println("<=5");
			y='5';
		}
		else y=(x==6)?'6':'N';
		System.out.println(y);

		switch (x) {
			case 3:
			case 4:
				System.out.println("---------");
			default:
				System.out.println("xxxxxxxxx");	
		}
	}

	public static void main(String args[]) {
		for (int i=0;i<10;i++)
			switch (i) {
				case 0:
					Math.min(0,1);
					break;
				case 1:
					Math.min(1,1);
					break;
				case 2:
					System.out.println("-2-\n");
					break;
				default:
					Math.min(i,1);
					System.out.println(i);
					testit(i);
					break;
			}
	}
}
