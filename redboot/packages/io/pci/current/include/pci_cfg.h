#ifndef CYGONCE_PCI_CFG_H
#define CYGONCE_PCI_CFG_H
//=============================================================================
//
//      pci_cfg.h
//
//      PCI configuration definitions
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov
// Contributors: jskov
// Date:         1999-08-09
// Purpose:      Provides PCI configuration information
//               (common structure layout & defined vendor/class codes)
// Usage:
//              #include <cyg/io/pci_cfg.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

//----------------------------------------------------------------------
// PCI Configuration structure offsets
#define CYG_PCI_CFG_VENDOR                      0x00
#define CYG_PCI_CFG_DEVICE                      0x02
#define CYG_PCI_CFG_COMMAND                     0x04
#define CYG_PCI_CFG_STATUS                      0x06
#define CYG_PCI_CFG_CLASS_REV                   0x08
#define CYG_PCI_CFG_CACHE_LINE_SIZE             0x0c
#define CYG_PCI_CFG_LATENCY_TIMER               0x0d
#define CYG_PCI_CFG_HEADER_TYPE                 0x0e
#define CYG_PCI_CFG_BIST                        0x0f
#define CYG_PCI_CFG_BAR_BASE                    0x10
#define CYG_PCI_CFG_BAR_0                       0x10
#define CYG_PCI_CFG_BAR_1                       0x14
#define CYG_PCI_CFG_BAR_2                       0x18
#define CYG_PCI_CFG_BAR_3                       0x1c
#define CYG_PCI_CFG_BAR_4                       0x20
#define CYG_PCI_CFG_BAR_5                       0x24
#define CYG_PCI_CFG_CARDBUS_CIS                 0x28
#define CYG_PCI_CFG_SUB_VENDOR                  0x2c
#define CYG_PCI_CFG_SUB_ID                      0x2e
#define CYG_PCI_CFG_ROM_ADDRESS                 0x30
#define CYG_PCI_CFG_CAP_LIST                    0x34
#define CYG_PCI_CFG_INT_LINE                    0x3c
#define CYG_PCI_CFG_INT_PIN                     0x3d
#define CYG_PCI_CFG_MIN_GNT                     0x3e
#define CYG_PCI_CFG_MAX_LAT                     0x3f

// Bridge configuration offsets
#define CYG_PCI_CFG_PRI_BUS                     0x18
#define CYG_PCI_CFG_SEC_BUS                     0x19
#define CYG_PCI_CFG_SUB_BUS                     0x1a
#define CYG_PCI_CFG_SEC_LATENCY_TIMER           0x1b
#define CYG_PCI_CFG_IO_BASE                     0x1c
#define CYG_PCI_CFG_IO_LIMIT                    0x1d
#define CYG_PCI_CFG_SEC_STATUS                  0x1e
#define CYG_PCI_CFG_MEM_BASE                    0x20
#define CYG_PCI_CFG_MEM_LIMIT                   0x22
#define CYG_PCI_CFG_PREFETCH_BASE               0x24
#define CYG_PCI_CFG_PREFETCH_LIMIT              0x26
#define CYG_PCI_CFG_PREFETCH_BASE_UPPER32       0x28
#define CYG_PCI_CFG_PREFETCH_LIMIT_UPPER32      0x2c
#define CYG_PCI_CFG_IO_BASE_UPPER16             0x30
#define CYG_PCI_CFG_IO_LIMIT_UPPER16            0x32
#define CYG_PCI_CFG_BRIDGE_ROM_ADDRESS          0x38
#define CYG_PCI_CFG_BRIDGE_CONTROL              0x3e


//-----------------------------------------------------------------
// PCI Control bits

// Command
#define CYG_PCI_CFG_COMMAND_IO                    0x0001
#define CYG_PCI_CFG_COMMAND_MEMORY                0x0002
#define CYG_PCI_CFG_COMMAND_MASTER                0x0004
#define CYG_PCI_CFG_COMMAND_SPECIAL               0x0008
#define CYG_PCI_CFG_COMMAND_INVALIDATE            0x0010
#define CYG_PCI_CFG_COMMAND_VGA_SNOOP             0x0020
#define CYG_PCI_CFG_COMMAND_PARITY                0x0040
#define CYG_PCI_CFG_COMMAND_WAIT                  0x0080
#define CYG_PCI_CFG_COMMAND_SERR                  0x0100
#define CYG_PCI_CFG_COMMAND_FAST_BACK             0x0200

// Consider the device active if any of these bits are set.
#define CYG_PCI_CFG_COMMAND_ACTIVE \
    (CYG_PCI_CFG_COMMAND_IO        \
     |CYG_PCI_CFG_COMMAND_MEMORY   \
     |CYG_PCI_CFG_COMMAND_MASTER)

// Header type
#define CYG_PCI_CFG_HEADER_TYPE_MASK              0x7f
#define CYG_PCI_CFG_HEADER_TYPE_MF                0x80

// BAR
#define CYG_PCI_CFG_BAR_SPACE_MASK                0x00000001
#define CYG_PCI_CFG_BAR_SPACE_MEM                 0x00000000
#define CYG_PCI_CFG_BAR_SPACE_IO                  0x00000001

#define CYG_PRI_CFG_BAR_MEM_TYPE_MASK             0x00000006
#define CYG_PRI_CFG_BAR_MEM_TYPE_32               0x00000000
#define CYG_PRI_CFG_BAR_MEM_TYPE_1M               0x00000002
#define CYG_PRI_CFG_BAR_MEM_TYPE_64               0x00000004

#define CYG_PRI_CFG_BAR_MEM_PREFETCH              0x00000008

#define CYG_PRI_CFG_BAR_MEM_MASK                  0xfffffff0
#define CYG_PRI_CFG_BAR_IO_MASK                   0xfffffffc

// Bridge control
#define CYG_PCI_CFG_BRIDGE_CTL_PARITY		0x0001
#define CYG_PCI_CFG_BRIDGE_CTL_SERR		0x0002
#define CYG_PCI_CFG_BRIDGE_CTL_ISA		0x0004
#define CYG_PCI_CFG_BRIDGE_CTL_VGA		0x0008
#define CYG_PCI_CFG_BRIDGE_CTL_MASTER		0x0020
#define CYG_PCI_CFG_BRIDGE_CTL_RESET		0x0040
#define CYG_PCI_CFG_BRIDGE_CTL_FAST_BACK	0x0080

//----------------------------------------------------------------------
// PCI Vendor IDs
/***********************************************************************
   Can be extracted from the PCICODE List at http://www.yourvote.com/pci
   maintained by Jib Boemler. Use the below Perl script and fix any
   bad vendor names by hand.

#!/usr/bin/perl
while(<>) {
    if (/PCI_VENTABLE/) {
	%mvendors = {};
	@vendors = ();
	@codes = ();
	while(<>) {
	    # Collect vendor names and codes.
	    if (m!.*0x([0-9A-F]{4}), \"([^\"]*)\".*!) {
		$c = lc($1);
		$v = uc($2);
		$v =~ tr/-+ \/&.!/______ /;
		$v =~ s/__/_/g;
		$v =~ s/_$//;
		push @vendors, $v;
		push @codes, $c;
		# Count occurences of vendor name.
		$mvendors{$v} += 1;
	    }
	    last if (/;/);
	}
	# Ouput definitions.
	while ($#vendors >= 0) {
	    $v = shift @vendors;
	    $c = shift @codes;

	    $o = "#define CYG_PCI_VENDOR_$v";
	    # If a vendor name occurs more than once, append code
	    # to get unique definitions.
	    if ($mvendors{$v} > 1) {
		$o .= "_0x$c";
	    }
	    print $o . " " x (60-length($o)) . "0x$c\n";
	}
    }
}
***********************************************************************/

//---------------------- AUTO GENERATED START --------------------------
#define CYG_PCI_VENDOR_LOCKHEED_MARTIN                      0x003d
#define CYG_PCI_VENDOR_COMPAQ_0x0e11                        0x0e11
#define CYG_PCI_VENDOR_SYM                                  0x1000
#define CYG_PCI_VENDOR_ATI                                  0x1002
#define CYG_PCI_VENDOR_ULSI                                 0x1003
#define CYG_PCI_VENDOR_VLSI                                 0x1004
#define CYG_PCI_VENDOR_AVANCE_0x1005                        0x1005
#define CYG_PCI_VENDOR_REPLY                                0x1006
#define CYG_PCI_VENDOR_NETFRAME                             0x1007
#define CYG_PCI_VENDOR_EPSON                                0x1008
#define CYG_PCI_VENDOR_PHOENIX                              0x100a
#define CYG_PCI_VENDOR_NSC                                  0x100b
#define CYG_PCI_VENDOR_TSENG_0x100c                         0x100c
#define CYG_PCI_VENDOR_AST                                  0x100d
#define CYG_PCI_VENDOR_WEITEK                               0x100e
#define CYG_PCI_VENDOR_VLOGIC                               0x1010
#define CYG_PCI_VENDOR_DEC_0x1011                           0x1011
#define CYG_PCI_VENDOR_MICRONICS                            0x1012
#define CYG_PCI_VENDOR_CIRRUS                               0x1013
#define CYG_PCI_VENDOR_IBM                                  0x1014
#define CYG_PCI_VENDOR_LSIL                                 0x1015
#define CYG_PCI_VENDOR_FUJITSU_ICL                          0x1016
#define CYG_PCI_VENDOR_SPEA                                 0x1017
#define CYG_PCI_VENDOR_UNISYS                               0x1018
#define CYG_PCI_VENDOR_ECS                                  0x1019
#define CYG_PCI_VENDOR_NCR                                  0x101a
#define CYG_PCI_VENDOR_VITESSE                              0x101b
#define CYG_PCI_VENDOR_WD                                   0x101c
#define CYG_PCI_VENDOR_AMI                                  0x101e
#define CYG_PCI_VENDOR_PICTURETEL                           0x101f
#define CYG_PCI_VENDOR_HITACHI_0x1020                       0x1020
#define CYG_PCI_VENDOR_OKI                                  0x1021
#define CYG_PCI_VENDOR_AMD                                  0x1022
#define CYG_PCI_VENDOR_TRIDENT                              0x1023
#define CYG_PCI_VENDOR_ZENITH_0x1024                        0x1024
#define CYG_PCI_VENDOR_ACER                                 0x1025
#define CYG_PCI_VENDOR_DELL                                 0x1028
#define CYG_PCI_VENDOR_SIEM_NIX                             0x1029
#define CYG_PCI_VENDOR_LSI                                  0x102a
#define CYG_PCI_VENDOR_MATROX                               0x102b
#define CYG_PCI_VENDOR_C_T                                  0x102c
#define CYG_PCI_VENDOR_WYSE                                 0x102d
#define CYG_PCI_VENDOR_OLIVETTI                             0x102e
#define CYG_PCI_VENDOR_TOSHIBA_0x102f                       0x102f
#define CYG_PCI_VENDOR_TMC_0x1030                           0x1030
#define CYG_PCI_VENDOR_MIRO                                 0x1031
#define CYG_PCI_VENDOR_COMPAQ_0x1032                        0x1032
#define CYG_PCI_VENDOR_NEC_0x1033                           0x1033
#define CYG_PCI_VENDOR_BURNDY                               0x1034
#define CYG_PCI_VENDOR_C_CRL                                0x1035
#define CYG_PCI_VENDOR_FDOMAIN                              0x1036
#define CYG_PCI_VENDOR_HITACHI_0x1037                       0x1037
#define CYG_PCI_VENDOR_AMP                                  0x1038
#define CYG_PCI_VENDOR_SIS                                  0x1039
#define CYG_PCI_VENDOR_SEIKO                                0x103a
#define CYG_PCI_VENDOR_TATUNG                               0x103b
#define CYG_PCI_VENDOR_HP                                   0x103c
#define CYG_PCI_VENDOR_SOLLIDAY                             0x103e
#define CYG_PCI_VENDOR_LOGIC_MOD                            0x103f
#define CYG_PCI_VENDOR_KUBOTA                               0x1040
#define CYG_PCI_VENDOR_COMPUTREND                           0x1041
#define CYG_PCI_VENDOR_PC_TECH                              0x1042
#define CYG_PCI_VENDOR_ASUSTEK                              0x1043
#define CYG_PCI_VENDOR_DPT                                  0x1044
#define CYG_PCI_VENDOR_OPTI                                 0x1045
#define CYG_PCI_VENDOR_IPC                                  0x1046
#define CYG_PCI_VENDOR_GENOA                                0x1047
#define CYG_PCI_VENDOR_ELSA                                 0x1048
#define CYG_PCI_VENDOR_FOUNTAIN                             0x1049
#define CYG_PCI_VENDOR_STM                                  0x104a
#define CYG_PCI_VENDOR_BUSLOGOC                             0x104b
#define CYG_PCI_VENDOR_TI                                   0x104c
#define CYG_PCI_VENDOR_SONY                                 0x104d
#define CYG_PCI_VENDOR_OAK                                  0x104e
#define CYG_PCI_VENDOR_CO_TIME                              0x104f
#define CYG_PCI_VENDOR_WINBOND                              0x1050
#define CYG_PCI_VENDOR_ANIGMA                               0x1051
#define CYG_PCI_VENDOR_YOUNG                                0x1052
#define CYG_PCI_VENDOR_HITACHI_0x1054                       0x1054
#define CYG_PCI_VENDOR_EFAR                                 0x1055
#define CYG_PCI_VENDOR_ICL                                  0x1056
#define CYG_PCI_VENDOR_MOTOROLA_0x1057                      0x1057
#define CYG_PCI_VENDOR_E_TR                                 0x1058
#define CYG_PCI_VENDOR_TEKNOR                               0x1059
#define CYG_PCI_VENDOR_PROMISE                              0x105a
#define CYG_PCI_VENDOR_FOXCONN                              0x105b
#define CYG_PCI_VENDOR_WIPRO                                0x105c
#define CYG_PCI_VENDOR_NUMBER_NINE                          0x105d
#define CYG_PCI_VENDOR_VTECH                                0x105e
#define CYG_PCI_VENDOR_INFOTRONIC                           0x105f
#define CYG_PCI_VENDOR_UMC                                  0x1060
#define CYG_PCI_VENDOR_8X8                                  0x1061
#define CYG_PCI_VENDOR_MASPAR                               0x1062
#define CYG_PCI_VENDOR_OOA                                  0x1063
#define CYG_PCI_VENDOR_ALCATEL                              0x1064
#define CYG_PCI_VENDOR_TM                                   0x1065
#define CYG_PCI_VENDOR_PICOPOWER                            0x1066
#define CYG_PCI_VENDOR_MITSUBISHI_0x1067                    0x1067
#define CYG_PCI_VENDOR_DIV_TECH                             0x1068
#define CYG_PCI_VENDOR_MYLEX                                0x1069
#define CYG_PCI_VENDOR_ATEN                                 0x106a
#define CYG_PCI_VENDOR_APPLE                                0x106b
#define CYG_PCI_VENDOR_HYUNDAI                              0x106c
#define CYG_PCI_VENDOR_SEQUENT                              0x106d
#define CYG_PCI_VENDOR_DFI                                  0x106e
#define CYG_PCI_VENDOR_CITYGATE                             0x106f
#define CYG_PCI_VENDOR_DAEWOO                               0x1070
#define CYG_PCI_VENDOR_MITAC                                0x1071
#define CYG_PCI_VENDOR_GIT                                  0x1072
#define CYG_PCI_VENDOR_YAMAHA                               0x1073
#define CYG_PCI_VENDOR_NEXGEN                               0x1074
#define CYG_PCI_VENDOR_AIR                                  0x1075
#define CYG_PCI_VENDOR_CHAINTECH_0x1076                     0x1076
#define CYG_PCI_VENDOR_Q_LOGIC                              0x1077
#define CYG_PCI_VENDOR_CYRIX                                0x1078
#define CYG_PCI_VENDOR_I_BUS                                0x1079
#define CYG_PCI_VENDOR_NETWORTH                             0x107a
#define CYG_PCI_VENDOR_GATEWAY                              0x107b
#define CYG_PCI_VENDOR_GOLDSTAR                             0x107c
#define CYG_PCI_VENDOR_LEADTEK                              0x107d
#define CYG_PCI_VENDOR_INTERPHASE                           0x107e
#define CYG_PCI_VENDOR_DTC                                  0x107f
#define CYG_PCI_VENDOR_CONTAQ                               0x1080
#define CYG_PCI_VENDOR_SUPERMAC                             0x1081
#define CYG_PCI_VENDOR_EFA                                  0x1082
#define CYG_PCI_VENDOR_FOREX                                0x1083
#define CYG_PCI_VENDOR_PARADOR                              0x1084
#define CYG_PCI_VENDOR_TULIP                                0x1085
#define CYG_PCI_VENDOR_J_BOND                               0x1086
#define CYG_PCI_VENDOR_CACHE                                0x1087
#define CYG_PCI_VENDOR_MS_SON                               0x1088
#define CYG_PCI_VENDOR_DG                                   0x1089
#define CYG_PCI_VENDOR_BIT3                                 0x108a
#define CYG_PCI_VENDOR_ELONEX                               0x108c
#define CYG_PCI_VENDOR_OLICOM                               0x108d
#define CYG_PCI_VENDOR_SUN                                  0x108e
#define CYG_PCI_VENDOR_SYSTEMSOFT                           0x108f
#define CYG_PCI_VENDOR_ENCORE                               0x1090
#define CYG_PCI_VENDOR_INTERGRAPH                           0x1091
#define CYG_PCI_VENDOR_DIAMOND                              0x1092
#define CYG_PCI_VENDOR_NAT_INST                             0x1093
#define CYG_PCI_VENDOR_FIC                                  0x1094
#define CYG_PCI_VENDOR_CMD                                  0x1095
#define CYG_PCI_VENDOR_ALACRON                              0x1096
#define CYG_PCI_VENDOR_APPIAN                               0x1097
#define CYG_PCI_VENDOR_QUANTUM_0x1098                       0x1098
#define CYG_PCI_VENDOR_SAMSUNG_0x1099                       0x1099
#define CYG_PCI_VENDOR_PACKARD_BELL                         0x109a
#define CYG_PCI_VENDOR_GEMLIGHT                             0x109b
#define CYG_PCI_VENDOR_MEGACHIPS                            0x109c
#define CYG_PCI_VENDOR_ZIDA                                 0x109d
#define CYG_PCI_VENDOR_BROOKTREE                            0x109e
#define CYG_PCI_VENDOR_TRIGEM_0x109f                        0x109f
#define CYG_PCI_VENDOR_MEIDENSHA                            0x10a0
#define CYG_PCI_VENDOR_JUKO                                 0x10a1
#define CYG_PCI_VENDOR_QUANTUM_0x10a2                       0x10a2
#define CYG_PCI_VENDOR_EVEREX                               0x10a3
#define CYG_PCI_VENDOR_GLOBE                                0x10a4
#define CYG_PCI_VENDOR_RACAL                                0x10a5
#define CYG_PCI_VENDOR_INFORMTECH                           0x10a6
#define CYG_PCI_VENDOR_BENCHMARQ                            0x10a7
#define CYG_PCI_VENDOR_SIERRA_0x10a8                        0x10a8
#define CYG_PCI_VENDOR_SG                                   0x10a9
#define CYG_PCI_VENDOR_ACC                                  0x10aa
#define CYG_PCI_VENDOR_DIGICOM                              0x10ab
#define CYG_PCI_VENDOR_HONEYWELL                            0x10ac
#define CYG_PCI_VENDOR_SYMPHONY_0x10ad                      0x10ad
#define CYG_PCI_VENDOR_CORNERSTONE                          0x10ae
#define CYG_PCI_VENDOR_MCS_0x10af                           0x10af
#define CYG_PCI_VENDOR_CARDEXPERT                           0x10b0
#define CYG_PCI_VENDOR_CABLETRON                            0x10b1
#define CYG_PCI_VENDOR_RAYTHEON                             0x10b2
#define CYG_PCI_VENDOR_DATABOOK                             0x10b3
#define CYG_PCI_VENDOR_STB                                  0x10b4
#define CYG_PCI_VENDOR_PLX                                  0x10b5
#define CYG_PCI_VENDOR_MADGE                                0x10b6
#define CYG_PCI_VENDOR_3COM                                 0x10b7
#define CYG_PCI_VENDOR_STANDARD                             0x10b8
#define CYG_PCI_VENDOR_ALI                                  0x10b9
#define CYG_PCI_VENDOR_MITSUBISHI_0x10ba                    0x10ba
#define CYG_PCI_VENDOR_DAPHA                                0x10bb
#define CYG_PCI_VENDOR_ALR                                  0x10bc
#define CYG_PCI_VENDOR_SURECOM                              0x10bd
#define CYG_PCI_VENDOR_TSENG_0x10be                         0x10be
#define CYG_PCI_VENDOR_MOST                                 0x10bf
#define CYG_PCI_VENDOR_BOCA                                 0x10c0
#define CYG_PCI_VENDOR_ICM                                  0x10c1
#define CYG_PCI_VENDOR_AUSPEX                               0x10c2
#define CYG_PCI_VENDOR_SAMSUNG_0x10c3                       0x10c3
#define CYG_PCI_VENDOR_AWARD                                0x10c4
#define CYG_PCI_VENDOR_XEROX                                0x10c5
#define CYG_PCI_VENDOR_RAMBUS                               0x10c6
#define CYG_PCI_VENDOR_MEDIA_VISION                         0x10c7
#define CYG_PCI_VENDOR_NEOMAGIC                             0x10c8
#define CYG_PCI_VENDOR_DATAEXPERT                           0x10c9
#define CYG_PCI_VENDOR_FUJITSU_0x10ca                       0x10ca
#define CYG_PCI_VENDOR_OMRON                                0x10cb
#define CYG_PCI_VENDOR_MENTOR                               0x10cc
#define CYG_PCI_VENDOR_ADVANSYS                             0x10cd
#define CYG_PCI_VENDOR_RADIUS                               0x10ce
#define CYG_PCI_VENDOR_TTI                                  0x10cf
#define CYG_PCI_VENDOR_FUJITSU_0x10d0                       0x10d0
#define CYG_PCI_VENDOR_FUTURE                               0x10d1
#define CYG_PCI_VENDOR_MOLEX                                0x10d2
#define CYG_PCI_VENDOR_JABIL                                0x10d3
#define CYG_PCI_VENDOR_HUALON                               0x10d4
#define CYG_PCI_VENDOR_AUTOLOGIC                            0x10d5
#define CYG_PCI_VENDOR_CETIA                                0x10d6
#define CYG_PCI_VENDOR_BCM                                  0x10d7
#define CYG_PCI_VENDOR_APL                                  0x10d8
#define CYG_PCI_VENDOR_MACRONIX                             0x10d9
#define CYG_PCI_VENDOR_T_C                                  0x10da
#define CYG_PCI_VENDOR_ROHM                                 0x10db
#define CYG_PCI_VENDOR_CERN                                 0x10dc
#define CYG_PCI_VENDOR_E_S                                  0x10dd
#define CYG_PCI_VENDOR_NVIDIA_0x10de                        0x10de
#define CYG_PCI_VENDOR_EMULEX                               0x10df
#define CYG_PCI_VENDOR_IMS                                  0x10e0
#define CYG_PCI_VENDOR_TEKRAM_0x10e1                        0x10e1
#define CYG_PCI_VENDOR_APTIX                                0x10e2
#define CYG_PCI_VENDOR_TUNDRA                               0x10e3
#define CYG_PCI_VENDOR_TANDEM                               0x10e4
#define CYG_PCI_VENDOR_MIC                                  0x10e5
#define CYG_PCI_VENDOR_GAINBERY                             0x10e6
#define CYG_PCI_VENDOR_VADEM                                0x10e7
#define CYG_PCI_VENDOR_AMCC                                 0x10e8
#define CYG_PCI_VENDOR_ALPS                                 0x10e9
#define CYG_PCI_VENDOR_INTERGRAPHICS                        0x10ea
#define CYG_PCI_VENDOR_ARTIST                               0x10eb
#define CYG_PCI_VENDOR_REALTEK                              0x10ec
#define CYG_PCI_VENDOR_ASCII                                0x10ed
#define CYG_PCI_VENDOR_XILINX                               0x10ee
#define CYG_PCI_VENDOR_RACORE                               0x10ef
#define CYG_PCI_VENDOR_PERITEK                              0x10f0
#define CYG_PCI_VENDOR_TYAN                                 0x10f1
#define CYG_PCI_VENDOR_ACHME                                0x10f2
#define CYG_PCI_VENDOR_ALARIS                               0x10f3
#define CYG_PCI_VENDOR_S_MOS                                0x10f4
#define CYG_PCI_VENDOR_NKK                                  0x10f5
#define CYG_PCI_VENDOR_CREATIVE                             0x10f6
#define CYG_PCI_VENDOR_MATSUSHITA_0x10f7                    0x10f7
#define CYG_PCI_VENDOR_ALTOS                                0x10f8
#define CYG_PCI_VENDOR_PC_DIRECT                            0x10f9
#define CYG_PCI_VENDOR_TRUEVISION                           0x10fa
#define CYG_PCI_VENDOR_THESYS                               0x10fb
#define CYG_PCI_VENDOR_I_O                                  0x10fc
#define CYG_PCI_VENDOR_SOYO                                 0x10fd
#define CYG_PCI_VENDOR_FAST                                 0x10fe
#define CYG_PCI_VENDOR_NCUBE                                0x10ff
#define CYG_PCI_VENDOR_JAZZ                                 0x1100
#define CYG_PCI_VENDOR_INITIO                               0x1101
#define CYG_PCI_VENDOR_CREATIVE_LABS                        0x1102
#define CYG_PCI_VENDOR_TRIONES                              0x1103
#define CYG_PCI_VENDOR_RASTEROPS                            0x1104
#define CYG_PCI_VENDOR_SIGMA_0x1105                         0x1105
#define CYG_PCI_VENDOR_VIA                                  0x1106
#define CYG_PCI_VENDOR_STRATUS                              0x1107
#define CYG_PCI_VENDOR_PROTEON                              0x1108
#define CYG_PCI_VENDOR_COGENT                               0x1109
#define CYG_PCI_VENDOR_SIEMENS                              0x110a
#define CYG_PCI_VENDOR_CHROMATIC                            0x110b
#define CYG_PCI_VENDOR_MINI_MAX                             0x110c
#define CYG_PCI_VENDOR_ZNYX                                 0x110d
#define CYG_PCI_VENDOR_CPU_TECH                             0x110e
#define CYG_PCI_VENDOR_ROSS                                 0x110f
#define CYG_PCI_VENDOR_POWERHOUSE                           0x1110
#define CYG_PCI_VENDOR_SCO                                  0x1111
#define CYG_PCI_VENDOR_OSICOM                               0x1112
#define CYG_PCI_VENDOR_ACCTON                               0x1113
#define CYG_PCI_VENDOR_ATMEL                                0x1114
#define CYG_PCI_VENDOR_DUPONT                               0x1115
#define CYG_PCI_VENDOR_DATA_TRANS                           0x1116
#define CYG_PCI_VENDOR_DATACUBE                             0x1117
#define CYG_PCI_VENDOR_BERG                                 0x1118
#define CYG_PCI_VENDOR_VORTEX                               0x1119
#define CYG_PCI_VENDOR_EFF_NET                              0x111a
#define CYG_PCI_VENDOR_TELEDYNE                             0x111b
#define CYG_PCI_VENDOR_TRICORD                              0x111c
#define CYG_PCI_VENDOR_IDT                                  0x111d
#define CYG_PCI_VENDOR_ELDEC                                0x111e
#define CYG_PCI_VENDOR_PDI                                  0x111f
#define CYG_PCI_VENDOR_EMC                                  0x1120
#define CYG_PCI_VENDOR_ZILOG                                0x1121
#define CYG_PCI_VENDOR_MULTI_TECH                           0x1122
#define CYG_PCI_VENDOR_EDI                                  0x1123
#define CYG_PCI_VENDOR_LEUTRON                              0x1124
#define CYG_PCI_VENDOR_EUROCORE                             0x1125
#define CYG_PCI_VENDOR_VIGRA                                0x1126
#define CYG_PCI_VENDOR_FORE                                 0x1127
#define CYG_PCI_VENDOR_FIRMWORKS                            0x1129
#define CYG_PCI_VENDOR_HERMES                               0x112a
#define CYG_PCI_VENDOR_LINOTYPE                             0x112b
#define CYG_PCI_VENDOR_ZENITH_0x112c                        0x112c
#define CYG_PCI_VENDOR_RAVICAD                              0x112d
#define CYG_PCI_VENDOR_INFOMEDIA                            0x112e
#define CYG_PCI_VENDOR_IMAGTECH                             0x112f
#define CYG_PCI_VENDOR_COMPUTERVISION                       0x1130
#define CYG_PCI_VENDOR_PHILIPS                              0x1131
#define CYG_PCI_VENDOR_MITEL                                0x1132
#define CYG_PCI_VENDOR_EIC                                  0x1133
#define CYG_PCI_VENDOR_MCS_0x1134                           0x1134
#define CYG_PCI_VENDOR_FUJI                                 0x1135
#define CYG_PCI_VENDOR_MOMENTUM                             0x1136
#define CYG_PCI_VENDOR_CISCO                                0x1137
#define CYG_PCI_VENDOR_ZIATECH                              0x1138
#define CYG_PCI_VENDOR_DYN_PICT                             0x1139
#define CYG_PCI_VENDOR_FWB                                  0x113a
#define CYG_PCI_VENDOR_NCD                                  0x113b
#define CYG_PCI_VENDOR_CYCLONE                              0x113c
#define CYG_PCI_VENDOR_LEADING_EDGE                         0x113d
#define CYG_PCI_VENDOR_SANYO                                0x113e
#define CYG_PCI_VENDOR_EQUINOX                              0x113f
#define CYG_PCI_VENDOR_INTERVOICE                           0x1140
#define CYG_PCI_VENDOR_CREST                                0x1141
#define CYG_PCI_VENDOR_ALLIANCE                             0x1142
#define CYG_PCI_VENDOR_NETPOWER_0x1143                      0x1143
#define CYG_PCI_VENDOR_CINN_MIL                             0x1144
#define CYG_PCI_VENDOR_WORKBIT                              0x1145
#define CYG_PCI_VENDOR_FORCE                                0x1146
#define CYG_PCI_VENDOR_INTERFACE                            0x1147
#define CYG_PCI_VENDOR_S_K                                  0x1148
#define CYG_PCI_VENDOR_WIN_SYSTEM                           0x1149
#define CYG_PCI_VENDOR_VMIC                                 0x114a
#define CYG_PCI_VENDOR_CANOPUS                              0x114b
#define CYG_PCI_VENDOR_ANNABOOKS                            0x114c
#define CYG_PCI_VENDOR_IC_CORP                              0x114d
#define CYG_PCI_VENDOR_NIKON                                0x114e
#define CYG_PCI_VENDOR_STARGATE                             0x114f
#define CYG_PCI_VENDOR_TMC_0x1150                           0x1150
#define CYG_PCI_VENDOR_JAE                                  0x1151
#define CYG_PCI_VENDOR_MEGATEK                              0x1152
#define CYG_PCI_VENDOR_LAND_WIN                             0x1153
#define CYG_PCI_VENDOR_MELCO                                0x1154
#define CYG_PCI_VENDOR_PINE                                 0x1155
#define CYG_PCI_VENDOR_PERISCOPE                            0x1156
#define CYG_PCI_VENDOR_AVSYS                                0x1157
#define CYG_PCI_VENDOR_VOARX                                0x1158
#define CYG_PCI_VENDOR_MUTECH                               0x1159
#define CYG_PCI_VENDOR_HARLEQUIN                            0x115a
#define CYG_PCI_VENDOR_PARALLAX                             0x115b
#define CYG_PCI_VENDOR_PHOTRON                              0x115c
#define CYG_PCI_VENDOR_XIRCOM                               0x115d
#define CYG_PCI_VENDOR_PEER                                 0x115e
#define CYG_PCI_VENDOR_MAXTOR                               0x115f
#define CYG_PCI_VENDOR_MEGASOFT                             0x1160
#define CYG_PCI_VENDOR_PFU                                  0x1161
#define CYG_PCI_VENDOR_OA_LAB                               0x1162
#define CYG_PCI_VENDOR_RENDITION                            0x1163
#define CYG_PCI_VENDOR_APT                                  0x1164
#define CYG_PCI_VENDOR_IMAGRAPH                             0x1165
#define CYG_PCI_VENDOR_PEQUR                                0x1166
#define CYG_PCI_VENDOR_MUTOH                                0x1167
#define CYG_PCI_VENDOR_THINE                                0x1168
#define CYG_PCI_VENDOR_CDAC                                 0x1169
#define CYG_PCI_VENDOR_POLARIS                              0x116a
#define CYG_PCI_VENDOR_CONNECTWARE                          0x116b
#define CYG_PCI_VENDOR_INT_RES                              0x116c
#define CYG_PCI_VENDOR_EFI                                  0x116e
#define CYG_PCI_VENDOR_WKSTA_TECH                           0x116f
#define CYG_PCI_VENDOR_INVENTEC                             0x1170
#define CYG_PCI_VENDOR_LOUGH_SOUND                          0x1171
#define CYG_PCI_VENDOR_ALTERA                               0x1172
#define CYG_PCI_VENDOR_ADOBE                                0x1173
#define CYG_PCI_VENDOR_BRIDGEPORT                           0x1174
#define CYG_PCI_VENDOR_MITRON                               0x1175
#define CYG_PCI_VENDOR_SBE                                  0x1176
#define CYG_PCI_VENDOR_SILICON_ENG                          0x1177
#define CYG_PCI_VENDOR_ALFA                                 0x1178
#define CYG_PCI_VENDOR_TOSHIBA_0x1179                       0x1179
#define CYG_PCI_VENDOR_A_TREND                              0x117a
#define CYG_PCI_VENDOR_LG_ELEC                              0x117b
#define CYG_PCI_VENDOR_ATTO                                 0x117c
#define CYG_PCI_VENDOR_B_D                                  0x117d
#define CYG_PCI_VENDOR_T_R                                  0x117e
#define CYG_PCI_VENDOR_ICS                                  0x117f
#define CYG_PCI_VENDOR_RICOH                                0x1180
#define CYG_PCI_VENDOR_TELMATICS                            0x1181
#define CYG_PCI_VENDOR_FUJIKURA                             0x1183
#define CYG_PCI_VENDOR_FORKS_0x1184                         0x1184
#define CYG_PCI_VENDOR_DATAWORLD                            0x1185
#define CYG_PCI_VENDOR_D_LINK                               0x1186
#define CYG_PCI_VENDOR_ATL                                  0x1187
#define CYG_PCI_VENDOR_SHIMA                                0x1188
#define CYG_PCI_VENDOR_MATSUSHITA_0x1189                    0x1189
#define CYG_PCI_VENDOR_HILEVEL                              0x118a
#define CYG_PCI_VENDOR_HYPERTEC                             0x118b
#define CYG_PCI_VENDOR_COROLLARY                            0x118c
#define CYG_PCI_VENDOR_BITFLOW                              0x118d
#define CYG_PCI_VENDOR_HERMSTEDT                            0x118e
#define CYG_PCI_VENDOR_GREEN                                0x118f
#define CYG_PCI_VENDOR_ARTOP                                0x1191
#define CYG_PCI_VENDOR_DENSAN                               0x1192
#define CYG_PCI_VENDOR_ZEITNET                              0x1193
#define CYG_PCI_VENDOR_TOUCAN                               0x1194
#define CYG_PCI_VENDOR_RATOC                                0x1195
#define CYG_PCI_VENDOR_HYTEC                                0x1196
#define CYG_PCI_VENDOR_GAGE                                 0x1197
#define CYG_PCI_VENDOR_LAMBDA                               0x1198
#define CYG_PCI_VENDOR_ATTACHMATE                           0x1199
#define CYG_PCI_VENDOR_MIND_SHARE                           0x119a
#define CYG_PCI_VENDOR_OMEGA                                0x119b
#define CYG_PCI_VENDOR_ITI                                  0x119c
#define CYG_PCI_VENDOR_BUG                                  0x119d
#define CYG_PCI_VENDOR_FUJITSU_0x119e                       0x119e
#define CYG_PCI_VENDOR_BULL                                 0x119f
#define CYG_PCI_VENDOR_CONVEX                               0x11a0
#define CYG_PCI_VENDOR_HAMAMATSU                            0x11a1
#define CYG_PCI_VENDOR_SIERRA_0x11a2                        0x11a2
#define CYG_PCI_VENDOR_DEURETZBACHER                        0x11a3
#define CYG_PCI_VENDOR_BARCO                                0x11a4
#define CYG_PCI_VENDOR_MICROUNITY                           0x11a5
#define CYG_PCI_VENDOR_PURE_DATA                            0x11a6
#define CYG_PCI_VENDOR_POWER_COMP                           0x11a7
#define CYG_PCI_VENDOR_SYSTECH                              0x11a8
#define CYG_PCI_VENDOR_INNOSYS                              0x11a9
#define CYG_PCI_VENDOR_ACTEL                                0x11aa
#define CYG_PCI_VENDOR_GALILEO                              0x11ab
#define CYG_PCI_VENDOR_CANON                                0x11ac
#define CYG_PCI_VENDOR_LITE_ON                              0x11ad
#define CYG_PCI_VENDOR_SCITEX                               0x11ae
#define CYG_PCI_VENDOR_PRO_LOG                              0x11af
#define CYG_PCI_VENDOR_V3                                   0x11b0
#define CYG_PCI_VENDOR_APRICOT                              0x11b1
#define CYG_PCI_VENDOR_KODAK                                0x11b2
#define CYG_PCI_VENDOR_BARR                                 0x11b3
#define CYG_PCI_VENDOR_LEITCH                               0x11b4
#define CYG_PCI_VENDOR_RADSTONE                             0x11b5
#define CYG_PCI_VENDOR_UNITED_VIDEO                         0x11b6
#define CYG_PCI_VENDOR_MOTOROLA_0x11b7                      0x11b7
#define CYG_PCI_VENDOR_XPOINT                               0x11b8
#define CYG_PCI_VENDOR_PATHLIGHT                            0x11b9
#define CYG_PCI_VENDOR_VIDEOTRON                            0x11ba
#define CYG_PCI_VENDOR_PYRAMID                              0x11bb
#define CYG_PCI_VENDOR_NET_PERIPH                           0x11bc
#define CYG_PCI_VENDOR_PINNACLE                             0x11bd
#define CYG_PCI_VENDOR_IMI                                  0x11be
#define CYG_PCI_VENDOR_ASTRODESIGN                          0x11bf
#define CYG_PCI_VENDOR_H_P                                  0x11c0
#define CYG_PCI_VENDOR_AT_T                                 0x11c1
#define CYG_PCI_VENDOR_SAND                                 0x11c2
#define CYG_PCI_VENDOR_NEC_0x11c3                           0x11c3
#define CYG_PCI_VENDOR_DOC_TECH                             0x11c4
#define CYG_PCI_VENDOR_SHIVA                                0x11c5
#define CYG_PCI_VENDOR_DAINIPPON                            0x11c6
#define CYG_PCI_VENDOR_D_C_M                                0x11c7
#define CYG_PCI_VENDOR_DOLPHIN                              0x11c8
#define CYG_PCI_VENDOR_MAGMA                                0x11c9
#define CYG_PCI_VENDOR_LSI_SYS                              0x11ca
#define CYG_PCI_VENDOR_SPECIALIX                            0x11cb
#define CYG_PCI_VENDOR_M_K                                  0x11cc
#define CYG_PCI_VENDOR_HAL                                  0x11cd
#define CYG_PCI_VENDOR_PRI                                  0x11ce
#define CYG_PCI_VENDOR_PEC                                  0x11cf
#define CYG_PCI_VENDOR_LORAL                                0x11d0
#define CYG_PCI_VENDOR_AURAVISION                           0x11d1
#define CYG_PCI_VENDOR_INTERCOM                             0x11d2
#define CYG_PCI_VENDOR_TRANCELL                             0x11d3
#define CYG_PCI_VENDOR_AD                                   0x11d4
#define CYG_PCI_VENDOR_IKON                                 0x11d5
#define CYG_PCI_VENDOR_TEKELEC                              0x11d6
#define CYG_PCI_VENDOR_TRENTON                              0x11d7
#define CYG_PCI_VENDOR_ITD                                  0x11d8
#define CYG_PCI_VENDOR_TEC                                  0x11d9
#define CYG_PCI_VENDOR_NOVELL                               0x11da
#define CYG_PCI_VENDOR_SEGA                                 0x11db
#define CYG_PCI_VENDOR_QUESTRA                              0x11dc
#define CYG_PCI_VENDOR_CROSFIELD                            0x11dd
#define CYG_PCI_VENDOR_ZORAN                                0x11de
#define CYG_PCI_VENDOR_NEW_WAVE                             0x11df
#define CYG_PCI_VENDOR_CRAY                                 0x11e0
#define CYG_PCI_VENDOR_GEC_PLESSEY                          0x11e1
#define CYG_PCI_VENDOR_SAMSUNG_0x11e2                       0x11e2
#define CYG_PCI_VENDOR_QUICKLOGIC                           0x11e3
#define CYG_PCI_VENDOR_SECOND_WAVE                          0x11e4
#define CYG_PCI_VENDOR_IIX                                  0x11e5
#define CYG_PCI_VENDOR_MITSUI                               0x11e6
#define CYG_PCI_VENDOR_TOSHIBA_0x11e7                       0x11e7
#define CYG_PCI_VENDOR_DPSI                                 0x11e8
#define CYG_PCI_VENDOR_HIGHWATER                            0x11e9
#define CYG_PCI_VENDOR_ELSAG                                0x11ea
#define CYG_PCI_VENDOR_FORMATION                            0x11eb
#define CYG_PCI_VENDOR_CORECO                               0x11ec
#define CYG_PCI_VENDOR_MEDIAMATICS                          0x11ed
#define CYG_PCI_VENDOR_DOME                                 0x11ee
#define CYG_PCI_VENDOR_NICOLET                              0x11ef
#define CYG_PCI_VENDOR_COMPU_SHACK                          0x11f0
#define CYG_PCI_VENDOR_SYMBIOS                              0x11f1
#define CYG_PCI_VENDOR_PIC_TEL                              0x11f2
#define CYG_PCI_VENDOR_KEITHLEY                             0x11f3
#define CYG_PCI_VENDOR_KINETIC                              0x11f4
#define CYG_PCI_VENDOR_COMP_DEV                             0x11f5
#define CYG_PCI_VENDOR_POWERMATIC                           0x11f6
#define CYG_PCI_VENDOR_S_A                                  0x11f7
#define CYG_PCI_VENDOR_PMC_SIERRA                           0x11f8
#define CYG_PCI_VENDOR_I_CUBE                               0x11f9
#define CYG_PCI_VENDOR_KASAN                                0x11fa
#define CYG_PCI_VENDOR_DATEL                                0x11fb
#define CYG_PCI_VENDOR_SILICON_MAGIC                        0x11fc
#define CYG_PCI_VENDOR_HIGH_STREET                          0x11fd
#define CYG_PCI_VENDOR_COMTROL                              0x11fe
#define CYG_PCI_VENDOR_SCION                                0x11ff
#define CYG_PCI_VENDOR_CSS                                  0x1200
#define CYG_PCI_VENDOR_VISTA                                0x1201
#define CYG_PCI_VENDOR_NETWORK_GEN                          0x1202
#define CYG_PCI_VENDOR_AGFA                                 0x1203
#define CYG_PCI_VENDOR_LATTICE                              0x1204
#define CYG_PCI_VENDOR_ARRAY                                0x1205
#define CYG_PCI_VENDOR_AMDAHL                               0x1206
#define CYG_PCI_VENDOR_PARSYTEC                             0x1208
#define CYG_PCI_VENDOR_SCI_SYS                              0x1209
#define CYG_PCI_VENDOR_SYNAPTEL                             0x120a
#define CYG_PCI_VENDOR_ADAPTIVE                             0x120b
#define CYG_PCI_VENDOR_COMP_LABS                            0x120d
#define CYG_PCI_VENDOR_CYCLADES                             0x120e
#define CYG_PCI_VENDOR_ESSENTIAL                            0x120f
#define CYG_PCI_VENDOR_HYPERPARALLEL                        0x1210
#define CYG_PCI_VENDOR_BRAINTECH                            0x1211
#define CYG_PCI_VENDOR_KINGSTON                             0x1212
#define CYG_PCI_VENDOR_AISI                                 0x1213
#define CYG_PCI_VENDOR_PERF_TECH                            0x1214
#define CYG_PCI_VENDOR_INTERWARE                            0x1215
#define CYG_PCI_VENDOR_PURUP                                0x1216
#define CYG_PCI_VENDOR_O2MICRO                              0x1217
#define CYG_PCI_VENDOR_HYBRICON                             0x1218
#define CYG_PCI_VENDOR_FIRST_VIRTUAL                        0x1219
#define CYG_PCI_VENDOR_3DFX                                 0x121a
#define CYG_PCI_VENDOR_ATM                                  0x121b
#define CYG_PCI_VENDOR_NIPPON_TEXA                          0x121c
#define CYG_PCI_VENDOR_LIPPERT                              0x121d
#define CYG_PCI_VENDOR_CSPI                                 0x121e
#define CYG_PCI_VENDOR_ARCUS                                0x121f
#define CYG_PCI_VENDOR_ARIEL                                0x1220
#define CYG_PCI_VENDOR_CONTEC                               0x1221
#define CYG_PCI_VENDOR_ANCOR                                0x1222
#define CYG_PCI_VENDOR_HEURIKON                             0x1223
#define CYG_PCI_VENDOR_INT_IMG                              0x1224
#define CYG_PCI_VENDOR_POWER_IO                             0x1225
#define CYG_PCI_VENDOR_TECH_SOURCE                          0x1227
#define CYG_PCI_VENDOR_NORSK                                0x1228
#define CYG_PCI_VENDOR_DATA_KIN                             0x1229
#define CYG_PCI_VENDOR_INT_TELECOM                          0x122a
#define CYG_PCI_VENDOR_LG_IND                               0x122b
#define CYG_PCI_VENDOR_SICAN                                0x122c
#define CYG_PCI_VENDOR_AZTECH                               0x122d
#define CYG_PCI_VENDOR_XYRATEX                              0x122e
#define CYG_PCI_VENDOR_ANDREW                               0x122f
#define CYG_PCI_VENDOR_FISHCAMP                             0x1230
#define CYG_PCI_VENDOR_W_MCCOACH                            0x1231
#define CYG_PCI_VENDOR_GPT                                  0x1232
#define CYG_PCI_VENDOR_BUS_TECH                             0x1233
#define CYG_PCI_VENDOR_TECHNICAL                            0x1234
#define CYG_PCI_VENDOR_RISQ_MOD                             0x1235
#define CYG_PCI_VENDOR_SIGMA_0x1236                         0x1236
#define CYG_PCI_VENDOR_ALTA_TECH                            0x1237
#define CYG_PCI_VENDOR_ADTRAN                               0x1238
#define CYG_PCI_VENDOR_3DO                                  0x1239
#define CYG_PCI_VENDOR_VISICOM                              0x123a
#define CYG_PCI_VENDOR_SEEQ                                 0x123b
#define CYG_PCI_VENDOR_CENTURY_SYS                          0x123c
#define CYG_PCI_VENDOR_EDT                                  0x123d
#define CYG_PCI_VENDOR_C_CUBE                               0x123f
#define CYG_PCI_VENDOR_MARATHON                             0x1240
#define CYG_PCI_VENDOR_DSC                                  0x1241
#define CYG_PCI_VENDOR_DELPHAX                              0x1243
#define CYG_PCI_VENDOR_AVM                                  0x1244
#define CYG_PCI_VENDOR_APD                                  0x1245
#define CYG_PCI_VENDOR_DIPIX                                0x1246
#define CYG_PCI_VENDOR_XYLON                                0x1247
#define CYG_PCI_VENDOR_CENTRAL_DATA                         0x1248
#define CYG_PCI_VENDOR_SAMSUNG_0x1249                       0x1249
#define CYG_PCI_VENDOR_AEG                                  0x124a
#define CYG_PCI_VENDOR_GREENSPRING                          0x124b
#define CYG_PCI_VENDOR_SOLITRON                             0x124c
#define CYG_PCI_VENDOR_STALLION                             0x124d
#define CYG_PCI_VENDOR_CYLINK                               0x124e
#define CYG_PCI_VENDOR_INFORTREND                           0x124f
#define CYG_PCI_VENDOR_HITACHI_0x1250                       0x1250
#define CYG_PCI_VENDOR_VLSI_SOL                             0x1251
#define CYG_PCI_VENDOR_GUZIK                                0x1253
#define CYG_PCI_VENDOR_LINEAR_SYSTEMS                       0x1254
#define CYG_PCI_VENDOR_OPTIBASE                             0x1255
#define CYG_PCI_VENDOR_PERCEPTIVE                           0x1256
#define CYG_PCI_VENDOR_VERTEX                               0x1257
#define CYG_PCI_VENDOR_GILBARCO                             0x1258
#define CYG_PCI_VENDOR_ALLIED_TSYN                          0x1259
#define CYG_PCI_VENDOR_ABB_PWR                              0x125a
#define CYG_PCI_VENDOR_ASIX                                 0x125b
#define CYG_PCI_VENDOR_AURORA                               0x125c
#define CYG_PCI_VENDOR_ESS                                  0x125d
#define CYG_PCI_VENDOR_SPECVIDEO                            0x125e
#define CYG_PCI_VENDOR_CONCURRENT                           0x125f
#define CYG_PCI_VENDOR_HARRIS                               0x1260
#define CYG_PCI_VENDOR_MATSUSHITA_0x1261                    0x1261
#define CYG_PCI_VENDOR_ES_COMP                              0x1262
#define CYG_PCI_VENDOR_SONIC_SOL                            0x1263
#define CYG_PCI_VENDOR_AVAL_NAG                             0x1264
#define CYG_PCI_VENDOR_CASIO                                0x1265
#define CYG_PCI_VENDOR_MICRODYNE                            0x1266
#define CYG_PCI_VENDOR_SA_TELECOM                           0x1267
#define CYG_PCI_VENDOR_TEKTRONIX                            0x1268
#define CYG_PCI_VENDOR_THOMSON_CSF                          0x1269
#define CYG_PCI_VENDOR_LEXMARK                              0x126a
#define CYG_PCI_VENDOR_ADAX                                 0x126b
#define CYG_PCI_VENDOR_NORTEL                               0x126c
#define CYG_PCI_VENDOR_SPLASH                               0x126d
#define CYG_PCI_VENDOR_SUMITOMO                             0x126e
#define CYG_PCI_VENDOR_SIL_MOTION                           0x126f
#define CYG_PCI_VENDOR_OLYMPUS                              0x1270
#define CYG_PCI_VENDOR_GW_INSTR                             0x1271
#define CYG_PCI_VENDOR_TELEMATICS                           0x1272
#define CYG_PCI_VENDOR_HUGHES                               0x1273
#define CYG_PCI_VENDOR_ENSONIQ                              0x1274
#define CYG_PCI_VENDOR_NETAPP                               0x1275
#define CYG_PCI_VENDOR_SW_NET_TECH                          0x1276
#define CYG_PCI_VENDOR_COMSTREAM                            0x1277
#define CYG_PCI_VENDOR_TRANSTECH                            0x1278
#define CYG_PCI_VENDOR_TRANSMETA                            0x1279
#define CYG_PCI_VENDOR_ROCKWELL_SEMI                        0x127a
#define CYG_PCI_VENDOR_PIXERA                               0x127b
#define CYG_PCI_VENDOR_CROSSPOINT                           0x127c
#define CYG_PCI_VENDOR_VELA_RES                             0x127d
#define CYG_PCI_VENDOR_WINNOW                               0x127e
#define CYG_PCI_VENDOR_FUJIFILM_0x127f                      0x127f
#define CYG_PCI_VENDOR_PHOTOSCRIPT                          0x1280
#define CYG_PCI_VENDOR_YOKOGAWA                             0x1281
#define CYG_PCI_VENDOR_DAVICOM                              0x1282
#define CYG_PCI_VENDOR_ITEXPRESS                            0x1283
#define CYG_PCI_VENDOR_SAHARA                               0x1284
#define CYG_PCI_VENDOR_PLAT_TECH                            0x1285
#define CYG_PCI_VENDOR_MAZET                                0x1286
#define CYG_PCI_VENDOR_LUXSONOR                             0x1287
#define CYG_PCI_VENDOR_TIMESTEP                             0x1288
#define CYG_PCI_VENDOR_AVC_TECH                             0x1289
#define CYG_PCI_VENDOR_ASANTE                               0x128a
#define CYG_PCI_VENDOR_TRANSWITCH                           0x128b
#define CYG_PCI_VENDOR_RETIX                                0x128c
#define CYG_PCI_VENDOR_G2_NET                               0x128d
#define CYG_PCI_VENDOR_SAMHO                                0x128e
#define CYG_PCI_VENDOR_TATENO                               0x128f
#define CYG_PCI_VENDOR_SORD                                 0x1290
#define CYG_PCI_VENDOR_NCS_COMP                             0x1291
#define CYG_PCI_VENDOR_TRITECH                              0x1292
#define CYG_PCI_VENDOR_M_REALITY                            0x1293
#define CYG_PCI_VENDOR_RHETOREX                             0x1294
#define CYG_PCI_VENDOR_IMAGENATION                          0x1295
#define CYG_PCI_VENDOR_KOFAX                                0x1296
#define CYG_PCI_VENDOR_HOLCO                                0x1297
#define CYG_PCI_VENDOR_SPELLCASTER                          0x1298
#define CYG_PCI_VENDOR_KNOW_TECH                            0x1299
#define CYG_PCI_VENDOR_VMETRO                               0x129a
#define CYG_PCI_VENDOR_IMG_ACCESS                           0x129b
#define CYG_PCI_VENDOR_COMPCORE                             0x129d
#define CYG_PCI_VENDOR_VICTOR_JPN                           0x129e
#define CYG_PCI_VENDOR_OEC_MED                              0x129f
#define CYG_PCI_VENDOR_A_B                                  0x12a0
#define CYG_PCI_VENDOR_SIMPACT                              0x12a1
#define CYG_PCI_VENDOR_NEWGEN                               0x12a2
#define CYG_PCI_VENDOR_LUCENT                               0x12a3
#define CYG_PCI_VENDOR_NTT_ELECT                            0x12a4
#define CYG_PCI_VENDOR_VISION_DYN                           0x12a5
#define CYG_PCI_VENDOR_SCALABLE                             0x12a6
#define CYG_PCI_VENDOR_AMO                                  0x12a7
#define CYG_PCI_VENDOR_NEWS_DATACOM                         0x12a8
#define CYG_PCI_VENDOR_XIOTECH                              0x12a9
#define CYG_PCI_VENDOR_SDL                                  0x12aa
#define CYG_PCI_VENDOR_YUAN_YUAN                            0x12ab
#define CYG_PCI_VENDOR_MEASUREX                             0x12ac
#define CYG_PCI_VENDOR_MULTIDATA                            0x12ad
#define CYG_PCI_VENDOR_ALTEON                               0x12ae
#define CYG_PCI_VENDOR_TDK_USA                              0x12af
#define CYG_PCI_VENDOR_JORGE_SCI                            0x12b0
#define CYG_PCI_VENDOR_GAMMALINK                            0x12b1
#define CYG_PCI_VENDOR_GEN_SIGNAL                           0x12b2
#define CYG_PCI_VENDOR_INTER_FACE                           0x12b3
#define CYG_PCI_VENDOR_FUTURE_TEL                           0x12b4
#define CYG_PCI_VENDOR_GRANITE                              0x12b5
#define CYG_PCI_VENDOR_NAT_MICRO                            0x12b6
#define CYG_PCI_VENDOR_ACUMEN                               0x12b7
#define CYG_PCI_VENDOR_KORG                                 0x12b8
#define CYG_PCI_VENDOR_US_ROBOTICS                          0x12b9
#define CYG_PCI_VENDOR_BITTWARE                             0x12ba
#define CYG_PCI_VENDOR_NIPPON_UNI                           0x12bb
#define CYG_PCI_VENDOR_ARRAY_MICRO                          0x12bc
#define CYG_PCI_VENDOR_COMPUTERM                            0x12bd
#define CYG_PCI_VENDOR_ANCHOR_CHIPS                         0x12be
#define CYG_PCI_VENDOR_FUJIFILM_0x12bf                      0x12bf
#define CYG_PCI_VENDOR_INFIMED                              0x12c0
#define CYG_PCI_VENDOR_GMM_RES                              0x12c1
#define CYG_PCI_VENDOR_MENTEC                               0x12c2
#define CYG_PCI_VENDOR_HOLTEK                               0x12c3
#define CYG_PCI_VENDOR_CONN_TECH                            0x12c4
#define CYG_PCI_VENDOR_PICTUREL                             0x12c5
#define CYG_PCI_VENDOR_MITANI                               0x12c6
#define CYG_PCI_VENDOR_DIALOGIC                             0x12c7
#define CYG_PCI_VENDOR_G_FORCE                              0x12c8
#define CYG_PCI_VENDOR_GIGI_OPS                             0x12c9
#define CYG_PCI_VENDOR_I_C_ENGINES                          0x12ca
#define CYG_PCI_VENDOR_ANTEX                                0x12cb
#define CYG_PCI_VENDOR_PLUTO                                0x12cc
#define CYG_PCI_VENDOR_AIMS_LAB                             0x12cd
#define CYG_PCI_VENDOR_NETSPEED                             0x12ce
#define CYG_PCI_VENDOR_PROPHET                              0x12cf
#define CYG_PCI_VENDOR_GDE_SYS                              0x12d0
#define CYG_PCI_VENDOR_PSITECH                              0x12d1
#define CYG_PCI_VENDOR_NVIDIA_0x12d2                        0x12d2
#define CYG_PCI_VENDOR_VINGMED                              0x12d3
#define CYG_PCI_VENDOR_DGM_S                                0x12d4
#define CYG_PCI_VENDOR_EQUATOR                              0x12d5
#define CYG_PCI_VENDOR_ANALOGIC                             0x12d6
#define CYG_PCI_VENDOR_BIOTRONIC                            0x12d7
#define CYG_PCI_VENDOR_PERICOM                              0x12d8
#define CYG_PCI_VENDOR_ACULAB                               0x12d9
#define CYG_PCI_VENDOR_TRUE_TIME                            0x12da
#define CYG_PCI_VENDOR_ANNAPOLIS                            0x12db
#define CYG_PCI_VENDOR_SYMICRON                             0x12dc
#define CYG_PCI_VENDOR_MGI                                  0x12dd
#define CYG_PCI_VENDOR_RAINBOW                              0x12de
#define CYG_PCI_VENDOR_SBS_TECH                             0x12df
#define CYG_PCI_VENDOR_CHASE                                0x12e0
#define CYG_PCI_VENDOR_NINTENDO                             0x12e1
#define CYG_PCI_VENDOR_DATUM                                0x12e2
#define CYG_PCI_VENDOR_IMATION                              0x12e3
#define CYG_PCI_VENDOR_BROOKTROUT                           0x12e4
#define CYG_PCI_VENDOR_CIREL                                0x12e6
#define CYG_PCI_VENDOR_SEBRING                              0x12e7
#define CYG_PCI_VENDOR_CRISC                                0x12e8
#define CYG_PCI_VENDOR_GE_SPACENET                          0x12e9
#define CYG_PCI_VENDOR_ZUKEN                                0x12ea
#define CYG_PCI_VENDOR_AUREAL                               0x12eb
#define CYG_PCI_VENDOR_3A_INTL                              0x12ec
#define CYG_PCI_VENDOR_OPTIVISION                           0x12ed
#define CYG_PCI_VENDOR_ORANGE_MICRO                         0x12ee
#define CYG_PCI_VENDOR_VIENNA                               0x12ef
#define CYG_PCI_VENDOR_PENTEK                               0x12f0
#define CYG_PCI_VENDOR_SORENSON                             0x12f1
#define CYG_PCI_VENDOR_GAMMAGRAPHX                          0x12f2
#define CYG_PCI_VENDOR_MEGATEL                              0x12f4
#define CYG_PCI_VENDOR_FORKS_0x12f5                         0x12f5
#define CYG_PCI_VENDOR_DAWSON_FR                            0x12f6
#define CYG_PCI_VENDOR_COGNEX                               0x12f7
#define CYG_PCI_VENDOR_ELECTRONIC_DESIGN                    0x12f8
#define CYG_PCI_VENDOR_FFT                                  0x12f9
#define CYG_PCI_VENDOR_ESD                                  0x12fe
#define CYG_PCI_VENDOR_RADISYS                              0x1331
#define CYG_PCI_VENDOR_VIDEOMAIL                            0x1335
#define CYG_PCI_VENDOR_ODETICS                              0x1347
#define CYG_PCI_VENDOR_ABB_NETWORK_PARTN                    0x135d
#define CYG_PCI_VENDOR_PTSC                                 0x137e
#define CYG_PCI_VENDOR_SYMPHONY_0x1c1c                      0x1c1c
#define CYG_PCI_VENDOR_TEKRAM_0x1de1                        0x1de1
#define CYG_PCI_VENDOR_CHAINTECH_0x270f                     0x270f
#define CYG_PCI_VENDOR_HANSOL                               0x3000
#define CYG_PCI_VENDOR_3DLABS                               0x3d3d
#define CYG_PCI_VENDOR_AVANCE_0x4005                        0x4005
#define CYG_PCI_VENDOR_DEC_0x4143                           0x4143
#define CYG_PCI_VENDOR_UMAX_COMP                            0x4680
#define CYG_PCI_VENDOR_BUSLOGIC                             0x4b10
#define CYG_PCI_VENDOR_S3                                   0x5333
#define CYG_PCI_VENDOR_TU_BERLIN                            0x5455
#define CYG_PCI_VENDOR_NETPOWER_0x5700                      0x5700
#define CYG_PCI_VENDOR_C4T                                  0x6374
#define CYG_PCI_VENDOR_QUANCOM                              0x8008
#define CYG_PCI_VENDOR_INTEL                                0x8086
#define CYG_PCI_VENDOR_TRIGEM_0x8800                        0x8800
#define CYG_PCI_VENDOR_SIL_MAGIC                            0x8888
#define CYG_PCI_VENDOR_COMPUTONE                            0x8e0e
#define CYG_PCI_VENDOR_ADAPTEC_0x9004                       0x9004
#define CYG_PCI_VENDOR_ADAPTEC_0x9005                       0x9005
#define CYG_PCI_VENDOR_ATRONICS                             0x907f
#define CYG_PCI_VENDOR_MOT_ENGRG                            0xc0fe
#define CYG_PCI_VENDOR_DY4                                  0xd4d4
#define CYG_PCI_VENDOR_TIGER_JET                            0xe159
#define CYG_PCI_VENDOR_ARC                                  0xedd8
#define CYG_PCI_VENDOR_BAD                                  0xffff
//----------------------- AUTO GENERATED END ---------------------------

#define CYG_PCI_VENDOR_UNDEFINED             0xffff

//----------------------------------------------------------------------
// PCI Class IDs
#define CYG_PCI_CLASS_OLD_NONVGA                  0x000000
#define CYG_PCI_CLASS_OLD_VGA                     0x000100

#define CYG_PCI_CLASS_STORAGE_SCSI                0x010000
#define CYG_PCI_CLASS_STORAGE_IDE                 0x010100
#define CYG_PCI_CLASS_STORAGE_FLOPPY              0x010200
#define CYG_PCI_CLASS_STORAGE_IPI                 0x010300
#define CYG_PCI_CLASS_STORAGE_RAID                0x010400
#define CYG_PCI_CLASS_STORAGE_OTHER               0x018000

#define CYG_PCI_CLASS_NET_ETHERNET                0x020000
#define CYG_PCI_CLASS_NET_TOKEN_RING              0x020100
#define CYG_PCI_CLASS_NET_FDDI                    0x020200
#define CYG_PCI_CLASS_NET_ATM                     0x020300
#define CYG_PCI_CLASS_NET_OTHER                   0x028000

#define CYG_PCI_CLASS_DISPLAY_PC_COMPAT_VGA       0x030000
#define CYG_PCI_CLASS_DISPLAY_PC_COMPAT_8514      0x030001
#define CYG_PCI_CLASS_DISPLAY_XGA                 0x030100
#define CYG_PCI_CLASS_DISPLAY_OTHER               0x038000

#define CYG_PCI_CLASS_MM_VIDEO                    0x040000
#define CYG_PCI_CLASS_MM_AUDIO                    0x040100
#define CYG_PCI_CLASS_MM_OTHER                    0x048000

#define CYG_PCI_CLASS_MEM_RAM                     0x050000
#define CYG_PCI_CLASS_MEM_FLASH                   0x050100
#define CYG_PCI_CLASS_MEM_OTHER                   0x058000

#define CYG_PCI_CLASS_BRIDGE_HOST_PCI             0x060000
#define CYG_PCI_CLASS_BRIDGE_PCI_ISA              0x060100
#define CYG_PCI_CLASS_BRIDGE_PCI_EISA             0x060200
#define CYG_PCI_CLASS_BRIDGE_PCI_MC               0x060300
#define CYG_PCI_CLASS_BRIDGE_PCI_PCI              0x060400
#define CYG_PCI_CLASS_BRIDGE_PCI_PCMCIA           0x060500
#define CYG_PCI_CLASS_BRIDGE_PCI_NUBUS            0x060600
#define CYG_PCI_CLASS_BRIDGE_PCI_CARDBUS          0x060700
#define CYG_PCI_CLASS_BRIDGE_OTHER                0x068000

#define CYG_PCI_CLASS_SCC_SERIAL_GENERIC          0x070000
#define CYG_PCI_CLASS_SCC_SERIAL_16450            0x070001
#define CYG_PCI_CLASS_SCC_SERIAL_16550            0x070002
#define CYG_PCI_CLASS_SCC_PARALLEL_STANDARD       0x070100
#define CYG_PCI_CLASS_SCC_PARALLEL_BIDRECTIONAL   0x070101
#define CYG_PCI_CLASS_SCC_PARALLEL_ECP            0x070102
#define CYG_PCI_CLASS_SCC_OTHER                   0x078000

#define CYG_PCI_CLASS_BSP_PIC_GENERIC             0x080000
#define CYG_PCI_CLASS_BSP_PIC_ISA                 0x080001
#define CYG_PCI_CLASS_BSP_PIC_PCI                 0x080002
#define CYG_PCI_CLASS_BSP_DMA_GENERIC             0x080100
#define CYG_PCI_CLASS_BSP_DMA_ISA                 0x080101
#define CYG_PCI_CLASS_BSP_DMA_EISA                0x080102
#define CYG_PCI_CLASS_BSP_TIMER_GENERIC           0x080200
#define CYG_PCI_CLASS_BSP_TIMER_ISA               0x080201
#define CYG_PCI_CLASS_BSP_TIMER_EISA              0x080202
#define CYG_PCI_CLASS_BSP_RTC_GENERIC             0x080300
#define CYG_PCI_CLASS_BSP_RTC_ISA                 0x080301
#define CYG_PCI_CLASS_BSP_OTHER                   0x088000

#define CYG_PCI_CLASS_INPUT_KEYBOARD              0x090000
#define CYG_PCI_CLASS_INPUT_DIGITIZER             0x090100
#define CYG_PCI_CLASS_INPUT_MOUSE                 0x090200
#define CYG_PCI_CLASS_INPUT_OTHER                 0x098000

#define CYG_PCI_CLASS_DOCKING_GENERIC             0x0a0000
#define CYG_PCI_CLASS_DOCKING_OTHER               0x0a8000

#define CYG_PCI_CLASS_CPU_I386                    0x0b0000
#define CYG_PCI_CLASS_CPU_I486                    0x0b0100
#define CYG_PCI_CLASS_CPU_PENTIUM                 0x0b0200
#define CYG_PCI_CLASS_CPU_ALPHA                   0x0b1000
#define CYG_PCI_CLASS_CPU_POWERPC                 0x0b2000
#define CYG_PCI_CLASS_CPU_COPROCESSOR             0x0b8000

#define CYG_PCI_CLASS_SBC_FIREWIRE                0x0c0000
#define CYG_PCI_CLASS_SBC_ACCESSBUS               0x0c0100
#define CYG_PCI_CLASS_SBC_SSA                     0x0c0200
#define CYG_PCI_CLASS_SBC_USB                     0x0c0300
#define CYG_PCI_CLASS_SBC_FIBRE_CHANNEL           0x0c0400

#define CYG_PCI_CLASS_UNKNOWN                     0xff0000

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_PCI_CFG_H
// End of pci_cfg.h
