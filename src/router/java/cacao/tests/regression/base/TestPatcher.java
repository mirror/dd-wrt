/* tests/regression/base/TestPatcher.java

   Copyright (C) 1996-2013
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


import org.junit.Test;
import static org.junit.Assert.*;

import java.io.*;

public class TestPatcher {
    static boolean doit = true;

    final static int    i = 123;
    final static long   l = 1234567890123L;
    final static float  f = 123.456F;
    final static double d = 789.012;
    final static Object o = new Object();

    @Test
    public void testNormal() {
        invokestatic();
        invokespecial();

        getstatic();
        putstatic();
        putstaticconst();

        getfield();
        putfield();
        putfieldconst();

        newarray();
        multianewarray();

        checkcast();
        _instanceof();

        aastoreconst();
    }

    public void testWithoutClasses() {
        // Delete all classes.
        //new File("TestPatcher$invokestatic.class").delete();

        invokestatic();
        invokespecial();

        getstatic();
        putstatic();
        putstaticconst();

        getfield();
        putfield();
        putfieldconst();

        newarray();
        multianewarray();

        checkcast();
        _instanceof();

        aastoreconst();
    }

    private void invokestatic() {
        try {
            if (doit)
                invokestatic.sub();
        } catch (NoClassDefFoundError e) {
            fail(e.toString());
        }
    }

    private void getstatic() {
        try {
            if (doit)
                assertTrue(getstaticI.i + " != " + i, getstaticI.i == i);
        } catch (NoClassDefFoundError e) {
            fail(e.toString());
        }

        try {
            if (doit)
                assertTrue(getstaticJ.l + " != " + l, getstaticJ.l == l);
        } catch (NoClassDefFoundError e) {
            fail(e.toString());
        }

        try {
            if (doit)
                assertTrue(getstaticF.f + " != " + f, getstaticF.f == f);
        } catch (NoClassDefFoundError e) {
            fail(e.toString());
        }

        try {
            if (doit)
                assertTrue(getstaticD.d + " != " + d, getstaticD.d == d);
        } catch (NoClassDefFoundError e) {
            fail(e.toString());
        }

        try {
            if (doit)
                assertTrue(getstaticL.o + " != null", getstaticL.o == null);
        } catch (NoClassDefFoundError e) {
            fail(e.toString());
        }
    }

    private void putstatic() {
        try {
            if (doit) {
                putstaticI.i = i;
                assertTrue(putstaticI.i + " != " + i, putstaticI.i == i);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticJ.l = l;
                assertTrue(putstaticJ.l + " != " + l, putstaticJ.l == l);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticF.f = f;
                assertTrue(putstaticF.f + " != " + f, putstaticF.f == f);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticD.d = d;
                assertTrue(putstaticD.d + " != " + d, putstaticD.d == d);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }


        try {
            if (doit) {
                putstaticL.o = o;
                assertTrue(putstaticL.o + " != " + o, putstaticL.o == o);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
    }

    private void putstaticconst() {
        try {
            if (doit) {
                putstaticconstI.i = i;
                assertTrue(putstaticconstI.i + " != " + i, putstaticconstI.i == i);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstJ.l = l;
                assertTrue(putstaticconstJ.l + " != " + l, putstaticconstJ.l == l);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstF.f = f;
                assertTrue(putstaticconstF.f + " != " + f, putstaticconstF.f == f);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstD.d = d;
                assertTrue(putstaticconstD.d + " != " + d, putstaticconstD.d == d);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstI.i = 0;
                assertTrue(putstaticconstI.i + " != " + 0, putstaticconstI.i == 0);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstJ.l = 0L;
                assertTrue(putstaticconstJ.l + " != " + 0L, putstaticconstJ.l == 0L);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstF.f = 0.0F;
                assertTrue(putstaticconstF.f + " != " + 0.0F, putstaticconstF.f == 0.0F);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstD.d = 0.0;
                assertTrue(putstaticconstD.d + " != " + 0.0, putstaticconstD.d == 0.0);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstL.o = null;
                assertTrue(putstaticconstL.o + " != " + null, putstaticconstL.o == null);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putstaticconstC.c = putstaticconstC.class;
                assertTrue(putstaticconstC.c + " != " + Class.forName("TestPatcher$putstaticconstC"), putstaticconstC.c == Class.forName("TestPatcher$putstaticconstC"));
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        } catch (ClassNotFoundException t) {
            fail(t.toString());
        }
    }

    private void getfield() {
        try {
            if (doit)
                assertTrue(new getfieldI().i + " != " + i, new getfieldI().i == i);
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit)
                assertTrue(new getfieldJ().l + " != " + l, new getfieldJ().l == l);
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit)
                assertTrue(new getfieldF().f + " != " + f, new getfieldF().f == f);
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit)
                assertTrue(new getfieldD().d + " != " + d, new getfieldD().d == d);
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit)
                assertTrue(new getfieldL().o + " != " + null, new getfieldL().o == null);
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
    }

    private void putfield() {
        try {
            if (doit) {
                TestPatcher.putfieldI pfi = new TestPatcher.putfieldI();
                pfi.i = i;
                assertTrue(pfi.i + " != " + i, pfi.i == i);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldJ pfj = new putfieldJ();
                pfj.l = l;
                assertTrue(pfj.l + " != " + l, pfj.l == l);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldF pff = new putfieldF();
                pff.f = f;
                assertTrue(pff.f + " != " + f, pff.f == f);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldD pfd = new putfieldD();
                pfd.d = d;
                assertTrue(pfd.d + " != " + d, pfd.d == d);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldL pfl = new putfieldL();
                pfl.o = o;
                assertTrue(pfl.o + " != " + o, pfl.o == o);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
    }

    private void putfieldconst() {
        try {
            if (doit) {
                putfieldconstI pfci = new putfieldconstI();
                pfci.i = i;
                assertTrue(pfci.i + " != " + i, pfci.i == i);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
 
        try {
            if (doit) {
                putfieldconstJ pfcj = new putfieldconstJ();
                pfcj.l = l;
                assertTrue(pfcj.l + " != " + l, pfcj.l == l);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldconstF pfcf = new putfieldconstF();
                pfcf.f = f;
                assertTrue(pfcf.f + " != " + f, pfcf.f == f);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
 
        try {
            if (doit) {
                putfieldconstD pfcd = new putfieldconstD();
                pfcd.d = d;
                assertTrue(pfcd.d + " != " + d, pfcd.d == d);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldconstI pfci = new putfieldconstI();
                pfci.i = 0;
                assertTrue(pfci.i + " != " + 0, pfci.i == 0);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
 
        try {
            if (doit) {
                putfieldconstJ pfcj = new putfieldconstJ();
                pfcj.l = 0L;
                assertTrue(pfcj.l + " != " + 0L, pfcj.l == 0L);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldconstF pfcf = new putfieldconstF();
                pfcf.f = 0.0F;
                assertTrue(pfcf.f + " != " + 0.0F, pfcf.f == 0.0F);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
 
        try {
            if (doit) {
                putfieldconstD pfcd = new putfieldconstD();
                pfcd.d = 0.0;
                assertTrue(pfcd.d + " != " + 0.0, pfcd.d == 0.0);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldconstL pfcl = new putfieldconstL();
                pfcl.o = null;
                assertTrue(pfcl.o + " != " + null, pfcl.o == null);
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit) {
                putfieldconstC pfcc = new putfieldconstC();
                pfcc.c = putfieldconstC.class;
                assertTrue(pfcc.c + " != " + Class.forName("TestPatcher$putfieldconstC"), pfcc.c == Class.forName("TestPatcher$putfieldconstC"));
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        } catch (ClassNotFoundException t) {
            fail(t.toString());
        }
    }

    private void newarray() {
        try {
            if (doit) {
                newarray[] na = new newarray[1];
                na[0] = null;
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
    }

    private void multianewarray() {
        try {
            if (doit) {
                multianewarray[][] ma = new multianewarray[1][1];
                ma[0][0] = null;
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
    }

    private void invokespecial() {
        try {
            if (doit)
                new invokespecial();
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
    }

    private void checkcast() {
        Object o = new Object();

        // class
        try {
            if (doit) {
                checkcastC cc = (checkcastC) o;
                fail();
            }
        } catch (ClassCastException success) {
            // This is OK.
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        // interface
        try {
            if (doit) {
                checkcastI ci = (checkcastI) o;
                fail();
            }
        } catch (ClassCastException success) {
            // This is OK.
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }


        // array
        Object[] oa = new Object[1];

        try {
            if (doit) {
                checkcastC[] cca = (checkcastC[]) oa;
                fail();
            }
        } catch (ClassCastException e) {
            // This is OK.
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
    }

    private void _instanceof() {
        Object o = new Object();

        try {
            if (doit)
                if (o instanceof instanceofC)
                    fail();
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }

        try {
            if (doit)
                if (o instanceof instanceofI)
                    fail();
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }


        // array
        Object[] oa = new Object[1];

        try {
            if (doit)
                if (oa instanceof instanceofC[])
                    fail();
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        }
    }

    private void aastoreconst() {
        Class[] ca = new Class[1];

        try {
            if (doit) {
                ca[0] = aastoreconstClass.class;

                if (ca[0] == null)
                    fail();

                assertTrue(ca[0] + " != " + Class.forName("TestPatcher$aastoreconstClass") , ca[0] == Class.forName("TestPatcher$aastoreconstClass"));
            }
        } catch (NoClassDefFoundError t) {
            fail(t.toString());
        } catch (ClassNotFoundException t) {
            fail(t.toString());
        }
    }

    static class invokestatic  { static void sub() {} }
    static class invokespecial { void invokespecial() {} }

    static class getstaticI { static int    i = TestPatcher.i; }
    static class getstaticJ { static long   l = TestPatcher.l; }
    static class getstaticF { static float  f = TestPatcher.f; }
    static class getstaticD { static double d = TestPatcher.d; }
    static class getstaticL { static Object o = null; }

    static class putstaticI { static int    i; }
    static class putstaticJ { static long   l; }
    static class putstaticF { static float  f; }
    static class putstaticD { static double d; }
    static class putstaticL { static Object o; }

    static class putstaticconstI { static int    i; }
    static class putstaticconstJ { static long   l; }
    static class putstaticconstF { static float  f; }
    static class putstaticconstD { static double d; }
    static class putstaticconstL { static Object o; }
    static class putstaticconstC { static Class<putstaticconstC> c; }

    static class getfieldI { int    i = TestPatcher.i; }
    static class getfieldJ { long   l = TestPatcher.l; }
    static class getfieldF { float  f = TestPatcher.f; }
    static class getfieldD { double d = TestPatcher.d; }
    static class getfieldL { Object o = null; }

    static class putfieldI { int    i; }
    static class putfieldJ { long   l; }
    static class putfieldF { float  f; }
    static class putfieldD { double d; }
    static class putfieldL { Object o; }

    static class putfieldconstI { int    i; }
    static class putfieldconstJ { long   l; }
    static class putfieldconstF { float  f; }
    static class putfieldconstD { double d; }
    static class putfieldconstL { Object o; }
    static class putfieldconstC { Class<putfieldconstC> c; }

    static class newarray {}
    static class multianewarray {}

    static class instanceofC {}
    static interface instanceofI {}

    static class checkcastC {}
    static interface checkcastI {}

    static class aastoreconstClass {}
}
