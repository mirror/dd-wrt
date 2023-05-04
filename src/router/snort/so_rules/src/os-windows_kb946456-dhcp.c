/*
 * VRT RULES
 * 
 * Copyright (C) 2005 Sourcefire, Inc.
 *
 * Autogen file with detection functionality by Patrick Mullen <pmullen@sourcefire.com>
 *
 * DOES NOT USE BUILT-IN DETECTION FUNCTION
 * alert udp $HOME_NET 67 -> $HOME_NET 68 (msg:"dhcp offer"; content: "|02 01 06|"; depth:3; content:"|63 82 53 63|"; offset:236; depth:4; reference:cve,2008-0084; reference:url,technet.microsoft.com/en-us/security/bulletin/ms08-004; classtype:attempted-dos;) 
 */


#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_WRAP(code) code
#else
#define DEBUG_WRAP(code)
#endif

/* declare detection functions */
int rule64225eval(void *p);

// content:"|02 01 06|", depth 3; 
static ContentInfo rule64225content1 = 
{
    (uint8_t *)"|02 01 06|", /* pattern (now in snort content format) */
    3, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule64225option0 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule64225content1
    }
};
// content:"c|82|Sc", offset 236, depth 4; 
static ContentInfo rule64225content2 = 
{
    (uint8_t *)"c|82|Sc", /* pattern (now in snort content format) */
    4, /* depth */
    236, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule64225option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule64225content2
    }
};

/* references for sid 64225 */
/* reference: cve "2008-0084"; */
static RuleReference rule64225ref1 = 
{
    "cve", /* type */
    "2008-0084" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/ms08-004"; */
static RuleReference rule64225ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/ms08-004" /* value */
};

static RuleReference *rule64225refs[] =
{
    &rule64225ref1,
    &rule64225ref2,
    NULL
};
RuleOption *rule64225options[] =
{
    &rule64225option0,
    &rule64225option1,
    NULL
};

Rule rule64225 = {
   
   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_UDP, /* proto */
       "$HOME_NET", /* SRCIP     */
       "67", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "68", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       13450, /* sigid 509f96ec-67df-42ea-99b4-817068c469a3 */
       7, /* revision f1ec963a-c869-402d-8246-7322994a610b */
   
       "attempted-dos", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "OS-WINDOWS invalid dhcp offer denial of service attempt",     /* message */
       rule64225refs /* ptr to references */
        ,NULL

   },
   rule64225options, /* ptr to rule options */
   &rule64225eval, /* use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule64225eval(void *p) {
    const uint8_t *cursor_normal = 0, *beg_of_payload = 0;
    const uint8_t *end_of_payload;

    uint32_t offered_ip, netmask, broadcast_addr;

    SFSnortPacket *sp = (SFSnortPacket *) p;

    DEBUG_WRAP(char name[] = "kb946456-dhcp");


    // content:"|02 01 06|", depth 3;
    if (contentMatch(p, rule64225options[0]->option_u.content, &cursor_normal) > 0) {

        // content:"c|82|Sc", offset 236, depth 4;  (aka "magic cookie")
        if (contentMatch(p, rule64225options[1]->option_u.content, &cursor_normal) > 0) {

           if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
               return RULE_NOMATCH;

           // Now cursor_normal points after the magic cookie, which is the first Option.
           // But first, grab the offered address from offset 16.
           // No need to verify packet boundary because this is before the cookie
           offered_ip = read_big_32(beg_of_payload + 16);

           DEBUG_WRAP(printf("%s: offered_ip = %d.%d.%d.%d\n", name,
                         (offered_ip >> 24 & 0xFF), (offered_ip >> 16 & 0xFF),
                         (offered_ip >> 8 & 0xFF), (offered_ip & 0xFF)));

           // Loop through the Options to find the Netmask (Option 0x01)
           // 0xFF is End Option
           // Options are 1-byte option, 1-byte size, data
           while((cursor_normal + 6 < end_of_payload) && (*cursor_normal != 0xFF)) {
              // If we get Netmask (Option 0x01), which has length 4...
              if(*cursor_normal == 0x01 && *(cursor_normal + 1) == 0x04) {
                 netmask = read_big_32(cursor_normal + 2);

                 DEBUG_WRAP(printf("%s: netmask = %d.%d.%d.%d\n", name,
                           (netmask >> 24 & 0xFF), (netmask >> 16 & 0xFF),
                           (netmask >> 8 & 0xFF), (netmask & 0xFF)));

                 broadcast_addr = offered_ip | ~netmask;

                 DEBUG_WRAP(printf("%s: broadcast_addr = %d.%d.%d.%d\n", name,
                           (broadcast_addr >> 24 & 0xFF), (broadcast_addr >> 16 & 0xFF),
                           (broadcast_addr >> 8 & 0xFF), (broadcast_addr & 0xFF)));

                 if(offered_ip == broadcast_addr)
                    return RULE_MATCH;
                 else
                    return RULE_NOMATCH;
              } else {
                 // Add the amount specified by the size plus the two bytes
                 // for the Option and Size
                 cursor_normal += 2 + *(cursor_normal + 1);
              }
           }
        } 
    }

    return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule64225,
    NULL
};
*/
