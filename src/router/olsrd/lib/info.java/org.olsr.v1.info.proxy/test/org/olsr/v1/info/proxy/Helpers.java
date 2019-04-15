package org.olsr.v1.info.proxy;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import org.junit.Ignore;

@Ignore
public class Helpers {

  enum STATE {
    INIT, IN_HEADER, IN_CONTENT;
  }

  static List<String> readFile(final File file) throws IOException {
    STATE state = STATE.INIT;
    final List<String> resultList = new LinkedList<>();
    try (BufferedReader fileReader = new BufferedReader(new FileReader(file))) {
      String line;
      while ((line = fileReader.readLine()) != null) {

        switch (state) {
          case INIT:
            if (line.trim().isEmpty()) {
              continue;
            }

            if (OlsrdInfoReader.PATTERN_HTTP_HEADER.matcher(line).matches()) {
              state = STATE.IN_HEADER;
              continue;
            }

            state = STATE.IN_CONTENT;
            break;

          case IN_HEADER:
            if (line.trim().isEmpty()) {
              state = STATE.IN_CONTENT;
            }
            continue;

          case IN_CONTENT:
          default:
            break;
        }

        resultList.add(line);
      }
    }
    return resultList;
  }
}