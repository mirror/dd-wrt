package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoSgwEgressEntry {
  private JsonInfoSgwEgressEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoSgwEgressEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Boolean.valueOf(this.impl.getSelected()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getName(), equalTo(""));
    assertThat(Integer.valueOf(this.impl.getIfIndex()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getTableNr()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getRuleNr()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getBypassRuleNr()), equalTo(Integer.valueOf(0)));
    assertThat(Boolean.valueOf(this.impl.getUpPrevious()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getUpCurrent()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getUpChanged()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getBwPrevious(), notNullValue());
    assertThat(this.impl.getBwCurrent(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getBwCostsChanged()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getBwNetworkChanged()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getBwGatewayChanged()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getBwChanged()), equalTo(Boolean.FALSE));
    assertThat(this.impl.getNetworkRouteCurrent(), notNullValue());
    assertThat(this.impl.getEgressRouteCurrent(), notNullValue());
    assertThat(Boolean.valueOf(this.impl.getInEgressFile()), equalTo(Boolean.FALSE));

    /* set */
    final JsonInfoSgwBandwidth bwPrevious = new JsonInfoSgwBandwidth();
    bwPrevious.setCosts(1);

    final JsonInfoSgwBandwidth bwCurrent = new JsonInfoSgwBandwidth();
    bwCurrent.setCosts(2);

    final JsonInfoSgwRouteInfo networkRouteCurrent = new JsonInfoSgwRouteInfo();
    networkRouteCurrent.setDstStoreLength(3);

    final JsonInfoSgwRouteInfo egressRouteCurrent = new JsonInfoSgwRouteInfo();
    egressRouteCurrent.setDstStoreLength(4);

    this.impl.setSelected(true);
    this.impl.setName("name");
    this.impl.setIfIndex(1);
    this.impl.setTableNr(2);
    this.impl.setRuleNr(3);
    this.impl.setBypassRuleNr(4);
    this.impl.setUpPrevious(true);
    this.impl.setUpCurrent(true);
    this.impl.setUpChanged(true);
    this.impl.setBwPrevious(bwPrevious);
    this.impl.setBwCurrent(bwCurrent);
    this.impl.setBwCostsChanged(true);
    this.impl.setBwNetworkChanged(true);
    this.impl.setBwGatewayChanged(true);
    this.impl.setBwChanged(true);
    this.impl.setNetworkRouteCurrent(networkRouteCurrent);
    this.impl.setEgressRouteCurrent(egressRouteCurrent);
    this.impl.setInEgressFile(true);

    /* get */
    assertThat(Boolean.valueOf(this.impl.getSelected()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getName(), equalTo("name"));
    assertThat(Integer.valueOf(this.impl.getIfIndex()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getTableNr()), equalTo(Integer.valueOf(2)));
    assertThat(Integer.valueOf(this.impl.getRuleNr()), equalTo(Integer.valueOf(3)));
    assertThat(Integer.valueOf(this.impl.getBypassRuleNr()), equalTo(Integer.valueOf(4)));
    assertThat(Boolean.valueOf(this.impl.getUpPrevious()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getUpCurrent()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getUpChanged()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getBwPrevious(), equalTo(bwPrevious));
    assertThat(this.impl.getBwCurrent(), equalTo(bwCurrent));
    assertThat(Boolean.valueOf(this.impl.getBwCostsChanged()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getBwNetworkChanged()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getBwGatewayChanged()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getBwChanged()), equalTo(Boolean.TRUE));
    assertThat(this.impl.getNetworkRouteCurrent(), equalTo(networkRouteCurrent));
    assertThat(this.impl.getEgressRouteCurrent(), equalTo(egressRouteCurrent));
    assertThat(Boolean.valueOf(this.impl.getInEgressFile()), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;

    final JsonInfoSgwBandwidth bwPrevious = new JsonInfoSgwBandwidth();
    bwPrevious.setCosts(1);

    final JsonInfoSgwBandwidth bwCurrent = new JsonInfoSgwBandwidth();
    bwCurrent.setCosts(2);

    final JsonInfoSgwRouteInfo networkRouteCurrent = new JsonInfoSgwRouteInfo();
    networkRouteCurrent.setDstStoreLength(3);

    final JsonInfoSgwRouteInfo egressRouteCurrent = new JsonInfoSgwRouteInfo();
    egressRouteCurrent.setDstStoreLength(4);

    final JsonInfoSgwEgressEntry other = new JsonInfoSgwEgressEntry();
    final JsonInfoSgwBandwidth bwPrevious2 = new JsonInfoSgwBandwidth();
    bwPrevious2.setEgressUk(1);
    final JsonInfoSgwRouteInfo networkRouteCurrent2 = new JsonInfoSgwRouteInfo();
    networkRouteCurrent2.setActive(true);

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-1)));

    /* selected */

    boolean booleanOrg = this.impl.getSelected();

    this.impl.setSelected(false);
    other.setSelected(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSelected(false);
    other.setSelected(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSelected(true);
    other.setSelected(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSelected(true);
    other.setSelected(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSelected(booleanOrg);
    other.setSelected(booleanOrg);

    /* name */

    final String nameOrg = this.impl.getName();

    this.impl.setName(null);
    other.setName(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setName(null);
    other.setName("name2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setName("name2");
    other.setName(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setName("name1");
    other.setName("name1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setName("name1");
    other.setName("name2");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setName("name2");
    other.setName("name1");
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setName(nameOrg);
    other.setName(nameOrg);

    /* ifIndex */

    int intOrg = this.impl.getIfIndex();

    this.impl.setIfIndex(1);
    other.setIfIndex(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setIfIndex(2);
    other.setIfIndex(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setIfIndex(1);
    other.setIfIndex(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setIfIndex(intOrg);
    other.setIfIndex(intOrg);

    /* tableNr */

    intOrg = this.impl.getTableNr();

    this.impl.setTableNr(1);
    other.setTableNr(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setTableNr(2);
    other.setTableNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setTableNr(1);
    other.setTableNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setTableNr(intOrg);
    other.setTableNr(intOrg);

    /* ruleNr */

    intOrg = this.impl.getRuleNr();

    this.impl.setRuleNr(1);
    other.setRuleNr(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setRuleNr(2);
    other.setRuleNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setRuleNr(1);
    other.setRuleNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setRuleNr(intOrg);
    other.setRuleNr(intOrg);

    /* bypassRuleNr */

    intOrg = this.impl.getBypassRuleNr();

    this.impl.setBypassRuleNr(1);
    other.setBypassRuleNr(2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBypassRuleNr(2);
    other.setBypassRuleNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBypassRuleNr(1);
    other.setBypassRuleNr(1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBypassRuleNr(intOrg);
    other.setBypassRuleNr(intOrg);

    /* upPrevious */

    booleanOrg = this.impl.getUpPrevious();

    this.impl.setUpPrevious(false);
    other.setUpPrevious(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUpPrevious(false);
    other.setUpPrevious(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setUpPrevious(true);
    other.setUpPrevious(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setUpPrevious(true);
    other.setUpPrevious(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUpPrevious(booleanOrg);
    other.setUpPrevious(booleanOrg);

    /* upCurrent */

    booleanOrg = this.impl.getUpCurrent();

    this.impl.setUpCurrent(false);
    other.setUpCurrent(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUpCurrent(false);
    other.setUpCurrent(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setUpCurrent(true);
    other.setUpCurrent(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setUpCurrent(true);
    other.setUpCurrent(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUpCurrent(booleanOrg);
    other.setUpCurrent(booleanOrg);

    /* upChanged */

    booleanOrg = this.impl.getUpChanged();

    this.impl.setUpChanged(false);
    other.setUpChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUpChanged(false);
    other.setUpChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setUpChanged(true);
    other.setUpChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setUpChanged(true);
    other.setUpChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setUpChanged(booleanOrg);
    other.setUpChanged(booleanOrg);

    /* bwPrevious */

    JsonInfoSgwBandwidth bwOrg = this.impl.getBwPrevious();

    this.impl.setBwPrevious(null);
    other.setBwPrevious(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwPrevious(null);
    other.setBwPrevious(bwPrevious);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBwPrevious(bwPrevious);
    other.setBwPrevious(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBwPrevious(bwPrevious);
    other.setBwPrevious(bwPrevious);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwPrevious(bwPrevious);
    other.setBwPrevious(bwPrevious2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBwPrevious(bwPrevious2);
    other.setBwPrevious(bwPrevious);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBwPrevious(bwOrg);
    other.setBwPrevious(bwOrg);

    /* bwCurrent */

    bwOrg = this.impl.getBwCurrent();

    this.impl.setBwCurrent(null);
    other.setBwCurrent(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwCurrent(null);
    other.setBwCurrent(bwPrevious);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBwCurrent(bwPrevious);
    other.setBwCurrent(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBwCurrent(bwPrevious);
    other.setBwCurrent(bwPrevious);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwCurrent(bwPrevious);
    other.setBwCurrent(bwPrevious2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBwCurrent(bwPrevious2);
    other.setBwCurrent(bwPrevious);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBwCurrent(bwOrg);
    other.setBwCurrent(bwOrg);

    /* bwCostsChanged */

    booleanOrg = this.impl.getBwCostsChanged();

    this.impl.setBwCostsChanged(false);
    other.setBwCostsChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwCostsChanged(false);
    other.setBwCostsChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBwCostsChanged(true);
    other.setBwCostsChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBwCostsChanged(true);
    other.setBwCostsChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwCostsChanged(booleanOrg);
    other.setBwCostsChanged(booleanOrg);

    /* bwNetworkChanged */

    booleanOrg = this.impl.getBwNetworkChanged();

    this.impl.setBwNetworkChanged(false);
    other.setBwNetworkChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwNetworkChanged(false);
    other.setBwNetworkChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBwNetworkChanged(true);
    other.setBwNetworkChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBwNetworkChanged(true);
    other.setBwNetworkChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwNetworkChanged(booleanOrg);
    other.setBwNetworkChanged(booleanOrg);

    /* bwGatewayChanged */

    booleanOrg = this.impl.getBwGatewayChanged();

    this.impl.setBwGatewayChanged(false);
    other.setBwGatewayChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwGatewayChanged(false);
    other.setBwGatewayChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBwGatewayChanged(true);
    other.setBwGatewayChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBwGatewayChanged(true);
    other.setBwGatewayChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwGatewayChanged(booleanOrg);
    other.setBwGatewayChanged(booleanOrg);

    /* bwChanged */

    booleanOrg = this.impl.getBwChanged();

    this.impl.setBwChanged(false);
    other.setBwChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwChanged(false);
    other.setBwChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setBwChanged(true);
    other.setBwChanged(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setBwChanged(true);
    other.setBwChanged(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setBwChanged(booleanOrg);
    other.setBwChanged(booleanOrg);

    /* networkRouteCurrent */

    JsonInfoSgwRouteInfo riOrg = this.impl.getNetworkRouteCurrent();

    this.impl.setNetworkRouteCurrent(null);
    other.setNetworkRouteCurrent(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNetworkRouteCurrent(null);
    other.setNetworkRouteCurrent(networkRouteCurrent);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNetworkRouteCurrent(networkRouteCurrent);
    other.setNetworkRouteCurrent(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNetworkRouteCurrent(networkRouteCurrent);
    other.setNetworkRouteCurrent(networkRouteCurrent);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setNetworkRouteCurrent(networkRouteCurrent);
    other.setNetworkRouteCurrent(networkRouteCurrent2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setNetworkRouteCurrent(networkRouteCurrent2);
    other.setNetworkRouteCurrent(networkRouteCurrent);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setNetworkRouteCurrent(riOrg);
    other.setNetworkRouteCurrent(riOrg);

    /* egressRouteCurrent */

    riOrg = this.impl.getEgressRouteCurrent();

    this.impl.setEgressRouteCurrent(null);
    other.setEgressRouteCurrent(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setEgressRouteCurrent(null);
    other.setEgressRouteCurrent(networkRouteCurrent);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setEgressRouteCurrent(networkRouteCurrent);
    other.setEgressRouteCurrent(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setEgressRouteCurrent(networkRouteCurrent);
    other.setEgressRouteCurrent(networkRouteCurrent);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setEgressRouteCurrent(networkRouteCurrent);
    other.setEgressRouteCurrent(networkRouteCurrent2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setEgressRouteCurrent(networkRouteCurrent2);
    other.setEgressRouteCurrent(networkRouteCurrent);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setEgressRouteCurrent(riOrg);
    other.setEgressRouteCurrent(riOrg);

    /* inEgressFile */

    booleanOrg = this.impl.getInEgressFile();

    this.impl.setInEgressFile(false);
    other.setInEgressFile(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setInEgressFile(false);
    other.setInEgressFile(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setInEgressFile(true);
    other.setInEgressFile(false);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setInEgressFile(true);
    other.setInEgressFile(true);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setInEgressFile(booleanOrg);
    other.setInEgressFile(booleanOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoSgwEgressEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoSgwEgressEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setTableNr(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(1393428876)));

    /* set */
    final JsonInfoSgwBandwidth bwPrevious = new JsonInfoSgwBandwidth();
    bwPrevious.setCosts(1);

    final JsonInfoSgwBandwidth bwCurrent = new JsonInfoSgwBandwidth();
    bwCurrent.setCosts(2);

    final JsonInfoSgwRouteInfo networkRouteCurrent = new JsonInfoSgwRouteInfo();
    networkRouteCurrent.setDstStoreLength(3);

    final JsonInfoSgwRouteInfo egressRouteCurrent = new JsonInfoSgwRouteInfo();
    egressRouteCurrent.setDstStoreLength(4);

    this.impl.setSelected(true);
    this.impl.setName("name");
    this.impl.setIfIndex(1);
    this.impl.setTableNr(2);
    this.impl.setRuleNr(3);
    this.impl.setBypassRuleNr(4);
    this.impl.setUpPrevious(true);
    this.impl.setUpCurrent(true);
    this.impl.setUpChanged(true);
    this.impl.setBwPrevious(bwPrevious);
    this.impl.setBwCurrent(bwCurrent);
    this.impl.setBwCostsChanged(true);
    this.impl.setBwNetworkChanged(true);
    this.impl.setBwGatewayChanged(true);
    this.impl.setBwChanged(true);
    this.impl.setNetworkRouteCurrent(networkRouteCurrent);
    this.impl.setEgressRouteCurrent(egressRouteCurrent);
    this.impl.setInEgressFile(true);

    r = this.impl.hashCode();
    assertThat(Long.valueOf(r), equalTo(Long.valueOf(-947683234)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}