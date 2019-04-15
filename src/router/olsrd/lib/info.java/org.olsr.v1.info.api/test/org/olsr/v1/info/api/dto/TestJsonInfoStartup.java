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
public class TestJsonInfoStartup {
  private JsonInfoStartup impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoStartup();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getVersion(), notNullValue());
    assertThat(this.impl.getPlugins(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getPlugins().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getConfig(), notNullValue());

    /* set */
    final JsonInfoVersionEntry version = new JsonInfoVersionEntry();
    version.setVersion("now");

    final List<JsonInfoPluginsEntry> plugins = new LinkedList<>();
    final JsonInfoPluginsEntry pluginsEntry = new JsonInfoPluginsEntry();
    plugins.add(pluginsEntry);

    final JsonInfoConfigEntry config = new JsonInfoConfigEntry();
    config.setExitValue(11);

    this.impl.setVersion(version);
    this.impl.setPlugins(plugins);
    this.impl.setConfig(config);

    /* get */
    assertThat(this.impl.getVersion(), equalTo(version));
    assertThat(this.impl.getPlugins(), equalTo(plugins));
    assertThat(Integer.valueOf(this.impl.getPlugins().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getConfig(), equalTo(config));

    /* set */
    this.impl.setVersion(null);
    this.impl.setPlugins(null);
    this.impl.setConfig(null);

    /* get */
    assertThat(this.impl.getVersion(), notNullValue());
    assertThat(this.impl.getPlugins(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getPlugins().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getConfig(), notNullValue());
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final JsonInfoVersionEntry version = new JsonInfoVersionEntry();
    version.setVersion("now");

    final List<JsonInfoPluginsEntry> plugins = new LinkedList<>();
    final JsonInfoPluginsEntry pluginsEntry = new JsonInfoPluginsEntry();
    plugins.add(pluginsEntry);

    final JsonInfoConfigEntry config = new JsonInfoConfigEntry();
    config.setExitValue(11);

    boolean r;
    final Object otherNull = null;
    final JsonInfoStartup otherEqual = new JsonInfoStartup();
    final JsonInfoStartup otherSuperNotEqual = new JsonInfoStartup();
    otherSuperNotEqual.setTimeSinceStartup(321);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* version */

    this.impl.setVersion(null);
    otherEqual.setVersion(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setVersion(null);
    otherEqual.setVersion(version);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setVersion(version);
    otherEqual.setVersion(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setVersion(version);
    otherEqual.setVersion(version);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setVersion(null);
    otherEqual.setVersion(null);

    /* plugins */

    this.impl.setPlugins(plugins);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setPlugins(null);

    /* config */

    this.impl.setConfig(null);
    otherEqual.setConfig(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setConfig(null);
    otherEqual.setConfig(config);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setConfig(config);
    otherEqual.setConfig(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setConfig(config);
    otherEqual.setConfig(config);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setConfig(null);
    otherEqual.setConfig(null);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1731827376)));

    /* set */
    final JsonInfoVersionEntry version = new JsonInfoVersionEntry();
    version.setVersion("now");

    final List<JsonInfoPluginsEntry> plugins = new LinkedList<>();
    final JsonInfoPluginsEntry pluginsEntry = new JsonInfoPluginsEntry();
    plugins.add(pluginsEntry);

    final JsonInfoConfigEntry config = new JsonInfoConfigEntry();
    config.setExitValue(11);

    this.impl.setVersion(version);
    this.impl.setPlugins(plugins);
    this.impl.setConfig(config);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(814643292)));

    /* set */
    this.impl.setVersion(null);
    this.impl.setPlugins(null);
    this.impl.setConfig(null);

    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(814643292)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/startup.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoStartup gws = Helpers.objectMapper.readValue(output, JsonInfoStartup.class);
    assertThat(gws, notNullValue());
  }
}