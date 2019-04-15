package org.olsr.v1.info.api.dto;

import java.net.InetAddress;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * An interface entry in the {@link InfoCommand#INTERFACES} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoInterfacesEntry {
  private String                         name                           = "";
  private boolean                        configured                     = false;
  private boolean                        hostEmulation                  = false;
  private String                         hostEmulationAddress           = "";
  private JsonInfoOlsrInterface          olsrInterface                  = new JsonInfoOlsrInterface();
  private JsonInfoInterfaceConfiguration interfaceConfiguration         = new JsonInfoInterfaceConfiguration();
  private JsonInfoInterfaceConfiguration interfaceConfigurationDefaults = new JsonInfoInterfaceConfiguration();

  /**
   * @return the name
   */
  public String getName() {
    return this.name;
  }

  /**
   * @param name the name to set
   */
  @JsonProperty("name")
  public void setName(final String name) {
    if (name == null) {
      this.name = "";
    } else {
      this.name = name;
    }
  }

  /**
   * @return the configured
   */
  public boolean getConfigured() {
    return this.configured;
  }

  /**
   * @param configured the configured to set
   */
  @JsonProperty("configured")
  public void setConfigured(final boolean configured) {
    this.configured = configured;
  }

  /**
   * @return the hostEmulation
   */
  public boolean getHostEmulation() {
    return this.hostEmulation;
  }

  /**
   * @param hostEmulation the hostEmulation to set
   */
  @JsonProperty("hostEmulation")
  public void setHostEmulation(final boolean hostEmulation) {
    this.hostEmulation = hostEmulation;
  }

  /**
   * @return the host emulation IP address
   */
  public String getHostEmulationAddress() {
    return this.hostEmulationAddress;
  }

  /**
   * @param hostEmulationAddress the host emulation IP address to set
   */
  @JsonProperty("hostEmulationAddress")
  public void setHostEmulationAddress(final InetAddress hostEmulationAddress) {
    if (hostEmulationAddress == null) {
      this.hostEmulationAddress = "";
    } else {
      this.hostEmulationAddress = hostEmulationAddress.getHostAddress();
    }
  }

  /**
   * @return the olsrInterface
   */
  public JsonInfoOlsrInterface getOlsrInterface() {
    return this.olsrInterface;
  }

  /**
   * @param olsrInterface the olsrInterface to set
   */
  @JsonProperty("olsrInterface")
  public void setOlsrInterface(final JsonInfoOlsrInterface olsrInterface) {
    if (olsrInterface == null) {
      this.olsrInterface = new JsonInfoOlsrInterface();
    } else {
      this.olsrInterface = olsrInterface;
    }
  }

  /**
   * @return the interfaceConfiguration
   */
  public JsonInfoInterfaceConfiguration getInterfaceConfiguration() {
    return this.interfaceConfiguration;
  }

  /**
   * @param interfaceConfiguration the interfaceConfiguration to set
   */
  @JsonProperty("InterfaceConfiguration")
  public void setInterfaceConfiguration(final JsonInfoInterfaceConfiguration interfaceConfiguration) {
    if (interfaceConfiguration == null) {
      this.interfaceConfiguration = new JsonInfoInterfaceConfiguration();
    } else {
      this.interfaceConfiguration = interfaceConfiguration;
    }
  }

  /**
   * @return the interfaceConfigurationDefaults
   */
  public JsonInfoInterfaceConfiguration getInterfaceConfigurationDefaults() {
    return this.interfaceConfigurationDefaults;
  }

  /**
   * @param interfaceConfigurationDefaults the interfaceConfigurationDefaults to set
   */
  @JsonProperty("InterfaceConfigurationDefaults")
  public void setInterfaceConfigurationDefaults(final JsonInfoInterfaceConfiguration interfaceConfigurationDefaults) {
    if (interfaceConfigurationDefaults == null) {
      this.interfaceConfigurationDefaults = new JsonInfoInterfaceConfiguration();
    } else {
      this.interfaceConfigurationDefaults = interfaceConfigurationDefaults;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = (prime * result) + this.name.hashCode();
    result = (prime * result) + (this.configured ? 1231 : 1237);
    result = (prime * result) + (this.hostEmulation ? 1231 : 1237);
    result = (prime * result) + this.hostEmulationAddress.hashCode();
    result = (prime * result) + this.olsrInterface.hashCode();
    result = (prime * result) + this.interfaceConfiguration.hashCode();
    result = (prime * result) + this.interfaceConfigurationDefaults.hashCode();
    return result;
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (obj == null) {
      return false;
    }
    if (this.getClass() != obj.getClass()) {
      return false;
    }
    final JsonInfoInterfacesEntry other = (JsonInfoInterfacesEntry) obj;

    if (!this.name.equals(other.name)) {
      return false;
    }
    if (this.configured != other.configured) {
      return false;
    }
    if (this.hostEmulation != other.hostEmulation) {
      return false;
    }
    if (!this.hostEmulationAddress.equals(other.hostEmulationAddress)) {
      return false;
    }
    if (!this.olsrInterface.equals(other.olsrInterface)) {
      return false;
    }
    if (!this.interfaceConfiguration.equals(other.interfaceConfiguration)) {
      return false;
    }
    if (!this.interfaceConfigurationDefaults.equals(other.interfaceConfigurationDefaults)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoInterfacesEntry [name=");
    builder.append(this.name);
    builder.append(", configured=");
    builder.append(this.configured);
    builder.append(", hostEmulation=");
    builder.append(this.hostEmulation);
    builder.append(", hostEmulationAddress=");
    builder.append(this.hostEmulationAddress);
    builder.append(", olsrInterface=");
    builder.append(this.olsrInterface);
    builder.append(", interfaceConfiguration=");
    builder.append(this.interfaceConfiguration);
    builder.append(", interfaceConfigurationDefaults=");
    builder.append(this.interfaceConfigurationDefaults);
    builder.append("]");
    return builder.toString();
  }
}