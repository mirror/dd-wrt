import java.io.*;
import java.net.*;

class whatip2 {
    static void output (InetAddress addr) {
	if (addr == null) {
	    System.out.println("address is null");
	} else {
	    System.out.println("inet address is " + addr.toString() );
	}
    }

    public static void main(String a[]) throws IOException {
	try {
	    System.out.println("hostname is " + a[0] );

	    output(InetAddress.getByName (a[0]));

	    output(InetAddress.getLocalHost());
	}
	catch (Throwable t) {
	    System.out.println("Catched error " + t.toString() );
	}
    }  
}
