package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoPudPositionEntrySatInfo {
  private JsonInfoPudPositionEntrySatInfo impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoPudPositionEntrySatInfo();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Integer.valueOf(this.impl.getInUseCount()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getInUse(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getInUse().size()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getInViewCount()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getInView(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getInView().size()), equalTo(Integer.valueOf(0)));

    /* set */
    final Set<Integer> inUse = new TreeSet<>();
    inUse.add(Integer.valueOf(42));

    final Set<JsonInfoPudPositionEntrySatellite> inView = new TreeSet<>();
    final JsonInfoPudPositionEntrySatellite sat = new JsonInfoPudPositionEntrySatellite();
    sat.setId(42);
    inView.add(sat);

    this.impl.setInUseCount(1);
    this.impl.setInUse(inUse);
    this.impl.setInViewCount(3);
    this.impl.setInView(inView);

    /* get */
    assertThat(Integer.valueOf(this.impl.getInUseCount()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getInUse(), equalTo(inUse));
    assertThat(Integer.valueOf(this.impl.getInUse().size()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getInViewCount()), equalTo(Integer.valueOf(3)));
    assertThat(this.impl.getInView(), equalTo(inView));
    assertThat(Integer.valueOf(this.impl.getInView().size()), equalTo(Integer.valueOf(1)));

    /* set */
    this.impl.setInUse(null);
    this.impl.setInView(null);

    assertThat(this.impl.getInUse(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getInUse().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getInView(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getInView().size()), equalTo(Integer.valueOf(0)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final Set<Integer> inUse = new TreeSet<>();
    inUse.add(Integer.valueOf(42));

    final Set<JsonInfoPudPositionEntrySatellite> inView = new TreeSet<>();
    final JsonInfoPudPositionEntrySatellite sat = new JsonInfoPudPositionEntrySatellite();
    sat.setId(42);
    inView.add(sat);

    boolean r;
    JsonInfoPudPositionEntrySatInfo other;

    /* self */

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* null */

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* wrong class */

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* equal */

    other = new JsonInfoPudPositionEntrySatInfo();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* inUseCount */

    other = new JsonInfoPudPositionEntrySatInfo();
    other.setInUseCount(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* inUse */

    other = new JsonInfoPudPositionEntrySatInfo();
    other.setInUse(inUse);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* inViewCount */

    other = new JsonInfoPudPositionEntrySatInfo();
    other.setInViewCount(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* inView */

    other = new JsonInfoPudPositionEntrySatInfo();
    other.setInView(inView);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(923521)));

    /* set */
    final Set<Integer> inUse = new TreeSet<>();
    inUse.add(Integer.valueOf(42));

    final Set<JsonInfoPudPositionEntrySatellite> inView = new TreeSet<>();
    final JsonInfoPudPositionEntrySatellite sat = new JsonInfoPudPositionEntrySatellite();
    sat.setId(42);
    inView.add(sat);

    this.impl.setInUseCount(1);
    this.impl.setInUse(inUse);
    this.impl.setInViewCount(3);
    this.impl.setInView(inView);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(3168510)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}