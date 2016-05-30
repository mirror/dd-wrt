/* tests/regressions/native/testarguments.java - tests argument passing

   Copyright (C) 1996-2012
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

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

*/


public class testarguments {
    public static native Object adr(int i);
    public static native void np(Object o);

    public static native void nisub(int a, int b, int c, int d, int e,
                                    int f, int g, int h, int i, int j,
                                    int k, int l, int m, int n, int o);

    public static native void nlsub(long a, long b, long c, long d, long e,
                                    long f, long g, long h, long i, long j,
                                    long k, long l, long m, long n, long o);

    public static native void nfsub(float a, float b, float c, float d, float e,
                                    float f, float g, float h, float i, float j,
                                    float k, float l, float m, float n, float o);

    public static native void ndsub(double a, double b, double c, double d, double e,
                                    double f, double g, double h, double i, double j,
                                    double k, double l, double m, double n, double o);

    public static native void nasub(Object a, Object b, Object c, Object d, Object e,
                                    Object f, Object g, Object h, Object i, Object j,
                                    Object k, Object l, Object m, Object n, Object o);

    public static native void nmsub(int a, long b, float c, double d,
                                    int e, long f, float g, double h,
                                    int i, long j, float k, double l,
                                    int m, long n, float o);

    public static native void nmfsub(double a, float b, double c, float d, double e, float f, double x, float y,
                                     float g, double h, int i,
                                     float j, double k, int l,
                                     float m, double n, int o,
                                     int p, long q, float r, double s,
                                     int t, long u, float v, double w);

    public static void main(String[] argv) {
        System.loadLibrary("testarguments");

        itest();
        ltest();
        ftest();
        dtest();
        atest();

        mtest();
    }

    static void itest() {
        pln("testing int --------------------------------------------------");

        isub(0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555,
             0x66666666, 0x77777777, 0x88888888, 0x99999999, 0xaaaaaaaa,
             0xbbbbbbbb, 0xcccccccc, 0xdddddddd, 0xeeeeeeee, 0xffffffff);

        pln();
    }

    static void ltest() {
        pln("testing long -------------------------------------------------");

        lsub(0x1111111111111111L, 0x2222222222222222L, 0x3333333333333333L,
             0x4444444444444444L, 0x5555555555555555L, 0x6666666666666666L,
             0x7777777777777777L, 0x8888888888888888L, 0x9999999999999999L,
             0xaaaaaaaaaaaaaaaaL, 0xbbbbbbbbbbbbbbbbL, 0xccccccccccccccccL,
             0xddddddddddddddddL, 0xeeeeeeeeeeeeeeeeL, 0xffffffffffffffffL);

        pln();
    }

    static void ftest() {
        pln("testing float ------------------------------------------------");

        fsub(i2f(0x11111111), i2f(0x22222222), i2f(0x33333333),
             i2f(0x44444444), i2f(0x55555555), i2f(0x66666666),
             i2f(0x77777777), i2f(0x88888888), i2f(0x99999999),
             i2f(0xaaaaaaaa), i2f(0xbbbbbbbb), i2f(0xcccccccc),
             i2f(0xdddddddd), i2f(0xeeeeeeee), i2f(0xffffffff));

        pln();
    }

    static void dtest() {
        pln("testing double -----------------------------------------------");

        dsub(l2d(0x1111111111111111L), l2d(0x2222222222222222L),
             l2d(0x3333333333333333L), l2d(0x4444444444444444L),
             l2d(0x5555555555555555L), l2d(0x6666666666666666L),
             l2d(0x7777777777777777L), l2d(0x8888888888888888L),
             l2d(0x9999999999999999L), l2d(0xaaaaaaaaaaaaaaaaL),
             l2d(0xbbbbbbbbbbbbbbbbL), l2d(0xccccccccccccccccL),
             l2d(0xddddddddddddddddL), l2d(0xeeeeeeeeeeeeeeeeL),
             l2d(0xffffffffffffffffL));

        pln();
    }

    static void atest() {
        pln("testing address ----------------------------------------------");

        asub(adr(1),  adr(2),  adr(3),  adr(4),  adr(5),
             adr(6),  adr(7),  adr(8),  adr(9),  adr(10),
             adr(11), adr(12), adr(13), adr(14), adr(15));

        pln();
    }

    static void mtest() {
        pln("testing mixed ------------------------------------------------");

        msub(0x11111111, 0x2222222222222222L,
             i2f(0x33333333), l2d(0x4444444444444444L),
             0x55555555, 0x6666666666666666L,
             i2f(0x77777777), l2d(0x8888888888888888L),
             0x99999999, 0xaaaaaaaaaaaaaaaaL,
             i2f(0xbbbbbbbb), l2d(0xccccccccccccccccL),
             0xdddddddd, 0xeeeeeeeeeeeeeeeeL,
             i2f(0xffffffff));

        pln("testing more mixed -------------------------------------------");

        mfsub(l2d(0x1111111111111111L), i2f(0x22222222), l2d(0x3333333333333333L), i2f(0x44444444),
              l2d(0x5555555555555555L), i2f(0x66666666), l2d(0x1122112233443344L), i2f(0x55667788),
              i2f(0x77777777), l2d(0x8888888888888888L), 0x99999999,
              i2f(0xaaaaaaaa), l2d(0xbbbbbbbbbbbbbbbbL), 0xcccccccc,
              i2f(0xdddddddd), l2d(0xeeeeeeeeeeeeeeeeL), 0xffffffff,
              0x80808080, 0x9191919191919191L, i2f(0xa2a2a2a2), l2d(0xb3b3b3b3b3b3b3b3L),
              0xc4c4c4c4, 0xd5d5d5d5d5d5d5d5L, i2f(0xe6e6e6e6), l2d(0xf7f7f7f7f7f7f7f7L));
    }


    // test java-java argument passing

    public static void isub(int a, int b, int c, int d, int e,
                            int f, int g, int h, int i, int j,
                            int k, int l, int m, int n, int o) {
        p("java-java  :");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();

        nisub(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
    }

    public static void lsub(long a, long b, long c, long d, long e,
                            long f, long g, long h, long i, long j,
                            long k, long l, long m, long n, long o) {
        p("java-java  :");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();

        nlsub(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
    }

    public static void fsub(float a, float b, float c, float d, float e,
                            float f, float g, float h, float i, float j,
                            float k, float l, float m, float n, float o) {
        p("java-java  :");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();

        nfsub(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
    }

    public static void dsub(double a, double b, double c, double d, double e,
                            double f, double g, double h, double i, double j,
                            double k, double l, double m, double n, double o) {
        p("java-java  :");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();

        ndsub(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
    }

    public static void asub(Object a, Object b, Object c, Object d, Object e,
                            Object f, Object g, Object h, Object i, Object j,
                            Object k, Object l, Object m, Object n, Object o) {
        p("java-java  :");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();

        nasub(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
    }

    public static void msub(int a, long b, float c, double d,
                            int e, long f, float g, double h,
                            int i, long j, float k, double l,
                            int m, long n, float o) {
        p("java-java  :");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();

        nmsub(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
    }

    public static void mfsub(double a, float b, double c, float d, double e, float f, double x, float y,
                             float g, double h, int i,
                             float j, double k, int l,
                             float m, double n, int o,
                             int p, long q, float r, double s,
                             int t, long u, float v, double w) {
        p("java-java  :");

        p(a); p(b); p(c); p(d); p(e); p(f); p(x); p(y);
        p(g); p(h); p(i);
        p(j); p(k); p(l);
        p(m); p(n); p(o);
        p(p); p(q); p(r); p(s);
        p(t); p(u); p(v); p(w);

        pln();

        nmfsub(a, b, c, d, e, f, x, y, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w);
    }

    // test native-java argument passing

    public static void jisub(int a, int b, int c, int d, int e,
                             int f, int g, int h, int i, int j,
                             int k, int l, int m, int n, int o) {
        p("native-java:");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();
    }

    public static void jlsub(long a, long b, long c, long d, long e,
                             long f, long g, long h, long i, long j,
                             long k, long l, long m, long n, long o) {
        p("native-java:");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();
    }

    public static void jfsub(float a, float b, float c, float d, float e,
                             float f, float g, float h, float i, float j,
                             float k, float l, float m, float n, float o) {
        p("native-java:");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();
    }

    public static void jdsub(double a, double b, double c, double d, double e,
                             double f, double g, double h, double i, double j,
                             double k, double l, double m, double n, double o) {
        p("native-java:");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();
    }

    public static void jasub(Object a, Object b, Object c, Object d, Object e,
                             Object f, Object g, Object h, Object i, Object j,
                             Object k, Object l, Object m, Object n, Object o) {
        p("native-java:");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();
    }

    public static void jmsub(int a, long b, float c, double d,
                             int e, long f, float g, double h,
                             int i, long j, float k, double l,
                             int m, long n, float o) {
        p("native-java:");

        p(a); p(b); p(c); p(d); p(e);
        p(f); p(g); p(h); p(i); p(j);
        p(k); p(l); p(m); p(n); p(o);

        pln();
    }

    public static void jmfsub(double a, float b, double c, float d, double e, float f, double x, float y,
                              float g, double h, int i,
                              float j, double k, int l,
                              float m, double n, int o,
                              int p, long q, float r, double s,
                              int t, long u, float v, double w) {
        p("native-java:");

        p(a); p(b); p(c); p(d); p(e); p(f); p(x); p(y);
        p(g); p(h); p(i);
        p(j); p(k); p(l);
        p(m); p(n); p(o);
        p(p); p(q); p(r); p(s);
        p(t); p(u); p(v); p(w);

        pln();
    }


    static float i2f(int i) {
        return Float.intBitsToFloat(i);
    }

    static double l2d(long l) {
        return Double.longBitsToDouble(l);
    }

    static void p(String s) { System.out.print(s); }

    static void p(int i) {
        System.out.print(" 0x" + Integer.toHexString(i));
    }

    static void p(long l) {
        System.out.print(" 0x" + Long.toHexString(l));
    }

    static void p(float f) {
        p(Float.floatToIntBits(f));
    }

    static void p(double d) {
        p(Double.doubleToLongBits(d));
    }

    static void p(Object o) {
        np(o);
    }

    static void pln() { System.out.println(); }
    static void pln(String s) { System.out.println(s); }
}
