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
public class TestJsonInfoGateways {
  private JsonInfoGateways impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoGateways();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getGateways(), notNullValue());

    /* set */
    final JsonInfoGatewaysFields gateways = new JsonInfoGatewaysFields();
    final Set<JsonInfoGatewaysEntry> ipv4 = new TreeSet<>();
    final JsonInfoGatewaysEntry entry = new JsonInfoGatewaysEntry();
    entry.setCost(123);
    ipv4.add(entry);
    gateways.setIpv4(ipv4);
    this.impl.setGateways(gateways);

    /* get */
    assertThat(this.impl.getGateways(), equalTo(gateways));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final JsonInfoGatewaysEntry entry = new JsonInfoGatewaysEntry();
    entry.setCost(123);
    final Set<JsonInfoGatewaysEntry> ipv4 = new TreeSet<>();
    ipv4.add(entry);
    final JsonInfoGatewaysFields gateways = new JsonInfoGatewaysFields();
    gateways.setIpv4(ipv4);

    boolean r;
    final Object otherNull = null;
    final JsonInfoGateways otherSuperNotEqual = new JsonInfoGateways();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoGateways otherSame = new JsonInfoGateways();
    final JsonInfoGateways otherNotSame1 = new JsonInfoGateways();
    otherNotSame1.setGateways(gateways);

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

    this.impl.setGateways(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame1.setGateways(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742811296)));

    /* set */
    final JsonInfoGatewaysFields gateways = new JsonInfoGatewaysFields();
    final Set<JsonInfoGatewaysEntry> ipv4 = new TreeSet<>();
    final JsonInfoGatewaysEntry entry = new JsonInfoGatewaysEntry();
    entry.setCost(123);
    ipv4.add(entry);
    gateways.setIpv4(ipv4);
    this.impl.setGateways(gateways);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1200368684)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/gateways.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoGateways gws = Helpers.objectMapper.readValue(output, JsonInfoGateways.class);
    assertThat(gws, notNullValue());
  }
}