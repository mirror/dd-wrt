/*
 * Broadcom UPnP module, WFADevice.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: WFADevice.c,v 1.10 2008/08/25 08:17:40 Exp $
 */
#include <upnp.h>
#include <WFADevice.h>
#include <eap_defs.h>
#include <ap_upnp_sm.h>

extern UPNP_SCBRCHAIN * get_subscriber_chain(UPNP_CONTEXT *context, UPNP_SERVICE *service);

/* Send data to WPS */
int
wfa_WriteToWPS(UPNP_CONTEXT *context, char *databuf, int datalen, int type)
{
	UPNP_WFACTRL *wfactrl = (UPNP_WFACTRL *)context->focus_ifp->focus_devchain->devctrl;
	UPNP_WPS_CMD cmd;

	struct sockaddr_in to;
	struct iovec iov[2];
	struct msghdr msg;

	/*
	 * Use sendmsg with two iovec.
	 * The first one is cmd and the second iov
	 * points to the bindary to send out.
	 */
	cmd.type = type;
	cmd.length = datalen;

	iov[0].iov_base = (void *)&cmd;
	iov[0].iov_len = UPNP_WPS_CMD_SIZE;
	iov[1].iov_base = (void *)databuf;
	iov[1].iov_len = datalen;

	to.sin_family = AF_INET;
	to.sin_port = htons(UPNP_WPS_PORT + context->focus_ifp->if_instance);
	to.sin_addr.s_addr = inet_addr(UPNP_WPS_ADDR);

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&to;
	msg.msg_namelen = sizeof(to);
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	sendmsg(wfactrl->m_write, &msg, 0);
	return 0;
}

/* Read data from WPS */
int
wfa_ReadFromWPS(UPNP_CONTEXT *context, char *databuf, int *datalen, int type)
{
	UPNP_WFACTRL *wfactrl = (UPNP_WFACTRL *)context->focus_ifp->focus_devchain->devctrl;
	UPNP_WPS_CMD cmd;

	struct timeval tv;
	fd_set  fds;
	int n;
	int bytes;

	struct sockaddr_in from;
	struct iovec iov[2];
	struct msghdr msg;

	time_t end_time;
	int	remain;
	int	len;

retry:
	len = *datalen;
	bytes = 0;
	remain = UPNP_WFA_READ_WPS_TIMEOUT;
	end_time = time(0) + remain;

	while (remain > 0) {
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		FD_ZERO(&fds);
		FD_SET(wfactrl->m_read, &fds);

		n = select(wfactrl->m_read+1, &fds, 0, 0, &tv);
		if (n > 0) {
			/* Selected */
			if (FD_ISSET(wfactrl->m_read, &fds)) {
				/* Prepare message to read */
				memset(&from, 0, sizeof(from));

				iov[0].iov_base = (void *)&cmd;
				iov[0].iov_len = UPNP_WPS_CMD_SIZE;
				iov[1].iov_base = (void *)databuf;
				iov[1].iov_len = len;

				memset(&msg, 0, sizeof(msg));
				msg.msg_name = (void *)&from;
				msg.msg_namelen = sizeof(from);
				msg.msg_iov = iov;
				msg.msg_iovlen = 2;

				/* Read this message */
				bytes = recvmsg(wfactrl->m_read, &msg, 0);
				if (bytes > 0)
					break;

				WFA_DBG("line %d, re-read!", __LINE__);
			}
		}

		/* Update the remaining seconds */
		remain = end_time - time(0);
	}

	if (bytes <= UPNP_WPS_CMD_SIZE) {

		WFA_DBG("Read error!");
		*datalen = 0;
		return 0;
	}

	/* Check if the return command type is what we want */
	len = bytes - UPNP_WPS_CMD_SIZE;
	if (cmd.type != type) {
		WFA_DBG("line %d want type %d, but got type %d, retry again.\n", __LINE__, type, cmd.type);
		goto retry;
	}

	*datalen = len;
	return *datalen;
}

/* Perform the SetSelectedRegistrar action */
int
wfa_SetSelectedRegistrar(UPNP_CONTEXT *context,	UPNP_VALUE *NewMessage)
{
	wfa_WriteToWPS(context,
			NewMessage->val.str,
			NewMessage->len,
			UPNP_WPS_TYPE_SSR);

	return 0;
}

/* Perform the PutMessage action */
int
wfa_PutMessage(UPNP_CONTEXT *context, UPNP_VALUE *NewInMessage, UPNP_VALUE *NewOutMessage)
{
	int	rc;

	/* Send PutMessage request to WPS module */
	wfa_WriteToWPS(context, 
			NewInMessage->val.str,
			NewInMessage->len,
			UPNP_WPS_TYPE_PMR);

	/* Read PugMessage response from WPS Module */
	NewOutMessage->len = sizeof(NewOutMessage->val.str);

	rc = wfa_ReadFromWPS(context,
				NewOutMessage->val.str,
				&NewOutMessage->len,
				UPNP_WPS_TYPE_PMR);
	if (rc <= 0)
		NewOutMessage->len = 0;

	return 0;
}

/* Perform the GetDeviceInfo action */
int
wfa_GetDeviceInfo(UPNP_CONTEXT *context, UPNP_VALUE *NewDeviceInfo)
{
	int rc;
	UPNP_WFACTRL *wfactrl = (UPNP_WFACTRL *)context->focus_ifp->focus_devchain->devctrl;


	if (wfactrl->m_devInfo && wfactrl->m_devInfoLen > 0) {
		memcpy(NewDeviceInfo->val.str, wfactrl->m_devInfo, wfactrl->m_devInfoLen);
		NewDeviceInfo->len = wfactrl->m_devInfoLen;
	}
	else {
		/* Send GetDeviceInfo request to WPS module */
		wfa_WriteToWPS(context, NULL, 0, UPNP_WPS_TYPE_GDIR);

		/* Read GetDeviceInfo response from WPS Module */
		NewDeviceInfo->len = sizeof(NewDeviceInfo->val.str);
		rc = wfa_ReadFromWPS(context,
					NewDeviceInfo->val.str,
					&NewDeviceInfo->len,
					UPNP_WPS_TYPE_GDIR);
		if (rc <= 0)
			NewDeviceInfo->len = 0;
	}

	return 0;
}

/* Perform the PutWLANResponse action */
int
wfa_PutWLANResponse(UPNP_CONTEXT *context, UPNP_VALUE *NewMessage)
{
	wfa_WriteToWPS(context,
			NewMessage->val.str,
			NewMessage->len,
			UPNP_WPS_TYPE_PWR);

	return 0;
}

/* Close wfa socket */
int
wfa_free(UPNP_CONTEXT *context)
{
	UPNP_WFACTRL *wfactrl = (UPNP_WFACTRL *)context->focus_ifp->focus_devchain->devctrl;

	if (wfactrl) {
		if (wfactrl->m_read >= 0)
			close(wfactrl->m_read);

		if (wfactrl->m_write >= 0)
			close(wfactrl->m_write);

		if (wfactrl->m_devInfo)
			free(wfactrl->m_devInfo);
	}

	/* Clear subscribe number */
	upnp_osl_update_wfa_subc_num(context->focus_ifp->if_instance, 0);

	return 0;
}

/* Open wfa socket */
int
wfa_init(UPNP_CONTEXT *context)
{
	UPNP_DEVCHAIN	*devchain;
	UPNP_WFACTRL 	*wfactrl = 0;
	unsigned short port;
	struct in_addr addr;
	
	/* Allocate soft control for WFA */
	devchain = context->focus_ifp->focus_devchain;
	devchain->devctrl = malloc(sizeof(*wfactrl));
	if (devchain->devctrl == 0)
		return -1;

	wfactrl = (UPNP_WFACTRL *)devchain->devctrl;
	wfactrl->m_read = -1;
	wfactrl->m_write = -1;
	wfactrl->m_devInfo = NULL;
	wfactrl->m_devInfoLen = 0;

	port = UPNP_WFA_PORT + context->focus_ifp->if_instance;
	addr.s_addr = inet_addr(UPNP_WFA_ADDR);
	wfactrl->m_read = oslib_udp_socket(addr, port);
	if (wfactrl->m_read < 0) {
		WFA_DBG("Create m_read");
		goto err;
	}

	/* Create m_write for send data to WPS */
	wfactrl->m_write = socket(AF_INET, SOCK_DGRAM, 0);
	if (wfactrl->m_write < 0) {
		WFA_DBG("Create m_write");
		close(wfactrl->m_read);
		goto err;
	}

	return 0;

err:
	wfa_free(context);
	return -1;
}


/*
 * WARNNING: PLEASE IMPLEMENT YOUR CODES AFTER 
 *          "<< USER CODE START >>"
 * AND DON'T REMOVE TAG :
 *          "<< AUTO GENERATED FUNCTION: "
 *          ">> AUTO GENERATED FUNCTION"
 *          "<< USER CODE START >>"
 */

/* << AUTO GENERATED FUNCTION: WFADevice_common_init() */
int WFADevice_common_init
(
	UPNP_CONTEXT *context
)
{
	/* << USER CODE START >> */
	WFADevice.attach_mode = DEVICE_ATTACH_DYNAMICALLY;
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_open() */
int
WFADevice_open(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	if (wfa_init(context) != 0)
		return -1;

	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_close() */
int
WFADevice_close(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	wfa_free(context);
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_request() */
int
WFADevice_request(UPNP_CONTEXT *context, void *cmd)
{
	/* << USER CODE START >> */
	UPNP_REQUEST *request = (UPNP_REQUEST *)cmd;
	UPNP_WFACTRL *wfactrl = (UPNP_WFACTRL *)context->focus_ifp->focus_devchain->devctrl;
	UPNP_VALUE *value = &request->var[0].value;

	switch (request->cmd) {
	case UPNP_REQ_SET_DEVINFO:
		if (wfactrl->m_devInfo) {
			free(wfactrl->m_devInfo);
			wfactrl->m_devInfo = NULL;
		}

		wfactrl->m_devInfoLen = value->len;
		wfactrl->m_devInfo = malloc(wfactrl->m_devInfoLen);
		if (wfactrl->m_devInfo) {
			memcpy(wfactrl->m_devInfo, value->val.data, wfactrl->m_devInfoLen);
		}
		break;

	default:
		break;
	}

	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_timeout() */
int
WFADevice_timeout(UPNP_CONTEXT *context, time_t now)
{
	/* << USER CODE START >> */
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_notify() */
int
WFADevice_notify(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	/* << USER CODE START >> */
	UPNP_SUBSCRIBER *subscriber;
	UPNP_SCBRCHAIN *scbrchain;
	int num = 0;

	/* count subscriber number */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain) {
		subscriber = scbrchain->subscriberlist;
		while (subscriber) {
			subscriber = subscriber->next;
			num ++;
		}
	}

	upnp_osl_update_wfa_subc_num(context->focus_ifp->if_instance, num);

	return 0;
}
/* >> AUTO GENERATED FUNCTION */
