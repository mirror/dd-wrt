package org.olsr.v1.info.api.constants;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.Test;
import org.olsr.v1.info.api.contants.OlsrdConstants;

@SuppressWarnings("static-method")
public class TestOlsrdConstants {
  @Test(timeout = 8000)
  public void test() {
    final OlsrdConstants constants = new OlsrdConstants();
    assertThat(constants, notNullValue());
    assertThat(constants.toString(), notNullValue());

    assertThat(Long.valueOf(OlsrdConstants.ROUTE_COST_BROKEN), equalTo(Long.valueOf(0xffffffffL)));
  }
}