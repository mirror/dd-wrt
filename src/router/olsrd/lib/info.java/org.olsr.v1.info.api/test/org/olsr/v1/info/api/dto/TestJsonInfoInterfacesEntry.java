package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoInterfacesEntry {
  private JsonInfoInterfacesEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoInterfacesEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() throws UnknownHostException {
    /* initial */
    assertThat(this.impl.getName(), equalTo(""));
    assertThat(Boolean.valueOf(this.impl.getConfigured()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getHostEmulation()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getHostEmulationAddress(), equalTo(""));
    assertThat(this.impl.getOlsrInterface(), notNullValue());
    assertThat(this.impl.getInterfaceConfiguration(), notNullValue());
    assertThat(this.impl.getInterfaceConfigurationDefaults(), notNullValue());

    /* set */
    final InetAddress addr = InetAddress.getByName("127.0.0.1");

    final JsonInfoOlsrInterface olsrInterface = new JsonInfoOlsrInterface();
    olsrInterface.setFlags(1);

    final JsonInfoInterfaceConfiguration interfaceConfiguration = new JsonInfoInterfaceConfiguration();
    interfaceConfiguration.setMode("mode1");

    final JsonInfoInterfaceConfiguration interfaceConfigurationDefaults = new JsonInfoInterfaceConfiguration();
    interfaceConfigurationDefaults.setMode("mode2");

    this.impl.setName("name");
    this.impl.setConfigured(true);
    this.impl.setHostEmulation(true);
    this.impl.setHostEmulationAddress(addr);
    this.impl.setOlsrInterface(olsrInterface);
    this.impl.setInterfaceConfiguration(interfaceConfiguration);
    this.impl.setInterfaceConfigurationDefaults(interfaceConfigurationDefaults);

    /* get */
    assertThat(this.impl.getName(), equalTo("name"));
    assertThat(Boolean.valueOf(this.impl.getConfigured()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getHostEmulation()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getHostEmulationAddress(), equalTo(addr.getHostAddress()));
    assertThat(this.impl.getOlsrInterface(), equalTo(olsrInterface));
    assertThat(this.impl.getInterfaceConfiguration(), equalTo(interfaceConfiguration));
    assertThat(this.impl.getInterfaceConfigurationDefaults(), equalTo(interfaceConfigurationDefaults));
  }

  @Test(timeout = 8000)
  public void testEquals() throws UnknownHostException {
    boolean r;
    JsonInfoInterfacesEntry other;

    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    final JsonInfoOlsrInterface olsrInterface = new JsonInfoOlsrInterface();
    olsrInterface.setFlags(1);
    final JsonInfoInterfaceConfiguration interfaceConfiguration = new JsonInfoInterfaceConfiguration();
    interfaceConfiguration.setMode("mode1");
    final JsonInfoInterfaceConfiguration interfaceConfigurationDefaults = new JsonInfoInterfaceConfiguration();
    interfaceConfigurationDefaults.setMode("mode2");

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoInterfacesEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* name */

    this.impl.setName(null);
    other.setName(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setName(null);
    other.setName("name");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setName("name");
    other.setName(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setName("name");
    other.setName("name");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* configured */

    this.impl.setConfigured(false);
    other.setConfigured(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setConfigured(false);
    other.setConfigured(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setConfigured(true);
    other.setConfigured(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setConfigured(true);
    other.setConfigured(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* hostEmulation */

    this.impl.setHostEmulation(false);
    other.setHostEmulation(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHostEmulation(false);
    other.setHostEmulation(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHostEmulation(true);
    other.setHostEmulation(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHostEmulation(true);
    other.setHostEmulation(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* hostEmulationAddress */

    this.impl.setHostEmulationAddress(null);
    other.setHostEmulationAddress(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHostEmulationAddress(null);
    other.setHostEmulationAddress(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHostEmulationAddress(addr);
    other.setHostEmulationAddress(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHostEmulationAddress(addr);
    other.setHostEmulationAddress(addr);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* olsrInterface */

    this.impl.setOlsrInterface(null);
    other.setOlsrInterface(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setOlsrInterface(null);
    other.setOlsrInterface(olsrInterface);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setOlsrInterface(olsrInterface);
    other.setOlsrInterface(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setOlsrInterface(olsrInterface);
    other.setOlsrInterface(olsrInterface);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* interfaceConfiguration */

    this.impl.setInterfaceConfiguration(null);
    other.setInterfaceConfiguration(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setInterfaceConfiguration(null);
    other.setInterfaceConfiguration(interfaceConfiguration);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaceConfiguration(interfaceConfiguration);
    other.setInterfaceConfiguration(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaceConfiguration(interfaceConfiguration);
    other.setInterfaceConfiguration(interfaceConfiguration);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* interfaceConfigurationDefaults */

    this.impl.setInterfaceConfigurationDefaults(null);
    other.setInterfaceConfigurationDefaults(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setInterfaceConfigurationDefaults(null);
    other.setInterfaceConfigurationDefaults(interfaceConfigurationDefaults);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaceConfigurationDefaults(interfaceConfigurationDefaults);
    other.setInterfaceConfigurationDefaults(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInterfaceConfigurationDefaults(interfaceConfigurationDefaults);
    other.setInterfaceConfigurationDefaults(interfaceConfigurationDefaults);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() throws UnknownHostException {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1872225481)));

    /* set */
    final InetAddress addr = InetAddress.getByName("127.0.0.1");
    final JsonInfoOlsrInterface olsrInterface = new JsonInfoOlsrInterface();
    olsrInterface.setFlags(1);
    final JsonInfoInterfaceConfiguration interfaceConfiguration = new JsonInfoInterfaceConfiguration();
    interfaceConfiguration.setMode("mode1");
    final JsonInfoInterfaceConfiguration interfaceConfigurationDefaults = new JsonInfoInterfaceConfiguration();
    interfaceConfigurationDefaults.setMode("mode2");

    this.impl.setName("name");
    this.impl.setConfigured(true);
    this.impl.setHostEmulation(true);
    this.impl.setHostEmulationAddress(addr);
    this.impl.setOlsrInterface(olsrInterface);
    this.impl.setInterfaceConfiguration(interfaceConfiguration);
    this.impl.setInterfaceConfigurationDefaults(interfaceConfigurationDefaults);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1971245301)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}