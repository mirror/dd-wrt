/* Plugin for asuscomm.com DDNS
 *
 * Copyright (C) 2012 Vladislav Grishenko <themiron@mail.ru>
 * Copyright (C) 2014 Andy Padavan <andy.padavan@gmail.com>
 * Copyright (C) 2018 Eric Sauvageau <rmerl@lostrealm.ca>
 * Based on Asus's ez-ipupdate implementation
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
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */
#define USE_IPV6

#include "md5.h"
#include "base64.h"
#include "plugin.h"

#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>

#ifdef ASUSWRT
#include <ddnvram.h>
#endif

#define ASUSDDNS_IP_SERVER "ns1.asuscomm.com"
#define ASUSDDNS_IP_SERVER_CN "ns1.asuscomm.cn"
//#define ASUSDDNS_IP_SERVER	"52.250.15.7"
#define ASUSDDNS_CHECKIP_URL "/myip.php"

#if defined(ASUSWRT) && defined(ASUSWRT_LE)
#define ASUSDDNS_ARGS "%s%s"
#elif defined(ASUSWRT)
#define ASUSDDNS_ARGS "%s"
#else
#define ASUSDDNS_ARGS ""
#endif

#define ASUSDDNS_IP_HTTP_REQUEST \
	"GET %s?"                \
	"hostname=%s&" ASUSDDNS_ARGS

#define ASUSDDNS_IP_HTTP_REQUEST_MYIP "myip=%s&"

#ifdef USE_IPV6
#define ASUSDDNS_IP_HTTP_REQUEST_MYIPV6 "myipv6=%s&"
#endif

#define ASUSDDNS_IP_HTTP_REQUEST_2    \
	"model=%s&"                   \
	"fw_ver=%s "                  \
	"HTTP/1.0\r\n"                \
	"Authorization: Basic %s\r\n" \
	"Host: %s\r\n"                \
	"User-Agent: %s\r\n\r\n"

#define ASUSDDNS_UNREG_HTTP_REQUEST   \
	"GET %s?"                     \
	"hostname=%s&"                \
	"action=unregister "          \
	"HTTP/1.0\r\n"                \
	"Authorization: Basic %s\r\n" \
	"Host: %s\r\n"                \
	"User-Agent: %s\r\n\r\n"

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias);
static int request_unregister(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias);
static int response_update(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);
static int response_register(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t asus_update = { .name = "default@asus.com",

				     .setup = NULL,
				     .request = (req_fn_t)request,
				     .response = (rsp_fn_t)response_update,

				     .checkip_name = ASUSDDNS_IP_SERVER,
				     .checkip_url = ASUSDDNS_CHECKIP_URL,

				     .server_name = ASUSDDNS_IP_SERVER,
				     .server_url = "/ddns/update.jsp" };

static ddns_system_t asus_register = { .name = "default@asusregister.com",

				       .setup = NULL,
				       .request = (req_fn_t)request,
				       .response = (rsp_fn_t)response_register,

				       .checkip_name = ASUSDDNS_IP_SERVER,
				       .checkip_url = ASUSDDNS_CHECKIP_URL,

				       .server_name = ASUSDDNS_IP_SERVER,
				       .server_url = "/ddns/register.jsp" };

static ddns_system_t asus_unregister = { .name = "default@asusunregister.com",

					 .setup = NULL,
					 .request = (req_fn_t)request_unregister,
					 .response = (rsp_fn_t)response_register,

					 .checkip_name = ASUSDDNS_IP_SERVER,
					 .checkip_url = ASUSDDNS_CHECKIP_URL,

					 .server_name = ASUSDDNS_IP_SERVER,
					 .server_url = "/ddns/register.jsp" };

static ddns_system_t asus_update_cn = { .name = "default@asus.cn",

					.setup = NULL,
					.request = (req_fn_t)request,
					.response = (rsp_fn_t)response_update,

					.checkip_name = ASUSDDNS_IP_SERVER_CN,
					.checkip_url = ASUSDDNS_CHECKIP_URL,

					.server_name = ASUSDDNS_IP_SERVER_CN,
					.server_url = "/ddns/update.jsp" };

static ddns_system_t asus_register_cn = { .name = "default@asusregister.cn",

					  .setup = NULL,
					  .request = (req_fn_t)request,
					  .response = (rsp_fn_t)response_register,

					  .checkip_name = ASUSDDNS_IP_SERVER_CN,
					  .checkip_url = ASUSDDNS_CHECKIP_URL,

					  .server_name = ASUSDDNS_IP_SERVER_CN,
					  .server_url = "/ddns/register.jsp" };

static ddns_system_t asus_unregister_cn = { .name = "default@asusunregister.cn",

					    .setup = NULL,
					    .request = (req_fn_t)request_unregister,
					    .response = (rsp_fn_t)response_register,

					    .checkip_name = ASUSDDNS_IP_SERVER_CN,
					    .checkip_url = ASUSDDNS_CHECKIP_URL,

					    .server_name = ASUSDDNS_IP_SERVER_CN,
					    .server_url = "/ddns/register.jsp" };

#define MD5_DIGEST_BYTES 16
static void hmac_md5(const unsigned char *input, size_t ilen, const unsigned char *key, size_t klen,
		     unsigned char output[MD5_DIGEST_BYTES])
{
	int i;
	md5_context ctx;
	unsigned char k_ipad[64], k_opad[64], tk[MD5_DIGEST_BYTES];

	/* if key is longer than 64 bytes reset it to key=MD5(key) */
	if (klen > 64) {
		md5(key, klen, tk);
		key = tk;
		klen = MD5_DIGEST_BYTES;
	}

	/* start out by storing key in pads */
	memset(k_ipad, 0, sizeof(k_ipad));
	memset(k_opad, 0, sizeof(k_opad));
	memcpy(k_ipad, key, klen);
	memcpy(k_opad, key, klen);

	/*xor key with ipad and opad values */
	for (i = 0; i < 64; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/* inner MD5 */
	md5_starts(&ctx);
	md5_update(&ctx, k_ipad, 64);
	md5_update(&ctx, input, ilen);
	md5_finish(&ctx, output);

	/* outter MD5 */
	md5_starts(&ctx);
	md5_update(&ctx, k_opad, 64);
	md5_update(&ctx, output, MD5_DIGEST_BYTES);
	md5_finish(&ctx, output);

	memset(&ctx, 0, sizeof(md5_context));
}


static void make_request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	unsigned char digest[MD5_DIGEST_BYTES];
	char auth[ETH_ALEN * 2 + 1 + MD5_DIGEST_BYTES * 2 + 1];
	char *p_tmp, *p_auth = auth;
	size_t dlen = 0;
	int i;

	/* prepare username (MAC) */
	p_tmp = info->creds.username;
	for (i = 0; i < ETH_ALEN * 2; i++) {
		while (*p_tmp && !isxdigit(*p_tmp))
			p_tmp++;
		*p_auth++ = *p_tmp ? toupper(*p_tmp++) : '0';
	}

	/* split username and password */
	*p_auth++ = ':';

	/* prepare password, reuse request_buf */
	p_tmp = alias->name;
	for (i = 0; i < ctx->request_buflen - 1 && *p_tmp; p_tmp++) {
		if (isalnum(*p_tmp))
			ctx->request_buf[i++] = *p_tmp;
	}
	p_tmp = alias->address;
	for (; i < ctx->request_buflen - 1 && *p_tmp; p_tmp++) {
		if (isalnum(*p_tmp))
			ctx->request_buf[i++] = *p_tmp;
	}
	ctx->request_buf[i] = '\0';
	hmac_md5((unsigned char *)ctx->request_buf, strlen(ctx->request_buf), (unsigned char *)info->creds.password,
		 strlen(info->creds.password), digest);
	for (i = 0; i < MD5_DIGEST_BYTES; i++)
		p_auth += sprintf(p_auth, "%02X", digest[i]);

	/*encode*/
	base64_encode(NULL, &dlen, (unsigned char *)auth, strlen(auth));
	p_tmp = malloc(dlen);
	if (p_tmp)
		base64_encode((unsigned char *)p_tmp, &dlen, (unsigned char *)auth, strlen(auth));

	if (info->creds.encoded_password)
		free(info->creds.encoded_password);

	info->creds.encoded_password = p_tmp;
	info->creds.encoded = p_tmp ? 1 : 0;
	info->creds.size = p_tmp ? strlen(p_tmp) : 0;
}

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	char fwver[32];

	logit(LOG_WARNING, "alias address=<%s>", alias->address);

	make_request(ctx, info, alias);

	snprintf(fwver, sizeof(fwver), "%s.%s_%s", "3.0.0.4", "666", "4");
	snprintf(ctx->request_buf, ctx->request_buflen, ASUSDDNS_IP_HTTP_REQUEST, info->server_url, alias->name);
	if (strstr(info->system->name, "ipv6")) {
		snprintf(ctx->request_buf + strlen(ctx->request_buf), ctx->request_buflen - strlen(ctx->request_buf),
			 ASUSDDNS_IP_HTTP_REQUEST_MYIPV6, alias->address);
	} else {
		snprintf(ctx->request_buf + strlen(ctx->request_buf), ctx->request_buflen - strlen(ctx->request_buf),
			 ASUSDDNS_IP_HTTP_REQUEST_MYIP, alias->address);
	}

	snprintf(ctx->request_buf + strlen(ctx->request_buf), ctx->request_buflen - strlen(ctx->request_buf),
		 info->system->server_req, "RT-AX89X", fwver, info->creds.encoded_password ?: "", info->server_name.name,
		 info->user_agent);
	logit(LOG_WARNING, "request<%s>", ctx->request_buf);
	return strlen(ctx->request_buf);
}

static int request_unregister(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	logit(LOG_WARNING, "do request_unregister");
	make_request(ctx, info, alias);
	snprintf(ctx->request_buf, ctx->request_buflen, ASUSDDNS_UNREG_HTTP_REQUEST, info->server_url, alias->name,
		 info->creds.encoded_password ?: "", info->server_name.name, info->user_agent);
	logit(LOG_WARNING, "unregister_request<%s>", ctx->request_buf);
	return strlen(ctx->request_buf);
}

static int response_update(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *p, *p_rsp;
	char domain[256] = { 0 };
	p_rsp = trans->rsp_body;

	if (trans->rsp)
		logit(LOG_WARNING, "[%s]%s", __FUNCTION__, trans->rsp);

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

	switch (trans->status) {
	case 200: /* update success */
	case 220: /* update same domain success -- unused?? */
		return RC_OK;
	case 203: /* update/reg/unreg failed */
		logit(LOG_WARNING, "Domain already in use, suggested domain '%s'", domain);
		return RC_DDNS_RSP_DOMAIN_IN_USE_REG;
	case 230:
		logit(LOG_WARNING, "New domain update success, old domain '%s'", domain);
		return RC_OK;
	case 233: /* update failed */
		logit(LOG_WARNING, "Domain already in use, current domain '%s'", domain);
		return RC_DDNS_RSP_DOMAIN_IN_USE_UPDATE;
	case 297: /* invalid hostname */
		logit(LOG_WARNING, "Invalid hostname");
		return RC_DDNS_INVALID_HOSTNAME;
	case 298: /* invalid domain name */
		logit(LOG_WARNING, "Invalid domain name");
		return RC_DDNS_INVALID_DOMAIN_NAME;
	case 299: /* invalid ip format */
		logit(LOG_WARNING, "Invalid IP address");
		return RC_DDNS_INVALID_IP;
	case 401: /* authentication failure */
		logit(LOG_WARNING, "Authentication failure");
		return RC_DDNS_RSP_AUTH_FAIL;
	case 402:
		logit(LOG_WARNING, "Registration blocked");
		return RC_DDNS_RSP_REG_BLOCK;
	case 407: /* proxy authentication required */
		logit(LOG_WARNING, "Proxy authenticatio blocked");
		return RC_DDNS_RSP_PROXY_AUTH_REQ;
	}

	if (trans->status >= 500 && trans->status < 600)
		return RC_DDNS_RSP_RETRY_LATER;

	return RC_DDNS_RSP_NOTOK;
}

static int response_register(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *p, *p_rsp;
	char domain[256] = { 0 };

	p_rsp = trans->rsp_body;

	if (trans->rsp)
		logit(LOG_WARNING, "[%s]%s", __FUNCTION__, trans->rsp);

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

	switch (trans->status) {
	case 200: /* registration success */
	case 220: /* registration same domain success*/
		return RC_OK;
	case 203: /* registration failed */
		logit(LOG_WARNING, "Domain already in use, suggested domain '%s'", domain);
		return RC_DDNS_RSP_DOMAIN_IN_USE_REG;
	case 230: /* registration new domain success */
		logit(LOG_WARNING, "Registration success, previous domain '%s'", domain);
		return RC_OK;
	case 233: /* registration failed */
		logit(LOG_WARNING, "Domain already in use, current domain '%s'", domain);
		return RC_DDNS_RSP_DOMAIN_IN_USE_UPDATE;
	case 297: /* invalid hostname */
		logit(LOG_WARNING, "Invalid hostname");
		return RC_DDNS_INVALID_HOSTNAME;
	case 298: /* invalid domain name */
		logit(LOG_WARNING, "Invalid domain name");
		return RC_DDNS_INVALID_DOMAIN_NAME;
	case 299: /* invalid ip format */
		logit(LOG_WARNING, "Invalid IP address");
		return RC_DDNS_INVALID_IP;
	case 401: /* authentication failure */
		logit(LOG_WARNING, "Authentication failure");
		return RC_DDNS_RSP_AUTH_FAIL;
	case 402:
		logit(LOG_WARNING, "Registration blocked");
		return RC_DDNS_RSP_REG_BLOCK;
	case 407: /* proxy authentication required */
		logit(LOG_WARNING, "Proxy authenticatio blocked");
		return RC_DDNS_RSP_PROXY_AUTH_REQ;
	}

	return RC_DDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&asus_update, ASUSDDNS_IP_HTTP_REQUEST_2);
	plugin_register(&asus_register, ASUSDDNS_IP_HTTP_REQUEST_2);
	plugin_register(&asus_unregister, ASUSDDNS_UNREG_HTTP_REQUEST);
	plugin_register(&asus_update_cn, ASUSDDNS_IP_HTTP_REQUEST_2);
	plugin_register(&asus_register_cn, ASUSDDNS_IP_HTTP_REQUEST_2);
	plugin_register(&asus_unregister_cn, ASUSDDNS_UNREG_HTTP_REQUEST);

	plugin_register_v6(&asus_update, ASUSDDNS_IP_HTTP_REQUEST_2);
	plugin_register_v6(&asus_register, ASUSDDNS_IP_HTTP_REQUEST_2);
	plugin_register_v6(&asus_unregister, ASUSDDNS_UNREG_HTTP_REQUEST);
	plugin_register_v6(&asus_update_cn, ASUSDDNS_IP_HTTP_REQUEST_2);
	plugin_register_v6(&asus_register_cn, ASUSDDNS_IP_HTTP_REQUEST_2);
	plugin_register_v6(&asus_unregister_cn, ASUSDDNS_UNREG_HTTP_REQUEST);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&asus_update);
	plugin_unregister(&asus_register);
	plugin_unregister(&asus_unregister);
	plugin_unregister(&asus_update_cn);
	plugin_unregister(&asus_register_cn);
	plugin_unregister(&asus_unregister_cn);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
