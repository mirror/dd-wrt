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
public class TestJsonInfoVersion {
  private JsonInfoVersion impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoVersion();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {

    /* initial */
    assertThat(this.impl.getVersion(), notNullValue());

    /* set */
    final JsonInfoVersionEntry version = new JsonInfoVersionEntry();
    version.setVersion("version2");
    this.impl.setVersion(version);

    /* get */
    assertThat(this.impl.getVersion(), equalTo(version));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final JsonInfoVersionEntry entry = new JsonInfoVersionEntry();
    entry.setVersion("version");
    this.impl.setVersion(entry);

    boolean r;
    final Object otherNull = null;
    final JsonInfoVersion otherSuperNotEqual = new JsonInfoVersion();
    otherSuperNotEqual.setTimeSinceStartup(321);

    final JsonInfoVersion otherSame = new JsonInfoVersion();
    final JsonInfoVersionEntry versionOtherSame = new JsonInfoVersionEntry();
    versionOtherSame.setVersion("version");
    otherSame.setVersion(versionOtherSame);

    final JsonInfoVersion otherNotSame1 = new JsonInfoVersion();
    final JsonInfoVersionEntry versionOtherNotSame1 = new JsonInfoVersionEntry();
    versionOtherNotSame1.setVersion("version otherNotSame1");
    otherNotSame1.setVersion(versionOtherNotSame1);

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

    this.impl.setVersion(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame1.setVersion(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testComapreTo() {
    int r;

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    final JsonInfoVersion v = new JsonInfoVersion();
    v.setError("error");
    r = this.impl.compareTo(v);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1771439486)));

    /* set */
    final JsonInfoVersionEntry version = new JsonInfoVersionEntry();
    version.setVersion("version2");
    this.impl.setVersion(version);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1623836376)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/version.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoVersion version = Helpers.objectMapper.readValue(output, JsonInfoVersion.class);
    assertThat(version, notNullValue());
  }
}