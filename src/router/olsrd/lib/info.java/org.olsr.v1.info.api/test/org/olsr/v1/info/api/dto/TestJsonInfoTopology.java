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
public class TestJsonInfoTopology {
  private JsonInfoTopology impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoTopology();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getTopology(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getTopology().isEmpty()), equalTo(Boolean.TRUE));

    /* set */
    final Set<JsonInfoTopologyEntry> routes = new TreeSet<>();
    final JsonInfoTopologyEntry entry = new JsonInfoTopologyEntry();
    entry.setPathCost(11);
    routes.add(entry);
    this.impl.setTopology(routes);

    /* get */
    assertThat(this.impl.getTopology(), equalTo(routes));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    final Object otherNull = null;
    final JsonInfoTopology otherSuperNotEqual = new JsonInfoTopology();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoTopology other = new JsonInfoTopology();
    final JsonInfoTopologyEntry e = new JsonInfoTopologyEntry();
    final JsonInfoTopologyEntry e2 = new JsonInfoTopologyEntry();
    final Set<JsonInfoTopologyEntry> nei1 = new TreeSet<>();
    nei1.add(e);
    final Set<JsonInfoTopologyEntry> nei2 = new TreeSet<>();
    nei2.add(e2);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTopology(null);
    other.setTopology(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setTopology(nei1);
    other.setTopology(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTopology(null);
    other.setTopology(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTopology(nei1);
    other.setTopology(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    final JsonInfoTopologyEntry entry = new JsonInfoTopologyEntry();
    nei2.add(entry);

    this.impl.setTopology(nei1);
    final JsonInfoTopologyEntry e3 = new JsonInfoTopologyEntry();
    e3.setAnsn(11);
    nei2.add(e3);
    other.setTopology(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setTopology(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810335)));

    final Set<JsonInfoTopologyEntry> routes = new TreeSet<>();
    final JsonInfoTopologyEntry entry = new JsonInfoTopologyEntry();
    entry.setPathCost(11);
    routes.add(entry);
    this.impl.setTopology(routes);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(924566315)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/topology.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoTopology gws = Helpers.objectMapper.readValue(output, JsonInfoTopology.class);
    assertThat(gws, notNullValue());
  }
}