package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigSgwEgress {
  private JsonInfoConfigSgwEgress impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigSgwEgress();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getInterfaces(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getInterfaces().size()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getInterfacesCount()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getFile(), equalTo(""));
    assertThat(Long.valueOf(this.impl.getFilePeriod()), equalTo(Long.valueOf(0)));

    /* set */
    final Set<String> interfaces = new TreeSet<>();
    interfaces.add("lo");
    this.impl.setInterfaces(interfaces);
    this.impl.setInterfacesCount(1);
    this.impl.setFile("file");
    this.impl.setFilePeriod(11);

    /* get */
    assertThat(this.impl.getInterfaces(), equalTo(interfaces));
    assertThat(Integer.valueOf(this.impl.getInterfacesCount()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getFile(), equalTo("file"));
    assertThat(Long.valueOf(this.impl.getFilePeriod()), equalTo(Long.valueOf(11)));

    this.impl.setInterfaces(null);
    assertThat(this.impl.getInterfaces(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getInterfaces().size()), equalTo(Integer.valueOf(0)));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigSgwEgress other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigSgwEgress();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* interfaces */

    final Set<String> interfaces = new TreeSet<>();
    interfaces.add("lo");

    this.impl.setInterfaces(null);
    other.setInterfaces(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setInterfaces(interfaces);
    other.setInterfaces(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaces(null);
    other.setInterfaces(interfaces);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaces(interfaces);
    other.setInterfaces(interfaces);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* interfacesCount */

    this.impl.setInterfacesCount(1);
    other.setInterfacesCount(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfacesCount(1);
    other.setInterfacesCount(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* file */

    this.impl.setFile(null);
    other.setFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setFile("file");
    other.setFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setFile(null);
    other.setFile("file");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setFile("file");
    other.setFile("file");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* filePeriod */

    this.impl.setFilePeriod(1);
    other.setFilePeriod(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setFilePeriod(1);
    other.setFilePeriod(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(923521)));

    /* set */
    final Set<String> interfaces = new TreeSet<>();
    interfaces.add("lo");
    this.impl.setInterfaces(interfaces);
    this.impl.setInterfacesCount(1);
    this.impl.setFile("file");
    this.impl.setFilePeriod(11);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(201405678)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}