package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoSgwFields {
  private JsonInfoSgwFields impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoSgwFields();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {

    /* initial */
    assertThat(this.impl.getEgress(), notNullValue());
    assertThat(this.impl.getIpv4(), notNullValue());
    assertThat(this.impl.getIpv6(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getEgress().isEmpty()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getIpv4().isEmpty()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getIpv6().isEmpty()), equalTo(Boolean.TRUE));

    /* set */
    final JsonInfoSgwEgressEntry ee = new JsonInfoSgwEgressEntry();
    final Set<JsonInfoSgwEgressEntry> egress = new TreeSet<>();
    egress.add(ee);
    final JsonInfoSgwEntry entry = new JsonInfoSgwEntry();
    entry.setCost(123);
    final Set<JsonInfoSgwEntry> ipv4 = new TreeSet<>();
    ipv4.add(entry);
    final Set<JsonInfoSgwEntry> ipv6 = new TreeSet<>();
    ipv6.add(entry);
    ipv6.add(entry);
    this.impl.setEgress(egress);
    this.impl.setIpv4(ipv4);
    this.impl.setIpv6(ipv6);

    /* get */
    assertThat(this.impl.getEgress(), notNullValue());
    assertThat(this.impl.getIpv4(), notNullValue());
    assertThat(this.impl.getIpv6(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getEgress().isEmpty()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getIpv4().isEmpty()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getIpv6().isEmpty()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getEgress(), equalTo(egress));
    assertThat(this.impl.getIpv4(), equalTo(ipv4));
    assertThat(this.impl.getIpv6(), equalTo(ipv6));

    /* clear */
    this.impl.setEgress(null);
    this.impl.setIpv4(null);
    this.impl.setIpv6(null);

    assertThat(this.impl.getEgress(), notNullValue());
    assertThat(this.impl.getIpv4(), notNullValue());
    assertThat(this.impl.getIpv6(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getEgress().isEmpty()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getIpv4().isEmpty()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getIpv6().isEmpty()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final JsonInfoSgwEgressEntry ee = new JsonInfoSgwEgressEntry();
    final Set<JsonInfoSgwEgressEntry> egress = new TreeSet<>();
    egress.add(ee);
    final JsonInfoSgwEntry entry = new JsonInfoSgwEntry();
    entry.setCost(123);
    final Set<JsonInfoSgwEntry> ipv4 = new TreeSet<>();
    ipv4.add(entry);
    final Set<JsonInfoSgwEntry> ipv6 = new TreeSet<>();
    ipv6.add(entry);
    ipv6.add(entry);

    boolean r;
    final Object otherNull = null;
    final Object otherOtherClass = new Object();

    final JsonInfoSgwFields otherNotSame0 = new JsonInfoSgwFields();
    otherNotSame0.setEgress(egress);

    final JsonInfoSgwFields otherNotSame1 = new JsonInfoSgwFields();
    otherNotSame1.setIpv4(ipv4);

    final JsonInfoSgwFields otherNotSame2 = new JsonInfoSgwFields();
    otherNotSame2.setIpv6(ipv6);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherOtherClass);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setEgress(null);
    r = this.impl.equals(otherNotSame0);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame0.setEgress(null);
    r = this.impl.equals(otherNotSame0);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setIpv4(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame1.setIpv4(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl = new JsonInfoSgwFields();

    r = this.impl.equals(otherNotSame2);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setIpv6(null);
    r = this.impl.equals(otherNotSame2);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame2.setIpv6(null);
    r = this.impl.equals(otherNotSame2);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(29791)));

    /* set */
    final JsonInfoSgwEgressEntry ee = new JsonInfoSgwEgressEntry();
    final Set<JsonInfoSgwEgressEntry> egress = new TreeSet<>();
    egress.add(ee);
    final JsonInfoSgwEntry entry = new JsonInfoSgwEntry();
    entry.setCost(123);
    final Set<JsonInfoSgwEntry> ipv4 = new TreeSet<>();
    ipv4.add(entry);
    final Set<JsonInfoSgwEntry> ipv6 = new TreeSet<>();
    ipv6.add(entry);
    ipv6.add(entry);
    this.impl.setEgress(egress);
    this.impl.setIpv4(ipv4);
    this.impl.setIpv6(ipv6);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(888994923)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}