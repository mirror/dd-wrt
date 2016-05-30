//	File: GenericServer.java
//	Author: Thomas Lea  (leat@goodnet.com,  http://www.goodnet.com/~leat)
//	Date: 12/7/95
//	Version: 1.0  (for Beta JDK API)
//
// Copyright (c) 1995 Thomas Lea. All Rights Reserved.
//


// Please feel free to take this program and modify it to your hearts content.
// The only thing I ask is that my name and the above version/copyright 
// information stay at the top of this module.

import java.net.*;
import java.io.*;
import java.util.Vector;

class GenericServer
{
    private static final int DEFAULT_PORT=8887;
    private ConnectionManager cm = null;

    public GenericServer(int port)
    {
        System.out.println("Server is initializing to port " + port);

        cm = new ConnectionManager(port);
        cm.start();
    }

    public static void main(String args[])
    {
        int server_port;

        try
        {
            server_port = Integer.parseInt(args[0],10);
        }
        catch(Exception e)
        {
            System.out.println("Defaulting to port " + DEFAULT_PORT);
                    server_port = DEFAULT_PORT;
        }
        new GenericServer(server_port);
    }
}

// Waits for a connection then spawns a ServerConnection to deal with it
class ConnectionManager extends Thread
{
    private static int _port;
    private static Vector _my_threads = new Vector(5,2);
                                           //size of 5 initially, grow by 2
    private ServerSocket _main_socket = null;

    public ConnectionManager(int port)
    {
        _port = port;
    }

    public void run()
    {
        serveRequests();
    }

    private void serveRequests()
    {
        try {_main_socket = new ServerSocket(_port);}
        catch(Exception e) { System.err.println(e); System.exit(1);}

        ServerConnection temp_sc = null;

        while (true)
        {
            try
            {
                Socket this_connection = _main_socket.accept();
	
                temp_sc = new ServerConnection(this_connection);
                temp_sc.start();
                _my_threads.addElement(temp_sc);

                //clean up the vector if needed
                for(int i=0;i<ConnectionManager._my_threads.size();i++)
                if(!((ServerConnection)(_my_threads.elementAt(i))).isAlive())
                        _my_threads.removeElementAt(i);
            }
            catch(Exception e)
            {
                System.err.println("Exception:\n" + e);
            }
        }
    }
}

class ServerConnection extends Thread
{
    private Socket _mysocket;
    private PrintStream _output;
    private InputStream _input;
    FileInputStream _dis;

    public ServerConnection(Socket s)
    {
        _mysocket = s;
    }

    private void doServerWork()
    {
        //This is where the server actually does its work.
        //when this method finishes, the socket will be closed
        //and this thread will exit.

        //This is just some junk... put your real work here.
        try
        {
            byte b[] = new byte[1024];
            int nbytes = _input.read(b,0,1024);
            String str = new String(b,0,0,nbytes);

            _dis = new FileInputStream("FILE.HTM");
            StringBuffer buf = new StringBuffer(4096);
			
            System.out.println("Received for server: " + str);

            nbytes = _dis.available();
            _output.println("HTTP/1.0 200 OK");
            _output.println("MIME-Version: 1.0");
            _output.println("Date: 22Dec95");
            _output.println("Server: Java_Server_0.01");
            _output.println("Content-type: text/html");
//          _output.println("Content-Length: 76");
            _output.println("");
//      _output.println("<HEAD><TITLE>Clock 2.0 Title</TITLE></HEAD><BODY>");
//          _output.println("<H1>Clock 2.0</H1>");
            b = new byte[nbytes];
            _dis.read(b, 0, nbytes);
            str = new String(b, 0, 0, nbytes);
            _output.println(str);
//          _output.println("</BODY>");
            _output.println("");
//          for(int i=0;i<10;i++)
//          {
//              _output.println("This is a message from the server");
//              sleep((int)(Math.random() * 4000));
//          }
        }
        catch(Exception e)
        {
        }
    }

    public void run()
    {
        System.out.println("Connected to: " +
                   _mysocket.getInetAddress() +":"+ _mysocket.getPort());
        try
        {
            _output = new PrintStream(_mysocket.getOutputStream());
            _input = _mysocket.getInputStream();

            //Lets get busy!
            doServerWork();

            //We are outta here....
            _mysocket.close();
        }
        catch ( Exception e )
        {
            System.err.println( "Exception:\n" + e );
        }

        System.out.println("Disconnecting: " + _mysocket.toString());
        stop();
    }
}
