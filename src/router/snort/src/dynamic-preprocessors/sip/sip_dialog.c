/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 * Provides convenience functions for dialog management
 * Dialog management is the central part of SIP call flow analysis
 *
 * 3/15/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "sf_types.h"
#include "sip_dialog.h"
#include "sip_parser.h"
#include "sip_debug.h"
#include "sf_ip.h"
#include "spp_sip.h"
#include "session_api.h"
#include "stream_api.h"
#include <assert.h>


static void SIP_updateMedias(SIP_MediaSession *, SIP_MediaList *);
static int SIP_compareMedias(SIP_MediaDataList , SIP_MediaDataList );
static int SIP_checkMediaChange(SIPMsg *sipMsg, SIP_DialogData *dialog);
static int SIP_processRequest(SIPMsg *, SIP_DialogData *, SIP_DialogList *, SFSnortPacket *);
static int SIP_processInvite(SIPMsg *, SIP_DialogData *, SIP_DialogList *);
static int SIP_processACK(SIPMsg *, SIP_DialogData *, SIP_DialogList *, SFSnortPacket *);
static int SIP_processResponse(SIPMsg *, SIP_DialogData *, SIP_DialogList *, SFSnortPacket *);
static int SIP_ignoreChannels( SIP_DialogData *, SFSnortPacket *p);
static SIP_DialogData* SIP_addDialog(SIPMsg *, SIP_DialogData *, SIP_DialogList *);
static int SIP_deleteDialog(SIP_DialogData *, SIP_DialogList *);
#ifdef DEBUG_MSGS
void SIP_displayMedias(SIP_MediaList *dList);
#endif


/********************************************************************
 * Function: SIP_processRequest()
 *
 *  Based on the new received sip request message, update the dialog information.
 *  Note: dialog is created through dialog
 * Arguments:
 *  SIPMsg *        - sip request message
 *  SIP_DialogData* - dialog to be updated,
 *  SFSnortPacket*  - the packet
 *
 * Returns:
 *  SIP_SUCCESS: request message has been processed correctly
 *  SIP_FAILURE: request message has not been processed correctly
 ********************************************************************/
static int SIP_processRequest(SIPMsg *sipMsg, SIP_DialogData *dialog, SIP_DialogList *dList, SFSnortPacket *p)
{

	SIPMethodsFlag methodFlag;
    int ret = SIP_SUCCESS;

    assert (NULL != sipMsg);

	/*If dialog not exist, create one */
    if((NULL == dialog)&&(SIP_METHOD_CANCEL != sipMsg->methodFlag))
    {
    	dialog = SIP_addDialog(sipMsg, dList->head, dList);
    }

    methodFlag = sipMsg->methodFlag;
    sip_stats.requests[TOTAL_REQUESTS]++;
    if (methodFlag > 0)
    	sip_stats.requests[methodFlag]++;
    switch (methodFlag)
	{
    	case SIP_METHOD_INVITE:

    		ret = SIP_processInvite(sipMsg, dialog, dList);

    		break;

    	case SIP_METHOD_CANCEL:

    		if (NULL == dialog)
    		    return SIP_FAILURE;
    	   /*dialog can be deleted in the early state*/
    		if((SIP_DLG_EARLY == dialog->state)||(SIP_DLG_INVITING == dialog->state)
    				|| (SIP_DLG_CREATE == dialog->state))
    		   SIP_deleteDialog(dialog, dList);

    	    break;

    	case SIP_METHOD_ACK:

    		SIP_processACK(sipMsg, dialog, dList, p);

    		break;

    	case SIP_METHOD_BYE:

    		if(SIP_DLG_ESTABLISHED == dialog->state)
    			dialog->state = SIP_DLG_TERMINATING;
    		break;

    	default:

    		break;

	}
	return ret;
}

/********************************************************************
 * Function: SIP_processInvite()
 *
 *  Based on the new received sip invite request message, update the dialog information.
 *  Note: dialog is created through dialog
 * Arguments:
 *  SIPMsg *        - sip request message
 *  SIP_DialogData* - dialog to be updated,
 *   SIP_DialogList*- dialog list
 * Returns:
 *  SIP_SUCCESS:
 *  SIP_FAILURE:
 ********************************************************************/
static int SIP_processInvite(SIPMsg *sipMsg, SIP_DialogData *dialog, SIP_DialogList *dList)
{

	int ret = SIP_SUCCESS;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Processing invite, dialog state %d \n", dialog->state ););

	if (NULL == dialog)
		return SIP_FAILURE;

	/*Check for the invite replay attack: authenticated invite without challenge*/
	// check whether this invite has authorization information
    if ((SIP_DLG_AUTHENCATING != dialog->state) && (NULL != sipMsg ->authorization))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Dialog state code: %u\n",
        		dialog->status_code));

    	ALERT(SIP_EVENT_AUTH_INVITE_REPLAY_ATTACK,SIP_EVENT_AUTH_INVITE_REPLAY_ATTACK_STR);
    	return SIP_FAILURE;

    }
    if (SIP_DLG_ESTABLISHED == dialog->state)
	{
		/* this is the case of re-INVITE*/
		// create a temporary new dialog before the current dialog
		dialog = SIP_addDialog(sipMsg, dialog, dList);
		dialog->state =  SIP_DLG_REINVITING;
		return SIP_SUCCESS;
	}
    /*Check for the fake busy attack:  change media session before dialog established*/
    else if((SIP_DLG_INVITING == dialog->state) || (SIP_DLG_EARLY == dialog->state)
    		|| (SIP_DLG_REINVITING == dialog->state)|| (SIP_DLG_AUTHENCATING == dialog->state))
    {
    	ret = SIP_checkMediaChange(sipMsg, dialog);
    	if (SIP_FAILURE == ret)
    	   ALERT(SIP_EVENT_AUTH_INVITE_DIFF_SESSION,SIP_EVENT_AUTH_INVITE_DIFF_SESSION_STR);
    	SIP_updateMedias(sipMsg->mediaSession, &dialog->mediaSessions);
    }
    else if (SIP_DLG_TERMINATED == dialog->state)
    {
    	SIP_updateMedias(sipMsg->mediaSession, &dialog->mediaSessions);
    }
    dialog->state = SIP_DLG_INVITING;
    return ret;
}

/********************************************************************
 * Function: SIP_processACK()
 *
 *  Based on the new received sip ACK request message, update the dialog information.
 *  Note: dialog is created through dialog
 * Arguments:
 *  SIPMsg *        - sip request message
 *  SIP_DialogData* - dialog to be updated,
 *  SIP_DialogList* - dialog list
 *  SFSnortPacket*  - the packet
 * Returns:
 *  SIP_SUCCESS:
 *  SIP_FAILURE:
 ********************************************************************/
static int SIP_processACK(SIPMsg *sipMsg, SIP_DialogData *dialog, SIP_DialogList *dList, SFSnortPacket *p)
{
	if (NULL == dialog)
		return SIP_FAILURE;

	if (SIP_DLG_ESTABLISHED == dialog->state)
	{
		if ((SIP_METHOD_INVITE == dialog->creator)&&(SIP_checkMediaChange(sipMsg, dialog) == SIP_FAILURE))
		{
			SIP_updateMedias(sipMsg->mediaSession, &dialog->mediaSessions);
			SIP_ignoreChannels(dialog, p);
            sipMsg->mediaUpdated = 1;
		}
	}
	return SIP_SUCCESS;
}
/********************************************************************
 * Function: SIP_processResponse()
 *
 *  Based on the new received sip response message, update the dialog information.
 *
 * Arguments:
 *  SIPMsg *        - sip response message
 *  SIP_DialogData* - dialog to be updated,
 *  SFSnortPacket*  - the packet
 *
 * Returns:
 *  SIP_SUCCESS:
 *  SIP_FAILURE:
 ********************************************************************/
static int SIP_processResponse(SIPMsg *sipMsg, SIP_DialogData *dialog, SIP_DialogList *dList, SFSnortPacket *p)
{

	int statusType;
	SIP_DialogData *currDialog = dialog;

	assert (NULL != sipMsg);

	statusType = sipMsg->status_code / 100;
	sip_stats.responses[TOTAL_RESPONSES]++;
	if (statusType < NUM_OF_RESPONSE_TYPES)
		sip_stats.responses[statusType]++;

	if(NULL == dialog)
		return SIP_FAILURE;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Processing response, dialog state %d \n", dialog->state ););

	if(sipMsg->status_code > 0)
		  dialog->status_code = sipMsg->status_code;


	switch (statusType)
	{
    	case 0:
    		break;
    	case RESPONSE1XX:

    		if (SIP_DLG_CREATE == currDialog->state)
    		   currDialog->state = SIP_DLG_EARLY;

            /*183 session progress can have SDP body for which we need to set mediaUpdated flag so that AppId can create pinhole for RTP/RTCP media session*/
			if ((183 == sipMsg->status_code) && (SIP_METHOD_INVITE == currDialog->creator) &&
					(SIP_checkMediaChange(sipMsg, dialog) == SIP_FAILURE))
			{
				SIP_updateMedias(sipMsg->mediaSession, &dialog->mediaSessions);
				SIP_ignoreChannels(currDialog, p);
				sipMsg->mediaUpdated = 1;
			}
			else
			{
				SIP_updateMedias(sipMsg->mediaSession, &dialog->mediaSessions);
			}

			break;
		case RESPONSE2XX:

			if (SIP_DLG_REINVITING == currDialog->state)
			{
				SIP_deleteDialog(currDialog->nextD, dList);
				if (SIP_checkMediaChange(sipMsg, dialog) == SIP_FAILURE)
				{
    				SIP_updateMedias(sipMsg->mediaSession, &dialog->mediaSessions);
    				SIP_ignoreChannels(currDialog, p);
                    sipMsg->mediaUpdated = 1;
    			}
    			currDialog->state = SIP_DLG_ESTABLISHED;
    		}
    		else if (SIP_DLG_TERMINATING == currDialog->state)
    		{
    			SIP_deleteDialog(currDialog, dList);
    			return SIP_SUCCESS;
    		}
    		else
    		{
    			if ((SIP_METHOD_INVITE == currDialog->creator)&&
    					(SIP_checkMediaChange(sipMsg, dialog) == SIP_FAILURE))
    			{
    				SIP_updateMedias(sipMsg->mediaSession, &dialog->mediaSessions);
    				SIP_ignoreChannels(currDialog, p);
                    sipMsg->mediaUpdated = 1;
    			}
    			currDialog->state = SIP_DLG_ESTABLISHED;
    		}
    		break;
    	case RESPONSE3XX:
    	case RESPONSE4XX:
    	case RESPONSE5XX:
    	case RESPONSE6XX:

    		// If authentication is required
    		if((401 == sipMsg->status_code) || (407 == sipMsg->status_code))
    		{
    			currDialog->state = SIP_DLG_AUTHENCATING;
    		}
    		/*Failed re-Invite will resume to the original state*/
    		else if(SIP_DLG_REINVITING == currDialog->state)
    		{
    			SIP_deleteDialog(currDialog, dList);
    		}
    		else
    			currDialog->state = SIP_DLG_TERMINATED;

    		break;

    	default:
    	    break;
	}


	return SIP_SUCCESS;

}
/********************************************************************
 * Function: SIP_checkMediaChange()
 *
 *  Based on the new received sip invite request message, check whether SDP has been changed
 *
 * Arguments:
 *  SIPMsg *        - sip request message
 *  SIP_DialogData* - dialog to be updated,
 *
 * Returns:
 *  SIP_SUCCESS: media not changed
 *  SIP_FAILURE: media changed
 ********************************************************************/
static int SIP_checkMediaChange(SIPMsg *sipMsg, SIP_DialogData *dialog)
{
	SIP_MediaSession *medias;

	// Compare the medias (SDP part)
	if (NULL == sipMsg->mediaSession)
		return SIP_SUCCESS;

	medias = dialog->mediaSessions;
	while(NULL != medias)
	{
		if (sipMsg->mediaSession->sessionID == medias->sessionID)
			break;
		medias = medias->nextS;
	}

	if (NULL == medias)
	{
		// Can't find the media session by ID, SDP has been changed.
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Can't find the media data, ID: %u\n", sipMsg->mediaSession->sessionID ););

		return SIP_FAILURE;
	}
	// The media content has been changed
	if (0 != SIP_compareMedias(medias->medias, sipMsg->mediaSession->medias))
	{
		// Can't find the media session by ID, SDP has been changed.
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "The media data is different!\n"););
		return SIP_FAILURE;
	}
	return SIP_SUCCESS;
}
/********************************************************************
 * Function: SIP_ignoreChannels
 *
 * Ignore the channels in the current dialog: for a dialog,there will be media
 * sessions, one from each side of conversation
 *
 * Arguments:
 *  SIP_DialogData * - the current dialog
 *
 *
 * Returns:
 *   SIP_SUCCESS: the channel has been ignored
 *   SIP_FAILURE: the channel has not been ignored
 *
 ********************************************************************/
static int SIP_ignoreChannels( SIP_DialogData *dialog, SFSnortPacket *p)
{
	SIP_MediaData *mdataA,*mdataB;

	if (0 == sip_eval_config->ignoreChannel)
    	return SIP_FAILURE;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Ignoring the media data in Dialog: %u\n", dialog->dlgID.callIdHash););
	// check the first media session
	if (NULL == dialog->mediaSessions)
		return SIP_FAILURE;
	// check the second media session
	if (NULL == dialog->mediaSessions->nextS)
		return SIP_FAILURE;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Ignoring the media sessions ID: %u and %u\n",
			dialog->mediaSessions->sessionID, dialog->mediaSessions->nextS->sessionID););
	mdataA = dialog->mediaSessions->medias;
	mdataB = dialog->mediaSessions->nextS->medias;
	sip_stats.ignoreSessions++;
	while((NULL != mdataA)&&(NULL != mdataB))
    {
        void *ssn;
    	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Ignoring channels Source IP: %s Port: %u\n",
    			sfip_to_str(&mdataA->maddress), mdataA->mport););
    	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Ignoring channels Destine IP: %s Port: %u\n",
    			sfip_to_str(&mdataB->maddress), mdataB->mport););
    	/* Call into Streams to mark data channel as something to ignore. */
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
       uint32_t cid = GET_SFOUTER_IPH_PROTOID(p, pkt_header);

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
      uint16_t sAsId = p->pkt_header->address_space_id_src;
      uint16_t dAsId = p->pkt_header->address_space_id_dst;

      ssn = _dpd.sessionAPI->get_session_ptr_from_ip_port(&mdataA->maddress,mdataA->mport,
                                                          &mdataB->maddress,
                                                          mdataB->mport,
                                                          IPPROTO_UDP, 0, 0,
                                                          sAsId, dAsId, cid);                           
#else
      ssn = _dpd.sessionAPI->get_session_ptr_from_ip_port(&mdataA->maddress,mdataA->mport, &mdataB->maddress,
                                                          mdataB->mport, IPPROTO_UDP, 0, 0, 
                                                          p->pkt_header->address_space_id, cid);
#endif
#else
      ssn = _dpd.sessionAPI->get_session_ptr_from_ip_port(&mdataA->maddress,mdataA->mport, &mdataB->maddress,
                                                          mdataB->mport, IPPROTO_UDP, 0, 0, 0, cid);
#endif
#else /* No carrier id support */
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
      uint16_t sAsId = p->pkt_header->address_space_id_src;
      uint16_t dAsId = p->pkt_header->address_space_id_dst;

      ssn = _dpd.sessionAPI->get_session_ptr_from_ip_port(&mdataA->maddress,mdataA->mport,
                                                          &mdataB->maddress,
                                                          mdataB->mport,
                                                          IPPROTO_UDP, 0, 0,
                                                          sAsId, dAsId);
#else
      ssn = _dpd.sessionAPI->get_session_ptr_from_ip_port(&mdataA->maddress,mdataA->mport, &mdataB->maddress,
                                                          mdataB->mport, IPPROTO_UDP, 0, 0,
                                                          p->pkt_header->address_space_id);
#endif
#else
      ssn = _dpd.sessionAPI->get_session_ptr_from_ip_port(&mdataA->maddress,mdataA->mport, &mdataB->maddress,
                                                          mdataB->mport, IPPROTO_UDP, 0, 0, 0);
#endif
#endif
    	if ( _dpd.sessionAPI->is_session_verified( ssn ) )
    	{
    	    _dpd.sessionAPI->set_ignore_direction(ssn, SSN_DIR_BOTH);
    	}
    	else
    	{
    	    _dpd.sessionAPI->ignore_session(p, &mdataA->maddress, mdataA->mport, &mdataB->maddress,
    	                            mdataB->mport, IPPROTO_UDP, PP_SIP, SSN_DIR_BOTH, 0 /* Not permanent */,
                                    &p->expectedSession );
    	}
    	sip_stats.ignoreChannels++;
    	mdataA = mdataA->nextM;
    	mdataB = mdataB->nextM;
    }
	return SIP_SUCCESS;

}
/********************************************************************
 * Function: SIP_compareMedias
 *
 * Compare two media list
 *
 * Arguments:
 *  SIPMsg * - the message used to create a dialog
 *  SIP_DialogData * - the current dialog location
 *  SIP_DialogList * - the dialogs to be added.
 *
 *
 * Returns:
 *   1: not the same
 *   0: the same
 *
 ********************************************************************/
static int SIP_compareMedias(SIP_MediaDataList mlistA, SIP_MediaDataList mlistB )
{
	SIP_MediaData *mdataA,*mdataB;
	mdataA = mlistA;
	mdataB = mlistB;
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Compare the media data \n"););
	while((NULL != mdataA) && (NULL != mdataB))
	{
		if(sfip_compare(&mdataA->maddress, &mdataB->maddress) != SFIP_EQUAL)
			break;
		if((mdataA->mport != mdataB->mport)|| (mdataA->numPort != mdataB->numPort))
			break;
        mdataA = mdataA->nextM;
        mdataB = mdataB->nextM;
	}
	if((NULL == mdataA) && (NULL == mdataB))
	    return 0;
	else
		return 1;
}
/********************************************************************
 * Function: SIP_updateMedias()
 *
 *  Based on the new received media session information, update the media list.
 *  If not in the current list, created one and add it to the head.
 *
 * Arguments:
 *  SIP_MediaSession*  - media session
 *  SIP_MediaList*     - media session list to be updated,
 *
 * Returns:
 *
 ********************************************************************/
static void SIP_updateMedias(SIP_MediaSession *mSession, SIP_MediaList *dList)
{
   	SIP_MediaSession *currSession, *preSession = NULL;

    if(NULL == mSession)
    	return;
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Updating session id: %u\n",
   		mSession->sessionID));
    mSession->savedFlag = SIP_SESSION_SAVED;
   	// Find out the media session based on session id
   	currSession = *dList;
    while(NULL != currSession)
    {
    	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Session id: %u\n",
    			currSession->sessionID));
    	if(currSession->sessionID == mSession->sessionID)
    	{
    		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Found Session id: %u\n",
    		    			currSession->sessionID));
    		break;
    	}
    	preSession = currSession;
    	currSession = currSession->nextS;
    }
    // if this is a new session data, add to the list head
    if (NULL == currSession)
    {
        mSession->nextS = *dList;
        *dList = mSession;
        DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Add Session id: %u\n",
                mSession->sessionID));
    }
    else
    {
        // if this session needs to be updated
        DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Update Session id: %u\n",
                mSession->sessionID));
        mSession->nextS = currSession->nextS;
        // if this is the header, update the new header
        if (NULL == preSession)
            *dList = mSession;
        else
            preSession->nextS = mSession;

        // Clear the old session
        currSession->nextS = NULL;
        sip_freeMediaSession(currSession);
    }
    // Display the final media session
#ifdef DEBUG_MSGS
    SIP_displayMedias(dList);
#endif
	return;
}
#ifdef DEBUG_MSGS
void SIP_displayMedias(SIP_MediaList *dList)
{
	SIP_MediaSession *currSession;
	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Updated Session information------------\n"));
	currSession = *dList;
    while(NULL != currSession)
    {
    	SIP_MediaData *mdata;
    	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Session id: %u\n", currSession->sessionID));
        mdata =  currSession->medias;
    	while(NULL != mdata)
    	{
    		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Media IP: %s, port: %u, number of ports %u\n",
    				sfip_to_str(&mdata->maddress), mdata->mport, mdata->numPort));
    		mdata = mdata->nextM;
    	}
    	currSession = currSession->nextS;
    }
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "End  of Session information------------\n"));
}
#endif
/********************************************************************
 * Function: SIP_addDialog
 *
 * Add a sip dialog before the current dialog
 *
 * Arguments:
 *  SIPMsg * - the message used to create a dialog
 *  SIP_DialogData * - the current dialog location
 *  SIP_DialogList * - the dialogs to be added.
 *
 *
 * Returns: None
 *
 ********************************************************************/
static SIP_DialogData* SIP_addDialog(SIPMsg *sipMsg, SIP_DialogData *currDialog, SIP_DialogList *dList)
{
	SIP_DialogData* dialog;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Add Dialog id: %u, From: %u, To: %u, status code: %u\n",
			sipMsg->dlgID.callIdHash,sipMsg->dlgID.fromTagHash,sipMsg->dlgID.toTagHash, sipMsg->status_code));

	sip_stats.dialogs++;

	dialog = (SIP_DialogData *) _dpd.snortAlloc(1, sizeof(SIP_DialogData), PP_SIP,
                                                    PP_MEM_CATEGORY_SESSION);
	if (NULL == dialog)
		return NULL;

	// Add to the head
	dialog->nextD = currDialog;
	if(NULL != currDialog)
	{
	    dialog->prevD = currDialog->prevD;
	    if (NULL != currDialog->prevD)
	    	currDialog->prevD->nextD = dialog;
	    else
	    	dList->head = dialog;  // become the head
	    currDialog->prevD = dialog;
	}
	else
	{
		// The first dialog
		dialog->prevD = NULL;
		dList->head = dialog;
	}
	dialog->dlgID = sipMsg->dlgID;
	dialog->creator = sipMsg->methodFlag;
	dialog->state = SIP_DLG_CREATE;

	SIP_updateMedias(sipMsg->mediaSession, &dialog->mediaSessions);
    dList->num_dialogs++;
	return dialog;

}
/********************************************************************
 * Function: SIP_deleteDialog
 *
 * Delete a sip dialog from the list
 *
 * Arguments:
 *  SIP_DialogData * - the current dialog to be deleted
 *  SIP_DialogList * - the dialog list.
 *
 * Returns: None
 *
 ********************************************************************/
static int SIP_deleteDialog(SIP_DialogData *currDialog, SIP_DialogList *dList)
{
    if ((NULL == currDialog)||(NULL == dList))
    	return SIP_FAILURE;

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Delete Dialog id: %u, From: %u, To: %u \n",
    		currDialog->dlgID.callIdHash,currDialog->dlgID.fromTagHash,currDialog->dlgID.toTagHash));
    // If this is the header
    if(NULL ==  currDialog->prevD)
    {
    	if(NULL != currDialog->nextD)
    		currDialog->nextD->prevD = NULL;
    	dList->head = currDialog->nextD;
    }
    else
    {
    	currDialog->prevD->nextD = currDialog->nextD;
    	if(NULL != currDialog->nextD)
    		currDialog->nextD->prevD = currDialog->prevD;
    }
    sip_freeMediaList(currDialog->mediaSessions);
    _dpd.snortFree(currDialog, sizeof(SIP_DialogData), PP_SIP, PP_MEM_CATEGORY_SESSION);
    if( dList->num_dialogs > 0)
        dList->num_dialogs--;
    return SIP_SUCCESS;
}

/*********************************************************************
 * Update appId sip detector with parsed SIP message and dialog
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet structure
 *  SIPMsg        * - pointer to parserd SIP messgage
 *  SIPData       * - pointer to SIP session
 *
 * Returns:
 *  None
 *
 *********************************************************************/
static void sip_update_appid(SFSnortPacket *p, const SIPMsg *sipMsg, const SIP_DialogData *dialog)
{
    SipHeaders hdrs;
    SipDialog        dlg;
    SipEventData sipEventData;

    hdrs.callid = sipMsg->call_id;
    hdrs.callidLen = sipMsg->callIdLen;
    hdrs.methodFlag = sipMsg->methodFlag;

    hdrs.userAgent = sipMsg->userAgent;
    hdrs.userAgentLen = sipMsg->userAgentLen;
    hdrs.server = sipMsg->server;
    hdrs.serverLen = sipMsg->serverLen;
    hdrs.userName = sipMsg->userName;
    hdrs.userNameLen = sipMsg->userNameLen;
    hdrs.from = sipMsg->from;
    hdrs.fromLen= sipMsg->fromLen;

    sipEventData.headers = &hdrs;

    if (dialog)
    {
        dlg.state = dialog->state;
        dlg.mediaSessions = dialog->mediaSessions;
        dlg.mediaUpdated = sipMsg->mediaUpdated;
        sipEventData.dialog = &dlg;

    }
    else
    {
        sipEventData.dialog = NULL;
    }

    sipEventData.packet = p;

    if (_dpd.streamAPI->service_event_publish(PP_SIP, p->stream_session, SIP_EVENT_TYPE_SIP_DIALOG, &sipEventData) == false)
        _dpd.errMsg("failed to publish to SIP_DIALOG\n");

}

/********************************************************************
 * Function: SIP_updateDialog()
 *
 *  Based on the new received sip message, update the dialog information.
 *  If not in the current list, created one and add it to the head.
 *
 * Arguments:
 *  SIPMsg *        - sip message
 *  SIP_DialogList* - dialog list to be updated,
 *
 * Returns:
 *  SIP_SUCCESS: dialog has been updated
 *  SIP_FAILURE: dialog has not been updated
 ********************************************************************/
int SIP_updateDialog(SIPMsg *sipMsg, SIP_DialogList *dList, SFSnortPacket *p)
{
   	SIP_DialogData* dialog;
   	SIP_DialogData* oldDialog = NULL;
   	int ret;

   	if ((NULL == sipMsg)||(0 == sipMsg->dlgID.callIdHash))
   		return SIP_FAILURE;

   	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Updating Dialog id: %u, From: %u, To: %u\n",
   			sipMsg->dlgID.callIdHash,sipMsg->dlgID.fromTagHash,sipMsg->dlgID.toTagHash));

   	dialog = dList->head;

   	/*Find out the dialog in the dialog list*/

   	while(NULL != dialog)
   	{
   		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Dialog id: %u, From: %u, To: %u\n",
   				dialog->dlgID.callIdHash,dialog->dlgID.fromTagHash,dialog->dlgID.toTagHash));
   		if (sipMsg->dlgID.callIdHash == dialog->dlgID.callIdHash)
   		{
   			DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Found Dialog id: %u, From: %u, To: %u\n",
   			   				dialog->dlgID.callIdHash,dialog->dlgID.fromTagHash,dialog->dlgID.toTagHash));
   			break;

   		}
   		oldDialog = dialog;
   		dialog = dialog->nextD;
   	}

   	/*If the number of dialogs exceeded, release the oldest one*/
   	if((dList->num_dialogs >= sip_eval_config->maxNumDialogsInSession) && (!dialog))
   	{
   	    ALERT(SIP_EVENT_MAX_DIALOGS_IN_A_SESSION, SIP_EVENT_MAX_DIALOGS_IN_A_SESSION_STR);
   	    SIP_deleteDialog(oldDialog, dList);
   	}

   	/*Update the  dialog information*/

   	if (sipMsg->status_code == 0)
   	    ret = SIP_processRequest(sipMsg, dialog, dList, p);
   	else if (sipMsg->status_code > 0)
   	    ret = SIP_processResponse(sipMsg, dialog, dList, p);
   	else
   	    ret = SIP_FAILURE;

   	for (dialog = dList->head;
   	        dialog;
   	        dialog = dialog->nextD)
   	{
   	    if (sipMsg->dlgID.callIdHash == dialog->dlgID.callIdHash)
   	        break;
   	}

   	sip_update_appid(p, sipMsg, dialog);


	return ret;
}

/********************************************************************
 * Function: sip_freeDialogs
 *
 * Frees a sip dialog
 *
 * Arguments:
 *  SIP_DialogList
 *      The dialogs to free.
 *
 * Returns: None
 *
 ********************************************************************/
void sip_freeDialogs (SIP_DialogList *list)
{
	SIP_DialogData *nextNode;
	SIP_DialogData *curNode = list->head;

	while (NULL != curNode)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "*Clean Dialog creator: 0x%x, id: %u, From: %u, To: %u, State: %d\n",
				curNode->creator, curNode->dlgID.callIdHash,curNode->dlgID.fromTagHash,curNode->dlgID.toTagHash,curNode->state));
		nextNode = curNode->nextD;
		sip_freeMediaList(curNode->mediaSessions);
		_dpd.snortFree(curNode, sizeof(SIP_DialogData), PP_SIP,
                               PP_MEM_CATEGORY_SESSION);
		curNode = nextNode;
	}

}
