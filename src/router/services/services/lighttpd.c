/*
 * privoxy.c
 *
 * Copyright (C) 2013 Richard Schneidt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#ifdef HAVE_WEBSERVER
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>

void start_lighttpd(void)
{
	char path[64];
	if (!nvram_matchi("lighttpd_enable", 1))
		return;

	FILE *fp;
	eval("mkdir", "-p", "/tmp/lighttpd/logs");
	eval("mkdir", "-p", nvram_safe_get("lighttpd_root"));

	fp = fopen("/tmp/lighttpd.conf", "wb");
	if (fp != NULL) {
		fprintf(fp,
			"debug.log-request-handling   = \"disable\"\n"
			"debug.log-request-header     = \"disable\"\n"
			"debug.log-response-header    = \"disable\"\n"
			"debug.log-condition-handling = \"disable\"\n"
			"server.max-request-size      = 65000\n"
			"accesslog.filename           = \"/tmp/lighttpd/logs/lighttpd.access.log\"\n"
			"server.errorlog              = \"/tmp/lighttpd/logs/lighttpd.error.log\"\n"
			"server.breakagelog           = \"/tmp/lighttpd/logs/lighttpd.breakage.log\"\n"
			"server.dir-listing           = \"enable\"\n"
			"server.modules               = (\n"
			"\"mod_rewrite\",\n"
			"\"mod_setenv\",\n"
			"\"mod_access\",\n"
			"\"mod_alias\",\n"
			"\"mod_expire\",\n"
			"\"mod_webdav\",\n"
			"\"mod_auth\",\n"
			"\"mod_simple_vhost\",\n"
			"\"mod_redirect\",\n"
			"\"mod_fastcgi\",\n"
			"\"mod_cgi\",\n"
			"\"mod_compress\",\n"
			"\"mod_accesslog\",\n"
			"\"mod_openssl\"\n"
			")\n\n"
			"server.indexfiles           = ( \"index.php\", \"index.html\", \"index.htm\" )\n\n"
			"mimetype.assign             = (\n"
			"\".png\"  => \"image/png\",\n"
			"\".jpg\"  => \"image/jpeg\",\n"
			"\".jpeg\" => \"image/jpeg\",\n"
			"\".gif\"  => \"image/gif\",\n"
			"\".html\" => \"text/html\",\n"
			"\".htm\"  => \"text/html\",\n"
			"\".pdf\"  => \"application/pdf\",\n"
			"\".swf\"  => \"application/x-shockwave-flash\",\n"
			"\".txt\"  => \"text/plain\",\n"
			"\".tar.gz\" =>   \"application/x-tgz\",\n"
			"\".tgz\"  => \"application/x-tgz\",\n"
			"\".gz\"   => \"application/x-gzip\",\n"
			"\".css\"   => \"text/css\",\n"
			")\n\n"
			"compress.cache-dir = \"/tmp/lighttpd/cache/compress/\"\n"
			"compress.filetype   = (\"text/plain\", \"text/html\")\n"
			"fastcgi.debug       = 0\n"
			"fastcgi.server = (\n"
			"\".php\" =>\n"
			"( \"localhost\" =>\n"
			"\t( \"socket\" => \"/tmp/php-fcgi.sock\",\n"
			"\t\"bin-path\" => \"/usr/bin/php-cgi\",\n"
			"\t\"max-procs\" => 1,\n"
			"\t\"bin-environment\" =>\n"
			"\t\t(\n"
			"\t\t\"PHP_FCGI_CHILDREN\" => \"2\",\n"
			" \t\t\"PHP_FCGI_MAX_REQUESTS\" => \"500\"\n"
			"\t\t)\n"
			"\t)\n"
			")\n"
			")\n\n"
			"url.access-deny = ( \"~\", \".inc\")\n\n"
			"$SERVER[\"socket\"] == \":%s\" {\n"
			"ssl.engine		= \"enable\"\n"
			"ssl.pemfile		= \"/etc/host.pem\"\n"
			"}\n\n"
			"$HTTP[\"url\"] =~ \"^/owncloud/data/\" {\n"
			"url.access-deny = (\"\")\n"
			"}\n\n"
			"$HTTP[\"url\"] =~ \"^/owncloud($|/)\" {\n"
			"dir-listing.activate = \"disable\"\n"
			"}\n\n"
			"$HTTP[\"url\"] =~ \"^/data/\" {\n"
			"url.access-deny = (\"\")\n"
			"}\n\n"
			"auth.backend                   = \"plain\"\n"
			"auth.backend.plain.userfile    = \"/tmp/lighttpd/lighttpd.user\"\n"
			"auth.backend.htpasswd.userfile = \"/tmp/lighttpd/lighttpd.htpasswd\"\n"
			"server.bind           = \"%s\"\n"
			"server.port           = %s\n"
			"server.document-root  = \"%s\"\n",
			nvram_safe_get("lighttpd_sslport"),
			nvram_match("lighttpd_wan", "1") ?
				"0.0.0.0" :
				nvram_safe_get("lan_ipaddr"),
			nvram_safe_get("lighttpd_port"),
			nvram_safe_get("lighttpd_root"));
		fclose(fp);
	}

	log_eval("lighttpd", "-f",
		 getdefaultconfig(NULL, path, sizeof(path), "lighttpd.conf"));
	return;
}

void stop_lighttpd(void)
{
	stop_process("lighttpd", "daemon");
}
#endif
