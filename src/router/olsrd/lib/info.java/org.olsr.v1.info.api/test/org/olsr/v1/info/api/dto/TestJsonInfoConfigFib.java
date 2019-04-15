package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigFib {
  private JsonInfoConfigFib impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigFib();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getMetric(), equalTo(""));
    assertThat(this.impl.getMetricDefault(), equalTo(""));

    /* set */
    this.impl.setMetric("a");
    this.impl.setMetricDefault("b");

    /* get */
    assertThat(this.impl.getMetric(), equalTo("a"));
    assertThat(this.impl.getMetricDefault(), equalTo("b"));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigFib other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigFib();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setMetric("a");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoConfigFib other = new JsonInfoConfigFib();

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* metric */

    String stringOrg = this.impl.getMetric();

    this.impl.setMetric(null);
    other.setMetric(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMetric(null);
    other.setMetric("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMetric("a");
    other.setMetric(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMetric("a");
    other.setMetric("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMetric("a1");
    other.setMetric("a2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMetric("a2");
    other.setMetric("a1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMetric(stringOrg);
    other.setMetric(stringOrg);

    /* metricDefault */

    stringOrg = this.impl.getMetricDefault();

    this.impl.setMetricDefault(null);
    other.setMetricDefault(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMetricDefault(null);
    other.setMetricDefault("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMetricDefault("a");
    other.setMetricDefault(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMetricDefault("a");
    other.setMetricDefault("a");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setMetricDefault("a1");
    other.setMetricDefault("a2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setMetricDefault("a2");
    other.setMetricDefault("a1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setMetricDefault(stringOrg);
    other.setMetricDefault(stringOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(961)));

    /* set */
    this.impl.setMetric("a");
    this.impl.setMetricDefault("b");

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(4066)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}