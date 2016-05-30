/* tests/regression/bugzilla/PR187.java

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

public class PR187 {
    @Test
    public void testBootPackages() {
        boolean found = false;
        for (Package p: Package.getPackages())
            if (p.getName().equals("java.lang")) {
                found = true;
                break;
            }
        assertTrue(found);
    }

    @Test
    public void testClasspath() {
        try {
            String[] result = SecondaryVMRunner.run(
                System.getProperty("cacao.test.javacmd"), "-cp boottest.jar boottest.bootpackage");
            assertEquals("out:boottest\nout:vendor:FantasyVendor\n", result[0]);
            assertTrue(result[1].isEmpty());
        } catch (Exception e) {
            fail(e.toString());
        }
    }

    @Test
    public void testBootclasspath() {
        try {
            // Not completely there yet -- we get GNU Classpath's
            // vendor instead of the one from the manifest
            String expect = "out:boottest\nout:vendor:";
            String[] result = SecondaryVMRunner.run(
                System.getProperty("cacao.test.javacmd"), "-Xbootclasspath/a:boottest.jar boottest.bootpackage");
            assertEquals(expect, result[0].substring(0, expect.length()));
            assertTrue(result[1].isEmpty());
        } catch (Exception e) {
            fail(e.toString());
        }
    }
}
