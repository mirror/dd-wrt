/* tests/regression/bugzilla/PR116.java

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

import java.lang.reflect.Array;

public class PR116 {
    private static final int N = 5;
    private static class Foo {}

    @Test
    public void test() {
        for (int i = 1; i <= N; i++) {
            int[] dim = new int[i];
            for (int j = 0; j < i; j++)
                dim[j] = 23;

            Object o = Array.newInstance(Foo.class, dim);
            //System.out.println("PR116: dim=" + i + ", class='" + o.getClass() + "'");

	    assertNotNull(o);
	    assertTrue(o.getClass().isArray());
        }
    }
}
