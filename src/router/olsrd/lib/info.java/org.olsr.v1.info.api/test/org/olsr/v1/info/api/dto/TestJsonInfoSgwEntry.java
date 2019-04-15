package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoSgwEntry {
  private JsonInfoSgwEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoSgwEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getDestination(), equalTo(""));
    assertThat(this.impl.getTunnel(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getTableNr()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getRuleNr()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getBypassRuleNr()), equalTo(Integer.valueOf(0)));

    /* set */
    final InetAddress destination = InetAddress.getByName("127.0.0.3");

    this.impl.setDestination(destination);
    this.impl.setTunnel("tunnel");
    this.impl.setTableNr(3);
    this.impl.setRuleNr(4);
    this.impl.setBypassRuleNr(5);

    /* get */
    assertThat(this.impl.getDestination(), equalTo(destination.getHostAddress()));
    assertThat(this.impl.getTunnel(), equalTo("tunnel"));
    assertThat(Integer.valueOf(this.impl.getTableNr()), equalTo(Integer.valueOf(3)));
    assertThat(Integer.valueOf(this.impl.getRuleNr()), equalTo(Integer.valueOf(4)));
    assertThat(Integer.valueOf(this.impl.getBypassRuleNr()), equalTo(Integer.valueOf(5)));
  }

  @Test(timeout = 8000)
  public void testCompareTo() throws UnknownHostException {
    int r;
    final JsonInfoSgwEntry other = new JsonInfoSgwEntry();
    final InetAddress destination1 = InetAddress.getByName("127.0.0.1");
    final InetAddress destination2 = InetAddress.getByName("127.0.0.2");
    final String tunnel1 = "1";
    final String tunnel2 = "2";

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* super */

    final boolean selectedOrg = other.getSelected();
    other.setSelected(!selectedOrg);

    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    other.setSelected(selectedOrg);

    /* destination */

    final String destinationOrg = this.impl.getDestination();

    this.impl.setDestination(null);
    other.setDestination(destination2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestination(destination2);
    other.setDestination(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDestination(destination1);
    other.setDestination(destination2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDestination(destination1);
    other.setDestination(destination1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDestination(InetAddress.getByName(destinationOrg));
    other.setDestination(InetAddress.getByName(destinationOrg));

    /* tunnel */

    final String tunnelOrg = this.impl.getTunnel();

    this.impl.setTunnel(null);
    other.setTunnel(tunnel2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTunnel(tunnel2);
    other.setTunnel(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTunnel(tunnel1);
    other.setTunnel(tunnel2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTunnel(tunnel1);
    other.setTunnel(tunnel1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTunnel(tunnelOrg);
    other.setTunnel(tunnelOrg);

    /* tableNr */

    final int tableNrOrg = this.impl.getTableNr();

    this.impl.setTableNr(1);
    other.setTableNr(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTableNr(2);
    other.setTableNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTableNr(1);
    other.setTableNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTableNr(tableNrOrg);
    other.setTableNr(tableNrOrg);

    /* ruleNr */

    final int ruleNrOrg = this.impl.getRuleNr();

    this.impl.setRuleNr(1);
    other.setRuleNr(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setRuleNr(2);
    other.setRuleNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setRuleNr(1);
    other.setRuleNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRuleNr(ruleNrOrg);
    other.setRuleNr(ruleNrOrg);

    /* bypassRuleNr */

    final int bypassRuleNrOrg = this.impl.getBypassRuleNr();

    this.impl.setBypassRuleNr(1);
    other.setBypassRuleNr(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBypassRuleNr(2);
    other.setBypassRuleNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBypassRuleNr(1);
    other.setBypassRuleNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBypassRuleNr(bypassRuleNrOrg);
    other.setBypassRuleNr(bypassRuleNrOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoSgwEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoSgwEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setBypassRuleNr(12);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(52139404)));

    final InetAddress destination = InetAddress.getByName("127.0.0.3");

    this.impl.setDestination(destination);
    this.impl.setTunnel("tunnel");
    this.impl.setTableNr(3);
    this.impl.setRuleNr(4);
    this.impl.setBypassRuleNr(5);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-431409689)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}