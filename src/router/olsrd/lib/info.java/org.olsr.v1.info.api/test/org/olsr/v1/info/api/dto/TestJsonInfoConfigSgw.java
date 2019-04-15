package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoConfigSgw {
  private JsonInfoConfigSgw impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoConfigSgw();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(Boolean.valueOf(this.impl.getEnabled()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getAlwaysRemoveServerTunnel()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getAllowNAT()), equalTo(Boolean.FALSE));
    assertThat(Boolean.valueOf(this.impl.getUplinkNAT()), equalTo(Boolean.FALSE));
    assertThat(Integer.valueOf(this.impl.getUseCount()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getTakeDownPercentage()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getInstanceId(), equalTo(""));
    assertThat(this.impl.getPolicyRoutingScript(), equalTo(""));
    assertThat(this.impl.getEgress(), notNullValue());
    assertThat(this.impl.getStatusFile(), equalTo(""));
    assertThat(Long.valueOf(this.impl.getTablesOffset()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getRulesOffset()), equalTo(Long.valueOf(0)));
    assertThat(Long.valueOf(this.impl.getPeriod()), equalTo(Long.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getStableCount()), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(this.impl.getThreshold()), equalTo(Integer.valueOf(0)));
    assertThat(this.impl.getCostsCalculation(), notNullValue());
    assertThat(Long.valueOf(this.impl.getMaxCostMaxEtx()), equalTo(Long.valueOf(0)));
    assertThat(this.impl.getUplink(), notNullValue());
    assertThat(this.impl.getBandwidth(), notNullValue());
    assertThat(this.impl.getPrefix(), notNullValue());

    /* set */
    final JsonInfoConfigSgwEgress egress = new JsonInfoConfigSgwEgress();
    egress.setFile("file");

    final JsonInfoConfigSgwCostsCalculation calc = new JsonInfoConfigSgwCostsCalculation();
    calc.setEtx(11);

    final JsonInfoConfigSgwBandwidth bw = new JsonInfoConfigSgwBandwidth();
    bw.setUplinkKbps(1234);

    final JsonInfoConfigSgwPrefix prefix = new JsonInfoConfigSgwPrefix();
    prefix.setLength(21);

    this.impl.setEnabled(true);
    this.impl.setAlwaysRemoveServerTunnel(true);
    this.impl.setAllowNAT(true);
    this.impl.setUplinkNAT(true);
    this.impl.setUseCount(1);
    this.impl.setTakeDownPercentage(2);
    this.impl.setInstanceId("instanceid");
    this.impl.setPolicyRoutingScript("routingscript");
    this.impl.setEgress(egress);
    this.impl.setStatusFile("statusfile");
    this.impl.setTablesOffset(3);
    this.impl.setRulesOffset(4);
    this.impl.setPeriod(5);
    this.impl.setStableCount(6);
    this.impl.setThreshold(7);
    this.impl.setCostsCalculation(calc);
    this.impl.setMaxCostMaxEtx(9);
    this.impl.setUplink("uplink");
    this.impl.setBandwidth(bw);
    this.impl.setPrefix(prefix);

    /* get */
    assertThat(Boolean.valueOf(this.impl.getEnabled()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getAlwaysRemoveServerTunnel()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getAllowNAT()), equalTo(Boolean.TRUE));
    assertThat(Boolean.valueOf(this.impl.getUplinkNAT()), equalTo(Boolean.TRUE));
    assertThat(Integer.valueOf(this.impl.getUseCount()), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(this.impl.getTakeDownPercentage()), equalTo(Integer.valueOf(2)));
    assertThat(this.impl.getInstanceId(), equalTo("instanceid"));
    assertThat(this.impl.getPolicyRoutingScript(), equalTo("routingscript"));
    assertThat(this.impl.getEgress(), equalTo(egress));
    assertThat(this.impl.getStatusFile(), equalTo("statusfile"));
    assertThat(Long.valueOf(this.impl.getTablesOffset()), equalTo(Long.valueOf(3)));
    assertThat(Long.valueOf(this.impl.getRulesOffset()), equalTo(Long.valueOf(4)));
    assertThat(Long.valueOf(this.impl.getPeriod()), equalTo(Long.valueOf(5)));
    assertThat(Integer.valueOf(this.impl.getStableCount()), equalTo(Integer.valueOf(6)));
    assertThat(Integer.valueOf(this.impl.getThreshold()), equalTo(Integer.valueOf(7)));
    assertThat(this.impl.getCostsCalculation(), equalTo(calc));
    assertThat(Long.valueOf(this.impl.getMaxCostMaxEtx()), equalTo(Long.valueOf(9)));
    assertThat(this.impl.getUplink(), equalTo("uplink"));
    assertThat(this.impl.getBandwidth(), equalTo(bw));
    assertThat(this.impl.getPrefix(), equalTo(prefix));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoConfigSgw other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoConfigSgw();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* enabled */

    boolean booleanOrg = this.impl.getEnabled();

    this.impl.setEnabled(false);
    other.setEnabled(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setEnabled(false);
    other.setEnabled(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setEnabled(true);
    other.setEnabled(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setEnabled(true);
    other.setEnabled(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setEnabled(booleanOrg);
    other.setEnabled(booleanOrg);

    /* alwaysRemoveServerTunnel */

    booleanOrg = this.impl.getAlwaysRemoveServerTunnel();

    this.impl.setAlwaysRemoveServerTunnel(false);
    other.setAlwaysRemoveServerTunnel(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setAlwaysRemoveServerTunnel(false);
    other.setAlwaysRemoveServerTunnel(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAlwaysRemoveServerTunnel(true);
    other.setAlwaysRemoveServerTunnel(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAlwaysRemoveServerTunnel(true);
    other.setAlwaysRemoveServerTunnel(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setAlwaysRemoveServerTunnel(booleanOrg);
    other.setAlwaysRemoveServerTunnel(booleanOrg);

    /* allowNAT */

    booleanOrg = this.impl.getAllowNAT();

    this.impl.setAllowNAT(false);
    other.setAllowNAT(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setAllowNAT(false);
    other.setAllowNAT(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAllowNAT(true);
    other.setAllowNAT(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setAllowNAT(true);
    other.setAllowNAT(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setAllowNAT(booleanOrg);
    other.setAllowNAT(booleanOrg);

    /* uplinkNAT */

    booleanOrg = this.impl.getUplinkNAT();

    this.impl.setUplinkNAT(false);
    other.setUplinkNAT(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUplinkNAT(false);
    other.setUplinkNAT(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUplinkNAT(true);
    other.setUplinkNAT(false);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUplinkNAT(true);
    other.setUplinkNAT(true);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUplinkNAT(booleanOrg);
    other.setUplinkNAT(booleanOrg);

    /* useCount */

    int intOrg = this.impl.getUseCount();

    this.impl.setUseCount(1);
    other.setUseCount(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUseCount(1);
    other.setUseCount(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUseCount(2);
    other.setUseCount(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUseCount(intOrg);
    other.setUseCount(intOrg);

    /* takeDownPercentage */

    intOrg = this.impl.getTakeDownPercentage();

    this.impl.setTakeDownPercentage(1);
    other.setTakeDownPercentage(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setTakeDownPercentage(1);
    other.setTakeDownPercentage(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTakeDownPercentage(2);
    other.setTakeDownPercentage(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTakeDownPercentage(intOrg);
    other.setTakeDownPercentage(intOrg);

    /* instanceId */

    String stringOrg = this.impl.getInstanceId();

    this.impl.setInstanceId(null);
    other.setInstanceId(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setInstanceId(null);
    other.setInstanceId("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInstanceId("string");
    other.setInstanceId(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setInstanceId("string");
    other.setInstanceId("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setInstanceId(stringOrg);
    other.setInstanceId(stringOrg);

    /* policyRoutingScript */

    stringOrg = this.impl.getPolicyRoutingScript();

    this.impl.setPolicyRoutingScript(null);
    other.setPolicyRoutingScript(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPolicyRoutingScript(null);
    other.setPolicyRoutingScript("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPolicyRoutingScript("string");
    other.setPolicyRoutingScript(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPolicyRoutingScript("string");
    other.setPolicyRoutingScript("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPolicyRoutingScript(stringOrg);
    other.setPolicyRoutingScript(stringOrg);

    /* egress */

    final JsonInfoConfigSgwEgress egress1 = new JsonInfoConfigSgwEgress();
    egress1.setFile("file");

    final JsonInfoConfigSgwEgress egressOrg = this.impl.getEgress();

    this.impl.setEgress(null);
    other.setEgress(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setEgress(null);
    other.setEgress(egress1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setEgress(egress1);
    other.setEgress(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setEgress(egress1);
    other.setEgress(egress1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setEgress(egressOrg);
    other.setEgress(egressOrg);

    /* statusFile */

    stringOrg = this.impl.getStatusFile();

    this.impl.setStatusFile(null);
    other.setStatusFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setStatusFile(null);
    other.setStatusFile("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setStatusFile("string");
    other.setStatusFile(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setStatusFile("string");
    other.setStatusFile("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setStatusFile(stringOrg);
    other.setStatusFile(stringOrg);

    /* tablesOffset */

    long longOrg = this.impl.getTablesOffset();

    this.impl.setTablesOffset(1);
    other.setTablesOffset(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setTablesOffset(1);
    other.setTablesOffset(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTablesOffset(2);
    other.setTablesOffset(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setTablesOffset(longOrg);
    other.setTablesOffset(longOrg);

    /* rulesOffset */

    longOrg = this.impl.getRulesOffset();

    this.impl.setRulesOffset(1);
    other.setRulesOffset(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setRulesOffset(1);
    other.setRulesOffset(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRulesOffset(2);
    other.setRulesOffset(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setRulesOffset(longOrg);
    other.setRulesOffset(longOrg);

    /* period */

    longOrg = this.impl.getPeriod();

    this.impl.setPeriod(1);
    other.setPeriod(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPeriod(1);
    other.setPeriod(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPeriod(2);
    other.setPeriod(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPeriod(longOrg);
    other.setPeriod(longOrg);

    /* stableCount */

    intOrg = this.impl.getStableCount();

    this.impl.setStableCount(1);
    other.setStableCount(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setStableCount(1);
    other.setStableCount(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setStableCount(2);
    other.setStableCount(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setStableCount(intOrg);
    other.setStableCount(intOrg);

    /* threshold */

    intOrg = this.impl.getThreshold();

    this.impl.setThreshold(1);
    other.setThreshold(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setThreshold(1);
    other.setThreshold(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setThreshold(2);
    other.setThreshold(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setThreshold(intOrg);
    other.setThreshold(intOrg);

    /* costsCalculation */

    final JsonInfoConfigSgwCostsCalculation costs1 = new JsonInfoConfigSgwCostsCalculation();
    costs1.setEtx(11);

    final JsonInfoConfigSgwCostsCalculation costsOrg = this.impl.getCostsCalculation();

    this.impl.setCostsCalculation(null);
    other.setCostsCalculation(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setCostsCalculation(null);
    other.setCostsCalculation(costs1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setCostsCalculation(costs1);
    other.setCostsCalculation(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setCostsCalculation(costs1);
    other.setCostsCalculation(costs1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setCostsCalculation(costsOrg);
    other.setCostsCalculation(costsOrg);

    /* maxCostMaxEtx */

    longOrg = this.impl.getMaxCostMaxEtx();

    this.impl.setMaxCostMaxEtx(1);
    other.setMaxCostMaxEtx(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setMaxCostMaxEtx(1);
    other.setMaxCostMaxEtx(2);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMaxCostMaxEtx(2);
    other.setMaxCostMaxEtx(1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setMaxCostMaxEtx(longOrg);
    other.setMaxCostMaxEtx(longOrg);

    /* uplink */

    stringOrg = this.impl.getUplink();

    this.impl.setUplink(null);
    other.setUplink(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUplink(null);
    other.setUplink("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUplink("string");
    other.setUplink(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setUplink("string");
    other.setUplink("string");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setUplink(stringOrg);
    other.setUplink(stringOrg);

    /* bandwidth */

    final JsonInfoConfigSgwBandwidth bw1 = new JsonInfoConfigSgwBandwidth();
    bw1.setUplinkKbps(123);

    final JsonInfoConfigSgwBandwidth bwOrg = this.impl.getBandwidth();

    this.impl.setBandwidth(null);
    other.setBandwidth(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setBandwidth(null);
    other.setBandwidth(bw1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setBandwidth(bw1);
    other.setBandwidth(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setBandwidth(bw1);
    other.setBandwidth(bw1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setBandwidth(bwOrg);
    other.setBandwidth(bwOrg);

    /* prefix */

    final JsonInfoConfigSgwPrefix prefix1 = new JsonInfoConfigSgwPrefix();
    prefix1.setLength(21);

    final JsonInfoConfigSgwPrefix prefixOrg = this.impl.getPrefix();

    this.impl.setPrefix(null);
    other.setPrefix(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPrefix(null);
    other.setPrefix(prefix1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPrefix(prefix1);
    other.setPrefix(null);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setPrefix(prefix1);
    other.setPrefix(prefix1);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setPrefix(prefixOrg);
    other.setPrefix(prefixOrg);
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(628246465)));

    /* set */
    final JsonInfoConfigSgwEgress egress = new JsonInfoConfigSgwEgress();
    egress.setFile("file");

    final JsonInfoConfigSgwCostsCalculation calc = new JsonInfoConfigSgwCostsCalculation();
    calc.setEtx(11);

    final JsonInfoConfigSgwBandwidth bw = new JsonInfoConfigSgwBandwidth();
    bw.setUplinkKbps(1234);

    final JsonInfoConfigSgwPrefix prefix = new JsonInfoConfigSgwPrefix();
    prefix.setLength(21);

    this.impl.setEnabled(true);
    this.impl.setAlwaysRemoveServerTunnel(true);
    this.impl.setAllowNAT(true);
    this.impl.setUplinkNAT(true);
    this.impl.setUseCount(1);
    this.impl.setTakeDownPercentage(2);
    this.impl.setInstanceId("instanceid");
    this.impl.setPolicyRoutingScript("routingscript");
    this.impl.setEgress(egress);
    this.impl.setStatusFile("statusfile");
    this.impl.setTablesOffset(3);
    this.impl.setRulesOffset(4);
    this.impl.setPeriod(5);
    this.impl.setStableCount(6);
    this.impl.setThreshold(7);
    this.impl.setCostsCalculation(calc);
    this.impl.setMaxCostMaxEtx(9);
    this.impl.setUplink("uplink");
    this.impl.setBandwidth(bw);
    this.impl.setPrefix(prefix);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(987180592)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}