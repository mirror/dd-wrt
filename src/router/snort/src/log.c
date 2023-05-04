/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* !WIN32 */
#include <errno.h>
#include <signal.h>

#include "log.h"
#include "rules.h"
#include "treenodes.h"
#include "util.h"
#include "snort_debug.h"
#include "signature.h"
#include "util_net.h"
#include "snort_bounds.h"
#include "obfuscation.h"
#include "detection_util.h"
#include "detect.h"

#include "snort.h"

extern OptTreeNode *otn_tmp;    /* global ptr to current rule data */

char *data_dump_buffer;     /* printout buffer for PrintNetData */
int data_dump_buffer_size = 0;/* size of printout buffer */
int dump_size;              /* amount of data to print */

extern int IsGzipData(void *);
extern int IsJSNormData(void *);
void AllocDumpBuf();


/***************** LOG ASCII ROUTINES *******************/

#ifndef NO_NON_ETHER_DECODER
#endif

/*
 * Function: PrintNetData(FILE *, u_char *,int, Packet *)
 *
 * Purpose: Do a side by side dump of a buffer, hex dump of buffer bytes on
 *          the left, decoded ASCII on the right.
 *
 * Arguments: fp => ptr to stream to print to
 *            start => pointer to buffer data
 *            len => length of data buffer
 *
 * Returns: void function
 */
void PrintNetData(FILE * fp, const u_char * start, const int len, Packet *p)
{
    char *end;          /* ptr to buffer end */
    int i;          /* counter */
    int j;          /* counter */
    int dbuf_size;      /* data buffer size */
    int done;           /* flag */
    char *data;         /* index pointer */
    char *frame_ptr;        /* we use 66 byte frames for a printed line */
    char *d_ptr;        /* data pointer into the frame */
    char *c_ptr;        /* char pointer into the frame */
    char conv[] = "0123456789ABCDEF";   /* xlation lookup table */
    int next_layer, ip_start, ip_ob_start, ip_ob_end, byte_pos;

    next_layer = ip_start = byte_pos = 0;

    ip_ob_start = ip_ob_end = -1;

    /* initialization */
    done = 0;

   /* zero, print a <CR> and get out */
    if(!len)
    {
        fputc('\n', fp);
        return;
    }

    if(start == NULL)
    {
        printf("Got NULL ptr in PrintNetData()\n");
        return;
    }

    end = (char*) (start + (len - 1));    /* set the end of buffer ptr */

    if(len > IP_MAXPACKET)
    {
        if (ScLogVerbose())
        {
            printf("Got bogus buffer length (%d) for PrintNetData, defaulting to 16 bytes!\n", len);
        }

        if (ScVerboseByteDump())
        {
            dbuf_size = (FRAME_SIZE + 8) + (FRAME_SIZE + 8) + 1;
        }
        else
        {
            dbuf_size = FRAME_SIZE + FRAME_SIZE + 1;
        }

        /* dbuf_size = 66 + 67; */
        end =  (char*) (start + 15);
    }
    else
    {
        if (ScVerboseByteDump())
        {
            /* figure out how big the printout data buffer has to be */
            dbuf_size = ((len / 16) * (FRAME_SIZE + 8)) + (FRAME_SIZE + 8) + 1;
        }
        else
        {
            /* figure out how big the printout data buffer has to be */
            dbuf_size = ((len / 16) * FRAME_SIZE) + FRAME_SIZE + 1;
        }

        /* dbuf_size = ((len / 16) * 66) + 67; */
    }

    /* generate the buffer */
    if (data_dump_buffer == NULL)
    {
        AllocDumpBuf();
    }

    if (data_dump_buffer == NULL)
        FatalError("Failed allocating %X bytes to data_dump_buffer!\n", data_dump_buffer_size);

    /* clean it out */
    memset(data_dump_buffer, 0x20, dbuf_size);

    /* set the byte buffer pointer to step thru the data buffer */
    data = (char*) start;

    /* set the frame pointer to the start of the printout buffer */
    frame_ptr = data_dump_buffer;

    /* initialize counters and frame index pointers */
    i = 0;
    j = 0;

    if(p && ScObfuscate() )
    {
        next_layer =  p->next_layer;
        for ( i = 0; i < next_layer; i++ )
        {
            if ( p->layers[i].proto == PROTO_IP4
                  || p->layers[i].proto == PROTO_IP6
              )
            {
                if(p->layers[i].length && p->layers[i].start)
                    break;
            }
        }
        ip_start = p->layers[i].start - start;

        if(ip_start > 0 )
        {
            ip_ob_start = ip_start + 10;
            if(p->layers[i].proto == PROTO_IP4)
                ip_ob_end = ip_ob_start + 2 + 2*(sizeof(struct in_addr));
            else
                ip_ob_end = ip_ob_start + 2 + 2*(sizeof(struct in6_addr));
        }


        i=0;
    }

    /* loop thru the whole buffer */
    while(!done)
    {
        if (ScVerboseByteDump())
        {
            d_ptr = frame_ptr + 8;
            c_ptr = (frame_ptr + 8 + C_OFFSET);
            SnortSnprintf(frame_ptr,
                          (data_dump_buffer + data_dump_buffer_size) - frame_ptr,
                          "0x%04X: ", j);
            j += 16;
        }
        else
        {
            d_ptr = frame_ptr;
            c_ptr = (frame_ptr + C_OFFSET);
        }

        /* process 16 bytes per frame */
        for(i = 0; i < 16; i++, byte_pos++)
        {
            if(ScObfuscate() && ((byte_pos >= ip_ob_start) && (byte_pos < ip_ob_end)))
            {
                *d_ptr = 'X';
                d_ptr++;
                *d_ptr = 'X';
                d_ptr++;

                *d_ptr = 0x20;
                d_ptr++;

                *c_ptr = 'X';

            }
            else
            {

                /*
                 * look up the ASCII value of the first nybble of the current
                 * data buffer
                 */
                *d_ptr = conv[((*data & 0xFF) >> 4)];
                d_ptr++;

                /* look up the second nybble */
                *d_ptr = conv[((*data & 0xFF) & 0x0F)];
                d_ptr++;

                /* put a space in between */
                *d_ptr = 0x20;
                d_ptr++;

                /* print out the char equivalent */
                if(*data > 0x1F && *data < 0x7F)
                    *c_ptr = (char) (*data & 0xFF);
                else
                    *c_ptr = 0x2E;
            }

            c_ptr++;

            /* increment the pointer or finish up */
            if(data < end)
                data++;
            else
            {
                *c_ptr = '\n';
                c_ptr++;
                *c_ptr = '\n';
                c_ptr++;
                *c_ptr = 0;

                dump_size = (int) (c_ptr - data_dump_buffer);
                fwrite(data_dump_buffer, dump_size, 1, fp);

                //ClearDumpBuf();
                return;
            }
        }

        *c_ptr = '\n';
        if (ScVerboseByteDump())
        {
            frame_ptr += (FRAME_SIZE + 8);
        }
        else
        {
            frame_ptr += FRAME_SIZE;
        }
    }

    //ClearDumpBuf();
}



/*
 * Function: PrintCharData(FILE *, char *,int)
 *
 * Purpose: Dump the ASCII data from a packet
 *          the left, decoded ASCII on the right.
 *
 * Arguments: fp => ptr to stream to print to
 *            data => pointer to buffer data
 *            data_len => length of data buffer
 *
 * Returns: void function
 */
void PrintCharData(FILE * fp, const char *data, int data_len)
{
    int bytes_processed;    /* count of bytes in the data buffer
                 * processed so far */
    int linecount = 0;      /* number of lines in this dump */
    const char *index;        /* index pointer into the data buffer */
    char *ddb_ptr;      /* index pointer into the data_dump_buffer */
    int size;

    /* if there's no data, return */
    if(data == NULL)
    {
        return;
    }

    /* setup the pointers and counters */
    bytes_processed = data_len;
    index = data;

    /* allocate a buffer to print the data to */
    //data_dump_buffer = (char *) calloc(data_len + (data_len >> 6) + 2, sizeof(char));
    if (data_dump_buffer == NULL)
    {
        AllocDumpBuf();
    }

    size = (data_len + (data_len >> 6) + 2) * sizeof(char);

    /* Based on data_len < 65535, this should never happen, but check just in
     * case sizeof(char) is big or something. */
    if (data_dump_buffer_size < size)
    {
        data_dump_buffer_size = size;
        ClearDumpBuf();

        /* Reallocate for a bigger size. */
        AllocDumpBuf();
    }

    if (data_dump_buffer == NULL)
        FatalError("Failed allocating %X bytes to data_dump_buffer!\n", data_dump_buffer_size);

    /* clean it out */
    memset(data_dump_buffer, 0x20, size);

    ddb_ptr = data_dump_buffer;

    /* loop thru the bytes in the data buffer */
    while(bytes_processed)
    {
        if(*index > 0x1F && *index < 0x7F)
        {
            *ddb_ptr = *index;
        }
        else
        {
            *ddb_ptr = '.';
        }

        if(++linecount == 64)
        {
            ddb_ptr++;
            *ddb_ptr = '\n';
            linecount = 0;
        }
        ddb_ptr++;
        index++;
        bytes_processed--;
    }

    /* slam a \n on the back */
    ddb_ptr++;
    *ddb_ptr = '\n';
    ddb_ptr++;

    /* setup the globals */

    dump_size = (int) (ddb_ptr - data_dump_buffer);
    fwrite(data_dump_buffer, dump_size, 1, fp);

    //ClearDumpBuf();
}

static int PrintObfuscatedData(FILE* fp, Packet *p)
{
    uint8_t *payload = NULL;
    uint16_t payload_len = 0;

    if (obApi->getObfuscatedPayload(p, &payload,
                (uint16_t *)&payload_len) != OB_RET_SUCCESS)
    {
        return -1;
    }

    /* dump the application layer data */
    if (ScOutputAppData() && !ScVerboseByteDump())
    {
        if (ScOutputCharData())
            PrintCharData(fp, (char *)payload, payload_len);
        else
            PrintNetData(fp, payload, payload_len, NULL);
    }
    else if (ScVerboseByteDump())
    {
        uint8_t buf[UINT16_MAX];
        uint16_t dlen = p->data - p->pkt;
        int ret;

        ret = SafeMemcpy(buf, p->pkt, dlen, buf, buf + sizeof(buf));
        if (ret != SAFEMEM_SUCCESS) 
        {
           DEBUG_WRAP(DebugMessage(DEBUG_LOG,
                   "%s: SafeMemcpy() Failed !!!",  __FUNCTION__);)
           free(payload);
           return -1;
        }

        ret = SafeMemcpy(buf + dlen, payload, payload_len,
                buf, buf + sizeof(buf));
        if (ret != SAFEMEM_SUCCESS)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_LOG,
                    "%s: SafeMemcpy() Failed !!!",  __FUNCTION__);)
            free(payload);
            return -1;
        }

        PrintNetData(fp, buf, dlen + payload_len, NULL);
    }

    fprintf(fp, "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"
            "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n\n");

    p->packet_flags |= PKT_LOGGED;

    free(payload);

    return 0;
}


/*
 * Function: PrintIPPkt(FILE *, int, Packet *)
 *
 * Purpose: Dump the packet to the stream pointer
 *
 * Arguments: fp => pointer to print data to
 *            type => packet protocol
 *            p => pointer to decoded packet struct
 *
 * Returns: void function
 */
void PrintIPPkt(FILE * fp, int type, Packet * p)
{
    char timestamp[TIMEBUF_SIZE];

    if (p->packet_flags & PKT_LOGGED)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "PrintIPPkt type = %d\n", type););

    memset((char *) timestamp, 0, TIMEBUF_SIZE);
    ts_print((struct timeval *) & p->pkth->ts, timestamp);

    /* dump the timestamp */
    fwrite(timestamp, strlen(timestamp), 1, fp);

    /* dump the ethernet header if we're doing that sort of thing */
    if(ScOutputDataLink())
    {
        Print2ndHeader(fp, p);

#ifdef MPLS
        if(p->mpls)
        {
            PrintMPLSHeader(fp, p);
        }
#endif

#ifdef GRE
        if (p->outer_iph)
        {
            PrintOuterIPHeader(fp, p);
            if (p->greh)
                PrintGREHeader(fp, p);
        }
#endif
    }

    PrintIPHeader(fp, p);

    /* if this isn't a fragment, print the other header info */
    if(!p->frag_flag)
    {
        switch(GET_IPH_PROTO(p))
        {
            case IPPROTO_TCP:
                if(p->tcph != NULL)

                {
                    PrintTCPHeader(fp, p);
                }
                else
                {
                    PrintNetData(fp, (u_char *)
                            (u_char *)p->iph + (GET_IPH_HLEN(p) << 2),
                            GET_IP_PAYLEN(p), NULL);
                }

                break;

            case IPPROTO_UDP:
                if(p->udph != NULL)
                {
                    PrintUDPHeader(fp, p);
                }
                else
                {
                    PrintNetData(fp, (u_char *)
                            (u_char *)p->iph + (GET_IPH_HLEN(p) << 2),
                            GET_IP_PAYLEN(p), NULL);
                }

                break;

            case IPPROTO_ICMP:
                if(p->icmph != NULL)
                {
                    PrintICMPHeader(fp, p);
                }
                else
                {
                    PrintNetData(fp, (u_char *)
                            ((u_char *)p->iph + (GET_IPH_HLEN(p) << 2)),
                            GET_IP_PAYLEN(p), NULL);
                }

                break;

            default:
                break;
        }
    }

    if ((p->dsize > 0) && obApi->payloadObfuscationRequired(p)
            && (PrintObfuscatedData(fp, p) == 0))
    {
        return;
    }

    /* dump the application layer data */
    if (ScOutputAppData() && !ScVerboseByteDump())
    {
        if (ScOutputCharData())
        {
            PrintCharData(fp, (char*) p->data, p->dsize);
            if(!IsJSNormData(p->ssnptr))
            {
                fprintf(fp, "%s\n", "Normalized JavaScript for this packet");
                PrintCharData(fp, (const char*)file_data_ptr.data, file_data_ptr.len);
            }
            else if(!IsGzipData(p->ssnptr))
            {
                fprintf(fp, "%s\n", "Decompressed Data for this packet");
                PrintCharData(fp, (const char*)file_data_ptr.data, file_data_ptr.len);
            }
        }
        else
        {
            PrintNetData(fp, p->data, p->dsize, NULL);
            if(!IsJSNormData(p->ssnptr))
            {
                fprintf(fp, "%s\n", "Normalized JavaScript for this packet");
                PrintNetData(fp, file_data_ptr.data, file_data_ptr.len, NULL);
            }
            else if(!IsGzipData(p->ssnptr))
            {
                fprintf(fp, "%s\n", "Decompressed Data for this packet");
                PrintNetData(fp, file_data_ptr.data, file_data_ptr.len, NULL);
            }
        }
    }
    else if (ScVerboseByteDump())
    {
        PrintNetData(fp, p->pkt, p->pkth->caplen, p);
    }

    fprintf(fp, "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"
            "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n\n");

    p->packet_flags |= PKT_LOGGED;
}



/****************************************************************************
 *
 * Function: OpenAlertFile(char *)
 *
 * Purpose: Set up the file pointer/file for alerting
 *
 * Arguments: filearg => the filename to open
 *
 * Returns: file handle
 *
 ***************************************************************************/
FILE *OpenAlertFile(const char *filearg)
{
    char filename[STD_BUF+1];
    FILE *file;
    char suffix[5];     /* filename suffix */
#ifdef WIN32
    SnortStrncpy(suffix, ".ids", sizeof(suffix));
#else
    suffix[0] = '\0';
#endif

    if(filearg == NULL)
    {
        if (snort_conf->alert_file == NULL)
        {
            if(!ScDaemonMode())
                SnortSnprintf(filename, STD_BUF, "%s/alert%s", snort_conf->log_dir, suffix);
            else
                SnortSnprintf(filename, STD_BUF, "%s/%s", snort_conf->log_dir,
                        DEFAULT_DAEMON_ALERT_FILE);
        }
        else
        {
            SnortSnprintf(filename, STD_BUF, "%s/%s%s",
                    snort_conf->log_dir, snort_conf->alert_file, suffix);
        }
    }
    else
    {
        SnortSnprintf(filename, STD_BUF, "%s", filearg);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Opening alert file: %s\n", filename););

    if((file = fopen(filename, "a")) == NULL)
    {
        FatalError("OpenAlertFile() => fopen() alert file %s: %s\n",
                   filename, strerror(errno));
    }
#ifdef WIN32
    /* Do not buffer in WIN32 */
    setvbuf(file, (char *) NULL, _IONBF, (size_t) 0);
#else
    setvbuf(file, (char *) NULL, _IOLBF, (size_t) 0);
#endif

    return file;
}

/****************************************************************************
 *
 * Function: RollAlertFile(char *)
 *
 * Purpose: rename existing alert file with by appending time to name
 *
 * Arguments: filearg => the filename to rename (same as for OpenAlertFile())
 *
 * Returns: 0=success, else errno
 *
 ***************************************************************************/
int RollAlertFile(const char *filearg)
{
    char oldname[STD_BUF+1];
    char newname[STD_BUF+1];
    char suffix[5];     /* filename suffix */
    time_t now = time(NULL);

#ifdef WIN32
    SnortStrncpy(suffix, ".ids", sizeof(suffix));
#else
    suffix[0] = '\0';
#endif

    if(filearg == NULL)
    {
        if(!ScDaemonMode())
            SnortSnprintf(oldname, STD_BUF, "%s/alert%s", snort_conf->log_dir, suffix);
        else
            SnortSnprintf(oldname, STD_BUF, "%s/%s", snort_conf->log_dir,
                    DEFAULT_DAEMON_ALERT_FILE);
    }
    else
    {
        SnortSnprintf(oldname, STD_BUF, "%s", filearg);
    }
    SnortSnprintf(newname, sizeof(newname)-1, "%s.%lu", oldname, (unsigned long)now);
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Rolling alert file: %s\n", newname););

    if ( rename(oldname, newname) )
    {
        FatalError("RollAlertFile() => rename(%s, %s) = %s\n",
                   oldname, newname, strerror(errno));
    }
    return errno;
}


/*
 *
 * Function: AllocDumpBuf()
 *
 * Purpose: Allocate the buffer that PrintNetData() uses
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void AllocDumpBuf(void)
{
    if (data_dump_buffer_size == 0)
    {
        if (ScVerboseByteDump())
        {
            data_dump_buffer_size = (((IP_MAXPACKET+1)/16) * (FRAME_SIZE + 8)) + (FRAME_SIZE + 8) + 1;
        }
        else
        {
            data_dump_buffer_size = ((IP_MAXPACKET+1)/16) * FRAME_SIZE + FRAME_SIZE + 1;
        }
    }
    data_dump_buffer = (char *)calloc( 1,data_dump_buffer_size );

    /* make sure it got allocated properly */
    if(data_dump_buffer == NULL)
    {
        FatalError("AllocDumpBuf(): Failed allocating %X bytes!\n", data_dump_buffer_size);
    }
}

/*
 *
 * Function: ClearDumpBuf()
 *
 * Purpose: Clear out the buffer that PrintNetData() generates
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void ClearDumpBuf(void)
{
    if(data_dump_buffer)
        free(data_dump_buffer);
    else
        return;

    data_dump_buffer = NULL;

    dump_size  = 0;
}

/****************************************************************************
 *
 * Function: NoAlert(Packet *, char *)
 *
 * Purpose: Don't alert at all
 *
 * Arguments: p => pointer to the packet data struct
 *            msg => the message to not print in the alert
 *
 * Returns: void function
 *
 ***************************************************************************/
void NoAlert(Packet * p, char *msg, void *arg, Event *event)
{
    return;
}


/****************************************************************************
 *
 * Function: NoLog(Packet *)
 *
 * Purpose: Don't log anything
 *
 * Arguments: p => packet to not log
 *
 * Returns: void function
 *
 ***************************************************************************/
void NoLog(Packet * p, char *msg, void *arg, Event *event)
{
    return;
}

/****************************************************************************
 *
 * Function: Print2ndHeader(FILE *, Packet p)
 *
 * Purpose: Print2ndHeader -- prints second layber  header info.
 *
 * Arguments: fp => file stream to print to
 *
 * Returns: void function
 *
 ***************************************************************************/


void Print2ndHeader(FILE * fp, Packet * p)
{

    switch(DAQ_GetBaseProtocol())
    {
        case DLT_EN10MB:        /* Ethernet */
            if(p && p->eh)
                PrintEthHeader(fp, p);
            break;
#ifndef NO_NON_ETHER_DECODER
#ifdef DLT_IEEE802_11
        case DLT_IEEE802_11:
            if(p && p->wifih)
                PrintWifiHeader(fp, p);
            break;
#endif
        case DLT_IEEE802:                /* Token Ring */
            if(p && p->trh)
                PrintTrHeader(fp, p);
            break;
#ifdef DLT_LINUX_SLL
        case DLT_LINUX_SLL:
            if (p && p->sllh)
                PrintSLLHeader(fp, p);  /* Linux cooked sockets */
            break;
#endif
#endif  // NO_NON_ETHER_DECODER
        default:
            if (ScLogVerbose())
            {
                ErrorMessage("Datalink %i type 2nd layer display is not "
                             "supported\n", DAQ_GetBaseProtocol());
            }
    }
}



#ifndef NO_NON_ETHER_DECODER
/****************************************************************************
 *
 * Function: PrintTrHeader(FILE *, Packet p)
 &
 * Purpose: Print the packet TokenRing header to the specified stream
 *
 * Arguments: fp => file stream to print to
 *
 * Returns: void function
 ***************************************************************************/

void PrintTrHeader(FILE * fp, Packet * p)
{

    fprintf(fp, "%X:%X:%X:%X:%X:%X -> ", p->trh->saddr[0],
            p->trh->saddr[1], p->trh->saddr[2], p->trh->saddr[3],
            p->trh->saddr[4], p->trh->saddr[5]);
    fprintf(fp, "%X:%X:%X:%X:%X:%X\n", p->trh->daddr[0],
            p->trh->daddr[1], p->trh->daddr[2], p->trh->daddr[3],
            p->trh->daddr[4], p->trh->daddr[5]);

    fprintf(fp, "access control:0x%X frame control:0x%X\n", p->trh->ac,
            p->trh->fc);
    if(!p->trhllc)
        return;
    fprintf(fp, "DSAP: 0x%X SSAP 0x%X protoID: %X%X%X Ethertype: %X\n",
            p->trhllc->dsap, p->trhllc->ssap, p->trhllc->protid[0],
            p->trhllc->protid[1], p->trhllc->protid[2], p->trhllc->ethertype);
    if(p->trhmr)
    {
        fprintf(fp, "RIF structure is present:\n");
        fprintf(fp, "bcast: 0x%X length: 0x%X direction: 0x%X largest"
                "fr. size: 0x%X res: 0x%X\n",
                TRH_MR_BCAST(p->trhmr), TRH_MR_LEN(p->trhmr),
        TRH_MR_DIR(p->trhmr), TRH_MR_LF(p->trhmr),
                TRH_MR_RES(p->trhmr));
        fprintf(fp, "rseg -> %X:%X:%X:%X:%X:%X:%X:%X\n",
                p->trhmr->rseg[0], p->trhmr->rseg[1], p->trhmr->rseg[2],
                p->trhmr->rseg[3], p->trhmr->rseg[4], p->trhmr->rseg[5],
                p->trhmr->rseg[6], p->trhmr->rseg[7]);
    }
}
#endif  // NO_NON_ETHER_DECODER


/****************************************************************************
 *
 * Function: PrintEthHeader(FILE *)
 *
 * Purpose: Print the packet Ethernet header to the specified stream
 *
 * Arguments: fp => file stream to print to
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintEthHeader(FILE * fp, Packet * p)
{
    /* src addr */
    fprintf(fp, "%02X:%02X:%02X:%02X:%02X:%02X -> ", p->eh->ether_src[0],
            p->eh->ether_src[1], p->eh->ether_src[2], p->eh->ether_src[3],
            p->eh->ether_src[4], p->eh->ether_src[5]);

    /* dest addr */
    fprintf(fp, "%02X:%02X:%02X:%02X:%02X:%02X ", p->eh->ether_dst[0],
            p->eh->ether_dst[1], p->eh->ether_dst[2], p->eh->ether_dst[3],
            p->eh->ether_dst[4], p->eh->ether_dst[5]);

    /* protocol and pkt size */
    fprintf(fp, "type:0x%X len:0x%X\n", ntohs(p->eh->ether_type), p->pkth->pktlen);
}

#ifdef MPLS
void PrintMPLSHeader(FILE* log, Packet* p)
{

    fprintf(log,"label:0x%05X exp:0x%X bos:0x%X ttl:0x%X\n",
            p->mplsHdr.label, p->mplsHdr.exp, p->mplsHdr.bos, p->mplsHdr.ttl);
}
#endif

#ifdef GRE
void PrintGREHeader(FILE *log, Packet *p)
{
    if (p->greh == NULL)
        return;

    fprintf(log, "GRE version:%u flags:0x%02X ether-type:0x%04X\n",
            GRE_VERSION(p->greh), p->greh->flags, GRE_PROTO(p->greh));
}
#endif

#ifndef NO_NON_ETHER_DECODER
/****************************************************************************
 *
 * Function: PrintSLLHeader(FILE *)
 *
 * Purpose: Print the packet SLL (fake) header to the specified stream (piece
 * partly is borrowed from tcpdump :))
 *
 * Arguments: fp => file stream to print to
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintSLLHeader(FILE * fp, Packet * p)
{


    switch (ntohs(p->sllh->sll_pkttype)) {
        case LINUX_SLL_HOST:
            (void)fprintf(fp, "< ");
            break;
        case LINUX_SLL_BROADCAST:
            (void)fprintf(fp, "B ");
            break;
        case LINUX_SLL_MULTICAST:
            (void)fprintf(fp, "M ");
            break;
        case LINUX_SLL_OTHERHOST:
            (void)fprintf(fp, "P ");
            break;
        case LINUX_SLL_OUTGOING:
            (void)fprintf(fp, "> ");
            break;
        default:
            (void)fprintf(fp, "? ");
            break;
        }

    /* mac addr */
    fprintf(fp, "l/l len: %i l/l type: 0x%X %02X:%02X:%02X:%02X:%02X:%02X\n",
            htons(p->sllh->sll_halen), ntohs(p->sllh->sll_hatype),
            p->sllh->sll_addr[0], p->sllh->sll_addr[1], p->sllh->sll_addr[2],
            p->sllh->sll_addr[3], p->sllh->sll_addr[4], p->sllh->sll_addr[5]);

    /* protocol and pkt size */
    fprintf(fp, "pkt type:0x%X proto: 0x%X len:0x%X\n",
                 ntohs(p->sllh->sll_pkttype),
                 ntohs(p->sllh->sll_protocol), p->pkth->pktlen);
}


void PrintArpHeader(FILE * fp, Packet * p)
{
// XXX-IPv6 "NOT YET IMPLEMENTED - printing ARP header"
}
#endif  // NO_NON_ETHER_DECODER


/****************************************************************************
 *
 * Function: PrintIPHeader(FILE *)
 *
 * Purpose: Dump the IP header info to the specified stream
 *
 * Arguments: fp => stream to print to
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintIPHeader(FILE * fp, Packet * p)
{
    if(!IPH_IS_VALID(p))
    {
        fprintf(fp, "IP header truncated\n");
        return;
    }

    PrintIpAddrs(fp, p);

    if (!ScOutputDataLink())
    {
        fputc('\n', fp);
    }
    else
    {
        fputc(' ', fp);
    }

    fprintf(fp, "%s TTL:%u TOS:0x%X ID:%u IpLen:%u DgmLen:%u",
            protocol_names[GET_IPH_PROTO(p)],
            GET_IPH_TTL(p),
            GET_IPH_TOS(p),
            IS_IP6(p) ? ntohl(GET_IPH_ID(p)) : ntohs((uint16_t)GET_IPH_ID(p)),
            GET_IPH_HLEN(p) << 2,
            GET_IP_DGMLEN(p));

    /* print the reserved bit if it's set */
    if((uint8_t)((ntohs(GET_IPH_OFF(p)) & 0x8000) >> 15) == 1)
        fprintf(fp, " RB");

    /* printf more frags/don't frag bits */
    if((uint8_t)((ntohs(GET_IPH_OFF(p)) & 0x4000) >> 14) == 1)
        fprintf(fp, " DF");

    if((uint8_t)((ntohs(GET_IPH_OFF(p)) & 0x2000) >> 13) == 1)
        fprintf(fp, " MF");

    fputc('\n', fp);

    /* print IP options */
    if(p->ip_option_count != 0)
    {
        PrintIpOptions(fp, p);
    }

    /* print fragment info if necessary */
    if(p->frag_flag)
    {
        fprintf(fp, "Frag Offset: 0x%04X   Frag Size: 0x%04X\n",
                (p->frag_offset & 0x1FFF),
                GET_IP_PAYLEN(p));
    }
}

#ifdef GRE
void PrintOuterIPHeader(FILE *fp, Packet *p)
{
    int save_family = p->family;
    IPH_API *save_api = p->iph_api;
    const IPHdr *save_iph = p->iph;
    uint8_t save_ip_option_count = p->ip_option_count;
    IP4Hdr *save_ip4h = p->ip4h;
    IP6Hdr *save_ip6h = p->ip6h;
    uint8_t save_frag_flag = p->frag_flag;
    uint16_t save_sp = p->sp, save_dp = p->dp;

    p->family = p->outer_family;
    p->iph_api = p->outer_iph_api;
    p->iph = p->outer_iph;
    p->ip_option_count = 0;
    p->ip4h = &p->outer_ip4h;
    p->ip6h = &p->outer_ip6h;
    p->frag_flag = 0;

    if (p->proto_bits & PROTO_BIT__TEREDO)
    {
        if (p->outer_udph)
        {
            p->sp = ntohs(p->outer_udph->uh_sport);
            p->dp = ntohs(p->outer_udph->uh_dport);
        }
        else
        {
            p->sp = ntohs(p->udph->uh_sport);
            p->dp = ntohs(p->udph->uh_dport);
        }
    }

    PrintIPHeader(fp, p);

    p->family = save_family;
    p->iph_api = save_api;
    p->iph = save_iph;
    p->ip_option_count = save_ip_option_count;
    p->ip4h = save_ip4h;
    p->ip6h = save_ip6h;
    p->frag_flag = save_frag_flag;

    if (p->proto_bits & PROTO_BIT__TEREDO)
    {
        p->sp = save_sp;
        p->dp = save_dp;
    }
}
#endif

/****************************************************************************
 *
 * Function: PrintTCPHeader(FILE *)
 *
 * Purpose: Dump the TCP header info to the specified stream
 *
 * Arguments: fp => file stream to print data to
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintTCPHeader(FILE * fp, Packet * p)
{
    char tcpFlags[9];

    if(p->tcph == NULL)
    {
        fprintf(fp, "TCP header truncated\n");
        return;
    }
    /* print TCP flags */
    CreateTCPFlagString(p, tcpFlags);
    fwrite(tcpFlags, 8, 1, fp); /* We don't care about the NULL */

    /* print other TCP info */
    fprintf(fp, " Seq: 0x%lX  Ack: 0x%lX  Win: 0x%X  TcpLen: %d",
            (u_long) ntohl(p->tcph->th_seq),
            (u_long) ntohl(p->tcph->th_ack),
            ntohs(p->tcph->th_win), TCP_OFFSET(p->tcph) << 2);

    if((p->tcph->th_flags & TH_URG) != 0)
    {
        fprintf(fp, "  UrgPtr: 0x%X\n", (uint16_t) ntohs(p->tcph->th_urp));
    }
    else
    {
        fputc((int) '\n', fp);
    }

    /* dump the TCP options */
    if(p->tcp_option_count != 0)
    {
        PrintTcpOptions(fp, p);
    }
}

/* Input is packet and an nine-byte (including NULL) character array.  Results
 * are put into the character array.
 */
void CreateTCPFlagString(Packet * p, char *flagBuffer)
{
    /* parse TCP flags */
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_RES1) ? '1' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_RES2) ? '2' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_URG)  ? 'U' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_ACK)  ? 'A' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_PUSH) ? 'P' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_RST)  ? 'R' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_SYN)  ? 'S' : '*');
    *flagBuffer++ = (char) ((p->tcph->th_flags & TH_FIN)  ? 'F' : '*');
    *flagBuffer = '\0';

}


/****************************************************************************
 *
 * Function: PrintUDPHeader(FILE *)
 *
 * Purpose: Dump the UDP header to the specified file stream
 *
 * Arguments: fp => file stream
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintUDPHeader(FILE * fp, Packet * p)
{

    if(p->udph == NULL)
    {
        fprintf(fp, "UDP header truncated\n");
        return;
    }
    /* not much to do here... */
    fprintf(fp, "Len: %d\n", ntohs(p->udph->uh_len) - UDP_HEADER_LEN);
}



/****************************************************************************
 *
 * Function: PrintICMPHeader(FILE *)
 *
 * Purpose: Print ICMP header
 *
 * Arguments: fp => file stream
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintICMPHeader(FILE * fp, Packet * p)
{
    /* 32 digits plus 7 colons and a NULL byte */
    char buf[8*4 + 7 + 1];

    if(p->icmph == NULL)
    {
        fprintf(fp, "ICMP header truncated\n");
        return;
    }

    fprintf(fp, "Type:%d  Code:%d  ", p->icmph->type, p->icmph->code);

    switch(p->icmph->type)
    {
        case ICMP_ECHOREPLY:
            fprintf(fp, "ID:%d  Seq:%d  ", ntohs(p->icmph->s_icmp_id),
                    ntohs(p->icmph->s_icmp_seq));
            fwrite("ECHO REPLY", 10, 1, fp);
            break;

        case ICMP_DEST_UNREACH:
            fwrite("DESTINATION UNREACHABLE: ", 25, 1, fp);
            switch(p->icmph->code)
            {
                case ICMP_NET_UNREACH:
                    fwrite("NET UNREACHABLE", 15, 1, fp);
                    break;

                case ICMP_HOST_UNREACH:
                    fwrite("HOST UNREACHABLE", 16, 1, fp);
                    break;

                case ICMP_PROT_UNREACH:
                    fwrite("PROTOCOL UNREACHABLE", 20, 1, fp);
                    break;

                case ICMP_PORT_UNREACH:
                    fwrite("PORT UNREACHABLE", 16, 1, fp);
                    break;

                case ICMP_FRAG_NEEDED:
                    fprintf(fp, "FRAGMENTATION NEEDED, DF SET\n"
                            "NEXT LINK MTU: %u",
                            ntohs(p->icmph->s_icmp_nextmtu));
                    break;

                case ICMP_SR_FAILED:
                    fwrite("SOURCE ROUTE FAILED", 19, 1, fp);
                    break;

                case ICMP_NET_UNKNOWN:
                    fwrite("NET UNKNOWN", 11, 1, fp);
                    break;

                case ICMP_HOST_UNKNOWN:
                    fwrite("HOST UNKNOWN", 12, 1, fp);
                    break;

                case ICMP_HOST_ISOLATED:
                    fwrite("HOST ISOLATED", 13, 1, fp);
                    break;

                case ICMP_PKT_FILTERED_NET:
                    fwrite("ADMINISTRATIVELY PROHIBITED NETWORK FILTERED", 44,
                            1, fp);
                    break;

                case ICMP_PKT_FILTERED_HOST:
                    fwrite("ADMINISTRATIVELY PROHIBITED HOST FILTERED", 41,
                            1, fp);
                    break;

                case ICMP_NET_UNR_TOS:
                    fwrite("NET UNREACHABLE FOR TOS", 23, 1, fp);
                    break;

                case ICMP_HOST_UNR_TOS:
                    fwrite("HOST UNREACHABLE FOR TOS", 24, 1, fp);
                    break;

                case ICMP_PKT_FILTERED:
                    fwrite("ADMINISTRATIVELY PROHIBITED,\nPACKET FILTERED", 44,
                           1, fp);
                    break;

                case ICMP_PREC_VIOLATION:
                    fwrite("PREC VIOLATION", 14, 1, fp);
                    break;

                case ICMP_PREC_CUTOFF:
                    fwrite("PREC CUTOFF", 12, 1, fp);
                    break;

                default:
                    fwrite("UNKNOWN", 7, 1, fp);
                    break;

            }

            PrintICMPEmbeddedIP(fp, p);

            break;

        case ICMP_SOURCE_QUENCH:
            fwrite("SOURCE QUENCH", 13, 1, fp);

            PrintICMPEmbeddedIP(fp, p);

            break;

        case ICMP_REDIRECT:
            fwrite("REDIRECT", 8, 1, fp);
            switch(p->icmph->code)
            {
                case ICMP_REDIR_NET:
                    fwrite(" NET", 4, 1, fp);
                    break;

                case ICMP_REDIR_HOST:
                    fwrite(" HOST", 5, 1, fp);
                    break;

                case ICMP_REDIR_TOS_NET:
                    fwrite(" TOS NET", 8, 1, fp);
                    break;

                case ICMP_REDIR_TOS_HOST:
                    fwrite(" TOS HOST", 9, 1, fp);
                    break;
            }

/* written this way since inet_ntoa was typedef'ed to use sfip_ntoa
 * which requires sfcidr_t instead of inaddr's.  This call to inet_ntoa
 * is a rare case that doesn't use sfcidr_t's. */

// XXX-IPv6 NOT YET IMPLEMENTED - IPV6 addresses technically not supported - need to change ICMP header

            sfip_raw_ntop(AF_INET, (void *)&p->icmph->s_icmp_gwaddr, buf, sizeof(buf));
            fprintf(fp, " NEW GW: %s", buf);

            PrintICMPEmbeddedIP(fp, p);

            break;

        case ICMP_ECHO:
            fprintf(fp, "ID:%d   Seq:%d  ", ntohs(p->icmph->s_icmp_id),
                    ntohs(p->icmph->s_icmp_seq));
            fwrite("ECHO", 4, 1, fp);
            break;

        case ICMP_ROUTER_ADVERTISE:
            fprintf(fp, "ROUTER ADVERTISMENT: "
                    "Num addrs: %d Addr entry size: %d Lifetime: %u",
                    p->icmph->s_icmp_num_addrs, p->icmph->s_icmp_wpa,
                    ntohs(p->icmph->s_icmp_lifetime));
            break;

        case ICMP_ROUTER_SOLICIT:
            fwrite("ROUTER SOLICITATION", 19, 1, fp);
            break;

        case ICMP_TIME_EXCEEDED:
            fwrite("TTL EXCEEDED", 12, 1, fp);
            switch(p->icmph->code)
            {
                case ICMP_TIMEOUT_TRANSIT:
                    fwrite(" IN TRANSIT", 11, 1, fp);
                    break;

                case ICMP_TIMEOUT_REASSY:
                    fwrite(" TIME EXCEEDED IN FRAG REASSEMBLY", 33, 1, fp);
                    break;
            }

            PrintICMPEmbeddedIP(fp, p);

            break;

        case ICMP_PARAMETERPROB:
            fwrite("PARAMETER PROBLEM", 17, 1, fp);
            switch(p->icmph->code)
            {
                case ICMP_PARAM_BADIPHDR:
                    fprintf(fp, ": BAD IP HEADER BYTE %u",
                            p->icmph->s_icmp_pptr);
                    break;

                case ICMP_PARAM_OPTMISSING:
                    fwrite(": OPTION MISSING", 16, 1, fp);
                    break;

                case ICMP_PARAM_BAD_LENGTH:
                    fwrite(": BAD LENGTH", 12, 1, fp);
                    break;
            }

            PrintICMPEmbeddedIP(fp, p);

            break;

        case ICMP_TIMESTAMP:
            fprintf(fp, "ID: %u  Seq: %u  TIMESTAMP REQUEST",
                    ntohs(p->icmph->s_icmp_id), ntohs(p->icmph->s_icmp_seq));
            break;

        case ICMP_TIMESTAMPREPLY:
            fprintf(fp, "ID: %u  Seq: %u  TIMESTAMP REPLY:\n"
                    "Orig: %u Rtime: %u  Ttime: %u",
                    ntohs(p->icmph->s_icmp_id), ntohs(p->icmph->s_icmp_seq),
                    p->icmph->s_icmp_otime, p->icmph->s_icmp_rtime,
                    p->icmph->s_icmp_ttime);
            break;

        case ICMP_INFO_REQUEST:
            fprintf(fp, "ID: %u  Seq: %u  INFO REQUEST",
                    ntohs(p->icmph->s_icmp_id), ntohs(p->icmph->s_icmp_seq));
            break;

        case ICMP_INFO_REPLY:
            fprintf(fp, "ID: %u  Seq: %u  INFO REPLY",
                    ntohs(p->icmph->s_icmp_id), ntohs(p->icmph->s_icmp_seq));
            break;

        case ICMP_ADDRESS:
            fprintf(fp, "ID: %u  Seq: %u  ADDRESS REQUEST",
                    ntohs(p->icmph->s_icmp_id), ntohs(p->icmph->s_icmp_seq));
            break;

        case ICMP_ADDRESSREPLY:
            fprintf(fp, "ID: %u  Seq: %u  ADDRESS REPLY: 0x%08X",
                    ntohs(p->icmph->s_icmp_id), ntohs(p->icmph->s_icmp_seq),
                    (u_int) ntohl(p->icmph->s_icmp_mask));
            break;

        default:
            fwrite("UNKNOWN", 7, 1, fp);

            break;
    }

    putc('\n', fp);

}

/****************************************************************************
 *
 * Function: PrintICMPEmbeddedIP(FILE *, Packet *)
 *
 * Purpose: Prints the original/encapsulated IP header + 64 bits of the
 *          original IP payload in an ICMP packet
 *
 * Arguments: fp => file stream
 *            p  => packet struct
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintICMPEmbeddedIP(FILE *fp, Packet *p)
{
    Packet op;
    Packet *orig_p;
    uint32_t orig_ip_hlen;

    if (fp == NULL || p == NULL)
        return;

    memset((char *) &op, 0, sizeof(Packet));
    orig_p = &op;

    orig_p->iph = p->orig_iph;
    orig_p->tcph = p->orig_tcph;
    orig_p->udph = p->orig_udph;
    orig_p->sp = p->orig_sp;
    orig_p->dp = p->orig_dp;
    orig_p->icmph = p->orig_icmph;
    orig_p->iph_api = p->orig_iph_api;
    orig_p->ip4h = p->orig_ip4h;
    orig_p->ip6h = p->orig_ip6h;
    orig_p->family = p->orig_family;

    if(orig_p->iph != NULL)
    {
        fprintf(fp, "\n** ORIGINAL DATAGRAM DUMP:\n");
        PrintIPHeader(fp, orig_p);
        orig_ip_hlen = IP_HLEN(p->orig_iph) << 2;

        switch(GET_IPH_PROTO(orig_p))
        {
            case IPPROTO_TCP:
                if(orig_p->tcph != NULL)
                    fprintf(fp, "Seq: 0x%lX\n",
                            (u_long)ntohl(orig_p->tcph->th_seq));
                break;

            case IPPROTO_UDP:
                if(orig_p->udph != NULL)
                    fprintf(fp, "Len: %d  Csum: %d\n",
                            ntohs(orig_p->udph->uh_len) - UDP_HEADER_LEN,
                            ntohs(orig_p->udph->uh_chk));
                break;

            case IPPROTO_ICMP:
                if(orig_p->icmph != NULL)
                    PrintEmbeddedICMPHeader(fp, orig_p->icmph);
                break;

            default:
                fprintf(fp, "Protocol: 0x%X (unknown or "
                        "header truncated)", GET_IPH_PROTO(orig_p));
                break;
        }       /* switch */

        /* if more than 8 bytes of original IP payload sent */
        if (p->dsize - orig_ip_hlen > 8)
        {
            fprintf(fp, "(%d more bytes of original packet)\n",
                    p->dsize - orig_ip_hlen - 8);
        }

        fprintf(fp, "** END OF DUMP");
    }
    else
    {
        fprintf(fp, "\nORIGINAL DATAGRAM TRUNCATED");
    }
}

/****************************************************************************
 *
 * Function: PrintEmbeddedICMPHeader(FILE *, ICMPHdr *)
 *
 * Purpose: Prints the 64 bits of the original IP payload in an ICMP packet
 *          that requires it
 *
 * Arguments: fp => file stream
 *            icmph  => ICMPHdr struct pointing to original ICMP
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintEmbeddedICMPHeader(FILE *fp, const ICMPHdr *icmph)
{
    if (fp == NULL || icmph == NULL)
        return;

    fprintf(fp, "Type: %d  Code: %d  Csum: %u",
            icmph->type, icmph->code, ntohs(icmph->csum));

    switch (icmph->type)
    {
        case ICMP_DEST_UNREACH:
        case ICMP_TIME_EXCEEDED:
        case ICMP_SOURCE_QUENCH:
            break;

        case ICMP_PARAMETERPROB:
            if (icmph->code == 0)
                fprintf(fp, "  Ptr: %u", icmph->s_icmp_pptr);
            break;

        case ICMP_REDIRECT:
// XXX-IPv6 "NOT YET IMPLEMENTED - ICMP printing"
            break;

        case ICMP_ECHO:
        case ICMP_ECHOREPLY:
        case ICMP_TIMESTAMP:
        case ICMP_TIMESTAMPREPLY:
        case ICMP_INFO_REQUEST:
        case ICMP_INFO_REPLY:
        case ICMP_ADDRESS:
        case ICMP_ADDRESSREPLY:
            fprintf(fp, "  Id: %u  SeqNo: %u",
                    ntohs(icmph->s_icmp_id), ntohs(icmph->s_icmp_seq));
            break;

        case ICMP_ROUTER_ADVERTISE:
            fprintf(fp, "  Addrs: %u  Size: %u  Lifetime: %u",
                    icmph->s_icmp_num_addrs, icmph->s_icmp_wpa,
                    ntohs(icmph->s_icmp_lifetime));
            break;

        default:
            break;
    }

    fprintf(fp, "\n");

    return;
}

void PrintIpOptions(FILE * fp, Packet * p)
{
    int i;
    int j;
    u_long init_offset;
    u_long print_offset;

    init_offset = ftell(fp);

    if(!p->ip_option_count || p->ip_option_count > 40)
        return;

    fprintf(fp, "IP Options (%d) => ", p->ip_option_count);

    for(i = 0; i < (int) p->ip_option_count; i++)
    {
        print_offset = ftell(fp);

        if((print_offset - init_offset) > 60)
        {
            fwrite("\nIP Options => ", 15, 1, fp);
            init_offset = ftell(fp);
        }

        switch(p->ip_options[i].code)
        {
            case IPOPT_RR:
                fwrite("RR ", 3, 1, fp);
                break;

            case IPOPT_EOL:
                fwrite("EOL ", 4, 1, fp);
                break;

            case IPOPT_NOP:
                fwrite("NOP ", 4, 1, fp);
                break;

            case IPOPT_TS:
                fwrite("TS ", 3, 1, fp);
                break;

            case IPOPT_ESEC:
                fwrite("ESEC ", 5, 1, fp);
                break;

            case IPOPT_SECURITY:
                fwrite("SEC ", 4, 1, fp);
                break;

            case IPOPT_LSRR:
            case IPOPT_LSRR_E:
                fwrite("LSRR ", 5, 1, fp);
                break;

            case IPOPT_SATID:
                fwrite("SID ", 4, 1, fp);
                break;

            case IPOPT_SSRR:
                fwrite("SSRR ", 5, 1, fp);
                break;

            case IPOPT_RTRALT:
                fwrite("RTRALT ", 7, 1, fp);
                break;

            default:
                fprintf(fp, "Opt %d: ", p->ip_options[i].code);

                if(p->ip_options[i].len)
                {
                    for(j = 0; j < p->ip_options[i].len; j++)
                    {
                        if (p->ip_options[i].data)
                            fprintf(fp, "%02X", p->ip_options[i].data[j]);
                        else
                            fprintf(fp, "%02X", 0);

                        if((j % 2) == 0)
                            fprintf(fp, " ");
                    }
                }
                break;
        }
    }

    fwrite("\n", 1, 1, fp);
}


void PrintTcpOptions(FILE * fp, Packet * p)
{
    int i;
    int j;
    u_char tmp[5];
    u_long init_offset;
    u_long print_offset;

    init_offset = ftell(fp);

    fprintf(fp, "TCP Options (%d) => ", p->tcp_option_count);

    if(p->tcp_option_count > 40 || !p->tcp_option_count)
        return;

    for(i = 0; i < (int) p->tcp_option_count; i++)
    {
        print_offset = ftell(fp);

        if((print_offset - init_offset) > 60)
        {
            fwrite("\nTCP Options => ", 16, 1, fp);
            init_offset = ftell(fp);
        }

        switch(p->tcp_options[i].code)
        {
            case TCPOPT_MAXSEG:
                memset((char *) tmp, 0, 5);
                fwrite("MSS: ", 5, 1, fp);
                if (p->tcp_options[i].data)
                    memcpy(tmp, p->tcp_options[i].data, 2);
                fprintf(fp, "%u ", EXTRACT_16BITS(tmp));
                break;

            case TCPOPT_EOL:
                fwrite("EOL ", 4, 1, fp);
                break;

            case TCPOPT_NOP:
                fwrite("NOP ", 4, 1, fp);
                break;

            case TCPOPT_WSCALE:
                if (p->tcp_options[i].data)
                    fprintf(fp, "WS: %u ", p->tcp_options[i].data[0]);
                else
                    fprintf(fp, "WS: %u ", 0);
                break;

            case TCPOPT_SACK:
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data && (p->tcp_options[i].len >= 2))
                    memcpy(tmp, p->tcp_options[i].data, 2);
                fprintf(fp, "Sack: %u@", EXTRACT_16BITS(tmp));
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data && (p->tcp_options[i].len >= 4))
                    memcpy(tmp, (p->tcp_options[i].data) + 2, 2);
                fprintf(fp, "%u ", EXTRACT_16BITS(tmp));
                break;

            case TCPOPT_SACKOK:
                fwrite("SackOK ", 7, 1, fp);
                break;

            case TCPOPT_TFO:
                fwrite("TFO ", 4, 1, fp);
                break; 

            case TCPOPT_ECHO:
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data)
                    memcpy(tmp, p->tcp_options[i].data, 4);
                fprintf(fp, "Echo: %u ", EXTRACT_32BITS(tmp));
                break;

            case TCPOPT_ECHOREPLY:
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data)
                    memcpy(tmp, p->tcp_options[i].data, 4);
                fprintf(fp, "Echo Rep: %u ", EXTRACT_32BITS(tmp));
                break;

            case TCPOPT_TIMESTAMP:
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data)
                    memcpy(tmp, p->tcp_options[i].data, 4);
                fprintf(fp, "TS: %u ", EXTRACT_32BITS(tmp));
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data)
                    memcpy(tmp, (p->tcp_options[i].data) + 4, 4);
                fprintf(fp, "%u ", EXTRACT_32BITS(tmp));
                break;

            case TCPOPT_CC:
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data)
                    memcpy(tmp, p->tcp_options[i].data, 4);
                fprintf(fp, "CC %u ", EXTRACT_32BITS(tmp));
                break;

            case TCPOPT_CCNEW:
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data)
                    memcpy(tmp, p->tcp_options[i].data, 4);
                fprintf(fp, "CCNEW: %u ", EXTRACT_32BITS(tmp));
                break;

            case TCPOPT_CCECHO:
                memset((char *) tmp, 0, 5);
                if (p->tcp_options[i].data)
                    memcpy(tmp, p->tcp_options[i].data, 4);
                fprintf(fp, "CCECHO: %u ", EXTRACT_32BITS(tmp));
                break;

            default:
                if(p->tcp_options[i].len)
                {
                    fprintf(fp, "Opt %d (%d): ", p->tcp_options[i].code,
                            (int) p->tcp_options[i].len);

                    for(j = 0; j < p->tcp_options[i].len; j++)
                    {
                        if (p->tcp_options[i].data)
                            fprintf(fp, "%02X", p->tcp_options[i].data[j]);
                        else
                            fprintf(fp, "%02X", 0);

                        if ((j + 1) % 2 == 0)
                            fprintf(fp, " ");
                    }

                    fprintf(fp, " ");
                }
                else
                {
                    fprintf(fp, "Opt %d ", p->tcp_options[i].code);
                }
                break;
        }
    }

    fwrite("\n", 1, 1, fp);
}


/*
 * Function: PrintPriorityData(FILE *)
 *
 * Purpose: Prints out priority data associated with an alert
 *
 * Arguments: fp => file descriptor to write the data to
 *            do_newline => tack a \n to the end of the line or not (bool)
 *
 * Returns: void function
 */
void PrintPriorityData(FILE *fp, int do_newline)
{
    if (otn_tmp == NULL)
        return;

    if ((otn_tmp->sigInfo.classType != NULL)
            && (otn_tmp->sigInfo.classType->name != NULL))
    {
        fprintf(fp, "[Classification: %s] ",
                otn_tmp->sigInfo.classType->name);
    }

    fprintf(fp, "[Priority: %d] ", otn_tmp->sigInfo.priority);

    if (do_newline)
        fprintf(fp, "\n");
}


/*
 * Function: PrintXrefs(FILE *)
 *
 * Purpose: Prints out cross reference data associated with an alert
 *
 * Arguments: fp => file descriptor to write the data to
 *            do_newline => tack a \n to the end of the line or not (bool)
 *
 * Returns: void function
 */
void PrintXrefs(FILE *fp, int do_newline)
{
    ReferenceNode *refNode = NULL;

    if(otn_tmp)
    {
        refNode = otn_tmp->sigInfo.refs;

        while(refNode  != NULL)
        {
            FPrintReference(fp, refNode);
            refNode = refNode->next;

            /* on the last loop through, print a newline in
               Full mode */
            if(do_newline && (refNode == NULL))
                fprintf(fp, "\n");
        }
    }
}


/* This function name is being altered for Win32 because it
   conflicts with a Win32 SDK function name.  However calls to
   this function from within Snort do not need to change because
   SetEvent() is defined in log.h to evaluate to SnortSetEvent()
   on Win32 compiles.
 */
#ifndef WIN32
void SetEvent
#else
void SnortSetEvent
#endif
       (Event *event, uint32_t generator, uint32_t id, uint32_t rev,
#if !defined(FEAT_OPEN_APPID)
        uint32_t classification, uint32_t priority, uint32_t event_ref)
#else /* defined(FEAT_OPEN_APPID) */
        uint32_t classification, uint32_t priority, uint32_t event_ref, char *event_appid)
#endif /* defined(FEAT_OPEN_APPID) */
{
    event->sig_generator = generator;
    event->sig_id = id;
    event->sig_rev = rev;
    event->classification = classification;
    event->priority = priority;
    /* this one gets set automatically */
    event->event_id = ++event_id | ScEventLogId();
    if(event_ref)
        event->event_reference = event_ref;
    else
        event->event_reference = event->event_id;
#if defined(FEAT_OPEN_APPID)

    if (event_appid)
        memcpy(event->app_name, event_appid, MAX_EVENT_APPNAME_LEN);
    else
        event->app_name[0] = 0;
#endif /* defined(FEAT_OPEN_APPID) */

    event->ref_time.tv_sec = 0;

    return;
}

#ifndef NO_NON_ETHER_DECODER
/*
 * Function: PrintEapolPkt(FILE *, Packet *)
 *
 * Purpose: Dump the packet to the stream pointer
 *
 * Arguments: fp => pointer to print data to
 *            type => packet protocol
 *            p => pointer to decoded packet struct
 *
 * Returns: void function
 */
void PrintEapolPkt(FILE * fp, Packet * p)
{
  char timestamp[TIMEBUF_SIZE];


    memset((char *) timestamp, 0, TIMEBUF_SIZE);
    ts_print((struct timeval *) & p->pkth->ts, timestamp);

    /* dump the timestamp */
    fwrite(timestamp, strlen(timestamp), 1, fp);

    /* dump the ethernet header if we're doing that sort of thing */
    if (ScOutputDataLink())
    {
        Print2ndHeader(fp, p);
    }
    PrintEapolHeader(fp, p);
    if (p->eplh->eaptype == EAPOL_TYPE_EAP) {
      PrintEAPHeader(fp, p);
    }
    else if (p->eplh->eaptype == EAPOL_TYPE_KEY) {
      PrintEapolKey(fp, p);
    }

    /* dump the application layer data */
    if(ScOutputAppData() && !ScVerboseByteDump())
    {
        if (ScOutputCharData())
            PrintCharData(fp, (char*) p->data, p->dsize);
        else
            PrintNetData(fp, p->data, p->dsize, NULL);
    }
    else if (ScVerboseByteDump())
    {
        PrintNetData(fp, p->pkt, p->pkth->caplen, p);
    }

    fprintf(fp, "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n\n");
}

/****************************************************************************
 *
 * Function: PrintWifiHeader(FILE *)
 *
 * Purpose: Print the packet 802.11 header to the specified stream
 *
 * Arguments: fp => file stream to print to
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintWifiHeader(FILE * fp, Packet * p)
{
  /* This assumes we are printing a data packet, could be changed
     to print other types as well */
  const u_char *da = NULL, *sa = NULL, *bssid = NULL, *ra = NULL,
    *ta = NULL;
  /* per table 4, IEEE802.11 section 7.2.2 */
  if ((p->wifih->frame_control & WLAN_FLAG_TODS) &&
      (p->wifih->frame_control & WLAN_FLAG_FROMDS)) {
    ra = p->wifih->addr1;
    ta = p->wifih->addr2;
    da = p->wifih->addr3;
    sa = p->wifih->addr4;
  }
  else if (p->wifih->frame_control & WLAN_FLAG_TODS) {
    bssid = p->wifih->addr1;
    sa = p->wifih->addr2;
    da = p->wifih->addr3;
  }
  else if (p->wifih->frame_control & WLAN_FLAG_FROMDS) {
    da = p->wifih->addr1;
    bssid = p->wifih->addr2;
    sa = p->wifih->addr3;
  }
  else {
    da = p->wifih->addr1;
    sa = p->wifih->addr2;
    bssid = p->wifih->addr3;
  }

  /* DO this switch to provide additional info on the type */
  switch(p->wifih->frame_control & 0x00ff)
  {
  case WLAN_TYPE_MGMT_BEACON:
    fprintf(fp, "Beacon ");
    break;
    /* management frames */
  case WLAN_TYPE_MGMT_ASREQ:
    fprintf(fp, "Assoc. Req. ");
    break;
  case WLAN_TYPE_MGMT_ASRES:
    fprintf(fp, "Assoc. Resp. ");
    break;
  case WLAN_TYPE_MGMT_REREQ:
    fprintf(fp, "Reassoc. Req. ");
    break;
  case WLAN_TYPE_MGMT_RERES:
    fprintf(fp, "Reassoc. Resp. ");
    break;
  case WLAN_TYPE_MGMT_PRREQ:
    fprintf(fp, "Probe Req. ");
    break;
  case WLAN_TYPE_MGMT_PRRES:
    fprintf(fp, "Probe Resp. ");
    break;
  case WLAN_TYPE_MGMT_ATIM:
    fprintf(fp, "ATIM ");
    break;
  case WLAN_TYPE_MGMT_DIS:
    fprintf(fp, "Dissassoc. ");
    break;
  case WLAN_TYPE_MGMT_AUTH:
    fprintf(fp, "Authent. ");
    break;
  case WLAN_TYPE_MGMT_DEAUTH:
    fprintf(fp, "Deauthent. ");
    break;

    /* Control frames */
  case WLAN_TYPE_CONT_PS:
  case WLAN_TYPE_CONT_RTS:
  case WLAN_TYPE_CONT_CTS:
  case WLAN_TYPE_CONT_ACK:
  case WLAN_TYPE_CONT_CFE:
  case WLAN_TYPE_CONT_CFACK:
    fprintf(fp, "Control ");
    break;
  }

  if (sa != NULL) {
    fprintf(fp, "%X:%X:%X:%X:%X:%X -> ", sa[0],
        sa[1], sa[2], sa[3], sa[4], sa[5]);
  }
  else if (ta != NULL) {
    fprintf(fp, "ta: %X:%X:%X:%X:%X:%X da: ", ta[0],
        ta[1], ta[2], ta[3], ta[4], ta[5]);
  }

  fprintf(fp, "%X:%X:%X:%X:%X:%X\n", da[0],
      da[1], da[2], da[3], da[4], da[5]);

  if (bssid != NULL)
  {
      fprintf(fp, "bssid: %X:%X:%X:%X:%X:%X", bssid[0],
              bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  }

  if (ra != NULL) {
    fprintf(fp, " ra: %X:%X:%X:%X:%X:%X", ra[0],
        ra[1], ra[2], ra[3], ra[4], ra[5]);
  }
  fprintf(fp, " Flags:");
  if (p->wifih->frame_control & WLAN_FLAG_TODS)    fprintf(fp," ToDs");
  if (p->wifih->frame_control & WLAN_FLAG_TODS)    fprintf(fp," FrDs");
  if (p->wifih->frame_control & WLAN_FLAG_FRAG)    fprintf(fp," Frag");
  if (p->wifih->frame_control & WLAN_FLAG_RETRY)   fprintf(fp," Re");
  if (p->wifih->frame_control & WLAN_FLAG_PWRMGMT) fprintf(fp," Pwr");
  if (p->wifih->frame_control & WLAN_FLAG_MOREDAT) fprintf(fp," MD");
  if (p->wifih->frame_control & WLAN_FLAG_WEP)   fprintf(fp," Wep");
  if (p->wifih->frame_control & WLAN_FLAG_ORDER)  fprintf(fp," Ord");
  fprintf(fp, "\n");
}

/*
 * Function: PrintWifiPkt(FILE *, Packet *)
 *
 * Purpose: Dump the packet to the stream pointer
 *
 * Arguments: fp => pointer to print data to
 *            p => pointer to decoded packet struct
 *
 * Returns: void function
 */
void PrintWifiPkt(FILE * fp, Packet * p)
{
    char timestamp[TIMEBUF_SIZE];


    memset((char *) timestamp, 0, TIMEBUF_SIZE);
    ts_print((struct timeval *) & p->pkth->ts, timestamp);

    /* dump the timestamp */
    fwrite(timestamp, strlen(timestamp), 1, fp);

    /* dump the ethernet header if we're doing that sort of thing */
    Print2ndHeader(fp, p);

    /* dump the application layer data */
    if (ScOutputAppData() && !ScVerboseByteDump())
    {
        if (ScOutputCharData())
            PrintCharData(fp, (char*) p->data, p->dsize);
        else
            PrintNetData(fp, p->data, p->dsize, NULL);
    }
    else if (ScVerboseByteDump())
    {
        PrintNetData(fp, p->pkt, p->pkth->caplen, p);
    }

    fprintf(fp, "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"
            "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n\n");
}

/****************************************************************************
 *
 * Function: PrintEapolHeader(FILE *, Packet *)
 *
 * Purpose: Dump the EAPOL header info to the specified stream
 *
 * Arguments: fp => stream to print to
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintEapolHeader(FILE * fp, Packet * p)
{
    fprintf(fp, "EAPOL type: ");
    switch(p->eplh->eaptype) {
    case EAPOL_TYPE_EAP:
      fprintf(fp, "EAP");
      break;
    case EAPOL_TYPE_START:
      fprintf(fp, "Start");
      break;
    case EAPOL_TYPE_LOGOFF:
      fprintf(fp, "Logoff");
      break;
    case EAPOL_TYPE_KEY:
      fprintf(fp, "Key");
      break;
    case EAPOL_TYPE_ASF:
      fprintf(fp, "ASF Alert");
      break;
    default:
      fprintf(fp, "Unknown");
    }
    fprintf(fp, " Len: %d\n", ntohs(p->eplh->len));
}

/****************************************************************************
 *
 * Function: PrintEAPHeader(FILE *)
 *
 * Purpose: Dump the EAP header to the specified file stream
 *
 * Arguments: fp => file stream
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintEAPHeader(FILE * fp, Packet * p)
{

    if(p->eaph == NULL)
    {
        fprintf(fp, "EAP header truncated\n");
        return;
    }
    fprintf(fp, "code: ");
    switch(p->eaph->code) {
    case EAP_CODE_REQUEST:
      fprintf(fp, "Req ");
      break;
    case EAP_CODE_RESPONSE:
      fprintf(fp, "Resp");
      break;
    case EAP_CODE_SUCCESS:
      fprintf(fp, "Succ");
      break;
    case EAP_CODE_FAILURE:
      fprintf(fp, "Fail");
      break;
    }
    fprintf(fp, " id: 0x%x len: %d", p->eaph->id, ntohs(p->eaph->len));
    if (p->eaptype != NULL) {
      fprintf(fp, " type: ");
      switch(*(p->eaptype)) {
      case EAP_TYPE_IDENTITY:
    fprintf(fp, "id");
    break;
      case EAP_TYPE_NOTIFY:
    fprintf(fp, "notify");
    break;
      case EAP_TYPE_NAK:
    fprintf(fp, "nak");
    break;
      case EAP_TYPE_MD5:
    fprintf(fp, "md5");
    break;
      case EAP_TYPE_OTP:
    fprintf(fp, "otp");
    break;
      case EAP_TYPE_GTC:
    fprintf(fp, "token");
    break;
      case EAP_TYPE_TLS:
    fprintf(fp, "tls");
    break;
      default:
    fprintf(fp, "undef");
    break;
      }
    }
    fprintf(fp, "\n");
}


/****************************************************************************
 *
 * Function: PrintEapolKey(FILE *)
 *
 * Purpose: Dump the EAP header to the specified file stream
 *
 * Arguments: fp => file stream
 *
 * Returns: void function
 *
 ***************************************************************************/
void PrintEapolKey(FILE * fp, Packet * p)
{
    uint16_t length;

    if(p->eapolk == NULL)
    {
        fprintf(fp, "Eapol Key truncated\n");
        return;
    }
    fprintf(fp, "KEY type: ");
    if (p->eapolk->type == 1) {
      fprintf(fp, "RC4");
    }

    memcpy(&length, &p->eapolk->length, 2);
    length = ntohs(length);
    fprintf(fp, " len: %d", length);
    fprintf(fp, " index: %d ", p->eapolk->index & 0x7F);
    fprintf(fp, p->eapolk->index & 0x80 ? " unicast\n" : " broadcast\n");
}
#endif  // NO_NON_ETHER_DECODER

void PrintIpAddrs(FILE *fp, Packet *p)
{
    if (!IPH_IS_VALID(p))
        return;

    if (p->frag_flag
            || ((GET_IPH_PROTO(p) != IPPROTO_TCP)
                && (GET_IPH_PROTO(p) != IPPROTO_UDP)))
    {
        char *ip_fmt = "%s -> %s";

        if (ScObfuscate())
        {
            fprintf(fp, ip_fmt,
                    ObfuscateIpToText(GET_SRC_ADDR(p)),
                    ObfuscateIpToText(GET_DST_ADDR(p)));
        }
        else
        {
            fprintf(fp, ip_fmt,
                    inet_ntoax(GET_SRC_ADDR(p)),
                    inet_ntoax(GET_DST_ADDR(p)));
        }
    }
    else
    {
        char *ip_fmt = "%s:%d -> %s:%d";

        if (ScObfuscate())
        {
            fprintf(fp, ip_fmt,
                    ObfuscateIpToText(GET_SRC_ADDR(p)), p->sp,
                    ObfuscateIpToText(GET_DST_ADDR(p)), p->dp);
        }
        else
        {
            fprintf(fp, ip_fmt,
                    inet_ntoax(GET_SRC_ADDR(p)), p->sp,
                    inet_ntoax(GET_DST_ADDR(p)), p->dp);
        }
    }
}

