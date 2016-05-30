/* tests/regression/base/TestArrayClasses.java

   Copyright (C) 2008
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

import java.io.Serializable;

public class TestArrayClasses {
  public static class Foo {
  }

  public static class FooChild extends Foo {
  }
  
  private void doStore(Object[] array, Object obj, boolean oktostore, String msg) {
    try {
      array[0] = obj;
      assertTrue(msg, oktostore);
    }
    catch (ArrayStoreException x) {
      assertFalse(msg, oktostore);
    }
  }

  @Test
  public void testClone() {
    int[] ia1 = new int[100];
    Integer[] Ia1 = new Integer[ia1.length];
    int i;
    
    for (i=0; i<ia1.length; ++i) {
      ia1[i] = i*i;
      Ia1[i] = new Integer(ia1[i]);
    }

    int[] ia2 = (int[]) ia1.clone();

    assertEquals("cloned int array length", ia1.length, ia2.length);

    boolean eq = true;
    for (i=0; i<ia1.length; ++i) {
      if (ia2[i] != ia1[i])
        eq = false;
      //      System.out.println(ia2[i]);
    }
    assertTrue("cloned int array data", eq);

    Integer[] Ia2 = (Integer[]) Ia1.clone();
    
    assertEquals("cloned Integer array length", Ia1.length, Ia2.length);
    
    eq = true;
    for (i=0; i<ia1.length; ++i) {
      if (Ia2[i].intValue() != ia1[i])
        eq = false;
      //      System.out.println(ia2[i]);
    }
    assertTrue("cloned Integer array data", eq);
  }

  @Test
  public void testArraycopy() {
    int len;
    int i;

    long[] la1 = new long[1024];
    long[] la2 = (long[]) la1.clone();
    int size = la1.length;
    Long[] La1 = new Long[size];
    Long[] La2 = (Long[]) La1.clone();
    Number[] Na2 = new Number[size];

    for (i=0; i<la1.length; ++i) {
      la1[i] = i*i;
      La1[i] = new Long(la1[i]);
    }

    i = size;
    while (i>1) {
      if ((i & 1) != 0)
        System.err.println("ERROR: arracopy test only works for powers of two");
      i >>= 1;
    }
    
    // reverse array
    len = size / 2;
    while (len>0) {
      for (int j=0;j<(size/2)/len;++j) {
        //        System.err.println(len);
        //        System.err.println(j);
        System.arraycopy(la1,(2*j+1)*len,la2,2*j*len,len);
        System.arraycopy(la1,2*j*len,la1,(2*j+1)*len,len);
        System.arraycopy(la2,2*j*len,la1,2*j*len,len);
      }
      len /= 2;
    }

    boolean eq = true;
    for (i=0; i<size; ++i) {
      if (la1[i] != (size-i-1)*(size-i-1))
        eq = false;
      //      System.out.println(la1[i]);
    }
    assertTrue("arraycopy primitive", eq);
    
    // reverse array
    len = size / 2;
    while (len>0) {
      for (int j=0;j<(size/2)/len;++j) {
        //        System.err.println(len);
        //        System.err.println(j);
        System.arraycopy(La1,(2*j+1)*len,La2,2*j*len,len);
        System.arraycopy(La1,2*j*len,La1,(2*j+1)*len,len);
        System.arraycopy(La2,2*j*len,La1,2*j*len,len);
      }
      len /= 2;
    }

    eq = true;
    for (i=0; i<size; ++i) {
      if (La1[i].intValue() != (size-i-1)*(size-i-1))
        eq = false;
    }
    assertTrue("arraycopy ref", eq);
    
    // reverse array
    len = size / 2;
    while (len>0) {
      for (int j=0;j<(size/2)/len;++j) {
        //        System.err.println(len);
        //        System.err.println(j);
        System.arraycopy(La1,(2*j+1)*len,Na2,2*j*len,len);
        System.arraycopy(La1,2*j*len,La1,(2*j+1)*len,len);
        System.arraycopy(Na2,2*j*len,La1,2*j*len,len);
      }
      len /= 2;
    }

    eq = true;
    for (i=0; i<size; ++i) {
      if (La1[i].intValue() != i*i)
        eq = false;
    }
    assertTrue("arraycopy ref different classes", eq);

    Integer[] Ia = new Integer[size];

    try {
      System.arraycopy(Ia,0,Na2,0,1);
      assertTrue("arraycopy Integer to Number", true);
    }
    catch (ArrayStoreException x) {
      fail("arraycopy Integer to Number");
    }

    try {
      System.arraycopy(Na2,1,Ia,1,1);
      fail("!arraycopy Number to Integer");
    }
    catch (ArrayStoreException x) {
      assertTrue("!arraycopy Number to Integer", true);
    }
  }

  @Test
  public void testMain() {
    int[] ia = new int[5];
    Object[] oa = new Object[2];
    Object[][] oaa = new Object[1][1];
    String[] sa = new String[3];
    String[][] saa = new String[1][1];
    String[][] saa2 = new String[2][];
    String[][][] saaa = new String[1][2][3];
    Object o = new Object();
    java.io.Serializable[] sera = new java.io.Serializable[1];
    Cloneable[] cloa = new Cloneable[1];
    StringBuffer[][] sbaa = new StringBuffer[1][1];
    Foo[] fooa = new Foo[1];
    FooChild[] fooca = new FooChild[1];

    Class[] ifs = String[].class.getInterfaces();
    assertEquals("String[] implements 2 interfaces", 2, ifs.length);
    assertTrue("String[] implements Cloneable", ifs[0] == java.lang.Cloneable.class || ifs[1] == java.lang.Cloneable.class);
    assertTrue("String[] implements Serializable", ifs[0] == java.io.Serializable.class || ifs[1] == java.io.Serializable.class);

    assertEquals("String[] is public final abstract", 1041, String[].class.getModifiers());

    assertEquals("classname ref", "[Ljava.lang.Object;", oa.getClass().getName());
    assertEquals("classname primitive", "[I", ia.getClass().getName());
    assertEquals("arraylength primitive", 5, ia.length);
    assertEquals("arraylength ref", 2, oa.length);

    assertEquals("arraylength of saa2", 2, saa2.length);
    saa2[1] = new String[4];
    assertEquals("arraylength of saa2[1]", 4, saa2[1].length);

    assertEquals("arraylength of saaa", 1, saaa.length);
    assertEquals("arraylength of saaa[0]", 2, saaa[0].length);
    assertEquals("arraylength of saaa[0][1]", 3, saaa[0][1].length);

    assertTrue("Object[].isArray", oa.getClass().isArray());
    assertTrue("int[].isArray", ia.getClass().isArray());
    assertFalse("!Object.isArray", o.getClass().isArray());
    assertFalse("!Object.isPrimitive", o.getClass().isPrimitive());

    assertEquals("component ref", "java.lang.Object", oa.getClass().getComponentType().getName());
    assertFalse("component ref !isPrimitive", oa.getClass().getComponentType().isPrimitive());
    assertEquals("component primitive", "int", ia.getClass().getComponentType().getName());
    assertTrue("component primitive isPrimitive", ia.getClass().getComponentType().isPrimitive());

    assertTrue("component of String[][] equals String[]", saa.getClass().getComponentType().equals(sa.getClass()));
    assertFalse("component of String[][] !equals Object[]", saa.getClass().getComponentType().equals(oa.getClass()));

    assertTrue("saa[0].getClass equals component of String[][]", saa[0].getClass().equals(saa.getClass().getComponentType()));

    doStore(sa, new Object(), false, "!store Object in String[]");
    doStore(sa, new String("test"), true, "store String in String[]");
    doStore(oa, new Object(), true, "store Object in Object[]");
    doStore(oa, new String("test"), true, "store String in Object[]");

    doStore(oaa, sa, true, "store String[] in Object[][]");
    doStore(saa, oa, false, "!store Object[] in String[][]");

    doStore(sera, sa, true, "store String[] in java.io.Serializable[]");
    doStore(cloa, sa, true, "store String[] in Cloneable[]");
    
    doStore(sbaa, sa, false, "!store String[] in StringBuffer[][]");

    doStore(fooa, new Foo(), true, "store Foo in Foo[]");
    doStore(fooa, new FooChild(), true, "store FooChild in Foo[]");
    doStore(fooca, new Foo(), false, "!store Foo in FooChild[]");
    doStore(fooca, new FooChild(), true, "store FooChild in FooChild[]");
    
    try {
      Object[] oa2 = (Object[]) sa;
      assertTrue("cast String[] to Object[]", true);
    }
    catch (ClassCastException x) {
      fail("cast String[] to Object[]");
    }

    try {
      String[] sa2 = (String[]) oa;
      fail("!cast Object[] to String[]");
    }
    catch (ClassCastException x) {
      assertTrue("!cast Object[] to String[]", true);
    }

    assertTrue("String[] instanceof String[]", sa instanceof String[]);
    assertTrue("String[] instanceof Object[]", sa instanceof Object[]);
    assertFalse("Object[] !instanceof String[]", oa instanceof String[]);
    assertTrue("Object[] instanceof Object[]", oa instanceof Object[]);

    assertTrue("Object[][] instanceof Object[]", oaa instanceof Object[]);
    assertTrue("String[][] instanceof Object[]", saa instanceof Object[]);

    assertTrue("String[] instanceof java.io.Serializable", sa instanceof java.io.Serializable);
    assertTrue("String[] instanceof java.lang.Cloneable", sa instanceof java.lang.Cloneable);
    assertTrue("String[] instanceof java.lang.Object", sa instanceof java.lang.Object);
    assertTrue("saa[0] instanceof java.io.Serializable", saa[0] instanceof java.io.Serializable);
    assertTrue("saa[0] instanceof java.lang.Cloneable", saa[0] instanceof java.lang.Cloneable);
    assertTrue("saa[0] instanceof java.lang.Object", saa[0] instanceof java.lang.Object);
  }
}
