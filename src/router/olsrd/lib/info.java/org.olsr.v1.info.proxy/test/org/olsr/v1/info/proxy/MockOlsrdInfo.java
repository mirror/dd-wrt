package org.olsr.v1.info.proxy;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

import org.junit.Ignore;

/*
 * Fakes the olsrd info plugin
 */
@Ignore
public class MockOlsrdInfo extends Thread {
  private ServerSocket           socket                           = null;
  private Thread                 thread                           = null;
  private final AtomicBoolean    run                              = new AtomicBoolean(true);
  private final AtomicBoolean    running                          = new AtomicBoolean(false);

  public InetAddress             address;
  public int                     port                             = -1;
  public boolean                 breakConnectionOnConnection      = false;
  public boolean                 breakConnectionOnCommandReceived = false;
  public int                     breakConnectionBeforeLine        = Integer.MAX_VALUE;
  public Map<String, List<File>> responses                        = new HashMap<>();

  public MockOlsrdInfo() throws IOException {
    final SocketAddress sa = new InetSocketAddress(InetAddress.getByName("127.0.0.1"), 0);
    this.socket = new ServerSocket();
    this.socket.bind(sa);
    this.socket.setSoTimeout(100);
    this.address = this.socket.getInetAddress();
    this.port = this.socket.getLocalPort();

    this.thread = new Thread(this);
    this.thread.setName(this.getClass().getSimpleName());
    this.thread.start();
    while (!this.running.get()) {
      try {
        Thread.sleep(1);
      }
      catch (final InterruptedException e) {
        /* swallow */
      }
    }
  }

  public void stopAndCleanup() throws IOException {
    if (this.run.compareAndSet(true, false)) {
      this.thread.interrupt();
      try {
        this.thread.join(1000);
      }
      catch (final InterruptedException e) {
        /* swallow */
      }
      this.thread = null;
      this.socket.close();
      this.port = -1;
      this.socket = null;
    }
  }

  @Override
  public void run() {
    this.running.compareAndSet(false, true);
    while (this.run.get()) {
      try (Socket connection = this.socket.accept()) {
        if (connection == null) {
          continue;
        }
        if (this.breakConnectionOnConnection) {
          this.run.compareAndSet(true, false);
          break;
        }

        try (BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(connection.getOutputStream()))) {
          final String command = reader.readLine();
          if (this.breakConnectionOnCommandReceived) {
            this.run.compareAndSet(true, false);
            break;
          }
          final List<File> responses = this.responses.get(command);
          if ((responses != null) && (responses.size() > 0)) {
            final File response = responses.remove(0);
            try (BufferedReader fileReader = new BufferedReader(new FileReader(response))) {
              int lineNr = 0;
              String line;
              while ((line = fileReader.readLine()) != null) {
                lineNr++;
                if (lineNr >= this.breakConnectionBeforeLine) {
                  this.run.compareAndSet(true, false);
                  break;
                }
                writer.write(line);
                writer.newLine();
              }

            }
          }
        }
        catch (final Exception e) {
          e.printStackTrace();
        }
      }
      catch (final Exception e) {
        /* swallow */
      }
    }
    this.running.compareAndSet(true, false);
  }
}
