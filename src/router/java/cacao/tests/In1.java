// inline execution time test
//    warning: with curent max is execution time very long
public class In1 {
public static void main(String[] s) {
 In1 I = new In1();
 long max=10;//00000000;
 int i;
 int x0=0; int x1=0;
 int y0=0; int y1=0;
 int z;

 for (i=0; i<max; i++) {   // Inlined by:
  x0 = In1.plus1s(x0);      // -in & -ino & -inv & -inov
  x1 = I.plus1f(x0);        // -inv & -inov
  y0 = I.plus1p(x0);        // -in & -ino & -inv & -inov 
  y1 = I.plus1v(x0);        // -inv & -inov
  }
 System.out.println("x0="+x0); 
// For -inov :
//   java/lang/StringBuffer.append(I)Ljava/lang/StringBuffer;
//   java/io/PrintStream.println(Ljava/lang/String;)V
 } 

public static int plus1s(int x) {return x+1;}
public final  int plus1f(int x) {return x+1;}
private       int plus1p(int x) {return x+1;}
public        int plus1v(int x) {return x+1;}
}

