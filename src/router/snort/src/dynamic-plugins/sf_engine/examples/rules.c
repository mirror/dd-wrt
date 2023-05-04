/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
extern Rule sid1970;
extern Rule sid2519;
extern Rule testSid;
extern Rule sid2389;
extern Rule rule1902;
extern Rule rule2578;
extern Rule rule1026;
extern Rule rule2489;
extern Rule rule2257;
extern Rule rule_abcxyz;
extern Rule rule_ftpbounce;
extern Rule rule_httpBufferTest;
extern Rule rule_relativeContentTest;
extern Rule rule_relativeContentTest2;
extern Rule rule9999;
extern Rule rule_abcContentX2;
extern Rule rule_contentNotContent;
extern Rule rule1000;
extern Rule rule1001;
extern Rule rule109;
extern Rule rule1915;
extern Rule rule2044;
extern Rule rule3665;
extern Rule rule593;
extern Rule rule652;
extern Rule rule2922;
extern Rule rule1939;
extern Rule rule2486;
extern Rule rule3052;
extern Rule rule3036;
extern Rule rule2570;
extern Rule rule637;
extern Rule rule2313;
extern Rule rule3099;
extern Rule rule2404;
extern Rule rule12759;
extern Rule rule2527;
extern Rule rule2528;
extern Rule rule3682;
extern Rule ruleWINNY;
extern Rule ruleDHCPCAT;
extern Rule ruleIGMPIPOPTDOS;
extern Rule ruleIPOPTDOS;
extern Rule ruleSQUID_NTLM_AUTH;
extern Rule ruleMYSQL_COM_TABLE_DUMP;
extern Rule ruleMOZILLA_SSLV2_CMK;
extern Rule ruleEXCHANGE_BASE64_DECODE;
extern Rule ruleDOMINO_LDAP_INVALID_DN;
extern Rule rule_smtp_rcptto_data;
extern Rule ruleVD_OPENLDAP;
extern Rule ruleMERCUR_IMAPD_NTLMSSP;
extern Rule ruleLINUXICMPDOS;
extern Rule ruleAPACHEAUTHLDAP;
extern Rule rule1000028;
extern Rule rule_smbWriteAndX;
extern Rule rule_smbReadAndX;
//extern Rule test_rule_data;
extern Rule rule64111;
extern Rule rule64222;
extern Rule rule64333;
extern Rule testFlowbitsSet;
extern Rule testFlowbitsToggle;
extern Rule testFlowbitsIsSet;
extern Rule testFlowbitsIsNotSet;
extern Rule rule1090590;
extern Rule rule1090591;


/* This stuff should go in a more global file that has a
 * NULL terminated list of all Rule objects.  Shared lib will
 * go through that and register each rule. */
Rule *rules[] =
{
    &rule1026,
    &sid1970,
    &sid2389,
    &sid2519,
    &rule1902,
    &rule2578,
    &rule2489,
    &rule2257,
    &rule_abcxyz,
    &rule_ftpbounce,
    &rule_httpBufferTest,
    &rule_relativeContentTest,
    &rule_relativeContentTest2,
    &rule9999,
    &rule_abcContentX2,
    &rule_contentNotContent,
    &rule1000,
    &rule1001,
    &rule109,
    &rule1915,
    &rule2044,
    &rule3665,
    &rule593,
    &rule652,
    &rule2922,
    &rule1939,
    &rule2486,
    &rule3052,
    &rule3036,
    &rule2570,
    &rule637,
    &rule2313,
    &rule3099,
    &rule2404,
    &rule12759,
    &rule2527,
    &rule2528,
    //&testSid,
    &rule3682,
    &ruleWINNY,
    &ruleIGMPIPOPTDOS,
    &ruleDHCPCAT,
    &ruleIPOPTDOS,
    &ruleSQUID_NTLM_AUTH,
    &ruleMYSQL_COM_TABLE_DUMP,
    &ruleMOZILLA_SSLV2_CMK,
    &ruleEXCHANGE_BASE64_DECODE,
    &ruleDOMINO_LDAP_INVALID_DN,
    &rule_smtp_rcptto_data,
    &ruleVD_OPENLDAP,
    &ruleMERCUR_IMAPD_NTLMSSP,
    &ruleLINUXICMPDOS,
    &ruleAPACHEAUTHLDAP,
    &rule1000028,
    &rule_smbWriteAndX,
    &rule_smbReadAndX,
    //&test_rule_data,
    &rule64111,
    &rule64222,
    &rule64333,
    &testFlowbitsSet,
    &testFlowbitsToggle,
    &testFlowbitsIsSet,
    &testFlowbitsIsNotSet,
    &rule1090590,
    &rule1090591,
    NULL
};

