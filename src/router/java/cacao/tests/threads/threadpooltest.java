// Use with "nc localhost 8080 < /dev/null"
// Idle threads are parked by the jsr166 framework and caused lots of CPU usage
// before the addition of park/unpark to CACAO.

import java.net.*;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.net.InetSocketAddress;

public class threadpooltest {
	private static final int NTHREADS = 3;
	private static final Executor exec = Executors.newFixedThreadPool(NTHREADS);
	
	private static void handleRequest(Socket s) {
		String ss = "hello";
		try {
			if (Thread.currentThread().isInterrupted())
				ss += " (wasinterrupted)";
			ss += " " + Thread.currentThread().toString();
			PrintWriter pw = new PrintWriter(s.getOutputStream());
			//Thread.currentThread().interrupt();
			pw.println(ss);
			pw.flush();
			Thread.sleep(100);
			pw.println("closing");
			pw.flush();
			s.close();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public static void main(String[] args) throws IOException {
		ServerSocket socket = new ServerSocket();
		socket.setReuseAddress(true);
		socket.bind(new InetSocketAddress(8080));
		while (true) {
			final Socket connection = socket.accept();
			Runnable task = new Runnable() {
				public void run() {
					handleRequest(connection);
				}
			};
			exec.execute(task);
		}
	}
}
