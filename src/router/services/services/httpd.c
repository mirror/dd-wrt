#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>

int
start_httpd (void)
{
  int ret = 0;

  if (nvram_invmatch ("http_enable", "0") && !is_exist ("/var/run/httpd.pid"))
    {
      chdir ("/www");
//      if (chdir ("/tmp/www") == 0)
//      cprintf ("[HTTPD Starting on /tmp/www]\n");
//      else
      cprintf ("[HTTPD Starting on /www]\n");
      if (nvram_invmatch ("http_lanport", ""))
	{
	  char *lan_port = nvram_safe_get ("http_lanport");
	  ret = eval ("httpd", "-p", lan_port);
	}
      else
	{
	  ret = eval ("httpd");
	  syslog (LOG_INFO, "httpd : http daemon successfully started\n");
	}
      chdir ("/");
    }
#ifdef HAVE_HTTPS
  if (nvram_invmatch ("https_enable", "0")
      && !is_exist ("/var/run/httpsd.pid"))
    {

      // Generate a new certificate
      //if(!is_exist("/tmp/cert.pem") || !is_exist("/tmp/key.pem"))
      //      eval("gencert.sh", BUILD_SECS);         

      chdir ("/www");
      ret = eval ("httpd", "-S");
      syslog (LOG_INFO, "httpd : https daemon successfully started\n");
      chdir ("/");
    }
#endif

  cprintf ("done\n");
  return ret;
}

int
stop_httpd (void)
{
  if (pidof ("httpd") > 0)
    syslog (LOG_INFO, "httpd : http daemon successfully stopped\n");
  //int ret = killps("httpd",NULL);
  int ret = killall ("httpd", SIGTERM);

  unlink ("/var/run/httpd.pid");
#ifdef HAVE_HTTPS
  unlink ("/var/run/httpsd.pid");
#endif
  cprintf ("done\n");
  return ret;
}
