/* src/tests/native/checkjni.java - for testing JNI related stuff

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


public class checkjni {
    public static native boolean IsAssignableFrom(Class sub, Class sup);
    public static native boolean IsInstanceOf(Object obj, Class clazz);
    public static native boolean IsSameObject(Object obj1, Object obj2);
    public static native int     PushLocalFrame(int capacity);
	public static native void    Throw() throws Exception;
	public static native Class   GetObjectClass(Object obj);
	public static native String  NewString(int type);
	public static native int     GetIntField(Object obj);
	public static native Object  GetObjectField(Object obj);
	public static native int     GetStaticIntField();
	public static native Object  GetStaticObjectField();
	public static native void    SetStaticIntField(int val);
	public static native int     GetIntField();
	public static native int[]   NewIntArray(int length);
	public static native long[]  NewLongArray(int length);

	public static int     jsfI = 0x123456;
	public static Object  jsfL = new Object();

	class checkjniobj {
		public int    jfI = 0x123456;
		public Object jfL = new Object();
	}

    public static void main(String[] argv) {
        System.loadLibrary("checkjni");

        new checkjni();
    }

    public checkjni() {
        checkIsAssignableFrom();
        checkIsInstanceOf();
		checkIsSameObject();
        checkPushLocalFrame();
		checkThrow();
		checkGetObjectClass();
		checkFields();
		checkArrays();
		checkNewString();
    }

    void checkIsAssignableFrom() {
        p("IsAssignableFrom:");

        Class sub = Integer.class;
        Class sup = Object.class;

        equal(IsAssignableFrom(sup, sup), true);
        equal(IsAssignableFrom(sub, sup), true);
        equal(IsAssignableFrom(sup, sub), false);
    }

    void checkIsInstanceOf() {
        p("IsInstanceOf:");

        Object obj = new Object();
        Object obj2 = new Integer(1);
        Class clazz = Object.class;
        Class clazz2 = Integer.class;

        equal(IsInstanceOf(obj, clazz), true);
        equal(IsInstanceOf(obj2, clazz), true);
        equal(IsInstanceOf(obj, clazz2), false);
    }

    void checkIsSameObject() {
        p("IsSameObject:");

        Object obj1 = new Object();
        Object obj2 = new Integer(1);
        Class clazz = Object.class;

        equal(IsSameObject(obj1, obj1), true);
        equal(IsSameObject(clazz, clazz), true);
        equal(IsSameObject(null, null), true);
        equal(IsSameObject(obj1, obj2), false);
        equal(IsSameObject(obj1, clazz), false);
        equal(IsSameObject(obj1, null), false);
    }

    void checkPushLocalFrame() {
        p("PushLocalFrame:");

        equal(PushLocalFrame(100), 0);
    }

	void checkThrow() {
		p("Throw");
		
		try {
			Throw();
			p("FAILED, no exception thrown");
		} catch (Exception e) {
			p("PASS, " + e);
		}
	}

    void checkGetObjectClass() {
        p("GetObjectClass:");

        Object obj1 = new Object();
        Object obj2 = new Integer(1);
        Class clazz1 = Object.class;
        Class clazz2 = Integer.class;

        equal(GetObjectClass(obj1), clazz1);
        equal(GetObjectClass(obj2), clazz2);
    }

	void checkNewString() {
		p("NewString:");
		
		equal(NewString(2), "Test String from JNI with UTF");
	}

	void checkFields() {
		p("Field Access:");
		
		equal(GetStaticIntField(), jsfI);
		equal(GetStaticObjectField(), jsfL);
		
		checkjniobj o = new checkjniobj();

		equal(GetIntField(o), o.jfI);
		equal(GetObjectField(o), o.jfL);
		
		SetStaticIntField(0xABCDEF); equal(jsfI, 0xABCDEF);
	}

	void checkArrays() {
		p("Array Access:");

		int i;
		boolean result;

		int[] aI = NewIntArray(10);
		for (i = 0, result = true; i < aI.length; i++) result &= (aI[i] == i);
		if (result)
			p("PASS, size=" + aI.length);
		else
			p("FAILED");

		long[] aL = NewLongArray(20);
		for (i = 0, result = true; i < aL.length; i++) result &= (aL[i] == i);
		if (result)
			p("PASS, size=" + aL.length);
		else
			p("FAILED");
	}

    void equal(boolean a, boolean b) {
        if (a == b)
            p("PASS");
        else
            p("FAILED");
    }

    void equal(int a, int b) {
        if (a == b)
            p("PASS");
        else
            p("FAILED ("+a+"!="+b+")");
    }

    void equal(Object a, Object b) {
        if (a == b)
            p("PASS");
        else
            p("FAILED");
    }

    void equal(String a, String b) {
        if (a.equals(b))
            p("PASS");
        else
            p("FAILED ("+a+"!="+b+")");
    }

    void p(String s) {
        System.out.println(s);
    }
}
