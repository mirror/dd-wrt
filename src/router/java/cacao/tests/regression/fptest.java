/* tests/fptest.java - checks most of the floating point instructions

   Copyright (C) 1996-2005, 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   TU Wien

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Contact: cacao@cacaojvm.org

   Authors: Andreas Krall

   Changes: Christian Thalinger

*/

public class fptest {
    public static void main(String [] s) {
        float  fnan  = Float.NaN;
        float  fpinf = Float.POSITIVE_INFINITY;
        float  fninf = Float.NEGATIVE_INFINITY;
        float  fmax  = Float.MAX_VALUE;
        float  fmin  = Float.MIN_VALUE;
        float  f1    = 0F;
        float  f2    = 0F;

        double dnan  = Double.NaN;
        double dpinf = Double.POSITIVE_INFINITY;
        double dninf = Double.NEGATIVE_INFINITY;
        double dmax  = Double.MAX_VALUE;
        double dmin  = Double.MIN_VALUE;
        double d1    = 0D;
        double d2    = 0D;
		
        p("---------------------------- tests NaNs and Infs -------------------");
        p("------------------- print NaNs and Infs");

        p("NaNQ ", fnan);
        p("+INF ", fpinf);
        p("-INF ", fninf);

        p("NaNQ ", dnan);
        p("+INF ", dpinf);
        p("-INF ", dninf);

        p("------------------- test zero division");

        zerodiv("0 / 0 = NaNQ ",  0F, f1);
        zerodiv("+ / 0 = +INF ",  5F, f1);
        zerodiv("- / 0 = -INF ", -5F, f1);

        zerodiv("0 / 0 = NaNQ ",  0D, d1);
        zerodiv("+ / 0 = +INF ",  5D, d1);
        zerodiv("- / 0 = -INF ", -5D, d1);

        p("------------------- test conversions");
        testfcvt("NaNQ", fnan, dnan);
        testfcvt("+INF", fpinf, dpinf);
        testfcvt("-INF", fninf, dninf);
        testfcvt(" MAX",  fmax, dmax);
        testfcvt(" MIN",  fmin, dmin);
        testfcvt("MAXINT-1",  2147483646.0F, 2147483646.0D);
        testfcvt("MAXINT+0",  2147483647.0F, 2147483647.0D);
        testfcvt("MAXINT+1",  2147483648.0F, 2147483648.0D);
        testfcvt("-MAXINT+1",  -2147483647.0F, -2147483647.0D);
        testfcvt("-MAXINT+0",  -2147483648.0F, -2147483648.0D);
        testfcvt("-MAXINT-1",  -2147483649.0F, -2147483649.0D);
        testfcvt("MAXLNG-1",  9223372036854775806.0F, 9223372036854775806.0D);
        testfcvt("MAXLNG+0",  9223372036854775807.0F, 9223372036854775807.0D);
        testfcvt("MAXLNG+1",  9223372036854775808.0F, 9223372036854775808.0D);
        testfcvt("-MAXLNG+1",  -9223372036854775807.0F, -9223372036854775807.0D);
        testfcvt("-MAXLNG+0",  -9223372036854775808.0F, -9223372036854775808.0D);
        testfcvt("-MAXLNG-1",  -9223372036854775809.0F, -9223372036854775809.0D);

        p("------------------- test NaNQ op value");
        testfops("NaNQ", "-5.0", fnan, -5F, dnan, -5D);
        testfcmp("NaNQ", "-5.0", fnan, -5F, dnan, -5D);
        testfops("NaNQ", "-0.0", fnan, -0F, dnan, -0D);
        testfcmp("NaNQ", "-0.0", fnan, -0F, dnan, -0D);
        testfops("NaNQ", "0.0", fnan, 0F, dnan, 0D);
        testfcmp("NaNQ", "0.0", fnan, 0F, dnan, 0D);
        testfops("NaNQ", "5.0", fnan, 5F, dnan, 5D);
        testfcmp("NaNQ", "5.0", fnan, 5F, dnan, 5D);

        p("------------------- test value op NaNQ");
        testfops("-5.0", "NaNQ", -5F, fnan, -5D, dnan);
        testfcmp("-5.0", "NaNQ", -5F, fnan, -5D, dnan);
        testfops("-0.0", "NaNQ", -0F, fnan, -0D, dnan);
        testfcmp("-0.0", "NaNQ", -0F, fnan, -0D, dnan);
        testfops("0.0", "NaNQ", 0F, fnan, 0D, dnan);
        testfcmp("0.0", "NaNQ", 0F, fnan, 0D, dnan);
        testfops("5.0", "NaNQ", 5F, fnan, 5D, dnan);
        testfcmp("5.0", "NaNQ", 5F, fnan, 5D, dnan);

        p("------------------- test +INF op value");
        testfops("+INF", "-5.0", fpinf, -5F, dpinf, -5D);
        testfcmp("+INF", "-5.0", fpinf, -5F, dpinf, -5D);
        testfops("+INF", "-0.0", fpinf, -0F, dpinf, -0D);
        testfcmp("+INF", "-0.0", fpinf, -0F, dpinf, -0D);
        testfops("+INF", "0.0", fpinf, 0F, dpinf, 0D);
        testfcmp("+INF", "0.0", fpinf, 0F, dpinf, 0D);
        testfops("+INF", "5.0", fpinf, 5F, dpinf, 5D);
        testfcmp("+INF", "5.0", fpinf, 5F, dpinf, 5D);

        p("------------------- test +INF op value");
        testfops("-5.0", "+INF", -5F, fpinf, -5D, dpinf);
        testfcmp("-5.0", "+INF", -5F, fpinf, -5D, dpinf);
        testfops("-0.0", "+INF", -0F, fpinf, -0D, dpinf);
        testfcmp("-0.0", "+INF", -0F, fpinf, -0D, dpinf);
        testfops("0.0", "+INF", 0F, fpinf, 0D, dpinf);
        testfcmp("0.0", "+INF", 0F, fpinf, 0D, dpinf);
        testfops("5.0", "+INF", 5F, fpinf, 5D, dpinf);
        testfcmp("5.0", "+INF", 5F, fpinf, 5D, dpinf);

        p("------------------- test -INF op value");
        testfops("-INF", "-5.0", fninf, -5F, dninf, -5D);
        testfcmp("-INF", "-5.0", fninf, -5F, dninf, -5D);
        testfops("-INF", "-0.0", fninf, -0F, dninf, -0D);
        testfcmp("-INF", "-0.0", fninf, -0F, dninf, -0D);
        testfops("-INF", "0.0", fninf, 0F, dninf, 0D);
        testfcmp("-INF", "0.0", fninf, 0F, dninf, 0D);
        testfops("-INF", "5.0", fninf, 5F, dninf, 5D);
        testfcmp("-INF", "5.0", fninf, 5F, dninf, 5D);

        p("------------------- test -INF op value");
        testfops("-5.0", "-INF", -5F, fninf, -5D, dninf);
        testfcmp("-5.0", "-INF", -5F, fninf, -5D, dninf);
        testfops("-0.0", "-INF", -0F, fninf, -0D, dninf);
        testfcmp("-0.0", "-INF", -0F, fninf, -0D, dninf);
        testfops("0.0", "-INF", 0F, fninf, 0D, dninf);
        testfcmp("0.0", "-INF", 0F, fninf, 0D, dninf);
        testfops("5.0", "-INF", 5F, fninf, 5D, dninf);
        testfcmp("5.0", "-INF", 5F, fninf, 5D, dninf);

        p("------------------- test MAX op value");
        testfops("MAX", "5.0", fmax, 5F, dmax, 5D);

        p("------------------- test value op MAX");
        testfops("5.0", "MAX", 5F, fmax, 5D, dmax);

        p("------------------- test MIN op value");
        testfops("MIN", "5.0", fmin, 5F, dmin, 5D);

        p("------------------- test value op MIN");
        testfops("5.0", "MIN", 5F, fmin, 5D, dmin);

    }
		
    public static void zerodiv(String s, float f1, float f2) {
        p(s, f1 / f2);
    }

    public static void zerodiv(String s, double d1, double d2) {
        p(s, d1 / d2);
    }

    public static void testfcvt(String s1, float f1, double d1) {
        p("convert " + s1 + " (" + f1 + "," + d1 + ") to ", (int)  f1);
        p("convert " + s1 + " (" + f1 + "," + d1 + ") to ", (int)  d1);
        p("convert " + s1 + " (" + f1 + "," + d1 + ") to ", (long) f1);
        p("convert " + s1 + " (" + f1 + "," + d1 + ") to ", (long) d1);
    }

    public static void testfops(String s1, String s2, float f1, float f2,
                                double d1, double d2) {
        p(s1 + " + " + s2 + " = ", f1 + f2);
        p(s1 + " - " + s2 + " = ", f1 - f2);
        p(s1 + " * " + s2 + " = ", f1 * f2);
        p(s1 + " / " + s2 + " = ", f1 / f2);
        p(s1 + " % " + s2 + " = ", f1 % f2);
        p(s1 + " + " + s2 + " = ", d1 + d2);
        p(s1 + " - " + s2 + " = ", d1 - d2);
        p(s1 + " * " + s2 + " = ", d1 * d2);
        p(s1 + " / " + s2 + " = ", d1 / d2);
        p(s1 + " % " + s2 + " = ", d1 % d2);
    }

    public static void testfcmp(String s1, String s2, float f1, float f2,
                                double d1, double d2) {
        if ( (f1 == f2)) p(" (" + s1 + " == " + s2 + ") = float: true");
        else             p(" (" + s1 + " == " + s2 + ") = float: false");
        if ( (f1 != f2)) p(" (" + s1 + " != " + s2 + ") = float: true");
        else             p(" (" + s1 + " != " + s2 + ") = float: false");
        if ( (f1 <  f2)) p(" (" + s1 + " <  " + s2 + ") = float: true");
        else             p(" (" + s1 + " <  " + s2 + ") = float: false");
        if ( (f1 <= f2)) p(" (" + s1 + " <= " + s2 + ") = float: true");
        else             p(" (" + s1 + " <= " + s2 + ") = float: false");
        if ( (f1 >  f2)) p(" (" + s1 + " >  " + s2 + ") = float: true");
        else             p(" (" + s1 + " >  " + s2 + ") = float: false");
        if ( (f1 >= f2)) p(" (" + s1 + " >= " + s2 + ") = float: true");
        else             p(" (" + s1 + " >= " + s2 + ") = float: false");

        if (!(f1 == f2)) p("!(" + s1 + " == " + s2 + ") = float: true");
        else             p("!(" + s1 + " == " + s2 + ") = float: false");
        if (!(f1 != f2)) p("!(" + s1 + " != " + s2 + ") = float: true");
        else             p("!(" + s1 + " != " + s2 + ") = float: false");
        if (!(f1 <  f2)) p("!(" + s1 + " <  " + s2 + ") = float: true");
        else             p("!(" + s1 + " <  " + s2 + ") = float: false");
        if (!(f1 <= f2)) p("!(" + s1 + " <= " + s2 + ") = float: true");
        else             p("!(" + s1 + " <= " + s2 + ") = float: false");
        if (!(f1 >  f2)) p("!(" + s1 + " >  " + s2 + ") = float: true");
        else             p("!(" + s1 + " >  " + s2 + ") = float: false");
        if (!(f1 >= f2)) p("!(" + s1 + " >= " + s2 + ") = float: true");
        else             p("!(" + s1 + " >= " + s2 + ") = float: false");

        if ( (d1 == d2)) p(" (" + s1 + " == " + s2 + ") = double: true");
        else             p(" (" + s1 + " == " + s2 + ") = double: false");
        if ( (d1 != d2)) p(" (" + s1 + " != " + s2 + ") = double: true");
        else             p(" (" + s1 + " != " + s2 + ") = double: false");
        if ( (d1 <  d2)) p(" (" + s1 + " <  " + s2 + ") = double: true");
        else             p(" (" + s1 + " <  " + s2 + ") = double: false");
        if ( (d1 <= d2)) p(" (" + s1 + " <= " + s2 + ") = double: true");
        else             p(" (" + s1 + " <= " + s2 + ") = double: false");
        if ( (d1 >  d2)) p(" (" + s1 + " >  " + s2 + ") = double: true");
        else             p(" (" + s1 + " >  " + s2 + ") = double: false");
        if ( (d1 >= d2)) p(" (" + s1 + " >= " + s2 + ") = double: true");
        else             p(" (" + s1 + " >= " + s2 + ") = double: false");

        if (!(d1 == d2)) p("!(" + s1 + " == " + s2 + ") = double: true");
        else             p("!(" + s1 + " == " + s2 + ") = double: false");
        if (!(d1 != d2)) p("!(" + s1 + " != " + s2 + ") = double: true");
        else             p("!(" + s1 + " != " + s2 + ") = double: false");
        if (!(d1 <  d2)) p("!(" + s1 + " <  " + s2 + ") = double: true");
        else             p("!(" + s1 + " <  " + s2 + ") = double: false");
        if (!(d1 <= d2)) p("!(" + s1 + " <= " + s2 + ") = double: true");
        else             p("!(" + s1 + " <= " + s2 + ") = double: false");
        if (!(d1 >  d2)) p("!(" + s1 + " >  " + s2 + ") = double: true");
        else             p("!(" + s1 + " >  " + s2 + ") = double: false");
        if (!(d1 >= d2)) p("!(" + s1 + " >= " + s2 + ") = double: true");
        else             p("!(" + s1 + " >= " + s2 + ") = double: false");
    }

    // ********************* output methods ****************************

    public static int linenum = 0;

    public static void pnl() {
        int i;

        System.out.println();
        for (i = 4 - Integer.toString(linenum).length(); i > 0; i--)
            System.out.print(' ');
        System.out.print(linenum);
        System.out.print(".    ");
        linenum++;
    }

    public static void p(String a) {
        System.out.print(a); pnl();
    }
    public static void p(boolean a) {
        System.out.print(a); pnl();
    }
    public static void p(int a) {
        System.out.print("int:    "); System.out.print(a); pnl();
    }
    public static void p(long a) {
        System.out.print("long:   "); System.out.print(a); pnl();
    }
    public static void p(short a) {
        System.out.print("short:  "); System.out.print(a); pnl();
    }
    public static void p(byte a) {
        System.out.print("byte:   "); System.out.print(a); pnl();
    }
    public static void p(char a) {
        System.out.print("char:   "); System.out.print((int)a); pnl();
    }
    public static void p(float a) {
        System.out.print("float:  "); System.out.print(a); pnl();
    }
    public static void p(double a) {
        System.out.print("double: "); System.out.print(a); pnl();
    }

    public static void p(String s, boolean i) { 
        System.out.print(s); p(i);
    }
    public static void p(String s, int i) { 
        System.out.print(s); p(i);
    }
    public static void p(String s, byte i) { 
        System.out.print(s); p(i);
    }
    public static void p(String s, char i) { 
        System.out.print(s); p(i);
    }
    public static void p(String s, short i) { 
        System.out.print(s); p(i);
    }
    public static void p(String s, long l) { 
        System.out.print(s); p(l);
    }
    public static void p(String s, float f) { 
        System.out.print(s); p(f);
    }
    public static void p(String s, double d) {
        System.out.print(s); p(d);
    }

}
