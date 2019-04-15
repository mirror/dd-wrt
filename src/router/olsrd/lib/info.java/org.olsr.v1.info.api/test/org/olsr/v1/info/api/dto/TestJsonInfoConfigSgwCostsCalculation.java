package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigSgwCostsCalculation {
  private JsonInfoConfigSgwCostsCalculation impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigSgwCostsCalculation();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Integer.valueOf(this.impl.getExitLinkUp()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getExitLinkDown()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getEtx()), equalTo(Integer.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getDividerEtx()), equalTo(Long.valueOf(0)));

    /* set */
    this.impl.setExitLinkUp(1);
    this.impl.setExitLinkDown(2);
    this.impl.setEtx(3);
    this.impl.setDividerEtx(4);

    /* get */
    assertThat(Integer.valueOf(this.impl.getExitLinkUp()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getExitLinkDown()), equalTo(Integer.valueOf(2)));
    assertThat(Integer.valueOf(this.impl.getEtx()), equalTo(Integer.valueOf(3)));
    assertThat(Long.valueOf(this.impl.getDividerEtx()), equalTo(Long.valueOf(4)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigSgwCostsCalculation other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigSgwCostsCalculation();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = new JsonInfoConfigSgwCostsCalculation();
    other.setEtx(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoConfigSgwCostsCalculation other = new JsonInfoConfigSgwCostsCalculation();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* exitLinkUp */

    int intOrg = this.impl.getExitLinkUp();

    this.impl.setExitLinkUp(1);
    other.setExitLinkUp(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setExitLinkUp(2);
    other.setExitLinkUp(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setExitLinkUp(1);
    other.setExitLinkUp(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setExitLinkUp(intOrg);
    other.setExitLinkUp(intOrg);

    /* exitLinkDown */

    intOrg = this.impl.getExitLinkDown();

    this.impl.setExitLinkDown(1);
    other.setExitLinkDown(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setExitLinkDown(2);
    other.setExitLinkDown(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setExitLinkDown(1);
    other.setExitLinkDown(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setExitLinkDown(intOrg);
    other.setExitLinkDown(intOrg);

    /* etx */

    intOrg = this.impl.getEtx();

    this.impl.setEtx(1);
    other.setEtx(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setEtx(2);
    other.setEtx(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setEtx(1);
    other.setEtx(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setEtx(intOrg);
    other.setEtx(intOrg);

    /* dividerEtx */

    final long longOrg = this.impl.getDividerEtx();

    this.impl.setDividerEtx(1);
    other.setDividerEtx(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setDividerEtx(2);
    other.setDividerEtx(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setDividerEtx(1);
    other.setDividerEtx(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setDividerEtx(longOrg);
    other.setDividerEtx(longOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(923521)));

    /* set */
    this.impl.setExitLinkUp(1);
    this.impl.setExitLinkDown(2);
    this.impl.setEtx(3);
    this.impl.setDividerEtx(4);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(955331)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}