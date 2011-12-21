%{

/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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
    yylval = NULL;
    return TOK_SLASH;
}

\{ {
    yylval = NULL;
    return TOK_OPEN;
}

\} {
    yylval = NULL;
    return TOK_CLOSE;
}

\"[^\"]*\" {
    yylval = get_string_token(yytext + 1, yyleng - 2);
    if (yylval == NULL) {
        yyterminate();
    }
    return TOK_STRING;
}

0x{HEX8}+ {
    yylval = get_integer_token(yytext);
    return TOK_INTEGER;
}

{FLOAT} {
    yylval = get_floating_token(yytext);
    return TOK_FLOAT;
}

{IPV4ADDR} {
    yylval = get_string_token(yytext, yyleng + 1);
    if (yylval == NULL) {
        yyterminate();
    }
    return TOK_IPV4_ADDR;
}
{IPV6ADDR} {
    yylval = get_string_token(yytext, yyleng + 1);
    if (yylval == NULL) {
        yyterminate();
    }
    return TOK_IPV6_ADDR;
}

"default" {
    yylval = NULL;
    return TOK_DEFAULT;
}

"auto" {
    yylval = NULL;
    return TOK_AUTO;
}

"none" {
    yylval = NULL;
    return TOK_NONE;
}

{DECDIGIT}+ {
    yylval = get_integer_token(yytext);
    return TOK_INTEGER;
}


"yes" {
    yylval = get_boolean_token(true);
    return TOK_BOOLEAN;
}

"no" {
    yylval = get_boolean_token(false);
    return TOK_BOOLEAN;
}

"Host" {
    yylval = NULL;
    return TOK_HOSTLABEL;
}

"Net" {
    yylval = NULL;
    return TOK_NETLABEL;
}

"MaxConnections" {
    yylval = NULL;
    return TOK_MAXIPC;
}

"DebugLevel" {
    yylval = NULL;
    return TOK_DEBUGLEVEL;
}

"IpVersion" {
    yylval = NULL;
    return TOK_IPVERSION;
}

"NicChgsPollInt" {
    yylval = NULL;
    return TOK_NICCHGSPOLLRT;
}

"Hna4" {
    yylval = NULL;
    return TOK_HNA4;
}

"Hna6" {
    yylval = NULL;
    return TOK_HNA6;
}

"LoadPlugin" {
    yylval = NULL;
    return TOK_PLUGIN;
}

"PlParam" {
    yylval = NULL;
    return TOK_PLPARAM;
}

"Interface" {
    yylval = NULL;
    return TOK_INTERFACE;
}
"InterfaceDefaults" {
    yylval = NULL;
    return TOK_INTERFACE_DEFAULTS;
}

"AllowNoInt" {
    yylval = NULL;
    return TOK_NOINT;
}

"TosValue" {
    yylval = NULL;
    return TOK_TOS;
}


"OlsrPort" {
  yylval = NULL;
  return TOK_OLSRPORT;
}

"RtProto" {
  yylval = NULL;
  return TOK_RTPROTO;
}

"RtTable" {
  yylval = NULL;
  return TOK_RTTABLE;
}

"RtTableDefault" {
  yylval = NULL;
  return TOK_RTTABLE_DEFAULT;
}

"RtTableTunnel" {
  yylval = NULL;
  return TOK_RTTABLE_TUNNEL;
}

"RtTablePriority" {
  yylval = NULL;
  return TOK_RTTABLE_PRIORITY;
}

"RtTableDefaultOlsrPriority" {
  yylval = NULL;
  return TOK_RTTABLE_DEFAULTOLSR_PRIORITY;
}

"RtTableTunnelPriority" {
  yylval = NULL;
  return TOK_RTTABLE_TUNNEL_PRIORITY;
}

"RtTableDefaultPriority" {
  yylval = NULL;
  return TOK_RTTABLE_DEFAULT_PRIORITY;
}

"Willingness" {
    yylval = NULL;
    return TOK_WILLINGNESS;
}

"IpcConnect" {
    yylval = NULL;
    return TOK_IPCCON;
}

"FIBMetric" {
    yylval = NULL;
    return TOK_FIBMETRIC;
}

"UseHysteresis" {
    yylval = NULL;
    return TOK_USEHYST;
}

"HystScaling" {
    yylval = NULL;
    return TOK_HYSTSCALE;
}

"HystThrHigh" {
    yylval = NULL;
    return TOK_HYSTUPPER;
}

"HystThrLow" {
    yylval = NULL;
    return TOK_HYSTLOWER;
}

"Pollrate" {
    yylval = NULL;
    return TOK_POLLRATE;
}


"TcRedundancy" {
    yylval = NULL;
    return TOK_TCREDUNDANCY;
}

"MprCoverage" {
    yylval = NULL;
    return TOK_MPRCOVERAGE;
}

"LinkQualityLevel" {
    yylval = NULL;
    return TOK_LQ_LEVEL;
}

"LinkQualityFishEye" {
    yylval = NULL;
    return TOK_LQ_FISH;
}

"LinkQualityAging" {
    yylval = NULL;
    return TOK_LQ_AGING;
}

"LinkQualityAlgorithm" {
    yylval = NULL;
    return TOK_LQ_PLUGIN;
}

"NatThreshold" {
    yylval = NULL;
    return TOK_LQ_NAT_THRESH;
}

"LinkQualityMult" {
    yylval = NULL;
    return TOK_LQ_MULT;
}

"MinTCVTime" {
    yylval = NULL;
    return TOK_MIN_TC_VTIME;
}

"LockFile" {
    yylval = NULL;
    return TOK_LOCK_FILE;
}

"ClearScreen" {
    yylval = NULL;
    return TOK_CLEAR_SCREEN;
}

"UseNiit" {
    yylval = NULL;
    return TOK_USE_NIIT;
}

"SmartGateway" {
    yylval = NULL;
    return TOK_SMART_GW;
}

"SmartGatewayAllowNAT" {
    yylval = NULL;
    return TOK_SMART_GW_ALLOW_NAT;
}

"SmartGatewayUplink" {
    yylval = NULL;
    return TOK_SMART_GW_UPLINK;
}
 
"SmartGatewayUplinkNAT" {
    yylval = NULL;
    return TOK_SMART_GW_UPLINK_NAT;
}
 
"SmartGatewaySpeed" {
    yylval = NULL;
    return TOK_SMART_GW_SPEED;
}

"SmartGatewayPrefix" {
    yylval = NULL;
    return TOK_SMART_GW_PREFIX;
}

"SrcIpRoutes" {
    yylval = NULL;
    return TOK_SRC_IP_ROUTES;
}
"Weight" {
    yylval = NULL;
    return TOK_IFWEIGHT;
}
"MainIp" {
    yylval = NULL;
    return TOK_MAIN_IP;
}
"Ip4Broadcast" {
    yylval = NULL;
    return TOK_IP4BROADCAST;
}
"IPv4Broadcast" {
    yylval = NULL;
    return TOK_IPV4BROADCAST;
}
"IPv4Multicast" {
    yylval = NULL;
    return TOK_IPV4MULTICAST;
}
"Mode" {
    yylval = NULL;
    return TOK_IFMODE;
}
"IPv6Multicast" {
    yylval = NULL;
    return TOK_IPV6MULTICAST;
}
"IPv4Src" {
		yylval = NULL;
		return TOK_IPV4SRC;
}
"IPv6Src" {
		yylval = NULL;
		return TOK_IPV6SRC;
}
"HelloInterval" {
    yylval = NULL;
    return TOK_HELLOINT;
}
"HelloValidityTime" {
    yylval = NULL;
    return TOK_HELLOVAL;
}
"TcInterval" {
    yylval = NULL;
    return TOK_TCINT;
}
"TcValidityTime" {
    yylval = NULL;
    return TOK_TCVAL;
}
"MidInterval" {
    yylval = NULL;
    return TOK_MIDINT;
}
"MidValidityTime" {
    yylval = NULL;
    return TOK_MIDVAL;
}
"HnaInterval" {
    yylval = NULL;
    return TOK_HNAINT;
}
"HnaValidityTime" {
    yylval = NULL;
    return TOK_HNAVAL;
}
"AutoDetectChanges" {
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
