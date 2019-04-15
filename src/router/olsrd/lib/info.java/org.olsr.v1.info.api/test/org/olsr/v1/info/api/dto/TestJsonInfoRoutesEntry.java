package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoRoutesEntry {
  private JsonInfoRoutesEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoRoutesEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getDestination(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getDestinationPrefixLength()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getGateway(), equalTo(""));
    assertThat(Long.valueOf(this.impl.getMetric()), equalTo(Long.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getEtx()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));
    assertThat(Double.valueOf(this.impl.getRtpMetricCost()), equalTo(Double.valueOf(Double.POSITIVE_INFINITY)));
    assertThat(this.impl.getNetworkInterface(), equalTo(""));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");

    this.impl.setDestination(ipAddress);
    this.impl.setDestinationPrefixLength(11);
    this.impl.setGateway(ipAddress);
    this.impl.setMetric(21);
    this.impl.setEtx(31.12);
    this.impl.setRtpMetricCost(5.5);
    this.impl.setNetworkInterface("lo");

    /* get */
    assertThat(this.impl.getDestination(), equalTo(ipAddress.getHostAddress()));
    assertThat(Integer.valueOf(this.impl.getDestinationPrefixLength()), equalTo(Integer.valueOf(11)));
    assertThat(this.impl.getGateway(), equalTo(ipAddress.getHostAddress()));
    assertThat(Long.valueOf(this.impl.getMetric()), equalTo(Long.valueOf(21)));
    assertThat(Double.valueOf(this.impl.getEtx()), equalTo(Double.valueOf(31.12)));
    assertThat(Double.valueOf(this.impl.getRtpMetricCost()), equalTo(Double.valueOf(5.5)));
    assertThat(this.impl.getNetworkInterface(), equalTo("lo"));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoRoutesEntry other = new JsonInfoRoutesEntry();
    final InetAddress originator1 = InetAddress.getByName("127.0.0.1");
    final InetAddress originator2 = InetAddress.getByName("127.0.0.2");

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* destination */

    String ipOrg = this.impl.getDestination();

    this.impl.setDestination(null);
    other.setDestination(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestination(originator2);
    other.setDestination(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDestination(originator1);
    other.setDestination(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestination(originator1);
    other.setDestination(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDestination(InetAddress.getByName(ipOrg));
    other.setDestination(InetAddress.getByName(ipOrg));

    /* genMask */

    final int genMaskOrg = this.impl.getDestinationPrefixLength();

    this.impl.setDestinationPrefixLength(1);
    other.setDestinationPrefixLength(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestinationPrefixLength(2);
    other.setDestinationPrefixLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDestinationPrefixLength(1);
    other.setDestinationPrefixLength(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDestinationPrefixLength(genMaskOrg);
    other.setDestinationPrefixLength(genMaskOrg);

    /* gateway */

    ipOrg = this.impl.getGateway();

    this.impl.setGateway(null);
    other.setGateway(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGateway(originator2);
    other.setGateway(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGateway(originator1);
    other.setGateway(originator2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGateway(originator1);
    other.setGateway(originator1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGateway(InetAddress.getByName(ipOrg));
    other.setGateway(InetAddress.getByName(ipOrg));

    /* metric */

    final long metricOrg = this.impl.getMetric();

    this.impl.setMetric(1);
    other.setMetric(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setMetric(2);
    other.setMetric(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setMetric(1);
    other.setMetric(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setMetric(metricOrg);
    other.setMetric(metricOrg);

    /* etx */

    final double etxOrg = this.impl.getEtx();

    this.impl.setEtx(1);
    other.setEtx(2);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(-1)));

    this.impl.setEtx(2);
    other.setEtx(1);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(1)));

    this.impl.setEtx(1);
    other.setEtx(1);
    r = this.impl.compareTo(other);
    assertThat(Double.valueOf(r), equalTo(Double.valueOf(0)));

    this.impl.setEtx(etxOrg);
    other.setEtx(etxOrg);

    /* rtpMetricCost */

    final double doubleOrg = this.impl.getRtpMetricCost();

    this.impl.setRtpMetricCost(1);
    other.setRtpMetricCost(2);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    this.impl.setRtpMetricCost(2);
    other.setRtpMetricCost(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1)));

    this.impl.setRtpMetricCost(1);
    other.setRtpMetricCost(1);
    r = this.impl.compareTo(other);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(0)));

    this.impl.setRtpMetricCost(doubleOrg);
    other.setRtpMetricCost(doubleOrg);

    /* networkInterface */

    final String networkInterfaceOrg = this.impl.getNetworkInterface();

    this.impl.setNetworkInterface(null);
    other.setNetworkInterface("originator2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNetworkInterface("originator2");
    other.setNetworkInterface(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNetworkInterface("originator1");
    other.setNetworkInterface("originator2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNetworkInterface("originator1");
    other.setNetworkInterface("originator1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNetworkInterface(networkInterfaceOrg);
    other.setNetworkInterface(networkInterfaceOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoRoutesEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoRoutesEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setDestinationPrefixLength(11);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(702622943)));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");

    this.impl.setDestination(ipAddress);
    this.impl.setDestinationPrefixLength(11);
    this.impl.setGateway(ipAddress);
    this.impl.setMetric(21);
    this.impl.setEtx(31.12);
    this.impl.setRtpMetricCost(5);
    this.impl.setNetworkInterface("lo");

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1104016803)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}