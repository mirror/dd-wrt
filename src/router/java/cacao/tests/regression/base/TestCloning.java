/* tests/regression/base/TestCloning.java

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

public class TestCloning
{
	private static class FooUncloneable {
		public Object clone() throws CloneNotSupportedException {
			return super.clone();
		}
	}

	private static class FooCloneable implements Cloneable {
		public String s;
		public FooCloneable(String s) {
			this.s = s;
		}
		public Object clone() throws CloneNotSupportedException {
			return super.clone();
		}
	}

	private static class FooOverridden {
		public Object clone() {
			return this;
		}
	}

	@Test
	public void testObject() throws CloneNotSupportedException {
		// Test cloning of cloneable object.
		FooCloneable o1 = new FooCloneable("Simple Test");
		Object oc1 = o1.clone();
		assertNotNull(oc1);
		assertNotSame(o1, oc1);
		assertEquals(o1.getClass(), oc1.getClass());
		assertSame(o1.s, ((FooCloneable) oc1).s);

		// Test cloning of uncloneable object.
		try {
			FooUncloneable o2 = new FooUncloneable();
			Object oc2 = o2.clone();
			fail("Exception expected");
		} catch (CloneNotSupportedException e) {
		}

		// Test cloning of object with overridden clone method.
		FooOverridden o3 = new FooOverridden();
		Object oc3 = o3.clone();
		assertNotNull(oc3);
		assertSame(o3, oc3);
	}

	@Test
	public void testArray() {
		// Test cloning of integer array.
		int[] a = { 1, 23, 42 };
		int[] ac = a.clone();
		assertNotNull(ac);
		assertNotSame(a, ac);
		assertEquals(a.getClass(), ac.getClass());
		assertEquals(a.length, ac.length);
		assertArrayEquals(a, ac);
	}

	@Test
	public void testLocked() throws CloneNotSupportedException {
		// Test cloning of locked object.
		FooCloneable o = new FooCloneable("Locked Test");
		synchronized(o) {
			Object oc = o.clone();
			assertNotNull(oc);
			assertTrue(Thread.holdsLock(o));
			assertFalse(Thread.holdsLock(oc));
		}
	}
}
