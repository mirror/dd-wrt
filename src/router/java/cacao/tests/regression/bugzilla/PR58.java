/* tests/regression/bugzilla/PR58.java

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

import java.io.*;

public class PR58 {
    class x extends y {}
    class y {}

    @Test
    public void testSuperClass() {
        // Delete the class file which is extended.
        new File("PR58$y.class").delete();

        try {
            Class.forName("PR58$x");
            fail("Should throw NoClassDefFoundError");
        }
        catch (ClassNotFoundException error) {
            fail("Unexpected exception: " + error);
        }
        catch (NoClassDefFoundError success) {
            // Check if the cause is correct.
            assertTrue(success.getCause() instanceof ClassNotFoundException);
        }
    }

    interface i {}
    class j implements i {}

    @Test
    public void testSuperInterface() {
        // Delete the interface file which is implemented.
        new File("PR58$i.class").delete();

        try {
            Class.forName("PR58$j");
            fail("Should throw NoClassDefFoundError");
        }
        catch (ClassNotFoundException error) {
            fail("Unexpected exception: " + error);
        }
        catch (NoClassDefFoundError success) {
            // Check if the cause is correct.
            assertTrue(success.getCause() instanceof ClassNotFoundException);
        }
    }
}
