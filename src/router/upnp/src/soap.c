/*
 * Broadcom UPnP module SOAP implementation
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: soap.c,v 1.13 2008/12/26 10:29:10 Exp $
 */

#include <upnp.h>

/* Find a statevar with a given name */
UPNP_STATE_VAR *find_state_var(UPNP_CONTEXT *context, UPNP_SERVICE *service, char *name)
{
	UPNP_STATE_VAR *statevar;

	for (statevar = service->statevar_table; statevar->name; statevar++) {
		if (strcmp(name, statevar->name) == 0)
			break;
	}
	if (statevar->name == 0)
		return 0;

	return statevar;
}

/*
 * Return token before and character pointer after the delimiter.
 * Specially developed for UPnP SOAP.
 */
static char *strtok_n(char *s, char *delim, char **endp)
{
	char *p = s;
	char *token = NULL;

	/* pre-condition */
	*endp = NULL;

	if (p == NULL)
		return NULL;

	/* consume leading delimiters */
	while (*p) {
		if (strchr(delim, *p) == NULL) {
			token = p;
			break;
		}

		*p++ = 0;
	}

	if (*p == 0)
		return NULL;

	/* make this token */
	while (*p) {
		if (strchr(delim, *p) != NULL)
			break;

		p++;
	}

	if (*p != 0) {
		*p++ = 0;

		/* consume tailing delimiters */
		while (*p) {
			if (strchr(delim, *p) == NULL)
				break;

			*p++ = 0;
		}

		if (*p)
			*endp = p;
	}

	return token;
}

/* Convertmpare two strings ignore case */
static int struppercmp(char *s1, char *s2)
{
	int i;
	int len = strlen(s1);
	char x = 0, y = 0;

	/* To upper case */
	for (i = 0; i < len + 1; i++) {
		x = toupper(s1[i]);
		y = toupper(s2[i]);
		if (x != y)
			break;
	}

	return (x - y);
}

/* Parse version line in an XML file */
static char *parse_version(char *str)
{
	char *ptr = str;
	char *start;
	char *value;
	int span;

	/* eat leading white space */
	while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n')) {
		ptr++;
	}

	if (*ptr != '<')
		return NULL;

	/* locate '<' and '>' */
	start = ptr;

	if (strtok_n(start, ">", &value) == NULL)
		return NULL;

	if ((value - start > 5) && memcmp(start, "<?xml", 5) == 0) {
		start = value;
		span = strspn(start, "\r\n");
		start += span;
	}

	return start;
}

/* Parse elments in an XML file */
static char *parse_element(char *str, char **tag, char **name, char **attr, char **next)
{
	char *ptr = str;
	char *start;
	char *element;
	char *aux;
	char *value;
	char *value_end;
	char *next_element;

	/* eat leading white space */
	while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n')) {
		ptr++;
	}

	if (*ptr != '<')
		return NULL;

	/* locate '<' and '>' */
	start = ptr;

	if (strtok_n(start, ">", &value) == NULL)
		return NULL;

	/* locate attribute */
	strtok_n(start, " \t\r\n", &aux);

	/* parse "<s:element" and convert to "/s:element" */
	ptr = strchr(start, ':');
	if (!ptr) {
		element = start + 1;
	} else {
		element = ptr + 1;
	}

	*start = '/';

	/*
	 * locate "</s:element>";
	 * sometimes, XP forgets to send the balance element
	 */

	/* value can be 0 */
	if (value) {
		ptr = strstr(value, start);
	} else {
		ptr = 0;
	}

	if (ptr) {
		/* Check "<" */
		if (*(ptr - 1) != '<')
			return NULL;

		*(ptr - 1) = '\0';

		/* save value end for eat white space */
		value_end = ptr - 2;

		/* Check ">" */
		ptr += strlen(start);
		if (*ptr != '>')
			return NULL;

		ptr++;
		next_element = ptr;

		/* consume leading white spaces */
		while (value <= value_end) {
			if (*value == ' ' || *value == '\t' || *value == '\r' || *value == '\n') {
				/* Skip it */
				value++;
			} else {
				break;
			}
		}

		/* consume tailing white spaces */
		while (value_end >= value) {
			if (*value_end == ' ' || *value_end == '\t' || *value_end == '\r' || *value_end == '\n') {
				/*
				 * Null ending the white space,
				 * and moving backward to do
				 * further checking.
				 */
				*value_end = '\0';
				value_end--;
			} else {
				break;
			}
		}
	} else {
		next_element = NULL;
	}

	/* output values */
	if (tag) {
		if (element == start + 1) {
			*tag = NULL;
		} else {
			*tag = start + 1; /* Skip < */
			*(element - 1) = 0; /* Set ":" to NULL */
		}
	}

	if (name)
		*name = element;

	if (attr)
		*attr = aux;

	if (next)
		*next = next_element;

	return value;
}

/* Send SOAP error response */
void soap_send_error(UPNP_CONTEXT *context, int error)
{
	char time_buf[64];
	char *err_str;
	int body_len;
	int len;

	switch (error) {
	case SOAP_INVALID_ACTION:
		err_str = "Invalid Action";
		break;

	case SOAP_INVALID_ARGS:
		err_str = "Invalid Argument";
		break;

	case SOAP_OUT_OF_SYNC:
		err_str = "Out of Sync";
		break;

	case SOAP_INVALID_VARIABLE:
		err_str = "Invalid Variable";
		break;

	case SOAP_DEVICE_INTERNAL_ERROR:
		err_str = "Action Failed";
		break;

	default:
		err_str = "Invalid Argument";
		break;
	}

	/* format body */
	body_len = sprintf(context->body_buffer,
			   "<s:Envelope "
			   "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			   "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
			   "<s:Body>"
			   "<s:Fault>"
			   "<faultcode>s:Client</faultcode>"
			   "<faultstring>UPnPError</faultstring>"
			   "<detail>"
			   "<UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">"
			   "<errorCode>%d</errorCode>"
			   "<errorDescription>%s</errorDescription>"
			   "</UPnPError>"
			   "</detail>"
			   "</s:Fault>"
			   "</s:Body>"
			   "</s:Envelope>",
			   error, err_str);

	/* format header */
	gmt_time(time_buf); /* get GMT time */
	len = sprintf(context->head_buffer,
		      "HTTP/1.1 500 Internal Server Error\r\n"
		      "Content-Length: %d\r\n"
		      "Content-Type: text/xml; charset=\"utf-8\"\r\n"
		      "Date: %s\r\n"
		      "EXT: \r\n"
		      "Server: POSIX, UPnP/1.0 %s/%s\r\n"
		      "Connection: close\r\n"
		      "\r\n"
		      "%s",
		      body_len, time_buf, context->config.os_name, context->config.os_ver, context->body_buffer);

	/* send error message */
	if (send(context->fd, context->head_buffer, len, 0) == -1) {
		upnp_syslog(LOG_ERR, "soap_send_error() failed");
	}

	return;
}

/* Send SOAP response to the peer */
void soap_send(UPNP_CONTEXT *context)
{
	char time_buf[64];
	int head_len;
	int body_len;

	/* get GMT time string */
	gmt_time(time_buf);

	/* Get Content-Length */
	body_len = strlen(context->body_buffer);

	/* Print String */
	head_len = sprintf(context->head_buffer,
			   "HTTP/1.1 200 OK\r\n"
			   "Content-Length: %d\r\n"
			   "Content-Type: text/xml; charset=\"utf-8\"\r\n"
			   "Date: %s\r\n"
			   "EXT: \r\n"
			   "Server: POSIX, UPnP/1.0 %s/%s\r\n"
			   "Connection: close\r\n"
			   "\r\n"
			   "%s",
			   body_len, time_buf, context->config.os_name, context->config.os_ver, context->body_buffer);

	send(context->fd, context->head_buffer, head_len, 0);
	return;
}

/* Compose action response message and send */
void send_action_response(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_ACTION *action)
{
	char *p = context->body_buffer;
	OUT_ARGUMENT *temp = context->out_arguments;
	int len;

	len = sprintf(context->body_buffer,
		      "<s:Envelope "
		      "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		      "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		      "<s:Body>\r\n"
		      "<u:%sResponse xmlns:u=\"%s:1\">\r\n",
		      action->name, service->name);
	p += len;

	/* concatenate output values */
	while (temp) {
		/* Traslate the value to output string */
		translate_value(context, &temp->value);

		len = sprintf(p, "<%s>%s</%s>\r\n", temp->name, temp->value.val.str, temp->name);

		p += len;
		temp = temp->next;
	}

	/* adding closure */
	sprintf(p,
		"</u:%sResponse>\r\n"
		"</s:Body>\r\n"
		"</s:Envelope>\r\n",
		action->name);

	soap_send(context);
	return;
}

/* 
 * SOAP action invoke function.
 *  (1) verify all required input arguments
 *  (2) prepare all output arguements buffer
 *  (3) invoke action function
 *  (4) send back soap action response
 */
int soap_control(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_ACTION *action)
{
	int error = SOAP_INVALID_ARGS;
	int i;
	IN_ARGUMENT *in_temp;
	OUT_ARGUMENT *temp;

	/* Qualify the input argument */
	for (i = 0; i < action->in_num; i++) {
		/* in argument list */
		in_temp = context->in_args;
		while (in_temp) {
			if (strcmp(in_temp->name, action->in_argument[i].name) == 0) {
				/* Get the related state variable */
				in_temp->statevar = service->statevar_table + action->in_argument[i].related_id;

				/* convert types */
				in_temp->value.type = action->in_argument[i].type;

				if (convert_value(context, &in_temp->value) != 0)
					goto err_out;

				break;
			}

			in_temp = in_temp->next;
		}

		if (in_temp == NULL)
			goto err_out;
	}

	/* allocate buffer for all output arguments */
	temp = context->out_args;
	for (i = 0; i < action->out_num; i++, temp++) {
		if (temp == context->out_args + UPNP_MAX_OUT_ARG) {
			upnp_syslog(LOG_ERR, "Check UPNP_MAX_OUT_ARG!!");
			return R_ERROR;
		}

		temp->name = action->out_argument[i].name;
		temp->statevar = service->statevar_table + action->out_argument[i].related_id;
		temp->value.type = action->out_argument[i].type;

		/* append to out_argument list */
		temp->next = 0;
		if (i > 0)
			(temp - 1)->next = temp;
	}

	/* invoke action */
	context->in_arguments = action->in_num ? context->in_args : 0;
	context->out_arguments = action->out_num ? context->out_args : 0;
	error = (*action->action)(context, service, context->in_arguments, context->out_arguments);
	if (error == 0) {
		send_action_response(context, service, action);
		return 0;
	}

err_out:
	soap_send_error(context, error);
	return 0;
}

/* 
 * Parse the elements and get the action name and argument lists,
 * then invoke SOAP action.
 */
int action_process(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	char *p = context->content;
	UPNP_ACTION *action;
	char *element;
	char *next;
	int error = 0;
	char *arg_name;
	char *arg_value;

	IN_ARGUMENT *temp;

	while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
		p++;
	}

	if (memcmp(p, "<?xml", 5) == 0) {
		p = parse_version(p);
		if (p == NULL)
			return R_BAD_REQUEST;
	}

	/* process Envelope */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || struppercmp(element, "Envelope") != 0)
		return R_BAD_REQUEST;

	/* process Body */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || struppercmp(element, "Body") != 0)
		return R_BAD_REQUEST;

	/* process action */
	p = parse_element(p, 0, &element, 0, 0);
	if (strcmp(element, context->SOAPACTION) != 0)
		return R_BAD_REQUEST;

	/* find action */
	for (action = service->action_table; action->name; action++) {
		if (strcmp(context->SOAPACTION, action->name) == 0)
			break;
	}

	if (action->name == NULL) {
		soap_send_error(context, SOAP_INVALID_ACTION);
		return 0;
	}

	/* process argument list */
	temp = context->in_args;
	next = p;
	while (next) {
		arg_value = parse_element(next, 0, &arg_name, 0, &next);
		if (!arg_value)
			break;

		if (temp == context->in_args + UPNP_MAX_IN_ARG) {
			soap_send_error(context, SOAP_INVALID_ARGS);
			return 0;
		}

		temp->name = arg_name;
		strlcpy(temp->value.val.str, arg_value, sizeof(temp->value.val.str));

		/* append to in argument list */
		temp->next = 0;
		if (temp != context->in_args)
			(temp - 1)->next = temp;

		temp++;
	}

	/* Make sure in argument count is same */
	if (context->in_args + action->in_num != temp) {
		soap_send_error(context, SOAP_INVALID_ARGS);
		return 0;
	}

	/* invoke control action */
	error = soap_control(context, service, action);
	if (error == 0) {
		/* 
		 * Check whether we need further
		 * eventing processing?
		 */
		if (service->evented) {
			/* invoke gena function */
			gena_notify(context, service, NULL);
			gena_notify_complete(context, service);
		}
	}

	return error;
}

/* Send SOAP query response */
void send_query_response(UPNP_CONTEXT *context, char *value)
{
	sprintf(context->body_buffer,
		"<s:Envelope "
		"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		"<s:Body>\r\n"
		"<u:QueryStateVariableResponse xmlns:u=\"urn:schemas-upnp-org:control1-1-0\">\r\n"
		"<return>"
		"%s"
		"</return>\r\n"
		"</u:QueryStateVariableResponse>\r\n"
		"</s:Body>\r\n"
		"</s:Envelope>\r\n",
		value);

	soap_send(context);
	return;
}

/* Invoke statevar query function */
int soap_query(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar)
{
	UPNP_VALUE value;
	int rc = 0;

	/* invoke query function */
	value.type = statevar->type;
	if (statevar->func) {
		rc = (*statevar->func)(context, service, statevar, &value);
		if (rc != 0) {
			soap_send_error(context, rc);
			return 0;
		}

		/* translate value to string */
		translate_value(context, &value);
	} else {
		/* Should not be here, send null string */
		value.val.str[0] = 0;
	}

	send_query_response(context, value.val.str);
	return 0;
}

/* 
 * Parse the elements and get the statevar name,
 * then invoke query.
 */
int query_process(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_STATE_VAR *statevar;
	char *p = context->content;
	char *element;

	while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
		p++;
	}

	if (memcmp(p, "<?xml", 5) == 0) {
		p = parse_version(p);
		if (p == NULL)
			return R_BAD_REQUEST;
	}

	/* process Envelope */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || struppercmp(element, "Envelope") != 0)
		return R_BAD_REQUEST;

	/* process Body */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || struppercmp(element, "Body") != 0)
		return R_BAD_REQUEST;

	/* process QueryStateVariable */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || strcmp(element, "QueryStateVariable") != 0)
		return R_BAD_REQUEST;

	/* process varName */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || strcmp(element, "varName") != 0)
		return R_BAD_REQUEST;

	/* find state variable */
	statevar = find_state_var(context, service, p);
	if (statevar == NULL) {
		soap_send_error(context, SOAP_INVALID_VARIABLE);
		return 0;
	}

	return soap_query(context, service, statevar);
}

/* Get statevar called by upnp request */
int soap_get_state_var(UPNP_CONTEXT *context, char *url, char *name, UPNP_VALUE *value)
{
	UPNP_SERVICE *service;
	UPNP_STATE_VAR *statevar;
	int rc = 0;

	/* Search for service */
	service = get_service(context, url);
	if (service == 0) {
		/*
		 * Maybe the caller use event url,
		 * Try it again.
		 */
		service = find_event(context, url);
		if (service == 0)
			return -1;
	}

	/* Search for statevar */
	statevar = find_state_var(context, service, name);
	if (statevar == 0 || statevar->func == 0)
		return -1;

	/* Retrieve the value */
	value->type = statevar->type;
	rc = (*statevar->func)(context, service, statevar, value);
	if (rc)
		return -1;

	return 0;
}

/*
 * SOAP process entry;
 * Parse header and decide which control function
 * to go.
 */
int soap_process(UPNP_CONTEXT *context)
{
	char *action_field;
	UPNP_SERVICE *service;

	/* find service */
	service = get_service(context, context->url);
	if (service == NULL)
		return R_NOT_FOUND;

	action_field = context->SOAPACTION;
	if (action_field == NULL)
		return R_BAD_REQUEST;

	if (strcmp(action_field, "\"urn:schemas-upnp-org:control-1-0#QueryStateVariable\"") == 0 ||
	    strcmp(action_field, "urn:schemas-upnp-org:control-1-0#QueryStateVariable") == 0) {
		/* query control */
		return query_process(context, service);
	} else {
		/* action control */
		char *action_name;
		int pos, name_len;

		if (*action_field == '\"')
			action_field++;

		/* compare the service name */
		name_len = strlen(service->name);
		if (memcmp(action_field, service->name, name_len) == 0) {
			/* get action name */
			pos = strcspn(action_field, "#");
			action_field += pos + 1;

			action_name = action_field;
			pos = strcspn(action_field, "\"\t ");
			action_name[pos] = 0;

			context->SOAPACTION = action_name;

			return action_process(context, service);
		} else {
			return R_BAD_REQUEST;
		}
	}
}

/* Search the service table for the target control URL */
UPNP_SERVICE *get_service(UPNP_CONTEXT *context, char *url)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	UPNP_DEVCHAIN *chain;
	UPNP_SERVICE *service;

	for (chain = ifp->device_chain; chain; chain = chain->next) {
		for (service = chain->device->service_table; service && service->control_url; service++) {
			if (strcmp(url, service->control_url) == 0) {
				ifp->focus_devchain = chain;
				return service;
			}
		}
	}

	return NULL;
}

/* Search the advertise table for the target name */
UPNP_ADVERTISE *get_advertise(UPNP_CONTEXT *context, char *name)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	UPNP_DEVCHAIN *chain;
	UPNP_ADVERTISE *advertise;

	for (chain = ifp->device_chain; chain; chain = chain->next) {
		for (advertise = chain->device->advertise_table; advertise && advertise->name; advertise++) {
			if (strcmp(name, advertise->name) == 0) {
				ifp->focus_devchain = chain;
				return advertise;
			}
		}
	}

	return NULL;
}
