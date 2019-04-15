package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

@SuppressWarnings("static-method")
public class TestJsonInfoMid {
  private JsonInfoMid impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoMid();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getMid(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getMid().isEmpty()), equalTo(Boolean.TRUE));

    /* set */
    final List<JsonInfoMidEntry> mid = new LinkedList<>();
    final JsonInfoMidEntry entry = new JsonInfoMidEntry();
    final JsonInfoTimedIpAddress main = new JsonInfoTimedIpAddress();
    entry.setMain(main);
    mid.add(entry);
    this.impl.setMid(mid);

    /* get */
    assertThat(this.impl.getMid(), equalTo(mid));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    final Object otherNull = null;
    final JsonInfoMid otherSuperNotEqual = new JsonInfoMid();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoMid other = new JsonInfoMid();
    final JsonInfoMidEntry e = new JsonInfoMidEntry();
    final JsonInfoMidEntry e1 = new JsonInfoMidEntry();
    final List<JsonInfoMidEntry> nei1 = new LinkedList<>();
    nei1.add(e);
    final List<JsonInfoMidEntry> nei2 = new LinkedList<>();
    nei2.add(e1);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMid(null);
    other.setMid(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMid(nei1);
    other.setMid(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMid(null);
    other.setMid(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMid(nei1);
    other.setMid(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    final JsonInfoMidEntry entry = new JsonInfoMidEntry();
    nei2.add(entry);

    this.impl.setMid(nei1);
    other.setMid(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setMid(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810336)));

    final List<JsonInfoMidEntry> mid = new LinkedList<>();
    final JsonInfoMidEntry entry = new JsonInfoMidEntry();
    final JsonInfoTimedIpAddress main = new JsonInfoTimedIpAddress();
    entry.setMain(main);
    mid.add(entry);
    this.impl.setMid(mid);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742841118)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/mid.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoMid gws = Helpers.objectMapper.readValue(output, JsonInfoMid.class);
    assertThat(gws, notNullValue());
  }
}