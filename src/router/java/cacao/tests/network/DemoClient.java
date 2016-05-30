import java.net.*;
import java.io.*;

public class DemoClient extends Thread
{
	public static void main(String[] args)
	{
		DemoClient app = new DemoClient();
		app.start();
	}

	public void run()
	{
		Socket newSocket;

		try {
			newSocket = new Socket("localhost", 5758);
			System.out.println("Client connected to server.");
			DataOutputStream outStream = new DataOutputStream(
				newSocket.getOutputStream());
			outStream.writeInt(12345);
			System.exit(0);
		} catch (Exception oops) {
			System.out.println("Error connecting to server");
			return;
		}
	}
}
