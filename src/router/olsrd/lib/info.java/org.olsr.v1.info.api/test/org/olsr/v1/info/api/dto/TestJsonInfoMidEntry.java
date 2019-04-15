package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoMidEntry {
  private JsonInfoMidEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoMidEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getMain(), notNullValue());
    assertThat(this.impl.getAliases(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getAliases().size()), equalTo(Integer.valueOf(0)));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");
    final InetAddress ipAddress2 = InetAddress.getByName("127.0.0.2");

    final JsonInfoTimedIpAddress main = new JsonInfoTimedIpAddress();
    main.setIpAddress(ipAddress);
    main.setValidityTime(11);
    final JsonInfoTimedIpAddress alias = new JsonInfoTimedIpAddress();
    alias.setIpAddress(ipAddress2);
    alias.setValidityTime(10);
    final Set<JsonInfoTimedIpAddress> aliases = new TreeSet<>();

    this.impl.setMain(main);
    this.impl.setAliases(aliases);

    /* get */
    assertThat(this.impl.getMain(), equalTo(main));
    assertThat(this.impl.getAliases(), equalTo(aliases));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoMidEntry other;
    final JsonInfoTimedIpAddress main = new JsonInfoTimedIpAddress();
    main.setValidityTime(123);
    final Set<JsonInfoTimedIpAddress> aliases = new TreeSet<>();
    final JsonInfoTimedIpAddress e = new JsonInfoTimedIpAddress();
    aliases.add(e);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoMidEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* main */

    this.impl.setMain(null);
    other.setMain(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMain(null);
    other.setMain(main);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMain(main);
    other.setMain(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMain(main);
    other.setMain(main);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* aliases */

    this.impl.setAliases(null);
    other.setAliases(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setAliases(null);
    other.setAliases(aliases);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAliases(aliases);
    other.setAliases(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAliases(aliases);
    other.setAliases(aliases);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(30752)));

    /* set */
    final InetAddress ipAddress = InetAddress.getByName("127.0.0.1");
    final InetAddress ipAddress2 = InetAddress.getByName("127.0.0.2");

    final JsonInfoTimedIpAddress main = new JsonInfoTimedIpAddress();
    main.setIpAddress(ipAddress);
    main.setValidityTime(11);
    final JsonInfoTimedIpAddress alias = new JsonInfoTimedIpAddress();
    alias.setIpAddress(ipAddress2);
    alias.setValidityTime(10);
    final Set<JsonInfoTimedIpAddress> aliases = new TreeSet<>();

    this.impl.setMain(main);
    this.impl.setAliases(aliases);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-139672654)));

    this.impl.setAliases(null);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-139672654)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}