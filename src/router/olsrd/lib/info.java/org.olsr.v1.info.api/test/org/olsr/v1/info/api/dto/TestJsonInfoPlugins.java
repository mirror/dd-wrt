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
public class TestJsonInfoPlugins {
  private JsonInfoPlugins impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoPlugins();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getPlugins(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getPlugins().isEmpty()), equalTo(Boolean.TRUE));

    /* set */
    final List<JsonInfoPluginsEntry> plugins = new LinkedList<>();
    final JsonInfoPluginsEntry entry = new JsonInfoPluginsEntry();
    entry.setPlugin("plugin");
    plugins.add(entry);
    this.impl.setPlugins(plugins);

    /* get */
    assertThat(this.impl.getPlugins(), equalTo(plugins));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    final Object otherNull = null;
    final JsonInfoPlugins otherSuperNotEqual = new JsonInfoPlugins();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoPlugins other = new JsonInfoPlugins();

    final List<JsonInfoPluginsEntry> plugins0 = new LinkedList<>();
    final List<JsonInfoPluginsEntry> plugins1 = new LinkedList<>();
    final JsonInfoPluginsEntry entry = new JsonInfoPluginsEntry();
    entry.setPlugin("plugin1");
    plugins1.add(entry);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPlugins(null);
    other.setPlugins(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPlugins(plugins1);
    other.setPlugins(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPlugins(null);
    other.setPlugins(plugins1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPlugins(plugins0);
    other.setPlugins(plugins0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPlugins(plugins0);
    other.setPlugins(plugins1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setPlugins(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810336)));

    /* set */
    final List<JsonInfoPluginsEntry> plugins = new LinkedList<>();
    final JsonInfoPluginsEntry entry = new JsonInfoPluginsEntry();
    entry.setPlugin("plugin");
    plugins.add(entry);
    this.impl.setPlugins(plugins);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1267181548)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/plugins.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoPlugins gws = Helpers.objectMapper.readValue(output, JsonInfoPlugins.class);
    assertThat(gws, notNullValue());
  }
}