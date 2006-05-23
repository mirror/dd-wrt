/*
 *   fs/cifs/cifssmb.c
 *
 *   Copyright (C) International Business Machines  Corp., 2002,2003
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   Contains the routines for constructing the SMB PDUs themselves
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

 /* SMB/CIFS PDU handling routines here - except for leftovers in connect.c   */
 /* These are mostly routines that operate on a pathname, or on a tree id     */
 /* (mounted volume), but there are eight handle based routines which must be */
 /* treated slightly different for reconnection purposes since we never want  */
 /* to reuse a stale file handle and the caller knows the file handle */

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/vfs.h>
#include <asm/uaccess.h>
#include "cifspdu.h"
#include "cifsglob.h"
#include "cifsproto.h"
#include "cifs_unicode.h"
#include "cifs_debug.h"

#ifdef CONFIG_CIFS_POSIX
static struct {
	int index;
	char *name;
} protocols[] = {
	{CIFS_PROT, "\2NT LM 0.12"}, 
	{CIFS_PROT, "\2POSIX 2"},
	{BAD_PROT, "\2"}
};
#else
static struct {
	int index;
	char *name;
} protocols[] = {
	{CIFS_PROT, "\2NT LM 0.12"}, 
	{BAD_PROT, "\2"}
};
#endif


/* Mark as invalid, all open files on tree connections since they
   were closed when session to server was lost */
static void mark_open_files_invalid(struct cifsTconInfo * pTcon)
{
	struct cifsFileInfo *open_file = NULL;
	struct list_head * tmp;
	struct list_head * tmp1;

/* list all files open on tree connection and mark them invalid */
	write_lock(&GlobalSMBSeslock);
	list_for_each_safe(tmp, tmp1, &pTcon->openFileList) {
		open_file = list_entry(tmp,struct cifsFileInfo, tlist);
		if(open_file) {
			open_file->invalidHandle = TRUE;
		}
	}
	write_unlock(&GlobalSMBSeslock);
	/* BB Add call to invalidate_inodes(sb) for all superblocks mounted to this tcon */
}

static int
smb_init(int smb_command, int wct, struct cifsTconInfo *tcon,
	 void **request_buf /* returned */ ,
	 void **response_buf /* returned */ )
{
	int rc = 0;
	int timeout = 10 * HZ;

	/* SMBs NegProt, SessSetup, uLogoff do not have tcon yet so
	   check for tcp and smb session status done differently
	   for those three - in the calling routine */
	if(tcon) {
		if((tcon->ses) && (tcon->ses->server)){
			struct nls_table *nls_codepage;
				/* Give Demultiplex thread up to 10 seconds to 
					reconnect, should be greater than cifs socket
					timeout which is 7 seconds */
			while(tcon->ses->server->tcpStatus == CifsNeedReconnect) {
				while ((tcon->ses->server->tcpStatus != CifsGood) && (timeout > 0)){
					timeout = interruptible_sleep_on_timeout(&tcon->ses->server->response_q,timeout);
				}
				if(tcon->ses->server->tcpStatus == CifsNeedReconnect) {
					/* on "soft" mounts we wait once */
					if((tcon->retry == FALSE) || 
					   (tcon->ses->status == CifsExiting)) {
						cFYI(1,("gave up waiting on reconnect in smb_init"));
						return -EHOSTDOWN;
					} /* else "hard" mount - keep retrying until 
					process is killed or server comes back up */
				} else /* TCP session is reestablished now */
					break;
				 
			}
			
			nls_codepage = load_nls_default();
		/* need to prevent multiple threads trying to
		simultaneously reconnect the same SMB session */
			down(&tcon->ses->sesSem);
			if(tcon->ses->status == CifsNeedReconnect)
				rc = cifs_setup_session(0, tcon->ses, nls_codepage);
			if(!rc && (tcon->tidStatus == CifsNeedReconnect)) {
				mark_open_files_invalid(tcon);
				rc = CIFSTCon(0, tcon->ses, tcon->treeName, tcon,
					nls_codepage);
				up(&tcon->ses->sesSem);
				if(rc == 0)
					atomic_inc(&tconInfoReconnectCount);

				cFYI(1, ("reconnect tcon rc = %d", rc));
				/* Removed call to reopen open files here - 
					it is safer (and faster) to reopen files
					one at a time as needed in read and write */

				/* Check if handle based operation so we 
					know whether we can continue or not without
					returning to caller to reset file handle */
				switch(smb_command) {
					case SMB_COM_READ_ANDX:
					case SMB_COM_WRITE_ANDX:
					case SMB_COM_CLOSE:
					case SMB_COM_FIND_CLOSE2:
					case SMB_COM_LOCKING_ANDX: {
						unload_nls(nls_codepage);
						return -EAGAIN;
					}
				}
			} else {
				up(&tcon->ses->sesSem);
			}
			unload_nls(nls_codepage);

		} else {
			return -EIO;
		}
	}
	if(rc)
		return rc;

	*request_buf = cifs_buf_get();
	if (*request_buf == 0) {
		/* BB should we add a retry in here if not a writepage? */
		return -ENOMEM;
	}
    /* Although the original thought was we needed the response buf for  */
    /* potential retries of smb operations it turns out we can determine */
    /* from the mid flags when the request buffer can be resent without  */
    /* having to use a second distinct buffer for the response */
	*response_buf = *request_buf; 

	header_assemble((struct smb_hdr *) *request_buf, smb_command, tcon,
			wct /*wct */ );

#ifdef CONFIG_CIFS_STATS
        if(tcon != NULL) {
                atomic_inc(&tcon->num_smbs_sent);
        }
#endif
	return rc;
}

int
CIFSSMBNegotiate(unsigned int xid, struct cifsSesInfo *ses)
{
	NEGOTIATE_REQ *pSMB;
	NEGOTIATE_RSP *pSMBr;
	int rc = 0;
	int bytes_returned;
	struct TCP_Server_Info * server;

	if(ses->server)
		server = ses->server;
	else {
		rc = -EIO;
		return rc;
	}
	rc = smb_init(SMB_COM_NEGOTIATE, 0, NULL /* no tcon yet */ ,
		      (void **) &pSMB, (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->hdr.Flags2 |= SMBFLG2_UNICODE;
	if (extended_security)
		pSMB->hdr.Flags2 |= SMBFLG2_EXT_SEC;

	pSMB->ByteCount = strlen(protocols[0].name) + 1;
	strncpy(pSMB->DialectsArray, protocols[0].name, 30);	
    /* null guaranteed to be at end of source and target buffers anyway */

	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc == 0) {
		server->secMode = pSMBr->SecurityMode;	
		server->secType = NTLM; /* BB override default for NTLMv2 or krb*/
		/* one byte - no need to convert this or EncryptionKeyLen from le,*/
		server->maxReq = le16_to_cpu(pSMBr->MaxMpxCount);
		/* probably no need to store and check maxvcs */
		server->maxBuf =
			min(le32_to_cpu(pSMBr->MaxBufferSize),
			(__u32) CIFS_MAX_MSGSIZE + MAX_CIFS_HDR_SIZE);
		server->maxRw = le32_to_cpu(pSMBr->MaxRawSize);
		cFYI(0, ("Max buf = %d ", ses->server->maxBuf));
		GETU32(ses->server->sessid) = le32_to_cpu(pSMBr->SessionKey);
		server->capabilities = le32_to_cpu(pSMBr->Capabilities);
		server->timeZone = le16_to_cpu(pSMBr->ServerTimeZone);	
        /* BB with UTC do we ever need to be using srvr timezone? */
		if (pSMBr->EncryptionKeyLength == CIFS_CRYPTO_KEY_SIZE) {
			memcpy(server->cryptKey, pSMBr->u.EncryptionKey,
			       CIFS_CRYPTO_KEY_SIZE);
		} else if ((pSMBr->hdr.Flags2 & SMBFLG2_EXT_SEC)
			   && (pSMBr->EncryptionKeyLength == 0)) {
			/* decode security blob */
		} else
			rc = -EIO;

		/* BB might be helpful to save off the domain of server here */

		if (pSMBr->hdr.Flags2 & SMBFLG2_EXT_SEC) {
			if (pSMBr->ByteCount < 16)
				rc = -EIO;
			else if (pSMBr->ByteCount == 16) {
				server->secType = RawNTLMSSP;
				if (server->socketUseCount.counter > 1) {
					if (memcmp
						(server->server_GUID,
						pSMBr->u.extended_response.
						GUID, 16) != 0) {
						cFYI(1,
							("UID of server does not match previous connection to same ip address"));
						memcpy(server->
							server_GUID,
							pSMBr->u.
							extended_response.
							GUID, 16);
					}
				} else
					memcpy(server->server_GUID,
					       pSMBr->u.extended_response.
					       GUID, 16);
			} else {
				rc = decode_negTokenInit(pSMBr->u.
							 extended_response.
							 SecurityBlob,
							 pSMBr->ByteCount -
							 16, &server->secType);
			}
		} else
			server->capabilities &= ~CAP_EXTENDED_SECURITY;
		if(sign_CIFS_PDUs == FALSE) {        
			if(server->secMode & SECMODE_SIGN_REQUIRED)
				cERROR(1,
				 ("Server requires /proc/fs/cifs/PacketSigningEnabled"));
			server->secMode &= ~(SECMODE_SIGN_ENABLED | SECMODE_SIGN_REQUIRED);
		} else if(sign_CIFS_PDUs == 1) {
			if((server->secMode & SECMODE_SIGN_REQUIRED) == 0)
				server->secMode &= ~(SECMODE_SIGN_ENABLED | SECMODE_SIGN_REQUIRED);
		}
				
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	return rc;
}

int
CIFSSMBTDis(const int xid, struct cifsTconInfo *tcon)
{
	struct smb_hdr *smb_buffer;
	struct smb_hdr *smb_buffer_response;
	int rc = 0;
	int length;

	cFYI(1, ("In tree disconnect"));
	/*
	 *  If last user of the connection and
	 *  connection alive - disconnect it
	 *  If this is the last connection on the server session disconnect it
	 *  (and inside session disconnect we should check if tcp socket needs 
	 *  to be freed and kernel thread woken up).
	 */
	if (tcon)
		down(&tcon->tconSem);
	else
		return -EIO;

	atomic_dec(&tcon->useCount);
	if (atomic_read(&tcon->useCount) > 0) {
		up(&tcon->tconSem);
		return -EBUSY;
	}

	/* No need to return error on this operation if tid invalidated and 
	closed on server already e.g. due to tcp session crashing */
	if(tcon->tidStatus == CifsNeedReconnect) {
		up(&tcon->tconSem);
		return 0;  
	}

	if((tcon->ses == 0) || (tcon->ses->server == 0)) {    
		up(&tcon->tconSem);
		return -EIO;
	}

	rc = smb_init(SMB_COM_TREE_DISCONNECT, 0, tcon,
		      (void **) &smb_buffer, (void **) &smb_buffer_response);
	if (rc) {
		up(&tcon->tconSem);
		return rc;
	}
	rc = SendReceive(xid, tcon->ses, smb_buffer, smb_buffer_response,
			 &length, 0);
	if (rc)
		cFYI(1, (" Tree disconnect failed %d", rc));

	if (smb_buffer)
		cifs_buf_release(smb_buffer);
	up(&tcon->tconSem);

	/* No need to return error on this operation if tid invalidated and 
	closed on server already e.g. due to tcp session crashing */
	if (rc == -EAGAIN)
		rc = 0;

	return rc;
}

int
CIFSSMBLogoff(const int xid, struct cifsSesInfo *ses)
{
	struct smb_hdr *smb_buffer_response;
	LOGOFF_ANDX_REQ *pSMB;
	int rc = 0;
	int length;

	cFYI(1, ("In SMBLogoff for session disconnect"));
	if (ses)
		down(&ses->sesSem);
	else
		return -EIO;

	atomic_dec(&ses->inUse);
	if (atomic_read(&ses->inUse) > 0) {
		up(&ses->sesSem);
		return -EBUSY;
	}

	rc = smb_init(SMB_COM_LOGOFF_ANDX, 2, NULL /* no tcon anymore */,
		 (void **) &pSMB, (void **) &smb_buffer_response);

	if(ses->server->secMode & (SECMODE_SIGN_REQUIRED | SECMODE_SIGN_ENABLED))
		pSMB->hdr.Flags2 |= SMBFLG2_SECURITY_SIGNATURE;

	if (rc) {
		up(&ses->sesSem);
		return rc;
	}

	pSMB->hdr.Uid = ses->Suid;

	pSMB->AndXCommand = 0xFF;
	rc = SendReceive(xid, ses, (struct smb_hdr *) pSMB,
			 smb_buffer_response, &length, 0);
	if (ses->server) {
		atomic_dec(&ses->server->socketUseCount);
		if (atomic_read(&ses->server->socketUseCount) == 0) {
			spin_lock(&GlobalMid_Lock);
			ses->server->tcpStatus = CifsExiting;
			spin_unlock(&GlobalMid_Lock);
			rc = -ESHUTDOWN;
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	up(&ses->sesSem);

	/* if session dead then we do not need to do ulogoff,
		since server closed smb session, no sense reporting 
		error */
	if (rc == -EAGAIN)
		rc = 0;
	return rc;
}

int
CIFSSMBDelFile(const int xid, struct cifsTconInfo *tcon,
	       const char *fileName, const struct nls_table *nls_codepage)
{
	DELETE_FILE_REQ *pSMB = NULL;
	DELETE_FILE_RSP *pSMBr = NULL;
	int rc = 0;
	int bytes_returned;
	int name_len;

DelFileRetry:
	rc = smb_init(SMB_COM_DELETE, 1, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->fileName, fileName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(fileName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->fileName, fileName, name_len);
	}
	pSMB->SearchAttributes =
	    cpu_to_le16(ATTR_READONLY | ATTR_HIDDEN | ATTR_SYSTEM);
	pSMB->ByteCount = name_len + 1;
	pSMB->BufferFormat = 0x04;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Error in RMFile = %d", rc));
	} 
#ifdef CONFIG_CIFS_STATS
        else {
		atomic_inc(&tcon->num_deletes);
        }
#endif

	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto DelFileRetry;

	return rc;
}

int
CIFSSMBRmDir(const int xid, struct cifsTconInfo *tcon,
	     const char *dirName, const struct nls_table *nls_codepage)
{
	DELETE_DIRECTORY_REQ *pSMB = NULL;
	DELETE_DIRECTORY_RSP *pSMBr = NULL;
	int rc = 0;
	int bytes_returned;
	int name_len;

	cFYI(1, ("In CIFSSMBRmDir"));
RmDirRetry:
	rc = smb_init(SMB_COM_DELETE_DIRECTORY, 0, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len = cifs_strtoUCS((wchar_t *) pSMB->DirName, dirName, 530
				/* find define for this maxpathcomponent */
				, nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(dirName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->DirName, dirName, name_len);
	}

	pSMB->ByteCount = name_len + 1;
	pSMB->BufferFormat = 0x04;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Error in RMDir = %d", rc));
	}
#ifdef CONFIG_CIFS_STATS
        else {
		atomic_inc(&tcon->num_rmdirs);
        }
#endif

	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto RmDirRetry;
	return rc;
}

int
CIFSSMBMkDir(const int xid, struct cifsTconInfo *tcon,
	     const char *name, const struct nls_table *nls_codepage)
{
	int rc = 0;
	CREATE_DIRECTORY_REQ *pSMB = NULL;
	CREATE_DIRECTORY_RSP *pSMBr = NULL;
	int bytes_returned;
	int name_len;

	cFYI(1, ("In CIFSSMBMkDir"));
MkDirRetry:
	rc = smb_init(SMB_COM_CREATE_DIRECTORY, 0, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len = cifs_strtoUCS((wchar_t *) pSMB->DirName, name, 530
					 /* find define for this maxpathcomponent */
					 , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(name, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->DirName, name, name_len);
	}

	pSMB->ByteCount = name_len + 1 /* for buf format */ ;
	pSMB->BufferFormat = 0x04;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Error in Mkdir = %d", rc));
	}
#ifdef CONFIG_CIFS_STATS
        else {
		atomic_inc(&tcon->num_mkdirs);
        }
#endif
	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto MkDirRetry;
	return rc;
}

int
CIFSSMBOpen(const int xid, struct cifsTconInfo *tcon,
	    const char *fileName, const int openDisposition,
	    const int access_flags, const int create_options, __u16 * netfid,
	    int *pOplock, FILE_ALL_INFO * pfile_info, 
	    const struct nls_table *nls_codepage)
{
	int rc = -EACCES;
	OPEN_REQ *pSMB = NULL;
	OPEN_RSP *pSMBr = NULL;
	int bytes_returned;
	int name_len;

openRetry:
	rc = smb_init(SMB_COM_NT_CREATE_ANDX, 24, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->AndXCommand = 0xFF;	/* none */

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		pSMB->ByteCount = 1;	/* account for one byte pad to word boundary */
		name_len =
		    cifs_strtoUCS((wchar_t *) (pSMB->fileName + 1),
				  fileName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
		pSMB->NameLength = cpu_to_le16(name_len);
	} else {		/* BB improve the check for buffer overruns BB */
		pSMB->ByteCount = 0;	/* no pad */
		name_len = strnlen(fileName, 530);
		name_len++;	/* trailing null */
		pSMB->NameLength = cpu_to_le16(name_len);
		strncpy(pSMB->fileName, fileName, name_len);
	}
	if (*pOplock & REQ_OPLOCK)
		pSMB->OpenFlags = cpu_to_le32(REQ_OPLOCK);
	else if (*pOplock & REQ_BATCHOPLOCK) {
		pSMB->OpenFlags = cpu_to_le32(REQ_BATCHOPLOCK);
	}
	pSMB->DesiredAccess = cpu_to_le32(access_flags);
	pSMB->AllocationSize = 0;
	pSMB->FileAttributes = ATTR_NORMAL;
	/* XP does not handle ATTR_POSIX_SEMANTICS */
	/* but it helps speed up case sensitive checks for other
	servers such as Samba */
	if (tcon->ses->capabilities & CAP_UNIX)
		pSMB->FileAttributes |= ATTR_POSIX_SEMANTICS;

	/* if ((omode & S_IWUGO) == 0)
		pSMB->FileAttributes |= ATTR_READONLY;*/
	/*  Above line causes problems due to vfs splitting create into two
		pieces - need to set mode after file created not while it is
		being created */
	pSMB->FileAttributes = cpu_to_le32(pSMB->FileAttributes);
	pSMB->ShareAccess = cpu_to_le32(FILE_SHARE_ALL);
	pSMB->CreateDisposition = cpu_to_le32(openDisposition);
	pSMB->CreateOptions = cpu_to_le32(create_options);
	pSMB->ImpersonationLevel = cpu_to_le32(SECURITY_IMPERSONATION);	/* BB ??*/
	pSMB->SecurityFlags =
	    cpu_to_le32(SECURITY_CONTEXT_TRACKING | SECURITY_EFFECTIVE_ONLY);

	pSMB->ByteCount += name_len;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;

	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	/* long_op set to 1 to allow for oplock break timeouts */
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 1);
	if (rc) {
		cFYI(1, ("Error in Open = %d", rc));
	} else {
		*pOplock = pSMBr->OplockLevel;	/* one byte no need to le_to_cpu */
		*netfid = pSMBr->Fid;	/* cifs fid stays in le */
		/* Let caller know file was created so we can set the mode. */
		/* Do we care about the CreateAction in any other cases? */
		if(cpu_to_le32(FILE_CREATE) == pSMBr->CreateAction)
			*pOplock |= CIFS_CREATE_ACTION; 
		if(pfile_info) {
		    memcpy((char *)pfile_info,(char *)&pSMBr->CreationTime,
			36 /* CreationTime to Attributes */);
		    /* the file_info buf is endian converted by caller */
		    pfile_info->AllocationSize = pSMBr->AllocationSize;
		    pfile_info->EndOfFile = pSMBr->EndOfFile;
		    pfile_info->NumberOfLinks = cpu_to_le32(1);
		}

#ifdef CONFIG_CIFS_STATS
		atomic_inc(&tcon->num_opens);
#endif
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto openRetry;
	return rc;
}

/* If no buffer passed in, then caller wants to do the copy
	as in the case of readpages so the SMB buffer must be
	freed by the caller */

int
CIFSSMBRead(const int xid, struct cifsTconInfo *tcon,
	    const int netfid, const unsigned int count,
	    const __u64 lseek, unsigned int *nbytes, char **buf)
{
	int rc = -EACCES;
	READ_REQ *pSMB = NULL;
	READ_RSP *pSMBr = NULL;
	char *pReadData = NULL;
	int bytes_returned;

	*nbytes = 0;
	rc = smb_init(SMB_COM_READ_ANDX, 12, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	/* tcon and ses pointer are checked in smb_init */
	if (tcon->ses->server == NULL)
		return -ECONNABORTED;

	pSMB->AndXCommand = 0xFF;	/* none */
	pSMB->Fid = netfid;
	pSMB->OffsetLow = cpu_to_le32(lseek & 0xFFFFFFFF);
	pSMB->OffsetHigh = cpu_to_le32(lseek >> 32);
	pSMB->Remaining = 0;
	pSMB->MaxCount = cpu_to_le16(count);
	pSMB->MaxCountHigh = 0;
	pSMB->ByteCount = 0;  /* no need to do le conversion since it is 0 */

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cERROR(1, ("Send error in read = %d", rc));
	} else {
		pSMBr->DataLength = le16_to_cpu(pSMBr->DataLength);
		*nbytes = pSMBr->DataLength;
		/*check that DataLength would not go beyond end of SMB */
		if ((pSMBr->DataLength > CIFS_MAX_MSGSIZE) 
				|| (pSMBr->DataLength > count)) {
			cFYI(1,("bad length %d for count %d",pSMBr->DataLength,count));
			rc = -EIO;
			*nbytes = 0;
		} else {
			pReadData =
			    (char *) (&pSMBr->hdr.Protocol) +
			    le16_to_cpu(pSMBr->DataOffset);
/*			if(rc = copy_to_user(buf, pReadData, pSMBr->DataLength)) {
				cERROR(1,("Faulting on read rc = %d",rc));
				rc = -EFAULT;
			}*/ /* can not use copy_to_user when using page cache*/
			if(*buf)
			    memcpy(*buf,pReadData,pSMBr->DataLength);
		}
	}
	if (pSMB) {
		if(*buf)
			cifs_buf_release(pSMB);
		else
			*buf = (char *)pSMB;
	}

	/* Note: On -EAGAIN error only caller can retry on handle based calls 
		since file handle passed in no longer valid */
	return rc;
}

int
CIFSSMBWrite(const int xid, struct cifsTconInfo *tcon,
	     const int netfid, const unsigned int count,
	     const __u64 offset, unsigned int *nbytes, const char *buf,
	     const int long_op)
{
	int rc = -EACCES;
	WRITE_REQ *pSMB = NULL;
	WRITE_RSP *pSMBr = NULL;
	int bytes_returned;

	rc = smb_init(SMB_COM_WRITE_ANDX, 14, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;
	/* tcon and ses pointer are checked in smb_init */
	if (tcon->ses->server == NULL)
		return -ECONNABORTED;

	pSMB->AndXCommand = 0xFF;	/* none */
	pSMB->Fid = netfid;
	pSMB->OffsetLow = cpu_to_le32(offset & 0xFFFFFFFF);
	pSMB->OffsetHigh = cpu_to_le32(offset >> 32);
	pSMB->Remaining = 0;
	if (count > ((tcon->ses->server->maxBuf - MAX_CIFS_HDR_SIZE) & 0xFFFFFF00))
		pSMB->DataLengthLow =
		    (tcon->ses->server->maxBuf - MAX_CIFS_HDR_SIZE) & 0xFFFFFF00;
	else
		pSMB->DataLengthLow = count;
	pSMB->DataLengthHigh = 0;
	pSMB->DataOffset =
	    cpu_to_le16(offsetof(struct smb_com_write_req,Data) - 4);

	memcpy(pSMB->Data,buf,pSMB->DataLengthLow);

	pSMB->ByteCount += pSMB->DataLengthLow + 1 /* pad */ ;
	pSMB->DataLengthLow = cpu_to_le16(pSMB->DataLengthLow);
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, long_op);
	if (rc) {
		cFYI(1, ("Send error in write = %d", rc));
		*nbytes = 0;
	} else
		*nbytes = le16_to_cpu(pSMBr->Count);

	if (pSMB)
		cifs_buf_release(pSMB);

	/* Note: On -EAGAIN error only caller can retry on handle based calls 
		since file handle passed in no longer valid */

	return rc;
}

int
CIFSSMBLock(const int xid, struct cifsTconInfo *tcon,
	    const __u16 smb_file_id, const __u64 len,
	    const __u64 offset, const __u32 numUnlock,
	    const __u32 numLock, const __u8 lockType, const int waitFlag)
{
	int rc = 0;
	LOCK_REQ *pSMB = NULL;
	LOCK_RSP *pSMBr = NULL;
	int bytes_returned;
	int timeout = 0;
	__u64 temp;

	cFYI(1, ("In CIFSSMBLock - timeout %d numLock %d",waitFlag,numLock));
	rc = smb_init(SMB_COM_LOCKING_ANDX, 8, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if(lockType == LOCKING_ANDX_OPLOCK_RELEASE) {
		timeout = -1; /* no response expected */
		pSMB->Timeout = 0;
	} else if (waitFlag == TRUE) {
		timeout = 3;  /* blocking operation, no timeout */
		pSMB->Timeout = -1; /* blocking - do not time out */
	} else {
		pSMB->Timeout = 0;
	}

	pSMB->NumberOfLocks = cpu_to_le32(numLock);
	pSMB->NumberOfUnlocks = cpu_to_le32(numUnlock);
	pSMB->LockType = lockType;
	pSMB->AndXCommand = 0xFF;	/* none */
	pSMB->Fid = smb_file_id; /* netfid stays le */

	if(numLock != 0) {
		pSMB->Locks[0].Pid = cpu_to_le16(current->tgid);
		/* BB where to store pid high? */
		temp = cpu_to_le64(len);
		pSMB->Locks[0].LengthLow = (__u32)(temp & 0xFFFFFFFF);
		pSMB->Locks[0].LengthHigh =  (__u32)(temp>>32);
		temp = cpu_to_le64(offset);
		pSMB->Locks[0].OffsetLow = (__u32)(temp & 0xFFFFFFFF);
		pSMB->Locks[0].OffsetHigh = (__u32)(temp>>32);
		pSMB->ByteCount = sizeof (LOCKING_ANDX_RANGE);
	} else {
		/* oplock break */
		pSMB->ByteCount = 0;
	}
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, timeout);

	if (rc) {
		cFYI(1, ("Send error in Lock = %d", rc));
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	/* Note: On -EAGAIN error only caller can retry on handle based calls 
	since file handle passed in no longer valid */
	return rc;
}

int
CIFSSMBClose(const int xid, struct cifsTconInfo *tcon, int smb_file_id)
{
	int rc = 0;
	CLOSE_REQ *pSMB = NULL;
	CLOSE_RSP *pSMBr = NULL;
	int bytes_returned;
	cFYI(1, ("In CIFSSMBClose"));

/* do not retry on dead session on close */
	rc = smb_init(SMB_COM_CLOSE, 3, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if(rc == -EAGAIN)
		return 0;
	if (rc)
		return rc;

	pSMB->FileID = (__u16) smb_file_id;
	pSMB->LastWriteTime = 0;
	pSMB->ByteCount = 0;
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		if(rc!=-EINTR) {
			/* EINTR is expected when user ctl-c to kill app */
			cERROR(1, ("Send error in Close = %d", rc));
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	/* Since session is dead, file will be closed on server already */
	if(rc == -EAGAIN)
		rc = 0;

	return rc;
}

int
CIFSSMBRename(const int xid, struct cifsTconInfo *tcon,
	      const char *fromName, const char *toName,
	      const struct nls_table *nls_codepage)
{
	int rc = 0;
	RENAME_REQ *pSMB = NULL;
	RENAME_RSP *pSMBr = NULL;
	int bytes_returned;
	int name_len, name_len2;

	cFYI(1, ("In CIFSSMBRename"));
renameRetry:
	rc = smb_init(SMB_COM_RENAME, 1, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->BufferFormat = 0x04;
	pSMB->SearchAttributes =
	    cpu_to_le16(ATTR_READONLY | ATTR_HIDDEN | ATTR_SYSTEM |
			ATTR_DIRECTORY);

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->OldFileName, fromName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
		pSMB->OldFileName[name_len] = 0x04;	/* pad */
	/* protocol requires ASCII signature byte on Unicode string */
		pSMB->OldFileName[name_len + 1] = 0x00;
		name_len2 =
		    cifs_strtoUCS((wchar_t *) & pSMB->
				  OldFileName[name_len + 2], toName, 530,
				  nls_codepage);
		name_len2 += 1 /* trailing null */  + 1 /* Signature word */ ;
		name_len2 *= 2;	/* convert to bytes */
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(fromName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->OldFileName, fromName, name_len);
		name_len2 = strnlen(toName, 530);
		name_len2++;	/* trailing null */
		pSMB->OldFileName[name_len] = 0x04;  /* 2nd buffer format */
		strncpy(&pSMB->OldFileName[name_len + 1], toName, name_len2);
		name_len2++;	/* trailing null */
		name_len2++;	/* signature byte */
	}

	pSMB->ByteCount = 1 /* 1st signature byte */  + name_len + name_len2;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in rename = %d", rc));
	} 

#ifdef CONFIG_CIFS_STATS
	  else {
		atomic_inc(&tcon->num_renames);
	}
#endif

	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto renameRetry;

	return rc;
}

int CIFSSMBRenameOpenFile(const int xid,struct cifsTconInfo *pTcon, 
		int netfid, char * target_name, const struct nls_table * nls_codepage) 
{
	struct smb_com_transaction2_sfi_req *pSMB  = NULL;
	struct smb_com_transaction2_sfi_rsp *pSMBr = NULL;
	struct set_file_rename * rename_info;
	char *data_offset;
	char dummy_string[30];
	int rc = 0;
	int bytes_returned = 0;
	int len_of_str;

	cFYI(1, ("Rename to File by handle"));
	rc = smb_init(SMB_COM_TRANSACTION2, 15, pTcon, (void **) &pSMB,
			(void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->ParameterCount = 6;
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = offsetof(struct smb_com_transaction2_sfi_req,
				Fid) - 4;
	pSMB->DataOffset = pSMB->ParameterOffset + pSMB->ParameterCount;

	data_offset = (char *) (&pSMB->hdr.Protocol) + pSMB->DataOffset;
	rename_info = (struct set_file_rename *) data_offset;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(1000); /* BB find max SMB PDU from sess */
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_SET_FILE_INFORMATION);
	pSMB->ByteCount = 3 /* pad */  + pSMB->ParameterCount;
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	pSMB->ParameterOffset = cpu_to_le16(pSMB->ParameterOffset);
	pSMB->DataOffset = cpu_to_le16(pSMB->DataOffset);
	/* construct random name ".cifs_tmp<inodenum><mid>" */
	rename_info->overwrite = cpu_to_le32(1);
	rename_info->root_fid  = 0;
	/* unicode only call */
	if(target_name == NULL) {
		sprintf(dummy_string,"cifs%x",pSMB->hdr.Mid);
	        len_of_str = cifs_strtoUCS((wchar_t *) rename_info->target_name, dummy_string, 24, nls_codepage);
	} else {
		len_of_str = cifs_strtoUCS((wchar_t *) rename_info->target_name, target_name, 530, nls_codepage);
	}
	rename_info->target_name_len = cpu_to_le32(2 * len_of_str);
	pSMB->DataCount = 12 /* sizeof(struct set_file_rename) */ + (2 * len_of_str) + 2;
	pSMB->ByteCount += pSMB->DataCount;
	pSMB->DataCount = cpu_to_le16(pSMB->DataCount);
	pSMB->TotalDataCount = pSMB->DataCount;
	pSMB->Fid = netfid;
	pSMB->InformationLevel =
		cpu_to_le16(SMB_SET_FILE_RENAME_INFORMATION);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, pTcon->ses, (struct smb_hdr *) pSMB,
                         (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1,("Send error in Rename (by file handle) = %d", rc));
	}
#ifdef CONFIG_CIFS_STATS
	  else {
		atomic_inc(&pTcon->num_t2renames);
	}
#endif
	if (pSMB)
		cifs_buf_release(pSMB);

	/* Note: On -EAGAIN error only caller can retry on handle based calls
		since file handle passed in no longer valid */

	return rc;
}

int
CIFSSMBCopy(const int xid, struct cifsTconInfo *tcon, const char * fromName, 
            const __u16 target_tid, const char *toName, const int flags,
            const struct nls_table *nls_codepage)
{
	int rc = 0;
	COPY_REQ *pSMB = NULL;
	COPY_RSP *pSMBr = NULL;
	int bytes_returned;
	int name_len, name_len2;

	cFYI(1, ("In CIFSSMBCopy"));
copyRetry:
	rc = smb_init(SMB_COM_COPY, 1, tcon, (void **) &pSMB,
			(void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->BufferFormat = 0x04;
	pSMB->Tid2 = target_tid;

	if(flags & COPY_TREE)
		pSMB->Flags |= COPY_TREE;
	pSMB->Flags = cpu_to_le16(pSMB->Flags);

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len = cifs_strtoUCS((wchar_t *) pSMB->OldFileName, 
				fromName, 
				530 /* find define for this maxpathcomponent */,
				nls_codepage);
		name_len++;     /* trailing null */
		name_len *= 2;
		pSMB->OldFileName[name_len] = 0x04;     /* pad */
		/* protocol requires ASCII signature byte on Unicode string */
		pSMB->OldFileName[name_len + 1] = 0x00;
		name_len2 = cifs_strtoUCS((wchar_t *) & pSMB->
				OldFileName[name_len + 2], toName, 530,
				nls_codepage);
		name_len2 += 1 /* trailing null */  + 1 /* Signature word */ ;
		name_len2 *= 2; /* convert to bytes */
	} else {                /* BB improve the check for buffer overruns BB */
		name_len = strnlen(fromName, 530);
		name_len++;     /* trailing null */
		strncpy(pSMB->OldFileName, fromName, name_len);
		name_len2 = strnlen(toName, 530);
		name_len2++;    /* trailing null */
		pSMB->OldFileName[name_len] = 0x04;  /* 2nd buffer format */
		strncpy(&pSMB->OldFileName[name_len + 1], toName, name_len2);
		name_len2++;    /* trailing null */
		name_len2++;    /* signature byte */
	}

	pSMB->ByteCount = 1 /* 1st signature byte */  + name_len + name_len2;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
		(struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in copy = %d with %d files copied",
			rc, pSMBr->CopyCount));
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto copyRetry;

	return rc;
}

int
CIFSUnixCreateSymLink(const int xid, struct cifsTconInfo *tcon,
		      const char *fromName, const char *toName,
		      const struct nls_table *nls_codepage)
{
	TRANSACTION2_SPI_REQ *pSMB = NULL;
	TRANSACTION2_SPI_RSP *pSMBr = NULL;
	char *data_offset;
	int name_len;
	int name_len_target;
	int rc = 0;
	int bytes_returned = 0;

	cFYI(1, ("In Symlink Unix style"));
createSymLinkRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, fromName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;

	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(fromName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, fromName, name_len);
	}
	pSMB->ParameterCount = 6 + name_len;
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = offsetof(struct smb_com_transaction2_spi_req,
                                     InformationLevel) - 4;
	pSMB->DataOffset = pSMB->ParameterOffset + pSMB->ParameterCount;

	data_offset = (char *) (&pSMB->hdr.Protocol) + pSMB->DataOffset;
	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len_target =
		    cifs_strtoUCS((wchar_t *) data_offset, toName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len_target++;	/* trailing null */
		name_len_target *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len_target = strnlen(toName, 530);
		name_len_target++;	/* trailing null */
		strncpy(data_offset, toName, name_len_target);
	}

	pSMB->DataCount = name_len_target;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	/* BB find exact max on data count below from sess */
	pSMB->MaxDataCount = cpu_to_le16(1000);
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_SET_PATH_INFORMATION);
	pSMB->ByteCount = 3 /* pad */  + pSMB->ParameterCount + pSMB->DataCount;
	pSMB->DataCount = cpu_to_le16(pSMB->DataCount);
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->TotalDataCount = pSMB->DataCount;
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	pSMB->ParameterOffset = cpu_to_le16(pSMB->ParameterOffset);
	pSMB->DataOffset = cpu_to_le16(pSMB->DataOffset);
	pSMB->InformationLevel = cpu_to_le16(SMB_SET_FILE_UNIX_LINK);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1,
		     ("Send error in SetPathInfo (create symlink) = %d",
		      rc));
	}

	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto createSymLinkRetry;

	return rc;
}

int
CIFSUnixCreateHardLink(const int xid, struct cifsTconInfo *tcon,
		       const char *fromName, const char *toName,
		       const struct nls_table *nls_codepage)
{
	TRANSACTION2_SPI_REQ *pSMB = NULL;
	TRANSACTION2_SPI_RSP *pSMBr = NULL;
	char *data_offset;
	int name_len;
	int name_len_target;
	int rc = 0;
	int bytes_returned = 0;

	cFYI(1, ("In Create Hard link Unix style"));
createHardLinkRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len = cifs_strtoUCS((wchar_t *) pSMB->FileName, toName, 530
					 /* find define for this maxpathcomponent */
					 , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;

	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(toName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, toName, name_len);
	}
	pSMB->ParameterCount = 6 + name_len;
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = offsetof(struct smb_com_transaction2_spi_req,
                                     InformationLevel) - 4;
	pSMB->DataOffset = pSMB->ParameterOffset + pSMB->ParameterCount;

	data_offset = (char *) (&pSMB->hdr.Protocol) + pSMB->DataOffset;
	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len_target =
		    cifs_strtoUCS((wchar_t *) data_offset, fromName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len_target++;	/* trailing null */
		name_len_target *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len_target = strnlen(fromName, 530);
		name_len_target++;	/* trailing null */
		strncpy(data_offset, fromName, name_len_target);
	}

	pSMB->DataCount = name_len_target;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	/* BB find exact max on data count below from sess*/
	pSMB->MaxDataCount = cpu_to_le16(1000);
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_SET_PATH_INFORMATION);
	pSMB->ByteCount = 3 /* pad */  + pSMB->ParameterCount + pSMB->DataCount;
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	pSMB->DataCount = cpu_to_le16(pSMB->DataCount);
	pSMB->TotalDataCount = pSMB->DataCount;
	pSMB->ParameterOffset = cpu_to_le16(pSMB->ParameterOffset);
	pSMB->DataOffset = cpu_to_le16(pSMB->DataOffset);
	pSMB->InformationLevel = cpu_to_le16(SMB_SET_FILE_UNIX_HLINK);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in SetPathInfo (hard link) = %d", rc));
	}

	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto createHardLinkRetry;

	return rc;
}

int
CIFSCreateHardLink(const int xid, struct cifsTconInfo *tcon,
		   const char *fromName, const char *toName,
		   const struct nls_table *nls_codepage)
{
	int rc = 0;
	NT_RENAME_REQ *pSMB = NULL;
	RENAME_RSP *pSMBr = NULL;
	int bytes_returned;
	int name_len, name_len2;

	cFYI(1, ("In CIFSCreateHardLink"));
winCreateHardLinkRetry:

	rc = smb_init(SMB_COM_NT_RENAME, 4, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->SearchAttributes =
	    cpu_to_le16(ATTR_READONLY | ATTR_HIDDEN | ATTR_SYSTEM |
			ATTR_DIRECTORY);
	pSMB->Flags = cpu_to_le16(CREATE_HARD_LINK);
	pSMB->ClusterCount = 0;

	pSMB->BufferFormat = 0x04;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->OldFileName, fromName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
		pSMB->OldFileName[name_len] = 0;	/* pad */
		pSMB->OldFileName[name_len + 1] = 0x04; 
		name_len2 =
		    cifs_strtoUCS((wchar_t *) & pSMB->
				  OldFileName[name_len + 2], toName, 530,
				  nls_codepage);
		name_len2 += 1 /* trailing null */  + 1 /* Signature word */ ;
		name_len2 *= 2;	/* convert to bytes */
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(fromName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->OldFileName, fromName, name_len);
		name_len2 = strnlen(toName, 530);
		name_len2++;	/* trailing null */
		pSMB->OldFileName[name_len] = 0x04;	/* 2nd buffer format */
		strncpy(&pSMB->OldFileName[name_len + 1], toName, name_len2);
		name_len2++;	/* trailing null */
		name_len2++;	/* signature byte */
	}

	pSMB->ByteCount = 1 /* string type byte */  + name_len + name_len2;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in hard link (NT rename) = %d", rc));
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto winCreateHardLinkRetry;

	return rc;
}

int
CIFSSMBUnixQuerySymLink(const int xid, struct cifsTconInfo *tcon,
			const unsigned char *searchName,
			char *symlinkinfo, const int buflen,
			const struct nls_table *nls_codepage)
{
/* SMB_QUERY_FILE_UNIX_LINK */
	TRANSACTION2_QPI_REQ *pSMB = NULL;
	TRANSACTION2_QPI_RSP *pSMBr = NULL;
	int rc = 0;
	int bytes_returned;
	int name_len;

	cFYI(1, ("In QPathSymLinkInfo (Unix) for path %s", searchName));

querySymLinkRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, searchName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(searchName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, searchName, name_len);
	}

	pSMB->TotalParameterCount =
	    2 /* level */  + 4 /* rsrvd */  + name_len /* incl null */ ;
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	/* BB find exact max data count below from sess structure BB */
	pSMB->MaxDataCount = cpu_to_le16(4000);
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(
        struct smb_com_transaction2_qpi_req ,InformationLevel) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_QUERY_PATH_INFORMATION);
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->InformationLevel = cpu_to_le16(SMB_QUERY_FILE_UNIX_LINK);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in QuerySymLinkInfo = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		pSMBr->DataCount = le16_to_cpu(pSMBr->DataCount);
		if ((pSMBr->ByteCount < 2) || (pSMBr->DataOffset > 512))
		/* BB also check enough total bytes returned */
			rc = -EIO;	/* bad smb */
		else {
			if (pSMBr->hdr.Flags2 & SMBFLG2_UNICODE) {
				name_len = UniStrnlen((wchar_t *) ((char *)
					&pSMBr->hdr.Protocol +pSMBr->DataOffset),
					min_t(const int, buflen,pSMBr->DataCount) / 2);
				cifs_strfromUCS_le(symlinkinfo,
					(wchar_t *) ((char *)&pSMBr->hdr.Protocol +
						pSMBr->DataOffset),
					name_len, nls_codepage);
			} else {
				strncpy(symlinkinfo,
					(char *) &pSMBr->hdr.Protocol + 
						pSMBr->DataOffset,
					min_t(const int, buflen, pSMBr->DataCount));
			}
			symlinkinfo[buflen] = 0;
	/* just in case so calling code does not go off the end of buffer */
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto querySymLinkRetry;
	return rc;
}



int
CIFSSMBQueryReparseLinkInfo(const int xid, struct cifsTconInfo *tcon,
			const unsigned char *searchName,
			char *symlinkinfo, const int buflen,__u16 fid,
			const struct nls_table *nls_codepage)
{
	int rc = 0;
	int bytes_returned;
	int name_len;
	struct smb_com_transaction_ioctl_req * pSMB;
	struct smb_com_transaction_ioctl_rsp * pSMBr;

	cFYI(1, ("In Windows reparse style QueryLink for path %s", searchName));
	rc = smb_init(SMB_COM_NT_TRANSACT, 23, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->TotalParameterCount = 0 ;
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le32(2);
	/* BB find exact data count max from sess structure BB */
	pSMB->MaxDataCount = cpu_to_le32(4000);
	pSMB->MaxSetupCount = 4;
	pSMB->Reserved = 0;
	pSMB->ParameterOffset = 0;
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 4;
	pSMB->SubCommand = cpu_to_le16(NT_TRANSACT_IOCTL);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->FunctionCode = cpu_to_le32(FSCTL_GET_REPARSE_POINT);
	pSMB->IsFsctl = 1; /* FSCTL */
	pSMB->IsRootFlag = 0;
	pSMB->Fid = fid; /* file handle always le */
	pSMB->ByteCount = 0;

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in QueryReparseLinkInfo = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		pSMBr->DataCount = le16_to_cpu(pSMBr->DataCount);
		if ((pSMBr->ByteCount < 2) || (pSMBr->DataOffset > 512))
		/* BB also check enough total bytes returned */
			rc = -EIO;	/* bad smb */
		else {
			if(pSMBr->DataCount && (pSMBr->DataCount < 2048)) {
		/* could also validate reparse tag && better check name length */
				struct reparse_data * reparse_buf = (struct reparse_data *)
					((char *)&pSMBr->hdr.Protocol + pSMBr->DataOffset);
				if (pSMBr->hdr.Flags2 & SMBFLG2_UNICODE) {
					name_len = UniStrnlen((wchar_t *)
							(reparse_buf->LinkNamesBuf + 
							reparse_buf->TargetNameOffset),
							min(buflen/2, reparse_buf->TargetNameLen / 2)); 
					cifs_strfromUCS_le(symlinkinfo,
						(wchar_t *) (reparse_buf->LinkNamesBuf + 
						reparse_buf->TargetNameOffset),
						name_len, nls_codepage);
				} else { /* ASCII names */
					strncpy(symlinkinfo,reparse_buf->LinkNamesBuf + 
						reparse_buf->TargetNameOffset, 
						min_t(const int, buflen, reparse_buf->TargetNameLen));
				}
			} else {
				rc = -EIO;
				cFYI(1,("Invalid return data count on get reparse info ioctl"));
			}
			symlinkinfo[buflen] = 0; /* just in case so the caller
					does not go off the end of the buffer */
			cFYI(1,("readlink result - %s ",symlinkinfo));
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	/* Note: On -EAGAIN error only caller can retry on handle based calls
		since file handle passed in no longer valid */

	return rc;
}

int
CIFSSMBQPathInfo(const int xid, struct cifsTconInfo *tcon,
		 const unsigned char *searchName,
		 FILE_ALL_INFO * pFindData,
		 const struct nls_table *nls_codepage)
{
/* level 263 SMB_QUERY_FILE_ALL_INFO */
	TRANSACTION2_QPI_REQ *pSMB = NULL;
	TRANSACTION2_QPI_RSP *pSMBr = NULL;
	int rc = 0;
	int bytes_returned;
	int name_len;

	cFYI(1, ("In QPathInfo path %s", searchName));
QPathInfoRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, searchName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(searchName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, searchName, name_len);
	}

	pSMB->TotalParameterCount = 2 /* level */  + 4 /* reserved */  +
	    name_len /* includes null */ ;
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(4000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(
        struct smb_com_transaction2_qpi_req ,InformationLevel) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_QUERY_PATH_INFORMATION);
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->InformationLevel = cpu_to_le16(SMB_QUERY_FILE_ALL_INFO);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in QPathInfo = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		/* BB also check enough total bytes returned */
		/* BB we need to improve the validity checking
		of these trans2 responses */
		if ((pSMBr->ByteCount < 40) || (pSMBr->DataOffset > 512)) 
			rc = -EIO;	/* bad smb */
		else if (pFindData){
			memcpy((char *) pFindData,
			       (char *) &pSMBr->hdr.Protocol +
			       pSMBr->DataOffset, sizeof (FILE_ALL_INFO));
		} else
		    rc = -ENOMEM;
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto QPathInfoRetry;

	return rc;
}

int
CIFSSMBUnixQPathInfo(const int xid, struct cifsTconInfo *tcon,
		     const unsigned char *searchName,
		     FILE_UNIX_BASIC_INFO * pFindData,
		     const struct nls_table *nls_codepage)
{
/* SMB_QUERY_FILE_UNIX_BASIC */
	TRANSACTION2_QPI_REQ *pSMB = NULL;
	TRANSACTION2_QPI_RSP *pSMBr = NULL;
	int rc = 0;
	int bytes_returned = 0;
	int name_len;

	cFYI(1, ("In QPathInfo (Unix) the path %s", searchName));
UnixQPathInfoRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, searchName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(searchName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, searchName, name_len);
	}

	pSMB->TotalParameterCount = 2 /* level */  + 4 /* reserved */  +
	    name_len /* includes null */ ;
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxDataCount = cpu_to_le16(4000); 
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(
        struct smb_com_transaction2_qpi_req ,InformationLevel) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_QUERY_PATH_INFORMATION);
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->InformationLevel = cpu_to_le16(SMB_QUERY_FILE_UNIX_BASIC);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in QPathInfo = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		/* BB also check if enough total bytes returned */
		if ((pSMBr->ByteCount < sizeof(FILE_UNIX_BASIC_INFO)) || 
			(pSMBr->DataOffset > 512) || 
			(pSMBr->DataOffset < sizeof(struct smb_hdr))) {
			cFYI(1,("UnixQPathinfo invalid data offset %d bytes returned %d",
					(int)pSMBr->DataOffset,bytes_returned));
			rc = -EIO;	/* bad smb */
		} else {
			memcpy((char *) pFindData,
			       (char *) &pSMBr->hdr.Protocol +
			       pSMBr->DataOffset,
			       sizeof (FILE_UNIX_BASIC_INFO));
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto UnixQPathInfoRetry;

	return rc;
}

int
CIFSFindSingle(const int xid, struct cifsTconInfo *tcon,
	       const char *searchName, FILE_ALL_INFO * findData,
	       const struct nls_table *nls_codepage)
{
/* level 257 SMB_ */
	TRANSACTION2_FFIRST_REQ *pSMB = NULL;
	TRANSACTION2_FFIRST_RSP *pSMBr = NULL;
	int rc = 0;
	int bytes_returned;
	int name_len;

	cFYI(1, ("In FindUnique"));
findUniqueRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, searchName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(searchName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, searchName, name_len);
	}

	pSMB->TotalParameterCount = 12 + name_len /* includes null */ ;
	pSMB->TotalDataCount = 0;	/* no EAs */
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(4000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = cpu_to_le16(
        offsetof(struct smb_com_transaction2_ffirst_req,InformationLevel) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;	/* one byte, no need to le convert */
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_FIND_FIRST);
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalDataCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->SearchAttributes =
	    cpu_to_le16(ATTR_READONLY | ATTR_HIDDEN | ATTR_SYSTEM |
			ATTR_DIRECTORY);
	pSMB->SearchCount = cpu_to_le16(16);	/* BB increase */
	pSMB->SearchFlags = cpu_to_le16(1);
	pSMB->InformationLevel = cpu_to_le16(SMB_FIND_FILE_DIRECTORY_INFO);
	pSMB->SearchStorageType = 0;	/* BB what should we set this to? BB */
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);

	if (rc) {
		cFYI(1, ("Send error in FindFileDirInfo = %d", rc));
	} else {		/* decode response */

		/* BB fill in */
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto findUniqueRetry;

	return rc;
}

int
CIFSFindFirst(const int xid, struct cifsTconInfo *tcon,
	      const char *searchName, FILE_DIRECTORY_INFO * findData,
	      T2_FFIRST_RSP_PARMS * findParms,
	      const struct nls_table *nls_codepage, int *pUnicodeFlag,
	      int *pUnixFlag)
{
/* level 257 SMB_ */
	TRANSACTION2_FFIRST_REQ *pSMB = NULL;
	TRANSACTION2_FFIRST_RSP *pSMBr = NULL;
	char *response_data;
	int rc = 0;
	int bytes_returned;
	int name_len;

	cFYI(1, ("In FindFirst"));
findFirstRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, searchName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(searchName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, searchName, name_len);
	}

	pSMB->TotalParameterCount = 12 + name_len /* includes null */ ;
	pSMB->TotalDataCount = 0;	/* no EAs */
	pSMB->MaxParameterCount = cpu_to_le16(10);
	pSMB->MaxDataCount = cpu_to_le16((tcon->ses->server->maxBuf -
					  MAX_CIFS_HDR_SIZE) & 0xFFFFFF00);
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(struct 
        smb_com_transaction2_ffirst_req, SearchAttributes) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;	/* one byte no need to make endian neutral */
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_FIND_FIRST);
	pSMB->SearchAttributes =
	    cpu_to_le16(ATTR_READONLY | ATTR_HIDDEN | ATTR_SYSTEM |
			ATTR_DIRECTORY);
	pSMB->SearchCount = cpu_to_le16(CIFS_MAX_MSGSIZE / sizeof (FILE_DIRECTORY_INFO));	/* should this be shrunk even more ? */
	pSMB->SearchFlags = cpu_to_le16(CIFS_SEARCH_CLOSE_AT_END | CIFS_SEARCH_RETURN_RESUME);

	/* test for Unix extensions */
	if (tcon->ses->capabilities & CAP_UNIX) {
		pSMB->InformationLevel = cpu_to_le16(SMB_FIND_FILE_UNIX);
		*pUnixFlag = TRUE;
	} else {
		pSMB->InformationLevel =
		    cpu_to_le16(SMB_FIND_FILE_DIRECTORY_INFO);
		*pUnixFlag = FALSE;
	}
	pSMB->SearchStorageType = 0;	/* BB what should we set this to? It is not clear if it matters BB */
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);

	if (rc) {		/* BB add logic to retry regular search if Unix search rejected unexpectedly by server */
		cFYI(1, ("Error in FindFirst = %d", rc));
	} else {		/* decode response */
		/* BB add safety checks for these memcpys */
		if (pSMBr->hdr.Flags2 & SMBFLG2_UNICODE)
			*pUnicodeFlag = TRUE;
		else
			*pUnicodeFlag = FALSE;
		memcpy(findParms,
		       (char *) &pSMBr->hdr.Protocol +
		       le16_to_cpu(pSMBr->ParameterOffset),
		       sizeof (T2_FFIRST_RSP_PARMS));
		/* search handle can stay LE and EAoffset not needed so not converted */
		findParms->EndofSearch = le16_to_cpu(findParms->EndofSearch);
		findParms->LastNameOffset =
		    le16_to_cpu(findParms->LastNameOffset);
		findParms->SearchCount = le16_to_cpu(findParms->SearchCount);
		response_data =
		    (char *) &pSMBr->hdr.Protocol +
		    le16_to_cpu(pSMBr->DataOffset);
		memcpy(findData, response_data, le16_to_cpu(pSMBr->DataCount));
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto findFirstRetry;

	return rc;
}

int
CIFSFindNext(const int xid, struct cifsTconInfo *tcon,
		FILE_DIRECTORY_INFO * findData, T2_FNEXT_RSP_PARMS * findParms,
		const __u16 searchHandle, char * resume_file_name, int name_len,
		__u32 resume_key, int *pUnicodeFlag, int *pUnixFlag)
{
/* level 257 SMB_ */
	TRANSACTION2_FNEXT_REQ *pSMB = NULL;
	TRANSACTION2_FNEXT_RSP *pSMBr = NULL;
	char *response_data;
	int rc = 0;
	int bytes_returned;

	cFYI(1, ("In FindNext"));

	if(resume_file_name == NULL) {
		return -EIO;
	}
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->TotalParameterCount = 14;	/* includes 2 bytes of null string, converted to LE below */
	pSMB->TotalDataCount = 0;	/* no EAs */
	pSMB->MaxParameterCount = cpu_to_le16(8);
	pSMB->MaxDataCount =
	    cpu_to_le16((tcon->ses->server->maxBuf - MAX_CIFS_HDR_SIZE) & 0xFFFFFF00);
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset =  cpu_to_le16(offsetof(
        struct smb_com_transaction2_fnext_req,SearchHandle) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_FIND_NEXT);
	pSMB->SearchHandle = searchHandle;	/* always kept as le */
	findParms->SearchCount = 0;	/* set to zero in case of error */
	pSMB->SearchCount =
	    cpu_to_le16(CIFS_MAX_MSGSIZE / sizeof (FILE_DIRECTORY_INFO));
	/* test for Unix extensions */
	if (tcon->ses->capabilities & CAP_UNIX) {
		pSMB->InformationLevel = cpu_to_le16(SMB_FIND_FILE_UNIX);
		*pUnixFlag = TRUE;
	} else {
		pSMB->InformationLevel =
		    cpu_to_le16(SMB_FIND_FILE_DIRECTORY_INFO);
		*pUnixFlag = FALSE;
	}
	pSMB->ResumeKey = resume_key;
	pSMB->SearchFlags =
	    cpu_to_le16(CIFS_SEARCH_CLOSE_AT_END | CIFS_SEARCH_RETURN_RESUME);
	/* BB add check to make sure we do not cross end of smb */
	if(name_len < CIFS_MAX_MSGSIZE) {
		memcpy(pSMB->ResumeFileName, resume_file_name, name_len);
		pSMB->ByteCount += name_len;
	}
	pSMB->TotalParameterCount += name_len;
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	/* BB improve error handling here */
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);

	if (rc) {
		if (rc == -EBADF)
			rc = 0;	/* search probably was closed at end of search above */
		else
			cFYI(1, ("FindNext returned = %d", rc));
	} else {		/* decode response */
		/* BB add safety checks for these memcpys */
		if (pSMBr->hdr.Flags2 & SMBFLG2_UNICODE)
			*pUnicodeFlag = TRUE;
		else
			*pUnicodeFlag = FALSE;
		memcpy(findParms,
		       (char *) &pSMBr->hdr.Protocol +
		       le16_to_cpu(pSMBr->ParameterOffset),
		       sizeof (T2_FNEXT_RSP_PARMS));
		findParms->EndofSearch = le16_to_cpu(findParms->EndofSearch);
		findParms->LastNameOffset =
		    le16_to_cpu(findParms->LastNameOffset);
		findParms->SearchCount = le16_to_cpu(findParms->SearchCount);
		response_data =
		    (char *) &pSMBr->hdr.Protocol +
		    le16_to_cpu(pSMBr->DataOffset);
		memcpy(findData, response_data, le16_to_cpu(pSMBr->DataCount));
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	/* Note: On -EAGAIN error only caller can retry on handle based calls
		since file handle passed in no longer valid */

	return rc;
}

int
CIFSFindClose(const int xid, struct cifsTconInfo *tcon, const __u16 searchHandle)
{
	int rc = 0;
	FINDCLOSE_REQ *pSMB = NULL;
	CLOSE_RSP *pSMBr = NULL;
	int bytes_returned;

	cFYI(1, ("In CIFSSMBFindClose"));
	rc = smb_init(SMB_COM_FIND_CLOSE2, 1, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	/* no sense returning error if session restarted
		file handle has been closed */
	if(rc == -EAGAIN)
		return 0;
	if (rc)
		return rc;

	pSMB->FileID = searchHandle;
	pSMB->ByteCount = 0;
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cERROR(1, ("Send error in FindClose = %d", rc));
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	/* Since session is dead, search handle closed on server already */
	if (rc == -EAGAIN)
		rc = 0;

	return rc;
}

int
CIFSGetDFSRefer(const int xid, struct cifsSesInfo *ses,
		const unsigned char *searchName,
		unsigned char **targetUNCs,
		unsigned int *number_of_UNC_in_array,
		const struct nls_table *nls_codepage)
{
/* TRANS2_GET_DFS_REFERRAL */
	TRANSACTION2_GET_DFS_REFER_REQ *pSMB = NULL;
	TRANSACTION2_GET_DFS_REFER_RSP *pSMBr = NULL;
	struct dfs_referral_level_3 * referrals = NULL;
	int rc = 0;
	int bytes_returned;
	int name_len;
	unsigned int i;
	char * temp;
	*number_of_UNC_in_array = 0;
	*targetUNCs = NULL;

	cFYI(1, ("In GetDFSRefer the path %s", searchName));
	if (ses == NULL)
		return -ENODEV;
getDFSRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, NULL, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->hdr.Tid = ses->ipc_tid;
	pSMB->hdr.Uid = ses->Suid;
	if (ses->capabilities & CAP_STATUS32) {
		pSMB->hdr.Flags2 |= SMBFLG2_ERR_STATUS;
	}
	if (ses->capabilities & CAP_DFS) {
		pSMB->hdr.Flags2 |= SMBFLG2_DFS;
	}

	if (ses->capabilities & CAP_UNICODE) {
		pSMB->hdr.Flags2 |= SMBFLG2_UNICODE;
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->RequestFileName,
				  searchName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(searchName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->RequestFileName, searchName, name_len);
	}

	pSMB->ParameterCount = 2 /* level */  + name_len /*includes null */ ;
	pSMB->TotalDataCount = 0;
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->MaxParameterCount = 0;
	pSMB->MaxDataCount = cpu_to_le16(4000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(
        struct smb_com_transaction2_get_dfs_refer_req, MaxReferralLevel) - 4);
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_GET_DFS_REFERRAL);
	pSMB->ByteCount = pSMB->ParameterCount + 3 /* pad */ ;
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	pSMB->MaxReferralLevel = cpu_to_le16(3);
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in GetDFSRefer = %d", rc));
	} else {		/* decode response */
/* BB Add logic to parse referrals here */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		pSMBr->DataCount = le16_to_cpu(pSMBr->DataCount);
		cFYI(1,
		     ("Decoding GetDFSRefer response.  BCC: %d  Offset %d",
		      pSMBr->ByteCount, pSMBr->DataOffset));
		if ((pSMBr->ByteCount < 17) || (pSMBr->DataOffset > 512))	/* BB also check enough total bytes returned */
			rc = -EIO;	/* bad smb */
		else {
			referrals = 
			    (struct dfs_referral_level_3 *) 
					(8 /* sizeof start of data block */ +
					pSMBr->DataOffset +
					(char *) &pSMBr->hdr.Protocol); 
			cFYI(1,("num_referrals: %d dfs flags: 0x%x ... \nfor referral one refer size: 0x%x srv type: 0x%x refer flags: 0x%x ttl: 0x%x",pSMBr->NumberOfReferrals,pSMBr->DFSFlags, referrals->ReferralSize,referrals->ServerType,referrals->ReferralFlags,referrals->TimeToLive));
			/* BB This field is actually two bytes in from start of
			   data block so we could do safety check that DataBlock
			   begins at address of pSMBr->NumberOfReferrals */
			*number_of_UNC_in_array = le16_to_cpu(pSMBr->NumberOfReferrals);

			/* BB Fix below so can return more than one referral */
			if(*number_of_UNC_in_array > 1)
				*number_of_UNC_in_array = 1;

			/* get the length of the strings describing refs */
			name_len = 0;
			for(i=0;i<*number_of_UNC_in_array;i++) {
				/* make sure that DfsPathOffset not past end */
				referrals->DfsPathOffset = le16_to_cpu(referrals->DfsPathOffset);
				if(referrals->DfsPathOffset > pSMBr->DataCount) {
					/* if invalid referral, stop here and do 
					not try to copy any more */
					*number_of_UNC_in_array = i;
					break;
				} 
				temp = ((char *)referrals) + referrals->DfsPathOffset;

				if (pSMBr->hdr.Flags2 & SMBFLG2_UNICODE) {
					name_len += UniStrnlen((wchar_t *)temp,pSMBr->DataCount);
				} else {
					name_len += strnlen(temp,pSMBr->DataCount);
				}
				referrals++;
				/* BB add check that referral pointer does not fall off end PDU */
				
			}
			/* BB add check for name_len bigger than bcc */
			*targetUNCs = 
				kmalloc(name_len+1+ (*number_of_UNC_in_array),GFP_KERNEL);
			/* copy the ref strings */
			referrals =  
			    (struct dfs_referral_level_3 *) 
					(8 /* sizeof data hdr */ +
					pSMBr->DataOffset + 
					(char *) &pSMBr->hdr.Protocol);

			for(i=0;i<*number_of_UNC_in_array;i++) {
				temp = ((char *)referrals) + referrals->DfsPathOffset;
				if (pSMBr->hdr.Flags2 & SMBFLG2_UNICODE) {
					cifs_strfromUCS_le(*targetUNCs,
						(wchar_t *) temp, name_len, nls_codepage);
				} else {
					strncpy(*targetUNCs,temp,name_len);
				}
				/*  BB update target_uncs pointers */
				referrals++;
			}
			temp = *targetUNCs;
			temp[name_len] = 0;
		}

	}
	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto getDFSRetry;

	return rc;
}

int
CIFSSMBQFSInfo(const int xid, struct cifsTconInfo *tcon,
	       struct statfs *FSData, const struct nls_table *nls_codepage)
{
/* level 0x103 SMB_QUERY_FILE_SYSTEM_INFO */
	TRANSACTION2_QFSI_REQ *pSMB = NULL;
	TRANSACTION2_QFSI_RSP *pSMBr = NULL;
	FILE_SYSTEM_INFO *response_data;
	int rc = 0;
	int bytes_returned = 0;

	cFYI(1, ("In QFSInfo"));
QFSInfoRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->TotalParameterCount = 2;	/* level */
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(1000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(
        struct smb_com_transaction2_qfsi_req, InformationLevel) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_QUERY_FS_INFORMATION);
	pSMB->InformationLevel = cpu_to_le16(SMB_QUERY_FS_SIZE_INFO);
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cERROR(1, ("Send error in QFSInfo = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		cFYI(1,
		     ("Decoding qfsinfo response.  BCC: %d  Offset %d",
		      pSMBr->ByteCount, pSMBr->DataOffset));
		if ((pSMBr->ByteCount < 24) || (pSMBr->DataOffset > 512))	/* BB also check enough total bytes returned */
			rc = -EIO;	/* bad smb */
		else {
			response_data =
			    (FILE_SYSTEM_INFO
			     *) (((char *) &pSMBr->hdr.Protocol) +
				 pSMBr->DataOffset);
			FSData->f_bsize =
			    le32_to_cpu(response_data->BytesPerSector) *
			    le32_to_cpu(response_data->
					SectorsPerAllocationUnit);
			FSData->f_blocks =
			    le64_to_cpu(response_data->TotalAllocationUnits);
			FSData->f_bfree = FSData->f_bavail =
			    le64_to_cpu(response_data->FreeAllocationUnits);
			cFYI(1,
			     ("Blocks: %lld  Free: %lld Block size %ld",
			      (unsigned long long)FSData->f_blocks,
			      (unsigned long long)FSData->f_bfree,
			      FSData->f_bsize));
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto QFSInfoRetry;

	return rc;
}

int
CIFSSMBQFSAttributeInfo(int xid, struct cifsTconInfo *tcon,
			const struct nls_table *nls_codepage)
{
/* level 0x105  SMB_QUERY_FILE_SYSTEM_INFO */
	TRANSACTION2_QFSI_REQ *pSMB = NULL;
	TRANSACTION2_QFSI_RSP *pSMBr = NULL;
	FILE_SYSTEM_ATTRIBUTE_INFO *response_data;
	int rc = 0;
	int bytes_returned = 0;

	cFYI(1, ("In QFSAttributeInfo"));
QFSAttributeRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->TotalParameterCount = 2;	/* level */
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(1000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(
        struct smb_com_transaction2_qfsi_req, InformationLevel) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_QUERY_FS_INFORMATION);
	pSMB->InformationLevel = cpu_to_le16(SMB_QUERY_FS_ATTRIBUTE_INFO);
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cERROR(1, ("Send error in QFSAttributeInfo = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		if ((pSMBr->ByteCount < 13) || (pSMBr->DataOffset > 512)) {	/* BB also check enough bytes returned */
			rc = -EIO;	/* bad smb */
		} else {
			response_data =
			    (FILE_SYSTEM_ATTRIBUTE_INFO
			     *) (((char *) &pSMBr->hdr.Protocol) +
				 pSMBr->DataOffset);
			response_data->Attributes = le32_to_cpu(response_data->Attributes);
			response_data->MaxPathNameComponentLength = 
				le32_to_cpu(response_data->MaxPathNameComponentLength);
			response_data->FileSystemNameLen = 
				le32_to_cpu(response_data->FileSystemNameLen);
			memcpy(&tcon->fsAttrInfo, response_data,
			       sizeof (FILE_SYSTEM_ATTRIBUTE_INFO));
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto QFSAttributeRetry;

	return rc;
}

int
CIFSSMBQFSDeviceInfo(int xid, struct cifsTconInfo *tcon,
		     const struct nls_table *nls_codepage)
{
/* level 0x104 SMB_QUERY_FILE_SYSTEM_INFO */
	TRANSACTION2_QFSI_REQ *pSMB = NULL;
	TRANSACTION2_QFSI_RSP *pSMBr = NULL;
	FILE_SYSTEM_DEVICE_INFO *response_data;
	int rc = 0;
	int bytes_returned = 0;

	cFYI(1, ("In QFSDeviceInfo"));
QFSDeviceRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->TotalParameterCount = 2;	/* level */
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(1000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(
        struct smb_com_transaction2_qfsi_req, InformationLevel) - 4);

	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_QUERY_FS_INFORMATION);
	pSMB->InformationLevel = cpu_to_le16(SMB_QUERY_FS_DEVICE_INFO);
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in QFSDeviceInfo = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		if ((pSMBr->ByteCount < sizeof (FILE_SYSTEM_DEVICE_INFO))
                 || (pSMBr->DataOffset > 512))
			rc = -EIO;	/* bad smb */
		else {
			response_data =
			    (FILE_SYSTEM_DEVICE_INFO
			     *) (((char *) &pSMBr->hdr.Protocol) +
				 pSMBr->DataOffset);
			response_data->DeviceType = 
				le32_to_cpu(response_data->DeviceType);
			response_data->DeviceCharacteristics = 
				le32_to_cpu(response_data->DeviceCharacteristics);
			memcpy(&tcon->fsDevInfo, response_data,
			       sizeof (FILE_SYSTEM_DEVICE_INFO));
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto QFSDeviceRetry;

	return rc;
}

int
CIFSSMBQFSUnixInfo(int xid, struct cifsTconInfo *tcon,
		   const struct nls_table *nls_codepage)
{
/* level 0x200  SMB_QUERY_CIFS_UNIX_INFO */
	TRANSACTION2_QFSI_REQ *pSMB = NULL;
	TRANSACTION2_QFSI_RSP *pSMBr = NULL;
	FILE_SYSTEM_UNIX_INFO *response_data;
	int rc = 0;
	int bytes_returned = 0;

	cFYI(1, ("In QFSUnixInfo"));
QFSUnixRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->ParameterCount = 2;	/* level */
	pSMB->TotalDataCount = 0;
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(100);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ByteCount = pSMB->ParameterCount + 1 /* pad */ ;
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(struct 
        smb_com_transaction2_qfsi_req, InformationLevel) - 4);
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_QUERY_FS_INFORMATION);
	pSMB->InformationLevel = cpu_to_le16(SMB_QUERY_CIFS_UNIX_INFO);
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cERROR(1, ("Send error in QFSUnixInfo = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = cpu_to_le16(pSMBr->DataOffset);
		if ((pSMBr->ByteCount < 13) || (pSMBr->DataOffset > 512)) {
			rc = -EIO;	/* bad smb */
		} else {
			response_data =
			    (FILE_SYSTEM_UNIX_INFO
			     *) (((char *) &pSMBr->hdr.Protocol) +
				 pSMBr->DataOffset);
			response_data->MajorVersionNumber =
				le16_to_cpu(response_data->MajorVersionNumber);
			response_data->MinorVersionNumber =
				le16_to_cpu(response_data->MinorVersionNumber);
			response_data->Capability =
				le64_to_cpu(response_data->Capability);
			memcpy(&tcon->fsUnixInfo, response_data,
			       sizeof (FILE_SYSTEM_UNIX_INFO));
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto QFSUnixRetry;


	return rc;
}

/* We can not use write of zero bytes trick to 
   set file size due to need for large file support.  Also note that 
   this SetPathInfo is preferred to SetFileInfo based method in next 
   routine which is only needed to work around a sharing violation bug
   in Samba which this routine can run into */

int
CIFSSMBSetEOF(int xid, struct cifsTconInfo *tcon, char *fileName,
	      __u64 size, int SetAllocation, const struct nls_table *nls_codepage)
{
	struct smb_com_transaction2_spi_req *pSMB = NULL;
	struct smb_com_transaction2_spi_rsp *pSMBr = NULL;
	struct file_end_of_file_info *parm_data;
	int name_len;
	int rc = 0;
	int bytes_returned = 0;

	cFYI(1, ("In SetEOF"));
SetEOFRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, fileName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(fileName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, fileName, name_len);
	}
	pSMB->ParameterCount = 6 + name_len;
	pSMB->DataCount = sizeof (struct file_end_of_file_info);
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(1000);	/* BB find max SMB size from sess */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = offsetof(struct smb_com_transaction2_spi_req,
                                     InformationLevel) - 4;
	pSMB->DataOffset = pSMB->ParameterOffset + pSMB->ParameterCount;
	if(SetAllocation) {
        	if (tcon->ses->capabilities & CAP_INFOLEVEL_PASSTHRU)
	            pSMB->InformationLevel =
                	cpu_to_le16(SMB_SET_FILE_ALLOCATION_INFO2);
        	else
	            pSMB->InformationLevel =
        	        cpu_to_le16(SMB_SET_FILE_ALLOCATION_INFO);
	} else /* Set File Size */  {    
	    if (tcon->ses->capabilities & CAP_INFOLEVEL_PASSTHRU)
		    pSMB->InformationLevel =
		        cpu_to_le16(SMB_SET_FILE_END_OF_FILE_INFO2);
	    else
		    pSMB->InformationLevel =
		        cpu_to_le16(SMB_SET_FILE_END_OF_FILE_INFO);
	}

	parm_data =
	    (struct file_end_of_file_info *) (((char *) &pSMB->hdr.Protocol) +
				       pSMB->DataOffset);
	pSMB->ParameterOffset = cpu_to_le16(pSMB->ParameterOffset);
	pSMB->DataOffset = cpu_to_le16(pSMB->DataOffset);
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_SET_PATH_INFORMATION);
	pSMB->ByteCount = 3 /* pad */  + pSMB->ParameterCount + pSMB->DataCount;
	pSMB->DataCount = cpu_to_le16(pSMB->DataCount);
	pSMB->TotalDataCount = pSMB->DataCount;
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	parm_data->FileSize = cpu_to_le64(size);
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("SetPathInfo (file size) returned %d", rc));
	}

	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto SetEOFRetry;

	return rc;
}

int
CIFSSMBSetFileSize(const int xid, struct cifsTconInfo *tcon, __u64 size, 
                   __u16 fid, __u32 pid_of_opener, int SetAllocation)
{
	struct smb_com_transaction2_sfi_req *pSMB  = NULL;
	struct smb_com_transaction2_sfi_rsp *pSMBr = NULL;
	char *data_offset;
	struct file_end_of_file_info *parm_data;
	int rc = 0;
	int bytes_returned = 0;
	__u32 tmp;

	cFYI(1, ("SetFileSize (via SetFileInfo) %lld",
			(long long)size));
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	tmp = cpu_to_le32(pid_of_opener);  /* override pid of current process
                                         so network fid will be valid */
	pSMB->hdr.Pid = tmp & 0xFFFF;
	tmp >>= 16;
	pSMB->hdr.PidHigh = tmp & 0xFFFF;
    
	pSMB->ParameterCount = 6;
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = offsetof(struct smb_com_transaction2_sfi_req,
                                     Fid) - 4;
	pSMB->DataOffset = pSMB->ParameterOffset + pSMB->ParameterCount;

	data_offset = (char *) (&pSMB->hdr.Protocol) + pSMB->DataOffset;	

	pSMB->DataCount = sizeof(struct file_end_of_file_info);
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(1000);	/* BB find max SMB PDU from sess */
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_SET_FILE_INFORMATION);
	pSMB->ByteCount = 3 /* pad */  + pSMB->ParameterCount + pSMB->DataCount;
	pSMB->DataCount = cpu_to_le16(pSMB->DataCount);
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->TotalDataCount = pSMB->DataCount;
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	pSMB->ParameterOffset = cpu_to_le16(pSMB->ParameterOffset);
	parm_data =
		(struct file_end_of_file_info *) (((char *) &pSMB->hdr.Protocol) +
			pSMB->DataOffset);
	pSMB->DataOffset = cpu_to_le16(pSMB->DataOffset); /* now safe to change to le */
	parm_data->FileSize = cpu_to_le64(size);
	pSMB->Fid = fid;
	if(SetAllocation) {
		if (tcon->ses->capabilities & CAP_INFOLEVEL_PASSTHRU)
			pSMB->InformationLevel =
				cpu_to_le16(SMB_SET_FILE_ALLOCATION_INFO2);
		else
			pSMB->InformationLevel =
				cpu_to_le16(SMB_SET_FILE_ALLOCATION_INFO);
	} else /* Set File Size */  {    
	    if (tcon->ses->capabilities & CAP_INFOLEVEL_PASSTHRU)
		    pSMB->InformationLevel =
		        cpu_to_le16(SMB_SET_FILE_END_OF_FILE_INFO2);
	    else
		    pSMB->InformationLevel =
		        cpu_to_le16(SMB_SET_FILE_END_OF_FILE_INFO);
	}
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1,
		     ("Send error in SetFileInfo (SetFileSize) = %d",
		      rc));
	}

	if (pSMB)
		cifs_buf_release(pSMB);

	/* Note: On -EAGAIN error only caller can retry on handle based calls 
		since file handle passed in no longer valid */

	return rc;
}

int
CIFSSMBSetTimes(int xid, struct cifsTconInfo *tcon, char *fileName,
		FILE_BASIC_INFO * data, const struct nls_table *nls_codepage)
{
	TRANSACTION2_SPI_REQ *pSMB = NULL;
	TRANSACTION2_SPI_RSP *pSMBr = NULL;
	int name_len;
	int rc = 0;
	int bytes_returned = 0;
	char *data_offset;

	cFYI(1, ("In SetTimes"));

SetTimesRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, fileName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(fileName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, fileName, name_len);
	}

	pSMB->ParameterCount = 6 + name_len;
	pSMB->DataCount = sizeof (FILE_BASIC_INFO);
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(1000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = offsetof(struct smb_com_transaction2_spi_req,
                                     InformationLevel) - 4;
	pSMB->DataOffset = pSMB->ParameterOffset + pSMB->ParameterCount;
	data_offset = (char *) (&pSMB->hdr.Protocol) + pSMB->DataOffset;
	pSMB->ParameterOffset = cpu_to_le16(pSMB->ParameterOffset);
	pSMB->DataOffset = cpu_to_le16(pSMB->DataOffset);
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_SET_PATH_INFORMATION);
	pSMB->ByteCount = 3 /* pad */  + pSMB->ParameterCount + pSMB->DataCount;

	pSMB->DataCount = cpu_to_le16(pSMB->DataCount);
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->TotalDataCount = pSMB->DataCount;
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	if (tcon->ses->capabilities & CAP_INFOLEVEL_PASSTHRU)
		pSMB->InformationLevel = cpu_to_le16(SMB_SET_FILE_BASIC_INFO2);
	else
		pSMB->InformationLevel = cpu_to_le16(SMB_SET_FILE_BASIC_INFO);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	memcpy(data_offset, data, sizeof (FILE_BASIC_INFO));
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("SetPathInfo (times) returned %d", rc));
	}

	if (pSMB)
		cifs_buf_release(pSMB);

	if (rc == -EAGAIN)
		goto SetTimesRetry;

	return rc;
}

int
CIFSSMBUnixSetPerms(const int xid, struct cifsTconInfo *tcon,
		    char *fileName, __u64 mode, __u64 uid, __u64 gid,
		    dev_t device, const struct nls_table *nls_codepage)
{
	TRANSACTION2_SPI_REQ *pSMB = NULL;
	TRANSACTION2_SPI_RSP *pSMBr = NULL;
	int name_len;
	int rc = 0;
	int bytes_returned = 0;
	FILE_UNIX_BASIC_INFO *data_offset;

	cFYI(1, ("In SetUID/GID/Mode"));
setPermsRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, fileName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(fileName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, fileName, name_len);
	}

	pSMB->ParameterCount = 6 + name_len;
	pSMB->DataCount = sizeof (FILE_UNIX_BASIC_INFO);
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(1000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = offsetof(struct smb_com_transaction2_spi_req,
                                     InformationLevel) - 4;
	pSMB->DataOffset = pSMB->ParameterOffset + pSMB->ParameterCount;
	data_offset =
	    (FILE_UNIX_BASIC_INFO *) ((char *) &pSMB->hdr.Protocol +
				      pSMB->DataOffset);
	pSMB->DataOffset = cpu_to_le16(pSMB->DataOffset);
	pSMB->ParameterOffset = cpu_to_le16(pSMB->ParameterOffset);
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_SET_PATH_INFORMATION);
	pSMB->ByteCount = 3 /* pad */  + pSMB->ParameterCount + pSMB->DataCount;
	pSMB->ParameterCount = cpu_to_le16(pSMB->ParameterCount);
	pSMB->DataCount = cpu_to_le16(pSMB->DataCount);
	pSMB->TotalParameterCount = pSMB->ParameterCount;
	pSMB->TotalDataCount = pSMB->DataCount;
	pSMB->InformationLevel = cpu_to_le16(SMB_SET_FILE_UNIX_BASIC);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	data_offset->Uid = cpu_to_le64(uid);
	data_offset->Gid = cpu_to_le64(gid);
	/* better to leave device as zero when it is  */
	data_offset->DevMajor = cpu_to_le64(MAJOR(device));
	data_offset->DevMinor = cpu_to_le64(MINOR(device));
	data_offset->Permissions = cpu_to_le64(mode);
    
	if(S_ISREG(mode))
		data_offset->Type = cpu_to_le32(UNIX_FILE);
	else if(S_ISDIR(mode))
		data_offset->Type = cpu_to_le32(UNIX_DIR);
	else if(S_ISLNK(mode))
		data_offset->Type = cpu_to_le32(UNIX_SYMLINK);
	else if(S_ISCHR(mode))
		data_offset->Type = cpu_to_le32(UNIX_CHARDEV);
	else if(S_ISBLK(mode))
		data_offset->Type = cpu_to_le32(UNIX_BLOCKDEV);
	else if(S_ISFIFO(mode))
		data_offset->Type = cpu_to_le32(UNIX_FIFO);
	else if(S_ISSOCK(mode))
		data_offset->Type = cpu_to_le32(UNIX_SOCKET);


	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("SetPathInfo (perms) returned %d", rc));
	}

	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto setPermsRetry;
	return rc;
}

int CIFSSMBNotify(const int xid, struct cifsTconInfo *tcon, 
			const int notify_subdirs, const __u16 netfid,
			__u32 filter, const struct nls_table *nls_codepage)
{
	int rc = 0;
	struct smb_com_transaction_change_notify_req * pSMB = NULL;
	struct smb_com_transaction_change_notify_rsp * pSMBr = NULL;
	int bytes_returned;

	cFYI(1, ("In CIFSSMBNotify for file handle %d",(int)netfid));
	rc = smb_init(SMB_COM_NT_TRANSACT, 23, tcon, (void **) &pSMB,
                      (void **) &pSMBr);
	if (rc)
		return rc;

	pSMB->TotalParameterCount = 0 ;
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le32(2);
	/* BB find exact data count max from sess structure BB */
	pSMB->MaxDataCount = 0; /* same in little endian or be */
	pSMB->MaxSetupCount = 4;
	pSMB->Reserved = 0;
	pSMB->ParameterOffset = 0;
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 4; /* single byte does not need le conversion */
	pSMB->SubCommand = cpu_to_le16(NT_TRANSACT_NOTIFY_CHANGE);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	if(notify_subdirs)
		pSMB->WatchTree = 1; /* one byte - no le conversion needed */
	pSMB->Reserved2 = 0;
	pSMB->CompletionFilter = cpu_to_le32(filter);
	pSMB->Fid = netfid; /* file handle always le */
	pSMB->ByteCount = 0;

	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			(struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Error in Notify = %d", rc));
	}
	if (pSMB)
		cifs_buf_release(pSMB);
/*		if (rc == -EAGAIN)
			goto NotifyRetry; */
	return rc;	
}
#ifdef CONFIG_CIFS_XATTR
int
CIFSSMBQAllEAs(const int xid, struct cifsTconInfo *tcon,
		 const unsigned char *searchName,
		 char * EAData, size_t size,
		 const struct nls_table *nls_codepage)
{
		/* BB assumes one setup word */
	TRANSACTION2_QPI_REQ *pSMB = NULL;
	TRANSACTION2_QPI_RSP *pSMBr = NULL;
	int rc = 0;
	int bytes_returned;
	int name_len;

	cFYI(1, ("In Query All EAs path %s", searchName));
QAllEAsRetry:
	rc = smb_init(SMB_COM_TRANSACTION2, 15, tcon, (void **) &pSMB,
		      (void **) &pSMBr);
	if (rc)
		return rc;

	if (pSMB->hdr.Flags2 & SMBFLG2_UNICODE) {
		name_len =
		    cifs_strtoUCS((wchar_t *) pSMB->FileName, searchName, 530
				  /* find define for this maxpathcomponent */
				  , nls_codepage);
		name_len++;	/* trailing null */
		name_len *= 2;
	} else {		/* BB improve the check for buffer overruns BB */
		name_len = strnlen(searchName, 530);
		name_len++;	/* trailing null */
		strncpy(pSMB->FileName, searchName, name_len);
	}

	pSMB->TotalParameterCount = 2 /* level */  + 4 /* reserved */  +
	    name_len /* includes null */ ;
	pSMB->TotalDataCount = 0;
	pSMB->MaxParameterCount = cpu_to_le16(2);
	pSMB->MaxDataCount = cpu_to_le16(4000);	/* BB find exact max SMB PDU from sess structure BB */
	pSMB->MaxSetupCount = 0;
	pSMB->Reserved = 0;
	pSMB->Flags = 0;
	pSMB->Timeout = 0;
	pSMB->Reserved2 = 0;
	pSMB->ParameterOffset = cpu_to_le16(offsetof(
        struct smb_com_transaction2_qpi_req ,InformationLevel) - 4);
	pSMB->DataCount = 0;
	pSMB->DataOffset = 0;
	pSMB->SetupCount = 1;
	pSMB->Reserved3 = 0;
	pSMB->SubCommand = cpu_to_le16(TRANS2_QUERY_PATH_INFORMATION);
	pSMB->ByteCount = pSMB->TotalParameterCount + 1 /* pad */ ;
	pSMB->TotalParameterCount = cpu_to_le16(pSMB->TotalParameterCount);
	pSMB->ParameterCount = pSMB->TotalParameterCount;
	pSMB->InformationLevel = cpu_to_le16(SMB_INFO_QUERY_ALL_EAS);
	pSMB->Reserved4 = 0;
	pSMB->hdr.smb_buf_length += pSMB->ByteCount;
	pSMB->ByteCount = cpu_to_le16(pSMB->ByteCount);

	rc = SendReceive(xid, tcon->ses, (struct smb_hdr *) pSMB,
			 (struct smb_hdr *) pSMBr, &bytes_returned, 0);
	if (rc) {
		cFYI(1, ("Send error in QueryAllEAs = %d", rc));
	} else {		/* decode response */
		pSMBr->DataOffset = le16_to_cpu(pSMBr->DataOffset);
		/* BB also check enough total bytes returned */
		/* BB we need to improve the validity checking
		of these trans2 responses */
		if ((pSMBr->ByteCount < 4) || (pSMBr->DataOffset > 512)) 
			rc = -EIO;	/* bad smb */
	   /* else if (pFindData){
			memcpy((char *) pFindData,
			       (char *) &pSMBr->hdr.Protocol +
			       pSMBr->DataOffset, kl);
		}*/ else {
			/* check that length of list is not more than bcc */
			/* check that each entry does not go beyond length
			   of list */
			/* check that each element of each entry does not
			   go beyond end of list */
			struct fealist * ea_response_data;
			rc = 0;
			/* validate_trans2_offsets() */
			/* BB to check if(start of smb + pSMBr->DataOffset > &bcc+ bcc)*/
			ea_response_data = (struct fealist *)
				(((char *) &pSMBr->hdr.Protocol) +
				pSMBr->DataOffset);
			cFYI(1,("ea length %d",ea_response_data->list_len));
		}
	}
	if (pSMB)
		cifs_buf_release(pSMB);
	if (rc == -EAGAIN)
		goto QAllEAsRetry;

	return rc;
}
#endif
