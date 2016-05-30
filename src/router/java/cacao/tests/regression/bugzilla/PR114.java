/* tests/regression/bugzilla/PR114.java

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

import java.io.File;

public class PR114 {
    @Test ( expected = NoClassDefFoundError.class )
    public void test() throws NoClassDefFoundError {
        // Delete the class file which is extended.
        new File("PR114$A.class").delete();

        new A().sub();
    }

    class A {
        public void sub() {}
    }
}
