package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigLinkQuality {
  private JsonInfoConfigLinkQuality impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigLinkQuality();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Integer.valueOf(this.impl.getLevel()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getFishEye()), equalTo(Boolean.FALSE));
    assertThat(Double.valueOf(this.impl.getAging()), equalTo(Double.valueOf(0.0)));
    assertThat(this.impl.getAlgorithm(), equalTo(""));

    /* set */
    this.impl.setLevel(1);
    this.impl.setFishEye(true);
    this.impl.setAging(2.1);
    this.impl.setAlgorithm("algo");

    /* get */
    assertThat(Integer.valueOf(this.impl.getLevel()), equalTo(Integer.valueOf(1)));
    assertThat(Boolean.valueOf(this.impl.getFishEye()), equalTo(Boolean.TRUE));
    assertThat(Double.valueOf(this.impl.getAging()), equalTo(Double.valueOf(2.1)));
    assertThat(this.impl.getAlgorithm(), equalTo("algo"));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoConfigLinkQuality other = new JsonInfoConfigLinkQuality();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* level */

    final int intOrg = this.impl.getLevel();

    this.impl.setLevel(1);
    other.setLevel(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setLevel(2);
    other.setLevel(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setLevel(1);
    other.setLevel(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setLevel(intOrg);
    other.setLevel(intOrg);

    /* fishEye */

    final boolean booleanOrg = this.impl.getFishEye();

    this.impl.setFishEye(false);
    other.setFishEye(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setFishEye(true);
    other.setFishEye(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setFishEye(true);
    other.setFishEye(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setFishEye(booleanOrg);
    other.setFishEye(booleanOrg);

    /* aging */

    final double doubleOrg = this.impl.getAging();

    this.impl.setAging(1.0);
    other.setAging(2.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setAging(2.0);
    other.setAging(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setAging(1.0);
    other.setAging(1.0);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setAging(doubleOrg);
    other.setAging(doubleOrg);

    /* algorithm */

    final String stringOrg = this.impl.getAlgorithm();

    this.impl.setAlgorithm(null);
    other.setAlgorithm(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setAlgorithm(null);
    other.setAlgorithm("algo");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setAlgorithm("algo");
    other.setAlgorithm(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setAlgorithm("algo");
    other.setAlgorithm("algo");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setAlgorithm("algo1");
    other.setAlgorithm("algo2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setAlgorithm("algo2");
    other.setAlgorithm("algo1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setAlgorithm(stringOrg);
    other.setAlgorithm(stringOrg);

  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigLinkQuality other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigLinkQuality();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setFishEye(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(2112278)));

    /* set */
    this.impl.setLevel(1);
    this.impl.setFishEye(true);
    this.impl.setAging(2.1);
    this.impl.setAlgorithm("algo");

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(218256225)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}