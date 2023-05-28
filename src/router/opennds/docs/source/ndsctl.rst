Using ndsctl
############

openNDS includes ndsctl, a separate utility application. Some command line options:

* To print to stdout the status of the openNDS daemon:

    ``/usr/bin/ndsctl status``

* To print to stdout the list of clients and trusted devices in json format:

    ``/usr/bin/ndsctl json``

* To print to stdout the details of a particular client in json format (This is particularly useful if called from a FAS or Binauth script.):

    ``/usr/bin/ndsctl json [mac|ip|token|hid]``

    *Note: clients that are not in the preauthenticated state (ie CPD has not triggered redirection) will not be authenticated unless the configuration option "allow_preemptive_authentication" is enabled.*

* To authenticate client given their IP or MAC address:

    ``/usr/bin/ndsctl auth IP|MAC``

* To deauthenticate a currently authenticated client given their IP or MAC address:

    ``/usr/bin/ndsctl deauth IP|MAC``

* To b64encode a plain text string:

    ``/usr/bin/ndsctl b64encode "character string"``

* To b64decode a b64encoded string:

    ``/usr/bin/ndsctl b64decode "b64encodedstring"``

* To set the verbosity of logged messages to n:

    ``/usr/bin/ndsctl debuglevel n``

  * debuglevel 0 : Silent (only LOG_ERR and LOG_EMERG messages will be seen, otherwise there will be no logging.)
  * debuglevel 1 : LOG_ERR, LOG_EMERG, LOG_WARNING and LOG_NOTICE (this is the default level).
  * debuglevel 2 : debuglevel 1 + LOG_INFO
  * debuglevel 3 : debuglevel 2 + LOG_DEBUG

  All other levels are undefined and will result in debug level 3 being set.


For details, run ndsctl -h. (Note that the effect of ndsctl commands does not persist across openNDS restarts.)

