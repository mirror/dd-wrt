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
public class TestJsonInfoRoutes {
  private JsonInfoRoutes impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoRoutes();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getRoutes(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getRoutes().isEmpty()), equalTo(Boolean.TRUE));

    /* set */
    final Set<JsonInfoRoutesEntry> routes = new TreeSet<>();
    final JsonInfoRoutesEntry entry = new JsonInfoRoutesEntry();
    entry.setDestinationPrefixLength(11);
    routes.add(entry);
    this.impl.setRoutes(routes);

    /* get */
    assertThat(this.impl.getRoutes(), equalTo(routes));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    final Object otherNull = null;
    final JsonInfoRoutes otherSuperNotEqual = new JsonInfoRoutes();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoRoutes other = new JsonInfoRoutes();
    final JsonInfoRoutesEntry e = new JsonInfoRoutesEntry();
    final JsonInfoRoutesEntry e1 = new JsonInfoRoutesEntry();
    final Set<JsonInfoRoutesEntry> nei1 = new TreeSet<>();
    nei1.add(e);
    final Set<JsonInfoRoutesEntry> nei2 = new TreeSet<>();
    nei2.add(e1);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRoutes(null);
    other.setRoutes(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setRoutes(nei1);
    other.setRoutes(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRoutes(null);
    other.setRoutes(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRoutes(nei1);
    other.setRoutes(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    final JsonInfoRoutesEntry entry = new JsonInfoRoutesEntry();
    nei2.add(entry);

    this.impl.setRoutes(nei1);
    final JsonInfoRoutesEntry e2 = new JsonInfoRoutesEntry();
    e2.setEtx(123123);
    nei2.add(e2);
    other.setRoutes(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setRoutes(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810335)));

    final Set<JsonInfoRoutesEntry> routes = new TreeSet<>();
    final JsonInfoRoutesEntry entry = new JsonInfoRoutesEntry();
    entry.setDestinationPrefixLength(11);
    routes.add(entry);
    this.impl.setRoutes(routes);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1534613357)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/routes.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoRoutes gws = Helpers.objectMapper.readValue(output, JsonInfoRoutes.class);
    assertThat(gws, notNullValue());
  }
}