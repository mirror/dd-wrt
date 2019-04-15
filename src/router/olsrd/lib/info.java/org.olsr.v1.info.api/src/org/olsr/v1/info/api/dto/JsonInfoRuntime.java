package org.olsr.v1.info.api.dto;

import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * The {@link InfoCommand#ALL} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoRuntime extends JsonInfoBase {
  private final Set<JsonInfoNeighborsEntry>   neighbors   = new TreeSet<>();
  private final Set<JsonInfoLinksEntry>       links       = new TreeSet<>();
  private final Set<JsonInfoRoutesEntry>      routes      = new TreeSet<>();
  private final Set<JsonInfoHnaEntry>         hna         = new TreeSet<>();
  private final List<JsonInfoMidEntry>        mid         = new LinkedList<>();
  private final Set<JsonInfoTopologyEntry>    topology    = new TreeSet<>();
  private JsonInfoGatewaysFields              gateways    = new JsonInfoGatewaysFields();
  private final List<JsonInfoInterfacesEntry> interfaces  = new LinkedList<>();
  private final List<JsonInfoTwoHopEntry>     twohop      = new LinkedList<>();
  private JsonInfoSgwFields                   sgw         = new JsonInfoSgwFields();
  private JsonInfoPudPositionEntry            pudPosition = new JsonInfoPudPositionEntry();

  /**
   * @return the neighbors response
   */
  public Set<JsonInfoNeighborsEntry> getNeighbors() {
    return this.neighbors;
  }

  /**
   * @param neighbors the neighbors response to set
   */
  @JsonProperty("neighbors")
  public void setNeighbors(final Set<JsonInfoNeighborsEntry> neighbors) {
    this.neighbors.clear();
    if (neighbors != null) {
      this.neighbors.addAll(neighbors);
    }
  }

  /**
   * @return the links response
   */
  public Set<JsonInfoLinksEntry> getLinks() {
    return this.links;
  }

  /**
   * @param links the links response to set
   */
  @JsonProperty("links")
  public void setLinks(final Set<JsonInfoLinksEntry> links) {
    this.links.clear();
    if (links != null) {
      this.links.addAll(links);
    }
  }

  /**
   * @return the routes response
   */
  public Set<JsonInfoRoutesEntry> getRoutes() {
    return this.routes;
  }

  /**
   * @param routes the routes response to set
   */
  @JsonProperty("routes")
  public void setRoutes(final Set<JsonInfoRoutesEntry> routes) {
    this.routes.clear();
    if (routes != null) {
      this.routes.addAll(routes);
    }
  }

  /**
   * @return the HNA response
   */
  public Set<JsonInfoHnaEntry> getHna() {
    return this.hna;
  }

  /**
   * @param hna the HNA response to set
   */
  @JsonProperty("hna")
  public void setHna(final Set<JsonInfoHnaEntry> hna) {
    this.hna.clear();
    if (hna != null) {
      this.hna.addAll(hna);
    }
  }

  /**
   * @return the mid response
   */
  public List<JsonInfoMidEntry> getMid() {
    return this.mid;
  }

  /**
   * @param mid the mid response to set
   */
  @JsonProperty("mid")
  public void setMid(final List<JsonInfoMidEntry> mid) {
    this.mid.clear();
    if (mid != null) {
      this.mid.addAll(mid);
    }
  }

  /**
   * @return the topology response
   */
  public Set<JsonInfoTopologyEntry> getTopology() {
    return this.topology;
  }

  /**
   * @param topology the topology response to set
   */
  @JsonProperty("topology")
  public void setTopology(final Set<JsonInfoTopologyEntry> topology) {
    this.topology.clear();
    if (topology != null) {
      this.topology.addAll(topology);
    }
  }

  /**
   * @return the gateways response
   */
  public JsonInfoGatewaysFields getGateways() {
    return this.gateways;
  }

  /**
   * @param gateways the gateways response to set
   */
  @JsonProperty("gateways")
  public void setGateways(final JsonInfoGatewaysFields gateways) {
    if (gateways == null) {
      this.gateways = new JsonInfoGatewaysFields();
    } else {
      this.gateways = gateways;
    }
  }

  /**
   * @return the interfaces response
   */
  public List<JsonInfoInterfacesEntry> getInterfaces() {
    return this.interfaces;
  }

  /**
   * @param interfaces the interfaces response to set
   */
  @JsonProperty("interfaces")
  public void setInterfaces(final List<JsonInfoInterfacesEntry> interfaces) {
    this.interfaces.clear();
    if (interfaces != null) {
      this.interfaces.addAll(interfaces);
    }
  }

  /**
   * @return the 2-hop response
   */
  public List<JsonInfoTwoHopEntry> getTwoHop() {
    return this.twohop;
  }

  /**
   * @param twohop the 2-hop response to set
   */
  @JsonProperty("2hop")
  public void setTwoHop(final List<JsonInfoTwoHopEntry> twohop) {
    this.twohop.clear();
    if (twohop != null) {
      this.twohop.addAll(twohop);
    }
  }

  /**
   * @return the smart-gateways response
   */
  public JsonInfoSgwFields getSgw() {
    return this.sgw;
  }

  /**
   * @param sgw the smart-gateways response to set
   */
  @JsonProperty("sgw")
  public void setSgw(final JsonInfoSgwFields sgw) {
    if (sgw == null) {
      this.sgw = new JsonInfoSgwFields();
    } else {
      this.sgw = sgw;
    }
  }

  /**
   * @return the pud position response
   */
  public JsonInfoPudPositionEntry getPudPosition() {
    return this.pudPosition;
  }

  /**
   * @param pudPosition the pud position response to set
   */
  @JsonProperty("pudPosition")
  public void setPudPosition(final JsonInfoPudPositionEntry pudPosition) {
    if (pudPosition == null) {
      this.pudPosition = new JsonInfoPudPositionEntry();
    } else {
      this.pudPosition = pudPosition;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = super.hashCode();
    result = (prime * result) + this.neighbors.hashCode();
    result = (prime * result) + this.links.hashCode();
    result = (prime * result) + this.routes.hashCode();
    result = (prime * result) + this.hna.hashCode();
    result = (prime * result) + this.mid.hashCode();
    result = (prime * result) + this.topology.hashCode();
    result = (prime * result) + this.gateways.hashCode();
    result = (prime * result) + this.interfaces.hashCode();
    result = (prime * result) + this.twohop.hashCode();
    result = (prime * result) + this.sgw.hashCode();
    result = (prime * result) + this.pudPosition.hashCode();
    return result;
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (!super.equals(obj)) {
      return false;
    }
    /* class comparison is already done in super.equals() */
    final JsonInfoRuntime other = (JsonInfoRuntime) obj;

    if (!this.neighbors.equals(other.neighbors)) {
      return false;
    }
    if (!this.links.equals(other.links)) {
      return false;
    }
    if (!this.routes.equals(other.routes)) {
      return false;
    }
    if (!this.hna.equals(other.hna)) {
      return false;
    }
    if (!this.mid.equals(other.mid)) {
      return false;
    }
    if (!this.topology.equals(other.topology)) {
      return false;
    }
    if (!this.gateways.equals(other.gateways)) {
      return false;
    }
    if (!this.interfaces.equals(other.interfaces)) {
      return false;
    }
    if (!this.twohop.equals(other.twohop)) {
      return false;
    }
    if (!this.sgw.equals(other.sgw)) {
      return false;
    }
    if (!this.pudPosition.equals(other.pudPosition)) {
      return false;
    }

    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoAll [neighbors=");
    builder.append(this.neighbors);
    builder.append(", links=");
    builder.append(this.links);
    builder.append(", routes=");
    builder.append(this.routes);
    builder.append(", hna=");
    builder.append(this.hna);
    builder.append(", mid=");
    builder.append(this.mid);
    builder.append(", topology=");
    builder.append(this.topology);
    builder.append(", gateways=");
    builder.append(this.gateways);
    builder.append(", interfaces=");
    builder.append(this.interfaces);
    builder.append(", twohop=");
    builder.append(this.twohop);
    builder.append(", sgw=");
    builder.append(this.sgw);
    builder.append(", pudPosition=");
    builder.append(this.pudPosition);
    builder.append(",");
    builder.append(super.toString());
    builder.append("]");
    return builder.toString();
  }
}