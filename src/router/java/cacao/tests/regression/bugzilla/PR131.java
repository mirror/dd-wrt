/* tests/regression/bugzilla/PR131.java

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

class testobject {
    private final Object o = new Object();

    private final Object f = new Object() {
        protected void finalize() throws Throwable {
            synchronized(o) {
                o.notify();
            }
        }
    };

    testobject() {
        synchronized(o) {
            // inflate the lock
            Thread.currentThread().interrupt();
            try {
                o.wait();
            }
            catch (InterruptedException e) {
            }
        }
    }
}

public class PR131 {
    @Test
    public void test() {
        for (int i=0; i<20000; i++)
            new testobject();

    }
}
