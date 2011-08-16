/*
** Copyright (C) 2006 Joshua Wright <jwright@arubanetworks.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* $Id$ */

/* spo_alert_arubaaction
 * 
 * Purpose: output plugin for dynamically changing station access status on
 *          an Aruba switch.
 *
 * Arguments:  switch secret_type secret action
 * 	switch		IP address of the Aruba switch
 * 	secret_type	How secret is represented, one of "sha1", "md5" or
 * 			"cleartext"
 *	secret		The shared secret configured on the Aruba switch
 *	action		The action the switch should take with the target user
 *   
 * Effect:
 *
 * When an alert is passed to this output plugin, the plugin connects to the
 * specified switch using the secret for authentication and applies the
 * configured action for the source IP address of the alert.  This allows the
 * administrator to establish rules that will dynamically blacklist a user,
 * allowing the administrator to define rules that take action based on the
 * power of the Snort rules language.
 */

/* output plugin header file */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "event.h"
#include "decode.h"
#include "debug.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "util.h"
#include "log.h"
#include "mstring.h"

#include "snort.h"

#include "ipv6_port.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* !WIN32 */

#include <sys/types.h>

typedef struct _SpoAlertArubaActionData
{
	char		*secret;
	uint8_t		secret_type;
	uint8_t		action_type;
	char		*role_name;
#ifdef SUP_IP6
	sfip_t         aswitch;
#else
	struct in_addr aswitch;
#endif
	int		fd;
} SpoAlertArubaActionData;


#define MAX_XML_PAYLOAD_LEN 512
#define MAX_POST_LEN 1024
#define MAX_RESPONSE_LEN MAX_POST_LEN

typedef struct _ArubaSecretType {
	uint8_t	type;
	char	*name;
} ArubaSecretType;

#define ARUBA_SECRET_UNKNOWN 0
#define ARUBA_SECRET_SHA1 1
#define ARUBA_SECRET_MD5 2
#define ARUBA_SECRET_PLAIN 4

const ArubaSecretType secret_lookup[] = {
	{ ARUBA_SECRET_SHA1,  "sha1"  },
	{ ARUBA_SECRET_MD5,   "md5"   },
	{ ARUBA_SECRET_PLAIN, "cleartext" },
	{ 0, NULL }
};


#define ArubaActionType ArubaSecretType

#define ARUBA_ACTION_UNKNOWN 0
#define ARUBA_ACTION_BLACKLIST 1
#define ARUBA_ACTION_SETROLE 2

const ArubaActionType action_lookup[] = {
	{ ARUBA_ACTION_BLACKLIST, "blacklist" },
	{ ARUBA_ACTION_SETROLE,   "setrole"   },
	{ 0, NULL }
};


#define ArubaResponseCode ArubaSecretType 

#define ARUBA_RESP_SUCCESS 0
#define ARUBA_RESP_UNKN_USER 1
#define ARUBA_RESP_UNKN_ROLE 2
#define ARUBA_RESP_UNKN_EXT_AGENT 3
#define ARUBA_RESP_AUTH_FAILED 4
#define ARUBA_RESP_INVAL_CMD 5
#define ARUBA_RESP_INVAL_AUTH_METHOD 6
#define ARUBA_RESP_INVAL_MSG_DGST 7
#define ARUBA_RESP_MSSNG_MSG_AUTH 8

const ArubaResponseCode response_lookup[] = {
	{ ARUBA_RESP_SUCCESS,          "success" },
	{ ARUBA_RESP_UNKN_USER,        "unknown user" },
	{ ARUBA_RESP_UNKN_ROLE,        "unknown role" },
	{ ARUBA_RESP_UNKN_EXT_AGENT,   "unknown external agent" },
	{ ARUBA_RESP_AUTH_FAILED,      "authentication failed" },
	{ ARUBA_RESP_INVAL_CMD,        "invalid command" },
	{ ARUBA_RESP_INVAL_AUTH_METHOD, 
			"invalid message authentication method" },
	{ ARUBA_RESP_INVAL_MSG_DGST,   "invalid message digest" },
	{ ARUBA_RESP_MSSNG_MSG_AUTH,   "missing message authentication" },
	{ 0, NULL }
};


void AlertArubaActionInit(char *);
SpoAlertArubaActionData *ParseAlertArubaActionArgs(char *);
void AlertArubaActionCleanExitFunc(int, void *);
void AlertArubaActionRestartFunc(int, void *);
void AlertArubaAction(Packet *, char *, void *, Event *);
int ArubaSwitchConnect(SpoAlertArubaActionData *data);
int ArubaSwitchSend(SpoAlertArubaActionData *data, uint8_t *post, int len);
int ArubaSwitchRecv(SpoAlertArubaActionData *data, uint8_t *recv, int maxlen);

/*
 * Function: SetupAlertArubaAction()
 *
 * Purpose: Registers the output plugin keyword and initialization 
 *          function into the output plugin list.  This is the function that
 *          gets called from InitOutputPlugins() in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void AlertArubaActionSetup(void)
{
	/* link the preprocessor keyword to the init function in 
	   the preproc list */
    RegisterOutputPlugin("alert_aruba_action", OUTPUT_TYPE_FLAG__ALERT,
                         AlertArubaActionInit);

	DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output plugin: AlertArubaAction is "
			"setup...\n"););
}


/*
 * Function: AlertArubaActionInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
void AlertArubaActionInit(char *args)
{
	SpoAlertArubaActionData *data;

	DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output: AlertArubaAction "
			"Initialized\n"););

	/* parse the argument list from the rules file */
	data = ParseAlertArubaActionArgs(args);

	DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Linking AlertArubaAction functions "
			"to call lists...\n"););
	
	/* Set the preprocessor function into the function list */
	AddFuncToOutputList(AlertArubaAction, OUTPUT_TYPE__ALERT, data);
	AddFuncToCleanExitList(AlertArubaActionCleanExitFunc, data);
	AddFuncToRestartList(AlertArubaActionRestartFunc, data);
}

void AlertArubaAction(Packet *p, char *msg, void *arg, Event *event)
{
	char cmdbuf[MAX_XML_PAYLOAD_LEN], post[MAX_POST_LEN];
	char response[MAX_RESPONSE_LEN];
	char *cmdbufp, *responsecode, *responsemsg;
	int postlen, xmllenrem, i, responsecodei;


	SpoAlertArubaActionData *data = (SpoAlertArubaActionData *)arg;
	cmdbufp = cmdbuf;

	/* Establish a connection to the switch */
	data->fd = ArubaSwitchConnect(data);
	if (data->fd < 0) {
		ErrorMessage("Unable to connect to Aruba switch at %s\n",
#ifdef SUP_IP6
				inet_ntoa(&data->aswitch));
#else
				inet_ntoa(data->aswitch));
#endif
		return;
	}

	xmllenrem = MAX_XML_PAYLOAD_LEN;

	switch(data->action_type) {
		case ARUBA_ACTION_BLACKLIST:
		snprintf(cmdbufp, xmllenrem, "xml=<aruba "
				"command=user_blacklist>");
		break;

		case ARUBA_ACTION_SETROLE:
		snprintf(cmdbufp, xmllenrem, "xml=<aruba command=user_add>"
				"<role>%s</role>", data->role_name);
		break;

		default: /* The parser prevents this from happening */
		ErrorMessage("aruba_action: invalid action type specified");
		return;
		break;
	}

	xmllenrem -= strlen(cmdbufp);
	cmdbufp += strlen(cmdbufp);

	if (xmllenrem < 1) {
		ErrorMessage("aruba_action: configuration parameters too "
				"long\n");
		FatalError("Unable to parse configuration parameters for Aruba"
				"Action output plugin.\n");
		return;
	}

	snprintf(cmdbufp, xmllenrem, "<ipaddr>%s</ipaddr>",
#ifdef SUP_IP6
			inet_ntoa(GET_SRC_ADDR(p))
#else
			inet_ntoa(p->iph->ip_src)
#endif
        );

	xmllenrem -= strlen(cmdbufp);
	cmdbufp += strlen(cmdbufp);

	if (xmllenrem < 1) {
		ErrorMessage("aruba_action: configuration parameters too "
				"long\n");
		FatalError("Unable to parse configuration parameters for Aruba"
				"Action output plugin.\n");
		return;
	}

	switch(data->secret_type) {
		case ARUBA_SECRET_SHA1:
		snprintf(cmdbufp, xmllenrem, "<authentication>sha-1"
				"</authentication>");
		break;

		case ARUBA_SECRET_MD5:
		snprintf(cmdbufp, xmllenrem, "<authentication>md5"
				"</authentication>");
		break;

		case ARUBA_SECRET_PLAIN:
		snprintf(cmdbufp, xmllenrem, "<authentication>cleartext"
				"</authentication>");
		break;

		default: /* The parser prevents this from happening */
		ErrorMessage("aruba_action: invalid secret type specified");
		return;
		break;
	}

	xmllenrem -= strlen(cmdbufp);
	cmdbufp += strlen(cmdbufp);

	if (xmllenrem < 1) {
		ErrorMessage("aruba_action: configuration parameters too "
				"long\n");
		FatalError("Unable to parse configuration parameters for Aruba"
				"Action output plugin.\n");
		return;
	}

	snprintf(cmdbufp, xmllenrem, "<key>%s</key>", data->secret);
	xmllenrem -= strlen(cmdbufp);
	cmdbufp += strlen(cmdbufp);

	if (xmllenrem < 1) {
		ErrorMessage("aruba_action: configuration parameters too "
				"long\n");
		FatalError("Unable to parse configuration parameters for Aruba"
				"Action output plugin.\n");
		return;
	}

	snprintf(cmdbufp, xmllenrem, "<version>1.0</version>");
	xmllenrem -= strlen(cmdbufp);
	cmdbufp += strlen(cmdbufp);

	if (xmllenrem < 1) {
		ErrorMessage("aruba_action: configuration parameters too "
				"long\n");
		FatalError("Unable to parse configuration parameters for Aruba"
				"Action output plugin.\n");
		return;
	}

	snprintf(cmdbufp, xmllenrem, "</aruba>");
	xmllenrem -= strlen(cmdbufp);
	cmdbufp += strlen(cmdbufp);

	cmdbufp = NULL;

	postlen = snprintf(post, MAX_POST_LEN-1,
			"POST /auth/command.xml HTTP/1.0\r\n"
			"User-Agent: snort\r\n"
			"Host: %s\r\n"
			"Pragma: no-cache\r\n"
			"Content-Length: %lu\r\n"
			"Content-Type: application/xml\r\n"
			"\r\n"
			"%s",
#ifdef SUP_IP6
			inet_ntoa(&data->aswitch),
#else
			inet_ntoa(data->aswitch),
#endif
			(unsigned long)strlen(cmdbuf), cmdbuf
        );
	
	/* Send the action command to the switch */
	if (ArubaSwitchSend(data, (uint8_t *)post, postlen) != postlen) {
		ErrorMessage("aruba_action: Error sending data to Aruba "
				"switch.\n");
		close(data->fd);
		return;
	}

	/* Read the response from the switch */
	if (ArubaSwitchRecv(data, (uint8_t *)response, MAX_RESPONSE_LEN) < 0) {
		ErrorMessage("aruba_action: Error reading response from Aruba"
				" switch\n");
		close(data->fd);
		return;
	}

	/* Extract the result code from the response */
	responsecode = strstr(response, "<code>");
	if (responsecode == NULL) {
		ErrorMessage("aruba_action: Error extracting response code "
				"from Aruba switch\n");
		close(data->fd);
		return;
	}

	/* Advance beyond "<code>" */
	responsecode += (strlen("<code>"));

	/* Lookup code message */
	//responsecodei = 0;
	if (sscanf(responsecode, "%d", &responsecodei) != 1) {
		ErrorMessage("aruba_action: Invalid response code returned from"
				" the Aruba switch.\n");
		return;
	}

	if (responsecodei != 0) {
		responsemsg = NULL;
		for (i=0; response_lookup[i].name != NULL; i++) {
			if (response_lookup[i].type == responsecodei) {
				responsemsg = response_lookup[i].name;
				break;
			}
		}

		if (responsemsg == NULL) {
			ErrorMessage("aruba_action: Switch returned error "
					"status of %d \"unknown\"\n",
					responsecodei);
		} else {
			ErrorMessage("aruba_action: Switch returned error "
					"status of %d \"%s\"\n",
					responsecodei, responsemsg);
		}

		close(data->fd);
		return;
	}


	close(data->fd);
		
	return;
}

int ArubaSwitchSend(SpoAlertArubaActionData *data, uint8_t *post, int len)
{
	return(write(data->fd, post, len));
}

int ArubaSwitchRecv(SpoAlertArubaActionData *data, uint8_t *recv, int maxlen)
{
	return(read(data->fd, recv, maxlen));
}

int ArubaSwitchConnect(SpoAlertArubaActionData *data)
{
	struct sockaddr_in sa4;
	struct sockaddr_in6 sa6;

	data->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (data->fd < 0) {
		ErrorMessage("aruba_action: socket error\n");
		return -1;
	}

#ifdef SUP_IP6
    if(data->aswitch.family == AF_INET) {
	    sa4.sin_addr.s_addr = data->aswitch.ip32[0];
#else
	    sa4.sin_addr.s_addr = (unsigned int)(data->aswitch.s_addr);
#endif
    	sa4.sin_family = AF_INET;
    	sa4.sin_port = htons(80);

    	if (connect(data->fd, (struct sockaddr *)&sa4, sizeof(sa4)) < 0) {
    		perror("connect");
    		ErrorMessage("aruba_action: Unable to connect to switch\n");
    		close(data->fd);
    		return -1;
    	}
#ifdef SUP_IP6
    } 
    else {
	    memcpy(&sa6.sin6_addr, data->aswitch.ip8, 16);
    	sa6.sin6_family = AF_INET6;
    	sa6.sin6_port = htons(80);

       	if (connect(data->fd, (struct sockaddr *)&sa6, sizeof(sa6)) < 0) {
    		perror("connect");
    		ErrorMessage("aruba_action: Unable to connect to switch\n");
    		close(data->fd);
    		return -1;
    	}
    }  
#endif



#ifdef SUP_IP6
    if(data->aswitch.family == AF_INET) {
	    sa4.sin_addr.s_addr = data->aswitch.ip32[0];
    	sa4.sin_family = AF_INET;
    	sa4.sin_port = htons(80);

    	if (connect(data->fd, (struct sockaddr *)&sa4, sizeof(sa4)) < 0) {
    		perror("connect");
    		ErrorMessage("aruba_action: Unable to connect to switch\n");
    		close(data->fd);
    		return -1;
    	}
    } 
    else {
	    memcpy(&sa6.sin6_addr, data->aswitch.ip8, 16);
#else
	    memcpy(&sa6.sin6_addr, &data->aswitch, 16);
#endif
    	sa6.sin6_family = AF_INET6;
    	sa6.sin6_port = htons(80);

       	if (connect(data->fd, (struct sockaddr *)&sa6, sizeof(sa6)) < 0) {
    		perror("connect");
    		ErrorMessage("aruba_action: Unable to connect to switch\n");
    		close(data->fd);
    		return -1;
    	}
#ifdef SUP_IP6
    }  
#endif
     
	return data->fd;
}


/*
 * Function: ParseAlertArubaActionArgs(char *)
 *
 * Purpose: Process the preprocessor arguments from the rules file and 
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init 
 *          function.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
SpoAlertArubaActionData *ParseAlertArubaActionArgs(char *args)
{
	char **toks, **action_toks;
	int num_toks, num_action_toks, i;
	SpoAlertArubaActionData *data;

	data = (SpoAlertArubaActionData *)SnortAlloc(sizeof(SpoAlertArubaActionData));

	if(args == NULL) {
		ErrorMessage("aruba_action: you must specify arguments for the "
				"Aruba Action plugin\n");
		FatalError("No output plugin arguments specified\n");
		return NULL;
	}

	DEBUG_WRAP(DebugMessage(DEBUG_LOG, "ParseAlertArubaActionArgs: %s\n",
			args););

	toks = mSplit(args, " ", 4, &num_toks, 0);

	if (num_toks != 4) {
		ErrorMessage("aruba_action: incorrect number of arguments "
				"specified (%d)\n", num_toks);
		FatalError("Invalid argument count\n");
		return NULL;
	}

#ifdef SUP_IP6 // XXX could probably be changed to a macro
	if (sfip_pton(toks[0], &data->aswitch) == 0) 
#else
	if (inet_aton(toks[0], &data->aswitch) == 0) 
#endif
    {
		ErrorMessage("aruba_action: invalid Aruba switch address "
				"specified (%s)\n", toks[0]);
		FatalError("Invalid Aruba switch address.\n");
		return NULL;
	}

	for (i=0; secret_lookup[i].name != NULL; i++) {
		if (strncmp(toks[1], secret_lookup[i].name, 
				strlen(secret_lookup[i].name)) == 0) {
			data->secret_type = secret_lookup[i].type;
			break;
		}
	}

	if (data->secret_type == ARUBA_SECRET_UNKNOWN) {
		ErrorMessage("aruba_action: unknown secret type \"%s\"\n",
				toks[1]);
		FatalError("Unsupported secret type specified\n");
		return NULL;
	}

	data->secret = (char *)SnortAlloc(strlen(toks[2])+1);
	strncpy(data->secret, toks[2], strlen(toks[2]));

	/* action can be "blacklist" or "setrole:rolename", parse */
	for (i=0; action_lookup[i].name != NULL; i++) {
		if (strncmp(action_lookup[i].name, toks[3], 
				strlen(action_lookup[i].name)) == 0) {
			data->action_type = action_lookup[i].type;
			break;
		}
	}

	if (data->action_type == ARUBA_ACTION_UNKNOWN) {
		ErrorMessage("aruba_action: unknown action type \"%s\"\n",
				toks[3]);
		FatalError("Unsupported action type specified\n");
		return NULL;
	}

	/* Break out role name for setrole action */
	if (data->action_type == ARUBA_ACTION_SETROLE) {
		action_toks = mSplit(toks[3], ":", 2, &num_action_toks, 0);
		if (num_action_toks != 2) {
			ErrorMessage("aruba_action: malformed setrole action "
					"specification \"%s\"\n", toks[3]);
			FatalError("Improperly formatted action\n");
			return NULL;
		} 

		data->role_name = (char *)SnortAlloc(strlen(action_toks[1])+1);
		strncpy(data->role_name, action_toks[1], 
				strlen(action_toks[1]));
	}	

	/* free toks */
	mSplitFree(&toks, num_toks);

	return data;
}

void AlertArubaActionCleanExitFunc(int signal, void *arg)
{
	SpoAlertArubaActionData *data = (SpoAlertArubaActionData *)arg;

	DEBUG_WRAP(DebugMessage(DEBUG_LOG,"AlertArubaActionCleanExitFunc\n"););
	free(data->secret);
	free(data->role_name);
	free(data);
}

void AlertArubaActionRestartFunc(int signal, void *arg)
{
	SpoAlertArubaActionData *data = (SpoAlertArubaActionData *)arg;
	
	DEBUG_WRAP(DebugMessage(DEBUG_LOG,"AlertArubaActionRestartFunc\n"););
	free(data->secret);
	free(data->role_name);
	free(data);
}


