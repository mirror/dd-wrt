package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoNeighborsEntry {
  private JsonInfoNeighborsEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoNeighborsEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    final int r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final boolean r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    final int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1118777225)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}