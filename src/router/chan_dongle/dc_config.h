/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_DC_CONFIG_H_INCLUDED
#define CHAN_DONGLE_DC_CONFIG_H_INCLUDED

#include <asterisk.h>
#include <asterisk/channel.h>		/* AST_MAX_CONTEXT MAX_LANGUAGE */

#include "export.h"			/* EXPORT_DECL EXPORT_DEF */
#include "mutils.h"

#define CONFIG_FILE		"dongle.conf"
#define DEVNAMELEN		31
#define IMEI_SIZE		15
#define IMSI_SIZE		15
#define DEVPATHLEN		256

typedef enum {
	DEV_STATE_STOPPED	= 0,
	DEV_STATE_RESTARTED,
	DEV_STATE_REMOVED,
	DEV_STATE_STARTED,
} dev_state_t;
EXPORT_DECL const char * const dev_state_strs[4];

typedef enum {
	CALL_WAITING_DISALLOWED = 0,
	CALL_WAITING_ALLOWED,
	CALL_WAITING_AUTO
} call_waiting_t;

INLINE_DECL const char * dc_cw_setting2str(call_waiting_t cw)
{
	static const char * const options[] = { "disabled", "allowed", "auto" };
	return enum2str(cw, options, ITEMS_OF(options));
}

typedef enum {
	DC_DTMF_SETTING_OFF = 0,
	DC_DTMF_SETTING_INBAND,
	DC_DTMF_SETTING_RELAX,
} dc_dtmf_setting_t;

/*
 Config API
 Operations
 	convert from string to native
 	convent from native to string
 	get native value
	get alternative presentation

 	set native value ?

	types:
		string of limited length
		integer with limits
		enum
		boolean
*/

/* Global inherited (shared) settings */
typedef struct dc_sconfig
{
	char			context[AST_MAX_CONTEXT];	/*!< the context for incoming calls; 'default '*/
	char			exten[AST_MAX_EXTENSION];	/*!< exten, not overwrite valid subscriber_number */
	char			language[MAX_LANGUAGE];		/*!< default language 'en' */
	int			group;				/*!< group number for group dialling 0 */
	int			rxgain;				/*!< increase the incoming volume 0 */
	int			txgain;				/*!< increase the outgoint volume 0 */
	int			u2diag;				/*!< -1 */
	int			callingpres;			/*!< calling presentation */

	unsigned int		usecallingpres:1;		/*! -1 */
	unsigned int		autodeletesms:1;		/*! 0 */
	unsigned int		resetdongle:1;			/*! 1 */
	unsigned int		disablesms:1;			/*! 0 */
	unsigned int		smsaspdu:1;			/*! 0 */
	dev_state_t		initstate;			/*! DEV_STATE_STARTED */
//	unsigned int		disable:1;			/*! 0 */

	call_waiting_t		callwaiting;			/*!< enable/disable/auto call waiting CALL_WAITING_AUTO */
	dc_dtmf_setting_t	dtmf;				/*!< off/inband/relax incoming DTMF detection, default DC_DTMF_SETTING_RELAX */

	int			mindtmfgap;			/*!< minimal time in ms from end of previews DTMF and begining of next */
#define DEFAULT_MINDTMFGAP	45

	int			mindtmfduration;		/*!< minimal DTMF duration in ms */
#define DEFAULT_MINDTMFDURATION	80

	int			mindtmfinterval;		/*!< minimal DTMF interval beetween ends in ms, applied only on same digit */
#define DEFAULT_MINDTMFINTERVAL	200
} dc_sconfig_t;

/* Global settings */
typedef struct dc_gconfig
{
	struct ast_jb_conf	jbconf;				/*!< jitter buffer settings, disabled by default */
	int			discovery_interval;		/*!< The device discovery interval */
#define DEFAULT_DISCOVERY_INT	60
} dc_gconfig_t;

/* Local required (unique) settings */
typedef struct dc_uconfig
{
	/* unique settings */
	char			id[DEVNAMELEN];			/*!< id from dongle.conf */
	char			audio_tty[DEVPATHLEN];		/*!< tty for audio connection */
	char			data_tty[DEVPATHLEN];		/*!< tty for AT commands */
	char			imei[IMEI_SIZE+1];		/*!< search device by imei */
	char			imsi[IMSI_SIZE+1];		/*!< search device by imsi */
} dc_uconfig_t;

/* all Config settings join in one place */
typedef struct pvt_config
{
	dc_uconfig_t		unique;				/*!< unique settings */
	dc_sconfig_t		shared;				/*!< possible inherited settings */
} pvt_config_t;
#define SCONFIG(cfg,name)	((cfg)->shared.name)
#define UCONFIG(cfg,name)	((cfg)->unique.name)

EXPORT_DECL int dc_dtmf_str2setting(const char * str);
EXPORT_DECL const char * dc_dtmf_setting2str(dc_dtmf_setting_t dtmf);
EXPORT_DECL void dc_sconfig_fill_defaults(struct dc_sconfig * config);
EXPORT_DECL void dc_sconfig_fill(struct ast_config * cfg, const char * cat, struct dc_sconfig * config);
EXPORT_DECL void dc_gconfig_fill(struct ast_config * cfg, const char * cat, struct dc_gconfig * config);
EXPORT_DECL int dc_config_fill(struct ast_config * cfg, const char * cat, const struct dc_sconfig * parent, struct pvt_config * config);


#endif /* CHAN_DONGLE_DC_CONFIG_H_INCLUDED */
