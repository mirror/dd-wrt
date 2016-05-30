import java.util.*;
import java.io.*;

class SubClass1 extends Object {
}

class SubClass2 extends Object {
  int a,b,c;

  SubClass2 (int aa, int ab, int ac) {
    a = aa;
    b = ab;
    c = ac;
  }

  public void methodWithLocalAccess () {
    int la,lb,lc;
    for (int i=0; i<JavaPerformance.MAXCOUNT; i++) {
      la = 5;
      lb = 6;
      lc = 7;
    }
  }

  public void methodWithInstanceAccess () {
    for (int i=0; i<JavaPerformance.MAXCOUNT; i++) {    
      a = 5;
      b = 6;
      c = 7;
    }
  }  
}

class List extends Object {

  static Random r = new Random();
  
  List[] data;
  char[] memory;
  
  List (int depth) {
    memory = new char[512];
    if (depth>0) {
      int length = 1+Math.abs(r.nextInt())%5;
      data = new List[length];
      for (int i=0; i<length; i++)
        data[i] = new List (depth-1);
    }
  }
}

public class JavaPerformance {

  static final int MAXCOUNT = 500000;  
  static Throwable aThrowable = new Throwable ();
  
  public JavaPerformance () {
    super();
  }

  // static
  
  static public int staticCall () {
    return 0;
  }

  static public final int staticFinalCall () {
    return 0;
  }
  
  static public synchronized int synchronizedStaticCall () {
    return 0;
  }
  
  static public void throwingStaticCall () throws Throwable {
    throw new Throwable ();
  }

  // methods
  
  public int methodCall () {
    return 0;
  }

  public final int finalMethodCall () {
    return 0;
  }

  static public synchronized int synchronizedMethodCall () {
    return 0;
  }
  
  static public void throwingMethodCall () throws Throwable {
    throw new Throwable ();
  }

  // ggT

  public static int ggT (int x, int y) {
    while (x!=y) {
      if (x<y) y = y-x; else x = x-y;
    }
    return x;
  }

  public static long fak (long x) {
    if (x>1)
      return x*fak(x-1);
    else
      return 1;
  }
  
  public static void arithmeticTest () {
    System.out.println ("Arithmetic time");
    System.out.println ();

    System.out.print ("ggT (24,42)...");
    long l = System.currentTimeMillis();
    for (int i = 0; i<MAXCOUNT; i++)
      ggT (24,42);
    long e = System.currentTimeMillis();
    System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");    
    System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);
    System.out.println (")");

    System.out.print ("13!...");
    l = System.currentTimeMillis();
    for (int i = 0; i<MAXCOUNT; i++)
      fak (13);
    e = System.currentTimeMillis();    
    System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");    
    System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);
    System.out.println (")");    
  }

  public static void memoryTest () {
    System.out.println ("Memory time");
    System.out.println ();

    System.out.print ("Object creation...");
    long l = System.currentTimeMillis();
    for (int i = 0; i<MAXCOUNT; i++)
      new Object ();
    long e = System.currentTimeMillis();        
    System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");    
    System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);
    System.out.println (")");

    System.out.print ("Subclass creation...");
    l = System.currentTimeMillis();
    for (int i = 0; i<MAXCOUNT; i++)
      new SubClass1 ();
    e = System.currentTimeMillis();        
    System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");    
    System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);
    System.out.println (")");    

    System.out.print ("Subclass creation with constructor call...");
    l = System.currentTimeMillis();
    for (int i = 0; i<MAXCOUNT; i++)
      new SubClass2 (1,2,3);
    e = System.currentTimeMillis();        
    System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");    
    System.out.print(e-l);    
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);    
    System.out.println (")");    

    System.out.print ("List generation (GC test)...");
    l = System.currentTimeMillis();
    for (int i = 0; i<250; i++)
      new List (5);
    e = System.currentTimeMillis();        
    System.out.print ("(");
    System.out.print (((double) e-l)/250);
    System.out.print ("ms ");    
    System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (250);    
    System.out.println (")");    
    
  }

  public static void methodTest () {
    SubClass2 object = new SubClass2 (1,2,3);
    
    System.out.println ("Method execution time");
    System.out.println ();
    
    System.out.print ("Method with access only to local variables...");
    long l = System.currentTimeMillis();
    object.methodWithLocalAccess ();
    long e = System.currentTimeMillis();        
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");

     System.out.print ("Method with access only to instance variables...");
     l = System.currentTimeMillis();
     object.methodWithInstanceAccess ();
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");
  }
  
  public static void callTest () {
    JavaPerformance object = new JavaPerformance ();

    System.out.println ("Calling time");
    System.out.println ();
    System.out.print ("Static function call...");
    long l = System.currentTimeMillis();
     for (int i = 0; i<MAXCOUNT; i++)
       staticCall ();
     long e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");

    System.out.print ("Static final function call...");
     l = System.currentTimeMillis();
     for (int i = 0; i<MAXCOUNT; i++)
       staticFinalCall ();
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");

     System.out.print ("Static synchronized function call...");
     l = System.currentTimeMillis();
     for (int i = 0; i<MAXCOUNT; i++)
       synchronizedStaticCall();
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");
     
     System.out.print ("Static function call in try catch block...");
     l = System.currentTimeMillis();
     for (int i = 0; i<MAXCOUNT; i++)
       try {
         staticCall ();
       } catch (Throwable a) {
       }
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");
    
    System.out.print ("Static exception throwing function call...");
    l = System.currentTimeMillis();
    for (int i = 0; i<5000; i++)
      try {
        throwingStaticCall ();
      } catch (Throwable exp) {
      }
    e = System.currentTimeMillis();        
    System.out.print ("(");
    System.out.print (((double) e-l)/5000);
    System.out.print ("ms ");    
    System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (5000);
    System.out.println (")");


    System.out.print ("Method call...");
    l = System.currentTimeMillis();
     for (int i = 0; i<MAXCOUNT; i++)
       object.methodCall ();
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");

    System.out.print ("Final method call...");
    l = System.currentTimeMillis();
     for (int i = 0; i<MAXCOUNT; i++)
       object.finalMethodCall ();
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");

     System.out.print ("Synchronized method call...");
     l = System.currentTimeMillis();
     for (int i = 0; i<MAXCOUNT; i++)
       object.synchronizedMethodCall();
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");
     
     System.out.print ("Method call in try catch block...");
     l = System.currentTimeMillis();
     for (int i = 0; i<MAXCOUNT; i++)
       try {
         object.methodCall ();
       } catch (Throwable a) {
       }
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/MAXCOUNT);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (MAXCOUNT);     
     System.out.println (")");
    
     System.out.print ("Exception throwing method call...");
     l = System.currentTimeMillis();
     for (int i = 0; i<50000; i++)
       try {
         object.throwingMethodCall ();
       } catch (Throwable exp) {
       }
     e = System.currentTimeMillis();         
     System.out.print ("(");
    System.out.print (((double) e-l)/5000);
    System.out.print ("ms ");     
     System.out.print(e-l);
    System.out.print ("ms/");
    System.out.print (5000);
     System.out.println (")");
  }

  public static final void main (String[] args) {
    long l = System.currentTimeMillis();    
    arithmeticTest ();
    System.out.println ();
    callTest ();
    System.out.println ();
    methodTest ();
    System.out.println ();    
    memoryTest ();
    System.out.println ();
    System.out.print ("Overall time ");
    System.out.print(System.currentTimeMillis()-l);
    System.out.println ("ms");
  }
}
