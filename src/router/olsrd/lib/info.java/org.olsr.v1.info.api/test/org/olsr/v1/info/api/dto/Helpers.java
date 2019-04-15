package org.olsr.v1.info.api.dto;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;

import org.junit.Ignore;

import com.fasterxml.jackson.databind.ObjectMapper;

@Ignore
public class Helpers {
  public static final ObjectMapper objectMapper = new ObjectMapper();

  public static String readFile(final File f) throws IOException {
    try (BufferedReader reader =
        new BufferedReader(new InputStreamReader(new FileInputStream(f), Charset.defaultCharset()))) {
      final StringBuilder sb = new StringBuilder();

      String line;
      while ((line = reader.readLine()) != null) {
        sb.append(line);
        sb.append("\n");
      }

      return sb.toString();
    }
  }
}