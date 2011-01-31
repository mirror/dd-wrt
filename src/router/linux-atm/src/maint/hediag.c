#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <atm.h>
#include <linux/atmdev.h>
#include <linux/sonet.h>
#include <linux/atm_he.h>

struct reg_table {
	unsigned addr;
	char *name;
};

struct reg_table pci_regs[] =
{
	{ 0x80000, "reset_cntl" },
	{ 0x80004, "host_cntl" },
	{ 0x80008, "lb_swap" },
	{ 0x8000c, "lb_mem_addr" },
	{ 0x80010, "lb_mem_data" },
	{ 0x80014, "lb_mem_access" },
	{ 0x80018, "sdram_ctl" },
	{ 0x8001c, "int_fifo" },
	{ 0x80020, "abort_addr" },

	{ 0x80080, "irq0_base" },
	{ 0x80084, "irq0_head" },
	{ 0x80088, "irq0_cntl" },
	{ 0x8008c, "irq0_data" },
	/* fill in 1-4 later */
	{ 0x800c0, "grp_10_map" },
	{ 0x800c4, "grp_32_map" },
	{ 0x800c8, "grp_54_map" },
	{ 0x800cc, "grp_76_map" },

	{ 0x80400, "g0_rbps_s" },
	{ 0x80404, "g0_rbps_t" },
	{ 0x80408, "g0_rbps_qi" },
	{ 0x8040c, "g0_rbps_bs" },
	{ 0x80410, "g0_rbpl_s" },
	{ 0x80414, "g0_rbpl_t" },
	{ 0x80418, "g0_rbpl_qi" },
	{ 0x8041c, "g0_rbpl_bs" },
	/* fill in 1-7 later */

	{ 0x80580, "g0_inmq_s" },
	{ 0x80584, "g0_inmq_l" },
	{ 0x80588, "g1_inmq_s" },
	{ 0x8058c, "g1_inmq_l" },
	{ 0x80590, "g2_inmq_s" },
	{ 0x80594, "g2_inmq_l" },
	{ 0x80598, "g3_inmq_s" },
	{ 0x8059c, "g3_inmq_l" },
	{ 0x805a0, "g4_inmq_s" },
	{ 0x805a4, "g4_inmq_l" },
	{ 0x805a8, "g5_inmq_s" },
	{ 0x805ac, "g5_inmq_l" },
	{ 0x805b0, "g6_inmq_s" },
	{ 0x805b4, "g6_inmq_l" },
	{ 0x805b8, "g7_inmq_s" },
	{ 0x805bc, "g7_inmq_l" },

	{ 0x80680, "tpdrq_b_h" },
	{ 0x80684, "tpdrq_t" },
	{ 0x80688, "tpdrq_s" },

	{ 0x8068c, "ubuff_ba" },

	{ 0x806c0, "rlbf0_h" },
	{ 0x806c4, "rlbf0_t" },
	{ 0x806c8, "rlbf1_h" },
	{ 0x806cc, "rlbf1_t" },
	{ 0x806d0, "rlbc_h" },
	{ 0x806d4, "rlbc_t" },
	{ 0x806d8, "rlbc_h2" },
	{ 0x806e0, "tlbf_h" },
	{ 0x806e4, "tlbf_t" },
	{ 0x806e8, "rlbf0_c" },
	{ 0x806ec, "rlbf1_c" },

	{ 0x806f0, "rxthrsh" },
	{ 0x806f4, "lithrsh" },

	{ 0x80700, "lbarb" },
	{ 0x80704, "sdramcon" },
	{ 0x80708, "lbstat" },
	{ 0x8070c, "rcc_stat" },

	{ 0x80740, "tcmconfig" },
	{ 0x80744, "tsrb_ba" },
	{ 0x80748, "tsrc_ba" },
	{ 0x8074c, "tmabr_ba" },
	{ 0x80750, "tpd_ba" },
	{ 0x80758, "tsrd_ba" },

	{ 0x80760, "tx_config" },
	{ 0x80764, "txaal5_proto" },

	{ 0x80780, "rcmconfig" },
	{ 0x80784, "rsrb_ba" },
	{ 0x80788, "rcmlbm_ba" },
	{ 0x8078c, "rcmabr_ba" },

	{ 0x807c0, "rc_config" },

	{ 0x807c4, "mcc" },
	{ 0x807c8, "oec" },
	{ 0x807cc, "dcc" },
	{ 0x807d0, "cec" },

	{ 0x807f4, "lb_config" },

	{ 0x807f0, "hsp_ba" },

	{ 0x807f8, "con_dat" },
	{ 0x807fc, "con_ctl" },

	{ 0x00000, NULL }
};

char *
reg_name(unsigned addr)
{
	struct reg_table *regp = pci_regs;

	while ( regp->name != NULL )
	{
		if (regp->addr == addr) return regp->name;
		regp++;
	}

	return NULL;
}

int
main(int argc,char **argv)
{
	int s;
	struct atmif_sioc sioc;
	struct he_ioctl_reg req;

	if (argc < 3)
	{
		fprintf(stderr,"usage: hediag itf [cmd]\n");
		fprintf(stderr,"\t\treadpci address\n");
		fprintf(stderr,"\t\treadrcm address\n");
		fprintf(stderr,"\t\treadtcm address\n");
		fprintf(stderr,"\t\treadmbox address\n");
		exit(1);
	}

	if (strcmp(argv[2], "readpci") == 0)
	{
		req.addr = strtol(argv[3], NULL, 0);
		req.type = HE_REGTYPE_PCI;
	}
	else if (strcmp(argv[2], "readrcm") == 0)
	{
		req.addr = strtol(argv[3], NULL, 0);
		req.type = HE_REGTYPE_RCM;
	}
	else if (strcmp(argv[2], "readtcm") == 0)
	{
		req.addr = strtol(argv[3], NULL, 0);
		req.type = HE_REGTYPE_TCM;
	}
	else if (strcmp(argv[2], "readmbox") == 0)
	{
		req.addr = strtol(argv[3], NULL, 0);
		req.type = HE_REGTYPE_MBOX;
	}
	

	if ((s = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5)) < 0)
	{
		perror("socket");
		exit(1);
	}

        sioc.number = atoi(argv[1]);
        sioc.arg = &req;
        sioc.length = sizeof(req);

	if (ioctl(s, HE_GET_REG, &sioc) < 0)
	{
		perror("ioctl HE_GET_REG");
		exit(1);
	}

	if (reg_name(req.addr))
		printf("%s = 0x%x\n", reg_name(req.addr), req.val);
	else
		printf("0x%x = 0x%x\n", req.addr, req.val);

	exit(0);
}
