/* tests/regression/bugzilla/PR89.java

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

import java.lang.reflect.*;

public class PR89 {
    final static float  f = 123.456f;
    final static double d = 789.012;

    @Test
    public void testFloat() throws Exception {
        FloatReflect floatReflect = new FloatReflect();
        Method m = FloatReflect.class.getMethod("returnFloat", null);
        Float ret = (Float) m.invoke(floatReflect, null);
        assertEquals(f, ret, 0.0);
    }

    @Test
    public void testDouble() throws Exception {
        DoubleReflect doubleReflect = new DoubleReflect();
        Method m = DoubleReflect.class.getMethod("returnDouble", null);
        Double ret = (Double) m.invoke(doubleReflect, null);
        assertEquals(d, ret, 0.0);
    }

    class FloatReflect {
        public float returnFloat() {
            return f;
        }
    }

    class DoubleReflect {
        public double returnDouble() {
            return d;
        }
    }
}
