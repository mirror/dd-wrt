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
public class TestJsonInfoTwoHop {
  private JsonInfoTwoHop impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoTwoHop();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getTwoHop(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getTwoHop().isEmpty()), equalTo(Boolean.TRUE));

    /* set */
    final List<JsonInfoTwoHopEntry> neighbors = new LinkedList<>();
    final JsonInfoTwoHopEntry entry = new JsonInfoTwoHopEntry();
    entry.setLinkcount(1);
    neighbors.add(entry);
    this.impl.setTwoHop(neighbors);

    /* get */
    assertThat(this.impl.getTwoHop(), equalTo(neighbors));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    final Object otherNull = null;
    final JsonInfoTwoHop otherSuperNotEqual = new JsonInfoTwoHop();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoTwoHop other = new JsonInfoTwoHop();
    final JsonInfoTwoHopEntry e = new JsonInfoTwoHopEntry();
    final JsonInfoTwoHopEntry e1 = new JsonInfoTwoHopEntry();
    final List<JsonInfoTwoHopEntry> nei1 = new LinkedList<>();
    nei1.add(e);
    final List<JsonInfoTwoHopEntry> nei2 = new LinkedList<>();
    nei2.add(e1);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTwoHop(null);
    other.setTwoHop(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setTwoHop(nei1);
    other.setTwoHop(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTwoHop(null);
    other.setTwoHop(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTwoHop(nei1);
    other.setTwoHop(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    final JsonInfoTwoHopEntry entry = new JsonInfoTwoHopEntry();
    nei2.add(entry);

    this.impl.setTwoHop(nei1);
    other.setTwoHop(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setTwoHop(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810336)));

    final List<JsonInfoTwoHopEntry> neighbors = new LinkedList<>();
    final JsonInfoTwoHopEntry entry = new JsonInfoTwoHopEntry();
    entry.setLinkcount(1);
    neighbors.add(entry);
    this.impl.setTwoHop(neighbors);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1420455721)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/2hop.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoTwoHop gws = Helpers.objectMapper.readValue(output, JsonInfoTwoHop.class);
    assertThat(gws, notNullValue());
  }
}