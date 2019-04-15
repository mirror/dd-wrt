package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.util.HashMap;
import java.util.Map;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoPluginsEntry {
  private JsonInfoPluginsEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoPluginsEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getPlugin(), equalTo(""));
    assertThat(this.impl.getParameters(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getParameters().size()), equalTo(Integer.valueOf(0)));

    /* set */
    this.impl.setPlugin("plugin");
    final Map<String, String> params = new HashMap<>();
    params.put("key", "value");
    this.impl.setParameters(params);

    /* get */
    assertThat(this.impl.getPlugin(), equalTo("plugin"));
    assertThat(this.impl.getParameters(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getParameters().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getParameters().get("key"), equalTo("value"));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoPluginsEntry other;
    final String plugin = "plugin";
    final Map<String, String> parameters0 = new HashMap<>();
    final Map<String, String> parameters1 = new HashMap<>();
    parameters1.put("key", "value");

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoPluginsEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* plugin */

    this.impl.setPlugin(null);
    other.setPlugin(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPlugin(null);
    other.setPlugin(plugin);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPlugin(plugin);
    other.setPlugin(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPlugin(plugin);
    other.setPlugin(plugin);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* parameters */

    this.impl.setParameters(null);
    other.setParameters(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setParameters(null);
    other.setParameters(parameters0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setParameters(parameters0);
    other.setParameters(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setParameters(parameters0);
    other.setParameters(parameters0);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setParameters(parameters0);
    other.setParameters(parameters1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(961)));

    /* set */
    this.impl.setPlugin("plugin");
    final Map<String, String> params = new HashMap<>();
    params.put("key", "value");
    this.impl.setParameters(params);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-363623908)));

    this.impl.setParameters(null);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-475628818)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}