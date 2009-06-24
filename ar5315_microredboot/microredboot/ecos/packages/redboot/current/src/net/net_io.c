//==========================================================================
//
//      net/net_io.c
//
//      Stand-alone network logical I/O support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
// Copyright (C) 2002, 2003, 2004 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <net/net.h>
#include <cyg/hal/hal_misc.h>	// Helper functions
#include <cyg/hal/hal_if.h>	// HAL I/O interfaces
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_ass.h>	// assertion macros

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <flash_config.h>

RedBoot_config_option("GDB connection port",
		      gdb_port,
		      ALWAYS_ENABLED, true,
		      CONFIG_INT, CYGNUM_REDBOOT_NETWORKING_TCP_PORT);
//RedBoot_config_option("Network debug at boot time", 
//                      net_debug, 
//                      ALWAYS_ENABLED, true,
//                      CONFIG_BOOL,
//                      false
//    );
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
RedBoot_config_option("Default network device",
		      net_device, ALWAYS_ENABLED, true, CONFIG_NETPORT, "");
#endif
// Note: the following options are related.  If 'bootp' is false, then
// the other values are used in the configuration.  Because of the way
// that configuration tables are generated, they should have names which
// are related.  The configuration options will show up lexicographically
// ordered, thus the peculiar naming.  In this case, the 'use' option is
// negated (if false, the others apply) which makes the names even more
// confusing.

#ifndef CYGSEM_REDBOOT_DEFAULT_NO_BOOTP
#define CYGSEM_REDBOOT_DEFAULT_NO_BOOTP 1
#endif
RedBoot_config_option("Use BOOTP for network configuration",
		      bootp,
		      ALWAYS_ENABLED, true,
		      CONFIG_BOOL, !CYGSEM_REDBOOT_DEFAULT_NO_BOOTP);
RedBoot_config_option("Local IP address",
		      bootp_my_ip, "bootp", false, CONFIG_IP, 0);
#ifdef CYGSEM_REDBOOT_NETWORKING_USE_GATEWAY
RedBoot_config_option("Local IP address mask",
		      bootp_my_ip_mask, "bootp", false, CONFIG_IP, 0);
RedBoot_config_option("Gateway IP address",
		      bootp_my_gateway_ip, "bootp", false, CONFIG_IP, 0);
#endif
RedBoot_config_option("Default server IP address",
		      bootp_server_ip, ALWAYS_ENABLED, true, CONFIG_IP, 0);

// Note: the following options are related too.
RedBoot_config_option("Force console for special debug messages",
		      info_console_force,
		      ALWAYS_ENABLED, true, CONFIG_BOOL, false);
RedBoot_config_option("Console number for special debug messages",
		      info_console_number,
		      "info_console_force", true, CONFIG_INT, 0);
#endif

#define TCP_CHANNEL CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS

#ifdef DEBUG_TCP
int show_tcp = 0;
#endif

static tcp_socket_t tcp_sock;
static int state;
static int _timeout = 500;
static int orig_console, orig_debug;

static int in_buflen = 0;
static unsigned char in_buf[64];
static unsigned char *in_bufp;
static int out_buflen = 0;
static unsigned char out_buf[1024];
static unsigned char *out_bufp;
static bool flush_output_lines = false;

// Functions in this module
static void net_io_flush(void);
static void net_io_revert_console(void);
static void net_io_putc(void *, cyg_uint8);

// Special characters used by Telnet - must be interpretted here
#define TELNET_IAC    0xFF	// Interpret as command (escape)
#define TELNET_IP     0xF4	// Interrupt process
#define TELNET_WONT   0xFC	// I Won't do it
#define TELNET_DO     0xFD	// Will you XXX
#define TELNET_TM     0x06	// Time marker (special DO/WONT after IP)

static cyg_bool _net_io_getc_nonblock(void *__ch_data, cyg_uint8 * ch)
{
	if (in_buflen == 0) {
		__tcp_poll();
		if (tcp_sock.state == _CLOSE_WAIT) {
			// This connection is breaking
			if (tcp_sock.data_bytes == 0 && tcp_sock.rxcnt == 0) {
				__tcp_close(&tcp_sock);
				return false;
			}
		}
		if (tcp_sock.state == _CLOSED) {
			// The connection is gone
			net_io_revert_console();
			*ch = '\n';
			return true;
		}
		in_buflen = __tcp_read(&tcp_sock, in_buf, sizeof(in_buf));
		in_bufp = in_buf;
#ifdef DEBUG_TCP
		if (show_tcp && (in_buflen > 0)) {
			int old_console;
			old_console = start_console();
			diag_printf("%s:%d\n", __FUNCTION__, __LINE__);
			diag_dump_buf(in_buf, in_buflen);
			end_console(old_console);
		}
#endif				// DEBUG_TCP
	}
	if (in_buflen) {
		*ch = *in_bufp++;
		in_buflen--;
		return true;
	} else {
		return false;
	}
}

static cyg_bool net_io_getc_nonblock(void *__ch_data, cyg_uint8 * ch)
{
	cyg_uint8 esc;

	if (!_net_io_getc_nonblock(__ch_data, ch))
		return false;

	if (gdb_active || *ch != TELNET_IAC)
		return true;

	// Telnet escape - need to read/handle more
	while (!_net_io_getc_nonblock(__ch_data, &esc)) ;

	switch (esc) {
	case TELNET_IAC:
		// The other special case - escaped escape
		return true;
	case TELNET_IP:
		// Special case for ^C == Interrupt Process
		*ch = 0x03;
		// Just in case the other end needs synchronizing
		net_io_putc(__ch_data, TELNET_IAC);
		net_io_putc(__ch_data, TELNET_WONT);
		net_io_putc(__ch_data, TELNET_TM);
		net_io_flush();
		return true;
	case TELNET_DO:
		// Telnet DO option
		while (!_net_io_getc_nonblock(__ch_data, &esc)) ;
		// Respond with WONT option
		net_io_putc(__ch_data, TELNET_IAC);
		net_io_putc(__ch_data, TELNET_WONT);
		net_io_putc(__ch_data, esc);
		return false;	// Ignore this whole thing!
	default:
		return false;
	}
}

static cyg_uint8 net_io_getc(void *__ch_data)
{
	cyg_uint8 ch;
	int idle_timeout = 10;	// 10ms

	CYGARC_HAL_SAVE_GP();
	while (true) {
		if (net_io_getc_nonblock(__ch_data, &ch))
			break;
		if (--idle_timeout == 0) {
			net_io_flush();
			idle_timeout = 10;
		}
	}
	CYGARC_HAL_RESTORE_GP();
	return ch;
}

static void net_io_flush(void)
{
	int n;
	char *bp = out_buf;

#ifdef DEBUG_TCP
	if (show_tcp) {
		int old_console;
		old_console = start_console();
		diag_printf("%s.%d\n", __FUNCTION__, __LINE__);
		diag_dump_buf(out_buf, out_buflen);
		end_console(old_console);
	}
#endif				// SHOW_TCP
	n = __tcp_write_block(&tcp_sock, bp, out_buflen);
	if (n < 0) {
		// The connection is gone!
		net_io_revert_console();
	} else {
		out_buflen -= n;
		bp += n;
	}
	out_bufp = out_buf;
	out_buflen = 0;
	// Check interrupt flag
	if (CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG()) {
		CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG_SET(0);
		cyg_hal_user_break(0);
	}
}

static void net_io_putc(void *__ch_data, cyg_uint8 c)
{
	static bool have_dollar, have_hash;
	static int hash_count;

	CYGARC_HAL_SAVE_GP();
	*out_bufp++ = c;
	if (c == '$')
		have_dollar = true;
	if (have_dollar && (c == '#')) {
		have_hash = true;
		hash_count = 0;
	}
	if ((++out_buflen == sizeof(out_buf)) ||
	    (flush_output_lines && c == '\n') ||
	    (have_hash && (++hash_count == 3))) {
		net_io_flush();
		have_dollar = false;
	}
	CYGARC_HAL_RESTORE_GP();
}

static void
net_io_write(void *__ch_data, const cyg_uint8 * __buf, cyg_uint32 __len)
{
	int old_console;

	old_console = start_console();
	diag_printf("%s.%d\n", __FUNCTION__, __LINE__);
	end_console(old_console);
#if 0
	CYGARC_HAL_SAVE_GP();

	while (__len-- > 0)
		net_io_putc(__ch_data, *__buf++);

	CYGARC_HAL_RESTORE_GP();
#endif
}

static void net_io_read(void *__ch_data, cyg_uint8 * __buf, cyg_uint32 __len)
{
	int old_console;

	old_console = start_console();
	diag_printf("%s.%d\n", __FUNCTION__, __LINE__);
	end_console(old_console);
#if 0
	CYGARC_HAL_SAVE_GP();

	while (__len-- > 0)
		*__buf++ = net_io_getc(__ch_data);

	CYGARC_HAL_RESTORE_GP();
#endif
}

static cyg_bool net_io_getc_timeout(void *__ch_data, cyg_uint8 * ch)
{
	int delay_count;
	cyg_bool res;

	CYGARC_HAL_SAVE_GP();
	net_io_flush();		// Make sure any output has been sent
	delay_count = _timeout;

	for (;;) {
		res = net_io_getc_nonblock(__ch_data, ch);
		if (res || 0 == delay_count--)
			break;
	}

	CYGARC_HAL_RESTORE_GP();

	return res;
}

static int net_io_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
	static int vector = 0;
	int ret = 0;
	static int irq_state = 0;

	CYGARC_HAL_SAVE_GP();

	switch (__func) {
	case __COMMCTL_IRQ_ENABLE:
		irq_state = 1;
		if (vector == 0) {
			vector = eth_drv_int_vector();
		}
		HAL_INTERRUPT_UNMASK(vector);
		break;
	case __COMMCTL_IRQ_DISABLE:
		ret = irq_state;
		irq_state = 0;
		if (vector == 0) {
			vector = eth_drv_int_vector();
		}
		HAL_INTERRUPT_MASK(vector);
		break;
	case __COMMCTL_DBG_ISR_VECTOR:
		ret = vector;
		break;
	case __COMMCTL_SET_TIMEOUT:
		{
			va_list ap;

			va_start(ap, __func);

			ret = _timeout;
			_timeout = va_arg(ap, cyg_uint32);

			va_end(ap);
			break;
		}
	case __COMMCTL_FLUSH_OUTPUT:
		net_io_flush();
		break;
	case __COMMCTL_ENABLE_LINE_FLUSH:
		flush_output_lines = true;
		break;
	case __COMMCTL_DISABLE_LINE_FLUSH:
		flush_output_lines = false;
		break;
	default:
		break;
	}
	CYGARC_HAL_RESTORE_GP();
	return ret;
}

static int
net_io_isr(void *__ch_data, int *__ctrlc,
	   CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
	char ch;

	CYGARC_HAL_SAVE_GP();
	*__ctrlc = 0;
	if (net_io_getc_nonblock(__ch_data, &ch)) {
		if (ch == 0x03) {
			*__ctrlc = 1;
		}
	}
	CYGARC_HAL_RESTORE_GP();
	return CYG_ISR_HANDLED;
}

// TEMP

int start_console(void)
{
	int cur_console =
	    CYGACC_CALL_IF_SET_CONSOLE_COMM
	    (CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
	int i = 0;
	if (flash_get_config("info_console_force", &i, CONFIG_BOOL))
		if (i)
			if (!flash_get_config
			    ("info_console_number", &i, CONFIG_INT))
				i = 0;	// the default, if that call failed.
	if (i)
		CYGACC_CALL_IF_SET_CONSOLE_COMM(i);
	else
#endif
		CYGACC_CALL_IF_SET_CONSOLE_COMM(0);

	return cur_console;
}

void end_console(int old_console)
{
	// Restore original console
	CYGACC_CALL_IF_SET_CONSOLE_COMM(old_console);
}

// TEMP

static void net_io_revert_console(void)
{
#ifdef CYGPKG_REDBOOT_ANY_CONSOLE
	console_selected = false;
#endif
	CYGACC_CALL_IF_SET_CONSOLE_COMM(orig_console);
	CYGACC_CALL_IF_SET_DEBUG_COMM(orig_debug);
	console_echo = true;
}

static void net_io_assume_console(void)
{
#ifdef CYGPKG_REDBOOT_ANY_CONSOLE
	console_selected = true;
#endif
	console_echo = false;
	orig_console =
	    CYGACC_CALL_IF_SET_CONSOLE_COMM
	    (CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
	CYGACC_CALL_IF_SET_CONSOLE_COMM(TCP_CHANNEL);
	orig_debug =
	    CYGACC_CALL_IF_SET_DEBUG_COMM
	    (CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
	CYGACC_CALL_IF_SET_DEBUG_COMM(TCP_CHANNEL);
}

static void net_io_init(void)
{
	static int init = 0;
	if (!init) {
		hal_virtual_comm_table_t *comm;
		int cur =
		    CYGACC_CALL_IF_SET_CONSOLE_COMM
		    (CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

		// Setup procs in the vector table
		CYGACC_CALL_IF_SET_CONSOLE_COMM(TCP_CHANNEL);
		comm = CYGACC_CALL_IF_CONSOLE_PROCS();
		//CYGACC_COMM_IF_CH_DATA_SET(*comm, chan);
		CYGACC_COMM_IF_WRITE_SET(*comm, net_io_write);
		CYGACC_COMM_IF_READ_SET(*comm, net_io_read);
		CYGACC_COMM_IF_PUTC_SET(*comm, net_io_putc);
		CYGACC_COMM_IF_GETC_SET(*comm, net_io_getc);
		CYGACC_COMM_IF_CONTROL_SET(*comm, net_io_control);
		CYGACC_COMM_IF_DBG_ISR_SET(*comm, net_io_isr);
		CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, net_io_getc_timeout);

		// Disable interrupts via this interface to set static
		// state into correct state.
		net_io_control(comm, __COMMCTL_IRQ_DISABLE);

		// Restore original console
		CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);

		init = 1;
		gdb_active = false;
	}
	__tcp_listen(&tcp_sock, gdb_port);
	state = tcp_sock.state;
#ifdef DEBUG_TCP
	diag_printf("show tcp = %p\n", (void *)&show_tcp);
#endif
}

// Check for incoming TCP debug connection
void net_io_test(bool is_idle)
{
	if (!is_idle)
		return;		// Only care about idle case
	if (!have_net)
		return;
	__tcp_poll();
	if (state != tcp_sock.state) {
		// Something has changed
		if (tcp_sock.state == _ESTABLISHED) {
			// A new connection has arrived
			net_io_assume_console();
			in_bufp = in_buf;
			in_buflen = 1;
			*in_bufp = '\r';
			out_bufp = out_buf;
			out_buflen = 0;
		}
		if (tcp_sock.state == _CLOSED) {
			net_io_init();	// Get ready for another connection
		}
	}
	state = tcp_sock.state;
}

// This schedules the 'net_io_test()' function to be run by RedBoot's
// main command loop when idle (i.e. when no input arrives after some
// period of time).
RedBoot_idle(net_io_test, RedBoot_IDLE_NETIO);

//
// Network initialization
//
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/hal/hal_tables.h>

// Define table boundaries
CYG_HAL_TABLE_BEGIN(__NETDEVTAB__, netdev);
CYG_HAL_TABLE_END(__NETDEVTAB_END__, netdev);

RedBoot_init(net_init, RedBoot_INIT_LAST);

static void show_addrs(void)
{
	diag_printf("IP: %s", inet_ntoa((in_addr_t *) & __local_ip_addr));
#ifdef CYGSEM_REDBOOT_NETWORKING_USE_GATEWAY
	diag_printf("/%s", inet_ntoa((in_addr_t *) & __local_ip_mask));
	diag_printf(", Gateway: %s\n",
		    inet_ntoa((in_addr_t *) & __local_ip_gate));
#else
	diag_printf(", ");
#endif
	diag_printf("Default server: %s", inet_ntoa(&my_bootp_info.bp_siaddr));
#ifdef CYGPKG_REDBOOT_NETWORKING_DNS
	show_dns();
#endif
	diag_printf("\n");
}

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
static void flash_get_IP(char *id, ip_addr_t * val)
{
	ip_addr_t my_ip;
	int i;

	if (flash_get_config(id, &my_ip, CONFIG_IP)) {
		if (my_ip[0] != 0 || my_ip[1] != 0 ||
		    my_ip[2] != 0 || my_ip[3] != 0) {
			// 'id' is set to something so let it override any static IP
			for (i = 0; i < 4; i++)
				(*val)[i] = my_ip[i];
		}
	}
}
#endif

static cyg_netdevtab_entry_t *net_devtab_entry(unsigned index)
{
	cyg_netdevtab_entry_t *t = &__NETDEVTAB__[index];

	if (t < &__NETDEVTAB__[0] || t >= &__NETDEVTAB_END__)
		return NULL;

	return t;
}

const char *net_devname(unsigned index)
{
	cyg_netdevtab_entry_t *t = net_devtab_entry(index);
	if (t)
		return t->name;
	return NULL;
}

int net_devindex(char *name)
{
	const char *devname;
	int index;

	for (index = 0; (devname = net_devname(index)) != NULL; index++)
		if (!strcmp(name, devname))
			return index;
	return -1;
}

static void show_eth_info(void)
{
	diag_printf("Ethernet %s: MAC address %02x:%02x:%02x:%02x:%02x:%02x\n",
		    __local_enet_sc->dev_name,
		    __local_enet_addr[0],
		    __local_enet_addr[1],
		    __local_enet_addr[2],
		    __local_enet_addr[3],
		    __local_enet_addr[4], __local_enet_addr[5]);
}

void net_init(void)
{
	cyg_netdevtab_entry_t *t;
	unsigned index;
	struct eth_drv_sc *primary_net = (struct eth_drv_sc *)0;
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
	char *default_devname;
	int default_index;
#endif
#ifdef CYGDAT_REDBOOT_DEFAULT_BOOTP_SERVER_IP_ADDR
	char ip_addr[16];
#endif

	// Set defaults as appropriate
#ifdef CYGSEM_REDBOOT_DEFAULT_NO_BOOTP
	use_bootp = false;
#else
	use_bootp = true;
#endif
#ifdef CYGDBG_REDBOOT_NET_DEBUG
	net_debug = true;
#else
	net_debug = false;
#endif
	gdb_port = CYGNUM_REDBOOT_NETWORKING_TCP_PORT;
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
	// Fetch values from saved config data, if available
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
	flash_get_config("net_device", &default_devname, CONFIG_NETPORT);
#endif
//    flash_get_config("net_debug", &net_debug, CONFIG_BOOL);
	flash_get_config("gdb_port", &gdb_port, CONFIG_INT);
	flash_get_config("bootp", &use_bootp, CONFIG_BOOL);
	if (!use_bootp) {
		flash_get_IP("bootp_my_ip", &__local_ip_addr);
#ifdef CYGSEM_REDBOOT_NETWORKING_USE_GATEWAY
		flash_get_IP("bootp_my_ip_mask", &__local_ip_mask);
		flash_get_IP("bootp_my_gateway_ip", &__local_ip_gate);
#endif
	}
#endif
# ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
	// Don't override if the user has deliberately set something more
	// verbose.
	if (0 == cyg_io_eth_net_debug)
		cyg_io_eth_net_debug = net_debug;
# endif
	have_net = false;
	// Make sure the recv buffers are set up
	eth_drv_buffers_init();
	__pktbuf_init();

	// Initialize network device(s).
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
	default_index = net_devindex(default_devname);
	if (default_index < 0)
		default_index = 0;
#ifdef CYGSEM_REDBOOT_NETWORK_INIT_ONE_DEVICE
	if ((t = net_devtab_entry(default_index)) != NULL && t->init(t)) {
		t->status = CYG_NETDEVTAB_STATUS_AVAIL;
		primary_net = __local_enet_sc;
	} else
#endif
#endif
		for (index = 0; (t = net_devtab_entry(index)) != NULL; index++) {
#ifdef CYGSEM_REDBOOT_NETWORK_INIT_ONE_DEVICE
			if (index == default_index)
				continue;
#endif
			if (t->init(t)) {
				t->status = CYG_NETDEVTAB_STATUS_AVAIL;
				if (primary_net == (struct eth_drv_sc *)0) {
					primary_net = __local_enet_sc;
				}
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
				if (index == default_index) {
					primary_net = __local_enet_sc;
				}
#endif
			}
		}
	__local_enet_sc = primary_net;

	if (!__local_enet_sc) {
		diag_printf("No network interfaces found\n");
		return;
	}
	// Initialize the network [if present]
/*    if (use_bootp) {
        if (__bootp_find_local_ip(&my_bootp_info) == 0) {
            have_net = true;
        } else {
            // Is it an unset address, or has it been set to a static addr
            if (__local_ip_addr[0] == 0 && __local_ip_addr[1] == 0 &&
                __local_ip_addr[2] == 0 && __local_ip_addr[3] == 0) {
                show_eth_info();
                diag_printf("Can't get BOOTP info for device!\n");
            } else {
                diag_printf("Can't get BOOTP info, using default IP address\n");
                have_net = true;
            }
        }
    } else {
*/
	if (__local_ip_addr[0] == 0 && __local_ip_addr[1] == 0 &&
	    __local_ip_addr[2] == 0 && __local_ip_addr[3] == 0) {
		__local_ip_addr[0] = 192;
		__local_ip_addr[1] = 168;
		__local_ip_addr[2] = 1;
		__local_ip_addr[3] = 1;
//                __local_ip_mask[0]=255;
//                __local_ip_mask[1]=255;
//                __local_ip_mask[2]=0;
//                __local_ip_mask[3]=0;
	}

	{

		enet_addr_t enet_addr;
		have_net = true;	// Assume values in FLASH were OK
		// Tell the world that we are using this fixed IP address
		if (__arp_request((ip_addr_t *) __local_ip_addr, &enet_addr, 1)
		    >= 0) {
			diag_printf("Warning: IP address %s in use\n",
				    inet_ntoa((in_addr_t *) & __local_ip_addr));
		}
	}
//    }
	if (have_net) {
		show_eth_info();
#ifdef CYGDAT_REDBOOT_DEFAULT_BOOTP_SERVER_IP_ADDR
		diag_sprintf(ip_addr, "%d.%d.%d.%d",
			     CYGDAT_REDBOOT_DEFAULT_BOOTP_SERVER_IP_ADDR);
		inet_aton(ip_addr, &my_bootp_info.bp_siaddr);
#endif
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
		flash_get_IP("bootp_server_ip",
			     (ip_addr_t *) & my_bootp_info.bp_siaddr);
#endif
#ifdef CYGPKG_REDBOOT_NETWORKING_DNS
		redboot_dns_res_init();
#endif
		show_addrs();
		net_io_init();
	}
}

static char usage[] =
    "[-l <local_ip_address>[/<mask_len>]] [-h <server_address>]";

// Exported CLI function
static void do_ip_addr(int argc, char *argv[]);
RedBoot_cmd("ip_address", "Set/change IP addresses", usage, do_ip_addr);

void do_ip_addr(int argc, char *argv[])
{
	struct option_info opts[3];
	char *ip_addr, *host_addr;
	bool ip_addr_set, host_addr_set;
	struct sockaddr_in host;
#ifdef CYGPKG_REDBOOT_NETWORKING_DNS
	char *dns_addr;
	bool dns_addr_set;
#endif
	int num_opts;

	init_opts(&opts[0], 'l', true, OPTION_ARG_TYPE_STR,
		  (void *)&ip_addr, (bool *) & ip_addr_set, "local IP address");
	init_opts(&opts[1], 'h', true, OPTION_ARG_TYPE_STR,
		  (void *)&host_addr, (bool *) & host_addr_set,
		  "default server address");
	num_opts = 2;
#ifdef CYGPKG_REDBOOT_NETWORKING_DNS
	init_opts(&opts[2], 'd', true, OPTION_ARG_TYPE_STR,
		  (void *)&dns_addr, (bool *) & dns_addr_set,
		  "DNS server address");
	num_opts++;
#endif

	CYG_ASSERT(num_opts <= NUM_ELEMS(opts), "Too many options");

	if (!scan_opts(argc, argv, 1, opts, num_opts, 0, 0, "")) {
		return;
	}
	if (ip_addr_set) {
#ifdef CYGSEM_REDBOOT_NETWORKING_USE_GATEWAY
		char *slash_pos;
		/* see if the (optional) mask length was given */
		if ((slash_pos = strchr(ip_addr, '/'))) {
			int mask_len;
			unsigned long mask;
			*slash_pos = '\0';
			slash_pos++;
			if (!parse_num
			    (slash_pos, (unsigned long *)&mask_len, 0, 0)
			    || mask_len <= 0 || mask_len > 32) {
				diag_printf("Invalid mask length: %s\n",
					    slash_pos);
				return;
			}
			mask =
			    htonl((0xffffffff << (32 - mask_len)) & 0xffffffff);
			memcpy(&__local_ip_mask, &mask, 4);
		}
#endif
		if (!_gethostbyname(ip_addr, (in_addr_t *) & host)) {
			diag_printf("Invalid local IP address: %s\n", ip_addr);
			return;
		}
		// Of course, each address goes in its own place :-)
		memcpy(&__local_ip_addr, &host.sin_addr, sizeof(host.sin_addr));
	}
	if (host_addr_set) {
		if (!_gethostbyname(host_addr, (in_addr_t *) & host)) {
			diag_printf("Invalid server address: %s\n", host_addr);
			return;
		}
		my_bootp_info.bp_siaddr = host.sin_addr;
	}
#ifdef CYGPKG_REDBOOT_NETWORKING_DNS
	if (dns_addr_set) {
		set_dns(dns_addr);
	}
#endif
	show_addrs();
	if (!have_net) {
		have_net = true;
		net_io_init();
	}
}

// EOF net_io.c
