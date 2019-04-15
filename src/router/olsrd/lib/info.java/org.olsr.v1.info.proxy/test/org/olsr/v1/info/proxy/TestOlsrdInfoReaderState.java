package org.olsr.v1.info.proxy;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.Test;

@SuppressWarnings("static-method")
public class TestOlsrdInfoReaderState {
  @Test(timeout = 8000)
  public void testInstance() {
    assertThat(OlsrdInfoReaderState.INIT, notNullValue());
  }

  @Test(timeout = 8000)
  public void testValueOf() {
    assertThat(OlsrdInfoReaderState.valueOf("INIT"), equalTo(OlsrdInfoReaderState.INIT));
  }
}