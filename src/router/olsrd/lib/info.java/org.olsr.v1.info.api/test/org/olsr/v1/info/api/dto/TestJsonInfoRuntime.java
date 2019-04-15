package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

@SuppressWarnings("static-method")
public class TestJsonInfoRuntime {
  private JsonInfoRuntime impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoRuntime();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getNeighbors(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getNeighbors().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getLinks(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getLinks().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getRoutes(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getRoutes().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getHna(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getHna().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getMid(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getMid().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getTopology(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getTopology().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getGateways(), notNullValue());
    assertThat(this.impl.getInterfaces(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getInterfaces().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getTwoHop(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getTwoHop().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getSgw(), notNullValue());
    assertThat(this.impl.getPudPosition(), notNullValue());

    /* set */
    final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();
    final JsonInfoNeighborsEntry neighborsEntry = new JsonInfoNeighborsEntry();
    neighbors.add(neighborsEntry);

    final Set<JsonInfoLinksEntry> links = new TreeSet<>();
    final JsonInfoLinksEntry linksEntry = new JsonInfoLinksEntry();
    links.add(linksEntry);

    final Set<JsonInfoRoutesEntry> routes = new TreeSet<>();
    final JsonInfoRoutesEntry routesEntry = new JsonInfoRoutesEntry();
    routes.add(routesEntry);

    final Set<JsonInfoHnaEntry> hna = new TreeSet<>();
    final JsonInfoHnaEntry hnaEntry = new JsonInfoHnaEntry();
    hna.add(hnaEntry);

    final List<JsonInfoMidEntry> mid = new LinkedList<>();
    final JsonInfoMidEntry midEntry = new JsonInfoMidEntry();
    mid.add(midEntry);

    final Set<JsonInfoTopologyEntry> topology = new TreeSet<>();
    final JsonInfoTopologyEntry topologyEntry = new JsonInfoTopologyEntry();
    topology.add(topologyEntry);

    final JsonInfoGatewaysFields gateways = new JsonInfoGatewaysFields();
    final Set<JsonInfoGatewaysEntry> ipv4 = new TreeSet<>();
    final JsonInfoGatewaysEntry gw4Entry = new JsonInfoGatewaysEntry();
    gw4Entry.setCost(1);
    ipv4.add(gw4Entry);
    gateways.setIpv4(ipv4);

    final List<JsonInfoInterfacesEntry> interfaces = new LinkedList<>();
    final JsonInfoInterfacesEntry interfacesEntry = new JsonInfoInterfacesEntry();
    interfaces.add(interfacesEntry);

    final List<JsonInfoTwoHopEntry> twohop = new LinkedList<>();
    final JsonInfoTwoHopEntry twohopEntry = new JsonInfoTwoHopEntry();
    twohop.add(twohopEntry);

    final JsonInfoSgwFields sgw = new JsonInfoSgwFields();
    final Set<JsonInfoSgwEgressEntry> egress = new TreeSet<>();
    final JsonInfoSgwEgressEntry sgwEgressEntry = new JsonInfoSgwEgressEntry();
    sgwEgressEntry.setRuleNr(1);
    egress.add(sgwEgressEntry);
    sgw.setEgress(egress);

    final JsonInfoPudPositionEntry pudPosition = new JsonInfoPudPositionEntry();
    pudPosition.setLatitude(1.1);

    this.impl.setNeighbors(neighbors);
    this.impl.setLinks(links);
    this.impl.setRoutes(routes);
    this.impl.setHna(hna);
    this.impl.setMid(mid);
    this.impl.setTopology(topology);
    this.impl.setGateways(gateways);
    this.impl.setInterfaces(interfaces);
    this.impl.setTwoHop(twohop);
    this.impl.setSgw(sgw);
    this.impl.setPudPosition(pudPosition);

    /* get */
    assertThat(this.impl.getNeighbors(), equalTo(neighbors));
    assertThat(Integer.valueOf(this.impl.getNeighbors().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getLinks(), equalTo(links));
    assertThat(Integer.valueOf(this.impl.getLinks().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getRoutes(), equalTo(routes));
    assertThat(Integer.valueOf(this.impl.getRoutes().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getHna(), equalTo(hna));
    assertThat(Integer.valueOf(this.impl.getHna().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getMid(), equalTo(mid));
    assertThat(Integer.valueOf(this.impl.getMid().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getTopology(), equalTo(topology));
    assertThat(Integer.valueOf(this.impl.getTopology().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getGateways(), equalTo(gateways));
    assertThat(this.impl.getInterfaces(), equalTo(interfaces));
    assertThat(Integer.valueOf(this.impl.getInterfaces().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getTwoHop(), equalTo(twohop));
    assertThat(Integer.valueOf(this.impl.getTwoHop().size()), equalTo(Integer.valueOf(1)));
    assertThat(this.impl.getSgw(), equalTo(sgw));
    assertThat(this.impl.getPudPosition(), equalTo(pudPosition));

    /* set */
    this.impl.setNeighbors(null);
    this.impl.setLinks(null);
    this.impl.setRoutes(null);
    this.impl.setHna(null);
    this.impl.setMid(null);
    this.impl.setTopology(null);
    this.impl.setGateways(null);
    this.impl.setInterfaces(null);
    this.impl.setTwoHop(null);
    this.impl.setSgw(null);
    this.impl.setPudPosition(null);

    /* get */
    assertThat(this.impl.getNeighbors(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getNeighbors().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getLinks(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getLinks().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getRoutes(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getRoutes().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getHna(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getHna().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getMid(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getMid().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getTopology(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getTopology().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getGateways(), notNullValue());
    assertThat(this.impl.getInterfaces(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getInterfaces().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getTwoHop(), notNullValue());
    assertThat(Integer.valueOf(this.impl.getTwoHop().size()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getSgw(), notNullValue());
    assertThat(this.impl.getPudPosition(), notNullValue());
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();
    final JsonInfoNeighborsEntry neighborsEntry = new JsonInfoNeighborsEntry();
    neighbors.add(neighborsEntry);

    final Set<JsonInfoLinksEntry> links = new TreeSet<>();
    final JsonInfoLinksEntry linksEntry = new JsonInfoLinksEntry();
    links.add(linksEntry);

    final Set<JsonInfoRoutesEntry> routes = new TreeSet<>();
    final JsonInfoRoutesEntry routesEntry = new JsonInfoRoutesEntry();
    routes.add(routesEntry);

    final Set<JsonInfoHnaEntry> hna = new TreeSet<>();
    final JsonInfoHnaEntry hnaEntry = new JsonInfoHnaEntry();
    hna.add(hnaEntry);

    final List<JsonInfoMidEntry> mid = new LinkedList<>();
    final JsonInfoMidEntry midEntry = new JsonInfoMidEntry();
    mid.add(midEntry);

    final Set<JsonInfoTopologyEntry> topology = new TreeSet<>();
    final JsonInfoTopologyEntry topologyEntry = new JsonInfoTopologyEntry();
    topology.add(topologyEntry);

    final JsonInfoGatewaysFields gateways = new JsonInfoGatewaysFields();
    final Set<JsonInfoGatewaysEntry> ipv4 = new TreeSet<>();
    final JsonInfoGatewaysEntry gw4Entry = new JsonInfoGatewaysEntry();
    gw4Entry.setCost(1);
    ipv4.add(gw4Entry);
    gateways.setIpv4(ipv4);

    final List<JsonInfoInterfacesEntry> interfaces = new LinkedList<>();
    final JsonInfoInterfacesEntry interfacesEntry = new JsonInfoInterfacesEntry();
    interfaces.add(interfacesEntry);

    final List<JsonInfoTwoHopEntry> twohop = new LinkedList<>();
    final JsonInfoTwoHopEntry twohopEntry = new JsonInfoTwoHopEntry();
    twohop.add(twohopEntry);

    final JsonInfoSgwFields sgw = new JsonInfoSgwFields();
    final Set<JsonInfoSgwEgressEntry> egress = new TreeSet<>();
    final JsonInfoSgwEgressEntry sgwEgressEntry = new JsonInfoSgwEgressEntry();
    sgwEgressEntry.setRuleNr(1);
    egress.add(sgwEgressEntry);
    sgw.setEgress(egress);

    final JsonInfoPudPositionEntry pudPosition = new JsonInfoPudPositionEntry();
    pudPosition.setLatitude(1.1);

    boolean r;
    final Object otherNull = null;
    final JsonInfoRuntime otherEqual = new JsonInfoRuntime();
    final JsonInfoRuntime otherSuperNotEqual = new JsonInfoRuntime();
    otherSuperNotEqual.setTimeSinceStartup(321);

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherSuperNotEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* neighbors */

    this.impl.setNeighbors(neighbors);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setNeighbors(null);

    /* links */

    this.impl.setLinks(links);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setLinks(null);

    /* routes */

    this.impl.setRoutes(routes);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setRoutes(null);

    /* hna */

    this.impl.setHna(hna);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setHna(null);

    /* mid */

    this.impl.setMid(mid);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setMid(null);

    /* topology */

    this.impl.setTopology(topology);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setTopology(null);

    /* gateways */

    this.impl.setGateways(null);
    otherEqual.setGateways(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setGateways(null);
    otherEqual.setGateways(gateways);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setGateways(gateways);
    otherEqual.setGateways(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setGateways(gateways);
    otherEqual.setGateways(gateways);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setGateways(null);
    otherEqual.setGateways(null);

    /* interfaces */

    this.impl.setInterfaces(interfaces);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setInterfaces(null);

    /* twohop */

    this.impl.setTwoHop(twohop);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
    this.impl.setTwoHop(null);

    /* sgw */

    this.impl.setSgw(null);
    otherEqual.setSgw(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSgw(null);
    otherEqual.setSgw(sgw);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setSgw(sgw);
    otherEqual.setSgw(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setSgw(sgw);
    otherEqual.setSgw(sgw);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setSgw(null);
    otherEqual.setSgw(null);

    /* pudPosition */

    this.impl.setPudPosition(null);
    otherEqual.setPudPosition(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPudPosition(null);
    otherEqual.setPudPosition(pudPosition);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPudPosition(pudPosition);
    otherEqual.setPudPosition(null);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPudPosition(pudPosition);
    otherEqual.setPudPosition(pudPosition);
    r = this.impl.equals(otherEqual);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPudPosition(null);
    otherEqual.setPudPosition(null);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    this.impl.setNeighbors(null);
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(841357544)));

    /* set */
    final Set<JsonInfoNeighborsEntry> neighbors = new TreeSet<>();
    final JsonInfoNeighborsEntry neighborsEntry = new JsonInfoNeighborsEntry();
    neighbors.add(neighborsEntry);

    final Set<JsonInfoLinksEntry> links = new TreeSet<>();
    final JsonInfoLinksEntry linksEntry = new JsonInfoLinksEntry();
    links.add(linksEntry);

    final Set<JsonInfoRoutesEntry> routes = new TreeSet<>();
    final JsonInfoRoutesEntry routesEntry = new JsonInfoRoutesEntry();
    routes.add(routesEntry);

    final Set<JsonInfoHnaEntry> hna = new TreeSet<>();
    final JsonInfoHnaEntry hnaEntry = new JsonInfoHnaEntry();
    hna.add(hnaEntry);

    final List<JsonInfoMidEntry> mid = new LinkedList<>();
    final JsonInfoMidEntry midEntry = new JsonInfoMidEntry();
    mid.add(midEntry);

    final Set<JsonInfoTopologyEntry> topology = new TreeSet<>();
    final JsonInfoTopologyEntry topologyEntry = new JsonInfoTopologyEntry();
    topology.add(topologyEntry);

    final JsonInfoGatewaysFields gateways = new JsonInfoGatewaysFields();
    final Set<JsonInfoGatewaysEntry> ipv4 = new TreeSet<>();
    final JsonInfoGatewaysEntry gw4Entry = new JsonInfoGatewaysEntry();
    gw4Entry.setCost(1);
    ipv4.add(gw4Entry);
    gateways.setIpv4(ipv4);

    final List<JsonInfoInterfacesEntry> interfaces = new LinkedList<>();
    final JsonInfoInterfacesEntry interfacesEntry = new JsonInfoInterfacesEntry();
    interfaces.add(interfacesEntry);

    final List<JsonInfoTwoHopEntry> twohop = new LinkedList<>();
    final JsonInfoTwoHopEntry twohopEntry = new JsonInfoTwoHopEntry();
    twohop.add(twohopEntry);

    final JsonInfoSgwFields sgw = new JsonInfoSgwFields();
    final Set<JsonInfoSgwEgressEntry> egress = new TreeSet<>();
    final JsonInfoSgwEgressEntry sgwEgressEntry = new JsonInfoSgwEgressEntry();
    sgwEgressEntry.setRuleNr(1);
    egress.add(sgwEgressEntry);
    sgw.setEgress(egress);

    final JsonInfoPudPositionEntry pudPosition = new JsonInfoPudPositionEntry();
    pudPosition.setLatitude(1.1);

    this.impl.setNeighbors(neighbors);
    this.impl.setLinks(links);
    this.impl.setRoutes(routes);
    this.impl.setHna(hna);
    this.impl.setMid(mid);
    this.impl.setTopology(topology);
    this.impl.setGateways(gateways);
    this.impl.setInterfaces(interfaces);
    this.impl.setTwoHop(twohop);
    this.impl.setSgw(sgw);
    this.impl.setPudPosition(pudPosition);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-853119233)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/runtime.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoRuntime gws = Helpers.objectMapper.readValue(output, JsonInfoRuntime.class);
    assertThat(gws, notNullValue());
  }
}