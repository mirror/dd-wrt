import java.net.*;
import java.io.*;

class ServerThread extends Thread
{
    httpd server;

    ServerThread (httpd the_server)
    {
	server = the_server;
    }

    synchronized void wait_till_finished ()
    {
	try
	{
	    wait();
	}
	catch (Exception exc)
	{
	    System.out.println("Exception " + exc + " in wait");
	}
    }

    boolean is_info_line (String str)
    {
	return str.startsWith("User-Agent: ") || str.startsWith("Host: ")
	    || str.startsWith("Accept: ");
    }

    static void write_string (OutputStream os, String str) throws IOException
    {
	byte buf[] = new byte[str.length()];

	str.getBytes(0, str.length(), buf, 0);
	os.write(buf);
    }

    synchronized void handle_request (Socket socket)
    {
	System.out.println("begin handle_request");

	try
	{
	    InputStream is = socket.getInputStream();
	    DataInputStream dis = new DataInputStream(is);
	    String request;

	    request = dis.readLine();
	    if (request.startsWith("GET "))
	    {
		String doc = request.substring(4);

		System.out.println("request: " + doc);

		boolean is_10 = false;

		if (doc.endsWith(" HTTP/1.0"))
		{
		    is_10 = true;
		    
		    String info;

		    do
		    {
			info = dis.readLine();
		    } while (is_info_line(info));

		    doc = doc.substring(0, doc.length() - 9);
		}

		String filename = httpd.document_root + doc;
		File file = new File(filename);

		if (file.isDirectory())
		    file = new File(filename + "/index.html");
		if (file.exists() && file.isFile() && file.canRead())
		{
		    OutputStream os = socket.getOutputStream();

		    System.out.println("kurde 1");

		    FileInputStream fis = new FileInputStream(file);

		    System.out.println("kurde 2");
		    System.out.println("kurde 2a");

		    byte buffer[] = new byte[8192];
		    byte heusl[] = new byte[8192];

		    System.out.println("kurde 3");

		    if (is_10)
			write_string(os,
				     "HTTP/1.0 200 OK\n"
				     //+ "Content-Length: " + file.length() + "\n"
				     + "Content-Type: text/html\n"
				     + "Server: Schani's Kurden-Server\n"
				     + "\n");

		    System.out.println("begin copy");

		    int result;

		    do
		    {
			result = fis.read(buffer);
			if (result > 0)
			    os.write(buffer, 0, result);
		    } while (result != -1);
		    System.out.println("end copy");
		    
		    fis.close();
		}
		else if (is_10)
		    write_string(socket.getOutputStream(),
				 "HTTP/1.0 404 Not Found\n"
				 + "\n");
	    }
	    else
		System.out.println("malformed request: " + request);
	}
	catch (Exception exc)
	{
	    System.out.println("Exception " + exc + " in thread");
	}

	try
	{
	    InputStream is = socket.getInputStream();
	    int len;

	    while ((len = is.available()) > 0)
	    {
		for (int i = 0; i < len; ++i)
		    is.read();
	    }

	    socket.close();
	}
	catch (Exception exc)
	{
	    System.out.println("Exception " + exc + " in close");
	}

	server.thread_ready(this);
	notify();

	System.out.println("end handle_request");
    }
}

public class httpd
{

    static String document_root;
    static int num_threads = 10;

    ServerThread threads[] = null;
    boolean threads_ready[] = null;

    void run ()
    {
	threads = new ServerThread[num_threads];
	threads_ready = new boolean[num_threads];

	for (int i = 0; i < num_threads; ++i)
	{
	    threads[i] = new ServerThread(this);
	    threads_ready[i] = true;
	}

	try
	{
	    ServerSocket ss = new ServerSocket(8001, 5);

	    while (true)
	    {
		Socket socket = ss.accept();
		ServerThread thread = get_ready_thread();

		thread.handle_request(socket);
	    }
	}
	catch (Exception exc)
	{
	    System.out.println("Exception " + exc);
	}
    }

    public static void main (String args[])
    {
	document_root=args[0];
	new httpd().run();
    }

    synchronized void thread_ready (ServerThread thread)
    {
	for (int i = 0; i < num_threads; ++i)
	    if (threads[i] == thread)
	    {
		threads_ready[i] = true;
		break;
	    }
    }

    ServerThread get_ready_thread ()
    {
	for (int i = 0; i < num_threads; ++i)
	    if (threads_ready[i])
	    {
		threads_ready[i] = false;
		return threads[i];
	    }

	threads[0].wait_till_finished();
	threads_ready[0] = false;
	return threads[0];
    }
}
