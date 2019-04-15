package org.olsr.v1.info.proxy;

import org.osgi.service.metatype.annotations.AttributeDefinition;
import org.osgi.service.metatype.annotations.ObjectClassDefinition;

@ObjectClassDefinition(name = "OLSRd Info Proxy", description = "Configuration for the OLSRd Info Proxy service")
@interface Config {
  static public final String ADDRESS_DEFAULT            = "127.0.0.1";
  static public final int    LOCAL_PORT_DEFAULT         = 2301;
  static public final int    LOCAL_PORT_MIN             = 1;
  static public final int    LOCAL_PORT_MAX             = 65535;
  static public final int    PORT_MIN                   = 1;
  static public final int    PORT_MAX                   = 65535;
  static public final int    PORT_TXTINFO_DEFAULT       = 2006;
  static public final int    PORT_JSONINFO_DEFAULT      = 2007;
  static public final int    CONNECT_TIMEOUT_DEFAULT    = 500;
  static public final int    CONNECT_TIMEOUT_MIN        = 1;
  static public final int    CONNECT_TIMEOUT_MAX        = 60000;
  static public final int    SOCKET_TIMEOUT_DEFAULT     = 1000;
  static public final int    SOCKET_TIMEOUT_MIN         = 1;
  static public final int    SOCKET_TIMEOUT_MAX         = 60000;
  static public final int    RANDOM_SLEEP_DEFAULT       = 250;
  static public final int    RANDOM_SLEEP_MIN           = 100;
  static public final int    CONNECTION_RETRIES_DEFAULT = 4;
  static public final int    CONNECTION_RETRIES_MIN     = 0;

  @AttributeDefinition(required = false,
      description = "The host name or IP address of the OLSRd info plugins (default = " + ADDRESS_DEFAULT + ")")
  String address() default ADDRESS_DEFAULT;

  @AttributeDefinition(min = "" + LOCAL_PORT_MIN, max = "" + LOCAL_PORT_MAX, required = false,
      description = "The preferred local port to connect from to the OLSRd info plugins (default = "
          + LOCAL_PORT_DEFAULT + ")")
  int localPort() default LOCAL_PORT_DEFAULT;

  @AttributeDefinition(min = "" + PORT_MIN, max = "" + PORT_MAX, required = false,
      description = "The port of the OLSRd txtinfo plugin (default = " + PORT_TXTINFO_DEFAULT + ")")
  int portTxtInfo() default PORT_TXTINFO_DEFAULT;

  @AttributeDefinition(min = "" + PORT_MIN, max = "" + PORT_MAX, required = false,
      description = "The port of the OLSRd jsoninfo plugin (default = " + PORT_JSONINFO_DEFAULT + ")")
  int portJsonInfo() default PORT_JSONINFO_DEFAULT;

  @AttributeDefinition(min = "" + CONNECT_TIMEOUT_MIN, max = "" + CONNECT_TIMEOUT_MAX, required = false,
      description = "The connection timeout (msec) (default = " + CONNECT_TIMEOUT_DEFAULT + ")")
  int connectionTimeout() default CONNECT_TIMEOUT_DEFAULT;

  @AttributeDefinition(min = "" + SOCKET_TIMEOUT_MIN, max = "" + SOCKET_TIMEOUT_MAX, required = false,
      description = "The socket (read) timeout (msec) (default = " + SOCKET_TIMEOUT_DEFAULT + ")")
  int socketTimeout() default SOCKET_TIMEOUT_DEFAULT;

  @AttributeDefinition(min = "" + RANDOM_SLEEP_MIN, required = false,
      description = "The maximum number of random msec to sleep before reconnecting after a connection failure (default = "
          + RANDOM_SLEEP_DEFAULT + ")")
  int randomSleep() default RANDOM_SLEEP_DEFAULT;

  @AttributeDefinition(min = "" + CONNECTION_RETRIES_MIN, required = false,
      description = "The number of connection retries to perform after a connection failure (default = "
          + CONNECTION_RETRIES_DEFAULT + ")")
  int connectionRetries() default CONNECTION_RETRIES_DEFAULT;
}