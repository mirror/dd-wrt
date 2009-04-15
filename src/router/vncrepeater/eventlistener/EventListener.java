// /////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Jari Korhonen. jarit1.korhonen@dnainternet.net.
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Program is based on "TableDemo" example on Java documentation
// and uses "In" class and socket example code from "EchoServer Java"
//
// If the source code for the program is not available from the place
// from which you received this file, check
// http://koti.mbnet.fi/jtko
//
// ////////////////////////////////////////////////////////////////////


import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import java.awt.Dimension;
import java.awt.GridLayout;

import java.net.Socket;
import java.net.ServerSocket;
import java.util.StringTokenizer;
import java.util.Date;
import java.net.InetAddress;
import java.net.UnknownHostException;


/**
 *   ListenerThread listens repeater connections in method run() and
 *   processes them in method processEvents() 
 */
class ListenerThread extends Thread {
    static final int PORT = 2002;
    static final int BACKLOG = 5;
    static final int MAX_EVENTS = 500;
    String[] events = new String[MAX_EVENTS];
    String listenIp;
    int listenPort;
    JTable tableToPost;
    int lastRow;
        
    ListenerThread(JTable table, int lastRow, String listenIp, int listenPort) {
        super();
        tableToPost = table;    
        this.lastRow = lastRow;
        this.listenIp = listenIp;
        this.listenPort = listenPort;
    }
    
    String getHost(String ipAddress) {
        try {
            InetAddress addr = InetAddress.getByName(ipAddress);

            return addr.getHostName() + " (" + ipAddress + ")";
        } catch (UnknownHostException e) {
            return "unknown" + " (" + ipAddress + ")"; 
        }
    }
    
    void processEvents(int numEvents) {
        int numSlices;
            
        for (int ii = 0; ii < numEvents; ii++) {
            boolean httpEventLine;

            if (events[ii].indexOf("HTTP/1.0") >= 0) {
                httpEventLine = true;
            } else {
                httpEventLine = false;
            }
	
            // Kill http junk from event string
            if (httpEventLine) {
                // Kill "GET /? "
                events[ii] = events[ii].substring(events[ii].indexOf("EvMsg"));
            	
                // Kill " HTTP/1.0"
                events[ii] = events[ii].substring(0,
                        events[ii].indexOf("HTTP/1.0") - 1);
            	
                // Add \n
                events[ii] += "\n";
            }
            
            // Tokenize event line
            StringTokenizer slicedEvent = new StringTokenizer(events[ii],
                    (httpEventLine) ? "&" : ",");
            String[][] slices = new String[slicedEvent.countTokens() + 10][2];
        
            numSlices = 0;

            while (slicedEvent.hasMoreTokens()) {
                slices[numSlices][0] = slicedEvent.nextToken();
                
                StringTokenizer slicedField = new StringTokenizer(
                        slices[numSlices][0], (httpEventLine) ? "=" : ":");
                
                while (slicedField.hasMoreTokens()) {
                    slices[numSlices][1] = slicedField.nextToken();
                }
                
                numSlices++;
            }
            
            // Determine event meaning and contruct message to GUI
            try {
                int tableIndex;
                
                int eventMeaning = new Integer(slices[1][1]);
                String Message = new String();
                int viewerTableInd;
                int serverTableInd;
                long time = new Long(slices[2][1]);
                Date timeStamp = new Date(1000 * time);
                    
                switch (eventMeaning) {
                case 0: // Viewer connect
                    Message = "Connection: code:" + slices[5][1] + ", Host:"
                            + getHost(slices[7][1]);
                    
                    serverTableInd = -1;
                    viewerTableInd = new Integer(slices[4][1]);
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;
                        
                case 1: // Viewer disconnect
                    Message = "Inactive";
                    serverTableInd = -1;
                    viewerTableInd = new Integer(slices[4][1]);
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;

                case 2: // Server connect
                    Message = "Connection: code:" + slices[5][1] + ", Host:"
                            + getHost(slices[7][1]);
                    viewerTableInd = -1;
                    serverTableInd = new Integer(slices[4][1]);
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;

                case 3: // Server disconnect
                    Message = "Inactive";
                    viewerTableInd = -1;
                    serverTableInd = new Integer(slices[4][1]);
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;
                    
                case 4: // Viewer-Server session start
                    // Server message 1st
                    Message = "Active: code:" + slices[6][1] + ", Host:"
                            + getHost(slices[8][1]);
                    viewerTableInd = -1;
                    serverTableInd = new Integer(slices[4][1]);
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                        
                    // Viewer Message
                    Message = "Active: code:" + slices[6][1] + ", Host:"
                            + getHost(slices[9][1]);
                    serverTableInd = -1;
                    viewerTableInd = new Integer(slices[5][1]);
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;
                        
                case 5: // Viewer-Server session end
                    // Server message 1st
                    Message = "Inactive";
                    viewerTableInd = -1;
                    serverTableInd = new Integer(slices[4][1]);
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                        
                    // Viewer Message
                    Message = "Inactive";
                    serverTableInd = -1;
                    viewerTableInd = new Integer(slices[5][1]);
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;
                    
                case 6: // Repeater startup
                    Message = "Startup " + timeStamp.toString();
                    serverTableInd = lastRow;
                    viewerTableInd = -1;
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;
                        
                case 7: // Repeater shutdown
                    Message = "Shutdown " + timeStamp.toString();
                    serverTableInd = lastRow;
                    viewerTableInd = -1;
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;
                                    
                case 8: // Repeater heartbeat 
                    Message = "Heartbeat " + timeStamp.toString();
                    serverTableInd = lastRow;
                    viewerTableInd = -1;
                    postMsgToGui(serverTableInd, viewerTableInd, Message);
                    break;
                }                
            } catch (Exception e) {
                e.printStackTrace();
                System.exit(2);
            }
        }        
    }
    
    void postMsgToGui(final int srvInd, final int vwrInd, final String Message) {
        // Schedule a job for the event-dispatching thread:
        // updating table
        javax.swing.SwingUtilities.invokeLater(
                new Runnable() {
            public void run() {
                int row;
                int col;

                row = -1;
                col = -1;
        
                String Msg = new String(Message);

                if (srvInd != -1) {
                    col = 1;
                    row = srvInd;
                } else if (vwrInd != -1) {
                    col = 2;
                    row = vwrInd;
                }
                    
                System.out.println(Msg);
                
                if ((row != -1) && (col != -1)) {
                    if (row <= lastRow) {
                        tableToPost.getModel().setValueAt((Object) Msg, row, col);
                    }
                }
            }
        });
    }
    
    public void run() {
        try {
            ServerSocket serverSocket = new ServerSocket(listenPort, BACKLOG,
                    InetAddress.getByName(listenIp));

            while (true) {
                int nowEvents;
                
                // Forget previous events 
                nowEvents = 0;
                
                // Wait for repeater's connection
                Socket clientSocket = serverSocket.accept();

                // Open up IO stream
                In  in = new In(clientSocket);

                // Wait for data and read it in until connection dies
                String s;

                while ((s = in.readLine()) != null) {
                    events[nowEvents++] = s;
                }

                // Close stream and socket
                in.close();
                clientSocket.close();
                
                // Process events and send them to GUI table
                processEvents(nowEvents);
                
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}   // End of class ListenerThread


/**
 * EventListener is our main program, a modification of 
 * Java TableDemo example
 */
public class EventListener extends JPanel {
    public final int MAX_ROWS = 31;
    public final int MAX_COLS = 3;
    private boolean DEBUG = false;
    JTable table;
    
    public EventListener(String listenIp, int listenPort) {
        super(new GridLayout(1, 0));

        table = new JTable(new MyTableModel());
        table.setPreferredScrollableViewportSize(new Dimension(950, 600));
        
        // Create the scroll pane and add table to it.
        JScrollPane scrollPane = new JScrollPane(table);

        // Add the scroll pane to this panel.
        add(scrollPane);

        // Create listener thread to listen to repeater events
        ListenerThread thread = new ListenerThread(table, MAX_ROWS - 1, listenIp,
                listenPort);

        thread.start();    
    }

    class MyTableModel extends AbstractTableModel {
        
        private String[] columnNames = { "TableIndex", "Servers", "Viewers"};
        private String[][] data;

        MyTableModel() {
            data = new String[MAX_ROWS][MAX_COLS];    
            for (int ii = 0; ii < MAX_ROWS; ii++) {
                if (ii < MAX_ROWS - 1) {
                    data[ii][0] = new Integer(ii).toString();
                    data[ii][1] = "Inactive";
                    data[ii][2] = "Inactive";
                } else {
                    data[ii][0] = "Repeater status";
                    data[ii][1] = "No messages yet from Repeater";
                    data[ii][2] = "";
                }
            }
        }

        public int getColumnCount() {
            return MAX_COLS;
        }

        public int getRowCount() {
            return MAX_ROWS;
        }

        public String getColumnName(int col) {
            return columnNames[col];
        }

        public Object getValueAt(int row, int col) {
            return data[row][col];
        }

        /*
         * Don't need to implement this method unless your table's
         * editable.
         */
        public boolean isCellEditable(int row, int col) {
            return false;
        }

        /*
         * Don't need to implement this method unless your table's
         * data can change.
         */
        public void setValueAt(Object value, int row, int col) {
            if (DEBUG) {
                System.out.println(
                        "Setting value at " + row + "," + col + " to " + value
                        + " (an instance of " + value.getClass() + ")");
            }

            data[row][col] = (String) value;
            fireTableCellUpdated(row, col);

            if (DEBUG) {
                System.out.println("New value of data:");
                printDebugData();
            }
        }

        private void printDebugData() {
            int numRows = getRowCount();
            int numCols = getColumnCount();

            for (int i = 0; i < numRows; i++) {
                System.out.print("    row " + i + ":");
                for (int j = 0; j < numCols; j++) {
                    System.out.print("  " + data[i][j]);
                }
                System.out.println();
            }
            System.out.println("--------------------------");
        }
    }

    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event-dispatching thread.
     */
    private static void createAndShowGUI(String listenIp, int listenPort) {
        // Create and set up the window.
        JFrame frame = new JFrame("Repeater Event Listener");

        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        // Create and set up the content pane.
        EventListener newContentPane = new EventListener(listenIp, listenPort);

        newContentPane.setOpaque(true); // content panes must be opaque
        frame.setContentPane(newContentPane);

        // Display the window.
        frame.pack();
        frame.setVisible(true);
    }

    public static void main(String[] args) {
        final String listenIp;
        final int listenPort;

        try {
            if (args.length != 2) {
                System.out.println(
                        "Usage: java EventListener listenIp listenPort");
                System.out.println(
                        "listenIp: What interface we listen (default 0.0.0.0 == All interfaces)");
                System.out.println(
                        "listenPort: Port number where we listen (default 2002)");
                listenIp = new String("0.0.0.0");
                listenPort = 2002;
            } else {
                listenIp = new String(args[0]);
                listenPort = new Integer(args[1]);
            }
            
            // Schedule a job for the event-dispatching thread:
            // creating and showing this application's GUI.
            javax.swing.SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    createAndShowGUI(listenIp, listenPort);
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(3);
        }
    }
}
