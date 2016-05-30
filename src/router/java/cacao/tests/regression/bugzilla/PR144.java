/* tests/regression/bugzilla/PR144.java

   Copyright (C) 1996-2011
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

interface test_summable {
    long sum();
}

class y implements test_summable {
    public long l;
    public int i;
    public long sum() {
        return l+i;
    }
}

public class PR144 {
    static long do_test(Object o, int val) {
        y vy = (y) o;
        test_summable sm = null;
        if (o instanceof test_summable)
            sm = (test_summable) o;
        vy.l = 0x123456789L;
        vy.i = 0x98765;
        long r = sm.sum();
        vy.l = val;
        vy.i = val;
        return r + sm.sum();
    }

    @Test
    public void test() {
        try {
            do_test(null, 0);
        } catch (NullPointerException e) {
        }
        assertEquals(4887631770L, do_test(new y(), 0x23456));
    }
}




