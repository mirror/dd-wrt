package org.olsr.v1.info.api.commands;

import org.osgi.annotation.versioning.ProviderType;

/**
 * All commands supported by the jsoninfo and txtinfo OLSRd plugins
 */
@ProviderType
public enum InfoCommand {
  OLSRD_CONF("/con", "/olsrd.conf"), //
  ALL("/all", "/all"), //
  RUNTIME("/runtime", "/runtime"), //
  STARTUP("/startup", "/startup"), //
  NEIGHBORS("/nei", "/neighbors"), //
  LINKS("/lin", "/links"), //
  ROUTES("/rou", "/routes"), //
  HNA("/hna", "/hna"), //
  MID("/mid", "/mid"), //
  TOPOLOGY("/top", "/topology"), //
  GATEWAYS("/gat", "/gateways"), //
  INTERFACES("/int", "/interfaces"), //
  TWOHOP("/2ho", "/2hop"), //
  SMART_GATEWAY("/sgw", "/sgw"), //
  PUD_POSITION(null, "/pudposition"), //
  VERSION("/ver", "/version"), //
  CONFIG(null, "/config"), //
  PLUGINS(null, "/plugins"), //
  NEIGHBORS_FREIFUNK("/neighbours", "/neighbours");

  private String txtInfoCommand  = null;
  private String jsonInfoCommand = null;

  private InfoCommand(final String txtInfoCommand, final String jsonInfoCommand) {
    this.txtInfoCommand = txtInfoCommand;
    this.jsonInfoCommand = jsonInfoCommand;
  }

  /**
   * @return the command for the txtinfo plugin, null when not supported
   */
  public String getTxtInfoCommand() {
    return this.txtInfoCommand;
  }

  /**
   * @return the command for the jsoninfo plugin, null when not supported
   */
  public String getJsonInfoCommand() {
    return this.jsonInfoCommand;
  }
}