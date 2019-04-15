package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;
import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

@SuppressWarnings("static-method")
public class TestJsonInfoSgw {
  private JsonInfoSgw impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoSgw();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {

    /* initial */
    assertThat(this.impl.getSgw(), notNullValue());

    /* set */
    final JsonInfoSgwEntry entry = new JsonInfoSgwEntry();
    entry.setCost(123);
    final Set<JsonInfoSgwEntry> ipv4 = new TreeSet<>();
    ipv4.add(entry);
    final JsonInfoSgwFields sgw = new JsonInfoSgwFields();
    sgw.setIpv4(ipv4);
    this.impl.setSgw(sgw);

    /* get */
    assertThat(this.impl.getSgw(), equalTo(sgw));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final JsonInfoSgwEntry entry = new JsonInfoSgwEntry();
    entry.setCost(123);
    final Set<JsonInfoSgwEntry> ipv4 = new TreeSet<>();
    ipv4.add(entry);
    final JsonInfoSgwFields sgw = new JsonInfoSgwFields();
    sgw.setIpv4(ipv4);

    boolean r;
    final Object otherNull = null;
    final JsonInfoSgw otherSuperNotEqual = new JsonInfoSgw();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoSgw otherSame = new JsonInfoSgw();
    final JsonInfoSgw otherNotSame1 = new JsonInfoSgw();
    otherNotSame1.setSgw(sgw);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSame);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSgw(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame1.setSgw(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742840126)));

    /* set */
    final JsonInfoSgwEntry entry = new JsonInfoSgwEntry();
    entry.setCost(123);
    final Set<JsonInfoSgwEntry> ipv4 = new TreeSet<>();
    ipv4.add(entry);
    final JsonInfoSgwFields sgw = new JsonInfoSgwFields();
    sgw.setIpv4(ipv4);
    this.impl.setSgw(sgw);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(29490482)));

    this.impl.setSgw(null);
    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742840126)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/sgw.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoSgw sgw = Helpers.objectMapper.readValue(output, JsonInfoSgw.class);
    assertThat(sgw, notNullValue());
  }
}