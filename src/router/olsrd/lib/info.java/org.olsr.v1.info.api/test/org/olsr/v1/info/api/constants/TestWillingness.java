package org.olsr.v1.info.api.constants;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.junit.Assert.assertThat;

import org.junit.Test;
import org.olsr.v1.info.api.contants.Willingness;

@SuppressWarnings("static-method")
public class TestWillingness {
  @Test(timeout = 8000)
  public void testValueOf() {
    assertThat(Willingness.valueOf("NEVER"), equalTo(Willingness.NEVER));
  }

  @Test(timeout = 8000)
  public void testGetValue() {

    assertThat(Integer.valueOf(Willingness.NEVER.getValue()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(Willingness.LOW.getValue()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(Willingness.DEFAULT.getValue()), equalTo(Integer.valueOf(3)));
    assertThat(Integer.valueOf(Willingness.HIGH.getValue()), equalTo(Integer.valueOf(6)));
    assertThat(Integer.valueOf(Willingness.ALWAYS.getValue()), equalTo(Integer.valueOf(7)));
    assertThat(Integer.valueOf(Willingness.UNKNOWN.getValue()), equalTo(Integer.valueOf(-1)));
  }

  @Test(timeout = 8000)
  public void testFromValue() {
    assertThat(Willingness.fromValue(0), equalTo(Willingness.NEVER));
    assertThat(Willingness.fromValue(1), equalTo(Willingness.LOW));
    assertThat(Willingness.fromValue(3), equalTo(Willingness.DEFAULT));
    assertThat(Willingness.fromValue(6), equalTo(Willingness.HIGH));
    assertThat(Willingness.fromValue(7), equalTo(Willingness.ALWAYS));
    assertThat(Willingness.fromValue(-1), equalTo(Willingness.UNKNOWN));

    assertThat(Willingness.fromValue(10000), equalTo(Willingness.UNKNOWN));
  }
}