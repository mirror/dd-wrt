/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/eeprom.c#4 $
 */
#include "diag.h"

#include "ah.h"
#include "ah_internal.h"
#include "ah_eeprom.h"

#include <getopt.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

static void printEepromStruct(FILE *fd, HAL_EEPROM *ee, u_int mode);

struct	ath_diag atd;
int	s;
const char *progname;

u_int16_t
eeread(u_int16_t off)
{
	u_int16_t eedata;

	atd.ad_id = HAL_DIAG_EEREAD | ATH_DIAG_IN | ATH_DIAG_DYN;
	atd.ad_in_size = sizeof(off);
	atd.ad_in_data = (caddr_t) &off;
	atd.ad_out_size = sizeof(eedata);
	atd.ad_out_data = (caddr_t) &eedata;
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);
	return eedata;
}

static void
eewrite(u_long off, u_long value)
{
	HAL_DIAG_EEVAL eeval;

	eeval.ee_off = (u_int16_t) off;
	eeval.ee_data = (u_int16_t) value;

	atd.ad_id = HAL_DIAG_EEWRITE | ATH_DIAG_IN;
	atd.ad_in_size = sizeof(eeval);
	atd.ad_in_data = (caddr_t) &eeval;
	atd.ad_out_size = 0;
	atd.ad_out_data = NULL;
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);
}

static void
usage()
{
	fprintf(stderr, "usage: %s [-i] [offset | offset=value]\n", progname);
	exit(-1);
}

int
main(int argc, char *argv[])
{
	HAL_REVS revs;
	const char *ifname;
	int c;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(1, "socket");
	ifname = getenv("ATH");
	if (!ifname)
		ifname = ATH_DEFAULT;

	progname = argv[0];
	while ((c = getopt(argc, argv, "i:")) != -1)
		switch (c) {
		case 'i':
			ifname = optarg;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	argc -= optind;
	argv += optind;

	strncpy(atd.ad_name, ifname, sizeof (atd.ad_name));

	atd.ad_id = HAL_DIAG_REVS;
	atd.ad_out_data = (caddr_t) &revs;
	atd.ad_out_size = sizeof(revs);
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);

	if (argc == 0) {
		HAL_EEPROM eeprom;

		atd.ad_id = HAL_DIAG_EEPROM;
		atd.ad_out_data = (caddr_t) &eeprom;
		atd.ad_out_size = sizeof(eeprom);
		if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
			err(1, atd.ad_name);

		printEepromStruct(stdout, &eeprom, headerInfo11A);
		printEepromStruct(stdout, &eeprom, headerInfo11B);
		printEepromStruct(stdout, &eeprom, headerInfo11G);
	} else {
		for (; argc > 0; argc--, argv++) {
			u_int16_t off, val, oval;
			char line[256];
			char *cp;

			cp = strchr(argv[0], '=');
			if (cp != NULL)
				*cp = '\0';
			off = (u_int16_t) strtoul(argv[0], NULL, 0);
			if (off == 0 && errno == EINVAL)
				errx(1, "%s: invalid eeprom offset %s",
					progname, argv[0]);
			if (cp == NULL) {
				printf("%04x: %04x\n", off, eeread(off));
			} else {
				val = (u_int16_t) strtoul(cp+1, NULL, 0);
				if (val == 0 && errno == EINVAL)
				errx(1, "%s: invalid eeprom value %s",
					progname, cp+1);
				oval = eeread(off);
				printf("Write %04x: %04x = %04x? ",
					off, oval, val);
				fflush(stdout);
				if (fgets(line, sizeof(line), stdin) != NULL &&
				    line[0] == 'y')
					eewrite(off, val);
			}
		}
	}
	return 0;
}

/*
 * EEPROM REV 3/4 DEBUG PRINT FUNCTIONS
 *
 * NB: Don't change the format of the output; we want it identical
 *     to the Atheros HAL so we can easily compare debug output.
 */
static void
printHeaderInfo(FILE *fd, const HAL_EEPROM *ee, u_int mode)
{
    u_int16_t j, version = ee->ee_version;

    fprintf(fd ,"\n");
    fprintf(fd ," =================Header Information for mode %s===============\n",
	mode == headerInfo11A ? "11a" :
	mode == headerInfo11B ? "11b" :
	mode == headerInfo11G ? "11g" : "???");

    fprintf(fd ," |  Major Version           %2d  ", version >> 12);
    fprintf(fd ,"|  Minor Version           %2d  |\n", version & 0xFFF);
    if (version >= AR_EEPROM_VER4_0) {
	fprintf(fd ," |  EAR Start              %03x  ",
		ee->ee_earStart);
	fprintf(fd ,"|  Target Power Start     %03x  |\n",
		ee->ee_targetPowersStart);
	fprintf(fd ," |  EEP MAP                %3d  ",
		ee->ee_eepMap);
	fprintf(fd ,"|  Enable 32 khz	  %3d  |\n",
		ee->ee_exist32kHzCrystal);
    }
    if (version >= AR_EEPROM_VER5_0) {
	fprintf(fd ," |  EEP Map2PowerCalStart  %3d  ",
		ee->ee_eepMap2PowerCalStart);
	fprintf(fd ,"|                 	       |\n");
    }
    fprintf(fd ," |-------------------------------------------------------------|\n");

    fprintf(fd ," |  A Mode         %1d  ", ee->ee_Amode);
    fprintf(fd ,"|  B Mode         %1d  ", ee->ee_Bmode);
    fprintf(fd ,"|  G Mode        %1d  |\n", ee->ee_Gmode);

    if(ee->ee_regdomain >> 15) {
        fprintf(fd ," |  Country Code %03x  ", ee->ee_regdomain >> 15);
    } else {
        fprintf(fd ," |  Reg. Domain  %03x  ", ee->ee_regdomain & 0xFFF);
    }

    fprintf(fd ,"|  Turbo2 Disable %1d  ", ee->ee_turbo5Disable);
    fprintf(fd ,"|  Turbo5 Disable %1d |\n", ee->ee_turbo2Disable);
    fprintf(fd ," |  RF Silent      %1d  ", ee->ee_rfKill);
    fprintf(fd ,"|  XR5 Disable    %1d  ", ee->ee_disableXr5);
    fprintf(fd ,"|  XR2 Disable    %1d |\n", ee->ee_disableXr2);
    fprintf(fd ," | Turbo 2W Maximum dBm %2d | cckOfdmDelta(10x) %2d | GainI %2d   |\n",
	ee->ee_turbo2WMaxPower5, ee->ee_cckOfdmPwrDelta, ee->ee_gainI[mode]);
    fprintf(fd ," |-------------------------------------------------------------|\n");
    if (version >= AR_EEPROM_VER3_3) {
        fprintf(fd ," |  worldwide roaming        %1x  ",
		(ee->ee_regdomain >> 14) & 0x1);
        fprintf(fd ,"|  False detect backoff  0x%02x  |\n",
		ee->ee_falseDetectBackoff[mode]);
    }
    fprintf(fd ," |  device type              %1x  ", ee->ee_deviceType);

    fprintf(fd ,"|  Switch Settling Time  0x%02x  |\n", ee->ee_switchSettling[mode]);
    fprintf(fd ," |  ADC Desired size       %2d  ", ee->ee_adcDesiredSize[mode]);
    fprintf(fd ,"|  XLNA Gain             0x%02x  |\n", ee->ee_xlnaGain[mode]);

    fprintf(fd ," |  tx end to XLNA on     0x%02x  ", ee->ee_txEndToXLNAOn[mode]);
    fprintf(fd ,"|  Threashold 62         0x%02x  |\n", ee->ee_thresh62[mode]);

    fprintf(fd ," |  tx end to XPA off     0x%02x  ", ee->ee_txEndToXPAOff[mode]);
    fprintf(fd ,"|  tx end to XPA on      0x%02x  |\n", ee->ee_txFrameToXPAOn[mode]);

    fprintf(fd ," |  PGA Desired size       %2d  ", ee->ee_pgaDesiredSize[mode]);
    fprintf(fd ,"|  Noise Threshold        %3d  |\n", ee->ee_noiseFloorThresh[mode]);

    fprintf(fd ," |  XPD Gain              0x%02x  ", ee->ee_xgain[mode]);
    fprintf(fd ,"|  XPD                      %1d  |\n", ee->ee_xpd[mode]);

    fprintf(fd ," |  txrx Attenuation      0x%02x  ", ee->ee_txrxAtten[mode]);
    fprintf(fd ,"|  Capabilities        0x%04X  |\n", ee->ee_capField);
    if (version >= AR_EEPROM_VER5_0) {
	 int is11g = (mode != headerInfo11A);
         fprintf(fd ," |  Turbo txrx Attenuat   0x%02x  ", ee->ee_txrxAtten[is11g]);
         fprintf(fd ,"|  Turbo Switch Settling 0x%02X  |\n", ee->ee_switchSettlingTurbo[is11g]);
         fprintf(fd ," |  Turbo ADC Desired Size %2d  ", ee->ee_adcDesiredSizeTurbo[is11g]);
         fprintf(fd ,"|  Turbo PGA Desired Size %2d  |\n", ee->ee_pgaDesiredSizeTurbo[is11g]);
         fprintf(fd ," |  Turbo rxtx Margin     0x%02x  ", ee->ee_rxtxMarginTurbo[is11g]);
         fprintf(fd ,"|                              |\n");
    }
    
    for(j = 0; j < 10; j+=2) {
        fprintf(fd ," |  Antenna control   %2d  0x%02X  ",
		j, ee->ee_antennaControl[j][mode]);
	fprintf(fd ,"|  Antenna control   %2d  0x%02X  |\n",
		j + 1, ee->ee_antennaControl[j + 1][mode]);
    }
    fprintf(fd ," |  Antenna control   %2d  0x%02X  ",
	    j, ee->ee_antennaControl[j][mode]);
    fprintf(fd ,"|                              |\n");

    fprintf(fd ," |-------------------------------------------------------------|\n");
    if (mode == headerInfo11A) {
        fprintf(fd ," |   OB_1   %1d   ", ee->ee_ob1);
        fprintf(fd ,"|   OB_2    %1d   ", ee->ee_ob2);
        fprintf(fd ,"|   OB_3   %1d  ", ee->ee_ob3);
        fprintf(fd ,"|   OB_4     %1d   |\n", ee->ee_ob4);
        fprintf(fd ," |   DB_1   %1d   ", ee->ee_db1);
        fprintf(fd ,"|   DB_2    %1d   ", ee->ee_db2);
        fprintf(fd ,"|   DB_3   %1d  ", ee->ee_db3);
        fprintf(fd ,"|   DB_4     %1d   |\n", ee->ee_db4);
    } else {
        if(version >= AR_EEPROM_VER3_1) {
            if (mode == headerInfo11B) {
                fprintf(fd ," |   OB_1   %1d   ", ee->ee_obFor24);
                fprintf(fd ,"|   B_OB    %1d   ", ee->ee_ob2GHz[0]);
                fprintf(fd ,"|   DB_1   %1d  ", ee->ee_dbFor24);
                fprintf(fd ,"|   B_DB     %1d   |\n", ee->ee_db2GHz[0]);
            } else {
                fprintf(fd ," |   OB_1   %1d   ", ee->ee_obFor24g);
                fprintf(fd ,"|   B_OB    %1d   ", ee->ee_ob2GHz[1]);
                fprintf(fd ,"|   DB_1   %1d  ", ee->ee_dbFor24g);
                fprintf(fd ,"|   B_DB     %1d   |\n", ee->ee_db2GHz[1]);
            }
        } else {
            if (mode == headerInfo11B) {
                fprintf(fd ," |  OB_1                     %1d  ",
			ee->ee_obFor24);
                fprintf(fd ,"|  DB_1                     %1d  |\n",
			ee->ee_dbFor24);
            } else {
                fprintf(fd ," |  OB_1                     %1d  ",
			ee->ee_obFor24g);
                fprintf(fd ,"|  DB_1                     %1d  |\n",
			ee->ee_dbFor24g);
            }

        }
        
    }
    fprintf(fd ," ===============================================================\n");
}

static void
printChannelInfo(FILE *fd, const HAL_EEPROM *ee, u_int mode)
{
    u_int16_t            i, j, k = 0;
    const DATA_PER_CHANNEL *pDataPerChannel;

    switch(mode) {
    case headerInfo11A:
        pDataPerChannel = ee->ee_dataPerChannel11a;
        break;

    case headerInfo11G:
        pDataPerChannel = ee->ee_dataPerChannel11g;
        break;

    case headerInfo11B:
        pDataPerChannel = ee->ee_dataPerChannel11b;
        break;

    default:
        fprintf(fd ,"%s: Illegal mode %u\n", __func__, mode);
        return;
    }
    
    fprintf(fd ,"\n");
    if (mode == headerInfo11A) {
        fprintf(fd ,"=========================Calibration Information============================\n");
        
        for (k = 0; k < 10; k+=5) {
            for (i = k; i < k + 5; i++) {
                fprintf(fd ,"|     %04d     ",
                    pDataPerChannel[i].channelValue);
            }
            fprintf(fd ,"|\n");
            
            fprintf(fd ,"|==============|==============|==============|==============|==============|\n");
            for (i = k; i < k + 5; i++) {
                fprintf(fd ,"|pcdac pwr(dBm)");
            }
            fprintf(fd ,"|\n");

            for (j = 0; j < pDataPerChannel[0].numPcdacValues; j++) {
                for (i = k; i < k + 5; i++) {
                    fprintf(fd ,"|  %02d    %2d.%02d ",
                        pDataPerChannel[i].PcdacValues[j],
                        pDataPerChannel[i].PwrValues[j] / EEP_SCALE,
                        pDataPerChannel[i].PwrValues[j] % EEP_SCALE);
                }
                fprintf(fd ,"|\n");
            }
            
            fprintf(fd ,"|              |              |              |              |              |\n"); 
            for (i = k; i < k + 5; i++) {
                fprintf(fd ,"| pcdac min %02d ", pDataPerChannel[i].pcdacMin);
            }
            fprintf(fd ,"|\n");
            for (i = k; i < k + 5; i++) {
                fprintf(fd ,"| pcdac max %02d ", pDataPerChannel[i].pcdacMax);
            }
            fprintf(fd ,"|\n");
            fprintf(fd ,"|==============|==============|==============|==============|==============|\n");
        }
    } else {
        fprintf(fd ,"               ==========Calibration Information=============\n");
        
        for (i = 0; i < 3; i++) {
            if (0 == i) {
                fprintf(fd ,"               ");
            }
            fprintf(fd ,"|     %04d     ",
                pDataPerChannel[i].channelValue);
        }
        fprintf(fd ,"|\n");
        
        fprintf(fd ,"               |==============|==============|==============|\n");
        for (i = 0; i < 3; i++) {
            if (0 == i) {
                fprintf(fd ,"               ");
            }
            fprintf(fd ,"|pcdac pwr(dBm)");
        }
        fprintf(fd ,"|\n");

        for (j = 0; j < pDataPerChannel[0].numPcdacValues; j++) {
            for (i = 0; i < 3; i++) {
                if (0 == i) {
                    fprintf(fd ,"               ");
                }
                fprintf(fd ,"|  %02d    %2d.%02d ",
                    pDataPerChannel[i].PcdacValues[j],
                    pDataPerChannel[i].PwrValues[j] / EEP_SCALE,
                    pDataPerChannel[i].PwrValues[j] % EEP_SCALE);

            }
            fprintf(fd ,"|\n");
        }
        
        fprintf(fd ,"               |              |              |              |\n");    
        fprintf(fd ,"               ");
        for (i = 0; i < 3; i++) {
            fprintf(fd ,"| pcdac min %02d ", pDataPerChannel[i].pcdacMin);
        }
        fprintf(fd ,"|\n");
        fprintf(fd ,"               ");
        for (i = 0; i < 3; i++) {
            fprintf(fd ,"| pcdac max %02d ", pDataPerChannel[i].pcdacMax);
        }
        fprintf(fd ,"|\n");
        fprintf(fd ,"               |==============|==============|==============|\n");

    }
}

static void
printExpnPower5112(FILE *fd, const HAL_EEPROM *ee, const EEPROM_POWER_EXPN_5112 *pExpnPower, u_int mode)
{
    u_int16_t        i, j=0, kk;
    u_int16_t        singleXpd = 0xDEAD;
    const char       *modeString[3] = {"11a", "11b", "11g"};
    const char	     *rule;

    if (pExpnPower->xpdMask != 0x9) {
        for (j = 0; j < NUM_XPD_PER_CHANNEL; j++) {
            if (pExpnPower->xpdMask == (1 << j)) {
                singleXpd = j;
                break;
            }
        }
    } 

    if (pExpnPower->xpdMask == 0x9) {
        fprintf(fd ,"======================5112 Power Calibration Information========================\n");

        fprintf(fd ,"| XPD_Gain_mask = 0x%02x              | Number of channels for mode %s: %2d      |\n",
            pExpnPower->xpdMask, modeString[mode], pExpnPower->numChannels);
        rule = "|======|========|========|========|========|========|========|========|========|";
    } else {
        fprintf(fd ,"=========5112 Power Calibration Information==========\n");
	fprintf(fd ,"| XPD_Gain_mask     0x%02x | Number of channels    %2d |\n",
            pExpnPower->xpdMask, pExpnPower->numChannels);
	fprintf(fd ,"| XPD_GAIN            %2d |                          |\n", j);
	rule = "|======|========|========|========|========|========|";
    }

    /* print the frequency values */
    fprintf(fd ,"%s\n", rule);
    fprintf(fd ,"| freq |  pwr1  |  pwr2  |  pwr3  |  pwr4  |");
    if (pExpnPower->xpdMask == 0x9) {
        fprintf(fd ," pwr1_x3| pwr2_x3| pwr3_x3| maxPow |\n");
        fprintf(fd ,"|      | [pcd]  | [pcd]  | [pcd]  | [pcd]  | [pcd]  | [pcd]  | [pcd]  |        |\n");
    } else {
        fprintf(fd ," maxPow |\n");
        fprintf(fd ,"|      | [pcd]  | [pcd]  | [pcd]  | [pcd]  |        |\n");
    }

    for (i = 0; i < pExpnPower->numChannels; i++) {
        fprintf(fd ,"%s\n", rule);
        fprintf(fd ,"| %4d |", pExpnPower->pChannels[i]);
        if (pExpnPower->xpdMask != 0x9) {
            j = singleXpd;
        } else {
            j = 0;
        }
        for (kk=0; kk < pExpnPower->pDataPerChannel[i].pDataPerXPD[j].numPcdacs; kk++) {
            fprintf(fd ," %2d.%02d  |", (pExpnPower->pDataPerChannel[i].pDataPerXPD[j].pwr_t4[kk] / 4),
                (pExpnPower->pDataPerChannel[i].pDataPerXPD[j].pwr_t4[kk] % 4) * 25);
        }
        if (pExpnPower->xpdMask == 0x9) {
            for (kk=0; kk < pExpnPower->pDataPerChannel[i].pDataPerXPD[3].numPcdacs; kk++) {
                fprintf(fd ," %2d.%02d  |", pExpnPower->pDataPerChannel[i].pDataPerXPD[3].pwr_t4[kk] / 4,
                    (pExpnPower->pDataPerChannel[i].pDataPerXPD[3].pwr_t4[kk] % 4) * 25);
            }
        }
        fprintf(fd ," %2d.%02d  |\n", pExpnPower->pDataPerChannel[i].maxPower_t4 / 4,
            (pExpnPower->pDataPerChannel[i].maxPower_t4 % 4) * 25);
        fprintf(fd ,"|      |");        
        for (kk=0; kk<pExpnPower->pDataPerChannel[i].pDataPerXPD[j].numPcdacs; kk++) {
            fprintf(fd ,"  [%2d]  |", pExpnPower->pDataPerChannel[i].pDataPerXPD[j].pcdac[kk]);
        }
        if (pExpnPower->xpdMask == 0x9) {
            for (kk=0; kk<pExpnPower->pDataPerChannel[i].pDataPerXPD[3].numPcdacs; kk++) {
                fprintf(fd ,"  [%2d]  |", pExpnPower->pDataPerChannel[i].pDataPerXPD[3].pcdac[kk]);
            }
        }
        fprintf(fd ,"        |\n");     
    }
    fprintf(fd ,"%s\n", rule);
}

static void
printTargetPowerInfo(FILE *fd, const HAL_EEPROM *ee, u_int mode)
{
    u_int16_t i, k;
    const TRGT_POWER_INFO  *pPowerInfo;

    fprintf(fd ,"\n");
    if (mode == headerInfo11A) {
        pPowerInfo = ee->ee_trgtPwr_11a;
        fprintf(fd ,"============================Target Power Info===============================\n");
    
        for (k = 0; k < 8; k+=4) {
            fprintf(fd ,"|     rate     ");
            for (i = k; i < k + 4; i++) {
                fprintf(fd ,"|     %04d     ", pPowerInfo[i].testChannel);
            }
            fprintf(fd ,"|\n");
            
            fprintf(fd ,"|==============|==============|==============|==============|==============|\n");

            fprintf(fd ,"|     6-24     ");
            for (i = k; i < k + 4; i++) {
                fprintf(fd ,"|     %2d.%d     ", pPowerInfo[i].twicePwr6_24 / 2,
                    (pPowerInfo[i].twicePwr6_24 % 2) * 5);
            }
            fprintf(fd ,"|\n");

            fprintf(fd ,"|      36      ");
            for (i = k; i < k + 4; i++) {
                fprintf(fd ,"|     %2d.%d     ", pPowerInfo[i].twicePwr36 / 2,
                    (pPowerInfo[i].twicePwr36 % 2) * 5);
            }
            fprintf(fd ,"|\n");

            fprintf(fd ,"|      48      ");
            for (i = k; i < k + 4; i++) {
                fprintf(fd ,"|     %2d.%d     ", pPowerInfo[i].twicePwr48 / 2,
                    (pPowerInfo[i].twicePwr48 % 2) * 5);
            }
            fprintf(fd ,"|\n");

            fprintf(fd ,"|      54      ");
            for (i = k; i < k + 4; i++) {
                fprintf(fd ,"|     %2d.%d     ", pPowerInfo[i].twicePwr54 / 2,
                    (pPowerInfo[i].twicePwr54 % 2) * 5);
            }

            fprintf(fd ,"|\n");
            fprintf(fd ,"|==============|==============|==============|==============|==============|\n");
        }
    } else {
        if (mode == headerInfo11B) {
            pPowerInfo = ee->ee_trgtPwr_11b;
        } else {
            pPowerInfo = ee->ee_trgtPwr_11g;
        }
        fprintf(fd ,"=============Target Power Info================\n");

        fprintf(fd ,"|     rate     ");
        for (i = 0; i < 2; i++) {
            fprintf(fd ,"|     %04d     ",
                pPowerInfo[i].testChannel);
        }
        fprintf(fd ,"|\n");
        
        fprintf(fd ,"|==============|==============|==============|\n");

        if (mode == headerInfo11B) {
            fprintf(fd ,"|      1       ");
        } else {
            fprintf(fd ,"|     6-24     ");
        }

        for (i = 0; i < 2; i++) {
            fprintf(fd ,"|     %2d.%d     ", pPowerInfo[i].twicePwr6_24 / 2,
                (pPowerInfo[i].twicePwr6_24 % 2) * 5);
        }
        fprintf(fd ,"|\n");

        if (mode == headerInfo11B) {
            fprintf(fd ,"|      2       ");
        } else {
            fprintf(fd ,"|      36      ");
        }
        for (i = 0; i < 2; i++) {
            fprintf(fd ,"|     %2d.%d     ", pPowerInfo[i].twicePwr36 / 2,
                (pPowerInfo[i].twicePwr36 % 2) * 5);
        }
        fprintf(fd ,"|\n");

        if (mode == headerInfo11B) {
            fprintf(fd ,"|      5.5     ");
        } else {
            fprintf(fd ,"|      48      ");
        }
        for (i = 0; i < 2; i++) {
            fprintf(fd ,"|     %2d.%d     ", pPowerInfo[i].twicePwr48 / 2,
                (pPowerInfo[i].twicePwr48 % 2) * 5);
        }
        fprintf(fd ,"|\n");

        if (mode == headerInfo11B) {
            fprintf(fd ,"|      11      ");
        } else {
            fprintf(fd ,"|      54      ");
        }
        for (i = 0; i < 2; i++) {
            fprintf(fd ,"|     %2d.%d     ", pPowerInfo[i].twicePwr54 / 2,
                (pPowerInfo[i].twicePwr54 % 2) * 5);
        }

        fprintf(fd ,"|\n");
        fprintf(fd ,"|==============|==============|==============|\n");

    }
}

static void
printRDEdges(FILE *fd, const HAL_EEPROM *ee,
	    const RD_EDGES_POWER *pRdEdgePwrInfo, const u_int16_t *pTestGroups,
            u_int mode, const u_int16_t maxNumCtl)
{
    u_int16_t version = ee->ee_version;
    u_int16_t    i=0, j;
    u_int16_t    ctlMode = 0;
    const static char *ctlType[16] = {
        "11a base mode ] ",
        "11b mode ]      ",
        "11g mode ]      ",
        "11a TURBO mode ]",
        "108g mode ]     ",
        "2GHT20 mode ]   ",
        "5GHT20 mode ]   ",
        "2GHT40 mode ]   ",
        "5GHT40 mode ]   ",
	"0x9 mode ]      ",
	"0xa mode ]      ",
	"0xb mode ]      ",
	"0xc mode ]      ",
	"0xd mode ]      ",
	"0xe mode ]      ",
	"0xf mode ]      ",
    };
    const static char *ctlRD[8] = {
	"0x00",
	" FCC",
	"0x20",
	"ETSI",
	" MKK",
	"0x50",
	"0x60",
	"0x70"
    };

    fprintf(fd ,"\n");
    fprintf(fd ,"=======================Test Group Band Edge Power========================\n");
    while ((pTestGroups[i] != 0) && (i < maxNumCtl)) {
        switch(pTestGroups[i] & 0x3) {
        case 0: 
        case 3:
            ctlMode = headerInfo11A;
            break;
        case 1: 
            ctlMode = headerInfo11B;
            break;
        case 2: 
            ctlMode = headerInfo11G;
            break;
        }
        if (mode != ctlMode) {
            i++;
            pRdEdgePwrInfo += NUM_EDGES;
            continue;
        }
        fprintf(fd ,"|                                                                       |\n");
        fprintf(fd ,"| CTL: 0x%02x   [ %s %s",
	    pTestGroups[i] & 0xff,
	    ctlRD[(pTestGroups[i] >> 4) & 0x7],
	    ctlType[pTestGroups[i] & CTL_MODE_M]);
        
        fprintf(fd ,"                                   |\n");
        fprintf(fd ,"|=======|=======|=======|=======|=======|=======|=======|=======|=======|\n");

        fprintf(fd ,"| edge  ");
        for (j = 0; j < NUM_EDGES; j++) {
            if (pRdEdgePwrInfo[j].rdEdge == 0) {
                fprintf(fd ,"|  --   ");
            } else {
                fprintf(fd ,"| %04d  ", pRdEdgePwrInfo[j].rdEdge);
            }
        }

        fprintf(fd ,"|\n");
        fprintf(fd ,"|=======|=======|=======|=======|=======|=======|=======|=======|=======|\n");
        fprintf(fd ,"| power ");
        for (j = 0; j < NUM_EDGES; j++) {
            if (pRdEdgePwrInfo[j].rdEdge == 0) {
                fprintf(fd ,"|  --   ");
            } else {
                fprintf(fd ,"| %2d.%d  ", pRdEdgePwrInfo[j].twice_rdEdgePower / 2,
                    (pRdEdgePwrInfo[j].twice_rdEdgePower % 2) * 5);
            }
        }

        fprintf(fd ,"|\n");
        if (version >= AR_EEPROM_VER3_3) {
            fprintf(fd ,"|=======|=======|=======|=======|=======|=======|=======|=======|=======|\n");
            fprintf(fd ,"| flag  ");
            for (j = 0; j < NUM_EDGES; j++) {
                if (pRdEdgePwrInfo[j].rdEdge == 0) {
                    fprintf(fd ,"|  --   ");
                } else {
                    fprintf(fd ,"|   %1d   ", pRdEdgePwrInfo[j].flag);
                }
            }

            fprintf(fd ,"|\n");
        }
        fprintf(fd ,"=========================================================================\n");
        i++;
        pRdEdgePwrInfo += NUM_EDGES;
    }
}

static int16_t
getMaxPower(const RAW_DATA_PER_CHANNEL_2413 *data)
{
    u_int32_t i;
    u_int16_t numVpd;

    for (i = 0; i < MAX_NUM_PDGAINS_PER_CHANNEL; i++) {
	numVpd = data->pDataPerPDGain[i].numVpd;
	if (numVpd > 0)
	    return data->pDataPerPDGain[i].pwr_t4[numVpd-1];
    }
    return 0;
}

static void
printChannelInfo2413(FILE *fd, const HAL_EEPROM *ee, const RAW_DATA_STRUCT_2413 *pRaw)
{
    u_int16_t            i, j, k = 0;
    int16_t maxPower_t4;
    const char *rule = "|========|======|========|========|========|========|========|";

    fprintf(fd ,"\n");

    fprintf(fd ,"=========2413 Power Calibration Information===================\n");
    fprintf(fd ,"| XPD_Gain_mask     0x%02x | Number of channels    %2d |        |\n",
            pRaw->xpd_mask, pRaw->numChannels);

    /* print the frequency values */
    fprintf(fd ,"%s\n", rule);
    fprintf(fd ,"|  freq  | pd   |  pwr1  |  pwr2  |  pwr3  |  pwr4  |  pwr5  |");
    fprintf(fd ,"\n");
    fprintf(fd ,"| maxpow | gain | [Vpd]  | [Vpd]  | [Vpd]  | [Vpd]  | [Vpd]  |\n");

    for (i = 0; i < pRaw->numChannels; i++) {
        fprintf(fd ,"%s\n", rule);
        fprintf(fd ,"|  %4d  |", pRaw->pChannels[i]);
        for (j=0; j < pRaw->pDataPerChannel[i].numPdGains; j++) {
	    if (j == 1) {
		maxPower_t4 = pRaw->pDataPerChannel[i].maxPower_t4;
		if (maxPower_t4 == 0) {
		    maxPower_t4 = getMaxPower(&pRaw->pDataPerChannel[i]);
		}
		fprintf(fd ,"| %2d.%02d  |", maxPower_t4 / 4,
		    (maxPower_t4 % 4) * 25);
	    }
	    else if (j > 0)
		fprintf(fd ,"|         |");
	    fprintf(fd, " %4d |", pRaw->pDataPerChannel[i].pDataPerPDGain[j].pd_gain);
	    for (k=0; k < pRaw->pDataPerChannel[i].pDataPerPDGain[j].numVpd; k++)
		    fprintf(fd ," %2d.%02d  |", (pRaw->pDataPerChannel[i].pDataPerPDGain[j].pwr_t4[k] / 4),
			(pRaw->pDataPerChannel[i].pDataPerPDGain[j].pwr_t4[k] % 4) * 25);
	    for (; k<NUM_POINTS_LAST_PDGAIN; k++)
		    fprintf(fd ,"        |");
	    fprintf(fd, "\n");
            fprintf(fd ,"|        |      |");
	    for (k=0; k < pRaw->pDataPerChannel[i].pDataPerPDGain[j].numVpd; k++)
		    fprintf(fd ," [%3d]  |", pRaw->pDataPerChannel[i].pDataPerPDGain[j].Vpd[k]);
	    for (; k<NUM_POINTS_LAST_PDGAIN; k++)
		    fprintf(fd ,"        |");
	    fprintf(fd, "\n");
        }
    }
    fprintf(fd ,"%s\n", rule);
}

static void
printEepromStruct(FILE *fd, HAL_EEPROM *ee, u_int mode)
{
    switch(mode) {
    case headerInfo11A:
	if (!ee->ee_Amode)
	    return;
        break;
    case headerInfo11G:
	if (!ee->ee_Gmode)
	    return;
        break;
    case headerInfo11B:
	if (!ee->ee_Bmode)
	    return;
        break;
    }
    printHeaderInfo(fd, ee, mode);
    if (ee->ee_version >= AR_EEPROM_VER5_0) {
        printChannelInfo2413(fd, ee, &ee->ee_rawDataset2413[mode]);
    } else if (ee->ee_version >= AR_EEPROM_VER4_0) {
	EEPROM_POWER_EXPN_5112 *exp;

	exp = &ee->ee_modePowerArray5112[mode];
	atd.ad_id = HAL_DIAG_EEPROM_EXP_11A+mode;
	atd.ad_out_size = roundup(sizeof(u_int16_t) * exp->numChannels,
				sizeof(u_int32_t)) +
		      sizeof(EXPN_DATA_PER_CHANNEL_5112) * exp->numChannels;
	atd.ad_out_data = (caddr_t) malloc(atd.ad_out_size);
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);
	exp->pChannels = (void *) atd.ad_out_data;
	exp->pDataPerChannel = (void *)((char *)atd.ad_out_data +
		roundup(sizeof(u_int16_t) * exp->numChannels, sizeof(u_int32_t)));
        printExpnPower5112(fd, ee, exp, mode);
	free(atd.ad_out_data);
    } else {
        printChannelInfo(fd, ee, mode);
    }
    printTargetPowerInfo(fd, ee, mode);
    printRDEdges(fd, ee, ee->ee_rdEdgesPower, ee->ee_ctl, mode, ee->ee_numCtls);
}
