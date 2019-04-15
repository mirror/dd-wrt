package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;
import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

@SuppressWarnings("static-method")
public class TestJsonInfoNeighbors {
  private JsonInfoNeighbors impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoNeighbors();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getNeighbors(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getNeighbors().isEmpty()), equalTo(Boolean.TRUE));

    /* set */
    final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();
    final JsonInfoNeighborsEntry entry = new JsonInfoNeighborsEntry();
    entry.setLinkcount(1);
    neighbors.add(entry);
    this.impl.setNeighbors(neighbors);

    /* get */
    assertThat(this.impl.getNeighbors(), equalTo(neighbors));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    final Object otherNull = null;
    final JsonInfoNeighbors otherSuperNotEqual = new JsonInfoNeighbors();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoNeighbors other = new JsonInfoNeighbors();
    final JsonInfoNeighborsEntry e = new JsonInfoNeighborsEntry();
    final JsonInfoNeighborsEntry e1 = new JsonInfoNeighborsEntry();
    final Set<JsonInfoNeighborsEntry> nei1 = new TreeSet<>();
    nei1.add(e);
    final Set<JsonInfoNeighborsEntry> nei2 = new TreeSet<>();
    nei2.add(e1);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNeighbors(null);
    other.setNeighbors(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setNeighbors(nei1);
    other.setNeighbors(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNeighbors(null);
    other.setNeighbors(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setNeighbors(nei1);
    other.setNeighbors(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    JsonInfoNeighborsEntry entry = new JsonInfoNeighborsEntry();
    nei2.add(entry);

    this.impl.setNeighbors(nei1);
    other.setNeighbors(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    entry = new JsonInfoNeighborsEntry();
    entry.setIsMultiPointRelay(true);
    nei2.add(entry);

    this.impl.setNeighbors(nei1);
    other.setNeighbors(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setNeighbors(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810335)));

    final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();
    final JsonInfoNeighborsEntry entry = new JsonInfoNeighborsEntry();
    entry.setLinkcount(1);
    neighbors.add(entry);
    this.impl.setNeighbors(neighbors);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(624033141)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/neighbors.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoNeighbors gws = Helpers.objectMapper.readValue(output, JsonInfoNeighbors.class);
    assertThat(gws, notNullValue());
  }
}