import java.net.*;
import java.io.*;

public class DemoServer extends Thread
{
	ServerSocket listenSock;

	public static void main(String[] args)
	{
		DemoServer app = new DemoServer();
		app.start();
	}

	public DemoServer()
	{
	}

	public void run()
	{
		Socket newSocket;

		try {
			listenSock = new ServerSocket(5758);
			System.out.println(listenSock.toString());
		} catch (Exception hell) {
			System.out.println("Error setting up listen socket:\n");
			System.out.println(hell);
			return;
		}

		while (true)
		{
			try {
				System.out.println("Trying accept");
				newSocket = listenSock.accept();
				System.out.println("Accept returned");
			} catch (Exception badAccept) {
				continue;
			}
			System.out.println("Accepted new client");

			try {
				DataInputStream inStream = new DataInputStream(
					newSocket.getInputStream());
				System.out.println("Client send "+inStream.readInt());
				newSocket.close();
			} catch (Exception oops) {
				System.out.println("Error reading from client");
				System.out.println(oops);
			}
		}
	}
}
