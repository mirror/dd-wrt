package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

@SuppressWarnings("static-method")
public class TestJsonInfoInterfaces {
  private JsonInfoInterfaces impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoInterfaces();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getInterfaces(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getInterfaces().isEmpty()), equalTo(Boolean.TRUE));

    /* set */
    final List<JsonInfoInterfacesEntry> mid = new LinkedList<>();
    final JsonInfoInterfacesEntry entry = new JsonInfoInterfacesEntry();
    entry.setName("name");
    mid.add(entry);
    this.impl.setInterfaces(mid);

    /* get */
    assertThat(this.impl.getInterfaces(), equalTo(mid));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    final Object otherNull = null;
    final JsonInfoInterfaces otherSuperNotEqual = new JsonInfoInterfaces();
    otherSuperNotEqual.setTimeSinceStartup(321);
    final JsonInfoInterfaces other = new JsonInfoInterfaces();
    final JsonInfoInterfacesEntry e = new JsonInfoInterfacesEntry();
    final JsonInfoInterfacesEntry e1 = new JsonInfoInterfacesEntry();
    final List<JsonInfoInterfacesEntry> nei1 = new LinkedList<>();
    nei1.add(e);
    final List<JsonInfoInterfacesEntry> nei2 = new LinkedList<>();
    nei2.add(e1);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaces(null);
    other.setInterfaces(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setInterfaces(nei1);
    other.setInterfaces(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaces(null);
    other.setInterfaces(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaces(nei1);
    other.setInterfaces(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    final JsonInfoInterfacesEntry entry = new JsonInfoInterfacesEntry();
    nei2.add(entry);

    this.impl.setInterfaces(nei1);
    other.setInterfaces(nei2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setInterfaces(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810336)));

    final List<JsonInfoInterfacesEntry> mid = new LinkedList<>();
    final JsonInfoInterfacesEntry entry = new JsonInfoInterfacesEntry();
    entry.setName("name");
    mid.add(entry);
    this.impl.setInterfaces(mid);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(380319762)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/interfaces.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoInterfaces gws = Helpers.objectMapper.readValue(output, JsonInfoInterfaces.class);
    assertThat(gws, notNullValue());
  }
}