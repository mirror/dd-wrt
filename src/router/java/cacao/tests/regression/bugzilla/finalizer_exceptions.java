/* tests/regression/bugzilla/finalizer_exceptions.java

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

public class finalizer_exceptions {
    static class Tester {
        static int hadWithException = 0;
        static boolean hadWithoutException = false;
        public static Object lock = new Object();
        Tester(int num) {
            this.num = num;
        }
        static void checkReady() {
            synchronized (lock) {
                if (hadWithException >= 3 && hadWithoutException)
                    lock.notifyAll();
            }
        }
        public void finalize() throws Exception {
            try {
                throw new Exception("final");
            } catch (Exception e) {
                if (num == 0) {
                    System.out.println("Success!");
                    hadWithoutException = true;
                    checkReady();
                }
                else {
                    hadWithException++;
                    checkReady();
                    throw e;
                }
            }
        }
        private int num;
    }

    public static void main(String[] args) {
        for (int i=0; i<10; i++)
            new Tester(i);
        System.gc();
        System.gc();
        System.runFinalization();
        long timestamp_start = System.currentTimeMillis();
        try {
            synchronized (Tester.lock) {
                // Don't stop at the first throwing finalizer. We want the
                // exception to escape and be dealt with at the JVM level.
                while (System.currentTimeMillis() - 2000 < timestamp_start &&
                       (Tester.hadWithException < 3 || !Tester.hadWithoutException))
                    Tester.lock.wait(2000);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
