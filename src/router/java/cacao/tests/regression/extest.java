public class extest {
    final static int COLUMN = 55;

    final static int INDEX1 = 0xcafebabe;
    final static int INDEX2 = 0xbabecafe;
    final static int INDEX3 = 0xdeadbeef;

    static boolean printStackTrace;
    
    public static void main(String[] argv) {
	printStackTrace = true;

        Runtime r = Runtime.getRuntime();
        int maxmem = (int) r.maxMemory();

//  	if (argv.length > 0) 
//              if (argv[0].equals("stacktrace"))
//                  printStackTrace = true;

        boolean caught = false;

	pheader("normal exceptions");

   	try {
            p("throw new Exception():");
    	    throw new Exception();
    	} catch (Exception e) {
            caught = true;
      	    ok();
	    pstacktrace(e);
    	} finally {
            /* check if catch block was executed */
            if (!caught) {
                failed();
            }
        }

   	try {
            p("throw new Exception() (from subroutines):");
    	    sub();
            failed();
    	} catch (Exception e) {
      	    ok();
	    pstacktrace(e);
    	}

        try {
            p("NullPointerException:");
            int[] ia = null;
            int i = ia.length;
            failed();
        } catch (NullPointerException e) {
  	    ok();
	    pstacktrace(e);
  	}

	pln();


	pheader("exceptions thrown in JIT code");

        try {
            p("ArithmeticException (only w/ -softnull):");
            int i = 1, j = 0, k = i / j;
            failed();
        } catch (ArithmeticException e) {
	    String msg = e.getMessage();

	    if (msg == null || !msg.equals("/ by zero")) {
		pln("FAILED: wrong message: " + msg + ", should be: / by zero");
		pstacktrace(e);
	    } else {
                ok();
                pstacktrace(e);
            }
  	}

        try {
            p("ArrayIndexOutOfBoundsException:");
            int[] ia = new int[1];
            ia[INDEX1] = 1;
            failed();
        } catch (ArrayIndexOutOfBoundsException e) {
	    String msg = e.getMessage();

	    if (msg == null || !msg.equals(String.valueOf(INDEX1))) {
		pln("FAILED: wrong index: " + msg + ", should be: " + INDEX1);
		pstacktrace(e);
	    } else {
		ok();
	        pstacktrace(e);
	    }
  	}

        try {
            p("ArrayStoreException:");
	    Integer[] ia = new Integer[1];
            Object[] oa = (Object[]) ia;
	    oa[0] = new Object();
            failed();
        } catch (ArrayStoreException e) {
  	    ok();
	    pstacktrace(e);
  	}

        try {
            p("ClassCastException:");
            Object o = new Object();
            Integer i = (Integer) o;
            failed();
        } catch (ClassCastException e) {
  	    ok();
	    pstacktrace(e);
  	}

        try {
            p("NegativeArraySizeException (newarray):");
            int[] ia = new int[-1];
            failed();
        } catch (NegativeArraySizeException e) {
  	    ok();
	    pstacktrace(e);
  	}

        try {
            p("NegativeArraySizeException (multianewarray):");
            int[][] ia = new int[1][-1];
            failed();
        } catch (NegativeArraySizeException e) {
  	    ok();
	    pstacktrace(e);
  	}

        try {
            p("OutOfMemoryError:");
	    /* maxmem + 1 should be enough and hopefully not overflow the int so it becomes negative */
	    byte[] ba = new byte[maxmem];
            failed();
        } catch (OutOfMemoryError e) {
  	    ok();
	    pstacktrace(e);
  	}

        try {
            p("OutOfMemoryError (multianewarray):");
	    byte[][] ba = new byte[maxmem][maxmem];
            failed();
        } catch (OutOfMemoryError e) {
  	    ok();
	    pstacktrace(e);
  	}
        
	pln();


	pheader("exceptions in leaf functions");

        try {
            p("ArithmeticException:");
            aesub(1, 0);
            failed();
        } catch (ArithmeticException e) {
            ok();
            pstacktrace(e);
        }

        try {
            p("ArrayIndexOutOfBoundsException:");
            aioobesub(new int[1]);
            failed();
        } catch (ArrayIndexOutOfBoundsException e) {
	    String msg = e.getMessage();

	    if (msg == null || !msg.equals(String.valueOf(INDEX3))) {
		pln("FAILED: wrong index: " + msg + ", should be: " + INDEX3);
		pstacktrace(e);
	    } else {
		ok();
	        pstacktrace(e);

	    }
        }

        try {
            p("ClassCastException:");
            ccesub(new Object(), new Integer(0));
            failed();
        } catch (ClassCastException e) {
            ok();
            pstacktrace(e);
        }

        try {
            p("NullPointerException:");
            npesub(null);
            failed();
        } catch (NullPointerException e) {
            ok();
            pstacktrace(e);
        }

        try {
            p("Exception in <clinit> triggered from a leaf method:");
            extest_clinit_patcher.i = 1;
            failed();
        } catch (ExceptionInInitializerError e) {
            ok();
            pstacktrace(e);
        }

	pln();


	pheader("exception related things");

        try {
            p("load/link an exception class in asmpart:");
            throw new Exception();
        } catch (UnknownError e) {
            /* this exception class MUST NOT be loaded before!!!
               otherwise this test is useless */
        } catch (Exception e) {
  	    ok();
	    pstacktrace(e);
  	}

        pln();


	pheader("native stub exceptions");

        try {
            p("NullPointerException in <clinit>:");
            extest_clinit.sub();
            failed();
        } catch (ExceptionInInitializerError e) {
            ok();
            pstacktrace(e);
        }

        try {
            p("UnsatisfiedLinkError:");
            nsub();
            failed();
        } catch (UnsatisfiedLinkError e) {
            ok();
            pstacktrace(e);
        }

  	try {
            p("NullPointerException (native):");
            System.arraycopy(null, 1, null, 1, 1);
            failed();
    	} catch (NullPointerException e) {
  	    ok();
            pstacktrace(e);
  	}

        pln();


	pheader("special exceptions");

  	try {
            p("OutOfMemoryError (array clone):");
            /* use half of the heap size */
            byte[] ba1 = new byte[maxmem / 2];
            byte[] ba2 = (byte[]) ba1.clone();
            failed();
    	} catch (OutOfMemoryError e) {
  	    ok();
            pstacktrace(e);
  	}

        pln();


	pheader("exception thrown to command-line");

        pln("NullPointerException (without catch):");
        String s = null;
        int i = s.length();
        failed();
    }

    synchronized static void sub() throws Exception {
	sub2();
    }

    static void sub2() throws Exception {
	sub3();
    }

    synchronized static void sub3() throws Exception {
	sub4();
    }

    static void sub4() throws Exception {
	throw new Exception();
    }

    static void aesub(int a, int b) {
        int c = a / b;
    }

    static void aioobesub(int[] ia) {
        ia[INDEX3] = 0;
    }

    static void ccesub(Object o, Integer i) {
        i = (Integer) o;
    }

    static void npesub(int[] ia) {
        int a = ia.length;
    }

    static native void nsub();

    static void p(String s) {
	System.out.print(s);
        for (int i = s.length(); i < COLUMN; i++) {
            System.out.print(" ");
        }
    }

    static void pheader(String s) {
	System.out.print(s);
        for (int i = s.length(); i < COLUMN + 3; i++) {
            System.out.print("-");
        }
        System.out.println();
        System.out.println();
    }

    static void pln() {
	System.out.println();
    }

    static void pln(String s) {
	System.out.println(s);
    }

    static void ok() {
        pln("OK");
    }

    static void failed() {
        pln("FAILED");
    }

    static void pstacktrace(Throwable e) {
	if (!printStackTrace)
            return;
	e.printStackTrace();
	System.out.println();
    }
}

class extest_clinit {
    static {
        String s = null;
        s.length();
    }

    public static native void sub();
}

class extest_clinit_patcher {
    static int i;

    static {
        int[] ia = null;
        int a = ia.length;
    }
}
