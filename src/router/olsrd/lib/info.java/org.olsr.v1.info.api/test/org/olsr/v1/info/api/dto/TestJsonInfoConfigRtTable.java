package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigRtTable {
  private JsonInfoConfigRtTable impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigRtTable();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Integer.valueOf(this.impl.getMain()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getDefault()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getTunnel()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getPriority()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getTunnelPriority()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getDefaultOlsrPriority()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getDefaultPriority()), equalTo(Integer.valueOf(0)));

    /* set */
    this.impl.setMain(1);
    this.impl.setDefault(2);
    this.impl.setTunnel(3);
    this.impl.setPriority(4);
    this.impl.setTunnelPriority(5);
    this.impl.setDefaultOlsrPriority(6);
    this.impl.setDefaultPriority(7);

    /* get */
    assertThat(Integer.valueOf(this.impl.getMain()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getDefault()), equalTo(Integer.valueOf(2)));
    assertThat(Integer.valueOf(this.impl.getTunnel()), equalTo(Integer.valueOf(3)));
    assertThat(Integer.valueOf(this.impl.getPriority()), equalTo(Integer.valueOf(4)));
    assertThat(Integer.valueOf(this.impl.getTunnelPriority()), equalTo(Integer.valueOf(5)));
    assertThat(Integer.valueOf(this.impl.getDefaultOlsrPriority()), equalTo(Integer.valueOf(6)));
    assertThat(Integer.valueOf(this.impl.getDefaultPriority()), equalTo(Integer.valueOf(7)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigRtTable other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigRtTable();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = new JsonInfoConfigRtTable();
    other.setMain(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoConfigRtTable other = new JsonInfoConfigRtTable();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* main */

    int intOrg = this.impl.getMain();

    this.impl.setMain(1);
    other.setMain(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMain(2);
    other.setMain(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMain(1);
    other.setMain(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMain(intOrg);
    other.setMain(intOrg);

    /* default_ */

    intOrg = this.impl.getDefault();

    this.impl.setDefault(1);
    other.setDefault(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDefault(2);
    other.setDefault(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDefault(1);
    other.setDefault(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDefault(intOrg);
    other.setDefault(intOrg);

    /* tunnel */

    intOrg = this.impl.getTunnel();

    this.impl.setTunnel(1);
    other.setTunnel(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTunnel(2);
    other.setTunnel(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTunnel(1);
    other.setTunnel(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTunnel(intOrg);
    other.setTunnel(intOrg);

    /* priority */

    intOrg = this.impl.getPriority();

    this.impl.setPriority(1);
    other.setPriority(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setPriority(2);
    other.setPriority(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setPriority(1);
    other.setPriority(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setPriority(intOrg);
    other.setPriority(intOrg);

    /* tunnelPriority */

    intOrg = this.impl.getTunnelPriority();

    this.impl.setTunnelPriority(1);
    other.setTunnelPriority(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTunnelPriority(2);
    other.setTunnelPriority(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTunnelPriority(1);
    other.setTunnelPriority(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTunnelPriority(intOrg);
    other.setTunnelPriority(intOrg);

    /* defaultOlsrPriority */

    intOrg = this.impl.getDefaultOlsrPriority();

    this.impl.setDefaultOlsrPriority(1);
    other.setDefaultOlsrPriority(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDefaultOlsrPriority(2);
    other.setDefaultOlsrPriority(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDefaultOlsrPriority(1);
    other.setDefaultOlsrPriority(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDefaultOlsrPriority(intOrg);
    other.setDefaultOlsrPriority(intOrg);

    /* defaultPriority */

    intOrg = this.impl.getDefaultPriority();

    this.impl.setDefaultPriority(1);
    other.setDefaultPriority(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDefaultPriority(2);
    other.setDefaultPriority(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDefaultPriority(1);
    other.setDefaultPriority(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDefaultPriority(intOrg);
    other.setDefaultPriority(intOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810335)));

    /* set */
    this.impl.setMain(1);
    this.impl.setDefault(2);
    this.impl.setTunnel(3);
    this.impl.setPriority(4);
    this.impl.setTunnelPriority(5);
    this.impl.setDefaultOlsrPriority(6);
    this.impl.setDefaultOlsrPriority(7);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1604500229)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}