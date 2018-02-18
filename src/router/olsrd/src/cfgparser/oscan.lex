%{

/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#define YYSTYPE struct conf_token *

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "olsrd_conf.h"
#include "olsrd_conf_checksum.h"

#include "oparse.h"
 
#define ECHO if(fwrite( yytext, yyleng, 1, yyout )) {}

/* Prototypes */
int yyget_lineno(void);
FILE * yyget_in(void);
FILE* yyget_out(void);
char *yyget_text(void);
void yyset_lineno(int);
void yyset_in(FILE *);
void yyset_out(FILE *);
int yyget_debug(void);
void yyset_debug(int);
int yylex_destroy(void);
int yylex(void);

static struct conf_token *get_conf_token(void);
static struct conf_token *get_string_token(const char * const s, const size_t n);
static struct conf_token *get_integer_token(const char * const s);
static struct conf_token *get_floating_token(const char * const s);
static struct conf_token *get_boolean_token(const bool b);

static struct conf_token *get_conf_token(void)
{
    struct conf_token *t = calloc(1, sizeof(struct conf_token));
    if (t == NULL) {
        fprintf(stderr, "Cannot allocate %d bytes for an configuration token.\n", (int)sizeof(struct conf_token));
    }
    return t;
}

static struct conf_token *get_string_token(const char * const s, const size_t n)
{
    struct conf_token *rv = get_conf_token();
    if (rv != NULL) {
        rv->string = malloc(n + 1);
        if (rv->string == NULL) {
          fprintf(stderr, "Cannot allocate %lu bytes for string token data.\n", (unsigned long)(n+1)); /* size_t on 64bit */
            free(rv);
            return NULL;
        }
        memcpy(rv->string, s, n);
        rv->string[n] = '\0';
    }
    return rv;
}

static struct conf_token *get_integer_token(const char * const s)
{
    struct conf_token *rv = get_conf_token();
    if (rv != NULL) {
        rv->integer = strtol(s, NULL, 0);
    }
    return rv;
}

static struct conf_token *get_floating_token(const char * const s)
{
    struct conf_token *rv = get_conf_token();
    if (rv != NULL) {
	rv->floating = 0.0;
	sscanf(s, "%f", &rv->floating);
    }
    return rv;
}

static struct conf_token *get_boolean_token(const bool b)
{
    struct conf_token *rv = get_conf_token();
    if (rv != NULL) {
        rv->boolean = b;
    }
    return rv;
}

%}

%option never-interactive
%option noalways-interactive
%option nomain
%option nostack
%option noyywrap

DECDIGIT [0-9]
FLOAT {DECDIGIT}+\.{DECDIGIT}+
HEX8 [a-fA-F0-9]
QUAD {DECDIGIT}{1,3}

IPV4ADDR {QUAD}\.{QUAD}\.{QUAD}\.{QUAD}

HEX16 {HEX8}{1,4}

IPV6PAT2 ({HEX16}:){1}:({HEX16}:){0,5}{HEX16}
IPV6PAT3 ({HEX16}:){2}:({HEX16}:){0,4}{HEX16}
IPV6PAT4 ({HEX16}:){3}:({HEX16}:){0,3}{HEX16}
IPV6PAT5 ({HEX16}:){4}:({HEX16}:){0,2}{HEX16}
IPV6PAT6 ({HEX16}:){5}:({HEX16}:){0,1}{HEX16}
IPV6PAT7 ({HEX16}:){6}:({HEX16})
IPV6PAT1 ({HEX16}:){7}{HEX16}
IPV6PAT8 ({HEX16}:){1,7}:
IPV6PAT9 ::
IPV6PAT10 :(:{HEX16}){1,7}

IPV6ADDR {IPV6PAT1}|{IPV6PAT2}|{IPV6PAT3}|{IPV6PAT4}|{IPV6PAT5}|{IPV6PAT6}|{IPV6PAT7}|{IPV6PAT8}|{IPV6PAT9}|{IPV6PAT10}

%%

\s*"#".*\n {
    current_line++;
    return TOK_COMMENT;
}

\/ {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SLASH;
}

\{ {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_OPEN;
}

\} {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_CLOSE;
}

\"[^\"]*\" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = get_string_token(yytext + 1, yyleng - 2);
    if (yylval == NULL) {
        yyterminate();
    }
    return TOK_STRING;
}

0x{HEX8}+ {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = get_integer_token(yytext);
    return TOK_INTEGER;
}

{FLOAT} {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = get_floating_token(yytext);
    return TOK_FLOAT;
}

{IPV4ADDR} {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = get_string_token(yytext, yyleng + 1);
    if (yylval == NULL) {
        yyterminate();
    }
    return TOK_IPV4_ADDR;
}
{IPV6ADDR} {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = get_string_token(yytext, yyleng + 1);
    if (yylval == NULL) {
        yyterminate();
    }
    return TOK_IPV6_ADDR;
}

"default" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_DEFAULT;
}

"auto" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_AUTO;
}

"none" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_NONE;
}

{DECDIGIT}+ {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = get_integer_token(yytext);
    return TOK_INTEGER;
}


"yes" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = get_boolean_token(true);
    return TOK_BOOLEAN;
}

"no" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = get_boolean_token(false);
    return TOK_BOOLEAN;
}

"Host" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HOSTLABEL;
}

"Net" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_NETLABEL;
}

"MaxConnections" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_MAXIPC;
}

"DebugLevel" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_DEBUGLEVEL;
}

"IpVersion" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_IPVERSION;
}

"NicChgsPollInt" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_NICCHGSPOLLRT;
}

"Hna4" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HNA4;
}

"Hna6" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HNA6;
}

"LoadPlugin" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_PLUGIN;
}

"PlParam" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_PLPARAM;
}

"Interface" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_INTERFACE;
}
"InterfaceDefaults" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_INTERFACE_DEFAULTS;
}

"AllowNoInt" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_NOINT;
}

"TosValue" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_TOS;
}


"OlsrPort" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_OLSRPORT;
}

"RtProto" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_RTPROTO;
}

"RtTable" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_RTTABLE;
}

"RtTableDefault" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_RTTABLE_DEFAULT;
}

"RtTableTunnel" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_RTTABLE_TUNNEL;
}

"RtTablePriority" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_RTTABLE_PRIORITY;
}

"RtTableDefaultOlsrPriority" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_RTTABLE_DEFAULTOLSR_PRIORITY;
}

"RtTableTunnelPriority" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_RTTABLE_TUNNEL_PRIORITY;
}

"RtTableDefaultPriority" {
  olsrd_config_checksum_add(yytext, yyleng);
  yylval = NULL;
  return TOK_RTTABLE_DEFAULT_PRIORITY;
}

"Willingness" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_WILLINGNESS;
}

"IpcConnect" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_IPCCON;
}

"FIBMetric" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_FIBMETRIC;
}

"FIBMetricDefault" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_FIBMETRICDEFAULT;
}

"UseHysteresis" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_USEHYST;
}

"HystScaling" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HYSTSCALE;
}

"HystThrHigh" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HYSTUPPER;
}

"HystThrLow" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HYSTLOWER;
}

"Pollrate" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_POLLRATE;
}


"TcRedundancy" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_TCREDUNDANCY;
}

"MprCoverage" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_MPRCOVERAGE;
}

"LinkQualityLevel" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_LQ_LEVEL;
}

"LinkQualityFishEye" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_LQ_FISH;
}

"LinkQualityAging" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_LQ_AGING;
}

"LinkQualityAlgorithm" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_LQ_PLUGIN;
}

"NatThreshold" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_LQ_NAT_THRESH;
}

"LinkQualityMult" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_LQ_MULT;
}

"MinTCVTime" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_MIN_TC_VTIME;
}

"LockFile" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_LOCK_FILE;
}

"ClearScreen" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_CLEAR_SCREEN;
}

"UseNiit" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_USE_NIIT;
}

"SmartGateway" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW;
}

"SmartGatewayAlwaysRemoveServerTunnel" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_ALWAYS_REMOVE_SERVER_TUNNEL;
}

"SmartGatewayUseCount" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_USE_COUNT;
}

"SmartGatewayTakeDownPercentage" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_TAKEDOWN_PERCENTAGE;
}

"SmartGatewayInstanceId" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_INSTANCE_ID;
}

"SmartGatewayPolicyRoutingScript" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_POLICYROUTING_SCRIPT;
}

"SmartGatewayEgressInterfaces" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_EGRESS_IFS;
}

"SmartGatewayEgressFile" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_EGRESS_FILE;
}

"SmartGatewayEgressFilePeriod" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_EGRESS_FILE_PERIOD;
}

"SmartGatewayStatusFile" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_STATUS_FILE;
}

"SmartGatewayTablesOffset" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_OFFSET_TABLES;
}

"SmartGatewayRulesOffset" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_OFFSET_RULES;
}

"SmartGatewayAllowNAT" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_ALLOW_NAT;
}

"SmartGatewayPeriod" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_PERIOD;
}

"SmartGatewayStableCount" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_STABLECOUNT;
}

"SmartGatewayThreshold" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_THRESH;
}

"SmartGatewayWeightExitLinkUp" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_WEIGHT_EXITLINK_UP;
}

"SmartGatewayWeightExitLinkDown" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_WEIGHT_EXITLINK_DOWN;
}

"SmartGatewayWeightEtx" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_WEIGHT_ETX;
}

"SmartGatewayDividerEtx" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_DIVIDER_ETX;
}

"SmartGatewayMaxCostMaxEtx" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_MAX_COST_MAX_ETX;
}

"SmartGatewayUplink" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_UPLINK;
}
 
"SmartGatewayUplinkNAT" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_UPLINK_NAT;
}
 
"SmartGatewaySpeed" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_SPEED;
}

"SmartGatewayPrefix" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SMART_GW_PREFIX;
}

"SrcIpRoutes" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SRC_IP_ROUTES;
}
"Weight" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_IFWEIGHT;
}
"MainIp" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_MAIN_IP;
}
"SetIpForward" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_SET_IPFORWARD;
}
"Ip4Broadcast" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_IP4BROADCAST;
}
"IPv4Broadcast" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_IPV4BROADCAST;
}
"IPv4Multicast" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_IPV4MULTICAST;
}
"Mode" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_IFMODE;
}
"IPv6Multicast" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_IPV6MULTICAST;
}
"IPv4Src" {
        olsrd_config_checksum_add(yytext, yyleng);
		yylval = NULL;
		return TOK_IPV4SRC;
}
"IPv6Src" {
        olsrd_config_checksum_add(yytext, yyleng);
		yylval = NULL;
		return TOK_IPV6SRC;
}
"HelloInterval" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HELLOINT;
}
"HelloValidityTime" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HELLOVAL;
}
"TcInterval" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_TCINT;
}
"TcValidityTime" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_TCVAL;
}
"MidInterval" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_MIDINT;
}
"MidValidityTime" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_MIDVAL;
}
"HnaInterval" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HNAINT;
}
"HnaValidityTime" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_HNAVAL;
}
"AutoDetectChanges" {
    olsrd_config_checksum_add(yytext, yyleng);
    yylval = NULL;
    return TOK_AUTODETCHG;
}


\n|\r\n {
    current_line++;
}

\ |\t

. {
  /* Do nothing */
  //fprintf(stderr, "Failed to parse line %d of configuration file.\n",
  //      current_line);
  //yyterminate();
  //yy_fatal_error("Parsing failed.\n");

  /* To avoid compiler warning (stupid...) */
  if(0)
    yyunput(0, NULL);
}

%%
