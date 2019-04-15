package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoMessageTimes {
  private JsonInfoMessageTimes impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoMessageTimes();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Long.valueOf(this.impl.getHello()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getTc()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getMid()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getHna()), equalTo(Long.valueOf(0)));

    /* set */
    this.impl.setHello(1);
    this.impl.setTc(2);
    this.impl.setMid(3);
    this.impl.setHna(4);

    /* get */
    assertThat(Long.valueOf(this.impl.getHello()), equalTo(Long.valueOf(1)));
    assertThat(Long.valueOf(this.impl.getTc()), equalTo(Long.valueOf(2)));
    assertThat(Long.valueOf(this.impl.getMid()), equalTo(Long.valueOf(3)));
    assertThat(Long.valueOf(this.impl.getHna()), equalTo(Long.valueOf(4)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoMessageTimes other = new JsonInfoMessageTimes();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    /* hello */

    long longOrg = this.impl.getHello();

    this.impl.setHello(1);
    other.setHello(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setHello(2);
    other.setHello(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setHello(1);
    other.setHello(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setHello(longOrg);
    other.setHello(longOrg);

    /* tc */

    longOrg = this.impl.getTc();

    this.impl.setTc(1);
    other.setTc(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setTc(2);
    other.setTc(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setTc(1);
    other.setTc(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setTc(longOrg);
    other.setTc(longOrg);

    /* mid */

    longOrg = this.impl.getMid();

    this.impl.setMid(1);
    other.setMid(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setMid(2);
    other.setMid(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setMid(1);
    other.setMid(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setMid(longOrg);
    other.setMid(longOrg);

    /* hna */

    longOrg = this.impl.getHna();

    this.impl.setHna(1);
    other.setHna(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setHna(2);
    other.setHna(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setHna(1);
    other.setHna(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setHna(longOrg);
    other.setHna(longOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoMessageTimes other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoMessageTimes();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setHello(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(923521)));

    /* set */
    this.impl.setHello(1);
    this.impl.setTc(2);
    this.impl.setMid(3);
    this.impl.setHna(4);

    r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(955331)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}