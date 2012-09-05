/* https.c
 * HTTPS protocol client implementation
 * (c) 2002 Mikulas Patocka
 * This file is a part of the Links program, released under GPL.

 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

#include "links.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

#ifdef HAVE_SSL

SSL_CTX *context = NULL;

SSL *getSSL(void)
{
	if (!context) {
		char f_randfile[PATH_MAX];

		const char *f = RAND_file_name(f_randfile, sizeof(f_randfile));
		if (f && RAND_egd(f)<0) {
			/* Not an EGD, so read and write to it */
			if (RAND_load_file(f_randfile, -1))
				RAND_write_file(f_randfile);
		}
		SSLeay_add_ssl_algorithms();
		context = SSL_CTX_new(SSLv23_client_method());
		SSL_CTX_set_options(context, SSL_OP_ALL);
		SSL_CTX_set_default_verify_paths(context);
/* needed for systems without /dev/random, but obviously kills security. */
		/*{
			char pool[32768];
			int i;
			struct timeval tv;
			gettimeofday(&tv, NULL);
			for (i = 0; i < sizeof pool; i++) pool[i] = random() ^ tv.tv_sec ^ tv.tv_usec;
			RAND_add(pool, sizeof pool, sizeof pool);
		}*/
	}
	return (SSL_new(context));
}
void ssl_finish(void)
{
	if (context) SSL_CTX_free(context);
}

void https_func(struct connection *c)
{
	c->ssl = (void *)-1;
	http_func(c);
}

#else

void https_func(struct connection *c)
{
	setcstate(c, S_NO_SSL);
	abort_connection(c);
}

#endif
