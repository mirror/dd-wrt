/* tests/regression/base/TestAbstractMethodError.java

   Copyright (C) 2009
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

public class TestAbstractMethodError extends ClassLoader {
    @Test
    public void test() throws InstantiationException, IllegalAccessException {
        Class cls = super.defineClass(null, bytecode, 0, bytecode.length);
        Base obj = (Base) cls.newInstance();
        try {
            obj.foo();
            fail();
        } catch (AbstractMethodError e) {
            StackTraceElement[] st = e.getStackTrace();
            assertNotNull("stack trace available", st);
            assertTrue("stack trace size", st.length > 1);
            assertEquals("stack trace element",
                         "TestAbstractMethodError.test(TestAbstractMethodError.java:35)",
                         st[0].toString());
        }
    }

    public static abstract class Base {
        abstract void foo();
    };

    /*
     * The following Bytecode was derived from a class like this:
     *    public class Foo extends TestAbstractMethodError.Base {
     *        // empty.
     *    }
     *
     * This class is not abstract but it misses to implement the
     * abstract method "void foo()" of the base class.
     */
    static byte[] bytecode = {
	(byte)0xca, (byte)0xfe, (byte)0xba, (byte)0xbe, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x2e, 
	(byte)0x00, (byte)0x0e, (byte)0x07, (byte)0x00, (byte)0x02, (byte)0x01, (byte)0x00, (byte)0x03, 
	(byte)0x46, (byte)0x6f, (byte)0x6f, (byte)0x07, (byte)0x00, (byte)0x04, (byte)0x01, (byte)0x00, 
	(byte)0x1c, (byte)0x54, (byte)0x65, (byte)0x73, (byte)0x74, (byte)0x41, (byte)0x62, (byte)0x73, 
	(byte)0x74, (byte)0x72, (byte)0x61, (byte)0x63, (byte)0x74, (byte)0x4d, (byte)0x65, (byte)0x74, 
	(byte)0x68, (byte)0x6f, (byte)0x64, (byte)0x45, (byte)0x72, (byte)0x72, (byte)0x6f, (byte)0x72, 
	(byte)0x24, (byte)0x42, (byte)0x61, (byte)0x73, (byte)0x65, (byte)0x01, (byte)0x00, (byte)0x06, 
	(byte)0x3c, (byte)0x69, (byte)0x6e, (byte)0x69, (byte)0x74, (byte)0x3e, (byte)0x01, (byte)0x00, 
	(byte)0x03, (byte)0x28, (byte)0x29, (byte)0x56, (byte)0x01, (byte)0x00, (byte)0x04, (byte)0x43, 
	(byte)0x6f, (byte)0x64, (byte)0x65, (byte)0x0a, (byte)0x00, (byte)0x03, (byte)0x00, (byte)0x09, 
	(byte)0x0c, (byte)0x00, (byte)0x05, (byte)0x00, (byte)0x06, (byte)0x01, (byte)0x00, (byte)0x0c, 
	(byte)0x49, (byte)0x6e, (byte)0x6e, (byte)0x65, (byte)0x72, (byte)0x43, (byte)0x6c, (byte)0x61, 
	(byte)0x73, (byte)0x73, (byte)0x65, (byte)0x73, (byte)0x07, (byte)0x00, (byte)0x0c, (byte)0x01, 
	(byte)0x00, (byte)0x17, (byte)0x54, (byte)0x65, (byte)0x73, (byte)0x74, (byte)0x41, (byte)0x62, 
	(byte)0x73, (byte)0x74, (byte)0x72, (byte)0x61, (byte)0x63, (byte)0x74, (byte)0x4d, (byte)0x65, 
	(byte)0x74, (byte)0x68, (byte)0x6f, (byte)0x64, (byte)0x45, (byte)0x72, (byte)0x72, (byte)0x6f, 
	(byte)0x72, (byte)0x01, (byte)0x00, (byte)0x04, (byte)0x42, (byte)0x61, (byte)0x73, (byte)0x65, 
	(byte)0x00, (byte)0x21, (byte)0x00, (byte)0x01, (byte)0x00, (byte)0x03, (byte)0x00, (byte)0x00, 
	(byte)0x00, (byte)0x00, (byte)0x00, (byte)0x01, (byte)0x00, (byte)0x01, (byte)0x00, (byte)0x05, 
	(byte)0x00, (byte)0x06, (byte)0x00, (byte)0x01, (byte)0x00, (byte)0x07, (byte)0x00, (byte)0x00, 
	(byte)0x00, (byte)0x11, (byte)0x00, (byte)0x01, (byte)0x00, (byte)0x01, (byte)0x00, (byte)0x00, 
	(byte)0x00, (byte)0x05, (byte)0x2a, (byte)0xb7, (byte)0x00, (byte)0x08, (byte)0xb1, (byte)0x00, 
	(byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x01, (byte)0x00, (byte)0x0a, (byte)0x00, 
	(byte)0x00, (byte)0x00, (byte)0x0a, (byte)0x00, (byte)0x01, (byte)0x00, (byte)0x03, (byte)0x00, 
	(byte)0x0b, (byte)0x00, (byte)0x0d, (byte)0x04, (byte)0x09, 
    };
}
