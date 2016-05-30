/* tests/regression/bugzilla/PR185.java

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

public class PR185 {
    @Test
    public void test() {
        try {
            String[] result = SecondaryVMRunner.run(
                System.getProperty("cacao.test.javacmd"), "finalizer_exceptions");
            assertEquals("out:Success!\n", result[0]);
            assertTrue(result[1].isEmpty());
        } catch (Exception e) {
            fail(e.toString());
        }
    }
}
