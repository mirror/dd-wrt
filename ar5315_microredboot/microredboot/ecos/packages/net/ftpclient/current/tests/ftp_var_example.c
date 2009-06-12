#include <pkgconf/system.h>
#define _config_OK
#if !defined(CYGPKG_NET_FTPCLIENT)
#error "Configuration is missing: CYGPKG_NET_FTPCLIENT"
#undef _config_OK
#endif
#if !defined(CYGPKG_COMPRESS_ZLIB)
#error "Configuration is missing: CYGPKG_COMPRESS_ZLIB"
#undef _config_OK
#else
#include <pkgconf/compress_zlib.h>
#if !defined(CYGSEM_COMPRESS_ZLIB_DEFLATE_MAKES_GZIP)
#error "Configuration is missing: CYGSEM_COMPRESS_ZLIB_DEFLATE_MAKES_GZIP"
#undef _config_OK
#endif
#endif
#if !defined(CYGPKG_CRC)
#error "Configuration is missing: CYGPKG_CRC"
#undef _config_OK
#endif
 
#ifdef _config_OK
#include <stdio.h>
#include <stdlib.h>
#include <ftpclient.h>
#include <cyg/compress/zlib.h>

#include <pkgconf/net.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/route.h>

#include <cyg/infra/diag.h>
#include <cyg/kernel/kapi.h>

#include <bootp.h>
#include <network.h>
#include <arpa/inet.h>

#include "nvram.h"
#include "lkvm.h"
#include "getopt.h"

#define MAX_ARGV 32

#ifdef CYGHWR_NET_DRIVER_ETH0
static struct bootp eth0_bootp_data;
static const char  *eth0_name = "eth0";
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
static struct bootp eth1_bootp_data;
static const char  *eth1_name = "eth1";
#endif

// TEMP for debug
core_info_t test_core_info = {
    .fence0 = KCORE_FENCE0_PAT,
    .fence1 = KCORE_FENCE1_PAT,
    .fence2 = KCORE_FENCE2_PAT,
    .fence3 = KCORE_FENCE3_PAT,
    .crash = KCORE_CRASH_PANIC,
    .panicstr = "This is a panic string",
    .ncpu = 1,
    .nsegs = 2,
    .physical_banks[0] = { 0x700000, 0x40000 },
    .physical_banks[1] = { 0x740000, 0x40000 },
    .sr[0] = 0xB00B0000,
    .sr[1] = 0xB00B0001,
    .sr[2] = 0xB00B0002,
    .sr[3] = 0xB00B0003,
    .sr[4] = 0xB00B0004,
    .sr[5] = 0xB00B0005,
    .sr[6] = 0xB00B0006,
    .sr[7] = 0xB00B0007,
    .dbatl[0] = 0xDEAD0000, .dbatu[0] = 0xDEAD1000,
    .dbatl[1] = 0xDEAD0001, .dbatu[1] = 0xDEAD1001,
    .dbatl[2] = 0xDEAD0002, .dbatu[2] = 0xDEAD1002,
    .dbatl[3] = 0xDEAD0003, .dbatu[3] = 0xDEAD1003,
    .dbatl[4] = 0xDEAD0004, .dbatu[4] = 0xDEAD1004,
    .dbatl[5] = 0xDEAD0005, .dbatu[5] = 0xDEAD1005,
    .dbatl[6] = 0xDEAD0006, .dbatu[6] = 0xDEAD1006,
    .dbatl[7] = 0xDEAD2007, .dbatu[7] = 0xDEAD3007,
    .ibatl[0] = 0xDEAD2000, .ibatu[0] = 0xDEAD3000,
    .ibatl[1] = 0xDEAD2001, .ibatu[1] = 0xDEAD3001,
    .ibatl[2] = 0xDEAD2002, .ibatu[2] = 0xDEAD3002,
    .ibatl[3] = 0xDEAD2003, .ibatu[3] = 0xDEAD3003,
    .ibatl[4] = 0xDEAD2004, .ibatu[4] = 0xDEAD3004,
    .ibatl[5] = 0xDEAD2005, .ibatu[5] = 0xDEAD3005,
    .ibatl[6] = 0xDEAD2006, .ibatu[6] = 0xDEAD3006,
    .ibatl[7] = 0xDEAD2007, .ibatu[7] = 0xDEAD3007,
};
// TEMP

#define FTP_BUFSIZE 4096
struct ftp_data {
    z_stream stream;
    char     buf[FTP_BUFSIZE], *bufp;
    int      len;
    int      src_len;
    int      first;
    int    (*get_data)(char **ptr, void *param);
    void    *get_param;
};

int _ftp_write_gz(char *buf, int len, void *priv)
{
    struct ftp_data *dp = (struct ftp_data *)priv;
    int err;
    int res = 0;
    char *src;

    if (dp->first) {
        // Ugly hack - zlib needs a value for 'avail_in' at initialization
        // time, even though we don't have anything ready yet
        dp->stream.avail_in = 0;
        dp->first = 0;
    }
    while ((dp->len == 0) && (dp->src_len > 0)) {
        // Has input buffer been exhausted?
        if (dp->stream.avail_in == 0) {
            dp->src_len = (*dp->get_data)(&src, dp->get_param);
            dp->stream.next_in = src;
            dp->stream.avail_in = dp->src_len;
        }        
        // See if more data is available to be compressed
        dp->stream.avail_out = FTP_BUFSIZE;
        dp->stream.next_out = dp->buf;
        err = deflate(&dp->stream, dp->src_len ? Z_NO_FLUSH : Z_FINISH);
        dp->len = FTP_BUFSIZE - dp->stream.avail_out;
        dp->bufp = dp->buf;
    }
    // Move up to 'len' bytes from internal buffer
    res = len;
    if (res > dp->len) {
        res = dp->len;
    }
    memcpy(buf, dp->bufp, res);
    dp->bufp += res;
    dp->len -= res;
    return res;
}

struct _simple_get_data_param {
    char *ptr;
    int   len;
};

int
simple_get_data(char **ptr, void *param)
{
    struct _simple_get_data_param *gp = (struct _simple_get_data_param *)param;
    int res;

    *ptr = gp->ptr;
    res = gp->len;
    gp->len = 0;
    return res;
    
}


//
// Parse (scan/copy) network addresses.  If the IP address contains
// a '/', then use that to determine the netmask and ignore the
// specified value.
//
bool
parse_eth_addrs(char *desired_ip, char *desired_netmask, 
                char *ip, char *netmask, char *broadcast)
{
    char *src, *dst;
    bool found_netmask = false;
    int mask_size, mask, ip_addr;

//    diag_printf("parse - ip: '%s', mask: '%s'\n", desired_ip, desired_netmask);
    // IP address
    src = desired_ip;  dst = ip;
    while (*src) {
        if (*src == '/') {
            // Do something with netmask
            src++;
            found_netmask = true;
            mask_size = 0;
            while (*src) {
                mask_size = (mask_size * 10) + (*src++ - '0');
            }
            mask = 0xFFFFFFFF << (32-mask_size);
            break;
        }
        *dst++ = *src++;
    }
    *dst = '\0';
    if (!found_netmask) {
        if (!inet_aton(desired_netmask, (struct in_addr *)&mask)) {
            diag_printf("Error: Invalid netmask '%s'\n", desired_netmask);
            return false;  // Illegal netmask
        }
    }
    strcpy(netmask, inet_ntoa(*(struct in_addr *)&mask));
    if (!inet_aton(ip, (struct in_addr *)&ip_addr)) {
        diag_printf("Error: Invalid IP address '%s'\n", ip);
        return false;
    }
    ip_addr = (ip_addr & mask) | ~mask;
    strcpy(broadcast, inet_ntoa(*(struct in_addr *)&ip_addr));
//    diag_printf("ip: '%s', netmask: '%s', broadcast: '%s'\n", ip, netmask, broadcast);
    return true;
}

struct core_dump_param {
    int             phase;
    int             segno;
    core_info_t    *cp;
    kcore_hdr_t     hdr;
    kcore_seg_t     cpu_hdr, ram_hdr;
    cpu_kcore_hdr_t cpu_info;
    phys_ram_seg_t  cpu_phys_segs[8];
};

#define ALIGN(v,n) ((((v)+((n)-1))/(n))*(n))

int
dump_core(char **ptr, void *param)
{
    struct core_dump_param *dp = (struct core_dump_param *)param;
    int len;

    switch (dp->phase) {
    case 0:
        // File header (kcore_hdr_t)
        dp->phase = 1;
        *ptr = (char *)&dp->hdr;
        return ALIGN(sizeof(dp->hdr), 8);
    case 1:
        // CPU header (kcore_seg_t)
        dp->phase = 2;
        *ptr = (char *)&dp->cpu_hdr;
        return ALIGN(sizeof(dp->cpu_hdr), 8);
    case 2:
        // CPU info (cpu_kcore_hdr_t)
        dp->phase = 3;
        dp->segno = 0;
        *ptr = (char *)&dp->cpu_info;
        return sizeof(dp->cpu_info);
    case 3:
        // Physical RAM segment descriptions (phys_ram_seg_t)
        *ptr = (char *)&dp->cpu_phys_segs[dp->segno];
        if (++dp->segno == dp->cp->nsegs) {
            dp->phase = 4;
        }
        return sizeof(dp->cpu_phys_segs[0]);
    case 4:
        // RAM header (kcore_seg_t)
        dp->phase = 5;
        *ptr = (char *)&dp->ram_hdr;
        dp->segno = 0;
        return ALIGN(sizeof(dp->ram_hdr), 8);
    case 5:
        // RAM segment
        *ptr = (char *)dp->cpu_phys_segs[dp->segno].start;
        len = dp->cpu_phys_segs[dp->segno].size;
        if (++dp->segno == dp->cp->nsegs) {
            dp->phase = 6;
        }
        return len;
    case 6:
        // All done!
        return 0;
    default:
        diag_printf("Bad core dump phase: %d\n", dp->phase);
        return 0;
    }
}

void
expand_file_name(char *file_name, char *path, char *ip)
{
    char *fn = nvram_get("dump_name");
    char *dp = file_name;
    char last = '\0';

    if (!fn) fn = "XPathOS.%i.core.gz";       
    if (!path) path = "/";
    while (*path) {
        last = *path++;
        *dp++ = last;
    }
    if (last != '/') {
        *dp++ = '/';
    }
    while (*fn) {
        if (*fn == '%') {
            fn += 2;  // Skip '%i'
            while (*ip) {
                *dp++ = *ip++;
            }
        } else {
            *dp++ = *fn++;
        }
    }
    *dp = '\0';
}

void
do_core_dump(core_info_t *cp, char *ip)
{
    struct core_dump_param cd;
    // Caution - this can't be on the stack unless you increase the stack size!
    static struct ftp_data dp;
    int err, wlen, i;
    unsigned long total_ram_size;
    char file_name[256];
    char *dump_ip, *dump_user, *dump_pass;

    dump_ip = nvram_get("dump_server");
    if (!dump_ip) {
        diag_printf("Can't dump: server IP missing in NVRAM\n");
        return;
    }
    dump_user = nvram_get("dump_user");
    if (!dump_user) {
        diag_printf("Can't dump: user name missing in NVRAM\n");
        return;
    }
    dump_pass = nvram_get("dump_pass");
    if (!dump_pass) {
        diag_printf("Can't dump: password missing in NVRAM\n");
        return;
    }
    expand_file_name(file_name, nvram_get("dump_path"), ip);

    // Fill in core dump structures
    cd.hdr.c_midmag = KCORE_HDR_MAGIC;
    cd.hdr.c_hdrsize = ALIGN(sizeof(cd.hdr), 8);
    cd.hdr.c_seghdrsize = ALIGN(sizeof(cd.cpu_hdr), 8);
    cd.hdr.c_nseg = cp->nsegs;

    cd.cpu_hdr.c_midmag = KCORE_CPU_SEGMENT;
    cd.cpu_hdr.c_size = sizeof(kcore_seg_t)+sizeof(cpu_kcore_hdr_t)+(cp->nsegs*sizeof(phys_ram_seg_t));
    // Copy info from core-dump area
    cd.cpu_info.pvr = cp->pvr;
    cd.cpu_info.sdr1 = cp->sdr1;
    for (i = 0;  i < 8;  i++) {
        cd.cpu_info.sr[i] = cp->sr[i];
        cd.cpu_info.dbatl[i] = cp->dbatl[i];
        cd.cpu_info.dbatu[i] = cp->dbatu[i];
        cd.cpu_info.ibatl[i] = cp->ibatl[i];
        cd.cpu_info.ibatu[i] = cp->ibatu[i];
    }
    for (i = 0;  i < 8;  i++) {
        cd.cpu_phys_segs[i].start = cp->physical_banks[i].base;
        cd.cpu_phys_segs[i].size = cp->physical_banks[i].length;
    }

    total_ram_size = 0;
    cd.ram_hdr.c_midmag = KCORE_DATA_SEGMENT;
    cd.ram_hdr.c_size = sizeof(kcore_seg_t)+total_ram_size;

    cd.phase = 0;
    cd.cp = cp;

    dp.len = 0;
    dp.get_data = dump_core;
    dp.get_param = &cd;
    dp.src_len = 1;
    dp.first = 1;
    dp.stream.next_in = 0;
    dp.stream.avail_in = 0x1000;
    dp.stream.next_out = dp.buf;
    dp.stream.avail_out = FTP_BUFSIZE;
    dp.stream.zalloc = (alloc_func)0;
    dp.stream.zfree = (free_func)0;
    dp.stream.opaque = (voidpf)0;

    err = deflateInit(&dp.stream, Z_DEFAULT_COMPRESSION);
    diag_printf("err = %d\n", err);

    diag_printf("... dumping core as %s\n", file_name);

    wlen = ftp_put_var(dump_ip, dump_user, dump_pass, file_name, 
                       _ftp_write_gz, &dp, ftpclient_printf);
    diag_printf("res = %d\n", wlen);
    deflateEnd(&dp.stream);
}

void
scan_args(char *cmdline, int *argc, char **argv, char **alt_cmdline)
{
    char *cp = cmdline;
    int indx = 0;

    argv[indx++] = "secondary-load";
    *alt_cmdline = (char *)0;
    while (*cp) {
        // Skip over any white space
        while (*cp && (*cp == ' ')) cp++;
        if (!*cp) break;
        // Check for -- break
        if ((cp[0] == '-') && (cp[1] == '-')) {
            // End of normal list
            cp += 2;
            while (*cp && (*cp == ' ')) cp++;
            *alt_cmdline = cp;
            *argc = indx;
            return;
        }
        // Split argument
        argv[indx++] = cp;
        while (*cp) {
            if (*cp == ' ') {
                *cp++ = '\0';
                break;
            }
            cp++;
        }
    }
    *argc = indx;
}

int
loader_main(char *cmdline)
{
    int len, wlen;
    int err;
    int argc, i;
    char *argv[MAX_ARGV], *alt_cmdline;
    bool force_dump = false;
    bool quick_boot = false;
    bool verbose = false;
    char *boot_info = (char *)NULL;
    // Caution - this can't be on the stack unless you increase the stack size!
    static struct ftp_data dp;
    struct _simple_get_data_param gp;
#ifdef CYGHWR_NET_DRIVER_ETH0
    char eth0_ip[32], eth0_netmask[32], eth0_broadcast[32];
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    char eth1_ip[32], eth1_netmask[32], eth1_broadcast[32];
#endif
    char *eth_gateway;

    scan_args(cmdline, &argc, argv, &alt_cmdline);
    while (1) {
        i = getopt(argc, argv, "fqv");
        if (i < 0) break;
        switch (i) {
        case 'f':
            force_dump = true;
            break;
        case 'q':
            quick_boot = true;
            break;
        case 'v':
            verbose = true;
            break;
        default:
            diag_printf("Invalid return from getopt: %d\n", i);
        }
    }
    if (optind < argc) {
        boot_info = argv[optind];
    };

    if (!(eth_gateway = nvram_get("net_gateway"))) {
        diag_printf("Error: need network gateway\n");
        return 1;
    }
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (!parse_eth_addrs(nvram_get("net0_ip"), nvram_get("net0_mask"), 
                         eth0_ip, eth0_netmask, eth0_broadcast)) {
        diag_printf("Can't set up ethernet 'net0' - I give up!\n");
        return 1;
    }
    build_bootp_record(&eth0_bootp_data,
                       eth0_name,
                       eth0_ip,
                       eth0_netmask,
                       eth0_broadcast,
                       eth_gateway,
                       "0.0.0.0");
    show_bootp(eth0_name, &eth0_bootp_data);
    if (!init_net(eth0_name, &eth0_bootp_data)) {
        diag_printf("Network initialization failed for eth0\n");
    }
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    if (!parse_eth_addrs(nvram_get("net1_ip"), nvram_get("net1_mask"), 
                         eth1_ip, eth1_netmask, eth1_broadcast)) {
        diag_printf("Can't set up ethernet 'net1' - I give up!\n");
        return 1;
    }
    build_bootp_record(&eth1_bootp_data,
                       eth1_name,
                       eth1_ip,
                       eth1_netmask,
                       eth1_broadcast,
                       eth_gateway,
                       "0.0.0.0");
    show_bootp(eth1_name, &eth1_bootp_data);
    if (!init_net(eth1_name, &eth1_bootp_data)) {
        diag_printf("Network initialization failed for eth1\n");
    }
#endif

    if (!quick_boot) {
        if (!force_dump) {
            // See if the 'core dump' page contains an indication that a
            // dump should be taken
            force_dump = true;
        }
        do_core_dump(&test_core_info, eth0_ip);
    }

    do_load(boot_info);

    len = ftp_get("192.168.1.125", "gthomas", "Zorkle!!", "TEST.dat", 
                  (char *)0x200000, 0x100000, ftpclient_printf);
    diag_printf("res = %d\n", len);
    if (len <= 0) exit(0);
    wlen = ftp_put("192.168.1.125", "gthomas", "Zorkle!!", "TEST2.dat", 
                   (char *)0x200000, len, ftpclient_printf);
    diag_printf("res = %d\n", wlen);

    // Set up to compress data
    gp.ptr = (char *)0x200000;
    gp.len = len;
    dp.len = 0;
    dp.get_data = simple_get_data;
    dp.get_param = &gp;
    dp.src_len = 1;
    dp.first = 1;
    dp.stream.next_in = 0;
    dp.stream.avail_in = 0x1000;
    dp.stream.next_out = dp.buf;
    dp.stream.avail_out = FTP_BUFSIZE;
    dp.stream.zalloc = (alloc_func)0;
    dp.stream.zfree = (free_func)0;
    dp.stream.opaque = (voidpf)0;

    err = deflateInit(&dp.stream, Z_DEFAULT_COMPRESSION);
    diag_printf("err = %d\n", err);
    wlen = ftp_put_var("192.168.1.125", "gthomas", "Zorkle!!", "TEST3.dat.gz", 
                       _ftp_write_gz, &dp, ftpclient_printf);
    diag_printf("res = %d\n", wlen);
    deflateEnd(&dp.stream);

    return 0;
}

int
main(void)
{
    return (loader_main("-f -t test joe -- junk"));
}
#endif // _config_OK
