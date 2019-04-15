package org.olsr.v1.info.proxy.api;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.HttpURLConnection;
import java.util.List;

import org.junit.Test;

@SuppressWarnings("static-method")
public class TestInfoResult {
  @Test(timeout = 8000)
  public void testInitial() {
    final InfoResult r = new InfoResult();

    assertThat(Integer.valueOf(r.status), equalTo(Integer.valueOf(HttpURLConnection.HTTP_OK)));
    final List<String> o = r.output;
    assertThat(o, notNullValue());
    assertThat(Integer.valueOf(o.size()), equalTo(Integer.valueOf(0)));
  }
}