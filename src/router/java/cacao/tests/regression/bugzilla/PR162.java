/* tests/regression/bugzilla/PR162.java

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

// This test is not deterministic. It doesn't always fail, but it's very rare
// that 5 consecutive runs succeed.

import org.junit.Test;
import static org.junit.Assert.*;

import java.util.concurrent.Semaphore;

class M {
	public static Semaphore sem = new Semaphore(0);
}

class testglobal {
	public static String the_string = initializeString();

	static String initializeString() {
		M.sem.release();
		try {
			Thread.sleep(300);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		return "hullabaloo";
	}
}

class testthread extends Thread {
	public int the_length = 0;
	public void run() {
		try {
			M.sem.acquire();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		the_length = testglobal.the_string.length();
	}
}

public class PR162 {
	@Test
	public void test() {
		testthread t = new testthread();
		t.start();
		int length = testglobal.the_string.length();
		try {
			t.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		assertEquals(t.the_length, length);
	}
}
