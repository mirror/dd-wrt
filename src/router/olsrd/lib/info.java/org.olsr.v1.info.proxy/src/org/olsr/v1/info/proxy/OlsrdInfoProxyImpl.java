package org.olsr.v1.info.proxy;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.List;
import java.util.Random;
import java.util.concurrent.CopyOnWriteArrayList;

import org.olsr.v1.info.proxy.api.InfoResult;
import org.olsr.v1.info.proxy.api.OlsrdInfoProxy;
import org.osgi.service.component.annotations.Activate;
import org.osgi.service.component.annotations.Component;
import org.osgi.service.component.annotations.Deactivate;
import org.osgi.service.component.annotations.Modified;
import org.osgi.service.metatype.annotations.Designate;

@Component
@Designate(ocd = Config.class)
public class OlsrdInfoProxyImpl implements OlsrdInfoProxy {
  /*
   * Config
   */

  private final Object          configLock      = new Object();
  private Config                config          = null;
  InetSocketAddress             addressTxtInfo  =
      new InetSocketAddress(Config.ADDRESS_DEFAULT, Config.PORT_TXTINFO_DEFAULT);
  InetSocketAddress             addressJsonInfo =
      new InetSocketAddress(Config.ADDRESS_DEFAULT, Config.PORT_JSONINFO_DEFAULT);

  CopyOnWriteArrayList<Integer> usedPorts       = new CopyOnWriteArrayList<>();

  private final Random          random          = new Random();

  /* MUST be called in a synchronized (configLock) block */
  private Config setupConfig(final Config config) {
    final Config preConfig = this.config;
    this.config = config;

    final InetSocketAddress sa = new InetSocketAddress(this.config.address(), 0);
    if (!sa.isUnresolved()) {
      this.addressTxtInfo = new InetSocketAddress(this.config.address(), this.config.portTxtInfo());
      this.addressJsonInfo = new InetSocketAddress(this.config.address(), this.config.portJsonInfo());
    }

    return preConfig;
  }

  /*
   * Lifecycle
   */

  @Activate
  void activate(final Config config) {
    synchronized (this.configLock) {
      this.setupConfig(config);
      this.usedPorts.clear();
      this.usedPorts.add(Integer.valueOf(this.config.localPort()));
    }
  }

  @Deactivate
  void deactivate() {
    /* nothing to do */
  }

  @Modified
  void modified(final Config config) {
    Integer preLocalPort;
    Integer localPort;
    synchronized (this.configLock) {
      preLocalPort = Integer.valueOf(this.config.localPort());
      this.setupConfig(config);
      localPort = Integer.valueOf(this.config.localPort());
    }

    if (!localPort.equals(preLocalPort)) {
      this.usedPorts.remove(preLocalPort);
      this.usedPorts.add(0, localPort);
    }
  }

  /*
   * Helpers
   */

  Socket getSocket(final int socketTimeout) {
    Socket socket = null;

    for (final Integer port : this.usedPorts) {
      final SocketAddress bindpoint = new InetSocketAddress(port.intValue());

      socket = new Socket();
      try {
        socket.setSoTimeout(socketTimeout);
        socket.setReuseAddress(true);
        socket.bind(bindpoint);
      }
      catch (final IOException e) {
        /* skip to next port */
        socket = null;
      }
    }

    if (socket == null) {
      socket = new Socket();
      try {
        socket.setSoTimeout(socketTimeout);
        socket.setReuseAddress(true);
        socket.bind(null);
        final Integer localPort = Integer.valueOf(socket.getLocalPort());
        this.usedPorts.addIfAbsent(localPort);
      }
      catch (final IOException e) {
        /* swallow & can't be covered in a test */
      }
    }

    assert (socket != null);
    return socket;
  }

  public int sendInternal(final String command, final List<String> txtInfo, final StringBuilder jsonInfo)
      throws IOException {
    assert (!((txtInfo == null) && (jsonInfo == null)));
    assert (!((txtInfo != null) && (jsonInfo != null)));

    if ((command == null) //
        || command.isEmpty()) {
      throw new IOException("No command specified");
    }

    int socketTimeout;
    int connectionTimeout;
    InetSocketAddress address;
    int randomSleep;
    int retries;
    synchronized (this.configLock) {
      socketTimeout = this.config.socketTimeout();
      connectionTimeout = this.config.connectionTimeout();
      address = (txtInfo != null) ? this.addressTxtInfo : this.addressJsonInfo;
      randomSleep = this.config.randomSleep();
      retries = this.config.connectionRetries();
    }

    int retry = 0;
    while (retry <= retries) {
      try (Socket socket = this.getSocket(socketTimeout)) {
        final int r =
            OlsrdInfoReader.send(socket, command, connectionTimeout, socketTimeout, address, txtInfo, jsonInfo);
        if (r != HttpURLConnection.HTTP_UNAVAILABLE) {
          return r;
        }
      }

      try {
        Thread.sleep(Math.max(Config.RANDOM_SLEEP_MIN, this.random.nextInt(randomSleep)));
      }
      catch (final InterruptedException e) {
        /* swallow & can't be covered in a test */
        retry = retries;
      }

      retry++;
    }

    return HttpURLConnection.HTTP_UNAVAILABLE;
  }

  /*
   * OlsrdInfoProxy
   */

  @Override
  public InfoResult getTxtInfo(final String command) throws IOException {
    final InfoResult result = new InfoResult();
    result.status = this.sendInternal(command, result.output, null);
    return result;
  }

  @Override
  public InfoResult getJsonInfo(final String command) throws IOException {
    final InfoResult result = new InfoResult();
    final StringBuilder sb = new StringBuilder();
    result.status = this.sendInternal(command, null, sb);
    result.output.add(sb.toString());
    return result;
  }
}