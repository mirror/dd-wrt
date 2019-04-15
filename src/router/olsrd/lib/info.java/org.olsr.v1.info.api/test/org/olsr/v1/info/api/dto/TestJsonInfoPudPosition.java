package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoPudPosition {
  private JsonInfoPudPosition impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoPudPosition();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getPudPosition(), equalTo(new JsonInfoPudPositionEntry()));

    /* set */
    final JsonInfoPudPositionEntry entry = new JsonInfoPudPositionEntry();
    entry.setFix("dummy");
    this.impl.setPudPosition(entry);

    /* get */
    assertThat(this.impl.getPudPosition(), equalTo(entry));

    this.impl.setPudPosition(null);

    assertThat(this.impl.getPudPosition(), equalTo(new JsonInfoPudPositionEntry()));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoPudPosition other;

    /* self */

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* null */

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* wrong object */

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* same */

    other = new JsonInfoPudPosition();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* pudPosition */

    other = new JsonInfoPudPosition();
    final JsonInfoPudPositionEntry entry = new JsonInfoPudPositionEntry();
    entry.setFix("dummy");
    other.setPudPosition(entry);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1805278597)));

    /* set */
    final JsonInfoPudPositionEntry entry = new JsonInfoPudPositionEntry();
    entry.setFix("dummy");
    this.impl.setPudPosition(entry);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(2056706658)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}