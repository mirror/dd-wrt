/* tests/regression/bugzilla/PR80.java

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

public class PR80 {
    @Test ( expected = ArrayIndexOutOfBoundsException.class )
    public void test() throws ArrayIndexOutOfBoundsException {
        // Taken from Mauve gnu.testlet.java.lang.System.arraycopy
        int[] a = new int[5];
        int[] b = new int[5];
        System.arraycopy(a, 4, b, 4, Integer.MAX_VALUE);
    }
}
