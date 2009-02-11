/*
 * Broadcom UPnP module, soap_x_wfawlanconfig.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: soap_x_wfawlanconfig.c,v 1.7 2008/08/25 08:18:38 Exp $
 */
#include <upnp.h>
#include <WFADevice.h>

/*
 * WARNNING: PLEASE IMPLEMENT YOUR CODES AFTER 
 *          "<< USER CODE START >>"
 * AND DON'T REMOVE TAG :
 *          "<< AUTO GENERATED FUNCTION: "
 *          ">> AUTO GENERATED FUNCTION"
 *          "<< USER CODE START >>"
 */

/* << AUTO GENERATED FUNCTION: statevar_WLANResponse() */
static int
statevar_WLANResponse
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BIN_LEN(value), UPNP_BIN_DATA(value) )

	/* << USER CODE START >> */
	UPNP_BIN_LEN(value) = 0;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_WLANEventType() */
static int
statevar_WLANEventType
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_UI1(value) )

	/* << USER CODE START >> */
	/* 
	 * Simon note:
	 * Should find a way to solve this situation.
	 */
	/* UPNP_UI1(value) = WSC_WLAN_EVENT_TYPE_EAP_FRAME; */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_InMessage() */
static int
statevar_InMessage
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BIN_LEN(value), UPNP_BIN_DATA(value) )

	/* << USER CODE START >> */
	UPNP_BIN_LEN(value) = 0;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_OutMessage() */
static int
statevar_OutMessage
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BIN_LEN(value), UPNP_BIN_DATA(value) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_APSettings() */
static int
statevar_APSettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BIN_LEN(value), UPNP_BIN_DATA(value) )

	/* << USER CODE START >> */
	char *NewAPSettings = "Sample Binary";
	int len = strlen(NewAPSettings);
	
	UPNP_BIN_LEN(value) = len;
	memcpy(UPNP_BIN_DATA(value), NewAPSettings, len);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_Message() */
static int
statevar_Message
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BIN_LEN(value), UPNP_BIN_DATA(value) )

	/* << USER CODE START >> */
	UPNP_BIN_LEN(value) = 0;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_STAStatus() */
static int
statevar_STAStatus
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_UI1(value) )

	/* Simon note:
	 * Should find a way to solve this situation.
	 */
	UPNP_UI1(value) = 0x1;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_WLANEventMAC() */
static int
statevar_WLANEventMAC
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_STR(value) )

	/* << USER CODE START >> */
	strcpy(UPNP_STR(value), "");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_DeviceInfo() */
static int
statevar_DeviceInfo
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BIN_LEN(value), UPNP_BIN_DATA(value) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_STASettings() */
static int
statevar_STASettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BIN_LEN(value), UPNP_BIN_DATA(value) )

	/* << USER CODE START >> */
	char *NewSTASettings = "Sample Binary";
	int len = strlen(NewSTASettings);

	UPNP_BIN_LEN(value) = len;
	memcpy(UPNP_BIN_DATA(value), NewSTASettings, len);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_APStatus() */
static int
statevar_APStatus
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_UI1(value) )

	/* << USER CODE START >> */
	/*
	 * Simon note:
	 * Should find a way to solve this situation.
	 */
	UPNP_UI1(value) = 0x1;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_WLANEvent() */
static int
statevar_WLANEvent
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BIN_LEN(value), UPNP_BIN_DATA(value) )

	/* << USER CODE START >> */
	UPNP_EVALUE *evalue;

	evalue = get_evalue(context, statevar);
	if (evalue)
		*value = evalue->value;
	else
		UPNP_BIN_LEN(value) = 0;

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_DelAPSettings() */
static int
action_DelAPSettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewAPSettings = UPNP_IN_ARG("NewAPSettings"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewAPSettings), ARG_BIN_DATA(in_NewAPSettings) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_DelSTASettings() */
static int
action_DelSTASettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewSTASettings = UPNP_IN_ARG("NewSTASettings");)

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewSTASettings), ARG_BIN_DATA(in_NewSTASettings) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetAPSettings() */
static int
action_GetAPSettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewMessage = UPNP_IN_ARG("NewMessage"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewAPSettings = UPNP_OUT_ARG("NewAPSettings"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewMessage), ARG_BIN_DATA(in_NewMessage) )
	UPNP_USE_HINT( ARG_BIN_LEN(out_NewAPSettings), ARG_BIN_DATA(out_NewAPSettings) )

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewAPSettings = UPNP_OUT_ARG("NewAPSettings");

	return statevar_APSettings(context, service, out_NewAPSettings->statevar, ARG_VALUE(out_NewAPSettings));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetDeviceInfo() */
static int
action_GetDeviceInfo
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewDeviceInfo = UPNP_OUT_ARG("NewDeviceInfo"); )

	UPNP_USE_HINT( ARG_BIN_LEN(out_NewDeviceInfo), ARG_BIN_DATA(out_NewDeviceInfo) )

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewDeviceInfo = UPNP_OUT_ARG("NewDeviceInfo");

	wfa_GetDeviceInfo(context, ARG_VALUE(out_NewDeviceInfo));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetSTASettings() */
static int
action_GetSTASettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewMessage = UPNP_IN_ARG("NewMessage"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewSTASettings = UPNP_OUT_ARG("NewSTASettings"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewMessage), ARG_BIN_DATA(in_NewMessage) )
	UPNP_USE_HINT( ARG_BIN_LEN(out_NewSTASettings), ARG_BIN_DATA(out_NewSTASettings) )

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewSTASettings = UPNP_OUT_ARG("NewSTASettings");

	return statevar_STASettings(context, service, out_NewSTASettings->statevar, ARG_VALUE(out_NewSTASettings) );
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_PutMessage() */
static int
action_PutMessage
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewInMessage = UPNP_IN_ARG("NewInMessage"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewOutMessage = UPNP_OUT_ARG("NewOutMessage"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewMessage), ARG_BIN_DATA(in_NewMessage->value) )
	UPNP_USE_HINT( ARG_BIN_LEN(out_NewOutMessage), ARG_BIN_DATA(out_NewOutMessage) )

	/* << USER CODE START >> */
	IN_ARGUMENT *in_NewInMessage = UPNP_IN_ARG("NewInMessage");
	OUT_ARGUMENT *out_NewOutMessage = UPNP_OUT_ARG("NewOutMessage");

	wfa_PutMessage(context, ARG_VALUE(in_NewInMessage), ARG_VALUE(out_NewOutMessage));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_PutWLANResponse() */
static int
action_PutWLANResponse
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewMessage = UPNP_IN_ARG("NewMessage"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewWLANEventType = UPNP_IN_ARG("NewWLANEventType"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewWLANEventMAC = UPNP_IN_ARG("NewWLANEventMAC"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewMessage), ARG_BIN_DATA(in_NewMessage) )
	UPNP_USE_HINT( ARG_UI1(in_NewWLANEventType) )
	UPNP_USE_HINT( ARG_STR(in_NewWLANEventMAC) )

	/* << USER CODE START >> */
	IN_ARGUMENT *in_NewMessage = UPNP_IN_ARG("NewMessage");

	wfa_PutWLANResponse(context, ARG_VALUE(in_NewMessage));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_RebootAP() */
static int
action_RebootAP
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewAPSettings = UPNP_IN_ARG("NewAPSettings"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewAPSettings), ARG_BIN_DATA(in_NewAPSettings) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_RebootSTA() */
static int
action_RebootSTA
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewSTASettings = UPNP_IN_ARG("NewSTASettings"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewSTASettings), ARG_BIN_DATA(in_NewSTASettings) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_ResetAP() */
static int
action_ResetAP
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewMessage = UPNP_IN_ARG("NewMessage"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewMessage), ARG_BIN_DATA(in_NewMessage) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_ResetSTA() */
static int
action_ResetSTA
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewMessage = UPNP_IN_ARG("NewMessage"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewMessage), ARG_BIN_DATA(in_NewMessage) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetAPSettings() */
static int
action_SetAPSettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewAPSettings = UPNP_IN_ARG("NewAPSettings"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewAPSettings), ARG_BIN_DATA(in_NewAPSettings) )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetSelectedRegistrar() */
static int
action_SetSelectedRegistrar
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewMessage = UPNP_IN_ARG("NewMessage"); )

	UPNP_USE_HINT( ARG_BIN_LEN(in_NewMessage), ARG_BIN_DATA(in_NewMessage) )

	/* << USER CODE START >> */
	IN_ARGUMENT *in_NewMessage = UPNP_IN_ARG("NewMessage");

	wfa_SetSelectedRegistrar(context, ARG_VALUE(in_NewMessage));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetSTASettings() */
static int
action_SetSTASettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewSTASettings = UPNP_OUT_ARG("NewSTASettings"); )

	UPNP_USE_HINT( ARG_BIN_LEN(out_NewSTASettings), ARG_BIN_DATA(out_NewSTASettings) )

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewSTASettings = UPNP_OUT_ARG("NewSTASettings");

	return statevar_STASettings(context, service, out_NewSTASettings->statevar, ARG_VALUE(out_NewSTASettings));
}
/* >> AUTO GENERATED FUNCTION */


/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

#define	STATEVAR_WLANRESPONSE   0
#define	STATEVAR_WLANEVENTTYPE  1
#define	STATEVAR_INMESSAGE      2
#define	STATEVAR_OUTMESSAGE     3
#define	STATEVAR_APSETTINGS     4
#define	STATEVAR_MESSAGE        5
#define	STATEVAR_STASTATUS      6
#define	STATEVAR_WLANEVENTMAC   7
#define	STATEVAR_DEVICEINFO     8
#define	STATEVAR_STASETTINGS    9
#define	STATEVAR_APSTATUS       10
#define	STATEVAR_WLANEVENT      11

/* State Variable Table */
UPNP_STATE_VAR statevar_x_wfawlanconfig[] =
{
	{0, "WLANResponse",     UPNP_TYPE_BIN_BASE64,   &statevar_WLANResponse,	    0},
	{0, "WLANEventType",    UPNP_TYPE_UI1,          &statevar_WLANEventType,    0},
	{0, "InMessage",        UPNP_TYPE_BIN_BASE64,   &statevar_InMessage,        0},
	{0, "OutMessage",       UPNP_TYPE_BIN_BASE64,   &statevar_OutMessage,       0},
	{0, "APSettings",       UPNP_TYPE_BIN_BASE64,   &statevar_APSettings,       0},
	{0, "Message",          UPNP_TYPE_BIN_BASE64,   &statevar_Message,          0},
	{0, "STAStatus",        UPNP_TYPE_UI1,          &statevar_STAStatus,        1},
	{0, "WLANEventMAC",	    UPNP_TYPE_STR,          &statevar_WLANEventMAC,     0},
	{0, "DeviceInfo",       UPNP_TYPE_BIN_BASE64,   &statevar_DeviceInfo,       0},
	{0, "STASettings",      UPNP_TYPE_BIN_BASE64,   &statevar_STASettings,      0},
	{0, "APStatus",         UPNP_TYPE_UI1,          &statevar_APStatus,         1},
	{0, "WLANEvent",        UPNP_TYPE_BIN_BASE64,   &statevar_WLANEvent,        1},
	{0, 0,                  0,                      0,                          0}
};

/* Action Table */
static ACTION_ARGUMENT arg_in_DelAPSettings [] =
{
	{"NewAPSettings",       UPNP_TYPE_BIN_BASE64,    STATEVAR_APSETTINGS}
};

static ACTION_ARGUMENT arg_in_DelSTASettings [] =
{
	{"NewSTASettings",      UPNP_TYPE_BIN_BASE64,    STATEVAR_STASETTINGS}
};

static ACTION_ARGUMENT arg_in_GetAPSettings [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_out_GetAPSettings [] =
{
	{"NewAPSettings",       UPNP_TYPE_BIN_BASE64,    STATEVAR_APSETTINGS}
};

static ACTION_ARGUMENT arg_out_GetDeviceInfo [] =
{
	{"NewDeviceInfo",       UPNP_TYPE_BIN_BASE64,    STATEVAR_DEVICEINFO}
};

static ACTION_ARGUMENT arg_in_GetSTASettings [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_out_GetSTASettings [] =
{
	{"NewSTASettings",      UPNP_TYPE_BIN_BASE64,    STATEVAR_STASETTINGS}
};

static ACTION_ARGUMENT arg_in_PutMessage [] =
{
	{"NewInMessage",        UPNP_TYPE_BIN_BASE64,    STATEVAR_INMESSAGE}
};

static ACTION_ARGUMENT arg_out_PutMessage [] =
{
	{"NewOutMessage",       UPNP_TYPE_BIN_BASE64,    STATEVAR_OUTMESSAGE}
};

static ACTION_ARGUMENT arg_in_PutWLANResponse [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE},
	{"NewWLANEventType",    UPNP_TYPE_UI1,           STATEVAR_WLANEVENTTYPE},
	{"NewWLANEventMAC",     UPNP_TYPE_STR,           STATEVAR_WLANEVENTMAC}
};

static ACTION_ARGUMENT arg_in_RebootAP [] =
{
	{"NewAPSettings",       UPNP_TYPE_BIN_BASE64,    STATEVAR_APSETTINGS}
};

static ACTION_ARGUMENT arg_in_RebootSTA [] =
{
	{"NewSTASettings",      UPNP_TYPE_BIN_BASE64,    STATEVAR_STASETTINGS}
};

static ACTION_ARGUMENT arg_in_ResetAP [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_in_ResetSTA [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_in_SetAPSettings [] =
{
	{"NewAPSettings",       UPNP_TYPE_BIN_BASE64,    STATEVAR_APSETTINGS}
};

static ACTION_ARGUMENT arg_in_SetSelectedRegistrar [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_out_SetSTASettings [] =
{
	{"NewSTASettings",      UPNP_TYPE_BIN_BASE64,    STATEVAR_STASETTINGS}
};

UPNP_ACTION action_x_wfawlanconfig[] =
{
	{"DelAPSettings",          1,   arg_in_DelAPSettings,         0,   0,                       &action_DelAPSettings},
	{"DelSTASettings",         1,   arg_in_DelSTASettings,        0,   0,                       &action_DelSTASettings},
	{"GetAPSettings",          1,   arg_in_GetAPSettings,         1,   arg_out_GetAPSettings,   &action_GetAPSettings},
	{"GetDeviceInfo",          0,   0,                            1,   arg_out_GetDeviceInfo,   &action_GetDeviceInfo},
	{"GetSTASettings",         1,   arg_in_GetSTASettings,        1,   arg_out_GetSTASettings,  &action_GetSTASettings},
	{"PutMessage",             1,   arg_in_PutMessage,            1,   arg_out_PutMessage,      &action_PutMessage},
	{"PutWLANResponse",        3,   arg_in_PutWLANResponse,       0,   0,                       &action_PutWLANResponse},
	{"RebootAP",               1,   arg_in_RebootAP,              0,   0,                       &action_RebootAP},
	{"RebootSTA",              1,   arg_in_RebootSTA,             0,   0,                       &action_RebootSTA},
	{"ResetAP",                1,   arg_in_ResetAP,               0,   0,                       &action_ResetAP},
	{"ResetSTA",               1,   arg_in_ResetSTA,              0,   0,                       &action_ResetSTA},
	{"SetAPSettings",          1,   arg_in_SetAPSettings,         0,   0,                       &action_SetAPSettings},
	{"SetSTASettings",         0,   0,                            1,   arg_out_SetSTASettings,  &action_SetSTASettings},
	{"SetSelectedRegistrar",   1,   arg_in_SetSelectedRegistrar,  0,   0,                       &action_SetSelectedRegistrar},
	{0,                        0,   0,                            0,   0,                       0}
};
/* >> TABLE END */
