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
public class TestJsonInfoNeighbours {
  private JsonInfoNeighbours impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoNeighbours();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getNeighbors(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getNeighbors().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getLinks(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getLinks().size()), equalTo(Integer.valueOf(0)));

    /* set */
    final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();
    final JsonInfoNeighborsEntry neighborsEntry = new JsonInfoNeighborsEntry();
    neighbors.add(neighborsEntry);
    final Set<JsonInfoLinksEntry> links = new TreeSet<>();
    final JsonInfoLinksEntry linksEntry = new JsonInfoLinksEntry();
    links.add(linksEntry);

    this.impl.setNeighbors(neighbors);
    this.impl.setLinks(links);

    /* get */
    assertThat(this.impl.getNeighbors(), equalTo(neighbors));
    assertThat(Integer.valueOf(this.impl.getNeighbors().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getLinks(), equalTo(links));
    assertThat(Integer.valueOf(this.impl.getLinks().size()), equalTo(Integer.valueOf(1)));

    /* set */
    this.impl.setNeighbors(null);
    this.impl.setLinks(null);

    /* get */
    assertThat(this.impl.getNeighbors(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getNeighbors().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getLinks(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getLinks().size()), equalTo(Integer.valueOf(0)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();
    final JsonInfoNeighborsEntry neighborsEntry = new JsonInfoNeighborsEntry();
    neighbors.add(neighborsEntry);
    final Set<JsonInfoLinksEntry> links = new TreeSet<>();
    final JsonInfoLinksEntry linksEntry = new JsonInfoLinksEntry();
    links.add(linksEntry);

    boolean r;
    final Object otherNull = null;
    final JsonInfoNeighbours otherEqual = new JsonInfoNeighbours();
    final JsonInfoNeighbours otherSuperNotEqual = new JsonInfoNeighbours();
    otherSuperNotEqual.setTimeSinceStartup(321);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* neighbors */

    this.impl.setNeighbors(neighbors);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setNeighbors(null);

    /* links */

    this.impl.setLinks(links);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setLinks(null);

    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setNeighbors(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1807454463)));

    /* set */
    final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();
    final JsonInfoNeighborsEntry neighborsEntry = new JsonInfoNeighborsEntry();
    neighbors.add(neighborsEntry);
    final Set<JsonInfoLinksEntry> links = new TreeSet<>();
    final JsonInfoLinksEntry linksEntry = new JsonInfoLinksEntry();
    links.add(linksEntry);

    this.impl.setNeighbors(neighbors);
    this.impl.setLinks(links);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-512679457)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/neighbours.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoNeighbours gws = Helpers.objectMapper.readValue(output, JsonInfoNeighbours.class);
    assertThat(gws, notNullValue());
  }
}