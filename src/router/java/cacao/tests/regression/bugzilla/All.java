/* tests/regression/bugzilla/All.java - runs all CACAO regression unit tests

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


import org.junit.runner.RunWith;
import org.junit.runners.Suite;

@RunWith(Suite.class)

@Suite.SuiteClasses({
PR52.class,
PR57.class,
PR58.class,
PR65.class,
PR80.class,
PR89.class,
PR112.class,
PR113.class,
PR114.class,
PR116.class,
PR119.class,
PR125.class,
PR131.class,
PR144.class,
PR148.class,
PR162.class,
PR185.class,
PR187.class    
})

public class All {
}
