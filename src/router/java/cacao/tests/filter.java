import java.io.*;


public class filter {
   public static void main(String argv[]) {
   
      DataInputStream in = new DataInputStream(System.in);

      
      try {
         while (true) {
            byte b = in.readByte();
            if (b <= 128) { 
               int x = b;
               System.out.println("BYTE: " + x);
            } else {
               String x = "non ascii: " + b;
               System.out.println(x);
            }
         }
      }
      catch (IOException e) {
         System.out.println("-- END OF FILE --");
      }
      finally {
         System.out.println("--- Cool finally ---");
      }
   }
}
