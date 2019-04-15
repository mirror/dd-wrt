package org.olsr.v1.info.proxy;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketException;
import java.util.LinkedList;
import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

@SuppressWarnings("static-method")
public class TestOlsrdInfoReader {
  MockOlsrdInfo     mockInfo;

  InetSocketAddress addr = null;

  @Before
  public void setUp() throws Exception {
    this.mockInfo = new MockOlsrdInfo();
    this.addr = new InetSocketAddress(this.mockInfo.address, this.mockInfo.port);
  }

  @After
  public void tearDown() throws Exception {
    this.mockInfo.stopAndCleanup();
  }

  static StringBuilder convertListToStringBuilder(final List<String> expected) {
    if (expected == null) {
      return null;
    }

    final StringBuilder r = new StringBuilder();
    for (final String exp : expected) {
      r.append(exp);
      r.append(OlsrdInfoReader.EOL);
    }

    return r;
  }

  Socket getSocket() {
    final Socket socket = new Socket();
    try {
      socket.setSoTimeout(500);
      socket.setReuseAddress(true);
      socket.bind(null);
    }
    catch (final IOException e) {
      /* swallow */
    }
    return socket;
  }

  @Test(timeout = 8000)
  public void testInstance() {
    final OlsrdInfoReader reader = new OlsrdInfoReader();
    assertThat(reader, notNullValue());
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendAssert1() throws IOException {
    final List<String> result = new LinkedList<>();
    final StringBuilder resultBuilder = new StringBuilder();
    final int rr = OlsrdInfoReader.send(null, "/sgw", 500, 500,
        new InetSocketAddress(this.mockInfo.address, this.mockInfo.port), result, resultBuilder);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendAssert2() throws IOException {
    final List<String> result = new LinkedList<>();
    final StringBuilder resultBuilder = new StringBuilder();
    try (Socket socket = this.getSocket()) {
      final int rr = OlsrdInfoReader.send(socket, null, 500, 500,
          new InetSocketAddress(this.mockInfo.address, this.mockInfo.port), result, resultBuilder);
      assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    }
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendAssert3() throws IOException {
    final List<String> result = new LinkedList<>();
    final StringBuilder resultBuilder = new StringBuilder();
    try (Socket socket = this.getSocket()) {
      final int rr = OlsrdInfoReader.send(socket, "", 500, 500,
          new InetSocketAddress(this.mockInfo.address, this.mockInfo.port), result, resultBuilder);
      assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    }
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendAssert3a() throws IOException {
    final List<String> result = new LinkedList<>();
    final StringBuilder resultBuilder = new StringBuilder();
    try (Socket socket = this.getSocket()) {
      final int rr = OlsrdInfoReader.send(socket, "sgw", -1, 500,
          new InetSocketAddress(this.mockInfo.address, this.mockInfo.port), result, resultBuilder);
      assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    }
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendAssert3b() throws IOException {
    final List<String> result = new LinkedList<>();
    final StringBuilder resultBuilder = new StringBuilder();
    try (Socket socket = this.getSocket()) {
      final int rr = OlsrdInfoReader.send(socket, "sgw", 500, -1,
          new InetSocketAddress(this.mockInfo.address, this.mockInfo.port), result, resultBuilder);
      assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    }
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendAssert4() throws IOException {
    final List<String> result = new LinkedList<>();
    final StringBuilder resultBuilder = new StringBuilder();
    try (Socket socket = this.getSocket()) {
      final int rr = OlsrdInfoReader.send(socket, "/sgw", 500, 500, null, result, resultBuilder);
      assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    }
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendAssert5() throws IOException {
    try (Socket socket = this.getSocket()) {
      final int rr = OlsrdInfoReader.send(socket, "/sgw", 500, 500,
          new InetSocketAddress(this.mockInfo.address, this.mockInfo.port), null, null);
      assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    }
  }

  @Test(timeout = 8000, expected = AssertionError.class)
  public void testSendAssert6() throws IOException {
    final List<String> result = new LinkedList<>();
    final StringBuilder resultBuilder = new StringBuilder();
    try (Socket socket = this.getSocket()) {
      final int rr = OlsrdInfoReader.send(socket, "/sgw", 500, 500,
          new InetSocketAddress(this.mockInfo.address, this.mockInfo.port), result, resultBuilder);
      assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    }
  }

  @Test(timeout = 8000)
  public void testSendNoConnection() throws IOException {
    this.mockInfo.stopAndCleanup();

    final List<String> result = new LinkedList<>();

    final int rr = OlsrdInfoReader.send(this.getSocket(), "/sgw", 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_UNAVAILABLE)));
    assertThat(Boolean.valueOf(result.isEmpty()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnConnect() throws Exception {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionOnConnection = true;

    final List<String> result = new LinkedList<>();

    try {
      final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
      assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    }
    catch (final SocketException e) {
      /* is ok */
    }
    assertThat(Boolean.valueOf(result.isEmpty()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnCommand() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionOnCommandReceived = true;

    final List<String> result = new LinkedList<>();

    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Boolean.valueOf(result.isEmpty()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnLine1() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionBeforeLine = 1;

    final List<String> result = new LinkedList<>();

    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Boolean.valueOf(result.isEmpty()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnLine2() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionBeforeLine = 2;

    final List<String> result = new LinkedList<>();

    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Boolean.valueOf(result.isEmpty()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnLine3ResultList() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionBeforeLine = 3;

    final List<String> result = new LinkedList<>();

    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Boolean.valueOf(result.isEmpty()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnLine3ResultBuilder() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionBeforeLine = 3;

    final StringBuilder result = new StringBuilder();

    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, null, result);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Boolean.valueOf(result.length() == 0), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnLine13() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionBeforeLine = 13;

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Integer.valueOf(result.size()), equalTo(Integer.valueOf(0)));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnLine14() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionBeforeLine = 14;

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    assertThat(Integer.valueOf(result.size()), equalTo(Integer.valueOf(0)));
  }

  @Test(timeout = 8000)
  public void testSendBrokenConnectionOnLine15() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);
    this.mockInfo.breakConnectionBeforeLine = 15;

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    final List<String> expected = new LinkedList<>();
    expected.add("Table: Smart Gateway IPv4");
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testTxtSendOkResultList() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    assertThat(result, notNullValue());

    final List<String> expected = Helpers.readFile(file);
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testTxtSendOkResultListNoHeaders() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw-no-headers.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    assertThat(result, notNullValue());

    final List<String> expected = Helpers.readFile(file);
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testTxtSendOkResultBuilder() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final StringBuilder result = new StringBuilder();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, null, result);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    assertThat(result, notNullValue());

    final StringBuilder expected = TestOlsrdInfoReader.convertListToStringBuilder(Helpers.readFile(file));
    assertThat(result.toString(), equalTo(expected.toString()));
  }

  @Test(timeout = 8000)
  public void testTxtSendOkResultBuilderNoHeaders() throws IOException {
    final String command = "/sgw";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/sgw-no-headers.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final StringBuilder result = new StringBuilder();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, null, result);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    assertThat(result, notNullValue());

    final StringBuilder expected = TestOlsrdInfoReader.convertListToStringBuilder(Helpers.readFile(file));
    assertThat(result.toString(), equalTo(expected.toString()));
  }

  @Test(timeout = 8000)
  public void testTxtSendNotFoundResultList() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/dummy.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_NOT_FOUND)));

    assertThat(result, notNullValue());

    final List<String> expected = new LinkedList<>();
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testTxtSendNotFoundResultListNoHeaders() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/dummy-no-headers.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    assertThat(result, notNullValue());

    final List<String> expected = Helpers.readFile(file);
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testTxtSendNotFoundResultBuilder() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/dummy.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final StringBuilder result = new StringBuilder();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, null, result);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_NOT_FOUND)));

    assertThat(result, notNullValue());

    final StringBuilder expected = new StringBuilder();
    assertThat(result.toString(), equalTo(expected.toString()));
  }

  @Test(timeout = 8000)
  public void testTxtSendNotFoundResultBuilderNoHeaders() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/dummy-no-headers.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final StringBuilder result = new StringBuilder();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, null, result);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    assertThat(result, notNullValue());

    final StringBuilder expected = TestOlsrdInfoReader.convertListToStringBuilder(Helpers.readFile(file));
    assertThat(result.toString(), equalTo(expected.toString()));
  }

  @Test(timeout = 8000)
  public void testTxtSendNoContentResultList() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/no-content.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_NO_CONTENT)));

    assertThat(result, notNullValue());

    final List<String> expected = new LinkedList<>();
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testTxtSendNoContentResultListNoHeaders() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/no-content-no-headers.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    assertThat(result, notNullValue());

    final List<String> expected = new LinkedList<>();
    assertThat(result, equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testTxtSendNoContentResultBuilder() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/no-content.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final StringBuilder result = new StringBuilder();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, null, result);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_NO_CONTENT)));

    assertThat(result, notNullValue());

    final StringBuilder expected = new StringBuilder();
    assertThat(result.toString(), equalTo(expected.toString()));
  }

  @Test(timeout = 8000)
  public void testTxtSendNoContentResultBuilderNoHeaders() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/no-content-no-headers.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final StringBuilder result = new StringBuilder();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, null, result);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));

    assertThat(result, notNullValue());

    final StringBuilder expected = new StringBuilder();
    assertThat(result.toString(), equalTo(expected.toString()));
  }

  @Test(timeout = 8000)
  public void testTxtSendNFEResultList() throws IOException {
    final String command = "/dummy";
    final List<File> files = new LinkedList<>();
    final File file = new File("testresources/no-content-nfe.txt");
    files.add(file);

    this.mockInfo.responses.put(command, files);

    final List<String> result = new LinkedList<>();
    final int rr = OlsrdInfoReader.send(this.getSocket(), command, 500, 500, this.addr, result, null);
    assertThat(Integer.valueOf(rr), equalTo(Integer.valueOf(HttpURLConnection.HTTP_INTERNAL_ERROR)));

    assertThat(result, notNullValue());

    final List<String> expected = new LinkedList<>();
    assertThat(result, equalTo(expected));
  }
}