/* display.c - display frames, addresses, opcodes, etc. */

/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <atm.h>
#include <atmd.h>

#include "display.h"
#include "frame_defs.h"

#define COMPONENT "display.c"

#define MAX_TEXT 1024  /* Buffer size for displaying LE control frames */

static int my_atm2text(unsigned char *atm_addr, char *dest);
static const char *lan_dst2text(struct lan_dst *dst);
static void display_ready(void *buff);

/* Prints out more or less human readable summary of
 * LE Control Frame pointed by frame.
 */
void display_frame(void *frame)
{
        struct frame_hdr *hdr;
        struct ctrl_frame *f;
        char text[MAX_TEXT];
        char *p;

        hdr = (struct frame_hdr *)frame;
        if (hdr->opcode == htons(READY_QUERY) ||
            hdr->opcode == htons(READY_IND))
                display_ready(frame);
       
        p = text;
        p += sprintf(p, "\n");
        p += sprintf(p, "Marker         0x%x\n", ntohs(hdr->marker));
        p += sprintf(p, "Protocol       0x%x\n", hdr->protocol);
        p += sprintf(p, "Version        0x%x\n", hdr->version);
        p += sprintf(p, "Op-Code        0x%x (%s)\n", ntohs(hdr->opcode), opcode2text(hdr->opcode));
        p += sprintf(p, "Status           %d (%s)\n", hdr->status, status2text(hdr->status));
        p += sprintf(p, "Trans-ID         %d\n", ntohl(hdr->tran_id));
        p += sprintf(p, "Req Lec-ID       %d\n", ntohs(hdr->lec_id));
        p += sprintf(p, "Flags          0x%x", ntohs(hdr->flags));
        if (hdr->flags & htons(REMOTE_ADDRESS)) p+= sprintf(p, " 'Remote Address'");
        if (hdr->flags & htons(V2_CAPABLE))     p+= sprintf(p, " 'V2 Capable'");
        if (hdr->flags & htons(V2_REQUIRED))    p+= sprintf(p, " 'V2 Required'");
        if (hdr->flags & htons(PROXY_FLAG))     p+= sprintf(p, " 'Proxy Flag'");
        if (hdr->flags & htons(TOPO_CHANGE))    p+= sprintf(p, " 'Topology Change'");
        p += sprintf(p, "\n");
        
        f = (struct ctrl_frame *)frame;
        p += sprintf(p, "Source Lan     0x%x (%s)\n", ntohs(f->src_lan_dst.tag), lan_dst2text(&f->src_lan_dst));
        p += sprintf(p, "Target Lan     0x%x (%s)\n", ntohs(f->target_lan_dst.tag), lan_dst2text(&f->target_lan_dst));
        p += sprintf(p, "Source ATM     "); p += my_atm2text(f->src_atm_addr, p); p += sprintf(p, "\n");
        p += sprintf(p, "Lan type       0x%x\n", f->lan_type);
        p += sprintf(p, "Lan MTU        0x%x\n", f->max_frame_size);
        p += sprintf(p, "# of TLVs      0x%x\n", f->num_tlvs);
        p += sprintf(p, "Elan Name size 0x%x\n", f->elan_name_size);
        p += sprintf(p, "Target ATM     "); p += my_atm2text(f->target_atm_addr, p); p += sprintf(p, "\n");
        p += sprintf(p, "Elan Name          (");
        memcpy(p, f->elan_name, f->elan_name_size); p += f->elan_name_size;
        p += sprintf(p, ")\n");


        *p = '\0';
        diag(COMPONENT, DIAG_DEBUG, "%s", text);

        return;
}

static void display_ready(void *ready_frame)
{
        diag(COMPONENT, DIAG_DEBUG, "ready frame");

        return;
}

/* Poor man's atm2text */
static int my_atm2text(unsigned char *atm_addr, char *dest)
{
        int i, len;
        
        len = 0;
        for (i = 0; i < ATM_ESA_LEN; i++)
                len += sprintf(dest + len, "%2.2x ", *(atm_addr + i));
        
        return len;
}

static const char *lan_dst2text(struct lan_dst *dst)
{
        static char text[42]; /* big enough for text + MAC */
        char *p = text;

        switch(ntohs(dst->tag)) {
        case LAN_DST_NOT_PRESENT:
                sprintf(text, "Not present");
                break;
        case LAN_DST_MAC_ADDR:
                p += sprintf(p, "MAC address");
                p += sprintf(p, " ");
                mac2text(p, dst->mac);
                break;
        case LAN_DST_ROUTE_DESC:
                p += sprintf(p, "Route Descriptor");
                p += sprintf(p, " ");
                mac2text(p, dst->mac);
                break;
        default:
                sprintf(text, "<Unknown Lan destination>");
                break;
        }

        return text;
}

void mac2text(char *buff, unsigned char *mac_addr)
{
        sprintf(buff, "%02x-%02x-%02x-%02x-%02x-%02x",
                mac_addr[0], mac_addr[1], mac_addr[2], 
                mac_addr[3], mac_addr[4], mac_addr[5]);
        
        return;
}

const char *opcode2text(uint16_t opcode) {
        switch (ntohs(opcode)) {
        case LE_CONFIG_REQ:
                return "LE_CONFIG_REQUEST";
                break;
        case LE_CONFIG_RSP:
                return "LE_CONFIG_RESPONSE";
                break;
        case LE_JOIN_REQ:
                return "LE_JOIN_REQUEST";
                break;
        case LE_JOIN_RSP:
                return "LE_JOIN_RESPONSE";
                break;
        case LE_REG_REQ:
                return "LE_REGISTER_REQUEST";
                break;
        case LE_REG_RSP:
                return "LE_REGISTER_RESPONSE";
                break;
        case LE_ARP_REQ:
                return "LE_ARP_REQUEST";
                break;
        case LE_ARP_RSP:
                return "LE_ARP_RESPONSE";
                break;
        case LE_FLUSH_REQ:
                return "LE_FLUSH_REQUEST";
                break;
        case LE_FLUSH_RSP:
                return "LE_FLUSH_RESPONSE";
                break;
        case READY_QUERY:
                return "READY_QUERY";
                break;
        case READY_IND:
                return "READY_INDICATION";
                break;
        case LE_TOPO_REQ:
                return "LE_TOPOLOGY_REQUEST";
                break;
        case LE_NARP_REQ:
                return "LE_NARP_REQUEST";
                break;
        default:
                break;
        }

        return "<Unknown OP-CODE>";
}

const char *tlv2text(uint32_t type)
{
        switch (type) {
        case MAX_CUM_CTRL_TIMEOUT:
                return "Max-Cumulative-Control-Time-out";
                break;
        case MAX_UNKNOWN_FRAME_CNT:
                return "Max-Unknown-Frame-Count";
                break;
        case MAX_UNKNOWN_FRAME_TIME:
                return "Max-Unknown-Frame-Time";
                break;
        case VCC_TIMEOUT_PERIOD:
                return "VCC-Timeout-Period";
                break;
        case MAX_RETRY_COUNT:
                return "Max-Retry-Count";
                break;
        case AGING_TIME:
                return "Aging-Time";
                break;
        case FORWARD_DELAY_TIME:
                return "Forward-Delay-Time";
                break;
        case EXPECTED_LE_ARP_TIME:
                return "Expected-LE_ARP-Response-Time";
                break;
        case FLUSH_TIMEOUT:
                return "Flush-Time-out";
                break;
        case PATH_SWITCHING_DELAY:
                return "Path-Switching-Delay";
                break;
        case LOCAL_SEGMENT_ID:
                return "Local-Segment-ID";
                break;
        case DEF_MCAST_SND_VCC_TYPE:
                return "Default-Mcast-Send-VCC-Type";
                break;
        case DEF_MCAST_SND_VCC_AVG:
                return "Default-Mcast-Send-VCC-AvgRate";
                break;
        case DEF_MCAST_SEND_PEAK_RT:
                return "Default-Mcast-Send-VCC-PeakRate";
                break;
        case CONN_COMPLETION_TIMER:
                return "Connection-Completion-Timer";
                break;
        case CONFIG_FRAG_INFO:
                return "Config-Frag-Info";
                break;
        case LAYER3_ADDRESS:
                return "Layer-3-Address";
                break;
        case ELAN_ID:
                return "ELAN-ID";
                break;
        case SERVICE_CATEGORY:
                return "Service-Category";
                break;
        case LLC_MUXED_ATM_ADDR:
                return "LLC-Muxed-ATM-Address";
                break;
        case X5_ADJUSTMENT:
                return "X5-Adjustment";
                break;
        case PREFERRED_LES:
                return "Preferred-LES";
                break;
        case FORE_NAME:
                return "Fore's LANE client name";
                break;
        default:
                break;
        }

        return "<Unknown TLV type>";
}

const char *status2text(uint16_t status)
{

        switch (ntohs(status)) {
        case 0:
                return "Success";
                break;
        case 1:
                return "Version Not Supported";
                break;
        case 2:
                return "Invalid request parameters";
                break;
        case 4:
                return "Duplicate LAN Destination registration";
                break;
        case 5:
                return "Dupliate ATM address";
                break;
        case 6:
                return "Insufficient resources to grant request";
                break;
        case 7:
                return "Access denied";
                break;
        case 8:
                return "Invalid REQUESTOR-ID";
                break;
        case 9:
                return "Invalid LAN Destination";
                break;
        case 10:
                return "Invalid ATM Address";
                break;
        case 20:
                return "No Configuration";
                break;
        case 21:
                return "LE_CONFIGURE Error";
                break;
        case 22:
                return "Insufficient Information";
                break;
        case 24:
                return "TLV Not Found";
                break;
        default:
                break;
        }

        return "<Something not supported in LANEv2>";
}
