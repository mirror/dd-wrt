# include <stdio.h>
# include "conf.h"
# include "config.h"
# include "mime.h"

struct conf_t default_conf[] = {
    { "Version",	 PACKAGE_VERSION },

    /***  Default log level. ***/
    { "LogFacility",	 "syslog" },
    { "SyslogIdent",	 "NoCat" },
    { "Verbosity",	 "5" },

    /***  Gateway server networking values. ***/
    { "GatewayName",	"the NoCat Network" },
    { "GatewayAddr",	"192.168.1.1" },
    { "GatewayPort",	"5280" },
    { "ListenQueue",	"10" },
    { "HandleTimeout",	"3"  },

    /*** This is now all auto-detected in set_conf_defaults()
    { "ExternalDevice",	NULL },
    { "InternalDevice",	NULL },
    { "LocalNetwork",	NULL },
    ****/

    { "FirewallPath",	NC_FIREWALL_PATH },
    { "ResetCmd",	"$FirewallPath/initialize.fw" },
    { "PermitCmd",	"$FirewallPath/access.fw permit $MAC $IP $Class" },
    { "DenyCmd",	"$FirewallPath/access.fw deny $MAC $IP $Class" },
    { "InitCmd",	"$FirewallPath/reset.fw" },

    { "CheckARPCmd", "$FirewallPath/test_arp.sh $IP" },
    { "TrafficInputCountCmd", "$FirewallPath/traffic_input_count.sh $IP" },
    { "TrafficOutputCountCmd", "$FirewallPath/traffic_output_count.sh $IP" },
	  
    /***  Where to look for form templates? ***/
    { "DocumentRoot",	 NC_DOCUMENT_ROOT },
    { "StatusForm",	"status.html" },
    { "SplashForm",	"splash.html" },
    { "SplashURL",       ""},
    { "SplashURLTimeout",	"21600" },

    /*** How often to check for Peers which have timed out ***/
    { "PeerCheckTimeout",  "30" },
    /***  No. of seconds before logins/renewals expire. ***/
    /* Add for Open mode?: { "SplashTimeout",	 "86400" }, */
    { "LoginTimeout",	 "86400" },
    { "MinLoginTimeout", "60" },
    /***  Fraction of LoginTimeout to loiter before renewing. ***/
    { "RenewTimeout",	 ".75" },
    /***  Idle logout after MaxMissedARP*IdleTimeout seconds expire ***/
    { "IdleTimeout",	"0" },
    { "MaxMissedARP",	"5" },

    /*** ForcedRedirect 0=original-location, 1=Redirect to HomePage, 2=form-POST selected ***/
    { "ForcedRedirect",	"0" },
    { "HomePage",	"http://www.portless.net/" },

    /*** PGP stuff. ***/
    { "GpgPath",	 "/usr/bin/gpg" },
    { "PGPKeyPath",	 NC_PGP_PATH    },
    { "DecryptCmd",	 "$GpgPath --decrypt --homedir=$PGPKeyPath "
			 "--keyring trustedkeys.gpg --no-tty -o-" },

    /*** Trailing NULL ***/
    { NULL, NULL }
};

struct mime_type_t mime_types[] = {
    { "hqx", "application/mac-binhex40" },
    { "doc", "application/msword" },
    { "bin", "application/octet-stream" },
    { "class", "application/octet-stream" },
    { "so", "application/octet-stream" },
    { "pdf", "application/pdf" },
    { "ps", "application/postscript" },
    { "ppt", "application/vnd.ms-powerpoint" },
    { "bz2", "application/x-bzip2" },
    { "gz", "application/x-gzip" },
    { "tgz", "application/x-gzip" },
    { "js", "application/x-javascript" },
    { "ogg", "application/x-ogg" },
    { "swf", "application/x-shockwave-flash" },
    { "xhtml", "application/xhtml+xml" },
    { "xht", "application/xhtml+xml" },
    { "zip", "application/zip" },
    { "mid", "audio/midi" },
    { "mp2", "audio/mpeg" },
    { "mp3", "audio/mpeg" },
    { "m3u", "audio/x-mpegurl" },
    { "ra", "audio/x-realaudio" },
    { "bmp", "image/bmp" },
    { "gif", "image/gif" },
    { "jpeg", "image/jpeg" },
    { "jpg", "image/jpeg" },
    { "jpe", "image/jpeg" },
    { "png", "image/png" },
    { "tiff", "image/tiff" },
    { "tif", "image/tiff" },
    { "css", "text/css" },
    { "html", "text/html" },
    { "htm", "text/html" },
    { "asc", "text/plain" },
    { "txt", "text/plain" },
    { "rtx", "text/richtext" },
    { "rtf", "text/rtf" },
    { "xml", "text/xml" },
    { "xsl", "text/xml" },
    { "mpeg", "video/mpeg" },
    { "mpg", "video/mpeg" },
    { "mpe", "video/mpeg" },
    { "qt", "video/quicktime" },
    { "mov", "video/quicktime" },
    { "avi", "video/x-msvideo" },
    { "rmm", "audio/x-pn-realaudio" },
    { "ram", "audio/x-pn-realaudio" },
    { "ra", "audio/vnd.rn-realaudio" },
    { "smi", "application/smil" },
    { "smil", "application/smil" },
    { "rt", "text/vnd.rn-realtext" },
    { "rv", "video/vnd.rn-realvideo" },
    { "rm", "application/vnd.rn-realmedia" },
    { "wav", "audio/wav" },
    { NULL, NULL } };
