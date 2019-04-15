package org.olsr.v1.info.proxy;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.LinkedList;
import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.olsr.v1.info.proxy.api.InfoResult;

import com.googlecode.miyamoto.AnnotationProxyBuilder;

@SuppressWarnings("resource")
public class TestOlsrdInfoProxyImpl {
  OlsrdInfoProxyImpl impl;
  MockOlsrdInfo      mockInfo;

  static boolean     doUnresolved     = false;

  static File        doUnresolvedFile = new File("testresources/doUnresolved");

  @BeforeClass
  static public void init() {
    doUnresolved = doUnresolvedFile.exists();
  }

  void fillProps(final AnnotationProxyBuilder<Config> config) {
    config.setProperty("address", this.mockInfo.address.getHostAddress());
    config.setProperty("localPort", Integer.valueOf(Config.LOCAL_PORT_DEFAULT));
    config.setProperty("portTxtInfo", Integer.valueOf(this.mockInfo.port));
    config.setProperty("portJsonInfo", Integer.valueOf(this.mockInfo.port));
    config.setProperty("connectionTimeout", Integer.valueOf(100));
    config.setProperty("socketTimeout", Integer.valueOf(Config.SOCKET_TIMEOUT_DEFAULT));
    config.setProperty("randomSleep", Integer.valueOf(Config.RANDOM_SLEEP_MIN));
    config.setProperty("connectionRetries", Integer.valueOf(1));
  }

  @Before
  public void setUp() throws Exception {
    this.mockInfo = new MockOlsrdInfo();

    this.impl = new OlsrdInfoProxyImpl();

    final AnnotationProxyBuilder<Config> config = AnnotationProxyBuilder.newBuilder(Config.class);
    this.fillProps(config);

    this.impl.activate(config.getProxedAnnotation());
    this.impl.modified(config.getProxedAnnotation());
  }

  @After
  public void tearDown() throws Exception {
    this.impl.deactivate();
    this.impl = null;
    this.mockInfo.stopAndCleanup();
  }

  @Test(timeout = 8000)
  public void testUnknownHostInConfig() {
    if (!doUnresolved) {
      return;
    }

    final InetSocketAddress addressTxtInfo = this.impl.addressTxtInfo;
    final InetSocketAddress addressJsonInfo = this.impl.addressJsonInfo;

    final AnnotationProxyBuilder<Config> config = AnnotationProxyBuilder.newBuilder(Config.class);
    this.fillProps(config);
    config.setProperty("address", "1234567890.0.0.1");
    this.impl.modified(config.getProxedAnnotation());

    assert (addressTxtInfo.equals(this.impl.addressTxtInfo));
    assert (addressJsonInfo.equals(this.impl.addressJsonInfo));
  }

  @Test(timeout = 8000)
  public void testModifiedLocalPort() {
    assertThat(Integer.valueOf(this.impl.usedPorts.size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.usedPorts.get(0), equalTo(Integer.valueOf(Config.LOCAL_PORT_DEFAULT)));

    final InetSocketAddress addressTxtInfo = this.impl.addressTxtInfo;
    final InetSocketAddress addressJsonInfo = this.impl.addressJsonInfo;

    final AnnotationProxyBuilder<Config> config = AnnotationProxyBuilder.newBuilder(Config.class);
    this.fillProps(config);
    config.setProperty("address", "12.0.0.1");
    final Integer newLocalPort = Integer.valueOf(Integer.reverse(Config.LOCAL_PORT_DEFAULT));
    config.setProperty("localPort", newLocalPort);
    this.impl.modified(config.getProxedAnnotation());

    assertThat(Integer.valueOf(this.impl.usedPorts.size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.usedPorts.get(0), equalTo(newLocalPort));
    assert (!addressTxtInfo.equals(this.impl.addressTxtInfo));
    assert (!addressJsonInfo.equals(this.impl.addressJsonInfo));
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendInternalAssert1() throws IOException {
    this.impl.sendInternal("/sgw", null, null);
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendInternalAssert2() throws IOException {
    final List<String> result = new LinkedList<>();
    final StringBuilder jsonInfo = new StringBuilder();
    this.impl.sendInternal("/sgw", result, jsonInfo);
  }

  @Test(timeout = 8000, expected = IOException.class)
  public void testSendInternalNullCommand() throws IOException {
    final List<String> result = new LinkedList<>();
    this.impl.sendInternal(null, result, null);

    assert (false);
  }

  @Test(timeout = 8000, expected = IOException.class)
  public void testSendInternalEmptyCommand() throws IOException {
    final List<String> result = new LinkedList<>();
    this.impl.sendInternal("", result, null);

    assert (false);
  }

  @Test(timeout = 8000)
  public void testSendInternalBrokenConnectionOnConnectTxtInfo() {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionOnConnection = true;

    int rr = HttpURLConnection.HTTP_OK;
    final List<String> result = new LinkedList<>();
    try {
      rr = this.impl.sendInternal("/sgw", result, null);
    }
    catch (final IOException e) {
      /* is ok */
    }

    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Integer.valueOf(result.size()), equalTo(Integer.valueOf(0)));
  }

  @Test(timeout = 8000)
  public void testSendInternalBrokenConnectionOnConnectJsonInfo() {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.json");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionOnConnection = true;

    int rr = HttpURLConnection.HTTP_OK;
    final StringBuilder result = new StringBuilder();
    try {
      rr = this.impl.sendInternal("/sgw", null, result);
    }
    catch (final IOException e) {
      /* is ok */
    }
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Integer.valueOf(result.length()), equalTo(Integer.valueOf(0)));
  }

  @Test(timeout = 8000)
  public void testSendInternalOk() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final List<String> result = new LinkedList<>();
    final int rr = this.impl.sendInternal("/sgw", result, null);

    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(result, notNullValue());

    final List<String> expected = Helpers.readFile(file);
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testGetSocketAlreadyBound() throws IOException {
    final Socket s = new Socket();
    s.bind(null);
    final int takenLocalPort = s.getLocalPort();

    final AnnotationProxyBuilder<Config> config = AnnotationProxyBuilder.newBuilder(Config.class);
    this.fillProps(config);
    config.setProperty("localPort", Integer.valueOf(takenLocalPort));

    this.impl.modified(config.getProxedAnnotation());

    final Socket socket = this.impl.getSocket(1000);

    assertThat(socket, notNullValue());
    final int socketPort = socket.getLocalPort();

    assertThat(Integer.valueOf(this.impl.usedPorts.size()), equalTo(Integer.valueOf(2)));
    assertThat(this.impl.usedPorts.get(0), equalTo(Integer.valueOf(takenLocalPort)));
    assertThat(this.impl.usedPorts.get(1), equalTo(Integer.valueOf(socketPort)));
  }

  @Test(timeout = 8000)
  public void testGetTxtInfoOk() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final InfoResult infoResult = this.impl.getTxtInfo("/sgw");

    assertThat(infoResult, notNullValue());

    final int rr = infoResult.status;
    final List<String> result = infoResult.output;

    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(result, notNullValue());

    final List<String> expected = Helpers.readFile(file);
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testGetJsonInfoOk() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.json");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final InfoResult infoResult = this.impl.getJsonInfo("/sgw");

    assertThat(infoResult, notNullValue());

    final int rr = infoResult.status;
    final List<String> result = infoResult.output;

    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(result, notNullValue());
    assertThat(Integer.valueOf(result.size()), equalTo(Integer.valueOf(1)));

    final String expected = TestOlsrdInfoReader.convertListToStringBuilder(Helpers.readFile(file)).toString();
    assertThat(result.get(0), equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testGetTxtInfoNoConnection() throws IOException {
    this.mockInfo.stopAndCleanup();

    final InfoResult infoResult = this.impl.getTxtInfo("/sgw");

    assertThat(infoResult, notNullValue());

    final int rr = infoResult.status;
    final List<String> result = infoResult.output;

    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_UNAVAILABLE)));
    assertThat(result, notNullValue());

    final List<String> expected = new LinkedList<>();
    assertThat(result, equalTo(expected));
  }
}