public class fp {
    public static void main(String [] s) {
        p("-------------------- testfloat --------------------");
        testfloat(10.0F, 10.0F);

        p("-------------------- testdouble --------------------");
        testdouble(10.0, 10.0);
    }

    public static void testfloat(float a, float b) {
        int i;
        float x = a;

        p("---------- test fmul ----------");
        for (i = 0; i < 50; i++) {
            a *= b;
            p(a);
        }

        p("---------- test fdiv ----------");
        a = x;
        for (i = 0; i < 50; i++) {
            a /= b;
            p(a);
        }
		
        for (a = 0; a < 1; a += 0.2) {
            for (b = 0; b < 1; b += 0.2) {
                System.out.println("-----------");
                p(a);
                p(b);
                p(a + b);
                p(a - b);
                p(a * b);
                p(a / b);
            }
        }
    }
		
    public static void testdouble(double a, double b) {
        int i;
        double x = a;

        p("---------- test dmul ----------");
        for (i = 0; i < 330; i++) {
            a *= b;
            p(a);
        }

        p("---------- test ddiv ----------");
        a = x;
        for (i = 0; i < 330; i++) {
            a /= b;
            p(a);
        }
		
        for (a = 0; a < 1; a += 0.2) {
            for (b = 0; b < 1; b += 0.2) {
                System.out.println("-----------");
                p(a);
                p(b);
                p(a + b);
                p(a - b);
                p(a * b);
                p(a / b);
            }
        }
    }
		
    public static void p(String s) {
        System.out.println(s);
    }

    public static void p(double d) {
        System.out.println(d);
    }

    public static void p(float d) {
        System.out.println(d);
    }

}
