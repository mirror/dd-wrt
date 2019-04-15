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
public class TestJsonInfoBase {
  private JsonInfoBase impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoBase();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Long.valueOf(this.impl.getPid()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getSystemTime()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getTimeSinceStartup()), equalTo(Long.valueOf(0)));
    assertThat(this.impl.getConfigurationChecksum(), equalTo(""));
    assertThat(this.impl.getUuid(), equalTo(""));
    assertThat(this.impl.getError(), equalTo(""));

    /* set */
    this.impl.setPid(333);
    this.impl.setSystemTime(123);
    this.impl.setTimeSinceStartup(321);
    this.impl.setConfigurationChecksum("2e2c14b6e3562ddfd73111847d7a9d8277993ff7");
    this.impl.setUuid("d13d7ee3-fcca-4f81-9bf5-bb5848b5d96d");
    this.impl.setError("something");

    /* get */
    assertThat(Long.valueOf(this.impl.getPid()), equalTo(Long.valueOf(333)));
    assertThat(Long.valueOf(this.impl.getSystemTime()), equalTo(Long.valueOf(123)));
    assertThat(Long.valueOf(this.impl.getTimeSinceStartup()), equalTo(Long.valueOf(321)));
    assertThat(this.impl.getConfigurationChecksum(), equalTo("2e2c14b6e3562ddfd73111847d7a9d8277993ff7"));
    assertThat(this.impl.getUuid(), equalTo("d13d7ee3-fcca-4f81-9bf5-bb5848b5d96d"));
    assertThat(this.impl.getError(), equalTo("something"));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoBase other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoBase();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = new JsonInfoBase();
    other.setConfigurationChecksum("configChecksum");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoBase();
    other.setUuid("uuid");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testComapreTo() {
    int r;
    final JsonInfoBase other = new JsonInfoBase();
    final String checksum1 = "checksum 1";
    final String checksum2 = "checksum 2";
    final String gateway1 = "127.0.0.31";
    final String gateway2 = "127.0.0.32";

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* pid */

    long longOrg = this.impl.getPid();

    this.impl.setPid(1);
    other.setPid(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPid(2);
    other.setPid(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPid(1);
    other.setPid(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPid(longOrg);
    other.setPid(longOrg);

    /* systemTime */

    longOrg = this.impl.getSystemTime();

    this.impl.setSystemTime(1);
    other.setSystemTime(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSystemTime(2);
    other.setSystemTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSystemTime(1);
    other.setSystemTime(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSystemTime(longOrg);
    other.setSystemTime(longOrg);

    /* timeSinceStartup */

    longOrg = this.impl.getTimeSinceStartup();

    this.impl.setTimeSinceStartup(1);
    other.setTimeSinceStartup(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTimeSinceStartup(2);
    other.setTimeSinceStartup(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTimeSinceStartup(1);
    other.setTimeSinceStartup(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTimeSinceStartup(longOrg);
    other.setTimeSinceStartup(longOrg);

    /* configChecksum */

    String stringOrg = this.impl.getConfigurationChecksum();

    this.impl.setConfigurationChecksum(null);
    other.setConfigurationChecksum(checksum2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setConfigurationChecksum(checksum2);
    other.setConfigurationChecksum(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setConfigurationChecksum(checksum1);
    other.setConfigurationChecksum(checksum2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setConfigurationChecksum(checksum1);
    other.setConfigurationChecksum(checksum1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setConfigurationChecksum(stringOrg);
    other.setConfigurationChecksum(stringOrg);

    /* uuid */

    stringOrg = this.impl.getUuid();

    this.impl.setUuid(null);
    other.setUuid(gateway2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setUuid(gateway2);
    other.setUuid(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setUuid(gateway1);
    other.setUuid(gateway2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setUuid(gateway1);
    other.setUuid(gateway1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUuid(stringOrg);
    other.setUuid(stringOrg);

    /* error */

    stringOrg = this.impl.getError();

    this.impl.setError(null);
    other.setError(gateway2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setError(gateway2);
    other.setError(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setError(gateway1);
    other.setError(gateway2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setError(gateway1);
    other.setError(gateway1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setError(stringOrg);
    other.setError(stringOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setPid(333);
    this.impl.setSystemTime(123);
    this.impl.setTimeSinceStartup(321);
    this.impl.setConfigurationChecksum("2e2c14b6e3562ddfd73111847d7a9d8277993ff7");
    this.impl.setUuid("d13d7ee3-fcca-4f81-9bf5-bb5848b5d96d");
    this.impl.setError("something");

    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1558552252)));

    this.impl.setConfigurationChecksum(null);
    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-139543783)));

    this.impl.setUuid(null);
    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1389346984)));

    this.impl.setError(null);
    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1954232366)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    this.impl.setPid(333);
    this.impl.setSystemTime(123);
    this.impl.setTimeSinceStartup(321);
    this.impl.setConfigurationChecksum("2e2c14b6e3562ddfd73111847d7a9d8277993ff7");
    this.impl.setUuid("d13d7ee3-fcca-4f81-9bf5-bb5848b5d96d");
    this.impl.setError("something");

    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/base.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoBase base = Helpers.objectMapper.readValue(output, JsonInfoBase.class);
    assertThat(base, notNullValue());
  }
}