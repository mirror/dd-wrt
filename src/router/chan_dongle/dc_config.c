/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "dc_config.h"
#include <asterisk/callerid.h>				/* ast_parse_caller_presentation() */

static struct ast_jb_conf jbconf_default = 
{
	.flags			= 0,
	.max_size		= -1,
	.resync_threshold	= -1,
	.impl			= "",
	.target_extra		= -1,
};


static const char * const dtmf_values[] = { "off", "inband", "relax" };

EXPORT_DEF int dc_dtmf_str2setting(const char * value)
{
    return str2enum(value, dtmf_values, ITEMS_OF(dtmf_values));
}

EXPORT_DEF const char * dc_dtmf_setting2str(dc_dtmf_setting_t dtmf)
{
	return enum2str(dtmf, dtmf_values, ITEMS_OF(dtmf_values));
}

#/* assume config is zerofill */
static int dc_uconfig_fill(struct ast_config * cfg, const char * cat, struct dc_uconfig * config)
{
	const char * audio_tty;
	const char * data_tty;
	const char * imei;
	const char * imsi;

	audio_tty = ast_variable_retrieve (cfg, cat, "audio");
	data_tty  = ast_variable_retrieve (cfg, cat, "data");
	imei = ast_variable_retrieve (cfg, cat, "imei");
	imsi = ast_variable_retrieve (cfg, cat, "imsi");

	if(imei && strlen(imei) != IMEI_SIZE) {
		ast_log (LOG_WARNING, "[%s] Ignore invalid IMEI value '%s'\n", cat, imei);
		imei = NULL;
		}
	if(imsi && strlen(imsi) != IMSI_SIZE) {
		ast_log (LOG_WARNING, "[%s] Ignore invalid IMSI value '%s'\n", cat, imsi);
		imsi = NULL;
		}

	if(!audio_tty && !imei && !imsi)
	{
		ast_log (LOG_ERROR, "Skipping device %s. Missing required audio_tty setting\n", cat);
		return 1;
	}

	if(!data_tty && !imei && !imsi)
	{
		ast_log (LOG_ERROR, "Skipping device %s. Missing required data_tty setting\n", cat);
		return 1;
	}

	if((!data_tty && audio_tty) || (data_tty && !audio_tty))
	{
		ast_log (LOG_ERROR, "Skipping device %s. data_tty and audio_tty should use together\n", cat);
		return 1;
	}

	ast_copy_string (config->id,		cat,	             sizeof (config->id));
	ast_copy_string (config->data_tty,	S_OR(data_tty, ""),  sizeof (config->data_tty));
	ast_copy_string (config->audio_tty,	S_OR(audio_tty, ""), sizeof (config->audio_tty));
	ast_copy_string (config->imei,		S_OR(imei, ""),	     sizeof (config->imei));
	ast_copy_string (config->imsi,		S_OR(imsi, ""),	     sizeof (config->imsi));

	return 0;
}

#/* */
EXPORT_DEF void dc_sconfig_fill_defaults(struct dc_sconfig * config)
{
	/* first set default values */
	memset(config, 0, sizeof(*config));

	ast_copy_string (config->context, "default", sizeof (config->context));
	ast_copy_string (config->exten, "", sizeof (config->exten));
	ast_copy_string (config->language, DEFAULT_LANGUAGE, sizeof (config->language));

	config->u2diag			= -1;
	config->resetdongle		=  1;
	config->callingpres		= -1;
	config->initstate		= DEV_STATE_STARTED;
	config->callwaiting 		= CALL_WAITING_AUTO;
	config->dtmf			= DC_DTMF_SETTING_RELAX;

	config->mindtmfgap		= DEFAULT_MINDTMFGAP;
	config->mindtmfduration		= DEFAULT_MINDTMFDURATION;
	config->mindtmfinterval		= DEFAULT_MINDTMFINTERVAL;
}

#/* */
EXPORT_DEF void dc_sconfig_fill(struct ast_config * cfg, const char * cat, struct dc_sconfig * config)
{
	struct ast_variable * v;

	/*  read config and translate to values */
	for (v = ast_variable_browse (cfg, cat); v; v = v->next)
	{
		if (!strcasecmp (v->name, "context"))
		{
			ast_copy_string (config->context, v->value, sizeof (config->context));
		}
		else if (!strcasecmp (v->name, "exten"))
		{
			ast_copy_string (config->exten, v->value, sizeof (config->exten));
		}
		else if (!strcasecmp (v->name, "language"))
		{
			ast_copy_string (config->language, v->value, sizeof (config->language));/* set channel language */
		}
		else if (!strcasecmp (v->name, "group"))
		{
			config->group = (int) strtol (v->value, (char**) NULL, 10);		/* group is set to 0 if invalid */
		}
		else if (!strcasecmp (v->name, "rxgain"))
		{
			config->rxgain = (int) strtol (v->value, (char**) NULL, 10);		/* rxgain is set to 0 if invalid */
		}
		else if (!strcasecmp (v->name, "txgain"))
		{
			config->txgain = (int) strtol (v->value, (char**) NULL, 10);		/* txgain is set to 0 if invalid */
		}
		else if (!strcasecmp (v->name, "u2diag"))
		{
			errno = 0;
			config->u2diag = (int) strtol (v->value, (char**) NULL, 10);		/* u2diag is set to -1 if invalid */
			if (config->u2diag == 0 && errno == EINVAL)
			{
				config->u2diag = -1;
			}
		}
		else if (!strcasecmp (v->name, "callingpres"))
		{
			config->callingpres = ast_parse_caller_presentation (v->value);
			if (config->callingpres == -1)
			{
				errno = 0;
				config->callingpres = (int) strtol (v->value, (char**) NULL, 10);/* callingpres is set to -1 if invalid */
				if (config->callingpres == 0 && errno == EINVAL)
				{
					config->callingpres = -1;
				}
			}
		}
		else if (!strcasecmp (v->name, "usecallingpres"))
		{
			config->usecallingpres = ast_true (v->value);		/* usecallingpres is set to 0 if invalid */
		}
		else if (!strcasecmp (v->name, "autodeletesms"))
		{
			config->autodeletesms = ast_true (v->value);		/* autodeletesms is set to 0 if invalid */
		}
		else if (!strcasecmp (v->name, "resetdongle"))
		{
			config->resetdongle = ast_true (v->value);		/* resetdongle is set to 0 if invalid */
		}
		else if (!strcasecmp (v->name, "disablesms"))
		{
			config->disablesms = ast_true (v->value);		/* disablesms is set to 0 if invalid */
		}
		else if (!strcasecmp (v->name, "smsaspdu"))
		{
			config->smsaspdu = ast_true (v->value);			/* send_sms_as_pdu us set to 0 if invalid */
		}
		else if (!strcasecmp (v->name, "disable"))
		{
			config->initstate = ast_true (v->value) ? DEV_STATE_REMOVED : DEV_STATE_STARTED;
		}
		else if (!strcasecmp (v->name, "initstate"))
		{
			int val = str2enum(v->value, dev_state_strs, ITEMS_OF(dev_state_strs));
			if(val == DEV_STATE_STOPPED || val == DEV_STATE_STARTED || val == DEV_STATE_REMOVED)
				config->initstate = val;
			else
				ast_log(LOG_ERROR, "Invalid value for 'initstate': '%s', must be one of 'stop' 'start' 'remove' default is 'start'\n", v->value);
		}
		else if (!strcasecmp (v->name, "callwaiting"))
		{
			if(strcasecmp(v->value, "auto"))
				config->callwaiting = ast_true (v->value);
		}
		else if (!strcasecmp (v->name, "dtmf"))
		{
			int val = dc_dtmf_str2setting(v->value);
			if(val >= 0)
				config->dtmf = val;
			else
				ast_log(LOG_ERROR, "Invalid value for 'dtmf': '%s', setting default 'relax'\n", v->value);
		}
		else if (!strcasecmp (v->name, "mindtmfgap"))
		{
			errno = 0;
			config->mindtmfgap = (int) strtol (v->value, (char**) NULL, 10);
			if ((config->mindtmfgap == 0 && errno == EINVAL) || config->mindtmfgap < 0)
			{
				ast_log(LOG_ERROR, "Invalid value for 'mindtmfgap' '%s', setting default %d\n", v->value, DEFAULT_MINDTMFGAP);
				config->mindtmfgap = DEFAULT_MINDTMFGAP;
			}
		}
		else if (!strcasecmp (v->name, "mindtmfduration"))
		{
			errno = 0;
			config->mindtmfduration = (int) strtol (v->value, (char**) NULL, 10);
			if ((config->mindtmfduration == 0 && errno == EINVAL) || config->mindtmfduration < 0)
			{
				ast_log(LOG_ERROR, "Invalid value for 'mindtmfgap' '%s', setting default %d\n", v->value, DEFAULT_MINDTMFDURATION);
				config->mindtmfduration = DEFAULT_MINDTMFDURATION;
			}
		}
		else if (!strcasecmp (v->name, "mindtmfinterval"))
		{
			errno = 0;
			config->mindtmfinterval = (int) strtol (v->value, (char**) NULL, 10);
			if ((config->mindtmfinterval == 0 && errno == EINVAL) || config->mindtmfinterval < 0)
			{
				ast_log(LOG_ERROR, "Invalid value for 'mindtmfinterval' '%s', setting default %d\n", v->value, DEFAULT_MINDTMFINTERVAL);
				config->mindtmfduration = DEFAULT_MINDTMFINTERVAL;
			}
		}
	}
}

#/* */
EXPORT_DEF void dc_gconfig_fill(struct ast_config * cfg, const char * cat, struct dc_gconfig * config)
{
	struct ast_variable * v;
	int tmp;
	const char * stmp;

	/* set default values */
	memcpy(&config->jbconf, &jbconf_default, sizeof(config->jbconf));
	config->discovery_interval = DEFAULT_DISCOVERY_INT;

	stmp = ast_variable_retrieve (cfg, cat, "interval");
	if(stmp)
	{
		errno = 0;
		tmp = (int) strtol (stmp, (char**) NULL, 10);
		if (tmp == 0 && errno == EINVAL)
			ast_log (LOG_NOTICE, "Error parsing 'interval' in general section, using default value %d\n", config->discovery_interval);
		else
			config->discovery_interval = tmp;
	}


	for (v = ast_variable_browse (cfg, cat); v; v = v->next)
		/* handle jb conf */
		ast_jb_read_conf (&config->jbconf, v->name, v->value);
}

#/* */
EXPORT_DEF int dc_config_fill(struct ast_config * cfg, const char * cat, const struct dc_sconfig * parent, struct pvt_config * config)
{
	/* try set unique first */
	int err = dc_uconfig_fill(cfg, cat, &config->unique);
	if(!err)
	{
		/* inherit from parent */
		memcpy(&config->shared, parent, sizeof(config->shared));

		/* overwrite local */
		dc_sconfig_fill(cfg, cat, &config->shared);
	}

	return err;
}
