import java.lang.*;
import java.net.*;
import java.io.*;
class socktest {
        public static void main(String argv[]) {
                try {
                        ServerSocket ss = new ServerSocket(0);
                        Socket s = new Socket("quinta", 23);
                        InetAddress a1 = InetAddress.getByName("quinta");
                        InetAddress a2 = InetAddress.getLocalHost();

                        System.out.println(ss);
                        System.out.println(s);
                        System.out.println(a1);
                        System.out.println(a2);
                } catch (IOException e) {
                        System.out.println("error: "+e);
                }
        }
}
