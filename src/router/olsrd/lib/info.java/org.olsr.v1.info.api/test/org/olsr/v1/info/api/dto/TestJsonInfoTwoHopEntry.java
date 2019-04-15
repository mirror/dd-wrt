package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.LinkedList;
import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoTwoHopEntry {
  private JsonInfoTwoHopEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoTwoHopEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    List<InetAddress> expected = new LinkedList<>();

    /* initial */
    assertThat(this.impl.getTwoHopNeighbors(), equalTo(expected));

    /* set */
    expected = new LinkedList<>();
    final InetAddress ip = InetAddress.getByName("127.0.0.2");
    expected.add(ip);
    this.impl.setTwoHopNeighbors(expected);

    /* get */
    assertThat(this.impl.getTwoHopNeighbors(), equalTo(expected));
  }

  @Test(timeout = 8000)
  public void testEquals() throws UnknownHostException {
    boolean r;
    JsonInfoTwoHopEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoTwoHopEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* twoHopNeighbors */

    final List<InetAddress> expected = new LinkedList<>();
    final InetAddress ip = InetAddress.getByName("127.0.0.2");
    expected.add(ip);

    this.impl.setTwoHopNeighbors(expected);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other.setTwoHopNeighbors(expected);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    this.impl.setTwoHopNeighbors(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-322355606)));

    /* set */
    final List<InetAddress> expected = new LinkedList<>();
    final InetAddress ip = InetAddress.getByName("127.0.0.2");
    expected.add(ip);
    this.impl.setTwoHopNeighbors(expected);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1808350858)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}