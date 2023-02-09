// vim: tabstop=8 shiftwidth=4 softtabstop=4 noexpandtab
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

import fr.free.miniupnp.libnatpmp.NatPmp;
import fr.free.miniupnp.libnatpmp.NatPmpResponse;

class JavaTest {
    public static void main(String[] args) {
	NatPmp natpmp = new NatPmp();

        natpmp.sendPublicAddressRequest();
        NatPmpResponse response = new NatPmpResponse();

        int result = -1;
        for(;;) {
            result = natpmp.readNatPmpResponseOrRetry(response);
	    if (result == 0) break;
	    if (result != -100) { // -100 = NATPMP_TRYAGAIN
		String errorString = String.format("NATPMP error %d", result);
		throw new RuntimeException(errorString);
	    }
	    try {
		Thread.sleep(4000);
	    } catch (InterruptedException e) {
		//fallthrough
	    }
	}

	byte[] bytes = intToByteArray(response.addr);

	try {
	    InetAddress inetAddress = InetAddress.getByAddress(bytes);
	    System.out.println("Public address is " + inetAddress);
	} catch (UnknownHostException e) {
	    throw new RuntimeException(e);
	}
    }

    public static final byte[] intToByteArray(int value) {
        return new byte[] {
            (byte)value,
            (byte)(value >>> 8),
            (byte)(value >>> 16),
            (byte)(value >>> 24)};
    }
}
