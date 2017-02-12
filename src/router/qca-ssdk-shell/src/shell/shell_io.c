/*
 * Copyright (c) 2014, 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include "shell_io.h"
#include "shell.h"

#define SW_RTN_ON_NULL_PARAM(rtn) \
    do { if ((rtn) == NULL) return SW_BAD_PARAM; } while(0);

#define DEFAULT_FLAG "default"
static char **full_cmdstrp;
static int talk_mode = 1;



int
get_talk_mode(void)
{
    return talk_mode ;
}

void
set_talk_mode(int mode)
{
    talk_mode = mode;
}

void
set_full_cmdstrp(char **cmdstrp)
{
    full_cmdstrp = cmdstrp;
}

static char *
get_cmd_buf(char *tag, char *defval)
{
    if(!full_cmdstrp || !(*full_cmdstrp))
    {
        dprintf("parameter (%s) or default (%s) absent\n", tag, defval);
        exit(1);
    }

    if (!strcasecmp(*(full_cmdstrp), DEFAULT_FLAG))
    {
        full_cmdstrp++;
        return defval;
    }
    else
    {
        return *(full_cmdstrp++);
    }
}

static char *
get_cmd_stdin(char *tag, char *defval)
{
    static char gsubcmdstr[128];
    int pos = 0;
    int c;

    if(defval)
    {
        dprintf("%s(%s): ", tag, defval);
    }
    else
    {
        dprintf("%s: ", tag);
    }

    memset(gsubcmdstr, 0, 128);

    while ((c = getchar()) != '\n')
    {
        gsubcmdstr[pos++] = c;
        if (pos == 127)
        {
            dprintf("too long command\n");
            return NULL;
        }
    }

    gsubcmdstr[pos] = '\0';
    if ('\0' == gsubcmdstr[0])
    {
        return defval;
    }
    else
    {
        return gsubcmdstr;
    }
}

static char *
get_sub_cmd(char *tag, char *defval)
{
    if(talk_mode)
        return get_cmd_stdin(tag, defval);
    else
        return get_cmd_buf(tag, defval);
}


static inline  a_bool_t
is_hex(char c)
{
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F'))
        return A_TRUE;

    return A_FALSE;
}

static inline a_bool_t
is_dec(char c)
{
    if ((c >= '0') && (c <= '9'))
        return A_TRUE;

    return A_FALSE;
}

static sw_data_type_t sw_data_type[] =
{
    SW_TYPE_DEF(SW_UINT8, cmd_data_check_uint8, cmd_data_print_uint8),
    SW_TYPE_DEF(SW_INT8, NULL, NULL),
    SW_TYPE_DEF(SW_UINT16, cmd_data_check_uint16, cmd_data_print_uint16),
    SW_TYPE_DEF(SW_INT16, NULL, NULL),
    SW_TYPE_DEF(SW_UINT32, cmd_data_check_uint32, cmd_data_print_uint32),
    SW_TYPE_DEF(SW_INT32, NULL, NULL),
    SW_TYPE_DEF(SW_UINT64, NULL, NULL),
    SW_TYPE_DEF(SW_INT64, NULL, NULL),
    SW_TYPE_DEF(SW_CAP, cmd_data_check_capable, cmd_data_print_capable),
    SW_TYPE_DEF(SW_DUPLEX, cmd_data_check_duplex, cmd_data_print_duplex),
    SW_TYPE_DEF(SW_SPEED, cmd_data_check_speed, cmd_data_print_speed),
    SW_TYPE_DEF(SW_1QMODE, cmd_data_check_1qmode, cmd_data_print_1qmode),
    SW_TYPE_DEF(SW_EGMODE, cmd_data_check_egmode, cmd_data_print_egmode),
    SW_TYPE_DEF(SW_MIB, NULL, cmd_data_print_mib),
    SW_TYPE_DEF(SW_VLAN, cmd_data_check_vlan, cmd_data_print_vlan),
    SW_TYPE_DEF(SW_PBMP, cmd_data_check_pbmp, cmd_data_print_pbmp),
    SW_TYPE_DEF(SW_ENABLE, cmd_data_check_enable, cmd_data_print_enable),
    SW_TYPE_DEF(SW_MACADDR, cmd_data_check_macaddr, cmd_data_print_macaddr),
    SW_TYPE_DEF(SW_FDBENTRY, cmd_data_check_fdbentry, cmd_data_print_fdbentry),
    SW_TYPE_DEF(SW_SCH, cmd_data_check_qos_sch, cmd_data_print_qos_sch),
    SW_TYPE_DEF(SW_QOS, cmd_data_check_qos_pt, cmd_data_print_qos_pt),
    SW_TYPE_DEF(SW_STORM, cmd_data_check_storm, cmd_data_print_storm),
    SW_TYPE_DEF(SW_STP, cmd_data_check_stp_state, cmd_data_print_stp_state),
    SW_TYPE_DEF(SW_LEAKY, cmd_data_check_leaky, cmd_data_print_leaky),
    SW_TYPE_DEF(SW_MACCMD, cmd_data_check_maccmd, cmd_data_print_maccmd),
    SW_TYPE_DEF(SW_FLOWCMD, cmd_data_check_flowcmd, cmd_data_print_flowcmd),
    SW_TYPE_DEF(SW_FLOWTYPE, cmd_data_check_flowtype, cmd_data_print_flowtype),
    SW_TYPE_DEF(SW_UINT_A, cmd_data_check_uinta, cmd_data_print_uinta),
    SW_TYPE_DEF(SW_ACLRULE, cmd_data_check_aclrule, cmd_data_print_aclrule),
    SW_TYPE_DEF(SW_LEDPATTERN, cmd_data_check_ledpattern, cmd_data_print_ledpattern),
    SW_TYPE_DEF(SW_INVLAN, cmd_data_check_invlan_mode, cmd_data_print_invlan_mode),
    SW_TYPE_DEF(SW_VLANPROPAGATION, cmd_data_check_vlan_propagation, cmd_data_print_vlan_propagation),
    SW_TYPE_DEF(SW_VLANTRANSLATION, cmd_data_check_vlan_translation, cmd_data_print_vlan_translation),
    SW_TYPE_DEF(SW_QINQMODE, cmd_data_check_qinq_mode, cmd_data_print_qinq_mode),
    SW_TYPE_DEF(SW_QINQROLE, cmd_data_check_qinq_role, cmd_data_print_qinq_role),
    SW_TYPE_DEF(SW_CABLESTATUS, NULL, cmd_data_print_cable_status),
    SW_TYPE_DEF(SW_CABLELEN, NULL, cmd_data_print_cable_len),
    SW_TYPE_DEF(SW_SSDK_CFG, NULL, cmd_data_print_ssdk_cfg),
    SW_TYPE_DEF(SW_HDRMODE, cmd_data_check_hdrmode, cmd_data_print_hdrmode),
    SW_TYPE_DEF(SW_FDBOPRATION, cmd_data_check_fdboperation, NULL),
    SW_TYPE_DEF(SW_PPPOE, cmd_data_check_pppoe, cmd_data_print_pppoe),
    SW_TYPE_DEF(SW_ACL_UDF_TYPE, cmd_data_check_udf_type, cmd_data_print_udf_type),
    SW_TYPE_DEF(SW_IP_HOSTENTRY, cmd_data_check_host_entry, cmd_data_print_host_entry),
    SW_TYPE_DEF(SW_ARP_LEARNMODE, cmd_data_check_arp_learn_mode, cmd_data_print_arp_learn_mode),
    SW_TYPE_DEF(SW_IP_GUARDMODE, cmd_data_check_ip_guard_mode, cmd_data_print_ip_guard_mode),
    SW_TYPE_DEF(SW_NATENTRY, cmd_data_check_nat_entry, cmd_data_print_nat_entry),
    SW_TYPE_DEF(SW_NAPTENTRY, cmd_data_check_napt_entry, cmd_data_print_napt_entry),
    SW_TYPE_DEF(SW_FLOWENTRY, cmd_data_check_flow_entry, cmd_data_print_flow_entry),
    SW_TYPE_DEF(SW_NAPTMODE, cmd_data_check_napt_mode, cmd_data_print_napt_mode),
    SW_TYPE_DEF(SW_IP4ADDR, cmd_data_check_ip4addr, cmd_data_print_ip4addr),
    SW_TYPE_DEF(SW_IP6ADDR, cmd_data_check_ip6addr, cmd_data_print_ip6addr),
    SW_TYPE_DEF(SW_INTFMACENTRY, cmd_data_check_intf_mac_entry, cmd_data_print_intf_mac_entry),
    SW_TYPE_DEF(SW_PUBADDRENTRY, cmd_data_check_pub_addr_entry, cmd_data_print_pub_addr_entry),
    SW_TYPE_DEF(SW_INGPOLICER, cmd_data_check_port_policer, cmd_data_print_port_policer),
    SW_TYPE_DEF(SW_EGSHAPER, cmd_data_check_egress_shaper, cmd_data_print_egress_shaper),
    SW_TYPE_DEF(SW_ACLPOLICER, cmd_data_check_acl_policer, cmd_data_print_acl_policer),
    SW_TYPE_DEF(SW_MACCONFIG, cmd_data_check_mac_config, cmd_data_print_mac_config),
    SW_TYPE_DEF(SW_PHYCONFIG, cmd_data_check_phy_config, cmd_data_print_phy_config),
    SW_TYPE_DEF(SW_FDBSMODE, cmd_data_check_fdb_smode, cmd_data_print_fdb_smode),
    SW_TYPE_DEF(SW_FX100CONFIG, cmd_data_check_fx100_config, cmd_data_print_fx100_config),
    SW_TYPE_DEF(SW_SGENTRY, cmd_data_check_multi, cmd_data_print_multi),
    SW_TYPE_DEF(SW_SEC_MAC, cmd_data_check_sec_mac, NULL),
    SW_TYPE_DEF(SW_SEC_IP, cmd_data_check_sec_ip, NULL),
    SW_TYPE_DEF(SW_SEC_IP4, cmd_data_check_sec_ip4, NULL),
    SW_TYPE_DEF(SW_SEC_IP6, cmd_data_check_sec_ip6, NULL),
    SW_TYPE_DEF(SW_SEC_TCP, cmd_data_check_sec_tcp, NULL),
    SW_TYPE_DEF(SW_SEC_UDP, cmd_data_check_sec_udp, NULL),
    SW_TYPE_DEF(SW_SEC_ICMP4, cmd_data_check_sec_icmp4, NULL),
    SW_TYPE_DEF(SW_SEC_ICMP6, cmd_data_check_sec_icmp6, NULL),
    SW_TYPE_DEF(SW_REMARKENTRY, cmd_data_check_remark_entry, cmd_data_print_remark_entry),
    SW_TYPE_DEF(SW_DEFAULT_ROUTE_ENTRY, cmd_data_check_default_route_entry, cmd_data_print_default_route_entry),
    SW_TYPE_DEF(SW_HOST_ROUTE_ENTRY, cmd_data_check_host_route_entry, cmd_data_print_host_route_entry),
    SW_TYPE_DEF(SW_IP_WCMP_ENTRY, cmd_data_check_ip_wcmp_entry, cmd_data_print_ip_wcmp_entry),
    SW_TYPE_DEF(SW_IP_RFS_IP4, cmd_data_check_ip4_rfs_entry, NULL),
	SW_TYPE_DEF(SW_IP_RFS_IP6, cmd_data_check_ip6_rfs_entry, NULL),
	SW_TYPE_DEF(SW_FLOWCOOKIE, cmd_data_check_flow_cookie, NULL),
	SW_TYPE_DEF(SW_FLOWRFS, cmd_data_check_flow_rfs, NULL),
	SW_TYPE_DEF(SW_FDB_RFS, cmd_data_check_fdb_rfs, NULL),
	SW_TYPE_DEF(SW_CROSSOVER_MODE, cmd_data_check_crossover_mode, cmd_data_print_crossover_mode),
    SW_TYPE_DEF(SW_CROSSOVER_STATUS, cmd_data_check_crossover_status, cmd_data_print_crossover_status),
    SW_TYPE_DEF(SW_PREFER_MEDIUM, cmd_data_check_prefer_medium, cmd_data_print_prefer_medium),
    SW_TYPE_DEF(SW_FIBER_MODE, cmd_data_check_fiber_mode, cmd_data_print_fiber_mode),
    SW_TYPE_DEF(SW_INTERFACE_MODE, cmd_data_check_interface_mode, cmd_data_print_interface_mode),
    SW_TYPE_DEF(SW_COUNTER_INFO, NULL, cmd_data_print_counter_info),
    SW_TYPE_DEF(SW_REG_DUMP, NULL, cmd_data_print_register_info),
    SW_TYPE_DEF(SW_DBG_REG_DUMP, NULL, cmd_data_print_debug_register_info),
};

sw_data_type_t *
cmd_data_type_find(sw_data_type_e type)
{
    a_uint16_t i = 0;

    do
    {
        if (type == sw_data_type[i].data_type)
            return &sw_data_type[i];
    }
    while ( ++i < sizeof(sw_data_type)/sizeof(sw_data_type[0]));

    return NULL;
}

sw_error_t __cmd_data_check_quit_help(char *cmd, char *usage)
{
    sw_error_t ret = SW_OK;

    if (!strncasecmp(cmd, "quit", 4)) {
        return SW_ABORTED;
    } else if (!strncasecmp(cmd, "help", 4)) {
        dprintf("%s", usage);
        ret = SW_BAD_VALUE;
    }

    return ret;
}

sw_error_t __cmd_data_check_complex(char *info, char *defval, char *usage,
				sw_error_t(*chk_func)(), void *arg_val,
				a_uint32_t size)
{
    sw_error_t ret;
    char *cmd;

    do {
        cmd = get_sub_cmd(info, defval);
        SW_RTN_ON_NULL_PARAM(cmd);

        ret = __cmd_data_check_quit_help(cmd, usage);
        if (ret == SW_ABORTED)
            return ret;
        else if (ret == SW_OK) {
            ret = chk_func(cmd, arg_val, size);
            if (ret)
                dprintf("%s", usage);
        }
    } while (talk_mode && (SW_OK != ret));

    return SW_OK;
}

sw_error_t
cmd_data_check_uint8(char *cmd_str, a_uint32_t *arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (0 == cmd_str[0])
    {
        return SW_BAD_VALUE;
    }

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%x", arg_val);
    else
        sscanf(cmd_str, "%d", arg_val);

    if (255 < *arg_val)
    {
        return SW_BAD_PARAM;
    }

    return SW_OK;
}

void
cmd_data_print_uint8(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%x", param_name, *(a_uint8_t *) buf);

}


sw_error_t
cmd_data_check_uint32(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (0 == cmd_str[0])
    {
        return SW_BAD_VALUE;
    }

    if (strspn(cmd_str, "1234567890abcdefABCDEFXx") != strlen(cmd_str)){
        return SW_BAD_VALUE;
    }

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%x", arg_val);
    else
        sscanf(cmd_str, "%d", arg_val);

    return SW_OK;
}

void
cmd_data_print_uint32(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%x", param_name, *(a_uint32_t *) buf);
}

sw_error_t
cmd_data_check_uint16(char *cmd_str, a_uint32_t *arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (0 == cmd_str[0])
    {
        return SW_BAD_VALUE;
    }

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%x", arg_val);
    else
        sscanf(cmd_str, "%d", arg_val);

    if (65535 < *arg_val)
    {
        return SW_BAD_PARAM;
    }

    return SW_OK;
}

void
cmd_data_print_uint16(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%04x", param_name, *(a_uint16_t *) buf);

}

sw_error_t
cmd_data_check_pbmp(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%x", arg_val);
    else
        sscanf(cmd_str, "%d", arg_val);

    return SW_OK;

}

void
cmd_data_print_pbmp(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%x", param_name, *(a_uint32_t *) buf);

}

sw_error_t
cmd_data_check_enable(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "disable"))
        *arg_val = FAL_DISABLE;
    else if (!strcasecmp(cmd_str, "enable"))
        *arg_val = FAL_ENABLE;
    else
    {
        //dprintf("input error");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_enable(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == 1)
    {
        dprintf("ENABLE");
    }
    else if (*(a_uint32_t *) buf == 0)
    {
        dprintf("DISABLE");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

/*mib*/
static char *mib_regname[] =
{
    "RxBroad",
    "RxPause",
    "RxMulti",
    "RxFcsErr",
    "RxAlignErr",
    "RxRunt",
    "RxFragment",
    "Rx64Byte",
    "Rx128Byte",
    "Rx256Byte",
    "Rx512Byte",
    "Rx1024Byte",
    "Rx1518Byte",
    "RxMaxByte",
    "RxTooLong",
    "RxGoodByte",
    "RxGoodByte1",
    "RxBadByte",
    "RxBadByte1",
    "RxOverFlow",
    "Filtered",
    "TxBroad",
    "TxPause",
    "TxMulti",
    "TxUnderRun",
    "Tx64Byte",
    "Tx128Byte",
    "Tx256Byte",
    "Tx512Byte",
    "Tx1024Byte",
    "Tx1518Byte",
    "TxMaxByte",
    "TxOverSize",
    "TxByte",
    "TxByte1",
    "TxCollision",
    "TxAbortCol",
    "TxMultiCol",
    "TxSingleCol",
    "TxExcDefer",
    "TxDefer",
    "TxLateCol",
    "RxUniCast",
    "TxUniCast"
};

void
cmd_data_print_mib(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("\n[%s] \n", param_name);
    a_uint32_t offset = 0;
    for (offset = 0; offset < (sizeof (fal_mib_info_t) / sizeof (a_uint32_t));
            offset++)
    {

        dprintf("%-12s<0x%08x>  ", mib_regname[offset], *(buf + offset));
        if ((offset + 1) % 3 == 0)
            dprintf("\n");
    }
}

/*port counter*/
static char *counter_regname[] =
{
    "RxGoodFrame",
    "RxBadCRC   ",
    "TxGoodFrame",
    "TxBadCRC   ",
};

void
cmd_data_print_counter_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("\n[%s] \n", param_name);
    a_uint32_t offset = 0;
    for (offset = 0; offset < (sizeof (fal_port_counter_info_t) / sizeof (a_uint32_t));
            offset++)
    {

        dprintf("%s<0x%08x>\n", counter_regname[offset], *(buf + offset));

    }
}

void
cmd_data_print_debug_register_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("\n[%s]", param_name);
	fal_debug_reg_dump_t * reg_dump = (fal_debug_reg_dump_t * )buf;

	a_uint32_t reg_count;

	dprintf("\n%s. ", reg_dump->reg_name);

	reg_count = 0;
	dprintf("\n");
	for (;reg_count < reg_dump->reg_count;reg_count++)
	{
		dprintf("%08x:%08x  ",reg_dump->reg_addr[reg_count], reg_dump->reg_value[reg_count]);
		if ((reg_count + 1) % 4 == 0)
			dprintf("\n");
	}

	dprintf("\n\n\n");
}



void
cmd_data_print_register_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("\n[%s]", param_name);
	fal_reg_dump_t * reg_dump = (fal_reg_dump_t * )buf;

	a_uint32_t n[8]={0,4,8,0xc,0x10,0x14,0x18,0x1c};

	a_uint32_t i;
	a_uint32_t dump_addr, reg_count, reg_val;

	dprintf("\n%s. ", reg_dump->reg_name);
	dprintf("\n	%8x %8x %8x %8x %8x %8x %8x %8x\n",
					n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]);
	dprintf(" [%04x] ", reg_dump->reg_base);

	reg_count = 0;
	for (dump_addr = reg_dump->reg_base;
			(dump_addr <= reg_dump->reg_end )&& (reg_count <= reg_dump->reg_count);
			reg_count++)
	{
		dprintf("%08x ", reg_dump->reg_value[reg_count]);
		dump_addr += 4;
		if ((reg_count + 1) % 8 == 0)
			dprintf("\n [%04x] ", dump_addr);
	}

	dprintf("\n\n\n");
}


/*port ctrl*/
sw_error_t
cmd_data_check_duplex(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "half"))
        *arg_val = FAL_HALF_DUPLEX;
    else if (!strcasecmp(cmd_str, "full"))
        *arg_val = FAL_FULL_DUPLEX;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_duplex(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == 0)
    {
        dprintf("HALF");
    }
    else if (*(a_uint32_t *) buf == 1)
    {
        dprintf("FULL");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_speed(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strncasecmp(cmd_str, "10", 3))
        *arg_val = FAL_SPEED_10;
    else if (!strncasecmp(cmd_str, "100", 4))
        *arg_val = FAL_SPEED_100;
    else if (!strncasecmp(cmd_str, "1000", 5))
        *arg_val = FAL_SPEED_1000;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_speed(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_SPEED_10)
    {
        dprintf("10(Mbps)");
    }
    else if (*(a_uint32_t *) buf == FAL_SPEED_100)
    {
        dprintf("100(Mbps)");
    }
    else if (*(a_uint32_t *) buf == FAL_SPEED_1000)
    {
        dprintf("1000(Mbps)");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_capable(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    cmd_strtol(cmd_str, arg_val);
    if (*arg_val & (~FAL_PHY_COMBO_ADV_ALL))
    {
        //dprintf("input error should be within 0x3f\n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_capable(a_uint8_t * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    dprintf("[%s]:", param_name);

    if (*(a_uint32_t *) buf == 0)
    {
        dprintf("None Capable");
        return;
    }

    if (*(a_uint32_t *) buf & FAL_PHY_ADV_1000BX_FD)
    {
        dprintf("1000BX_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_1000BX_HD)
    {
        dprintf("1000BX_HD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_1000T_FD)
    {
        dprintf("1000T_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_100TX_FD)
    {
        dprintf("100TX_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_100TX_HD)
    {
        dprintf("100TX_HD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_10T_HD)
    {
        dprintf("10T_HD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_10T_FD)
    {
        dprintf("10T_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_PAUSE)
    {
        dprintf("PAUSE|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_ASY_PAUSE)
    {
        dprintf("ASY_PAUSE|");
    }
}

sw_error_t
cmd_data_check_crossover_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strncasecmp(cmd_str, "auto", 5))
        *arg_val = PHY_MDIX_AUTO;
    else if (!strncasecmp(cmd_str, "mdi", 4))
        *arg_val = PHY_MDIX_MDI;
    else if (!strncasecmp(cmd_str, "mdix", 5))
        *arg_val = PHY_MDIX_MDIX;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_crossover_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == PHY_MDIX_AUTO)
    {
        dprintf("AUTO");
    }
    else if (*(a_uint32_t *) buf == PHY_MDIX_MDI)
    {
        dprintf("MDI");
    }
    else if (*(a_uint32_t *) buf == PHY_MDIX_MDIX)
    {
        dprintf("MDIX");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_crossover_status(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;
    if (!strncasecmp(cmd_str, "mdi", 4))
        *arg_val = PHY_MDIX_STATUS_MDI;
    else if (!strncasecmp(cmd_str, "mdix", 5))
        *arg_val = PHY_MDIX_STATUS_MDIX;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_crossover_status(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == PHY_MDIX_STATUS_MDI)
    {
        dprintf("MDI");
    }
    else if (*(a_uint32_t *) buf == PHY_MDIX_STATUS_MDIX)
    {
        dprintf("MDIX");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_prefer_medium(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;
    if (!strncasecmp(cmd_str, "copper", 7))
        *arg_val = PHY_MEDIUM_COPPER;
    else if (!strncasecmp(cmd_str, "fiber", 6))
        *arg_val = PHY_MEDIUM_FIBER;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_prefer_medium(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == PHY_MEDIUM_COPPER)
    {
        dprintf("COPPER");
    }
    else if (*(a_uint32_t *) buf == PHY_MEDIUM_FIBER)
    {
        dprintf("FIBER");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_fiber_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;
    if (!strncasecmp(cmd_str, "100fx", 6))
        *arg_val = PHY_FIBER_100FX;
    else if (!strncasecmp(cmd_str, "1000bx", 7))
        *arg_val = PHY_FIBER_1000BX;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_fiber_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == PHY_FIBER_100FX)
    {
        dprintf("100FX");
    }
    else if (*(a_uint32_t *) buf == PHY_FIBER_1000BX)
    {
        dprintf("1000BX");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_interface_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strncasecmp(cmd_str, "psgmii_baset", 13))
        *arg_val = PHY_PSGMII_BASET;
    else if (!strncasecmp(cmd_str, "psgmii_bx1000", 14))
        *arg_val = PHY_PSGMII_BX1000;
    else if (!strncasecmp(cmd_str, "psgmii_fx100", 13))
        *arg_val = PHY_PSGMII_FX100;
    else if (!strncasecmp(cmd_str, "psgmii_amdet", 13))
        *arg_val = PHY_PSGMII_AMDET;
    else if (!strncasecmp(cmd_str, "sgmii_baset", 13))
        *arg_val = PHY_SGMII_BASET;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_interface_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == PHY_PSGMII_BASET)
    {
        dprintf("PSGMII_BASET");
    }
    else if (*(a_uint32_t *) buf == PHY_PSGMII_BX1000)
    {
        dprintf("PSGMII_BX1000");
    }
    else if (*(a_uint32_t *) buf == PHY_PSGMII_FX100)
    {
        dprintf("PSGMII_FX100");
    }
        else if (*(a_uint32_t *) buf == PHY_PSGMII_AMDET)
    {
        dprintf("PSGMII_AMDET");
    }
        else if (*(a_uint32_t *) buf == PHY_SGMII_BASET)
    {
        dprintf("SGMII_BASET");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

/*portvlan*/
sw_error_t
cmd_data_check_1qmode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "disable"))
    {
        *arg_val = FAL_1Q_DISABLE;
    }
    else if (!strcasecmp(cmd_str, "secure"))
    {
        *arg_val = FAL_1Q_SECURE;
    }
    else if (!strcasecmp(cmd_str, "check"))
    {
        *arg_val = FAL_1Q_CHECK;
    }
    else if (!strcasecmp(cmd_str, "fallback"))
    {
        *arg_val = FAL_1Q_FALLBACK;
    }
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_1qmode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_1Q_DISABLE)
    {
        dprintf("DISABLE\n");
    }
    else if (*(a_uint32_t *) buf == FAL_1Q_SECURE)
    {
        dprintf("SECURE\n");
    }
    else if (*(a_uint32_t *) buf == FAL_1Q_CHECK)
    {
        dprintf("CHECK\n");
    }
    else if (*(a_uint32_t *) buf == FAL_1Q_FALLBACK)
    {
        dprintf("FALLBACK\n");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_egmode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "unmodified"))
    {
        *arg_val = FAL_EG_UNMODIFIED;
    }
    else if (!strcasecmp(cmd_str, "untagged"))
    {
        *arg_val = FAL_EG_UNTAGGED;
    }
    else if (!strcasecmp(cmd_str, "tagged"))
    {
        *arg_val = FAL_EG_TAGGED;
    }
    else if (!strcasecmp(cmd_str, "hybrid"))
    {
        *arg_val = FAL_EG_HYBRID;
    }
    else if (!strcasecmp(cmd_str, "untouched"))
    {
        *arg_val = FAL_EG_UNTOUCHED;
    }
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_egmode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_EG_UNMODIFIED)
    {
        dprintf("UNMODIFIED");
    }
    else if (*(a_uint32_t *) buf == FAL_EG_UNTAGGED)
    {
        dprintf("UNTAGGED");
    }
    else if (*(a_uint32_t *) buf == FAL_EG_TAGGED)
    {
        dprintf("TAGGED");
    }
    else if (*(a_uint32_t *) buf == FAL_EG_HYBRID)
    {
        dprintf("HYBRID");
    }
    else if (*(a_uint32_t *) buf == FAL_EG_UNTOUCHED)
    {
        dprintf("UNTOUCHED");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

/*vlan*/
sw_error_t
cmd_data_check_vlan(char *cmdstr, fal_vlan_t * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_vlan_t entry;
    a_uint32_t tmp;

    memset(&entry, 0, sizeof (fal_vlan_t));

    do
    {
        cmd = get_sub_cmd("vlanid", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 4095\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 0 -- 4095\n");
        }

    }
    while (talk_mode && (SW_OK != rv));
    entry.vid = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("fid", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 4095 or 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 0 -- 4095 or 65535\n");
        }

    }
    while (talk_mode && (SW_OK != rv));
    entry.fid = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("port member", "null");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: input port number such as 1,3\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_portmap(cmd, &entry.mem_ports,
                                        sizeof (fal_pbmp_t));
            if (SW_OK != rv)
                dprintf("usage: input port number such as 1,3\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("tagged member", "null");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: input port number such as 1,3\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_portmap(cmd, &entry.tagged_ports,
                                        sizeof (fal_pbmp_t));
            if (SW_OK != rv)
                dprintf("usage: input port number such as 1,3\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("untagged member", "null");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: input port number such as 1,3\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_portmap(cmd, &entry.untagged_ports,
                                        sizeof (fal_pbmp_t));
            if (SW_OK != rv)
                dprintf("usage: input port number such as 1,3\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("unmodify member", "null");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: input port number such as 1,3\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_portmap(cmd, &entry.unmodify_ports,
                                        sizeof (fal_pbmp_t));
            if (SW_OK != rv)
                dprintf("usage: input port number such as 1,3\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("learn disable", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.learn_dis,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("queue override", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.vid_pri_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == entry.vid_pri_en)
    {
        do
        {
            cmd = get_sub_cmd("queue", NULL);
            SW_RTN_ON_NULL_PARAM(cmd);

            if (!strncasecmp(cmd, "quit", 4))
            {
                return SW_BAD_VALUE;
            }
            else if (!strncasecmp(cmd, "help", 4))
            {
                dprintf("usage: input number such as <0/1/2/3>\n");
                rv = SW_BAD_VALUE;
            }
            else
            {
                rv = cmd_data_check_uint32(cmd, &tmp, sizeof (a_uint32_t));
                if (SW_OK != rv)
                    dprintf("usage: input number such as <0/1/2/3>\n");
            }

        }
        while (talk_mode && (SW_OK != rv));
        entry.vid_pri = tmp;
    }

    *val = entry;
    return SW_OK;
}

void
cmd_data_print_vlan(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_vlan_t *sw_vlan = (fal_vlan_t *) buf;

    dprintf("\n[vid]:%-4d  [fid]:%-5d  [member]:0x%-4x",
            sw_vlan->vid, sw_vlan->fid, sw_vlan->mem_ports);

    dprintf("\n[tagged_member]:0x%-4x  [untagged_member]:0x%-4x  [unmodify_member]:0x%-4x  ",
            sw_vlan->tagged_ports, sw_vlan->untagged_ports, sw_vlan->unmodify_ports);

    if (sw_vlan->learn_dis == 1)
    {
        dprintf("[learn_dis]:enable   ");
    }
    else
    {
        dprintf("[learn_dis]:disable  ");
    }

    if (sw_vlan->vid_pri_en == 1)
    {
        dprintf("[pri_en]:enable  [pri]:0x%-4x\n", sw_vlan->vid_pri);
    }
    else
    {
        dprintf("[pri_en]:disable [pri]:0x%-4x\n", 0);
    }
}

/*qos*/
sw_error_t
cmd_data_check_qos_sch(char *cmdstr, fal_sch_mode_t * val, a_uint32_t size)
{
    if (cmdstr == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmdstr, "sp"))
    {
        *val = FAL_SCH_SP_MODE;
    }
    else if (!strcasecmp(cmdstr, "wrr"))
    {
        *val = FAL_SCH_WRR_MODE;
    }
    else if (!strcasecmp(cmdstr, "mixplus"))
    {
        *val = FAL_SCH_MIX_PLUS_MODE;
    }
    else if (!strcasecmp(cmdstr, "mix"))
    {
        *val = FAL_SCH_MIX_MODE;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_qos_sch(a_uint8_t * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_SCH_SP_MODE)
    {
        dprintf("SP");
    }
    else if (*(a_uint32_t *) buf == FAL_SCH_WRR_MODE)
    {
        dprintf("WRR");
    }
    else if (*(a_uint32_t *) buf == FAL_SCH_MIX_MODE)
    {
        dprintf("MIX");
    }
    else if (*(a_uint32_t *) buf == FAL_SCH_MIX_PLUS_MODE)
    {
        dprintf("MIXPLUS");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_qos_pt(char *cmdstr, fal_qos_mode_t * val, a_uint32_t size)
{
    if (cmdstr == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmdstr, "da"))
    {
        *val = FAL_QOS_DA_MODE;
    }
    else if (!strcasecmp(cmdstr, "up"))
    {
        *val = FAL_QOS_UP_MODE;
    }
    else if (!strcasecmp(cmdstr, "dscp"))
    {
        *val = FAL_QOS_DSCP_MODE;
    }
    else if (!strcasecmp(cmdstr, "port"))
    {
        *val = FAL_QOS_PORT_MODE;
    }
    else if (!strcasecmp(cmdstr, "flow"))
    {
        *val = FAL_QOS_FLOW_MODE;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_qos_pt(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_QOS_DA_MODE)
    {
        dprintf("DA");
    }
    else if (*(a_uint32_t *) buf == FAL_QOS_UP_MODE)
    {
        dprintf("UP");
    }
    else if (*(a_uint32_t *) buf == FAL_QOS_DSCP_MODE)
    {
        dprintf("DSCP");
    }
    else if (*(a_uint32_t *) buf == FAL_QOS_PORT_MODE)
    {
        dprintf("PORT");
    }
    else if (*(a_uint32_t *) buf == FAL_QOS_FLOW_MODE)
    {
        dprintf("FLOW");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

/*rate*/
sw_error_t
cmd_data_check_storm(char *cmdstr, fal_storm_type_t * val, a_uint32_t size)
{
    if (cmdstr == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmdstr, "unicast"))
    {
        *val = FAL_UNICAST_STORM;
    }
    else if (!strcasecmp(cmdstr, "multicast"))
    {
        *val = FAL_MULTICAST_STORM;
    }
    else if (!strcasecmp(cmdstr, "broadcast"))
    {
        *val = FAL_BROADCAST_STORM;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_storm(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_UNICAST_STORM)
    {
        dprintf("UNICAST");
    }
    else if (*(a_uint32_t *) buf == FAL_MULTICAST_STORM)
    {
        dprintf("MULTICAST");
    }
    else if (*(a_uint32_t *) buf == FAL_BROADCAST_STORM)
    {
        dprintf("BROADCAST");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

/*stp*/
sw_error_t
cmd_data_check_stp_state(char *cmdstr, fal_stp_state_t * val, a_uint32_t size)
{
    if (cmdstr == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmdstr, "disable"))
    {
        *val = FAL_STP_DISABLED;
    }
    else if (!strcasecmp(cmdstr, "block"))
    {
        *val = FAL_STP_BLOKING;
    }
    else if (!strcasecmp(cmdstr, "listen"))
    {
        *val = FAL_STP_LISTENING;
    }
    else if (!strcasecmp(cmdstr, "learn"))
    {
        *val = FAL_STP_LEARNING;
    }
    else if (!strcasecmp(cmdstr, "forward"))
    {
        *val = FAL_STP_FARWARDING;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_stp_state(a_uint8_t * param_name, a_uint32_t * buf,
                         a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_STP_DISABLED)
    {
        dprintf("DISABLE");
    }
    else if (*(a_uint32_t *) buf == FAL_STP_BLOKING)
    {
        dprintf("BLOCK");
    }
    else if (*(a_uint32_t *) buf == FAL_STP_LISTENING)
    {
        dprintf("LISTEN");
    }
    else if (*(a_uint32_t *) buf == FAL_STP_LEARNING)
    {
        dprintf("LEARN");
    }
    else if (*(a_uint32_t *) buf == FAL_STP_FARWARDING)
    {
        dprintf("FORWARD");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

/*general*/
sw_error_t
cmd_data_check_leaky(char *cmdstr, fal_leaky_ctrl_mode_t * val, a_uint32_t size)
{
    if (cmdstr == NULL)
        return SW_BAD_VALUE;

    if (!strcasecmp(cmdstr, "port"))
    {
        *val = FAL_LEAKY_PORT_CTRL;
    }
    else if (!strcasecmp(cmdstr, "fdb"))
    {
        *val = FAL_LEAKY_FDB_CTRL;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_leaky(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_LEAKY_PORT_CTRL)
    {
        dprintf("PORT");
    }
    else if (*(a_uint32_t *) buf == FAL_LEAKY_FDB_CTRL)
    {
        dprintf("FDB");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_uinta(char *cmdstr, a_uint32_t * val, a_uint32_t size)
{
    char *tmp_str = NULL;
    a_uint32_t *tmp_ptr = val;
    a_uint32_t i = 0;

    tmp_str = (void *) strtok(cmdstr, ",");
    while (tmp_str)
    {
        if (i >= (size / 4))
        {
            return SW_BAD_VALUE;
        }

        sscanf(tmp_str, "%d", tmp_ptr);
        tmp_ptr++;

        i++;
        tmp_str = (void *) strtok(NULL, ",");
    }

    if (i != (size / 4))
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_uinta(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    a_uint32_t i;
    a_uint32_t *tmp_ptr;

    dprintf("[%s]:", param_name);

    tmp_ptr = buf;
    for (i = 0; i < (size / 4); i++)
    {
        dprintf(" %d, ", *tmp_ptr);
        tmp_ptr++;
    }
}

/*fdb*/
sw_error_t
cmd_data_check_maccmd(char *cmdstr, fal_fwd_cmd_t * val, a_uint32_t size)
{
    if (NULL == cmdstr)
    {
        return SW_BAD_VALUE;
    }

    if (0 == cmdstr[0])
    {
        *val = FAL_MAC_FRWRD;   //defualt
    }
    else if (!strcasecmp(cmdstr, "forward"))
    {
        *val = FAL_MAC_FRWRD;
    }
    else if (!strcasecmp(cmdstr, "drop"))
    {
        *val = FAL_MAC_DROP;
    }
    else if (!strcasecmp(cmdstr, "cpycpu"))
    {
        *val = FAL_MAC_CPY_TO_CPU;
    }
    else if (!strcasecmp(cmdstr, "rdtcpu"))
    {
        *val = FAL_MAC_RDT_TO_CPU;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_maccmd(char * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_MAC_FRWRD)
    {
        dprintf("FORWARD");
    }
    else if (*(a_uint32_t *) buf == FAL_MAC_DROP)
    {
        dprintf("DROP");
    }
    else if (*(a_uint32_t *) buf == FAL_MAC_CPY_TO_CPU)
    {
        dprintf("CPYCPU");
    }
    else if (*(a_uint32_t *) buf == FAL_MAC_RDT_TO_CPU)
    {
        dprintf("RDTCPU");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

/*flow*/
sw_error_t
cmd_data_check_flowcmd(char *cmdstr, fal_default_flow_cmd_t * val, a_uint32_t size)
{
    if (NULL == cmdstr)
    {
        return SW_BAD_VALUE;
    }

    if (0 == cmdstr[0])
    {
        *val = FAL_DEFAULT_FLOW_FORWARD;   //defualt
    }
    else if (!strcasecmp(cmdstr, "forward"))
    {
        *val = FAL_DEFAULT_FLOW_FORWARD;
    }
    else if (!strcasecmp(cmdstr, "drop"))
    {
        *val = FAL_DEFAULT_FLOW_DROP;
    }
    else if (!strcasecmp(cmdstr, "rdtcpu"))
    {
        *val = FAL_DEFAULT_FLOW_RDT_TO_CPU;
    }
    else if (!strcasecmp(cmdstr, "admit_all"))
    {
        *val = FAL_DEFAULT_FLOW_ADMIT_ALL;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_flowcmd(char * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_DEFAULT_FLOW_FORWARD)
    {
        dprintf("FORWARD");
    }
    else if (*(a_uint32_t *) buf == FAL_DEFAULT_FLOW_DROP)
    {
        dprintf("DROP");
    }
    else if (*(a_uint32_t *) buf == FAL_DEFAULT_FLOW_RDT_TO_CPU)
    {
        dprintf("RDTCPU");
    }
    else if (*(a_uint32_t *) buf == FAL_DEFAULT_FLOW_ADMIT_ALL)
    {
        dprintf("ADMIT_ALL");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_flowtype(char *cmd_str, fal_flow_type_t * arg_val,
                        a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (!strcasecmp(cmd_str, "lan2lan"))
    {
        *arg_val = FAL_FLOW_LAN_TO_LAN;
    }
    else if (!strcasecmp(cmd_str, "wan2lan"))
    {
        *arg_val = FAL_FLOW_WAN_TO_LAN;
    }
    else if (!strcasecmp(cmd_str, "lan2wan"))
    {
        *arg_val = FAL_FLOW_LAN_TO_WAN;
    }
    else if (!strcasecmp(cmd_str, "wan2wan"))
    {
        *arg_val = FAL_FLOW_WAN_TO_WAN;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_flowtype(char * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_FLOW_LAN_TO_LAN)
    {
        dprintf("lan2lan");
    }
    else if (*(a_uint32_t *) buf == FAL_FLOW_WAN_TO_LAN)
    {
        dprintf("wan2lan");
    }
    else if (*(a_uint32_t *) buf == FAL_FLOW_LAN_TO_WAN)
    {
        dprintf("lan2wan");
    }
    else if (*(a_uint32_t *) buf == FAL_FLOW_WAN_TO_WAN)
    {
        dprintf("wan2wan");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_confirm(char *cmdstr, a_bool_t def, a_bool_t * val,
                       a_uint32_t size)
{
    if (0 == cmdstr[0])
    {
        *val = def;
    }
    else if ((!strcasecmp(cmdstr, "yes")) || (!strcasecmp(cmdstr, "y")))
    {
        *val = A_TRUE;
    }
    else if ((!strcasecmp(cmdstr, "no")) || (!strcasecmp(cmdstr, "n")))
    {
        *val = A_FALSE;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_confirm(char * param_name, a_bool_t val, a_uint32_t size)
{
    dprintf("%s", param_name);
    if (A_TRUE == val)
    {
        dprintf("YES");
    }
    else if (A_FALSE == val)
    {
        dprintf("NO");
    }
    else
    {
        dprintf("UNKNOW");
    }

    return;
}

sw_error_t
cmd_data_check_portmap(char *cmdstr, fal_pbmp_t * val, a_uint32_t size)
{
    char *tmp = NULL;
    a_uint32_t i = 0;
    a_uint32_t port;

    *val = 0;
    //default input null
    if(!strcasecmp(cmdstr, "null"))
    {
        return SW_OK;
    }

    tmp = (void *) strtok(cmdstr, ",");
    while (tmp)
    {
        if (SW_MAX_NR_PORT <= i)
        {
            return SW_BAD_VALUE;
        }

        sscanf(tmp, "%d", &port);
        if (SW_MAX_NR_PORT <= port)
        {
            return SW_BAD_VALUE;
        }

        *val |= (0x1 << port);
        tmp = (void *) strtok(NULL, ",");
    }

    return SW_OK;
}

void
cmd_data_print_portmap(char * param_name, fal_pbmp_t val, a_uint32_t size)
{
    a_uint32_t i;
    char tmp[16];
    tmp[0] = '\0';

    dprintf("%s", param_name);
    for (i = 0; i < SW_MAX_NR_PORT; i++)
    {
        if (val & (0x1 << i))
        {
            if(strlen(tmp) == 0)
                sprintf(tmp, "%d", i);
            else
                sprintf(tmp+strlen(tmp), ",%d", i);
        }
    }
    dprintf("%s ", tmp);
    return;
}

sw_error_t
cmd_data_check_macaddr(char *cmdstr, void *val, a_uint32_t size)
{
    char *tmp = NULL;
    a_uint32_t i = 0, j;
    a_uint32_t addr;
    fal_mac_addr_t mac;

    memset(&mac, 0, sizeof (fal_mac_addr_t));
    if (NULL == cmdstr)
    {
        *(fal_mac_addr_t *) val = mac;
        return SW_BAD_VALUE; /*was: SW_OK;*/
    }

    if (0 == cmdstr[0])
    {
        *(fal_mac_addr_t *) val = mac;
        return SW_OK;
    }

    tmp = (void *) strtok(cmdstr, "-");
    while (tmp)
    {
        if (6 <= i)
        {
            return SW_BAD_VALUE;
        }

        if ((2 < strlen(tmp)) || (0 == strlen(tmp)))
        {
            return SW_BAD_VALUE;
        }

        for (j = 0; j < strlen(tmp); j++)
        {
            if (A_FALSE == is_hex(tmp[j]))
                return SW_BAD_VALUE;
        }

        sscanf(tmp, "%x", &addr);
        if (0xff < addr)
        {
            return SW_BAD_VALUE;
        }

        mac.uc[i++] = addr;
        tmp = (void *) strtok(NULL, "-");
    }

    if (6 != i)
    {
        return SW_BAD_VALUE;
    }

    *(fal_mac_addr_t *) val = mac;
    return SW_OK;
}

void
cmd_data_print_macaddr(char * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    a_uint32_t i;
    fal_mac_addr_t *val;

    val = (fal_mac_addr_t *) buf;
    dprintf("%s", param_name);
    for (i = 0; i < 5; i++)
    {
        dprintf("%02x-", val->uc[i]);
    }
    dprintf("%02x", val->uc[5]);

}

sw_error_t
cmd_data_check_fdbentry(char *info, void *val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_fdb_entry_t entry;
    a_uint32_t tmp;

    memset(&entry, 0, sizeof (fal_fdb_entry_t));

    do
    {
        cmd = get_sub_cmd("addr", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the format is xx-xx-xx-xx-xx-xx \n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_macaddr(cmd, &entry.addr,
                                        sizeof (fal_mac_addr_t));
            if (SW_OK != rv)
                dprintf("usage: the format is xx-xx-xx-xx-xx-xx \n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("fid", "65535");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 1 -- 4095 or 65535\n");
            rv = SW_BAD_VALUE;
        }
        else if (0 == cmd[0])
        {
            entry.fid = 65535;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 1 -- 4095 or 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.fid = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("dacmd", "forward");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_maccmd(cmd, &entry.dacmd,
                                       sizeof (fal_fwd_cmd_t));
            if (SW_OK != rv)
                dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("sacmd", "forward");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_maccmd(cmd, &entry.sacmd,
                                       sizeof (fal_fwd_cmd_t));
            if (SW_OK != rv)
                dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("dest port", "null");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: input port number such as 1,3\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_portmap(cmd, &entry.port.map,
                                        sizeof (fal_pbmp_t));
            if (SW_OK != rv)
                dprintf("usage: input port number such as 1,3\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.portmap_en = A_TRUE;

    do
    {
        cmd = get_sub_cmd("static", "yes");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.static_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("leaky", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.leaky_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("mirror", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.mirror_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("clone", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.clone_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("queue override", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.da_pri_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == entry.da_pri_en)
    {
        do
        {
            cmd = get_sub_cmd("queue", NULL);
            SW_RTN_ON_NULL_PARAM(cmd);

            if (!strncasecmp(cmd, "quit", 4))
            {
                return SW_BAD_VALUE;
            }
            else if (!strncasecmp(cmd, "help", 4))
            {
                dprintf("usage: input number such as <0/1/2/3>\n");
                rv = SW_BAD_VALUE;
            }
            else
            {
                rv = cmd_data_check_uint32(cmd, &tmp, sizeof (a_uint32_t));
                if (SW_OK != rv)
                    dprintf("usage: input number such as <0/1/2/3>\n");
            }

        }
        while (talk_mode && (SW_OK != rv));
        entry.da_queue = tmp;
    }

    do
    {
        cmd = get_sub_cmd("cross_pt_state", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.cross_pt_state,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("white_list_en", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.white_list_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("load_balance_en", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.load_balance_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == entry.load_balance_en)
    {
        do
        {
            cmd = get_sub_cmd("load_balance", NULL);
            SW_RTN_ON_NULL_PARAM(cmd);

            if (!strncasecmp(cmd, "quit", 4))
            {
                return SW_BAD_VALUE;
            }
            else if (!strncasecmp(cmd, "help", 4))
            {
                dprintf("usage: input number such as <0/1/2/3>\n");
                rv = SW_BAD_VALUE;
            }
            else
            {
                rv = cmd_data_check_uint32(cmd, &tmp, sizeof (a_uint32_t));
                if (SW_OK != rv)
                    dprintf("usage: input number such as <0/1/2/3>\n");
            }

        }
        while (talk_mode && (SW_OK != rv));
        entry.load_balance = tmp;
    }

    *(fal_fdb_entry_t *) val = entry;

    return SW_OK;
}

void
cmd_data_print_fdbentry(a_uint8_t * param_name, a_uint32_t * buf,
                        a_uint32_t size)
{
    a_uint32_t tmp;
    fal_fdb_entry_t *entry;

    entry = (fal_fdb_entry_t *) buf;
    dprintf("\n");
    cmd_data_print_macaddr("[addr]:", (a_uint32_t *) & (entry->addr),
                           sizeof (fal_mac_addr_t));
    dprintf(" ");
    dprintf("[fid]:%d", entry->fid);
    dprintf(" ");
    cmd_data_print_confirm("[static]:", entry->static_en, sizeof (a_bool_t));
    dprintf(" ");
    cmd_data_print_portmap("[dest_port]:", entry->port.map,
                           sizeof (fal_pbmp_t));
    dprintf(" \n");
    cmd_data_print_maccmd("dacmd", (a_uint32_t *) & (entry->dacmd),
                          sizeof (fal_fwd_cmd_t));
    dprintf(" ");
    cmd_data_print_maccmd("sacmd", (a_uint32_t *) & (entry->sacmd),
                          sizeof (fal_fwd_cmd_t));
    dprintf(" ");
    cmd_data_print_confirm("[leaky]:", entry->leaky_en, sizeof (a_bool_t));
    dprintf(" ");
    cmd_data_print_confirm("[mirror]:", entry->mirror_en, sizeof (a_bool_t));
    dprintf(" ");
    cmd_data_print_confirm("[clone]:", entry->clone_en, sizeof (a_bool_t));
    dprintf(" ");
    cmd_data_print_confirm("[da_pri]:", entry->da_pri_en, sizeof (a_bool_t));
    dprintf(" ");
    if (A_TRUE == entry->da_pri_en)
    {
        tmp = entry->da_queue;
        dprintf("[queue]:%d", tmp);
    }
    else
    {
        dprintf("[queue]:0");
    }
    dprintf(" ");
    cmd_data_print_confirm("[cross_pt_state]:", entry->cross_pt_state, sizeof (a_bool_t));
    dprintf(" ");
    cmd_data_print_confirm("[white_list_en]:", entry->white_list_en, sizeof (a_bool_t));
    dprintf(" ");
    cmd_data_print_confirm("[load_balance_en]:", entry->load_balance_en, sizeof (a_bool_t));
    if (A_TRUE == entry->load_balance_en)
    {
        tmp = entry->load_balance;
	dprintf(" ");
        dprintf("[load_balance]:%d", tmp);
    }
    dprintf("\n");

    return;
}

#define cmd_data_check_element(info, defval, usage, chk_func, param) \
{\
    sw_error_t ret;\
    do {\
        cmd = get_sub_cmd(info, defval);\
        SW_RTN_ON_NULL_PARAM(cmd);\
        \
        if (!strncasecmp(cmd, "quit", 4)) {\
            return SW_BAD_VALUE;\
        } else if (!strncasecmp(cmd, "help", 4)) {\
            dprintf("%s", usage);\
            ret = SW_BAD_VALUE;\
        } else {\
            ret = chk_func param; \
            if (SW_OK != ret)\
                dprintf("%s", usage);\
        }\
    } while (talk_mode && (SW_OK != ret));\
}

sw_error_t
cmd_data_check_integer(char *cmd_str, a_uint32_t * arg_val, a_uint32_t max_val,
                       a_uint32_t min_val)
{
    a_uint32_t tmp;
    a_uint32_t i;

    if (NULL == cmd_str)
    {
        return SW_BAD_PARAM;
    }

    if (0 == cmd_str[0])
    {
        return SW_BAD_PARAM;
    }

    if ((cmd_str[0] == '0') && ((cmd_str[1] == 'x') || (cmd_str[1] == 'X')))
    {
        for (i = 2; i < strlen(cmd_str); i++)
        {
            if (A_FALSE == is_hex(cmd_str[i]))
            {
                return SW_BAD_VALUE;
            }
        }
        sscanf(cmd_str, "%x", &tmp);
    }
    else
    {
        for (i = 0; i < strlen(cmd_str); i++)
        {
            if (A_FALSE == is_dec(cmd_str[i]))
            {
                return SW_BAD_VALUE;
            }
        }
        sscanf(cmd_str, "%d", &tmp);
    }

    if ((tmp > max_val) || (tmp < min_val))
        return SW_BAD_PARAM;

    *arg_val = tmp;
    return SW_OK;
}

sw_error_t
cmd_data_check_ruletype(char *cmd_str, fal_acl_rule_type_t * arg_val,
                        a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (!strcasecmp(cmd_str, "mac"))
    {
        *arg_val = FAL_ACL_RULE_MAC;
    }
    else if (!strcasecmp(cmd_str, "ip4"))
    {
        *arg_val = FAL_ACL_RULE_IP4;
    }
    else if (!strcasecmp(cmd_str, "ip6"))
    {
        *arg_val = FAL_ACL_RULE_IP6;
    }
    else if (!strcasecmp(cmd_str, "udf"))
    {
        *arg_val = FAL_ACL_RULE_UDF;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_ruletype(char * param_name, a_uint32_t * buf,
                        a_uint32_t size)
{
    fal_acl_rule_type_t *val;

    val = (fal_acl_rule_type_t *) buf;
    dprintf("%s", param_name);

    if (FAL_ACL_RULE_MAC == *val)
    {
        dprintf("mac");
    }
    else if (FAL_ACL_RULE_IP4 == *val)
    {
        dprintf("ip4");
    }
    else if (FAL_ACL_RULE_IP6 == *val)
    {
        dprintf("ip6");
    }
    else if (FAL_ACL_RULE_UDF == *val)
    {
        dprintf("udf");
    }
    else
    {
        dprintf("unknow");
    }
}

sw_error_t
cmd_data_check_fieldop(char *cmdstr, fal_acl_field_op_t def,
                       fal_acl_field_op_t * val)
{
    if (0 == cmdstr[0])
    {
        *val = def;
    }
    else if ((!strcasecmp(cmdstr, "mask")) || (!strcasecmp(cmdstr, "m")))
    {
        *val = FAL_ACL_FIELD_MASK;
    }
    else if ((!strcasecmp(cmdstr, "range")) || (!strcasecmp(cmdstr, "r")))
    {
        *val = FAL_ACL_FIELD_RANGE;
    }
    else if ((!strcasecmp(cmdstr, "le")) || (!strcasecmp(cmdstr, "l")))
    {
        *val = FAL_ACL_FIELD_LE;
    }
    else if ((!strcasecmp(cmdstr, "ge")) || (!strcasecmp(cmdstr, "g")))
    {
        *val = FAL_ACL_FIELD_GE;
    }
    else if ((!strcasecmp(cmdstr, "ne")) || (!strcasecmp(cmdstr, "n")))
    {
        *val = FAL_ACL_FIELD_NE;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_fieldop(char * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    fal_acl_field_op_t *val;

    val = (fal_acl_field_op_t *) buf;
    dprintf("%s", param_name);

    if (FAL_ACL_FIELD_MASK == *val)
    {
        dprintf("mask");
    }
    else if (FAL_ACL_FIELD_RANGE == *val)
    {
        dprintf("range");
    }
    else if (FAL_ACL_FIELD_LE == *val)
    {
        dprintf("le");
    }
    else if (FAL_ACL_FIELD_GE == *val)
    {
        dprintf("ge");
    }
    else if (FAL_ACL_FIELD_NE == *val)
    {
        dprintf("ne");
    }
    else
    {
        dprintf("unknow");
    }
}

sw_error_t
cmd_data_check_ip4addr(char *cmdstr, void * val, a_uint32_t size)
{
    char *tmp = NULL;
    a_uint32_t i = 0, j;
    a_uint32_t addr;
    fal_ip4_addr_t ip4;
    char cmd[128+1] = { 0 };

    memset(&ip4, 0, sizeof (fal_ip4_addr_t));
    if (NULL == cmdstr)
    {
        return SW_BAD_VALUE;
    }

    if (0 == cmdstr[0])
    {
        return SW_BAD_VALUE;
    }

    for (i = 0; i < 128; i++)
    {
        if (0 == cmdstr[i])
        {
            break;
        }
    }

    i++;
    if (128 < i)
    {
        i = 128;
    }

    memcpy(cmd, cmdstr, i);

	/* make sure the string can be terminated */
	cmd[i] = '\0';
    tmp = (void *) strtok(cmd, ".");
    i = 0;
    while (tmp)
    {
        if (4 <= i)
        {
            return SW_BAD_VALUE;
        }

        if ((3 < strlen(tmp)) || (0 == strlen(tmp)))
        {
            return SW_BAD_VALUE;
        }

        for (j = 0; j < strlen(tmp); j++)
        {
            if (A_FALSE == is_dec(tmp[j]))
            {
                return SW_BAD_VALUE;
            }
        }

        sscanf(tmp, "%d", &addr);
        if (255 < addr)
        {
            return SW_BAD_VALUE;
        }

        ip4 |= ((addr & 0xff) << (24 - i * 8));
        i++;
        tmp = (void *) strtok(NULL, ".");
    }

    if (4 != i)
    {
        return SW_BAD_VALUE;
    }

    *(fal_ip4_addr_t*)val = ip4;
    return SW_OK;
}

void
cmd_data_print_ip4addr(char * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    a_uint32_t i;
    fal_ip4_addr_t ip4;

    ip4 = *((fal_ip4_addr_t *) buf);
    dprintf("%s", param_name);
    for (i = 0; i < 3; i++)
    {
        dprintf("%d.", (ip4 >> (24 - i * 8)) & 0xff);
    }
    dprintf("%d", (ip4 & 0xff));
}

sw_error_t
cmd_data_check_multi(char *info, void *val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_igmp_sg_entry_t entry;

    memset(&entry, 0, sizeof (fal_igmp_sg_entry_t));

    do
    {
        cmd = get_sub_cmd("group type", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.group.type), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    if(entry.group.type==0)
    {
        cmd_data_check_element("group ip4 addr", "0.0.0.0",
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd, &(entry.group.u.ip4_addr), 4));
    }
    else
        cmd_data_check_element("group ip6 addr", NULL,
                               "usage: the format is xxxx::xxxx \n",
                               cmd_data_check_ip6addr, (cmd, &(entry.group.u.ip6_addr), 16));

    do
    {
        cmd = get_sub_cmd("source type", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.source.type), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    if(entry.source.type==0)
    {
        cmd_data_check_element("source ip4 addr", "0.0.0.0",
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd, &(entry.source.u.ip4_addr), 4));
    }
    else
        cmd_data_check_element("source ip6 addr", NULL,
                               "usage: the format is xxxx::xxxx \n",
                               cmd_data_check_ip6addr, (cmd, &(entry.source.u.ip6_addr), 16));

    do
    {
        cmd = get_sub_cmd("portmap", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.port_map), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("vlanid", "0xffff");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 4095 or 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.vlan_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 0 -- 4095 or 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    *(fal_igmp_sg_entry_t *)val = entry;

    return SW_OK;
}
void
cmd_data_print_multi(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_igmp_sg_entry_t *entry;

    entry = (fal_igmp_sg_entry_t *) buf;

    dprintf("\n[multicast info]:  [group type]:%x [source type]:%x ", entry->group.type, entry->source.type);

    if(entry->group.type == 0)
        cmd_data_print_ip4addr("\n[group ip4 addr]:",
                               (a_uint32_t *) & (entry->group.u.ip4_addr),
                               sizeof (fal_ip4_addr_t));
    else
        cmd_data_print_ip6addr("\n[group ip6 addr]:",
                               (a_uint32_t *) & (entry->group.u.ip6_addr),
                               sizeof (fal_ip6_addr_t));

    if(entry->source.type == 0)
        cmd_data_print_ip4addr("\n[source ip4 addr]:",
                               (a_uint32_t *) & (entry->source.u.ip4_addr),
                               sizeof (fal_ip4_addr_t));
    else
        cmd_data_print_ip6addr("\n[source ip6 addr]:",
                               (a_uint32_t *) & (entry->source.u.ip6_addr),
                               sizeof (fal_ip6_addr_t));

    dprintf("\n[entry portmap]: [portmap]:0x%x  ", entry->port_map);
    dprintf("\n[entry vlanid]: [vlanid]:%d  ", entry->vlan_id);

}

sw_error_t
cmd_data_check_ip6addr(char *cmdstr, void * val, a_uint32_t size)
{
    char *tmp = NULL;
    a_uint32_t j;
    a_uint32_t i = 0, cnt = 0, rep = 0, loc = 0;
    a_uint32_t data;
    a_uint32_t addr[8];
    fal_ip6_addr_t ip6;

    if (NULL == cmdstr)
    {
        return SW_BAD_VALUE;
    }

    if (0 == cmdstr[0])
    {
        return SW_BAD_VALUE;
    }

    for (i = 0; i < 8; i++)
    {
        addr[i] = 0;
    }

    for (i = 0; i < strlen(cmdstr); i++)
    {
        if (':' == cmdstr[i])
        {
            if ((i == (strlen(cmdstr) - 1))
                    || (0 == i))
            {
                return SW_BAD_VALUE;
            }
            cnt++;

            if (':' == cmdstr[i - 1])
            {
                rep++;
                loc = cnt - 1;
            }
        }
    }

    if (1 < rep)
    {
        return SW_BAD_VALUE;
    }

    tmp = (void *) strtok(cmdstr, ":");
    i = 0;
    while (tmp)
    {
        if (8 <= i)
        {
            return SW_BAD_VALUE;
        }

        if ((4 < strlen(tmp)) || (0 == strlen(tmp)))
        {
            return SW_BAD_VALUE;
        }

        for (j = 0; j < strlen(tmp); j++)
        {
            if (A_FALSE == is_hex(tmp[j]))
            {
                return SW_BAD_VALUE;
            }
        }

        sscanf(tmp, "%x", &data);
        if (65535 < data)
        {
            return SW_BAD_VALUE;
        }

        addr[i++] = data;
        tmp = (void *) strtok(NULL, ":");
    }

    if (0 == rep)
    {
        if (8 != i)
        {
            return SW_BAD_VALUE;
        }
    }
    else
    {
        if (8 <= i)
        {
            return SW_BAD_VALUE;
        }

        for (j = i - 1; j >= loc; j--)
        {
            addr[8 - i + j] = addr[j];
            addr[j] = 0;
        }
    }

    for (i = 0; i < 4; i++)
    {
        ip6.ul[i] = (addr[i * 2] << 16) | addr[i * 2 + 1];
    }

    dprintf("\n");
    for (i = 0; i < 4; i++)
    {
        dprintf("%08x  ", ip6.ul[i]);
    }
    dprintf("\n");

    *(fal_ip6_addr_t*)val = ip6;
    return SW_OK;
}

void
cmd_data_print_ip6addr(char * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    a_uint32_t i;
    fal_ip6_addr_t ip6;

    ip6 = *(fal_ip6_addr_t *) buf;
    dprintf("%s", param_name);
    for (i = 0; i < 3; i++)
    {
        dprintf("%x:%x:", (ip6.ul[i] >> 16) & 0xffff, ip6.ul[i] & 0xffff);
    }
    dprintf("%x:%x", (ip6.ul[3] >> 16) & 0xffff, ip6.ul[3] & 0xffff);
}

sw_error_t
cmd_data_check_mac_field(fal_acl_rule_t * entry)
{
    char *cmd;
    a_uint32_t tmpdata;

    /* get destination mac address field configuration */
    cmd_data_check_element("mac dst addr field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("dst mac addr", NULL,
                               "usage: the format is xx-xx-xx-xx-xx-xx \n",
                               cmd_data_check_macaddr, (cmd,
                                       &(entry->dest_mac_val),
                                       sizeof
                                       (fal_mac_addr_t)));

        cmd_data_check_element("dst mac addr mask", NULL,
                               "usage: the format is xx-xx-xx-xx-xx-xx \n",
                               cmd_data_check_macaddr, (cmd,
                                       &(entry->dest_mac_mask),
                                       sizeof
                                       (fal_mac_addr_t)));

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_DA);
    }

    /* get source mac address field configuration */
    cmd_data_check_element("mac src addr field", "no",  "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("src mac addr", NULL,
                               "usage: the format is xx-xx-xx-xx-xx-xx \n",
                               cmd_data_check_macaddr, (cmd,
                                       &(entry->src_mac_val),
                                       sizeof
                                       (fal_mac_addr_t)));

        cmd_data_check_element("src mac addr mask", NULL,
                               "usage: the format is xx-xx-xx-xx-xx-xx \n",
                               cmd_data_check_macaddr, (cmd,
                                       &(entry->src_mac_mask),
                                       sizeof
                                       (fal_mac_addr_t)));

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_SA);
    }

    /* get ethernet type field configuration */
    cmd_data_check_element("ethernet type field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ethernet type", NULL,
                               "usage: the format is 0x0-0xffff or 0-65535\n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xffff,
                                       0x0));
        entry->ethtype_val = tmpdata & 0xffff;

        cmd_data_check_element("ethernet type mask", NULL,
                               "usage: the format is 0x0-0xffff or 0-65535\n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xffff,
                                       0x0));
        entry->ethtype_mask = tmpdata & 0xffff;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);
    }

    /* get vlanid field configuration */
    cmd_data_check_element("vlanid field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("vlanid opration", "mask",
                               "usage: <mask/range/le/ge/ne> \n",
                               cmd_data_check_fieldop, (cmd, FAL_ACL_FIELD_MASK,
                                       &(entry->vid_op)));

        if (FAL_ACL_FIELD_MASK == entry->vid_op)
        {
            cmd_data_check_element("vlanid", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->vid_val = tmpdata & 0xfff;

            cmd_data_check_element("vlanid mask", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->vid_mask = tmpdata & 0xfff;
        }
        else if (FAL_ACL_FIELD_RANGE == entry->vid_op)
        {
            cmd_data_check_element("vlanid low", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->vid_val = tmpdata & 0xfff;

            cmd_data_check_element("vlanid high", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->vid_mask = tmpdata & 0xfff;
        }
        else
        {
            cmd_data_check_element("vlanid", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->vid_val = tmpdata & 0xfff;
        }

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_VID);
    }

    /* get vlan tagged field configuration */
    cmd_data_check_element("vlan tagged field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("tagged", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->tagged_val = tmpdata & 0x1;

        cmd_data_check_element("tagged mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->tagged_mask = tmpdata & 0x1;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_TAGGED);
    }

    /* get up field configuration */
    cmd_data_check_element("up field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("up", NULL,
                               "usage: the format is 0x0-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->up_val = tmpdata & 0x7;

        cmd_data_check_element("up mask", NULL,
                               "usage: the format is 0x0-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->up_mask = tmpdata & 0x7;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_UP);
    }

    /* get cfi field configuration */
    cmd_data_check_element("cfi field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("cfi", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->cfi_val = tmpdata & 0x1;

        cmd_data_check_element("cfi mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->cfi_mask = tmpdata & 0x1;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_CFI);
    }

    /* get svlan tagged field configuration */
    cmd_data_check_element("svlan tagged field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("stagged", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->stagged_val = tmpdata & 0x1;

        cmd_data_check_element("stagged mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->stagged_mask = tmpdata & 0x1;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_STAGGED);
    }

    /* get stag vlanid field configuration */
    cmd_data_check_element("stag vid field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("stag vid opration", "mask",
                               "usage: <mask/range/le/ge/ne> \n",
                               cmd_data_check_fieldop, (cmd, FAL_ACL_FIELD_MASK,
                                       &(entry->stag_vid_op)));

        if (FAL_ACL_FIELD_MASK == entry->stag_vid_op)
        {
            cmd_data_check_element("stag vid", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->stag_vid_val = tmpdata & 0xfff;

            cmd_data_check_element("stag vid mask", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->stag_vid_mask = tmpdata & 0xfff;
        }
        else if (FAL_ACL_FIELD_RANGE == entry->stag_vid_op)
        {
            cmd_data_check_element("stag vid low", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->stag_vid_val = tmpdata & 0xfff;

            cmd_data_check_element("stag vid high", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->stag_vid_mask = tmpdata & 0xfff;
        }
        else
        {
            cmd_data_check_element("stag vid", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->stag_vid_val = tmpdata & 0xfff;
        }

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_STAG_VID);
    }


    /* get stag priority field configuration */
    cmd_data_check_element("stag pri field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("stag pri", NULL,
                               "usage: the format is 0x0-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->stag_pri_val = tmpdata & 0x7;

        cmd_data_check_element("stag pri mask", NULL,
                               "usage: the format is 0x0-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->stag_pri_mask = tmpdata & 0x7;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_STAG_PRI);
    }

    /* get stag dei field configuration */
    cmd_data_check_element("stag dei field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("stag dei", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->stag_dei_val = tmpdata & 0x1;

        cmd_data_check_element("stag dei mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->stag_dei_mask = tmpdata & 0x1;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_STAG_DEI);
    }

    /* get cvlan tagged field configuration */
    cmd_data_check_element("cvlan tagged field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ctagged", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->ctagged_val = tmpdata & 0x1;

        cmd_data_check_element("ctagged mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->ctagged_mask = tmpdata & 0x1;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_CTAGGED);
    }

    /* get ctag vlanid field configuration */
    cmd_data_check_element("ctag vid field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ctag vid opration", "mask",
                               "usage: <mask/range/le/ge/ne> \n",
                               cmd_data_check_fieldop, (cmd, FAL_ACL_FIELD_MASK,
                                       &(entry->ctag_vid_op)));

        if (FAL_ACL_FIELD_MASK == entry->ctag_vid_op)
        {
            cmd_data_check_element("ctag vid", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->ctag_vid_val = tmpdata & 0xfff;

            cmd_data_check_element("ctag vid mask", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->ctag_vid_mask = tmpdata & 0xfff;
        }
        else if (FAL_ACL_FIELD_RANGE == entry->ctag_vid_op)
        {
            cmd_data_check_element("ctag vid low", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->ctag_vid_val = tmpdata & 0xfff;

            cmd_data_check_element("ctag vid high", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->ctag_vid_mask = tmpdata & 0xfff;
        }
        else
        {
            cmd_data_check_element("ctag vid", NULL,
                                   "usage: the format is 0x0-0xfff or 0-4095 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xfff, 0x0));
            entry->ctag_vid_val = tmpdata & 0xfff;
        }

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_CTAG_VID);
    }

    /* get ctag priority field configuration */
    cmd_data_check_element("ctag pri field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ctag pri", NULL,
                               "usage: the format is 0x0-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->ctag_pri_val = tmpdata & 0x7;

        cmd_data_check_element("ctag pri mask", NULL,
                               "usage: the format is 0x0-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->ctag_pri_mask = tmpdata & 0x7;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_CTAG_PRI);
    }

    /* get ctag cfi field configuration */
    cmd_data_check_element("ctag cfi field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ctag cfi", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->ctag_cfi_val = tmpdata & 0x1;

        cmd_data_check_element("ctag cfi mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->ctag_cfi_mask = tmpdata & 0x1;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_MAC_CTAG_CFI);
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_ip4_field(fal_acl_rule_t * entry)
{
    char *cmd;
    a_uint32_t tmpdata;

    /* get ip4 source address field configuration */
    cmd_data_check_element("ip4 src address field", "no",
                           "usage: <yes/no/y/n>\n", cmd_data_check_confirm,
                           (cmd, A_FALSE, &tmpdata, sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ip4 src addr", NULL,
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd,
                                       &(entry->src_ip4_val), 4));

        cmd_data_check_element("ip4 src addr mask", NULL,
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd,
                                       &(entry->src_ip4_mask), 4));

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_IP4_SIP);
    }

    /* get ip4 destination address field configuration */
    cmd_data_check_element("ip4 dst address field", "no",
                           "usage: <yes/no/y/n>\n", cmd_data_check_confirm,
                           (cmd, A_FALSE, &tmpdata, sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ip4 dst addr", NULL,
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd,
                                       &(entry->
                                         dest_ip4_val), 4));

        cmd_data_check_element("ip4 dst addr mask", NULL,
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd,
                                       &(entry->
                                         dest_ip4_mask), 4));

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_IP4_DIP);
    }

    /* get ripv1 field configuration */
    cmd_data_check_element("ripv1 field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ripv1", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->ripv1_val = tmpdata & 0x1;

        cmd_data_check_element("ripv1 mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->ripv1_mask = tmpdata & 0x1;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_RIPV1);
    }

    /* get dhcpv4 field configuration */
    cmd_data_check_element("dhcpv4 field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("dhcpv4", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->dhcpv4_val = tmpdata & 0x1;

        cmd_data_check_element("dhcpv4 mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->dhcpv4_mask = tmpdata & 0x1;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_DHCPV4);
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_ip6_field(fal_acl_rule_t * entry)
{
    char *cmd;
    a_uint32_t tmpdata;

    /* get ip6 source address field configuration */
    cmd_data_check_element("ip6 src address field", "no",
                           "usage: <yes/no/y/n>\n", cmd_data_check_confirm,
                           (cmd, A_FALSE, &tmpdata, sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ip6 src addr", NULL,
                               "usage: the format is xxxx::xxxx \n",
                               cmd_data_check_ip6addr, (cmd,
                                       &(entry->src_ip6_val), 16));

        cmd_data_check_element("ip6 src addr mask", NULL,
                               "usage: the format is xxxx::xxxx \n",
                               cmd_data_check_ip6addr, (cmd,
                                       &(entry->
                                         src_ip6_mask), 16));

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_IP6_SIP);
    }

    /* get ip6 destination address field configuration */
    cmd_data_check_element("ip6 dst address field", "no",
                           "usage: <yes/no/y/n>\n", cmd_data_check_confirm,
                           (cmd, A_FALSE, &tmpdata, sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ip6 dst addr", NULL,
                               "usage: the format is xxxx::xxxx \n",
                               cmd_data_check_ip6addr, (cmd,
                                       &(entry->
                                         dest_ip6_val), 16));

        cmd_data_check_element("ip6 dst addr mask", NULL,
                               "usage: the format is xxxx::xxxx \n",
                               cmd_data_check_ip6addr, (cmd,
                                       &(entry->
                                         dest_ip6_mask), 16));

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_IP6_DIP);
    }

    /* get ip6 flow label field configuration */
    cmd_data_check_element("ip6 flow label field", "no",
                           "usage: <yes/no/y/n>\n", cmd_data_check_confirm,
                           (cmd, A_FALSE, &tmpdata, sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ip6 label", NULL,
                               "usage: the format is 0x0-0xfffff or 0-1048575\n",
                               cmd_data_check_integer, (cmd,
                                       &(entry->ip6_lable_val),
                                       0xfffff, 0x0));

        cmd_data_check_element("ip6 label mask", NULL,
                               "usage: the format is 0x0-0xfffff or 0-1048575\n",
                               cmd_data_check_integer, (cmd,
                                       &(entry->
                                         ip6_lable_mask),
                                       0xfffff, 0x0));

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_IP6_LABEL);
    }

    /* get dhcpv6 field configuration */
    cmd_data_check_element("dhcpv6 field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("dhcpv6", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->dhcpv6_val = tmpdata & 0xff;

        cmd_data_check_element("dhcpv6 mask", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->dhcpv6_mask = tmpdata & 0xff;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_DHCPV6);
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_ip_field(fal_acl_rule_t * entry)
{
    char *cmd;
    a_uint32_t tmpdata;

    /* get ip protocol field configuration */
    cmd_data_check_element("ip protocol field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ip protocol", NULL,
                               "usage: the format is 0x0-0xff or 0-255 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->ip_proto_val = tmpdata & 0xff;

        cmd_data_check_element("ip protocol mask", NULL,
                               "usage: the format is 0x0-0xff or 0-255 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->ip_proto_mask = tmpdata & 0xff;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_IP_PROTO);
    }

    /* get ip dscp field configuration */
    cmd_data_check_element("ip dscp field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (tmpdata)
    {
        cmd_data_check_element("ip dscp", NULL,
                               "usage: the format is 0x0-0xff or 0-255 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->ip_dscp_val = tmpdata & 0xff;

        cmd_data_check_element("ip dscp mask", NULL,
                               "usage: the format is 0x0-0xff or 0-255 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->ip_dscp_mask = tmpdata & 0xff;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_IP_DSCP);
    }

    /* get ip l4 destination port field configuration */
    cmd_data_check_element("ip l4 dst port field", "no",
                           "usage: <yes/no/y/n>\n", cmd_data_check_confirm,
                           (cmd, A_FALSE, &tmpdata, sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ip l4 dst port opration", "mask",
                               "usage: <mask/range/le/ge/ne> \n",
                               cmd_data_check_fieldop, (cmd, FAL_ACL_FIELD_MASK,
                                       &(entry->
                                         dest_l4port_op)));

        if (FAL_ACL_FIELD_MASK == entry->dest_l4port_op)
        {
            cmd_data_check_element("ip l4 dst port", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->dest_l4port_val = tmpdata & 0xffff;

            cmd_data_check_element("ip l4 dst port mask", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->dest_l4port_mask = tmpdata & 0xffff;
        }
        else if (FAL_ACL_FIELD_RANGE == entry->dest_l4port_op)
        {
            cmd_data_check_element("ip l4 dst port low", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->dest_l4port_val = tmpdata & 0xffff;

            cmd_data_check_element("ip l4 dst port high", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->dest_l4port_mask = tmpdata & 0xffff;
        }
        else
        {
            cmd_data_check_element("ip l4 dst port", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->dest_l4port_val = tmpdata & 0xffff;
        }

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_L4_DPORT);
    }

    /* get ip l4 source port field configuration */
    cmd_data_check_element("ip l4 src port field", "no",
                           "usage: <yes/no/y/n>\n", cmd_data_check_confirm,
                           (cmd, A_FALSE, &tmpdata, sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("ip l4 src port opration", "mask",
                               "usage: <mask/range/le/ge/ne> \n",
                               cmd_data_check_fieldop, (cmd, FAL_ACL_FIELD_MASK,
                                       &(entry->
                                         src_l4port_op)));

        if (FAL_ACL_FIELD_MASK == entry->src_l4port_op)
        {
            cmd_data_check_element("ip l4 src port", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->src_l4port_val = tmpdata & 0xffff;

            cmd_data_check_element("ip l4 src port mask", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->src_l4port_mask = tmpdata & 0xffff;
        }
        else if (FAL_ACL_FIELD_RANGE == entry->src_l4port_op)
        {
            cmd_data_check_element("ip l4 src port low", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->src_l4port_val = tmpdata & 0xffff;

            cmd_data_check_element("ip l4 src port high", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->src_l4port_mask = tmpdata & 0xffff;
        }
        else
        {
            cmd_data_check_element("ip l4 src port", NULL,
                                   "usage: the format is 0x0-0xffff or 0-65535 \n",
                                   cmd_data_check_integer, (cmd, &tmpdata,
                                           0xffff, 0x0));
            entry->src_l4port_val = tmpdata & 0xffff;
        }

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_L4_SPORT);
    }

    /* get tcp flags field configuration */
    cmd_data_check_element("tcp flags field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("tcp flags", NULL,
                               "usage: the format is 0x0-0x3f or 0-63 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x3f,
                                       0x0));
        entry->tcp_flag_val = tmpdata & 0x3f;

        cmd_data_check_element("tcp flags mask", NULL,
                               "usage: the format is 0x0-0x3f or 0-63 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x3f,
                                       0x0));
        entry->tcp_flag_mask = tmpdata & 0x3f;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_TCP_FLAG);
    }

    /* get icmp type field configuration */
    cmd_data_check_element("icmp type field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("icmp type", NULL,
                               "usage: the format is 0x0-0xff or 0-255 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->icmp_type_val = tmpdata & 0xff;

        cmd_data_check_element("icmp type mask", NULL,
                               "usage: the format is 0x0-0xff or 0-255 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->icmp_type_mask = tmpdata & 0xff;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_ICMP_TYPE);
    }

    /* get icmp code field configuration */
    cmd_data_check_element("icmp code field", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("icmp code", NULL,
                               "usage: the format is 0x0-0xff or 0-255 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->icmp_code_val = tmpdata & 0xff;

        cmd_data_check_element("icmp code mask", NULL,
                               "usage: the format is 0x0-0xff or 0-255 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xff,
                                       0x0));
        entry->icmp_code_mask = tmpdata & 0xff;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_ICMP_CODE);
    }

    return SW_OK;
}


sw_error_t
cmd_data_check_udf_type(char *cmdstr, fal_acl_udf_type_t * arg_val, a_uint32_t size)
{
    if (NULL == cmdstr)
    {
        return SW_BAD_VALUE;
    }

    if (!strcasecmp(cmdstr, "l2"))
    {
        *arg_val = FAL_ACL_UDF_TYPE_L2;
    }
    else if (!strcasecmp(cmdstr, "l2snap"))
    {
        *arg_val = FAL_ACL_UDF_TYPE_L2_SNAP;
    }
    else if (!strcasecmp(cmdstr, "l3"))
    {
        *arg_val = FAL_ACL_UDF_TYPE_L3;
    }
    else if (!strcasecmp(cmdstr, "l3plus"))
    {
        *arg_val = FAL_ACL_UDF_TYPE_L3_PLUS;
    }
    else if (!strcasecmp(cmdstr, "l4"))
    {
        *arg_val = FAL_ACL_UDF_TYPE_L4;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_udf_type(char * param_name, a_uint32_t * buf,
                        a_uint32_t size)
{
    fal_acl_udf_type_t *val;

    val = (fal_acl_udf_type_t *) buf;
    dprintf("%s", param_name);

    if (FAL_ACL_UDF_TYPE_L2 == *val)
    {
        dprintf("l2");
    }
    else if (FAL_ACL_UDF_TYPE_L2_SNAP == *val)
    {
        dprintf("l2snap");
    }
    else if (FAL_ACL_UDF_TYPE_L3 == *val)
    {
        dprintf("l3");
    }
    else if (FAL_ACL_UDF_TYPE_L3_PLUS == *val)
    {
        dprintf("l3plus");
    }
    else if (FAL_ACL_UDF_TYPE_L4 == *val)
    {
        dprintf("l4");
    }
    else
    {
        dprintf("unknow");
    }
}

sw_error_t
cmd_data_check_udf_element(char *cmdstr, a_uint8_t * val, a_uint32_t * len)
{
    char *tmp = NULL;
    a_uint32_t i = 0, j;
    a_uint32_t data;

    memset(val, 0, 16);
    if (NULL == cmdstr)
    {
        return SW_BAD_VALUE;
    }

    if (0 == cmdstr[0])
    {
        return SW_BAD_VALUE;
    }

    tmp = (void *) strtok(cmdstr, "-");
    while (tmp)
    {
        if (16 <= i)
        {
            return SW_BAD_VALUE;
        }

        if ((2 < strlen(tmp)) || (0 == strlen(tmp)))
        {
            return SW_BAD_VALUE;
        }

        for (j = 0; j < strlen(tmp); j++)
        {
            if (A_FALSE == is_hex(tmp[j]))
            {
                return SW_BAD_VALUE;
            }
        }

        sscanf(tmp, "%x", &data);

        val[i++] = data & 0xff;
        tmp = (void *) strtok(NULL, "-");
    }

    if (0 == i)
    {
        return SW_BAD_VALUE;
    }

    *len = i;
    return SW_OK;
}

void
cmd_data_print_udf_element(char * param_name, a_uint32_t * buf,
                           a_uint32_t size)
{
    a_uint8_t *val, i;

    if (size)
    {
        val = (a_uint8_t *) buf;
        dprintf("%s", param_name);

        for (i = 0; i < (size - 1); i++)
        {
            dprintf("%02x-", *val);
            val++;
        }
        dprintf("%02x", *val);
    }
}


sw_error_t
cmd_data_check_udf_field(fal_acl_rule_t * entry)
{
    char *cmd;
    a_uint32_t tmpdata, vlen, mlen;

    /* get udf field configuration */
    cmd_data_check_element("user define field", "no",
                           "usage: <yes/no/y/n>\n", cmd_data_check_confirm,
                           (cmd, A_FALSE, &tmpdata, sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("udf type", NULL,
                               "usage: <l2/l3>\n",
                               cmd_data_check_udf_type, (cmd,
                                       &(entry->udf_type), 4));

        cmd_data_check_element("udf offset", NULL,
                               "usage: <0-126, must be even>\n",
                               cmd_data_check_uint32, (cmd, &tmpdata, vlen));
        entry->udf_offset = tmpdata;

        cmd_data_check_element("udf value", NULL,
                               "usage: the format is xx-xx-xx-xx-xx\n",
                               cmd_data_check_udf_element, (cmd,
                                       &(entry->udf_val[0]), &vlen));

        cmd_data_check_element("udf mask", NULL,
                               "usage: the format is xx-xx-xx-xx-xx\n",
                               cmd_data_check_udf_element, (cmd,
                                       &(entry->udf_mask[0]), &mlen));

        if (vlen != mlen)
        {
            return SW_BAD_VALUE;
        }
        entry->udf_len = vlen;

        FAL_FIELD_FLG_SET(entry->field_flg, FAL_ACL_FIELD_UDF);
    }
    return SW_OK;
}

sw_error_t
cmd_data_check_acl_action(fal_acl_rule_t * entry)
{
    char *cmd;
    a_uint32_t tmpdata;

    /* get permit action configuration */
    cmd_data_check_element("permit", "yes", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_TRUE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_PERMIT);
    }

    /* get deny action configuration */
    cmd_data_check_element("deny", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_DENY);
    }

    /* get redirect to cpu action configuration */
    cmd_data_check_element("rdt to cpu", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_RDTCPU);
    }

    /* get port redirection action configuration */
    cmd_data_check_element("rdt to port", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("dst port", "null",
                               "usage: input port number such as 1,3\n",
                               cmd_data_check_portmap, (cmd, &entry->ports,
                                       sizeof (fal_pbmp_t)));
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REDPT);
    }

    /* get copy to cpu action configuration */
    cmd_data_check_element("copy to cpu", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_CPYCPU);
    }

    /* get mirror action configuration */
    cmd_data_check_element("mirror", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_MIRROR);
    }

    /* get remark dscp action configuration */
    cmd_data_check_element("remark dscp", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("dscp", NULL,
                               "usage: the format is 0x0-0x3f or 0-63 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x3f,
                                       0x0));
        entry->dscp = tmpdata & 0x3f;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_DSCP);
    }

    /* get remark up action configuration */
    cmd_data_check_element("remark up", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("up", NULL,
                               "usage: the format is 0x0-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->up = tmpdata & 0x7;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_UP);
    }

    /* get remark queue action configuration */
    cmd_data_check_element("remark queue", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        cmd_data_check_element("queue", NULL,
                               "usage: the format is 0x0-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->queue = tmpdata & 0x7;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_QUEUE);
    }

    /* get modify vlan action configuration */
    cmd_data_check_element("modify vlan", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("vlan", NULL,
                               "usage: the format is 0x0-0xfff or 0-4095 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xfff,
                                       0x0));
        entry->vid = tmpdata & 0xfff;
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_MODIFY_VLAN);

        if (!FAL_ACTION_FLG_TST(entry->action_flg, FAL_ACL_ACTION_REDPT))
        {
            cmd_data_check_element("port member", "null",
                                   "usage: input port number such as 1,3\n",
                                   cmd_data_check_portmap, (cmd, &entry->ports,
                                           sizeof (fal_pbmp_t)));
        }
    }

    /* get nest vlan action configuration */
    cmd_data_check_element("nest vlan", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("vlan", NULL,
                               "usage: the format is 0x1-0xfff or 1-4095 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xfff,
                                       0x1));
        entry->vid = tmpdata & 0xfff;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_NEST_VLAN);
    }

    cmd_data_check_element("stag vid", "0",
                           "usage: the format is 0x0-0xfff or 0-4095 \n",
                           cmd_data_check_integer, (cmd, &tmpdata, 0xfff,
                                   0x0));
    entry->stag_vid = tmpdata & 0xfff;

    cmd_data_check_element("ctag vid", "0",
                           "usage: the format is 0x0-0xfff or 0-4095 \n",
                           cmd_data_check_integer, (cmd, &tmpdata, 0xfff,
                                   0x0));
    entry->ctag_vid = tmpdata & 0xfff;

    /* chang lookup vid action configuration */
    cmd_data_check_element("lookup vid change", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_LOOKUP_VID);
    }

    /* chang stag vid action configuration */
    cmd_data_check_element("stag vid change", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_STAG_VID);
    }

    /* chang stag pri action configuration */
    cmd_data_check_element("stag pri change", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("stag pri", NULL,
                               "usage: the format is 0x1-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->stag_pri = tmpdata & 0x7;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_STAG_PRI);
    }

    /* chang stag dei action configuration */
    cmd_data_check_element("stag dei change", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("stag dei", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->stag_dei = tmpdata & 0x1;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_STAG_DEI);
    }

    /* chang ctag vid action configuration */
    cmd_data_check_element("ctag vid change", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_CTAG_VID);
    }


    /* chang ctag pri action configuration */
    cmd_data_check_element("ctag pri change", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("ctag pri", NULL,
                               "usage: the format is 0x1-0x7 or 0-7 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x7,
                                       0x0));
        entry->ctag_pri = tmpdata & 0x7;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_CTAG_PRI);
    }

    /* chang ctag cfi action configuration */
    cmd_data_check_element("ctag cfi change", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("ctag cfi", NULL,
                               "usage: the format is 0x0-0x1 or 0-1 \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0x1,
                                       0x0));
        entry->ctag_cfi = tmpdata & 0x1;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_REMARK_CTAG_CFI);
    }

    /* police action configuration */
    cmd_data_check_element("police en", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("policer ptr", NULL,
                               "usage: the format is integer \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xffffffff,
                                       0x0));
        entry->policer_ptr = tmpdata;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_POLICER_EN);
    }

    /* wcmp action configuration */
    cmd_data_check_element("wcmp en", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("wcmp ptr", NULL,
                               "usage: the format is integer \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xffffffff,
                                       0x0));
        entry->wcmp_ptr = tmpdata;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_WCMP_EN);
    }

    /* arp action configuration */
    cmd_data_check_element("arp en", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("arp ptr", "0",
                               "usage: the format is integer \n",
                               cmd_data_check_integer, (cmd, &tmpdata, 0xffffffff,
                                       0x0));
        entry->arp_ptr = tmpdata;

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_ARP_EN);
    }

    /* policy forward action configuration */
    cmd_data_check_element("policy en", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));
    if (A_TRUE == tmpdata)
    {
        cmd_data_check_element("route", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (tmpdata)
        {
            entry->policy_fwd = FAL_ACL_POLICY_ROUTE;
        }

        cmd_data_check_element("snat", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (tmpdata)
        {
            entry->policy_fwd = FAL_ACL_POLICY_SNAT;
        }

        cmd_data_check_element("dnat", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (tmpdata)
        {
            entry->policy_fwd = FAL_ACL_POLICY_DNAT;
        }

        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_POLICY_FORWARD_EN);
    }

    cmd_data_check_element("eg bypass", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_BYPASS_EGRESS_TRANS);
    }

    cmd_data_check_element("trigger intr", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        FAL_ACTION_FLG_SET(entry->action_flg, FAL_ACL_ACTION_MATCH_TRIGGER_INTR);
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_aclrule(char *info, void *val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_acl_rule_t entry;
    a_uint32_t tmpdata;

    memset(&entry, 0, sizeof (fal_acl_rule_t));

    dprintf("\n");

    /* get rule type configuration */
    cmd_data_check_element("rule type", NULL, "usage: <mac/ip4/ip6/udf> \n",
                           cmd_data_check_ruletype, (cmd, &entry.rule_type,
                                   sizeof
                                   (fal_acl_rule_type_t)));

    if (FAL_ACL_RULE_MAC == entry.rule_type)
    {
        rv = cmd_data_check_mac_field(&entry);
        if (SW_OK != rv)
        {
            return rv;
        }
    }

    if (FAL_ACL_RULE_IP4 == entry.rule_type)
    {
        rv = cmd_data_check_mac_field(&entry);
        if (SW_OK != rv)
        {
            return rv;
        }

        rv = cmd_data_check_ip4_field(&entry);
        if (SW_OK != rv)
        {
            return rv;
        }

        rv = cmd_data_check_ip_field(&entry);
        if (SW_OK != rv)
        {
            return rv;
        }
    }

    if (FAL_ACL_RULE_IP6 == entry.rule_type)
    {
        rv = cmd_data_check_mac_field(&entry);
        if (SW_OK != rv)
        {
            return rv;
        }

        rv = cmd_data_check_ip6_field(&entry);
        if (SW_OK != rv)
        {
            return rv;
        }

        rv = cmd_data_check_ip_field(&entry);
        if (SW_OK != rv)
        {
            return rv;
        }
    }

    rv = cmd_data_check_udf_field(&entry);
    if (SW_OK != rv)
    {
        return rv;
    }

    /* get rule inverse configuration */
    cmd_data_check_element("rule inverse", "no", "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                   sizeof (a_bool_t)));

    if (tmpdata)
    {
        FAL_FIELD_FLG_SET(entry.field_flg, FAL_ACL_FIELD_INVERSE_ALL);
    }

    rv = cmd_data_check_acl_action(&entry);
    if (SW_OK != rv)
    {
        return rv;
    }

    *(fal_acl_rule_t *) val = entry;
    return SW_OK;
}

void
cmd_data_print_aclrule(char * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    fal_acl_rule_t *rule;

    rule = (fal_acl_rule_t *) buf;

    cmd_data_print_ruletype("\n[rule_type]:",
                            (a_uint32_t *) & (rule->rule_type),
                            sizeof (fal_acl_rule_type_t));

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_DA))
    {
        cmd_data_print_macaddr("\n[mac_dst_addr]:",
                               (a_uint32_t *) & (rule->dest_mac_val),
                               sizeof (fal_mac_addr_t));
        cmd_data_print_macaddr("  [mac_dst_addr_mask]:",
                               (a_uint32_t *) & (rule->dest_mac_mask),
                               sizeof (fal_mac_addr_t));
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_SA))
    {
        cmd_data_print_macaddr("\n[mac_src_addr]:",
                               (a_uint32_t *) & (rule->src_mac_val),
                               sizeof (fal_mac_addr_t));
        cmd_data_print_macaddr("  [mac_src_addr_mask]:",
                               (a_uint32_t *) & (rule->src_mac_mask),
                               sizeof (fal_mac_addr_t));
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_ETHTYPE))
    {
        dprintf("\n[mac_eth_type]:0x%x", rule->ethtype_val);
        dprintf("  [mac_eth_type_mask]:0x%x", rule->ethtype_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_TAGGED))
    {
        dprintf("\n[mac_tagged]:0x%x", rule->tagged_val);
        dprintf("  [mac_tagged_mask]:0x%x", rule->tagged_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_UP))
    {
        dprintf("\n[mac_up]:0x%x", rule->up_val);
        dprintf("  [mac_up_mask]:0x%x", rule->up_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CFI))
    {
        dprintf("\n[mac_cfi]:0x%x", rule->cfi_val);
        dprintf("  [mac_cfi_mask]:0x%x", rule->cfi_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_VID))
    {
        cmd_data_print_fieldop("\n[mac_vlanid_op]:",
                               (a_uint32_t *) & (rule->vid_op),
                               sizeof (fal_acl_field_op_t));
        if (FAL_ACL_FIELD_MASK == rule->vid_op)
        {
            dprintf("  [vlanid]:0x%x", rule->vid_val);
            dprintf("  [vlanid_mask]:0x%x", rule->vid_mask);
        }
        else
        {
            dprintf("  [vlanid_low]:0x%x", rule->vid_val);
            dprintf("  [vlanid_high]:0x%x", rule->vid_mask);
        }
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAGGED))
    {
        dprintf("\n[mac_stagged]:0x%x", rule->stagged_val);
        dprintf("  [mac_stagged_mask]:0x%x", rule->stagged_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAG_PRI))
    {
        dprintf("\n[mac_stag_pri]:0x%x", rule->stag_pri_val);
        dprintf("  [mac_stag_pri_mask]:0x%x", rule->stag_pri_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAG_DEI))
    {
        dprintf("\n[mac_stag_dei]:0x%x", rule->stag_dei_val);
        dprintf("  [mac_stag_dei_mask]:0x%x", rule->stag_dei_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_STAG_VID))
    {
        cmd_data_print_fieldop("\n[mac_stag_vlanid_op]:",
                               (a_uint32_t *) & (rule->stag_vid_op),
                               sizeof (fal_acl_field_op_t));
        if (FAL_ACL_FIELD_MASK == rule->stag_vid_op)
        {
            dprintf("  [stag_vlanid]:0x%x", rule->stag_vid_val);
            dprintf("  [stag_vlanid_mask]:0x%x", rule->stag_vid_mask);
        }
        else
        {
            dprintf("  [stag_vlanid_low]:0x%x", rule->stag_vid_val);
            dprintf("  [stag_vlanid_high]:0x%x", rule->stag_vid_mask);
        }
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAGGED))
    {
        dprintf("\n[mac_ctagged]:0x%x", rule->ctagged_val);
        dprintf("  [mac_ctagged_mask]:0x%x", rule->ctagged_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_PRI))
    {
        dprintf("\n[mac_ctag_pri]:0x%x", rule->ctag_pri_val);
        dprintf("  [mac_ctag_pri_mask]:0x%x", rule->ctag_pri_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_CFI))
    {
        dprintf("\n[mac_ctag_cfi]:0x%x", rule->ctag_cfi_val);
        dprintf("  [mac_ctag_cfi_mask]:0x%x", rule->ctag_cfi_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_VID))
    {
        cmd_data_print_fieldop("\n[mac_ctag_vlanid_op]:",
                               (a_uint32_t *) & (rule->ctag_vid_op),
                               sizeof (fal_acl_field_op_t));
        if (FAL_ACL_FIELD_MASK == rule->ctag_vid_op)
        {
            dprintf("  [ctag_vlanid]:0x%x", rule->ctag_vid_val);
            dprintf("  [ctag_vlanid_mask]:0x%x", rule->ctag_vid_mask);
        }
        else
        {
            dprintf("  [ctag_vlanid_low]:0x%x", rule->ctag_vid_val);
            dprintf("  [ctag_vlanid_high]:0x%x", rule->ctag_vid_mask);
        }
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP4_DIP))
    {
        cmd_data_print_ip4addr("\n[ip4_dst_addr]:",
                               (a_uint32_t *) & (rule->dest_ip4_val),
                               sizeof (fal_ip4_addr_t));
        cmd_data_print_ip4addr("  [ip4_dst_addr_mask]:",
                               (a_uint32_t *) & (rule->dest_ip4_mask),
                               sizeof (fal_ip4_addr_t));
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP4_SIP))
    {
        cmd_data_print_ip4addr("\n[ip4_src_addr]:",
                               (a_uint32_t *) & (rule->src_ip4_val),
                               sizeof (fal_ip4_addr_t));
        cmd_data_print_ip4addr("  [ip4_src_addr_mask]:",
                               (a_uint32_t *) & (rule->src_ip4_mask),
                               sizeof (fal_ip4_addr_t));
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_RIPV1))
    {
        dprintf("\n[ip4_ripv1]:0x%x", rule->ripv1_val);
        dprintf("  [ip4_ripv1_mask]:0x%x", rule->ripv1_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_DHCPV4))
    {
        dprintf("\n[ip4_dhcpv4]:0x%x", rule->dhcpv4_val);
        dprintf("  [ip4_dhcpv4_mask]:0x%x", rule->dhcpv4_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP6_DIP))
    {
        cmd_data_print_ip6addr("\n[ip6_dst_addr]:",
                               (a_uint32_t *) & (rule->dest_ip6_val),
                               sizeof (fal_ip6_addr_t));
        cmd_data_print_ip6addr("\n[ip6_dst_addr_mask]:",
                               (a_uint32_t *) & (rule->dest_ip6_mask),
                               sizeof (fal_ip6_addr_t));
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP6_SIP))
    {
        cmd_data_print_ip6addr("\n[ip6_src_addr]:",
                               (a_uint32_t *) & (rule->src_ip6_val),
                               sizeof (fal_ip6_addr_t));
        cmd_data_print_ip6addr("\n[ip6_src_addr_mask]:",
                               (a_uint32_t *) & (rule->src_ip6_mask),
                               sizeof (fal_ip6_addr_t));
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP6_LABEL))
    {
        dprintf("\n[ip6_flow_label]:0x%x", rule->ip6_lable_val);
        dprintf("  [ip6_flow_label_mask]:0x%x", rule->ip6_lable_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_DHCPV6))
    {
        dprintf("\n[ip6_dhcpv6]:0x%x", rule->dhcpv6_val);
        dprintf("  [ip6_dhcpv6_mask]:0x%x", rule->dhcpv6_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP_PROTO))
    {
        dprintf("\n[ip_proto]:0x%x", rule->ip_proto_val);
        dprintf("  [ip_proto_mask]:0x%x", rule->ip_proto_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_IP_DSCP))
    {
        dprintf("\n[ip_dscp]:0x%x", rule->ip_dscp_val);
        dprintf("  [ip_dscp_mask]:0x%x", rule->ip_dscp_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L4_DPORT))
    {
        cmd_data_print_fieldop("\n[ip_l4_dport_op]:",
                               (a_uint32_t *) & (rule->dest_l4port_op),
                               sizeof (fal_acl_field_op_t));
        if (FAL_ACL_FIELD_MASK == rule->dest_l4port_op)
        {
            dprintf("  [dport]:0x%x", rule->dest_l4port_val);
            dprintf("  [dport_mask]:0x%x", rule->dest_l4port_mask);
        }
        else
        {
            dprintf("  [dport_low]:0x%x", rule->dest_l4port_val);
            dprintf("  [dport_high]:0x%x", rule->dest_l4port_mask);
        }
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_L4_SPORT))
    {
        cmd_data_print_fieldop("\n[ip_l4_sport_op]:",
                               (a_uint32_t *) & (rule->src_l4port_op),
                               sizeof (fal_acl_field_op_t));
        if (FAL_ACL_FIELD_MASK == rule->src_l4port_op)
        {
            dprintf("  [sport]:0x%x", rule->src_l4port_val);
            dprintf("  [sport_mask]:0x%x", rule->src_l4port_mask);
        }
        else
        {
            dprintf("  [sport_low]:0x%x", rule->src_l4port_val);
            dprintf("  [sport_high]:0x%x", rule->src_l4port_mask);
        }
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_TCP_FLAG))
    {
        dprintf("\n[ip_tcp_flags]:0x%x", rule->tcp_flag_val);
        dprintf("  [ip_tcp_flags_mask]:0x%x", rule->tcp_flag_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ICMP_TYPE))
    {
        dprintf("\n[ip_icmp_type]:0x%x", rule->icmp_type_val);
        dprintf("  [ip_icmp_type_mask]:0x%x", rule->icmp_type_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_ICMP_CODE))
    {
        dprintf("\n[ip_icmp_code]:0x%x", rule->icmp_code_val);
        dprintf("  [ip_icmp_code_mask]:0x%x", rule->icmp_code_mask);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF))
    {
        cmd_data_print_udf_type("\n[udf_type]:",
                                (a_uint32_t *) & (rule->udf_type),
                                sizeof (fal_acl_udf_type_t));

        dprintf("  [offset]:%d", rule->udf_offset);

        cmd_data_print_udf_element("\n[udf_value]:",
                                   (a_uint32_t *) & (rule->udf_val[0]),
                                   rule->udf_len);

        cmd_data_print_udf_element("\n[udf_mask]:",
                                   (a_uint32_t *) & (rule->udf_mask[0]),
                                   rule->udf_len);
    }

    if (FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_INVERSE_ALL))
    {
        dprintf("\n[rule_inverse]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_PERMIT))
    {
        dprintf("\n[permit]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_DENY))
    {
        dprintf("\n[deny]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_RDTCPU))
    {
        dprintf("\n[rdt_to_cpu]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_CPYCPU))
    {
        dprintf("\n[cpy_to_cpu]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_MIRROR))
    {
        dprintf("\n[mirror]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REDPT))
    {
        dprintf("\n[rdt_to_port]:yes");
        cmd_data_print_portmap("  [dest_port]:", rule->ports,
                               sizeof (fal_pbmp_t));
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_MODIFY_VLAN))
    {
        dprintf("\n[modify_vlan_id]:yes");
        dprintf("  [vlan_id]:%d", rule->vid);
        if (!FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REDPT))
        {
            cmd_data_print_portmap("  [port_member]:", rule->ports,
                                   sizeof (fal_pbmp_t));
        }
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_NEST_VLAN))
    {
        dprintf("\n[nest_vlan]:yes");
        dprintf("  [vlan_id]:%d", rule->vid);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_DSCP))
    {
        dprintf("\n[remark_dscp]:yes");
        dprintf("  [dscp]:%d", rule->dscp);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_UP))
    {
        dprintf("\n[remark_up]:yes");
        dprintf("  [up]:%d", rule->up);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_QUEUE))
    {
        dprintf("\n[remark_queue]:yes");
        dprintf("  [queue]:%d", rule->queue);
    }

    dprintf("\n[stag_vid]:%d", rule->stag_vid);
    dprintf("\n[ctag_vid]:%d", rule->ctag_vid);

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_LOOKUP_VID))
    {
        dprintf("\n[change_lookup_vid]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_VID))
    {
        dprintf("\n[change_stag_vid]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_VID))
    {
        dprintf("\n[change_ctag_vid]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_PRI))
    {
        dprintf("\n[change_stag_pri]:yes");
        dprintf("  [stag_pri]:%d", rule->stag_pri);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_DEI))
    {
        dprintf("\n[change_stag_dei]:yes");
        dprintf("  [stag_dei]:%d", rule->stag_dei);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_PRI))
    {
        dprintf("\n[change_ctag_pri]:yes");
        dprintf("  [ctag_pri]:%d", rule->ctag_pri);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_CFI))
    {
        dprintf("\n[change_ctag_cfi]:yes");
        dprintf("  [ctag_cfi]:%d", rule->ctag_cfi);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_POLICER_EN))
    {
        dprintf("\n[police_en]:yes");
        dprintf("  [policer_ptr]:%d", rule->policer_ptr);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_WCMP_EN))
    {
        dprintf("\n[wcmp_en]:yes");
        dprintf("  [wcmp_ptr]:%d", rule->wcmp_ptr);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_ARP_EN))
    {
        dprintf("\n[arp_en]:yes");
        dprintf("  [arp_ptr]:%d", rule->arp_ptr);
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_POLICY_FORWARD_EN))
    {
        if (FAL_ACL_POLICY_ROUTE == rule->policy_fwd)
        {
            dprintf("\n[policy_forward]:route");
        }

        if (FAL_ACL_POLICY_SNAT == rule->policy_fwd)
        {
            dprintf("\n[policy_forward]:snat");
        }

        if (FAL_ACL_POLICY_DNAT == rule->policy_fwd)
        {
            dprintf("\n[policy_forward]:dnat");
        }
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_BYPASS_EGRESS_TRANS))
    {
        dprintf("\n[eg_bypass]:yes");
    }

    if (FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_MATCH_TRIGGER_INTR))
    {
        dprintf("\n[trigger_intr]:yes");
    }

    dprintf("\n[match_counter]:%d", rule->match_cnt);

    return;
}

sw_error_t
cmd_data_check_patternmode(char *cmd_str, led_pattern_mode_t * arg_val,
                           a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (!strcasecmp(cmd_str, "always_off"))
    {
        *arg_val = LED_ALWAYS_OFF;
    }
    else if (!strcasecmp(cmd_str, "always_blink"))
    {
        *arg_val = LED_ALWAYS_BLINK;
    }
    else if (!strcasecmp(cmd_str, "always_on"))
    {
        *arg_val = LED_ALWAYS_ON;
    }
    else  if (!strcasecmp(cmd_str, "map"))
    {
        *arg_val = LED_PATTERN_MAP_EN;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_blinkfreq(char *cmd_str, led_blink_freq_t * arg_val,
                         a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (!strcasecmp(cmd_str, "2HZ"))
    {
        *arg_val = LED_BLINK_2HZ;
    }
    else if (!strcasecmp(cmd_str, "4HZ"))
    {
        *arg_val = LED_BLINK_4HZ;
    }
    else if (!strcasecmp(cmd_str, "8HZ"))
    {
        *arg_val = LED_BLINK_8HZ;
    }
    else if (!strcasecmp(cmd_str, "TXRX"))
    {
        *arg_val = LED_BLINK_TXRX;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_ledpattern(char *info, void * val, a_uint32_t size)
{
    char *cmd;
    led_ctrl_pattern_t pattern;
    a_uint32_t tmpdata;

    memset(&pattern, 0, sizeof (led_ctrl_pattern_t));

    dprintf("\n");

    /* get pattern mode configuration */
    cmd_data_check_element("pattern_mode", NULL, "usage: <always_off/always_blink/always_on/map>\n",
                           cmd_data_check_patternmode, (cmd, &pattern.mode,
                                   sizeof(led_pattern_mode_t)));

    if (LED_PATTERN_MAP_EN == pattern.mode)
    {
        cmd_data_check_element("full_duplex_light", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << FULL_DUPLEX_LIGHT_EN);
        }

        cmd_data_check_element("half_duplex_light", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << HALF_DUPLEX_LIGHT_EN);
        }

        cmd_data_check_element("power_on_light", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << POWER_ON_LIGHT_EN);
        }

        cmd_data_check_element("link_1000m_light", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << LINK_1000M_LIGHT_EN);
        }

        cmd_data_check_element("link_100m_light", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << LINK_100M_LIGHT_EN);
        }

        cmd_data_check_element("link_10m_light", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << LINK_10M_LIGHT_EN);
        }

        cmd_data_check_element("conllision_light", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << COLLISION_BLINK_EN);
        }

        cmd_data_check_element("rx_traffic_blink", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << RX_TRAFFIC_BLINK_EN);
        }

        cmd_data_check_element("tx_traffic_blink", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << TX_TRAFFIC_BLINK_EN);
        }

        cmd_data_check_element("linkup_override_light", "no", "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &tmpdata,
                                       sizeof (a_bool_t)));
        if (1 == tmpdata)
        {
            pattern.map |= (1 << LINKUP_OVERRIDE_EN);
        }

        cmd_data_check_element("blink freq", NULL, "usage: <2HZ/4HZ/8HZ/TXRX> \n",
                               cmd_data_check_blinkfreq, (cmd, &pattern.freq,
                                       sizeof(led_blink_freq_t)));
    }

    *(led_ctrl_pattern_t *)val = pattern;

    return SW_OK;
}

void
cmd_data_print_ledpattern(a_uint8_t * param_name, a_uint32_t * buf,
                          a_uint32_t size)
{
    led_ctrl_pattern_t *pattern;

    pattern = (led_ctrl_pattern_t *) buf;

    if (LED_ALWAYS_OFF == pattern->mode)
    {
        dprintf("[pattern_mode]:always_off");
    }
    else if (LED_ALWAYS_BLINK == pattern->mode)
    {
        dprintf("[pattern_mode]:always_blink");
    }
    else if (LED_ALWAYS_ON == pattern->mode)
    {
        dprintf("[pattern_mode]:always_on");
    }
    else
    {
        dprintf("[pattern_mode]:map");
    }
    dprintf("\n");

    if (LED_PATTERN_MAP_EN == pattern->mode)
    {
        if (pattern->map & (1 << FULL_DUPLEX_LIGHT_EN))
        {
            cmd_data_print_confirm("[full_duplex_light]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << HALF_DUPLEX_LIGHT_EN))
        {
            cmd_data_print_confirm("[half_duplex_light]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << POWER_ON_LIGHT_EN))
        {
            cmd_data_print_confirm("[power_on_light]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << LINK_1000M_LIGHT_EN))
        {
            cmd_data_print_confirm("[link_1000m_light]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << LINK_100M_LIGHT_EN))
        {
            cmd_data_print_confirm("[link_100m_light]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << LINK_10M_LIGHT_EN))
        {
            cmd_data_print_confirm("[link_10m_light]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << COLLISION_BLINK_EN))
        {
            cmd_data_print_confirm("[conllision_blink]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << RX_TRAFFIC_BLINK_EN))
        {
            cmd_data_print_confirm("[rx_traffic_blink]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << TX_TRAFFIC_BLINK_EN))
        {
            cmd_data_print_confirm("[tx_traffic_blink]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (pattern->map & (1 << LINKUP_OVERRIDE_EN))
        {
            cmd_data_print_confirm("[linkup_override]:", A_TRUE, sizeof (a_bool_t));
            dprintf("\n");
        }

        if (LED_BLINK_2HZ == pattern->freq)
        {
            dprintf("[blink_frequency]:2HZ\n");
        }
        else if (LED_BLINK_4HZ == pattern->freq)
        {
            dprintf("[blink_frequency]:4HZ\n");
        }
        else if (LED_BLINK_8HZ == pattern->freq)
        {
            dprintf("[blink_frequency]:8HZ\n");
        }
        else
        {
            dprintf("[blink_frequency]:TXRX\n");
        }
    }
}

/*Shiva*/
sw_error_t
cmd_data_check_invlan_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "admit_all"))
    {
        *arg_val = FAL_INVLAN_ADMIT_ALL;
    }
    else if (!strcasecmp(cmd_str, "admit_tagged"))
    {
        *arg_val = FAL_INVLAN_ADMIT_TAGGED;
    }
    else if (!strcasecmp(cmd_str, "admit_untagged"))
    {
        *arg_val = FAL_INVLAN_ADMIT_UNTAGGED;
    }
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_invlan_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);

    if (*(a_uint32_t *) buf == FAL_INVLAN_ADMIT_ALL)
    {
        dprintf("ADMIT_ALL");
    }
    else if (*(a_uint32_t *) buf == FAL_INVLAN_ADMIT_TAGGED)
    {
        dprintf("ADMIT_TAGGED");
    }
    else if (*(a_uint32_t *) buf == FAL_INVLAN_ADMIT_UNTAGGED)
    {
        dprintf("ADMIT_UNTAGGED");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_vlan_propagation(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "disable"))
    {
        *arg_val = FAL_VLAN_PROPAGATION_DISABLE;
    }
    else if (!strcasecmp(cmd_str, "clone"))
    {
        *arg_val = FAL_VLAN_PROPAGATION_CLONE;
    }
    else if (!strcasecmp(cmd_str, "replace"))
    {
        *arg_val = FAL_VLAN_PROPAGATION_REPLACE;
    }
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_vlan_propagation(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);

    if (*(a_uint32_t *) buf == FAL_VLAN_PROPAGATION_DISABLE)
    {
        dprintf("DISABLE");
    }
    else if (*(a_uint32_t *) buf == FAL_VLAN_PROPAGATION_CLONE)
    {
        dprintf("CLONE");
    }
    else if (*(a_uint32_t *) buf == FAL_VLAN_PROPAGATION_REPLACE)
    {
        dprintf("REPLACE");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_vlan_translation(char *info, fal_vlan_trans_entry_t *val, a_uint32_t size)
{
    char *cmd = NULL;
    sw_error_t rv;
    fal_vlan_trans_entry_t entry;

    memset(&entry, 0, sizeof (fal_vlan_trans_entry_t));

    do
    {
        cmd = get_sub_cmd("ovid", "1");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 4095\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &entry.o_vid, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 0 -- 4095\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("bi direction", "yes");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.bi_dir,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("forward direction", "yes");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.forward_dir,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("reverse direction", "yes");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.reverse_dir,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("svid", "1");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 4095\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &entry.s_vid, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 0 -- 4095\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("cvid", "1");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 4095\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &entry.c_vid, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 0 -- 4095\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("ovid_is_cvid", "yes");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.o_vid_is_cvid,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("svid_enable", "yes");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.s_vid_enable,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("cvid_enable", "yes");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.c_vid_enable,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("one_2_one_vlan", "no");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;

        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;

        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.one_2_one_vlan,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    *val = entry;
    return SW_OK;
}

void
cmd_data_print_vlan_translation(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_vlan_trans_entry_t *entry;

    entry = (fal_vlan_trans_entry_t *) buf;
    dprintf("[Ovid]:0x%x  [Svid]:0x%x  [Cvid]:0x%x  [BiDirect]:%s  [ForwardDirect]:%s  [ReverseDirect]:%s  ",
            entry->o_vid, entry->s_vid, entry->c_vid,
            entry->bi_dir?"ENABLE":"DISABLE",
            entry->forward_dir?"ENABLE":"DISABLE",
            entry->reverse_dir?"ENABLE":"DISABLE");

    dprintf("[OvidIsCvid]:%s  [SvidEnable]:%s  [CvidEnable]:%s  [One2OneVlan]:%s",
            entry->o_vid_is_cvid?"YES":"NO",
            entry->s_vid_enable?"YES":"NO",
            entry->c_vid_enable?"YES":"NO",
            entry->one_2_one_vlan?"YES":"NO");

}

sw_error_t
cmd_data_check_qinq_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "ctag"))
    {
        *arg_val = FAL_QINQ_CTAG_MODE;
    }
    else if (!strcasecmp(cmd_str, "stag"))
    {
        *arg_val = FAL_QINQ_STAG_MODE;
    }
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_qinq_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);

    if (*(a_uint32_t *) buf == FAL_QINQ_CTAG_MODE)
    {
        dprintf("CTAG");
    }
    else if (*(a_uint32_t *) buf == FAL_QINQ_STAG_MODE)
    {
        dprintf("STAG");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_qinq_role(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "edge"))
    {
        *arg_val = FAL_QINQ_EDGE_PORT;
    }
    else if (!strcasecmp(cmd_str, "core"))
    {
        *arg_val = FAL_QINQ_CORE_PORT;
    }
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_qinq_role(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);

    if (*(a_uint32_t *) buf == FAL_QINQ_EDGE_PORT)
    {
        dprintf("EDGE");
    }
    else if (*(a_uint32_t *) buf == FAL_QINQ_CORE_PORT)
    {
        dprintf("CORE");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

void
cmd_data_print_cable_status(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);

    if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_NORMAL)
    {
        dprintf("NORMAL");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_SHORT)
    {
        dprintf("SHORT");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_OPENED)
    {
        dprintf("OPENED");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_INVALID)
    {
        dprintf("INVALID");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

void
cmd_data_print_cable_len(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:%d", param_name, *(a_uint32_t *) buf);
}

static char*
cmd_cpu_mode(hsl_init_mode mode)
{
    switch (mode)
    {
        case HSL_NO_CPU:
            return "no_cpu";
        case HSL_CPU_1:
            return "cpu_1";
        case HSL_CPU_2:
            return "cpu_2";
        case HSL_CPU_1_PLUS:
            return "cpu_1_plus";
    }

    return "unknow";
}

static char*
cmd_access_mode(hsl_access_mode mode)
{
    switch (mode)
    {
        case HSL_MDIO:
            return "mdio";
        case HSL_HEADER:
            return "header";
    }

    return "unknow";
}

static void
_cmd_collect_shell_cfg(ssdk_cfg_t *shell_cfg)
{
    memset(shell_cfg, 0, sizeof(ssdk_cfg_t));
    shell_cfg->init_cfg = init_cfg;

#ifdef VERSION
    aos_mem_copy(shell_cfg->build_ver, VERSION, sizeof(VERSION));
#endif

#ifdef BUILD_DATE
    aos_mem_copy(shell_cfg->build_date, BUILD_DATE, sizeof(BUILD_DATE));
#endif

    if (ssdk_cfg.init_cfg.chip_type == CHIP_ATHENA)
        aos_mem_copy(shell_cfg->chip_type, "athena", sizeof("athena"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_GARUDA)
        aos_mem_copy(shell_cfg->chip_type, "garuda", sizeof("garuda"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_SHIVA)
        aos_mem_copy(shell_cfg->chip_type, "shiva", sizeof("shiva"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_HORUS)
        aos_mem_copy(shell_cfg->chip_type, "horus", sizeof("horus"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_ISIS)
        aos_mem_copy(shell_cfg->chip_type, "isis", sizeof("isis"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_ISISC)
        aos_mem_copy(shell_cfg->chip_type, "isisc", sizeof("isisc"));

#ifdef CPU
    aos_mem_copy(shell_cfg->cpu_type, CPU, sizeof(CPU));
#endif

#ifdef OS
    aos_mem_copy(shell_cfg->os_info, OS, sizeof(OS));
#if defined KVER26
    aos_mem_copy(shell_cfg->os_info+sizeof(OS)-1, " version 2.6", sizeof(" version 2.6"));
#elif defined KVER24
    aos_mem_copy(shell_cfg->os_info+sizeof(OS)-1, " version 2.4", sizeof(" version 2.4"));
#else
    aos_mem_copy(shell_cfg->os_info+sizeof(OS)-1, " version unknown", sizeof(" version unknown"));
#endif
#endif

#ifdef HSL_STANDALONG
    shell_cfg->fal_mod = A_FALSE;
#else
    shell_cfg->fal_mod = A_TRUE;
#endif

#ifdef USER_MODE
    shell_cfg->kernel_mode = A_FALSE;
#else
    shell_cfg->kernel_mode = A_TRUE;
#endif

#ifdef UK_IF
    shell_cfg->uk_if = A_TRUE;
#else
    shell_cfg->uk_if = A_FALSE;
#endif

    return;
}

#define BOOL2STR(val_bool) (((val_bool)==A_TRUE)?"true":"false" )
#define BOOL2NAME(val_bool) (((feature->in_##val_bool)==A_TRUE)?(#val_bool):"" )
#define DEFINED2STR(name) (((init->reg_func.name))?"y":"n" )

static void
_cmd_data_print_cfg(ssdk_cfg_t *entry)
{
    ssdk_init_cfg *init = &(entry->init_cfg);

    dprintf("[build verison]:%-10s [build date]:%s\n", entry->build_ver, entry->build_date);
    dprintf("[chip type]:%-14s [arch]:%-12s [os]:%s\n", entry->chip_type, entry->cpu_type, entry->os_info);
    dprintf("[fal]:%-20s [kernel mode]:%-5s [uk if]:%s\n",
            BOOL2STR(entry->fal_mod), BOOL2STR(entry->kernel_mode), BOOL2STR(entry->uk_if));

    dprintf("[cpu mode]:%-15s [reg access]:%-6s [ioctl minor]:%d\n",
            cmd_cpu_mode(init->cpu_mode), cmd_access_mode(init->reg_mode),
            init->nl_prot);

    dprintf("[inf defined]:mdio_set(%s) mdio_get(%s) header_reg_set(%s) header_reg_get(%s)\n",
            DEFINED2STR(mdio_set), DEFINED2STR(mdio_get), DEFINED2STR(header_reg_set), DEFINED2STR(header_reg_get));

}

void
cmd_data_print_ssdk_cfg(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    ssdk_cfg_t *ssdk_cfg = (ssdk_cfg_t *) buf;
    dprintf("1.SSDK CONFIGURATION:\n");
    _cmd_data_print_cfg(ssdk_cfg);

    dprintf("\n2.DEMO SHELL CONFIGURATION:\n");
    ssdk_cfg_t shell_cfg;
    _cmd_collect_shell_cfg(&shell_cfg);
    _cmd_data_print_cfg(&shell_cfg);

    dprintf("\n3.SSDK FEATURES LIST:\n");
    ssdk_features *feature = &(ssdk_cfg->features);
    dprintf("%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
            BOOL2NAME(acl), BOOL2NAME(fdb), BOOL2NAME(igmp), BOOL2NAME(leaky),
            BOOL2NAME(led), BOOL2NAME(mib), BOOL2NAME(mirror), BOOL2NAME(misc),
            BOOL2NAME(portcontrol), BOOL2NAME(portvlan), BOOL2NAME(qos), BOOL2NAME(rate),
            BOOL2NAME(stp), BOOL2NAME(vlan), BOOL2NAME(reduced_acl),
            BOOL2NAME(cosmap), BOOL2NAME(ip), BOOL2NAME(nat), BOOL2NAME(sec), BOOL2NAME(trunk), BOOL2NAME(interfacectrl));

}

sw_error_t
cmd_data_check_hdrmode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "noheader"))
    {
        *arg_val = FAL_NO_HEADER_EN;
    }
    else if (!strcasecmp(cmd_str, "onlymanagement"))
    {
        *arg_val = FAL_ONLY_MANAGE_FRAME_EN;
    }
    else if (!strcasecmp(cmd_str, "allframe"))
    {
        *arg_val = FAL_ALL_TYPE_FRAME_EN;
    }
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_hdrmode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_NO_HEADER_EN)
    {
        dprintf("NOHEADER");
    }
    else if (*(a_uint32_t *) buf == FAL_ONLY_MANAGE_FRAME_EN)
    {
        dprintf("ONLYMANAGEMENT");
    }
    else if (*(a_uint32_t *) buf == FAL_ALL_TYPE_FRAME_EN)
    {
        dprintf("ALLFRAME");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_fdboperation(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_fdb_op_t entry;

    memset(&entry, 0, sizeof (fal_fdb_op_t));

    do
    {
        cmd = get_sub_cmd("port_en", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.port_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("fid_en", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.fid_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("multi_en", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.multicast_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    *(fal_fdb_op_t *) val = entry;
    return SW_OK;
}

sw_error_t
cmd_data_check_pppoe(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_pppoe_session_t entry;

    aos_mem_zero(&entry, sizeof (fal_pppoe_session_t));

    do
    {
        cmd = get_sub_cmd("entryid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.entry_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("sessionid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &entry.session_id, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 0 -- 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("multi_session", "no");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.multi_session,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("uni_session", "no");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.uni_session,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("vrf_id", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 7\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &entry.vrf_id, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: the range is 0 -- 7\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    *(fal_pppoe_session_t*)val = entry;
    return SW_OK;
}

void
cmd_data_print_pppoe(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_pppoe_session_t *entry;

    entry = (fal_pppoe_session_t *) buf;
    dprintf("[EntryID]:0x%x  [SessionID]:0x%x  [MultiSession]:%s  [UniSession]:%s  [Vrf_ID]:0x%x",
            entry->entry_id,
            entry->session_id,
            entry->multi_session ? "YES":"NO",
            entry->uni_session ?   "YES":"NO",
            entry->vrf_id);
}

sw_error_t
cmd_data_check_host_entry(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_host_entry_t entry;

    aos_mem_zero(&entry, sizeof (fal_host_entry_t));

    do
    {
        cmd = get_sub_cmd("entryid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.entry_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("entryflags", "0x1");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: bitmap for host entry\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.flags), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: bitmap for host entry\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("entrystatus", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.status), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    if (FAL_IP_IP4_ADDR & (entry.flags))
    {
        cmd_data_check_element("ip4 addr", NULL,
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd, &(entry.ip4_addr), 4));
    }
    else
    {
        cmd_data_check_element("ip6 addr", NULL,
                               "usage: the format is xxxx::xxxx \n",
                               cmd_data_check_ip6addr, (cmd, &(entry.ip6_addr), 16));
    }

    cmd_data_check_element("mac addr", NULL,
                           "usage: the format is xx-xx-xx-xx-xx-xx \n",
                           cmd_data_check_macaddr, (cmd,
                                   &(entry.mac_addr),
                                   sizeof (fal_mac_addr_t)));

    do
    {
        cmd = get_sub_cmd("interface id", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.intf_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("load_balance num", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.lb_num), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("vrf id", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.vrf_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("port id", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.port_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("action", "forward");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_maccmd(cmd, &(entry.action),
                                       sizeof (fal_fwd_cmd_t));
            if (SW_OK != rv)
                dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("mirror", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.mirror_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("counter", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.counter_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == entry.counter_en)
    {
        do
        {
            cmd = get_sub_cmd("counter id", "0");
			SW_RTN_ON_NULL_PARAM(cmd);

            if (!strncasecmp(cmd, "quit", 4))
            {
                return SW_BAD_VALUE;
            }
            else if (!strncasecmp(cmd, "help", 4))
            {
                dprintf("usage: integer\n");
                rv = SW_BAD_VALUE;
            }
            else
            {
                rv = cmd_data_check_uint32(cmd, &(entry.counter_id), sizeof (a_uint32_t));
                if (SW_OK != rv)
                    dprintf("usage: integer\n");
            }
        }
        while (talk_mode && (SW_OK != rv));
    }

    *(fal_host_entry_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_host_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_host_entry_t *entry;

    entry = (fal_host_entry_t *) buf;
    dprintf("\n[entryid]:0x%x  [entryflags]:0x%x  [entrystatus]:0x%x",
            entry->entry_id, entry->flags, entry->status);

    if (FAL_IP_IP4_ADDR & entry->flags)
    {
        cmd_data_print_ip4addr("\n[ip_addr]:",
                               (a_uint32_t *) & (entry->ip4_addr),
                               sizeof (fal_ip4_addr_t));
    }
    else
    {
        cmd_data_print_ip6addr("\n[ip_addr]:",
                               (a_uint32_t *) & (entry->ip6_addr),
                               sizeof (fal_ip6_addr_t));
    }

    cmd_data_print_macaddr("  [mac_addr]:",
                           (a_uint32_t *) & (entry->mac_addr),
                           sizeof (fal_mac_addr_t));

    dprintf("\n[interfaceid]:0x%x  [portid]:0x%x  ", entry->intf_id, entry->port_id);
    dprintf("\n[load_balance num]:0x%x  [vrfid]:0x%x  ", entry->lb_num, entry->vrf_id);

    cmd_data_print_maccmd("action", (a_uint32_t *) & (entry->action),
                          sizeof (fal_fwd_cmd_t));

    if (A_TRUE == entry->mirror_en)
    {
        dprintf("\n[mirror]:Enable   ");
    }
    else
    {
        dprintf("\n[mirror]:Disable   ");
    }

    if (A_TRUE == entry->counter_en)
    {
        dprintf("\n[counter]:Enable   [counter_id]:%d    [pkt]%d    [byte]%d",
                entry->counter_id, entry->packet, entry->byte);
    }
    else
    {
        dprintf("\n[couter]:Disable   ");
    }

    if (A_TRUE == entry->pppoe_en)
    {
        dprintf("\n[pppoe]:Enable   [pppoe_id]:%d", entry->pppoe_id);
    }
    else
    {
        dprintf("\n[pppoe]:Disable   ");
    }
}

sw_error_t
cmd_data_check_arp_learn_mode(char *cmd_str, fal_arp_learn_mode_t * arg_val,
                              a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (!strcasecmp(cmd_str, "learnlocal"))
    {
        *arg_val = FAL_ARP_LEARN_LOCAL;
    }
    else if (!strcasecmp(cmd_str, "learnall"))
    {
        *arg_val = FAL_ARP_LEARN_ALL;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_arp_learn_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_ARP_LEARN_LOCAL)
    {
        dprintf("LearnLocal");
    }
    else if (*(a_uint32_t *) buf == FAL_ARP_LEARN_ALL)
    {
        dprintf("LearnAll");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_ip_guard_mode(char *cmd_str, fal_source_guard_mode_t * arg_val, a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (!strcasecmp(cmd_str, "mac_ip"))
    {
        *arg_val = FAL_MAC_IP_GUARD;
    }
    else if (!strcasecmp(cmd_str, "mac_ip_port"))
    {
        *arg_val = FAL_MAC_IP_PORT_GUARD;
    }
    else if (!strcasecmp(cmd_str, "mac_ip_vlan"))
    {
        *arg_val = FAL_MAC_IP_VLAN_GUARD;
    }
    else if (!strcasecmp(cmd_str, "mac_ip_port_vlan"))
    {
        *arg_val = FAL_MAC_IP_PORT_VLAN_GUARD;
    }
    else if (!strcasecmp(cmd_str, "no_guard"))
    {
        *arg_val = FAL_NO_SOURCE_GUARD;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_ip_guard_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_MAC_IP_GUARD)
    {
        dprintf("MAC_IP_GUARD");
    }
    else if (*(a_uint32_t *) buf == FAL_MAC_IP_PORT_GUARD)
    {
        dprintf("MAC_IP_PORT_GUARD");
    }
    else if (*(a_uint32_t *) buf == FAL_MAC_IP_VLAN_GUARD)
    {
        dprintf("MAC_IP_VLAN_GUARD");
    }
    else if (*(a_uint32_t *) buf == FAL_MAC_IP_PORT_VLAN_GUARD)
    {
        dprintf("MAC_IP_PORT_VLAN_GUARD");
    }
    else if (*(a_uint32_t *) buf == FAL_NO_SOURCE_GUARD)
    {
        dprintf("NO_GUARD");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_nat_entry(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    a_uint32_t tmp;
    fal_nat_entry_t entry;

    aos_mem_zero(&entry, sizeof (fal_nat_entry_t));

    do
    {
        cmd = get_sub_cmd("entryid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.entry_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("entryflags", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: bitmap for host entry\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.flags), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: bitmap for host entry\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("entrystatus", "0xf");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.status), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("select_idx", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.slct_idx), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

	do
    {
        cmd = get_sub_cmd("vrf_id", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.vrf_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    cmd_data_check_element("src addr", "0.0.0.0",
                           "usage: the format is xx.xx.xx.xx \n",
                           cmd_data_check_ip4addr, (cmd, &(entry.src_addr), 4));

    cmd_data_check_element("trans addr", "0.0.0.0",
                           "usage: the format is xx.xx.xx.xx \n",
                           cmd_data_check_ip4addr, (cmd, &(entry.trans_addr), 4));

    do
    {
        cmd = get_sub_cmd("port num", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0- 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0- 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.port_num = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("port range", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0- 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0- 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.port_range = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("action", "forward");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_maccmd(cmd, &entry.action,
                                       sizeof (fal_fwd_cmd_t));
            if (SW_OK != rv)
                dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("mirror", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.mirror_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("counter", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.counter_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == entry.counter_en)
    {
        do
        {
            cmd = get_sub_cmd("counter id", "0");
			SW_RTN_ON_NULL_PARAM(cmd);

            if (!strncasecmp(cmd, "quit", 4))
            {
                return SW_BAD_VALUE;
            }
            else if (!strncasecmp(cmd, "help", 4))
            {
                dprintf("usage: integer\n");
                rv = SW_BAD_VALUE;
            }
            else
            {
                rv = cmd_data_check_uint32(cmd, &(entry.counter_id), sizeof (a_uint32_t));
                if (SW_OK != rv)
                    dprintf("usage: integer\n");
            }
        }
        while (talk_mode && (SW_OK != rv));
    }

    *(fal_nat_entry_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_nat_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_nat_entry_t *entry;

    entry = (fal_nat_entry_t *) buf;
    dprintf("\n[entryid]:0x%x  [entryflags]:0x%x  [entrystatus]:0x%x  [select_idx]:0x%x",
            entry->entry_id, entry->flags, entry->status, entry->slct_idx);

	dprintf("\n[vrf_id]:0x%x  ", entry->vrf_id);

    cmd_data_print_ip4addr("\n[src_addr]:",
                           (a_uint32_t *) & (entry->src_addr),
                           sizeof (fal_ip4_addr_t));

    cmd_data_print_ip4addr("\n[trans_addr]:",
                           (a_uint32_t *) & (entry->trans_addr),
                           sizeof (fal_ip4_addr_t));

    dprintf("\n[port_num]:0x%x  [port_range]:0x%x  ", entry->port_num, entry->port_range);

    cmd_data_print_maccmd("action", (a_uint32_t *) & (entry->action),
                          sizeof (fal_fwd_cmd_t));

    if (A_TRUE == entry->mirror_en)
    {
        dprintf("\n[mirror]:Enable   ");
    }
    else
    {
        dprintf("\n[mirror]:Disable   ");
    }

    if (A_TRUE == entry->counter_en)
    {
        dprintf("\n[counter]:Enable   [counter_id]:%d    [in_pkt]%d    [in_byte]%d    [eg_pkt]%d    [eg_byte]%d    ",
                entry->counter_id, entry->ingress_packet, entry->ingress_byte,
                entry->egress_packet, entry->egress_byte);
    }
    else
    {
        dprintf("\n[couter]:Disable   ");
    }
}

sw_error_t
cmd_data_check_napt_entry(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    a_uint32_t tmp;
    fal_napt_entry_t entry;

    aos_mem_zero(&entry, sizeof (fal_napt_entry_t));

    do
    {
        cmd = get_sub_cmd("entryid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.entry_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("entryflags", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: bitmap for host entry\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.flags), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: bitmap for host entry\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("entrystatus", "0xf");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.status), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

	do
    {
        cmd = get_sub_cmd("vrf_id", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.vrf_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

	do
    {
        cmd = get_sub_cmd("flow_cookie", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.flow_cookie), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

	do
    {
        cmd = get_sub_cmd("load_balance", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.load_balance), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    cmd_data_check_element("src addr", "0.0.0.0",
                           "usage: the format is xx.xx.xx.xx \n",
                           cmd_data_check_ip4addr, (cmd, &(entry.src_addr), 4));

    cmd_data_check_element("dst addr", "0.0.0.0",
                           "usage: the format is xx.xx.xx.xx \n",
                           cmd_data_check_ip4addr, (cmd, &(entry.dst_addr), 4));

    if (FAL_NAT_ENTRY_TRANS_IPADDR_INDEX & (entry.flags))
    {
        do
        {
            cmd = get_sub_cmd("trans addr index", "0");
			SW_RTN_ON_NULL_PARAM(cmd);

            if (!strncasecmp(cmd, "quit", 4))
            {
                return SW_BAD_VALUE;
            }
            else if (!strncasecmp(cmd, "help", 4))
            {
                dprintf("usage: integer\n");
                rv = SW_BAD_VALUE;
            }
            else
            {
                rv = cmd_data_check_uint32(cmd, &(entry.trans_addr), sizeof (a_uint32_t));
                if (SW_OK != rv)
                    dprintf("usage: integer\n");
            }
        }
        while (talk_mode && (SW_OK != rv));
    }
    else
    {
        cmd_data_check_element("trans addr", "0.0.0.0",
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd, &(entry.trans_addr), 4));
    }

    do
    {
        cmd = get_sub_cmd("src port", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0- 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0- 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.src_port = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("dst port", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0- 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0- 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.dst_port = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("trans port", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0- 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0- 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.trans_port = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("action", "forward");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_maccmd(cmd, &(entry.action),
                                       sizeof (fal_fwd_cmd_t));
            if (SW_OK != rv)
                dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("mirror", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.mirror_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("counter", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.counter_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == entry.counter_en)
    {
        do
        {
            cmd = get_sub_cmd("counter id", "0");
			SW_RTN_ON_NULL_PARAM(cmd);

            if (!strncasecmp(cmd, "quit", 4))
            {
                return SW_BAD_VALUE;
            }
            else if (!strncasecmp(cmd, "help", 4))
            {
                dprintf("usage: integer\n");
                rv = SW_BAD_VALUE;
            }
            else
            {
                rv = cmd_data_check_uint32(cmd, &(entry.counter_id), sizeof (a_uint32_t));
                if (SW_OK != rv)
                    dprintf("usage: integer\n");
            }
        }
        while (talk_mode && (SW_OK != rv));
    }

	do
    {
        cmd = get_sub_cmd("priority", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.priority_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));


	if (A_TRUE == entry.priority_en)
		{
			do
			{
				cmd = get_sub_cmd("priority value", "0");
				SW_RTN_ON_NULL_PARAM(cmd);

				if (!strncasecmp(cmd, "quit", 4))
				{
					return SW_BAD_VALUE;
				}
				else if (!strncasecmp(cmd, "help", 4))
				{
					dprintf("usage: integer\n");
					rv = SW_BAD_VALUE;
				}
				else
				{
					rv = cmd_data_check_uint32(cmd, &(entry.priority_val), sizeof (a_uint32_t));
					if (SW_OK != rv)
						dprintf("usage: integer\n");
				}
			}
			while (talk_mode && (SW_OK != rv));
		}

    *(fal_napt_entry_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_napt_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_napt_entry_t *entry;

    entry = (fal_napt_entry_t *) buf;
    dprintf("\n[entryid]:0x%x  [entryflags]:0x%x  [entrystatus]:0x%x",
            entry->entry_id, entry->flags, entry->status);

	dprintf("\n[vrf_id]:0x%x  [flow_cookie]:0x%x  [load_balance]:0x%x",
            entry->vrf_id, entry->flow_cookie, entry->load_balance);

    cmd_data_print_ip4addr("\n[src_addr]:",
                           (a_uint32_t *) & (entry->src_addr),
                           sizeof (fal_ip4_addr_t));

    cmd_data_print_ip4addr("\n[dst_addr]:",
                           (a_uint32_t *) & (entry->dst_addr),
                           sizeof (fal_ip4_addr_t));

    if (FAL_NAT_ENTRY_TRANS_IPADDR_INDEX & entry->flags)
    {
        dprintf("\n[trans_addr_index]:0x%x", entry->trans_addr);
    }
    else
    {
        cmd_data_print_ip4addr("\n[trans_addr]:",
                               (a_uint32_t *) & (entry->trans_addr),
                               sizeof (fal_ip4_addr_t));
    }

    dprintf("\n[src_port]:0x%x  [dst_port]:0x%x  [trans_port]:0x%x  ", entry->src_port, entry->dst_port, entry->trans_port);

    cmd_data_print_maccmd("action", (a_uint32_t *) & (entry->action),
                          sizeof (fal_fwd_cmd_t));

    if (A_TRUE == entry->mirror_en)
    {
        dprintf("\n[mirror]:Enable   ");
    }
    else
    {
        dprintf("\n[mirror]:Disable   ");
    }

    if (A_TRUE == entry->counter_en)
    {
        dprintf("\n[counter]:Enable   [counter_id]:%d    [in_pkt]%d    [in_byte]%d    [eg_pkt]%d    [eg_byte]%d    ",
                entry->counter_id, entry->ingress_packet, entry->ingress_byte,
                entry->egress_packet, entry->egress_byte);
    }
    else
    {
        dprintf("\n[couter]:Disable   ");
    }

	if (A_TRUE == entry->priority_en)
    {
        dprintf("\n[priority]:Enable   [priority_val]:%d    ",
                entry->priority_val);
    }
    else
    {
        dprintf("\n[priority]:Disable   ");
    }
}

sw_error_t
cmd_data_check_flow_entry(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    a_uint32_t tmp;
    fal_napt_entry_t entry;

    aos_mem_zero(&entry, sizeof (fal_napt_entry_t));

    do
    {
        cmd = get_sub_cmd("entryid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.entry_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("entryflags", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: bitmap for host entry\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.flags), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: bitmap for host entry\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("entrystatus", "0xf");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.status), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

	do
    {
        cmd = get_sub_cmd("vrf_id", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.vrf_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

	do
    {
        cmd = get_sub_cmd("flow_cookie", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.flow_cookie), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

	do
    {
        cmd = get_sub_cmd("load_balance", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.load_balance), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    cmd_data_check_element("src addr", "0.0.0.0",
                           "usage: the format is xx.xx.xx.xx \n",
                           cmd_data_check_ip4addr, (cmd, &(entry.src_addr), 4));

    cmd_data_check_element("dst addr", "0.0.0.0",
                           "usage: the format is xx.xx.xx.xx \n",
                           cmd_data_check_ip4addr, (cmd, &(entry.dst_addr), 4));

    do
    {
        cmd = get_sub_cmd("src port", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0- 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0- 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.src_port = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("dst port", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0- 65535\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0- 65535\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.dst_port = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("action", "forward");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_maccmd(cmd, &(entry.action),
                                       sizeof (fal_fwd_cmd_t));
            if (SW_OK != rv)
                dprintf("usage: <forward/drop/cpycpu/rdtcpu>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("mirror", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.mirror_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("counter", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.counter_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == entry.counter_en)
    {
        do
        {
            cmd = get_sub_cmd("counter id", "0");
			SW_RTN_ON_NULL_PARAM(cmd);

            if (!strncasecmp(cmd, "quit", 4))
            {
                return SW_BAD_VALUE;
            }
            else if (!strncasecmp(cmd, "help", 4))
            {
                dprintf("usage: integer\n");
                rv = SW_BAD_VALUE;
            }
            else
            {
                rv = cmd_data_check_uint32(cmd, &(entry.counter_id), sizeof (a_uint32_t));
                if (SW_OK != rv)
                    dprintf("usage: integer\n");
            }
        }
        while (talk_mode && (SW_OK != rv));
    }

	do
    {
        cmd = get_sub_cmd("priority", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &entry.priority_en,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));


	if (A_TRUE == entry.priority_en)
		{
			do
			{
				cmd = get_sub_cmd("priority value", "0");
				SW_RTN_ON_NULL_PARAM(cmd);

				if (!strncasecmp(cmd, "quit", 4))
				{
					return SW_BAD_VALUE;
				}
				else if (!strncasecmp(cmd, "help", 4))
				{
					dprintf("usage: integer\n");
					rv = SW_BAD_VALUE;
				}
				else
				{
					rv = cmd_data_check_uint32(cmd, &(entry.priority_val), sizeof (a_uint32_t));
					if (SW_OK != rv)
						dprintf("usage: integer\n");
				}
			}
			while (talk_mode && (SW_OK != rv));
		}

    *(fal_napt_entry_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_flow_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_napt_entry_t *entry;

    entry = (fal_napt_entry_t *) buf;
    dprintf("\n[entryid]:0x%x  [entryflags]:0x%x  [entrystatus]:0x%x",
            entry->entry_id, entry->flags, entry->status);

	dprintf("\n[vrf_id]:0x%x  [flow_cookie]:0x%x  [load_balance]:0x%x",
            entry->vrf_id, entry->flow_cookie, entry->load_balance);

    cmd_data_print_ip4addr("\n[src_addr]:",
                           (a_uint32_t *) & (entry->src_addr),
                           sizeof (fal_ip4_addr_t));

    cmd_data_print_ip4addr("\n[dst_addr]:",
                           (a_uint32_t *) & (entry->dst_addr),
                           sizeof (fal_ip4_addr_t));

    dprintf("\n[src_port]:0x%x  [dst_port]:0x%x  ", entry->src_port, entry->dst_port);

    cmd_data_print_maccmd("action", (a_uint32_t *) & (entry->action),
                          sizeof (fal_fwd_cmd_t));

    if (A_TRUE == entry->mirror_en)
    {
        dprintf("\n[mirror]:Enable   ");
    }
    else
    {
        dprintf("\n[mirror]:Disable   ");
    }

    if (A_TRUE == entry->counter_en)
    {
        dprintf("\n[counter]:Enable   [counter_id]:%d    [in_pkt]%d    [in_byte]%d    [eg_pkt]%d    [eg_byte]%d    ",
                entry->counter_id, entry->ingress_packet, entry->ingress_byte,
                entry->egress_packet, entry->egress_byte);
    }
    else
    {
        dprintf("\n[couter]:Disable   ");
    }

	if (A_TRUE == entry->priority_en)
    {
        dprintf("\n[priority]:Enable   [priority_val]:%d    ",
                entry->priority_val);
    }
    else
    {
        dprintf("\n[priority]:Disable   ");
    }
}

sw_error_t
cmd_data_check_napt_mode(char *cmd_str, fal_napt_mode_t * arg_val,
                         a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (!strcasecmp(cmd_str, "fullcone"))
    {
        *arg_val = FAL_NAPT_FULL_CONE;
    }
    else if (!strcasecmp(cmd_str, "strictcone"))
    {
        *arg_val = FAL_NAPT_STRICT_CONE;
    }
    else if (!strcasecmp(cmd_str, "portstrict"))
    {
        *arg_val = FAL_NAPT_PORT_STRICT;
    }
    else if (!strcasecmp(cmd_str, "synmatric"))
    {
        *arg_val = FAL_NAPT_SYNMETRIC;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_napt_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_NAPT_FULL_CONE)
    {
        dprintf("FullCone");
    }
    else if (*(a_uint32_t *) buf == FAL_NAPT_STRICT_CONE)
    {
        dprintf("StrictCone");
    }
    else if (*(a_uint32_t *) buf == FAL_NAPT_PORT_STRICT)
    {
        dprintf("PortStrict");
    }
    else if (*(a_uint32_t *) buf == FAL_NAPT_SYNMETRIC)
    {
        dprintf("Synmatric");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_intf_mac_entry(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    a_uint32_t tmp;
    sw_error_t rv;
    fal_intf_mac_entry_t entry;

    aos_mem_zero(&entry, sizeof (fal_intf_mac_entry_t));

    do
    {
        cmd = get_sub_cmd("entryid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.entry_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("vrfid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.vrf_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("vid low", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: low vlan id\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: low vlan id\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.vid_low = tmp & 0xffff;

    do
    {
        cmd = get_sub_cmd("vid high", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: high vlan id\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: high vlan id\n");
        }
    }
    while (talk_mode && (SW_OK != rv));
    entry.vid_high = tmp & 0xffff;

    cmd_data_check_element("mac addr", NULL,
                           "usage: the format is xx-xx-xx-xx-xx-xx \n",
                           cmd_data_check_macaddr, (cmd, &(entry.mac_addr),
                                   sizeof (fal_mac_addr_t)));

    do
    {
        cmd = get_sub_cmd("ip4_route", "yes");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.ip4_route,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }

    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("ip6_route", "yes");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &entry.ip6_route,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    *(fal_intf_mac_entry_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_intf_mac_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_intf_mac_entry_t *entry;

    entry = (fal_intf_mac_entry_t *) buf;
    dprintf("\n[entryid]:0x%x  [vrf_id]:0x%x  [vid_low]:0x%x  [vid_high]:0x%x",
            entry->entry_id, entry->vrf_id, entry->vid_low, entry->vid_high);

    cmd_data_print_macaddr("\n[mac_addr]:",
                           (a_uint32_t *) & (entry->mac_addr),
                           sizeof (fal_mac_addr_t));

    if (A_TRUE == entry->ip4_route)
    {
        dprintf("\n[ip4_route]:TRUE");
    }
    else
    {
        dprintf("\n[ip4_route]:FALSE");
    }

    if (A_TRUE == entry->ip6_route)
    {
        dprintf("  [ip6_route]:TRUE");
    }
    else
    {
        dprintf("  [ip6_route]:FALSE");
    }
}

sw_error_t
cmd_data_check_pub_addr_entry(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_nat_pub_addr_t entry;

    aos_mem_zero(&entry, sizeof (fal_nat_pub_addr_t));

    do
    {
        cmd = get_sub_cmd("entryid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.entry_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    cmd_data_check_element("pub addr", NULL,
                           "usage: the format is xx.xx.xx.xx \n",
                           cmd_data_check_ip4addr, (cmd, &(entry.pub_addr), 4));

    *(fal_nat_pub_addr_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_pub_addr_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_nat_pub_addr_t *entry;

    entry = (fal_nat_pub_addr_t *) buf;
    dprintf("[entryid]:0x%x  ", entry->entry_id);
    cmd_data_print_ip4addr("[pub_addr]:",
                           (a_uint32_t *) & (entry->pub_addr),
                           sizeof (fal_ip4_addr_t));

}

sw_error_t
cmd_data_check_egress_shaper(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    a_bool_t bool;
    fal_egress_shaper_t entry;

    aos_mem_zero(&entry, sizeof (fal_egress_shaper_t));

    do
    {
        cmd = get_sub_cmd("bytebased", "yes");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &bool,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == bool)
    {
        entry.meter_unit = FAL_BYTE_BASED;
    }
    else
    {
        entry.meter_unit = FAL_FRAME_BASED;
    }

    do
    {
        cmd = get_sub_cmd("cir", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.cir), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("cbs", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.cbs), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("eir", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.eir), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("ebs", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.ebs), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    *(fal_egress_shaper_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_egress_shaper(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_egress_shaper_t *entry;

    entry = (fal_egress_shaper_t *) buf;

    if (FAL_BYTE_BASED == entry->meter_unit)
    {
        dprintf("\n[byte_based]:yes  ");
    }
    else
    {
        dprintf("\n[byte_based]:no  ");
    }

    dprintf("[cir]:0x%08x  [cbs]:0x%08x  [eir]:0x%08x  [ebs]:0x%08x",
            entry->cir, entry->cbs, entry->eir, entry->ebs);
}

sw_error_t
cmd_data_check_policer_timesslot(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strncasecmp(cmd_str, "100us", 5))
        *arg_val = FAL_RATE_MI_100US;
    else if (!strncasecmp(cmd_str, "1ms", 3))
        *arg_val = FAL_RATE_MI_1MS;
    else if (!strncasecmp(cmd_str, "10ms", 4))
        *arg_val = FAL_RATE_MI_10MS;
    else if (!strncasecmp(cmd_str, "100ms", 5))
        *arg_val = FAL_RATE_MI_100MS;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_policer_timesslot(char * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_RATE_MI_100US)
    {
        dprintf("100us");
    }
    else if (*(a_uint32_t *) buf == FAL_RATE_MI_1MS)
    {
        dprintf("1ms");
    }
    else if (*(a_uint32_t *) buf == FAL_RATE_MI_10MS)
    {
        dprintf("10ms");
    }
    else if (*(a_uint32_t *) buf == FAL_RATE_MI_100MS)
    {
        dprintf("100ms");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}


sw_error_t
cmd_data_check_acl_policer(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    a_bool_t bool;
    fal_acl_policer_t entry;

    aos_mem_zero(&entry, sizeof (fal_acl_policer_t));

    do
    {
        cmd = get_sub_cmd("counter_mode", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.counter_mode),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("bytebased", "yes");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &bool,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == bool)
    {
        entry.meter_unit = FAL_BYTE_BASED;
    }
    else
    {
        entry.meter_unit = FAL_FRAME_BASED;
    }

    if (A_TRUE == entry.counter_mode)
    {
        *(fal_acl_policer_t *)val = entry;
        return SW_OK;
    }

    do
    {
        cmd = get_sub_cmd("couple_flag", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.couple_flag),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("color_aware", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.color_mode),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("deficit_flag", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.deficit_en),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("cir", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.cir), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("cbs", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.cbs), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("eir", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.eir), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("ebs", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.ebs), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("meter_interval", "1ms");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the format <100us/1ms/10ms/100ms>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_policer_timesslot(cmd, &(entry.meter_interval),
                                                  sizeof (fal_rate_mt_t));
            if (SW_OK != rv)
                dprintf("usage: the format <100us/1ms/10ms/100ms>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    *(fal_acl_policer_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_acl_policer(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_acl_policer_t *entry;

    entry = (fal_acl_policer_t *) buf;

    if (A_TRUE == entry->counter_mode)
    {
        dprintf("[counter_mode]:yes  ");
    }
    else
    {
        dprintf("[counter_mode]:no  ");
    }

    if (FAL_BYTE_BASED == entry->meter_unit)
    {
        dprintf("[meter_unit]:byte_based  ");
    }
    else
    {
        dprintf("[meter_unit]:frame_based  ");
    }

    if (A_TRUE == entry->counter_mode)
    {
        dprintf("[counter_lo]:0x%x  [counter_hi]", entry->counter_low, entry->counter_high);
    }
    else
    {
        if (A_TRUE == entry->color_mode)
        {
            dprintf("[color_aware]:yes  ");
        }
        else
        {
            dprintf("[color_aware]:no  ");
        }

        if (A_TRUE == entry->couple_flag)
        {
            dprintf("[couple_falg]:yes  ");
        }
        else
        {
            dprintf("[couple_falg]:no  ");
        }

        if (A_TRUE == entry->deficit_en)
        {
            dprintf("[deficit_falg]:yes  ");
        }
        else
        {
            dprintf("[deficit_falg]:no  ");
        }

        cmd_data_print_policer_timesslot("meter_interval",
                                         (a_uint32_t *) & (entry->meter_interval),
                                         sizeof (fal_rate_mt_t));

        dprintf("\n[cir]:0x%08x  [cbs]:0x%08x  [eir]:0x%08x  [ebs]:0x%08x",
                entry->cir, entry->cbs, entry->eir, entry->ebs);
    }

    return;
}

sw_error_t
cmd_data_check_port_policer(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    a_bool_t bool;
    fal_port_policer_t entry;

    aos_mem_zero(&entry, sizeof (fal_port_policer_t));

    do
    {
        cmd = get_sub_cmd("combine_enable", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.combine_mode),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("bytebased", "yes");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_TRUE, &bool,
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    if (A_TRUE == bool)
    {
        entry.meter_unit = FAL_BYTE_BASED;
    }
    else
    {
        entry.meter_unit = FAL_FRAME_BASED;
    }

    do
    {
        cmd = get_sub_cmd("couple_flag", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.couple_flag),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("color_aware", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.color_mode),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("deficit_flag", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.deficit_en),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("c_enable", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.c_enable),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("cir", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.cir), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("cbs", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.cbs), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("c_rate_flag", "0xfe");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.c_rate_flag), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("c_meter_interval", "1ms");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the format <100us/1ms/10ms/100ms>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_policer_timesslot(cmd, &(entry.c_meter_interval),
                                                  sizeof (fal_rate_mt_t));
            if (SW_OK != rv)
                dprintf("usage: the format <100us/1ms/10ms/100ms>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("e_enable", "no");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <yes/no/y/n>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_confirm(cmd, A_FALSE, &(entry.e_enable),
                                        sizeof (a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <yes/no/y/n>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("eir", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.eir), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("ebs", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.ebs), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("e_rate_flag", "0xfe");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: integer\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.e_rate_flag), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: integer\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("e_meter_interval", "1ms");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the format <100us/1ms/10ms/100ms>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_policer_timesslot(cmd, &(entry.e_meter_interval),
                                                  sizeof (fal_rate_mt_t));
            if (SW_OK != rv)
                dprintf("usage: the format <100us/1ms/10ms/100ms>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    *(fal_port_policer_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_port_policer(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_port_policer_t *entry;

    entry = (fal_port_policer_t *) buf;

    if (A_TRUE == entry->combine_mode)
    {
        dprintf("[combine_mode]:yes  ");
    }
    else
    {
        dprintf("[combine_mode]:no  ");
    }

    if (FAL_BYTE_BASED == entry->meter_unit)
    {
        dprintf("[meter_unit]:byte_based  ");
    }
    else
    {
        dprintf("[meter_unit]:frame_based  ");
    }

    if (A_TRUE == entry->color_mode)
    {
        dprintf("[color_aware]:yes  ");
    }
    else
    {
        dprintf("[color_aware]:no  ");
    }

    if (A_TRUE == entry->couple_flag)
    {
        dprintf("[couple_falg]:yes  ");
    }
    else
    {
        dprintf("[couple_falg]:no  ");
    }

    if (A_TRUE == entry->deficit_en)
    {
        dprintf("[deficit_falg]:yes  ");
    }
    else
    {
        dprintf("[deficit_falg]:no  ");
    }

    if (A_TRUE == entry->c_enable)
    {
        dprintf("\n[c_enable]:yes  ");
    }
    else
    {
        dprintf("\n[c_enable]:no   ");
    }

    dprintf("[cir]:0x%08x  [cbs]:0x%08x  ", entry->cir,entry->cbs);



    dprintf("[c_rate_flag]:0x%08x  ", entry->c_rate_flag);

    cmd_data_print_policer_timesslot("c_meter_interval",
                                     (a_uint32_t *) & (entry->c_meter_interval),
                                     sizeof (fal_rate_mt_t));

    if (A_TRUE == entry->e_enable)
    {
        dprintf("\n[e_enable]:yes  ");
    }
    else
    {
        dprintf("\n[e_enable]:no   ");
    }

    dprintf("[eir]:0x%08x  [ebs]:0x%08x  ", entry->eir, entry->ebs);

    dprintf("[e_rate_flag]:0x%08x  ", entry->e_rate_flag);

    cmd_data_print_policer_timesslot("e_meter_interval",
                                     (a_uint32_t *) & (entry->e_meter_interval),
                                     sizeof (fal_rate_mt_t));
    return;
}

sw_error_t
cmd_data_check_mac_mode(char *cmd_str, fal_interface_mac_mode_t * arg_val,
                        a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (0 == cmd_str[0])
    {
        *arg_val = FAL_MAC_MODE_RGMII;
    }
    else if (!strcasecmp(cmd_str, "rgmii"))
    {
        *arg_val = FAL_MAC_MODE_RGMII;
    }
    else if (!strcasecmp(cmd_str, "rmii"))
    {
        *arg_val = FAL_MAC_MODE_RMII;
    }
    else if (!strcasecmp(cmd_str, "gmii"))
    {
        *arg_val = FAL_MAC_MODE_GMII;
    }
    else if (!strcasecmp(cmd_str, "mii"))
    {
        *arg_val = FAL_MAC_MODE_MII;
    }
    else if (!strcasecmp(cmd_str, "sgmii"))
    {
        *arg_val = FAL_MAC_MODE_SGMII;
    }
    else if (!strcasecmp(cmd_str, "fiber"))
    {
        *arg_val = FAL_MAC_MODE_FIBER;
    }
    else if (!strcasecmp(cmd_str, "default"))
    {
        *arg_val = FAL_MAC_MODE_DEFAULT;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_clock_mode(char *cmd_str, fal_interface_clock_mode_t * arg_val,
                          a_uint32_t size)
{
    if (NULL == cmd_str)
    {
        return SW_BAD_VALUE;
    }

    if (0 == cmd_str[0])
    {
        *arg_val = FAL_INTERFACE_CLOCK_MAC_MODE;
    }
    if (!strcasecmp(cmd_str, "mac"))
    {
        *arg_val = FAL_INTERFACE_CLOCK_MAC_MODE;
    }
    else if (!strcasecmp(cmd_str, "phy"))
    {
        *arg_val = FAL_INTERFACE_CLOCK_PHY_MODE;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}


sw_error_t
cmd_data_check_mac_config(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    fal_mac_config_t entry;

    aos_mem_zero(&entry, sizeof (fal_mac_config_t));

    cmd_data_check_element("mac_mode", "rgmii",
                           "usage: port0 <rgmii/rmii/gmii/mii/sgmii/fiber/default>\nport6 <rgmii/mii/sgmii/fiber/default>\n",
                           cmd_data_check_mac_mode, (cmd, &(entry.mac_mode), 4));

    if (FAL_MAC_MODE_RGMII == entry.mac_mode)
    {
        cmd_data_check_element("txclk_delay_cmd", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.rgmii.txclk_delay_cmd), 4));

        cmd_data_check_element("txclk_delay_select", "0",
                               "usage: <0-3>\n",
                               cmd_data_check_uint32, (cmd, &(entry.config.rgmii.txclk_delay_sel), 4));

        cmd_data_check_element("rxclk_delay_cmd", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.rgmii.rxclk_delay_cmd), 4));

        cmd_data_check_element("rxclk_delay_select", "0",
                               "usage: <0-3>\n",
                               cmd_data_check_uint32, (cmd, &(entry.config.rgmii.rxclk_delay_sel), 4));
    }

    if (FAL_MAC_MODE_RMII == entry.mac_mode)
    {
        cmd_data_check_element("master_mode", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.rmii.master_mode), 4));

        cmd_data_check_element("slave_mode", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.rmii.slave_mode), 4));

        cmd_data_check_element("clock_inverse", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.rmii.clock_inverse), 4));
        cmd_data_check_element("pipe_rxclk_sel", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.rmii.pipe_rxclk_sel), 4));

    }

    if ((FAL_MAC_MODE_GMII == entry.mac_mode)
            || (FAL_MAC_MODE_MII == entry.mac_mode))
    {
        cmd_data_check_element("clock_mode", "mac",
                               "usage: <phy/mac>\n",
                               cmd_data_check_clock_mode, (cmd, &(entry.config.gmii.clock_mode), 4));

        cmd_data_check_element("txclk_select", "0",
                               "usage: <0-1>\n",
                               cmd_data_check_uint32, (cmd, &(entry.config.gmii.txclk_select), 4));

        cmd_data_check_element("rxclk_select", "0",
                               "usage: <0-1>\n",
                               cmd_data_check_uint32, (cmd, &(entry.config.gmii.rxclk_select), 4));
    }

    if (FAL_MAC_MODE_SGMII == entry.mac_mode)
    {
        cmd_data_check_element("clock_mode", "mac",
                               "usage: <phy/mac>\n",
                               cmd_data_check_clock_mode, (cmd, &(entry.config.sgmii.clock_mode), 4));

        cmd_data_check_element("auto_neg", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.sgmii.auto_neg), 4));

        cmd_data_check_element("force_speed", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.sgmii.force_speed), 4));

        cmd_data_check_element("prbs_enable", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.sgmii.prbs_enable), 4));

        cmd_data_check_element("rem_phy_lpbk", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.sgmii.rem_phy_lpbk), 4));
    }

    if (FAL_MAC_MODE_FIBER == entry.mac_mode)
    {
        cmd_data_check_element("auto_neg", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.fiber.auto_neg), 4));

        cmd_data_check_element("fx100_enable", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.config.fiber.fx100_enable), 4));
    }

    *(fal_mac_config_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_mac_config(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_mac_config_t *entry;

    entry = (fal_mac_config_t *) buf;

    if (FAL_MAC_MODE_RGMII == entry->mac_mode)
    {
        dprintf("[mac_mode]:rgmii");
    }
    else if (FAL_MAC_MODE_RMII == entry->mac_mode)
    {
        dprintf("[mac_mode]:rmii");
    }
    else if (FAL_MAC_MODE_GMII == entry->mac_mode)
    {
        dprintf("[mac_mode]:gmii");
    }
    else if (FAL_MAC_MODE_MII == entry->mac_mode)
    {
        dprintf("[mac_mode]:mii");
    }
    else if (FAL_MAC_MODE_SGMII == entry->mac_mode)
    {
        dprintf("[mac_mode]:sgmii");
    }
    else if (FAL_MAC_MODE_FIBER == entry->mac_mode)
    {
        dprintf("[mac_mode]:fiber");
    }
    else
    {
        dprintf("[mac_mode]:default");
    }

    if (FAL_MAC_MODE_RGMII == entry->mac_mode)
    {
        if (A_TRUE == entry->config.rgmii.txclk_delay_cmd)
        {
            dprintf("\n[txclk_delay_cmd]:yes [txclk_delay_select]:%d", entry->config.rgmii.txclk_delay_sel);
        }
        else
        {
            dprintf("\n[txclk_delay_cmd]:no");
        }

        if (A_TRUE == entry->config.rgmii.rxclk_delay_cmd)
        {
            dprintf("\n[rxclk_delay_cmd]:yes [rxclk_delay_select]:%d", entry->config.rgmii.rxclk_delay_sel);
        }
        else
        {
            dprintf("\n[rxclk_delay_cmd]:no");
        }

    }
    else if (FAL_MAC_MODE_RMII == entry->mac_mode)
    {
        if (A_TRUE == entry->config.rmii.master_mode)
        {
            dprintf("\n[master_mode]:yes");
        }
        else
        {
            dprintf("\n[master_mode]:no");
        }

        if (A_TRUE == entry->config.rmii.slave_mode)
        {
            dprintf("\n[slave_mode]:yes");
        }
        else
        {
            dprintf("\n[slave_mode]:no");
        }

        if (A_TRUE == entry->config.rmii.clock_inverse)
        {
            dprintf("\n[clock_inverse]:yes");
        }
        else
        {
            dprintf("\n[clock_inverse]:no");
        }

        if (A_TRUE == entry->config.rmii.pipe_rxclk_sel)
        {
            dprintf("\n[pipe_rxclk_sel]:yes");
        }
        else
        {
            dprintf("\n[pipe_rxclk_sel]:no");
        }


    }
    else if ((FAL_MAC_MODE_GMII == entry->mac_mode)
             || (FAL_MAC_MODE_MII == entry->mac_mode))
    {

        if (FAL_INTERFACE_CLOCK_PHY_MODE == entry->config.gmii.clock_mode)
        {
            dprintf("\n[clock_mode]:phy [txclk_select]:%d [rxclk_select]:%d", entry->config.gmii.txclk_select, entry->config.gmii.rxclk_select);
        }
        else
        {
            dprintf("\n[clock_mode]:mac [txclk_select]:%d [rxclk_select]:%d", entry->config.gmii.txclk_select, entry->config.gmii.rxclk_select);
        }
    }
    else if (FAL_MAC_MODE_SGMII == entry->mac_mode)
    {
        if (FAL_INTERFACE_CLOCK_PHY_MODE == entry->config.sgmii.clock_mode)
        {
            dprintf("\n[clock_mode]:phy");
        }
        else
        {
            dprintf("\n[clock_mode]:mac");
        }

        if (A_TRUE == entry->config.sgmii.auto_neg)
        {
            dprintf("\n[auto_neg]:yes");
        }
        else
        {
            dprintf("\n[auto_neg]:no");
        }
        if (A_TRUE == entry->config.sgmii.force_speed)
        {
            dprintf("\n[force_speed]:yes");
        }
        else
        {
            dprintf("\n[force_speed]:no");
        }
        if (A_TRUE == entry->config.sgmii.prbs_enable)
        {
            dprintf("\n[prbs_enable]:yes");
        }
        else
        {
            dprintf("\n[prbs_enable]:no");
        }
        if (A_TRUE == entry->config.sgmii.rem_phy_lpbk)
        {
            dprintf("\n[rem_phy_lpbk]:yes");
        }
        else
        {
            dprintf("\n[rem_phy_lpbk]:no");
        }
    }
    else if (FAL_MAC_MODE_FIBER == entry->mac_mode)
    {
        if (A_TRUE == entry->config.fiber.auto_neg)
        {
            dprintf("\n[auto_neg]:yes");
        }
        else
        {
            dprintf("\n[auto_neg]:no");
        }
        if (A_TRUE == entry->config.fiber.fx100_enable)
        {
            dprintf("\n[fx100_enable]:yes");
        }
        else
        {
            dprintf("\n[fx100_enable]:no");
        }
    }

    return;
}

sw_error_t
cmd_data_check_phy_config(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    fal_phy_config_t entry;

    aos_mem_zero(&entry, sizeof (fal_phy_config_t));

    cmd_data_check_element("mac_mode", "rgmii",
                           "usage: <rgmii/default>\n",
                           cmd_data_check_mac_mode, (cmd, &(entry.mac_mode), 4));

    if (FAL_MAC_MODE_RGMII == entry.mac_mode)
    {

        cmd_data_check_element("txclk_delay_cmd", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.txclk_delay_cmd), 4));

        cmd_data_check_element("txclk_delay_select", "0",
                               "usage: <0-3>\n",
                               cmd_data_check_uint32, (cmd, &(entry.txclk_delay_sel), 4));

        cmd_data_check_element("rxclk_delay_cmd", "no",
                               "usage: <yes/no/y/n>\n",
                               cmd_data_check_confirm, (cmd, A_FALSE, &(entry.rxclk_delay_cmd), 4));

        cmd_data_check_element("rxclk_delay_select", "0",
                               "usage: <0-3>\n",
                               cmd_data_check_uint32, (cmd, &(entry.rxclk_delay_sel), 4));
    }
    else
    {
        return SW_BAD_VALUE;
    }

    *(fal_phy_config_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_phy_config(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_phy_config_t *entry;

    entry = (fal_phy_config_t *) buf;

    if (FAL_MAC_MODE_RGMII == entry->mac_mode)
    {
        dprintf("[mac_mode]:rgmii");
    }
    else
    {
        dprintf("[mac_mode]:default");
    }

    if (FAL_MAC_MODE_RGMII == entry->mac_mode)
    {
        if (A_TRUE == entry->txclk_delay_cmd)
        {
            dprintf("\n[txclk_delay_cmd]:yes [txclk_delay_select]:%d", entry->txclk_delay_sel);
        }
        else
        {
            dprintf("\n[txclk_delay_cmd]:no");
        }

        if (A_TRUE == entry->rxclk_delay_cmd)
        {
            dprintf("\n[rxclk_delay_cmd]:yes [rxclk_delay_select]:%d", entry->rxclk_delay_sel);
        }
        else
        {
            dprintf("\n[rxclk_delay_cmd]:no");
        }
    }
    return;
}

sw_error_t
cmd_data_check_fdb_smode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "ivl"))
        *arg_val = INVALID_VLAN_IVL;
    else if (!strcasecmp(cmd_str, "svl"))
        *arg_val = INVALID_VLAN_SVL;
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_fdb_smode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == 1)
    {
        dprintf("IVL");
    }
    else if (*(a_uint32_t *) buf == 0)
    {
        dprintf("SVL");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_fx100_link_mode(char* cmd_str, fx100_ctrl_link_mode_t* arg_val)
{
    if (0 == cmd_str[0])
    {
        *arg_val = Fx100BASE_MODE;
    }
    else if (!strcasecmp(cmd_str, "fx100base"))
    {
        *arg_val = Fx100BASE_MODE;
    }
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_fx100_fd_mode(char *cmd_str, a_uint32_t * arg_val)
{
    if (0 == cmd_str[0])
    {
        *arg_val = FX100_FULL_DUPLEX;
    }
    else if (!strcasecmp(cmd_str, "fullduplex"))
    {
        *arg_val = FX100_FULL_DUPLEX;
    }
    else if (!strcasecmp(cmd_str, "halfduplex"))
    {
        *arg_val = FX100_HALF_DUPLEX;
    }
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_sgmii_fiber_mode(char *cmd_str, a_uint32_t * arg_val)
{
    if (0 == cmd_str[0])
    {
        *arg_val = FX100_SERDS_MODE;
    }
    else if (!strcasecmp(cmd_str, "fx100serds"))
    {
        *arg_val = FX100_SERDS_MODE;
    }
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }
    return SW_OK;
}



sw_error_t
cmd_data_check_fx100_config(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    fal_fx100_ctrl_config_t entry;

    aos_mem_zero(&entry, sizeof (fal_fx100_ctrl_config_t));

    cmd_data_check_element("link_mode", "fx100base",
                           "usage: <fx100base>\n",
                           cmd_data_check_fx100_link_mode, (cmd, &(entry.link_mode)));

    cmd_data_check_element("overshoot", "no",
                           "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &(entry.overshoot), 4));

    cmd_data_check_element("loopback_mode", "no",
                           "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &(entry.loopback), 4));

    cmd_data_check_element("fd_mode", "fullduplex",
                           "usage: <fullduplex/halfduplex>\n",
                           cmd_data_check_fx100_fd_mode, (cmd, &(entry.fd_mode)));

    cmd_data_check_element("col_test", "no",
                           "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &(entry.col_test), 4));

    cmd_data_check_element("sgmii_fiber", "fx100serds",
                           "usage: <fx100serds>\n",
                           cmd_data_check_sgmii_fiber_mode, (cmd, &(entry.sgmii_fiber_mode)));

    cmd_data_check_element("crs_ctrl", "yes",
                           "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_TRUE, &(entry.crs_ctrl), 4));

    cmd_data_check_element("loopback_ctrl", "no",
                           "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &(entry.loopback_ctrl), 4));

    cmd_data_check_element("crs_col_100_ctrl", "yes",
                           "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_TRUE, &(entry.crs_col_100_ctrl), 4));

    cmd_data_check_element("loop_en", "no",
                           "usage: <yes/no/y/n>\n",
                           cmd_data_check_confirm, (cmd, A_FALSE, &(entry.loop_en), 4));



    *(fal_fx100_ctrl_config_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_fx100_config(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_fx100_ctrl_config_t* entry;

    entry = (fal_fx100_ctrl_config_t*)buf;

    if (Fx100BASE_MODE == entry->link_mode)
    {
        dprintf("[link_mode]: fx100base\n");
    }

    if (A_TRUE == entry->overshoot)
    {
        dprintf("[overshoot]: yes\n");
    }
    else
    {
        dprintf("[overshoot]: no\n");
    }

    if (A_TRUE == entry->loopback)
    {
        dprintf("[loopback_mode]: yes\n");
    }
    else
    {
        dprintf("[loopback_mode]: no\n");
    }

    if (FX100_FULL_DUPLEX == entry->fd_mode)
    {
        dprintf("[fd_mode]: fullduplex\n");
    }
    else
    {
        dprintf("[fd_mode]: halfduplex\n");
    }

    if (A_TRUE == entry->col_test)
    {
        dprintf("[col_test]: yes\n");
    }
    else
    {
        dprintf("[col_test]: no\n");
    }

    if (FX100_SERDS_MODE == entry->sgmii_fiber_mode)
    {
        dprintf("[sgmii_fiber]: fx100_serds\n");
    }

    if (A_TRUE == entry->crs_ctrl)
    {
        dprintf("[crs_ctrl]: yes\n");
    }
    else
    {
        dprintf("[crs_ctrl]: no\n");
    }

    if (A_TRUE == entry->loopback_ctrl)
    {
        dprintf("[loopback_ctrl]: yes\n");
    }
    else
    {
        dprintf("[loopback_ctrl]: no\n");
    }

    if (A_TRUE == entry->crs_col_100_ctrl)
    {
        dprintf("[crs_col_100_ctrl]: yes\n");
    }
    else
    {
        dprintf("[crs_col_100_ctrl]: no\n");
    }

    if (A_TRUE == entry->loop_en)
    {
        dprintf("[loop_en]: yes\n");
    }
    else
    {
        dprintf("[loop_en]: no\n");
    }

}

sw_error_t
cmd_data_check_sec_mac(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "resv_vid"))
        *arg_val = FAL_NORM_MAC_RESV_VID_CMD;
    else if (!strcasecmp(cmd_str, "invalid_src_addr"))
        *arg_val = FAL_NORM_MAC_INVALID_SRC_ADDR_CMD;
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_sec_ip(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "invalid_ver"))
        *arg_val = FAL_NORM_IP_INVALID_VER_CMD;
    else if (!strcasecmp(cmd_str, "same_addr"))
        *arg_val = FAL_NROM_IP_SAME_ADDR_CMD;
    else if (!strcasecmp(cmd_str, "ttl_change_status"))
        *arg_val = FAL_NROM_IP_TTL_CHANGE_STATUS;
    else if (!strcasecmp(cmd_str, "ttl_val"))
        *arg_val = FAL_NROM_IP_TTL_VALUE;
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}


sw_error_t
cmd_data_check_sec_ip4(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "invalid_hl"))
        *arg_val = FAL_NROM_IP4_INVALID_HL_CMD;
    else if (!strcasecmp(cmd_str, "hdr_opts"))
        *arg_val = FAL_NROM_IP4_HDR_OPTIONS_CMD;
    else if (!strcasecmp(cmd_str, "invalid_df"))
        *arg_val = FAL_NROM_IP4_INVALID_DF_CMD;
    else if (!strcasecmp(cmd_str, "frag_offset_min_len"))
        *arg_val = FAL_NROM_IP4_FRAG_OFFSET_MIN_LEN_CMD;
    else if (!strcasecmp(cmd_str, "frag_offset_min_size"))
        *arg_val = FAL_NROM_IP4_FRAG_OFFSET_MIN_SIZE;
    else if (!strcasecmp(cmd_str, "frag_offset_max_len"))
        *arg_val = FAL_NROM_IP4_FRAG_OFFSET_MAX_LEN_CMD;
    else if (!strcasecmp(cmd_str, "invalid_frag_offset"))
        *arg_val = FAL_NROM_IP4_INVALID_FRAG_OFFSET_CMD;
    else if (!strcasecmp(cmd_str, "invalid_sip"))
        *arg_val = FAL_NROM_IP4_INVALID_SIP_CMD;
    else if (!strcasecmp(cmd_str, "invalid_dip"))
        *arg_val = FAL_NROM_IP4_INVALID_DIP_CMD;
    else if (!strcasecmp(cmd_str, "invalid_chksum"))
        *arg_val = FAL_NROM_IP4_INVALID_CHKSUM_CMD;
    else if (!strcasecmp(cmd_str, "invalid_pl"))
        *arg_val = FAL_NROM_IP4_INVALID_PL_CMD;
    else if (!strcasecmp(cmd_str, "df_clear_status"))
        *arg_val = FAL_NROM_IP4_DF_CLEAR_STATUS;
    else if (!strcasecmp(cmd_str, "ipid_random_status"))
        *arg_val = FAL_NROM_IP4_IPID_RANDOM_STATUS;
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_sec_ip6(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "invalid_dip"))
        *arg_val = FAL_NROM_IP6_INVALID_DIP_CMD;
    else if (!strcasecmp(cmd_str, "invalid_sip"))
        *arg_val = FAL_NROM_IP6_INVALID_SIP_CMD;
    else if (!strcasecmp(cmd_str, "invalid_pl"))
        *arg_val = FAL_NROM_IP6_INVALID_PL_CMD;
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}


sw_error_t
cmd_data_check_sec_tcp(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "blat"))
        *arg_val = FAL_NROM_TCP_BLAT_CMD;
    else if (!strcasecmp(cmd_str, "invalid_hl"))
        *arg_val = FAL_NROM_TCP_INVALID_HL_CMD;
    else if (!strcasecmp(cmd_str, "min_hdr_size"))
        *arg_val = FAL_NROM_TCP_MIN_HDR_SIZE;
    else if (!strcasecmp(cmd_str, "invalid_syn"))
        *arg_val = FAL_NROM_TCP_INVALID_SYN_CMD;
    else if (!strcasecmp(cmd_str, "su_block"))
        *arg_val = FAL_NROM_TCP_SU_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "sp_block"))
        *arg_val = FAL_NROM_TCP_SP_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "sap_block"))
        *arg_val = FAL_NROM_TCP_SAP_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "xmas_scan"))
        *arg_val = FAL_NROM_TCP_XMAS_SCAN_CMD;
    else if (!strcasecmp(cmd_str, "null_scan"))
        *arg_val = FAL_NROM_TCP_NULL_SCAN_CMD;
    else if (!strcasecmp(cmd_str, "sr_block"))
        *arg_val = FAL_NROM_TCP_SR_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "sf_block"))
        *arg_val = FAL_NROM_TCP_SF_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "sar_block"))
        *arg_val = FAL_NROM_TCP_SAR_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "rst_scan"))
        *arg_val = FAL_NROM_TCP_RST_SCAN_CMD;
    else if (!strcasecmp(cmd_str, "rst_with_data"))
        *arg_val = FAL_NROM_TCP_RST_WITH_DATA_CMD;
    else if (!strcasecmp(cmd_str, "fa_block"))
        *arg_val = FAL_NROM_TCP_FA_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "pa_block"))
        *arg_val = FAL_NROM_TCP_PA_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "ua_block"))
        *arg_val = FAL_NROM_TCP_UA_BLOCK_CMD;
    else if (!strcasecmp(cmd_str, "invalid_chksum"))
        *arg_val = FAL_NROM_TCP_INVALID_CHKSUM_CMD;
    else if (!strcasecmp(cmd_str, "invalid_urgptr"))
        *arg_val = FAL_NROM_TCP_INVALID_URGPTR_CMD;
    else if (!strcasecmp(cmd_str, "invalid_opts"))
        *arg_val = FAL_NROM_TCP_INVALID_OPTIONS_CMD;
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}


sw_error_t
cmd_data_check_sec_udp(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "blat"))
        *arg_val = FAL_NROM_UDP_BLAT_CMD;
    else if (!strcasecmp(cmd_str, "invalid_len"))
        *arg_val = FAL_NROM_UDP_INVALID_LEN_CMD;
    else if (!strcasecmp(cmd_str, "invalid_chksum"))
        *arg_val = FAL_NROM_UDP_INVALID_CHKSUM_CMD;
    else
    {
        dprintf("UNKNOWN VALUE");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_sec_icmp4(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "ping_pl_exceed"))
        *arg_val = FAL_NROM_ICMP4_PING_PL_EXCEED_CMD;
    else if (!strcasecmp(cmd_str, "ping_frag"))
        *arg_val = FAL_NROM_ICMP4_PING_FRAG_CMD;
    else if (!strcasecmp(cmd_str, "ping_max_pl"))
        *arg_val = FAL_NROM_ICMP4_PING_MAX_PL_VALUE;
    else
    {
        //dprintf("input error");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_sec_icmp6(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "ping_pl_exceed"))
        *arg_val = FAL_NROM_ICMP6_PING_PL_EXCEED_CMD;
    else if (!strcasecmp(cmd_str, "ping_frag"))
        *arg_val = FAL_NROM_ICMP6_PING_FRAG_CMD;
    else if (!strcasecmp(cmd_str, "ping_max_pl"))
        *arg_val = FAL_NROM_ICMP6_PING_MAX_PL_VALUE;
    else
    {
        //dprintf("input error");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

sw_error_t
cmd_data_check_remark_entry(char *info, void *val, a_uint32_t size)
{
    char *cmd;
    sw_error_t rv;
    fal_egress_remark_table_t *pEntry = (fal_egress_remark_table_t *)val;
    a_uint32_t tmp;

    memset(pEntry, 0, sizeof(fal_egress_remark_table_t));

    /* get remark_dscp */
    do
    {
        cmd = get_sub_cmd("remark dscp", "enable");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <enable/disable>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_enable(cmd, &(pEntry->remark_dscp), sizeof(a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <enable/disable>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    /* get remark_up */
    do
    {
        cmd = get_sub_cmd("remark up", "enable");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <enable/disable>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_enable(cmd, &(pEntry->remark_up), sizeof(a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <enable/disable>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    /* get remark_dei */
    do
    {
        cmd = get_sub_cmd("remark dei", "enable");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: <enable/disable>\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_enable(cmd, &(pEntry->remark_dei), sizeof(a_bool_t));
            if (SW_OK != rv)
                dprintf("usage: <enable/disable>\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    /* get g_dscp */
    do
    {
        cmd = get_sub_cmd("green dscp", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 63\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
            {
                dprintf("usage: the range is 0 -- 63\n");
            }

            if (tmp > 63)
            {
                dprintf("usage: the range is 0 -- 63\n");
                rv = SW_OUT_OF_RANGE;
            }
        }
    }
    while (talk_mode && (SW_OK != rv));
    pEntry->g_dscp = tmp;

    /* get y_dscp */
    do
    {
        cmd = get_sub_cmd("yellow dscp", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 63\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
            {
                dprintf("usage: the range is 0 -- 63\n");
            }

            if (tmp > 63)
            {
                dprintf("usage: the range is 0 -- 63\n");
                rv = SW_OUT_OF_RANGE;
            }
        }
    }
    while (talk_mode && (SW_OK != rv));
    pEntry->y_dscp = tmp;

    /* get g_up */
    do
    {
        cmd = get_sub_cmd("green up", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 63\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
            {
                dprintf("usage: the range is 0 -- 7\n");
            }

            if (tmp > 63)
            {
                dprintf("usage: the range is 0 -- 7\n");
                rv = SW_OUT_OF_RANGE;
            }
        }
    }
    while (talk_mode && (SW_OK != rv));
    pEntry->g_up = tmp;

    /* get y_up */
    do
    {
        cmd = get_sub_cmd("yellow up", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 63\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
            {
                dprintf("usage: the range is 0 -- 7\n");
            }

            if (tmp > 63)
            {
                dprintf("usage: the range is 0 -- 7\n");
                rv = SW_OUT_OF_RANGE;
            }
        }
    }
    while (talk_mode && (SW_OK != rv));
    pEntry->y_up = tmp;

    /* get g_dei */
    do
    {
        cmd = get_sub_cmd("green dei", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 1\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
            {
                dprintf("usage: the range is 0 -- 1\n");
            }

            if (tmp > 1)
            {
                dprintf("usage: the range is 0 -- 1\n");
                rv = SW_OUT_OF_RANGE;
            }
        }
    }
    while (talk_mode && (SW_OK != rv));
    pEntry->g_dei = tmp;

    /* get y_dei */
    do
    {
        cmd = get_sub_cmd("yellow dei", NULL);
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: the range is 0 -- 1\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &tmp, sizeof (a_uint32_t));
            if (SW_OK != rv)
            {
                dprintf("usage: the range is 0 -- 1\n");
            }

            if (tmp > 1)
            {
                dprintf("usage: the range is 0 -- 1\n");
                rv = SW_OUT_OF_RANGE;
            }
        }
    }
    while (talk_mode && (SW_OK != rv));
    pEntry->y_dei = tmp;


/*
    dprintf("remark_dscp=%d, remark_up=%d, g_dscp=%d, y_dscp=%d\n",
            pEntry->remark_dscp,
            pEntry->remark_up,
            pEntry->g_dscp,
            pEntry->y_dscp);

    *(fal_egress_remark_table_t *) val = entry;
*/
    return SW_OK;
}

void
cmd_data_print_remark_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_egress_remark_table_t *entry;

    entry = (fal_egress_remark_table_t *) buf;
    dprintf("\n");
    dprintf("[remark dscp]:%s\n", entry->remark_dscp?"enabled":"disabled");
    dprintf("[remark up]:%s\n", entry->remark_up?"enabled":"disabled");
    dprintf("[remark dei]:%s\n", entry->remark_dei?"enabled":"disabled");
    dprintf("[green dscp]:%d\n", entry->g_dscp);
    dprintf("[yellow dscp]:%d\n", entry->y_dscp);
    dprintf("[green up]:%d\n", entry->g_up);
    dprintf("[yellow up]:%d\n", entry->y_up);
    dprintf("[green dei]:%d\n", entry->g_dei);
    dprintf("[yellow dei]:%d\n", entry->y_dei);

    return;
}

sw_error_t
cmd_data_check_default_route_entry(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    a_uint32_t tmp;
    sw_error_t rv;
    fal_default_route_t entry;

    aos_mem_zero(&entry, sizeof (fal_intf_mac_entry_t));

    do
    {
        cmd = get_sub_cmd("entry valid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0 for invalid and 1 for valid \n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.valid), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0 for invalid and 1 for valid \n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("vrf id", "0");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: VRF id\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &(entry.vrf_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: VRF id\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("ip version", "0");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0 for ipv4 and 1 for ipv6 \n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &(entry.ip_version), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0 for ipv4 and 1 for ipv6 \n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("route type", "0");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0 for arp and 1 for wcmp \n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &(entry.droute_type), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0 for arp and 1 for wcmp \n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("index", "0");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {

            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: index for arp entry or wcmp entry \n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &(entry.index), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: index for arp entry or wcmp entry \n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    *(fal_default_route_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_default_route_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_default_route_t *entry;

    entry = (fal_default_route_t *) buf;
    dprintf("\n[valid]:0x%x  [vrf_id]:0x%x  [ip_version]:0x%x  [host_type]:0x%x  [index]:0x%x \n",
            entry->valid, entry->vrf_id, entry->ip_version, entry->droute_type, entry->index);
}

sw_error_t
cmd_data_check_host_route_entry(char *cmd_str, void * val, a_uint32_t size)
{
    char *cmd;
    a_uint32_t tmp;
    sw_error_t rv;
    fal_host_route_t entry;

    aos_mem_zero(&entry, sizeof (fal_intf_mac_entry_t));

    do
    {
        cmd = get_sub_cmd("entry valid", "0");
		SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0 for invalid and 1 for valid \n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint32(cmd, &(entry.valid), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0 for invalid and 1 for valid \n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("vrf id", "0");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: VRF id\n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &(entry.vrf_id), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: VRF id\n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    do
    {
        cmd = get_sub_cmd("ip version", "0");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: 0 for ipv4 and 1 for ipv6 \n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &(entry.ip_version), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: 0 for ipv4 and 1 for ipv6 \n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    if (entry.ip_version == 0) /*IPv4*/
    {
        cmd_data_check_element("ip4 addr", NULL,
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd, &(entry.route_addr.ip4_addr), 4));
    }
    else if (entry.ip_version == 1) /*IPv6*/
    {
        cmd_data_check_element("ip6 addr", NULL,
                               "usage: the format is xx.xx.xx.xx \n",
                               cmd_data_check_ip4addr, (cmd, &(entry.route_addr.ip6_addr), 16));
    }
    else
    {
        return SW_BAD_VALUE;
    }

    do
    {
        cmd = get_sub_cmd("prefix_length", "0");
        SW_RTN_ON_NULL_PARAM(cmd);

        if (!strncasecmp(cmd, "quit", 4))
        {
            return SW_BAD_VALUE;
        }
        else if (!strncasecmp(cmd, "help", 4))
        {
            dprintf("usage: prefix length for this host route, 0~31 for ipv4 and 0~127 for ipv6 \n");
            rv = SW_BAD_VALUE;
        }
        else
        {
            rv = cmd_data_check_uint16(cmd, &(entry.prefix_length), sizeof (a_uint32_t));
            if (SW_OK != rv)
                dprintf("usage: prefix length for this host route, 0~31 for ipv4 and 0~127 for ipv6 \n");
        }
    }
    while (talk_mode && (SW_OK != rv));

    *(fal_host_route_t *)val = entry;
    return SW_OK;
}

void
cmd_data_print_host_route_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    fal_host_route_t *entry;

    entry = (fal_host_route_t *) buf;
    dprintf("\n[valid]:0x%x  [vrf_id]:0x%x  [prefix_length]:0x%x",
            entry->valid, entry->vrf_id, entry->prefix_length);

    if (0 == entry->ip_version)
    {
        cmd_data_print_ip4addr("\n[ip_addr]:",
                               (a_uint32_t *) & (entry->route_addr.ip4_addr),
                               sizeof (fal_ip4_addr_t));
    }
    else if (1 == entry->ip_version)
    {
        cmd_data_print_ip6addr("\n[ip_addr]:",
                               (a_uint32_t *) & (entry->route_addr.ip6_addr),
                               sizeof (fal_ip6_addr_t));
    }
    else
    {
        return SW_BAD_VALUE;
    }
}

sw_error_t
cmd_data_check_array(char *cmdstr, void *val, a_uint32_t size)
{
    char *tmp = NULL;
    a_uint32_t i = 0, j;
    a_uint32_t addr;
    int *dst = (int*)val;

    if (NULL == cmdstr)
    {
        return SW_BAD_VALUE; /*was: SW_OK;*/
    }

    if (0 == cmdstr[0])
    {
        return SW_OK;
    }

    tmp = (void *) strtok(cmdstr, "-");
    while (tmp)
    {
        if (size <= i)
        {
            return SW_BAD_VALUE;
        }

        if ((2 < strlen(tmp)) || (0 == strlen(tmp)))
        {
            return SW_BAD_VALUE;
        }

        for (j = 0; j < strlen(tmp); j++)
        {
            if (A_FALSE == is_hex(tmp[j]))
                return SW_BAD_VALUE;
        }

        sscanf(tmp, "%x", &addr);
        if (0xff < addr)
        {
            return SW_BAD_VALUE;
        }

        dst[i++] = addr;
        tmp = (void *) strtok(NULL, "-");
    }

    if (size != i)
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}


sw_error_t
cmd_data_check_ip_wcmp_entry(char *cmd_str, void * val, a_uint32_t size)
{
	
	char *cmd;
	a_uint32_t tmp = 0;
	sw_error_t rv;
	fal_ip_wcmp_t entry;
	char buf[12] ={0};

	aos_mem_zero(&entry, sizeof (fal_ip_wcmp_t));
	
	do
	{
		cmd = get_sub_cmd("nh_nr", "16");
		SW_RTN_ON_NULL_PARAM(cmd);

		if (!strncasecmp(cmd, "quit", 4))
		{
			return SW_BAD_VALUE;
		}
		else if (!strncasecmp(cmd, "help", 4))
		{
			dprintf("usage: integer\n");
			rv = SW_BAD_VALUE;
		}
		else
		{
			rv = cmd_data_check_uint32(cmd, &(entry.nh_nr), sizeof (a_uint32_t));
			if (SW_OK != rv)
				dprintf("usage: integer\n");
			else {
				if(entry.nh_nr > 16) {
					dprintf("usage: integer <= 16\n");
					rv = SW_BAD_VALUE;
				}
			}
		}

	}
	while (talk_mode && (SW_OK != rv));

	do
	{
		cmd = get_sub_cmd("nh_id", NULL);
        	SW_RTN_ON_NULL_PARAM(cmd);

        	if (!strncasecmp(cmd, "quit", 4))
        	{
            		return SW_BAD_VALUE;
        	}
        	else if (!strncasecmp(cmd, "help", 4))
        	{
            		dprintf("usage: the format is xx-xx-xx-xx-xx-xx \n");
            		rv = SW_BAD_VALUE;
        	}
        	else
        	{
            		rv = cmd_data_check_array(cmd, entry.nh_id, entry.nh_nr);
            		if (SW_OK != rv)
                		dprintf("usage: the format is xx-xx-xx-xx-xx-xx \n");
        	}
	}
	while (talk_mode && (SW_OK != rv));

	*(fal_ip_wcmp_t *)val = entry;
	return SW_OK;
}


void
cmd_data_print_ip_wcmp_entry(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
	fal_ip_wcmp_t *entry;
	int i = 0;

    entry = (fal_ip_wcmp_t *) buf;
	dprintf("\n[nh_nr]:0x%x",
            entry->nh_nr);
	dprintf("\n");
	for(i = 0; i < entry->nh_nr; i++) {
		dprintf("[nh_id[%d]]:0x%x ", i, entry->nh_id[i]);
		if(((i+1) % 4) == 0)
			dprintf("\n");
	}
}

sw_error_t
cmd_data_check_ip4_rfs_entry(char *cmd_str, void * val, a_uint32_t size)
{
	char *cmd;
	a_uint32_t tmp;
	sw_error_t rv;
	fal_ip4_rfs_t entry;

	aos_mem_zero(&entry, sizeof (fal_ip4_rfs_t));

	rv = __cmd_data_check_complex("mac addr", NULL,
                        "usage: the format is xx-xx-xx-xx-xx-xx \n",
                        cmd_data_check_macaddr, &(entry.mac_addr),
                        sizeof (fal_mac_addr_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("ip4 addr", NULL,
                            "usage: the format is xx.xx.xx.xx \n",
                            cmd_data_check_ip4addr, &(entry.ip4_addr),
                            4);
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("vid", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.vid),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("loadbalance", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &tmp,
                        sizeof (a_uint32_t));
    if (rv)
        return rv;
	entry.load_balance = tmp;
	*(fal_ip4_rfs_t *)val = entry;
	return SW_OK;
}

sw_error_t
cmd_data_check_fdb_rfs(char *cmd_str, void * val, a_uint32_t size)
{
	char *cmd;
	a_uint32_t tmp;
	sw_error_t rv;
	fal_fdb_rfs_t entry;

	aos_mem_zero(&entry, sizeof (fal_fdb_rfs_t));

	rv = __cmd_data_check_complex("mac addr", NULL,
                        "usage: the format is xx-xx-xx-xx-xx-xx \n",
                        cmd_data_check_macaddr, &(entry.addr),
                        sizeof (fal_mac_addr_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("fid", NULL,
                            "usage: the format is xx\n",
                            cmd_data_check_uint32, &tmp,
                            sizeof (a_uint32_t));
    if (rv)
        return rv;
	entry.fid = tmp;

	rv = __cmd_data_check_complex("loadbalance", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &tmp,
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	entry.load_balance = tmp;

	*(fal_fdb_rfs_t *)val = entry;
	return SW_OK;
}


sw_error_t
cmd_data_check_flow_cookie(char *cmd_str, void * val, a_uint32_t size)
{
	char *cmd;
	a_uint32_t tmp;
	sw_error_t rv;
	fal_flow_cookie_t entry;

	aos_mem_zero(&entry, sizeof (fal_flow_cookie_t));

	rv = __cmd_data_check_complex("proto", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.proto),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("src addr", NULL,
                            "usage: the format is xx.xx.xx.xx \n",
                            cmd_data_check_ip4addr, &(entry.src_addr),
                            4);
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("dst addr", NULL,
                            "usage: the format is xx.xx.xx.xx \n",
                            cmd_data_check_ip4addr, &(entry.dst_addr),
                            4);
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("src port", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.src_port),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("dst port", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.dst_port),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("flow cookie", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.flow_cookie),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;


	*(fal_flow_cookie_t *)val = entry;
	return SW_OK;
}

sw_error_t
cmd_data_check_flow_rfs(char *cmd_str, void * val, a_uint32_t size)
{
	char *cmd;
	a_uint32_t tmp;
	sw_error_t rv;
	fal_flow_rfs_t entry;

	aos_mem_zero(&entry, sizeof (fal_flow_cookie_t));

	rv = __cmd_data_check_complex("proto", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.proto),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("src addr", NULL,
                            "usage: the format is xx.xx.xx.xx \n",
                            cmd_data_check_ip4addr, &(entry.src_addr),
                            4);
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("dst addr", NULL,
                            "usage: the format is xx.xx.xx.xx \n",
                            cmd_data_check_ip4addr, &(entry.dst_addr),
                            4);
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("src port", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.src_port),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("dst port", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.dst_port),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("flow rfs", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &tmp,
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	entry.load_balance = tmp;


	*(fal_flow_rfs_t *)val = entry;
	return SW_OK;
}

sw_error_t
cmd_data_check_ip6_rfs_entry(char *cmd_str, void * val, a_uint32_t size)
{
	char *cmd;
	a_uint32_t tmp;
	sw_error_t rv;
	fal_ip6_rfs_t entry;

	aos_mem_zero(&entry, sizeof (fal_ip4_rfs_t));

	rv = __cmd_data_check_complex("mac addr", NULL,
                        "usage: the format is xx-xx-xx-xx-xx-xx \n",
                        cmd_data_check_macaddr, &(entry.mac_addr),
                        sizeof (fal_mac_addr_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("ip6 addr", NULL,
                            "usage: the format is xxxx::xxxx \n",
                            cmd_data_check_ip6addr, &(entry.ip6_addr),
                            16);
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("vid", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &(entry.vid),
                        sizeof (a_uint32_t));
    if (rv)
        return rv;

	rv = __cmd_data_check_complex("loadbalance", "0",
                        "usage: the format is xx \n",
                        cmd_data_check_uint32, &tmp,
                        sizeof (a_uint32_t));
    if (rv)
        return rv;
	entry.load_balance = tmp;

	*(fal_ip6_rfs_t *)val = entry;
	return SW_OK;
}


