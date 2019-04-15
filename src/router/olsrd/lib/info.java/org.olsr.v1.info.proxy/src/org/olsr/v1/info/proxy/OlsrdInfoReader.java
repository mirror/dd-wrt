package org.olsr.v1.info.proxy;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.charset.Charset;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class OlsrdInfoReader {
  public static final Charset CHARSET             = Charset.forName("UTF-8");

  public static final String  EOL                 = "\n";

  public static final Pattern PATTERN_HTTP_HEADER = Pattern.compile("^\\s*(HTTP/1.[01]\\s+)(\\d+)\\s+(.+)\\s*$");

  /**
   * Send a request and read the response.
   *
   * The result is always cleared before starting the connection.
   *
   * @param socket The socket to use
   * @param command The request to send, a newline is always appended
   * @param connectionTimeout The connection timeout in msec
   * @param timeout The read timeout in msec
   * @param address The address to send to
   * @param resultList Non-null to put the response in here (resultBuilder must be null)
   * @param resultBuilder Non-null to put the response in here (resultList must be null)
   * @return The HTTP status of the request. HttpURLConnection.HTTP_UNAVAILABLE when the connection can't be
   *         established, HttpURLConnection.HTTP_NOT_FOUND when the request is invalid,
   *         HttpURLConnection.HTTP_NO_CONTENT when the response is empty, HttpURLConnection.HTTP_OK otherwise
   * @throws IOException (SocketException, IOException, SocketTimeoutException, IllegalBlockingModeException)
   */
  public static int send(final Socket socket, final String command, final int connectionTimeout, final int timeout,
      final InetSocketAddress address, final List<String> resultList, final StringBuilder resultBuilder)
      throws IOException {
    assert (socket != null);
    assert (command != null);
    assert (!command.isEmpty());
    assert (connectionTimeout >= 0);
    assert (timeout >= 0);
    assert (address != null);
    assert (!((resultList == null) && (resultBuilder == null)));
    assert (!((resultList != null) && (resultBuilder != null)));

    if (resultList != null) {
      resultList.clear();
    } else {
      resultBuilder.setLength(0);
    }

    socket.setSoTimeout(timeout);

    try {
      socket.connect(address, connectionTimeout);
    }
    catch (final IOException e) {
      return HttpURLConnection.HTTP_UNAVAILABLE;
    }

    int status = HttpURLConnection.HTTP_OK;
    try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), CHARSET));
        BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream(), CHARSET))) {
      writer.write(command);
      writer.newLine();
      writer.flush();

      OlsrdInfoReaderState state = OlsrdInfoReaderState.INIT;
      String line;
      while ((line = reader.readLine()) != null) {
        switch (state) {
          case INIT:
            if (line.trim().isEmpty()) {
              continue;
            }

            final Matcher m = PATTERN_HTTP_HEADER.matcher(line);
            if (m.matches()) {
              try {
                status = Integer.parseInt(m.group(2));
              }
              catch (final NumberFormatException e) {
                status = HttpURLConnection.HTTP_INTERNAL_ERROR;
              }

              switch (status) {
                case HttpURLConnection.HTTP_OK:
                  break;

                case HttpURLConnection.HTTP_NO_CONTENT:
                case HttpURLConnection.HTTP_FORBIDDEN:
                case HttpURLConnection.HTTP_NOT_FOUND:
                case HttpURLConnection.HTTP_CLIENT_TIMEOUT:
                case HttpURLConnection.HTTP_REQ_TOO_LONG:
                case HttpURLConnection.HTTP_INTERNAL_ERROR:
                default:
                  if (resultList != null) {
                    resultList.clear();
                  } else {
                    resultBuilder.setLength(0);
                  }
                  return status;
              }

              state = OlsrdInfoReaderState.IN_HEADER;
              continue;
            }

            state = OlsrdInfoReaderState.IN_CONTENT;
            break;

          case IN_HEADER:
            if (line.trim().isEmpty()) {
              state = OlsrdInfoReaderState.IN_CONTENT;
            }
            continue;

          case IN_CONTENT:
          default:
            break;
        }

        if (resultList != null) {
          resultList.add(line);
        } else {
          resultBuilder.append(line);
          resultBuilder.append(EOL);
        }
      }
    }
    return status;
  }
}