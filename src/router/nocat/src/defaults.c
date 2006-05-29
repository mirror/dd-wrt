# include <stdio.h>
# include "conf.h"
# include "config.h"
# include "mime.h"

struct conf_t default_conf[] = {
    /***  Gateway server networking values. ***/
    { "GatewayAddr",	"0.0.0.0" },
    { "GatewayPort",	"5280" },
    { "ListenQueue",	"10" },
    { "HandleTimeout",	"3"  },
    { "IdleTimeout",	"300" },
    { "MaxMissedARP",	"2" },
    { "SplashTimeout",	"21600" },

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

    { "GatewayName",	"the NoCat Network" },
    { "HomePage",	"http://nocat.net/" },
    { "SplashForm",	"splash.html" },
    { "StatusForm",	"status.html" },

    /***  No. of seconds before logins/renewals expire. ***/
    { "LoginTimeout",	 "300" },
    { "MinLoginTimeout", "60" },

    /***  Fraction of LoginTimeout to loiter before renewing. ***/
    { "RenewTimeout",	 ".75" },

    /***  Where to look for form templates? ***/
    { "DocumentRoot",	 NC_DOCUMENT_ROOT },

    /***  Default log level. ***/
    { "Verbosity",	 "5" },
    { "LogFacility",	 "syslog" },
    { "SyslogIdent",	 "NoCat" },

    /*** PGP stuff. ***/
    { "GpgPath",	 "/usr/bin/gpg" },
    { "PGPKeyPath",	 NC_PGP_PATH    },
    { "DecryptCmd",	 "$GpgPath --decrypt --homedir=$PGPKeyPath "
			 "--keyring trustedkeys.gpg --no-tty -o-" },

    /*** Trailing NULL ***/
    { "Version",	 PACKAGE_VERSION },
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
