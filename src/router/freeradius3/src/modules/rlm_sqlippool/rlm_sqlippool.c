/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or (at
 *   your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id: cf8d9d0c64e4bccec19b18e4d4850518f7e576f4 $
 * @file rlm_sqlippool.c
 * @brief Allocates an IP address / prefix from pools stored in SQL.
 *
 * @copyright 2002  Globe.Net Communications Limited
 * @copyright 2006  The FreeRADIUS server project
 * @copyright 2006  Suntel Communications
 */
RCSID("$Id: cf8d9d0c64e4bccec19b18e4d4850518f7e576f4 $")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

#include <ctype.h>

#include <rlm_sql.h>

#define MAX_QUERY_LEN 4096

/*
 *	Define a structure for our module configuration.
 */
typedef struct rlm_sqlippool_t {
	char const	*sql_instance_name;

	uint32_t	lease_duration;

	rlm_sql_t	*sql_inst;

	char const	*pool_name;		//!< Name of the attribute in the check VPS for which the value will be used as key
	bool		ipv6;			//!< Whether or not we do IPv6 pools.
	bool		allow_duplicates;	//!< assign even if it already exists
	char const	*attribute_name;	//!< name of the IP address attribute
	char const	*req_attribute_name;	//!< name of the requested IP address attribute

	DICT_ATTR const *framed_ip_address; 	//!< the attribute for IP address allocation
	DICT_ATTR const *req_framed_ip_address;	//!< the attribute for requested IP address
	DICT_ATTR const *pool_attribute; 	//!< the attribute corresponding to the pool_name

	time_t		last_clear;		//!< So we only do it once a second.
	char const	*allocate_begin;	//!< SQL query to begin.
	char const	*allocate_clear;	//!< SQL query to clear an IP.
	uint32_t	allocate_clear_timeout; //!< Number of second between two allocate_clear SQL query
	char const	*allocate_existing;	//!< SQL query to find existing IP leased to the device.
	char const	*allocate_requested;	//!< SQL query to find requested IP.
	char const	*allocate_find;		//!< SQL query to find an unused IP.
	char const	*allocate_update;	//!< SQL query to mark an IP as used.
	char const	*allocate_commit;	//!< SQL query to commit.

	char const	*pool_check;		//!< Query to check for the existence of the pool.

	char const	*start_update;		//!< SQL query run on Accounting Start

	char const	*alive_update;		//!< SQL query run on Accounting Interim-Update

	char const	*stop_clear;		//!< SQL query run on Accounting Stop

	char const	*on_clear;		//!< SQL query run on Accounting On

	char const	*off_clear;		//!< SQL query run on Accounting Off

						/* Logging Section */
	char const	*log_exists;		//!< There was an ip address already assigned.
	char const	*log_success;		//!< We successfully allocated ip address from pool.
	char const	*log_clear;		//!< We successfully deallocated ip address from pool.
	char const	*log_failed;		//!< Failed to allocate ip from the pool.
	char const	*log_nopool;		//!< There was no Framed-IP-Address but also no Pool-Name.
} rlm_sqlippool_t;

static CONF_PARSER message_config[] = {
	{ "exists", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, log_exists), NULL },
	{ "success", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, log_success), NULL },
	{ "clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, log_clear), NULL },
	{ "failed", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, log_failed), NULL },
	{ "nopool", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, log_nopool), NULL },
	CONF_PARSER_TERMINATOR
};

/*
 *	A mapping of configuration file names to internal variables.
 *
 *	Note that the string is dynamically allocated, so it MUST
 *	be freed.  When the configuration file parse re-reads the string,
 *	it free's the old one, and strdup's the new one, placing the pointer
 *	to the strdup'd string into 'config.string'.  This gets around
 *	buffer over-flows.
 */
static CONF_PARSER module_config[] = {
	{ "sql-instance-name", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_DEPRECATED, rlm_sqlippool_t, sql_instance_name), NULL },
	{ "sql_module_instance", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_REQUIRED, rlm_sqlippool_t, sql_instance_name), "sql" },

	{ "lease-duration", FR_CONF_OFFSET(PW_TYPE_INTEGER | PW_TYPE_DEPRECATED, rlm_sqlippool_t, lease_duration), NULL },
	{ "lease_duration", FR_CONF_OFFSET(PW_TYPE_INTEGER, rlm_sqlippool_t, lease_duration), "86400" },

	{ "pool-name", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_DEPRECATED, rlm_sqlippool_t, pool_name), NULL },
	{ "pool_name", FR_CONF_OFFSET(PW_TYPE_STRING, rlm_sqlippool_t, pool_name), "Pool-Name" },

	{ "ipv6", FR_CONF_OFFSET(PW_TYPE_BOOLEAN, rlm_sqlippool_t, ipv6), NULL},
	{ "allow_duplicates", FR_CONF_OFFSET(PW_TYPE_BOOLEAN, rlm_sqlippool_t, allow_duplicates), NULL},
	{ "attribute_name", FR_CONF_OFFSET(PW_TYPE_STRING, rlm_sqlippool_t, attribute_name), NULL},
	{ "req_attribute_name", FR_CONF_OFFSET(PW_TYPE_STRING, rlm_sqlippool_t, req_attribute_name), NULL},

	{ "allocate-begin", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, allocate_begin), NULL },
	{ "allocate_begin", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, allocate_begin), "START TRANSACTION" },

	{ "allocate-clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, allocate_clear), NULL },
	{ "allocate_clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT , rlm_sqlippool_t, allocate_clear), ""  },

	{ "allocate_clear_timeout", FR_CONF_OFFSET(PW_TYPE_INTEGER, rlm_sqlippool_t, allocate_clear_timeout), "1" },

	{ "allocate-existing", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, allocate_existing), NULL },
	{ "allocate_existing", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, allocate_existing), ""  },

	{ "allocate-requested", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, allocate_requested), NULL },
	{ "allocate_requested", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, allocate_requested), ""  },

	{ "allocate-find", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, allocate_find), NULL },
	{ "allocate_find", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_REQUIRED, rlm_sqlippool_t, allocate_find), ""  },

	{ "allocate-update", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, allocate_update), NULL },
	{ "allocate_update", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT , rlm_sqlippool_t, allocate_update), ""  },

	{ "allocate-commit", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, allocate_commit), NULL },
	{ "allocate_commit", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, allocate_commit), "COMMIT" },


	{ "pool-check", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, pool_check), NULL },
	{ "pool_check", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT, rlm_sqlippool_t, pool_check), ""  },


	{ "start-update", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, start_update), NULL },
	{ "start_update", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT , rlm_sqlippool_t, start_update), ""  },

	{ "alive-update", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, alive_update), NULL },
	{ "alive_update", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT , rlm_sqlippool_t, alive_update), ""  },

	{ "stop-clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, stop_clear), NULL },
	{ "stop_clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT , rlm_sqlippool_t, stop_clear), ""  },

	{ "on-clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, on_clear), NULL },
	{ "on_clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT , rlm_sqlippool_t, on_clear), ""  },

	{ "off-clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT | PW_TYPE_DEPRECATED, rlm_sqlippool_t, off_clear), NULL },
	{ "off_clear", FR_CONF_OFFSET(PW_TYPE_STRING | PW_TYPE_XLAT , rlm_sqlippool_t, off_clear), ""  },

	{ "messages", FR_CONF_POINTER(PW_TYPE_SUBSECTION, NULL), (void const *) message_config },
	CONF_PARSER_TERMINATOR
};

/*
 *	Replace %<whatever> in a string.
 *
 *	%P	pool_name
 *	%I	param
 *	%J	lease_duration
 *
 */
static int sqlippool_expand(char * out, int outlen, char const * fmt,
			    rlm_sqlippool_t *data, char * param, int param_len)
{
	char *q;
	char const *p;
	char tmp[40]; /* For temporary storing of integers */

	q = out;
	for (p = fmt; *p ; p++) {
		int freespace;
		int c;

		/* Calculate freespace in output */
		freespace = outlen - (q - out);
		if (freespace <= 1)
			break;

		c = *p;
		if (c != '%') {
			*q++ = *p;
			continue;
		}

		if (*++p == '\0') {
			break;
		}

		if (c == '%') {
			switch (*p) {
			case 'P': /* pool name */
				strlcpy(q, data->pool_name, freespace);
				q += strlen(q);
				break;
			case 'I': /* IP address */
				if (param && param_len > 0) {
					if (param_len > freespace) {
						strlcpy(q, param, freespace);
						q += strlen(q);
					}
					else {
						memcpy(q, param, param_len);
						q += param_len;
					}
				}
				break;
			case 'J': /* lease duration */
				sprintf(tmp, "%d", data->lease_duration);
				strlcpy(q, tmp, freespace);
				q += strlen(q);
				break;

			default:
				*q++ = '%';
				*q++ = *p;
				break;
			}
		}
	}
	*q = '\0';

#if 0
	DEBUG2("sqlippool_expand: \"%s\"", out);
#endif

	return strlen(out);
}

/** Perform a single sqlippool query
 *
 * Mostly wrapper around sql_query which does some special sqlippool sequence substitutions and expands
 * the format string.
 *
 * @param fmt sql query to expand.
 * @param handle sql connection handle.
 * @param data Instance of rlm_sqlippool.
 * @param request Current request.
 * @param param ip address string.
 * @param param_len ip address string len.
 * @return 0 on success or < 0 on error.
 */
static int sqlippool_command(char const *fmt, rlm_sql_handle_t **handle,
			     rlm_sqlippool_t *data, REQUEST *request,
			     char *param, int param_len)
{
	char query[MAX_QUERY_LEN];
	char *expanded = NULL;

	int ret;
	int affected;

	/*
	 *	If we don't have a command, do nothing.
	 */
	if (!fmt || !*fmt) return 0;

	/*
	 *	No handle?  That's an error.
	 */
	if (!handle || !*handle) return -1;

	/*
	 *	@todo this needs to die (should just be done in xlat expansion)
	 */
	sqlippool_expand(query, sizeof(query), fmt, data, param, param_len);

	if (radius_axlat(&expanded, request, query, data->sql_inst->sql_escape_func, *handle) < 0) return -1;

	ret = data->sql_inst->sql_query(data->sql_inst, request, handle, expanded);
	if (ret < 0){
		talloc_free(expanded);
		return -1;
	}
	talloc_free(expanded);

	/*
	 *	No handle, we can't continue.
	 */
	if (!*handle) return -1;

	affected = (data->sql_inst->module->sql_affected_rows)(*handle, data->sql_inst->config);

	if (*handle) (data->sql_inst->module->sql_finish_query)(*handle, data->sql_inst->config);

	return affected;
}

/*
 *	Don't repeat yourself
 */
#undef DO
#define DO(_x) if (sqlippool_command(inst->_x, handle, inst, request, NULL, 0) < 0) return RLM_MODULE_FAIL
#define DO_AFFECTED(_x, _affected) _affected = sqlippool_command(inst->_x, handle, inst, request, NULL, 0); if (_affected < 0) return RLM_MODULE_FAIL
#define DO_PART(_x) if (sqlippool_command(inst->_x, &handle, inst, request, NULL, 0) < 0) goto error

/*
 * Query the database expecting a single result row
 */
static int CC_HINT(nonnull (1, 3, 4, 5)) sqlippool_query1(char *out, int outlen, char const *fmt,
							  rlm_sql_handle_t **handle, rlm_sqlippool_t *data,
							  REQUEST *request, char *param, int param_len)
{
	char query[MAX_QUERY_LEN];
	char *expanded = NULL;

	int rlen, retval;

	/*
	 *	@todo this needs to die (should just be done in xlat expansion)
	 */
	sqlippool_expand(query, sizeof(query), fmt, data, param, param_len);

	*out = '\0';

	/*
	 *	Do an xlat on the provided string
	 *
	 *	Note that on an escaping error the handle is still valid!
	 */
	if (radius_axlat(&expanded, request, query, data->sql_inst->sql_escape_func, *handle) < 0) {
		return 0;
	}

	retval = data->sql_inst->sql_select_query(data->sql_inst, request, handle, expanded);
	talloc_free(expanded);

	if ((retval != 0) || !*handle) {
		REDEBUG("database query error on '%s'", query);
		return 0;
	}

	if (data->sql_inst->sql_fetch_row(data->sql_inst, request, handle) < 0) {
		REDEBUG("Failed fetching query result");
		goto finish;
	}

	if (!(*handle)->row) {
		RDEBUG2("SQL query did not return any results");
		goto finish;
	}

	if (!(*handle)->row[0]) {
		REDEBUG("The first column of the result was NULL");
		goto finish;
	}

	rlen = strlen((*handle)->row[0]);
	if (rlen >= outlen) {
		REDEBUG("The first column of the result was too long (%d)", rlen);
		goto finish;
	}

	strcpy(out, (*handle)->row[0]);
	retval = rlen;
finish:
	(data->sql_inst->module->sql_finish_select_query)(*handle, data->sql_inst->config);

	return retval;
}

/*
 *	Do any per-module initialization that is separate to each
 *	configured instance of the module.  e.g. set up connections
 *	to external databases, read configuration files, set up
 *	dictionary entries, etc.
 *
 *	If configuration information is given in the config section
 *	that must be referenced in later calls, store a handle to it
 *	in *instance otherwise put a null pointer there.
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	module_instance_t *sql_inst;
	rlm_sqlippool_t *inst = instance;

	sql_inst = module_instantiate(cf_section_find("modules"),
					inst->sql_instance_name);
	if (!sql_inst) {
		cf_log_err_cs(conf, "failed to find sql instance named %s",
			   inst->sql_instance_name);
		return -1;
	}

	if (inst->pool_name) {
		DICT_ATTR const *da;

		da = dict_attrbyname(inst->pool_name);
		if (!da) {
			cf_log_err_cs(conf, "Unknown attribute 'pool_name = %s'", inst->pool_name);
			return -1;
		}

		if (da->type != PW_TYPE_STRING) {
			cf_log_err_cs(conf, "Cannot use non-string attributes for 'pool_name = %s'", inst->pool_name);
			return -1;
		}

		inst->pool_attribute = da;
	}

	if (inst->attribute_name) {
		DICT_ATTR const *da;

		da = dict_attrbyname(inst->attribute_name);
		if (!da) {
		fail:
			cf_log_err_cs(conf, "Unknown attribute 'attribute_name = %s'", inst->attribute_name);
			return -1;
		}

		switch (da->type) {
		default:
			cf_log_err_cs(conf, "Cannot use non-IP attributes for 'attribute_name = %s'", inst->attribute_name);
			return -1;

		case PW_TYPE_IPV4_ADDR:
		case PW_TYPE_IPV6_ADDR:
		case PW_TYPE_IPV4_PREFIX:
		case PW_TYPE_IPV6_PREFIX:
			break;

		}

		inst->framed_ip_address = da;
	} else {
		if (!inst->ipv6) {
			inst->attribute_name = "Framed-IP-Address";
			inst->framed_ip_address = dict_attrbyvalue(PW_FRAMED_IP_ADDRESS, 0);
		} else {
			inst->attribute_name = "Framed-IPv6-Prefix";
			inst->framed_ip_address = dict_attrbyvalue(PW_FRAMED_IPV6_PREFIX, 0);
		}

		if (!inst->framed_ip_address) goto fail;
	}

	if (inst->req_attribute_name) {
		DICT_ATTR const *da;

		da = dict_attrbyname(inst->req_attribute_name);
		if (!da) {
			cf_log_err_cs(conf, "Unknown attribute 'req_attribute_name = %s'", inst->req_attribute_name);
			return -1;
		}

		switch (da->type) {
		default:
			cf_log_err_cs(conf, "Cannot use non-IP attributes for 'req_attribute_name = %s'", inst->req_attribute_name);
			return -1;

		case PW_TYPE_IPV4_ADDR:
		case PW_TYPE_IPV6_ADDR:
		case PW_TYPE_IPV4_PREFIX:
		case PW_TYPE_IPV6_PREFIX:
			break;

		}

		inst->req_framed_ip_address = da;
	}

	if (strcmp(sql_inst->entry->name, "rlm_sql") != 0) {
		cf_log_err_cs(conf, "Module \"%s\""
		       " is not an instance of the rlm_sql module",
		       inst->sql_instance_name);
		return -1;
	}

	if (inst->allocate_clear) {
		FR_INTEGER_BOUND_CHECK("allocate_clear_timeout", inst->allocate_clear_timeout, >=, 1);
		FR_INTEGER_BOUND_CHECK("allocate_clear_timeout", inst->allocate_clear_timeout, <=, 2*86400);
	}

	inst->sql_inst = (rlm_sql_t *) sql_inst->insthandle;
	return 0;
}


/*
 *	If we have something to log, then we log it.
 *	Otherwise we return the retcode as soon as possible
 */
static int do_logging(REQUEST *request, char const *str, int rcode)
{
	char *expanded = NULL;

	if (!str || !*str) return rcode;

	if (radius_axlat(&expanded, request, str, NULL, NULL) < 0) {
		return rcode;
	}

	pair_make_config("Module-Success-Message", expanded, T_OP_SET);

	talloc_free(expanded);

	return rcode;
}


/*
 *	Allocate an IP number from the pool.
 */
static rlm_rcode_t CC_HINT(nonnull) mod_post_auth(void *instance, REQUEST *request)
{
	rlm_sqlippool_t *inst = (rlm_sqlippool_t *) instance;
	char allocation[MAX_STRING_LEN];
	int allocation_len;
	VALUE_PAIR *vp = NULL;
	rlm_sql_handle_t *handle;
	time_t now;
	uint32_t diff_time;

	/*
	 *	If there is already an attribute in the reply do nothing
	 */
	if (!inst->allow_duplicates && (fr_pair_find_by_num(request->reply->vps, inst->framed_ip_address->attr, inst->framed_ip_address->vendor, TAG_ANY) != NULL)) {
		RDEBUG("%s already exists", inst->attribute_name);

		return do_logging(request, inst->log_exists, RLM_MODULE_NOOP);
	}

	if (fr_pair_find_by_num(request->config, inst->pool_attribute->attr, inst->pool_attribute->vendor, TAG_ANY) == NULL) {
		RDEBUG("No %s defined", inst->pool_name);

		return do_logging(request, inst->log_nopool, RLM_MODULE_NOOP);
	}

	handle = fr_connection_get(inst->sql_inst->pool);
	if (!handle) {
		REDEBUG("Failed reserving SQL connection");
		return RLM_MODULE_FAIL;
	}

	if (inst->sql_inst->sql_set_user(inst->sql_inst, request, NULL) < 0) {
		return RLM_MODULE_FAIL;
	}

	/*
	 *	Limit the number of clears we do.  There are minor
	 *	race conditions for the check, but so what.  The
	 *	actual work is protected by a transaction.  The idea
	 *	here is that if we're allocating 100 IPs a second,
	 *	we're only do 1 CLEAR per allocate_clear_timeout.
	 *
	 *	This will avoid having several queries to deadlock and blocking all
	 *	the sqlippool module.
	 */
	now = time(NULL);
	diff_time = difftime(now, inst->last_clear);
	if (inst->allocate_clear && *inst->allocate_clear && (diff_time >= inst->allocate_clear_timeout)) {
		inst->last_clear = now;

		DO_PART(allocate_begin);
		DO_PART(allocate_clear);
		DO_PART(allocate_commit);
	}

	DO_PART(allocate_begin);

	/*
	 *	If we have a query to find an existing IP run that first
	 */
	if (inst->allocate_existing && *inst->allocate_existing) {
		allocation_len = sqlippool_query1(allocation, sizeof(allocation),
						  inst->allocate_existing, &handle,
						  inst, request, (char *) NULL, 0);
		if (!handle) return RLM_MODULE_FAIL;
	} else {
		allocation_len = 0;
	}

	/*
	 *	If we have a requested IP address and a query to find whether
	 *	it is available then run that next
	 */
	if (allocation_len == 0 && inst->allocate_requested && *inst->allocate_requested &&
	    fr_pair_find_by_num(request->packet->vps,
				inst->req_framed_ip_address->attr,
				inst->req_framed_ip_address->vendor,
				TAG_ANY) != NULL) {
		allocation_len = sqlippool_query1(allocation, sizeof(allocation),
						  inst->allocate_requested, &handle,
						  inst, request, (char *) NULL, 0);
		if (!handle) return RLM_MODULE_FAIL;
	}

	/*
	 *	If no IP found, look for a free one
	 */
	if (allocation_len == 0) {
		allocation_len = sqlippool_query1(allocation, sizeof(allocation),
						  inst->allocate_find, &handle,
						  inst, request, (char *) NULL, 0);
		if (!handle) return RLM_MODULE_FAIL;
	}

	/*
	 *	Nothing found...
	 */
	if (allocation_len == 0) {
		DO_PART(allocate_commit);

		/*
		 *Should we perform pool-check ?
		 */
		if (inst->pool_check && *inst->pool_check) {

			/*
			 *Ok, so the allocate-find query found nothing ...
			 *Let's check if the pool exists at all
			 */
			allocation_len = sqlippool_query1(allocation, sizeof(allocation),
							  inst->pool_check, &handle, inst, request,
							  (char *) NULL, 0);
			if (!handle) return RLM_MODULE_FAIL;

			fr_connection_release(inst->sql_inst->pool, handle);

			if (allocation_len) {

				/*
				 *	Pool exists after all... So,
				 *	the failure to allocate the IP
				 *	address was most likely due to
				 *	the depletion of the pool. In
				 *	that case, we should return
				 *	NOTFOUND
				 */
				REDEBUG("pool appears to be full");
				return do_logging(request, inst->log_failed, RLM_MODULE_NOTFOUND);

			}

			/*
			 *	Pool doesn't exist in the table. It
			 *	may be handled by some other instance of
			 *	sqlippool, so we should just ignore this
			 *	allocation failure and return NOOP
			 */
			REDEBUG("IP address could not be allocated as no pool exists with that name");
			return RLM_MODULE_NOOP;

		}

		fr_connection_release(inst->sql_inst->pool, handle);

		REDEBUG("IP address could not be allocated");
		return do_logging(request, inst->log_failed, RLM_MODULE_NOOP);
	}

	/*
	 *	See if we can create the VP from the returned data.  If not,
	 *	error out.  If so, add it to the list.
	 */
	vp = fr_pair_afrom_num(request->reply, inst->framed_ip_address->attr, inst->framed_ip_address->vendor);
	if (fr_pair_value_from_str(vp, allocation, allocation_len) < 0) {
		DO_PART(allocate_commit);

		talloc_free(vp);
		REDEBUG("Invalid IP address [%s] returned from database query.", allocation);
		fr_connection_release(inst->sql_inst->pool, handle);
		return do_logging(request, inst->log_failed, RLM_MODULE_NOOP);
	}

	/*
	 *	UPDATE
	 */
	if (sqlippool_command(inst->allocate_update, &handle, inst, request,
			      allocation, allocation_len) < 0) {
	error:
		talloc_free(vp);
		if (handle) fr_connection_release(inst->sql_inst->pool, handle);
		return RLM_MODULE_FAIL;
	}

	DO_PART(allocate_commit);

	RDEBUG("Allocated IP %s", allocation);
	fr_pair_add(&request->reply->vps, vp);

	if (handle) fr_connection_release(inst->sql_inst->pool, handle);

	return do_logging(request, inst->log_success, RLM_MODULE_OK);
}

static int mod_accounting_start(rlm_sql_handle_t **handle,
				rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(start_update);

	return RLM_MODULE_OK;
}

static int mod_accounting_alive(rlm_sql_handle_t **handle,
				rlm_sqlippool_t *inst, REQUEST *request)
{
	int affected;

	DO_AFFECTED(alive_update, affected);

	return (affected == 0 ? RLM_MODULE_NOTFOUND : RLM_MODULE_OK);
}

static int mod_accounting_stop(rlm_sql_handle_t **handle,
			       rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(stop_clear);

	return do_logging(request, inst->log_clear, RLM_MODULE_OK);
}

static int mod_accounting_on(rlm_sql_handle_t **handle,
			     rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(on_clear);

	return RLM_MODULE_OK;
}

static int mod_accounting_off(rlm_sql_handle_t **handle,
			      rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(off_clear);

	return RLM_MODULE_OK;
}

/*
 *	Check for an Accounting-Stop
 *	If we find one and we have allocated an IP to this nas/port
 *	combination, then deallocate it.
 */
static rlm_rcode_t CC_HINT(nonnull) mod_accounting(void *instance, REQUEST *request)
{
	int			rcode = RLM_MODULE_NOOP;
	VALUE_PAIR		*vp;

	int			acct_status_type;

	rlm_sqlippool_t		*inst = (rlm_sqlippool_t *) instance;
	rlm_sql_handle_t	*handle;

	vp = fr_pair_find_by_num(request->packet->vps, PW_ACCT_STATUS_TYPE, 0, TAG_ANY);
	if (!vp) {
		RDEBUG("Could not find account status type in packet");
		return RLM_MODULE_NOOP;
	}
	acct_status_type = vp->vp_integer;

	switch (acct_status_type) {
	case PW_STATUS_START:
	case PW_STATUS_ALIVE:
	case PW_STATUS_STOP:
	case PW_STATUS_ACCOUNTING_ON:
	case PW_STATUS_ACCOUNTING_OFF:
		break;		/* continue through to the next section */

	default:
		/* We don't care about any other accounting packet */
		return RLM_MODULE_NOOP;
	}

	handle = fr_connection_get(inst->sql_inst->pool);
	if (!handle) {
		RDEBUG("Failed reserving SQL connection");
		return RLM_MODULE_FAIL;
	}

	if (inst->sql_inst->sql_set_user(inst->sql_inst, request, NULL) < 0) return RLM_MODULE_FAIL;

	switch (acct_status_type) {
	case PW_STATUS_START:
		rcode = mod_accounting_start(&handle, inst, request);
		break;

	case PW_STATUS_ALIVE:
		rcode = mod_accounting_alive(&handle, inst, request);
		break;

	case PW_STATUS_STOP:
		rcode = mod_accounting_stop(&handle, inst, request);
		break;

	case PW_STATUS_ACCOUNTING_ON:
		rcode = mod_accounting_on(&handle, inst, request);
		break;

	case PW_STATUS_ACCOUNTING_OFF:
		rcode = mod_accounting_off(&handle, inst, request);
		break;
	}

	if (handle) fr_connection_release(inst->sql_inst->pool, handle);

	return rcode;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
extern module_t rlm_sqlippool;
module_t rlm_sqlippool = {
	.magic		= RLM_MODULE_INIT,
	.name		= "sqlippool",
	.type		= RLM_TYPE_THREAD_SAFE,
	.inst_size	= sizeof(rlm_sqlippool_t),
	.config		= module_config,
	.instantiate	= mod_instantiate,
	.methods = {
		[MOD_ACCOUNTING]	= mod_accounting,
		[MOD_POST_AUTH]		= mod_post_auth
	},
};
