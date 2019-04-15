package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

@SuppressWarnings("static-method")
public class TestJsonInfoConfig {
  private JsonInfoConfig impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfig();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getConfig(), notNullValue());

    /* set */
    final JsonInfoConfigEntry config = new JsonInfoConfigEntry();
    config.setConfigurationFile("configurationFile");
    this.impl.setConfig(config);

    /* get */
    assertThat(this.impl.getConfig(), equalTo(config));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    final Object otherNull = null;
    final JsonInfoConfig otherSuperNotEqual = new JsonInfoConfig();
    otherSuperNotEqual.setTimeSinceStartup(321);

    final JsonInfoConfig otherSame = new JsonInfoConfig();

    final JsonInfoConfig otherNotSame1 = new JsonInfoConfig();
    final JsonInfoConfigEntry configOtherNotSame1 = new JsonInfoConfigEntry();
    configOtherNotSame1.setConfigurationFile("configuration file otherNotSame1");
    otherNotSame1.setConfig(configOtherNotSame1);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSame);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setConfig(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame1.setConfig(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1928340850)));

    /* set */
    final JsonInfoConfigEntry config = new JsonInfoConfigEntry();
    config.setConfigurationFile("configurationFile");
    this.impl.setConfig(config);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1654009724)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/config.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoConfig config = Helpers.objectMapper.readValue(output, JsonInfoConfig.class);
    assertThat(config, notNullValue());
  }
}