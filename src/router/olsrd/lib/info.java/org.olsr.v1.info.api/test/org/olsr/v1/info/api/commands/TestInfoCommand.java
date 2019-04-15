package org.olsr.v1.info.api.commands;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.nullValue;
import static org.junit.Assert.assertThat;

import org.junit.Test;

@SuppressWarnings("static-method")
public class TestInfoCommand {
  @Test(timeout = 8000)
  public void testToString() {
    final InfoCommand tmp = InfoCommand.valueOf(InfoCommand.SMART_GATEWAY.toString());
    assertThat(tmp, equalTo(InfoCommand.SMART_GATEWAY));

    assertThat(InfoCommand.SMART_GATEWAY.toString(), equalTo("SMART_GATEWAY"));
  }

  @Test(timeout = 8000)
  public void testGetCommand() {
    assertThat(InfoCommand.OLSRD_CONF.getTxtInfoCommand(), equalTo("/con"));
    assertThat(InfoCommand.OLSRD_CONF.getJsonInfoCommand(), equalTo("/olsrd.conf"));
    assertThat(InfoCommand.ALL.getTxtInfoCommand(), equalTo("/all"));
    assertThat(InfoCommand.ALL.getJsonInfoCommand(), equalTo("/all"));
    assertThat(InfoCommand.RUNTIME.getTxtInfoCommand(), equalTo("/runtime"));
    assertThat(InfoCommand.RUNTIME.getJsonInfoCommand(), equalTo("/runtime"));
    assertThat(InfoCommand.STARTUP.getTxtInfoCommand(), equalTo("/startup"));
    assertThat(InfoCommand.STARTUP.getJsonInfoCommand(), equalTo("/startup"));
    assertThat(InfoCommand.NEIGHBORS.getTxtInfoCommand(), equalTo("/nei"));
    assertThat(InfoCommand.NEIGHBORS.getJsonInfoCommand(), equalTo("/neighbors"));
    assertThat(InfoCommand.LINKS.getTxtInfoCommand(), equalTo("/lin"));
    assertThat(InfoCommand.LINKS.getJsonInfoCommand(), equalTo("/links"));
    assertThat(InfoCommand.ROUTES.getTxtInfoCommand(), equalTo("/rou"));
    assertThat(InfoCommand.ROUTES.getJsonInfoCommand(), equalTo("/routes"));
    assertThat(InfoCommand.HNA.getTxtInfoCommand(), equalTo("/hna"));
    assertThat(InfoCommand.HNA.getJsonInfoCommand(), equalTo("/hna"));
    assertThat(InfoCommand.MID.getTxtInfoCommand(), equalTo("/mid"));
    assertThat(InfoCommand.MID.getJsonInfoCommand(), equalTo("/mid"));
    assertThat(InfoCommand.TOPOLOGY.getTxtInfoCommand(), equalTo("/top"));
    assertThat(InfoCommand.TOPOLOGY.getJsonInfoCommand(), equalTo("/topology"));
    assertThat(InfoCommand.GATEWAYS.getTxtInfoCommand(), equalTo("/gat"));
    assertThat(InfoCommand.GATEWAYS.getJsonInfoCommand(), equalTo("/gateways"));
    assertThat(InfoCommand.INTERFACES.getTxtInfoCommand(), equalTo("/int"));
    assertThat(InfoCommand.INTERFACES.getJsonInfoCommand(), equalTo("/interfaces"));
    assertThat(InfoCommand.TWOHOP.getTxtInfoCommand(), equalTo("/2ho"));
    assertThat(InfoCommand.TWOHOP.getJsonInfoCommand(), equalTo("/2hop"));
    assertThat(InfoCommand.SMART_GATEWAY.getTxtInfoCommand(), equalTo("/sgw"));
    assertThat(InfoCommand.SMART_GATEWAY.getJsonInfoCommand(), equalTo("/sgw"));
    assertThat(InfoCommand.PUD_POSITION.getTxtInfoCommand(), nullValue());
    assertThat(InfoCommand.PUD_POSITION.getJsonInfoCommand(), equalTo("/pudposition"));
    assertThat(InfoCommand.VERSION.getTxtInfoCommand(), equalTo("/ver"));
    assertThat(InfoCommand.VERSION.getJsonInfoCommand(), equalTo("/version"));
    assertThat(InfoCommand.CONFIG.getTxtInfoCommand(), nullValue());
    assertThat(InfoCommand.CONFIG.getJsonInfoCommand(), equalTo("/config"));
    assertThat(InfoCommand.PLUGINS.getTxtInfoCommand(), nullValue());
    assertThat(InfoCommand.PLUGINS.getJsonInfoCommand(), equalTo("/plugins"));
    assertThat(InfoCommand.NEIGHBORS_FREIFUNK.getTxtInfoCommand(), equalTo("/neighbours"));
    assertThat(InfoCommand.NEIGHBORS_FREIFUNK.getJsonInfoCommand(), equalTo("/neighbours"));
  }
}