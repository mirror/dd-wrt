/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_API_G997_H
#define _DRV_DSL_CPE_API_G997_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"

/** \file
   G.997.1 interface
*/

/** \addtogroup DRV_DSL_CPE_G997
 @{ */

/**
   This structure is used to store one imaginary number which consists of a
   real and a imaginary part.
*/
typedef struct
{
   /**
   Real part */
   DSL_uint16_t nReal;
   /**
   Imaginary part */
   DSL_uint16_t nImag;
} DSL_G997_ComplexNumber_t;

/**
   This structure is used to set/get data related to the number of used
   subcarriers. Regarding to the chosen ADSL standard, the used Annex type and
   if used for upstream or downstream the number of used data elements varies
   from 32 to \ref DSL_MAX_NSC.
   This structure includes an array of 32 bit values.
*/
typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_G997_ComplexNumber_t nNSCComplexData[DSL_MAX_NSC];
} DSL_G997_NSCComplexData_t;

/**
   This structure is used to set/get data related to the number of used
   subcarriers. Regarding to the chosen ADSL standard, the used Annex type and
   if used for upstream or downstream the number of used data elements varies
   from 32 to \ref DSL_MAX_NSC.
   This structure includes an array of 16 bit values.
*/
typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_uint16_t nNSCData[DSL_MAX_NSC];
} DSL_G997_NSCData16_t;

/**
   This structure is used to set/get data related to the number of used
   subcarriers. Regarding to the chosen ADSL standard, the used Annex type and
   if used for upstream or downstream the number of used data elements varies
   from 32 to \ref DSL_MAX_NSC.
   This structure includes an array of 8 bit values.

   \note For VDSL2 with 30a profiles this data uses 8kHz tone spacing what means
         tones 0..4096.
*/
typedef struct
{
   /**
   Number of used data elements */
   DSL_uint16_t nNumData;
   /**
   Data elements */
   DSL_uint8_t nNSCData[DSL_MAX_NSC];
} DSL_G997_NSCData8_t;


/**
   Loop Diagnostics Mode forced (LDSF) (see chapter 7.3.1.1.8 of G.997.1).
   This configuration parameter defines whether the line should be forced into
   the loop diagnostics mode by the near-end xTU on this line. It is coded as
   an integer value with following definition.
   \note
   The line needs to be forced to the L3 state (see chapter 7.3.1.1.3 of G.997.1)
   before it can be forced to the loop diagnostics mode. Only while the line
   power management state is the L3 state (see chapter 7.5.1.2 of G.997.1), the
   line can be forced into the loop diagnostics mode procedures. When the loop
   diagnostics mode procedures are completed successfully, the Access Node shall
   reset the LDSF MIB element to 0 and the line shall returns to remain in the
   L3 idle state. The loop diagnostics data shall be available at least until
   the line is forced to the L0 state (see chapter 7.3.1.1.3 of G.997.1).
   If the loop diagnostics procedures cannot be completed successfully, (after a
   vendor discretionary number of retries and/or within a vendor discretionary
   timeout), then an Initialization Failure occurs. As long as loop diagnostics
   procedures are not completed successfully, attempts shall be made to do so,
   until the loop diagnostics mode is no longer forced on the line through this
   configuration parameter.
*/
typedef enum
{
   /**
   Inhibits the near-end xTU from performing loop diagnostics mode
   procedures on the line (DELT). Loop diagnostic mode procedures may still be
   initiated by the far-end xTU. */
   DSL_G997_INHIBIT_LDSF = 0,
   /**
   Forces the near-end xTU to perform the loop diagnostics procedures (DELT). */
   DSL_G997_FORCE_LDSF = 1,
   /**
   The automatic mode has to be used to activate an automatic flow for a
   diagnostic measurement as follows:
   - after manual restart of the autoboot handling (using ioctl
     \ref DSL_FIO_AUTOBOOT_CONTROL_SET with option
     \ref DSL_AUTOBOOT_CTRL_RESTART) the first line activation will be done in
     diagnostic mode (if possible)
   - all following line activations will be done without diagnostic mode */
   DSL_G997_AUTO_LDSF = 2,
   /** Delimeter only*/
   DSL_G997_LDSF_LAST
} DSL_G997_LDSF_t;


/**
   Automode Cold Start Forced (ACSF), (see chapter 7.3.1.1.10 of G.997.1).
   This parameter is defined in order to improve testing of the performance of
   xTU's supporting automode when it is enabled in the MIB. The valid values
   are 0 and 1. A change in value of this parameter, indicates a change in
   loop conditions applied to the devices under test. The xTU's shall reset any
   historical information used for automode and for shortening G.994.1
   handshake and initialization.
   Automode is defined as the case where multiple operation-modes are enabled
   in the MIB in the G.997.1 "xTU Transmission System Enabling (XTSE)" table
   and where the selection of the operation-mode to be used for transmission,
   does not only depend on the common capabilities of both xTU's (as exchanged
   in G.994.1), but depends also on achievable datarates under given
   loop conditions.
   This parameter is mandatory at the Q interface for modems supporting
   automode.
*/
typedef enum
{
   /**
   Inhibits to use the Automode Cold Start Forced feature. */
   DSL_G997_INHIBIT_ACSF = 0,
   /**
   Forces to use the Automode Cold Start Forced feature. */
   DSL_G997_FORCE_ACSF = 1,
   /** Delimeter only*/
   DSL_G997_ACSF_LAST
} DSL_G997_ACSF_t;


/**
   Automode Cold Start Forced (ACSF), (see chapter 7.3.1.1.10 of G.997.1),
*/
typedef enum
{
   /**
   Activates the normal startup procedure which performs a full initialization
   in any case. */
   DSL_G997_NORMAL_STARTUP = 0,
   /**
   Activates the short startup feature which will be used for consecutive
   initializations (first init will be a full init in any case). */
   DSL_G997_FORCE_SHORT_STARTUP = 1,
   /** Delimeter only*/
   DSL_G997_STARTUP_LAST
} DSL_G997_StartupMode_t;


/**
   This configuration parameter defines the behavior of the line after
   activation.
*/
typedef struct
{
   /**
   Specifies whether to force Loop diagnostic mode or not (DELT).
   (default: DSL_G997_INHIBIT_LDSF) */
   DSL_CFG DSL_G997_LDSF_t nLDSF;
   /**
   This parameter is defined in order to improve testing of the performance of
   XTU's supporting automode when it is enabled in the MIB. The valid values are
   0 and 1. A change in value of this parameter indicates a change in loop
   conditions applied to the devices under test. The XTU's shall reset any
   historical information used for automode and for shortening G.994.1 handshake
   and initialization.
   Automode is defined as the case where multiple operation-modes are enabled
   in the MIB in the G.997.1 "xTU Transmission System Enabling (XTSE)" table
   and where the selection of the operation-mode to be used for transmission
   does not only depend on the common capabilities of both XTU's (as exchanged
   in G.994.1), but depends also on achievable data rates under given loop
   conditions.
   This parameter is mandatory at the Q interface for modems supporting
   automode.
   \attention This mode is currently not supported.
              The default value is DSL_G997_INHIBIT_ACSF */
   DSL_CFG DSL_G997_ACSF_t nACSF;
   /**
   Specifies whether to use the short startup mode or not
   (default: DSL_G997_INHIBIT_SHORT_STARTUP) */
   DSL_CFG DSL_G997_StartupMode_t nStartupMode;
} DSL_G997_LineActivateData_t;

/**
   This configuration parameter defines the behavior of the line after
   activation.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_LINE_ACTIVATE_CONFIG_SET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains line activation configuration settings */
   DSL_CFG DSL_G997_LineActivateData_t data;
} DSL_G997_LineActivate_t;


/**
   xTU Transmission System Enabling - Octet 1.
   - Format of definitions:  XTSE_v_ww_x_y_z
   - v: number of octet
   - w: number of bit in actual octet
   - x: character for Annex type (A, B, C, J, M)
   - y: ADSL G.992.y
   - z: non overlapped (NO) or overlapped (O) mode
*/
/**
   Regional standards: It is recommended to use this bit 1 the ANSI T1.413
   1998 Standard. */
#define XTSE_1_01_A_T1_413 0x01
/**
   Regional standards: It is recommended to use this bit the Annex C of
   TS 101 388 v1.3.1. */
#define XTSE_1_02_C_TS_101388 0x02
/**
   G.992.1 / ADSL operation over POTS non-overlapped spectrum
   (Annex A/G.992.1). */
#define XTSE_1_03_A_1_NO 0x04
/**
   G.992.1 / ADSL operation over POTS overlapped spectrum (Annex A/G.992.1). */
#define XTSE_1_04_A_1_O  0x08
/**
   G.992.1 / ADSL operation over ISDN non-overlapped spectrum (Annex B/G.992.1). */
#define XTSE_1_05_B_1_NO 0x10
/**
   G.992.1 / ADSL operation over ISDN overlapped spectrum (Annex B/G.992.1). */
#define XTSE_1_06_B_1_O  0x20
/**
   G.992.1 / ADSL operation in conjunction with TCM-ISDN non-overlapped spectrum
   (Annex C/G.992.1). */
#define XTSE_1_07_C_1_NO 0x40
/**
   G.992.1 / ADSL operation in conjunction with TCM-ISDN overlapped spectrum
   (Annex C/G.992.1). */
#define XTSE_1_08_C_1_O  0x80


/*
   xTU Transmission System Enabling - Octet 2.
*/
/**
   G.992.2 / ADSL lite operation over POTS non-overlapped spectrum
   (Annex A/G.992.2). */
#define XTSE_2_01_A_2_NO 0x01
/**
   G.992.2 / ADSL lite operation over POTS overlapped spectrum
   (Annex B/G.992.2). */
#define XTSE_2_02_B_2_O  0x02
/**
   G.992.2 / ADSL lite operation in conjunction with TCM-ISDN non-overlapped
   spectrum (Annex C/G.992.2). */
#define XTSE_2_03_C_2_NO 0x04
/**
   G.992.2 / ADSL lite operation in conjunction with TCM-ISDN overlapped
   spectrum (Annex C/G.992.2). */
#define XTSE_2_04_C_2_O  0x08
/**
   Reserved. */
#define XTSE_2_05 0x10
/**
   Reserved. */
#define XTSE_2_06 0x20
/**
   Reserved. */
#define XTSE_2_07 0x40
/**
   Reserved. */
#define XTSE_2_08 0x80


/*
   xTU Transmission System Enabling - Octet 3.
*/
/**
   Reserved. */
#define XTSE_3_01 0x01
/**
 Reserved. */
#define XTSE_3_02 0x02
/**
   G.992.3 / ADSL2 operation over POTS non-overlapped spectrum
   (Annex A/G.992.3). */
#define XTSE_3_03_A_3_NO 0x04
/**
   G.992.3 / ADSL2 operation over POTS overlapped spectrum (Annex A/G.992.3). */
#define XTSE_3_04_A_3_O  0x08
/**
   G.992.3 / ADSL2 operation over ISDN non-overlapped spectrum
   (Annex B/G.992.3). */
#define XTSE_3_05_B_3_NO 0x10
/**
   G.992.3 / ADSL2 operation over ISDN overlapped spectrum (Annex B/G.992.3). */
#define XTSE_3_06_B_3_O  0x20
/**
   Reserved. */
#define XTSE_3_07 0x40
/**
   Reserved. */
#define XTSE_3_08 0x80


/*
   xTU Transmission System Enabling - Octet 4.
*/
/**
   G.992.4 / ADSL2 lite operation over POTS non-overlapped spectrum
   (Annex A/G.992.4). */
#define XTSE_4_01_A_4_NO 0x01
/**
   G.992.4 / ADSL2 lite operation over POTS overlapped spectrum (Annex A/G.992.4). */
#define XTSE_4_02_A_4_O  0x02
/**
   Reserved. */
#define XTSE_4_03 0x04
/**
   Reserved. */
#define XTSE_4_04 0x08
/**
   G.992.3 / ADSL2 All Digital Mode operation with non-overlapped spectrum
   (Annex I/G.992.3). */
#define XTSE_4_05_I_3_NO 0x10
/**
   G.992.3 / ADSL2 All Digital Mode operation with overlapped spectrum
   (Annex I/G.992.3). */
#define XTSE_4_06_I_3_O  0x20
/**
   G.992.3 / ADSL2 All Digital Mode operation with non-overlapped spectrum
   (Annex J/G.992.3). */
#define XTSE_4_07_J_3_NO 0x40
/**
   G.992.3 / ADSL2 All Digital Mode operation with overlapped spectrum
   (Annex J/G.992.3) */
#define XTSE_4_08_J_3_O  0x80


/*
   xTU Transmission System Enabling - Octet 5.
*/
/**
   G.992.4 / ADSL2 lite All Digital Mode operation with non-overlapped spectrum
   (Annex I/G.992.4). */
#define XTSE_5_01_I_4_NO 0x01
/**
   G.992.4 / ADSL2 lite All Digital Mode operation with overlapped spectrum
   (Annex I/G.992.4). */
#define XTSE_5_02_I_4_O  0x02
/**
   G.992.3 / ADSL2 Reach Extended operation over POTS, Mode 1 (non-overlapped,
   wide upstream/Annex L of G.992.3) */
#define XTSE_5_03_L_3_NO 0x04
/**
   G.992.3 / ADSL2 Reach Extended operation over POTS, Mode 2 (non-overlapped,
   narrow upstream/Annex L of G.992.3) */
#define XTSE_5_04_L_3_NO 0x08
/**
   G.992.3 / ADSL2 Reach Extended operation over POTS, Mode 3 (overlapped,
   wide upstream/Annex L of G.992.3) */
#define XTSE_5_05_L_3_O  0x10
/**
   G.992.3 / ADSL2 Reach Extended operation over POTS, Mode 4 (overlapped,
   narrow upstream/Annex L of G.992.3) */
#define XTSE_5_06_L_3_O  0x20
/**
   G.992.3 / ADSL2 Extended upstream operation over POTS non-overlapped spectrum
   (Annex M of G.992.3) */
#define XTSE_5_07_M_3_NO 0x40
/**
   G.992.3 / ADSL2 Extended upstream operation over POTS overlapped spectrum
   (Annex M of G.992.3) */
#define XTSE_5_08_M_3_O  0x80


/*
   xTU Transmission System Enabling - Octet 6.
*/
/**
   G.992.5 / ADSL2 Plus operation over POTS non-overlapped spectrum
   (Annex A/G.992.5). */
#define XTSE_6_01_A_5_NO 0x01
/**
   G.992.5 / ADSL2 Plus operation over POTS overlapped spectrum
   (Annex A/G.992.5). */
#define XTSE_6_02_A_5_O  0x02
/**
   G.992.5 / ADSL2 Plus operation over ISDN non-overlapped spectrum
   (Annex B/G.992.5). */
#define XTSE_6_03_B_5_NO 0x04
/**
   G.992.5 / ADSL2 Plus operation over ISDN overlapped spectrum
   (Annex B/G.992.5). */
#define XTSE_6_04_B_5_O  0x08
/**
   Reserved. */
#define XTSE_6_05 0x10
/**
   Reserved. */
#define XTSE_6_06 0x20
/**
   G.992.5 / ADSL2 Plus All Digital Mode operation with non-overlapped spectrum
   (Annex I/G.992.5). */
#define XTSE_6_07_I_5_NO 0x40
/**
   G.992.5 / ADSL2 Plus All Digital Mode operation with overlapped spectrum
   (Annex I/G.992.5). */
#define XTSE_6_08_I_5_O  0x80


/*
   xTU Transmission System Enabling - Octet 7.
*/
/**
   G.992.5 All Digital Mode operation with non-overlapped spectrum
   (Annex J of G.992.5). */
#define XTSE_7_01_J_5_NO 0x01
/**
   G.992.5 All Digital Mode operation with overlapped spectrum
   (Annex J of G.992.5). */
#define XTSE_7_02_J_5_O  0x02
/**
   G.992.5 Extended upstream operation over POTS non-overlapped spectrum
   (Annex M of G.992.5) */
#define XTSE_7_03_M_5_NO 0x04
/**
   G.992.5 Extended upstream operation over POTS overlapped spectrum
   (Annex M of G.992.5) */
#define XTSE_7_04_M_5_O  0x08
/**
   Reserved. */
#define XTSE_7_05 0x10
/**
   Reserved. */
#define XTSE_7_06 0x20
/**
   Reserved. */
#define XTSE_7_07 0x40
/**
   Reserved. */
#define XTSE_7_08 0x80


/*
   xTU Transmission System Enabling - Octet 8.
*/
/**
   G.993.2 Region A (North America) (Annex A/G.993.2). */
#define XTSE_8_01_A 0x01
/**
   G.993.2 Region B (Europe) (Annex B/G.993.2). */
#define XTSE_8_02_B 0x02
/**
   G.993.2 Region C (Japan) (Annex C/G.993.2). */
#define XTSE_8_03_C 0x04
/**
   Reserved. */
#define XTSE_8_04 0x08
/**
   Reserved. */
#define XTSE_8_05 0x10
/**
   Reserved. */
#define XTSE_8_06 0x20
/**
   Reserved. */
#define XTSE_8_07 0x40
/**
   G.993.1 all modes
   This bit is intermediate and not from the G.997.1 */
#define XTSE_8_08 0x80


/**
   Structure to exchange Auxiliary near end Line Inventory information
   according to G.993.2
*/
typedef struct
{
   /**
   Number of used bytes in the buffer \ref pData. */
   DSL_IN DSL_uint32_t nLength;
   /**
   Auxiliary Inventory bytes.
   Though G.993.2 does not define an upper limit of this field, it is
   limited to \ref DSL_G993_LI_MAXLEN_AUX in DSl-API for the near end. */
   DSL_IN DSL_uint8_t pData[DSL_G993_LI_MAXLEN_AUX];
} DSL_AuxInventoryNe_t;

/**
   This structure is used to set the inventory data for the Near End which will
   be used later on by the device when inventory is requested by the Far End
   and if the DSL CPE API ioctl \ref DSL_FIO_G997_LINE_INVENTORY_GET is called.
   Therefor it only includes the data that has to be specified by the upper
   layer software for the initialization of the DSL Library.
*/
typedef struct
{
   /**
   System Vendor ID
   The meaning of this parameter is equal to the according parameter in
   \ref DSL_G997_LineInventory_t */
   DSL_IN DSL_uint8_t SystemVendorID[DSL_G997_LI_MAXLEN_VENDOR_ID];
   /**
   Version Number
   The meaning of this parameter is equal to the according parameter in
   \ref DSL_G997_LineInventory_t */
   DSL_IN DSL_uint8_t VersionNumber[DSL_G997_LI_MAXLEN_VERSION];
   /**
   Serial Number
   The meaning of this parameter is equal to the according parameter in
   \ref DSL_G997_LineInventory_t */
   DSL_IN DSL_uint8_t SerialNumber[DSL_G997_LI_MAXLEN_SERIAL];
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   /**
   Auxiliary inventory information according to
   ITU G.993.2 chapter 11.2.3.6 */
   DSL_IN DSL_AuxInventoryNe_t Auxiliary;
#endif
} DSL_G997_LineInventoryNeData_t;

/**
   This structure is used to set the inventory data for the Near End.
   This structure has to be used for ioctl \ref DSL_FIO_G997_LINE_INVENTORY_SET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains line inventory configuration settings */
   DSL_IN DSL_G997_LineInventoryNeData_t data;
} DSL_G997_LineInventoryNe_t;

/**
   This structure is used to get the inventory data for both ATU-C and ATU-R.
   See chapter 7.4 in G.997.1.
   \remarks This structure will be used for Near-End and Far-End handling.
            Therefore the parameter comments are done for both access
            possibilities.
*/
typedef struct
{
   /**
   ATU-C/ATU-R G.994.1 Vendor ID (see chapter 7.4.1/7.4.2 in G.997.1)
   The ATU-C/ATU-R G.994.1 Vendor ID is the Vendor ID as inserted by the
   ATU-C/ATU-R in the G.994.1 CL/CLR message. It consists of 8 binary octets,
   including a country code followed by a (regionally allocated) provider code,
   as defined in Recommendation T.35.
   Table 7 4/G.997.1 Vendor ID information block (8 octets)
      T.35 country code (2 octets)
      T.35 provider code (vendor identification) (4 octets)
      T.35 provider oriented code (vendor revision number) (2 octets)
   The G.994.1 Vendor ID should typically identify the vendor of the ATU-C/ATU-R
   G.994.1 functionality, whether implemented in hardware or software. It is
   not intended to indicate the system integrator. Further details are defined
   in Recommendation G.994.1. */
   DSL_OUT DSL_uint8_t G994VendorID[DSL_G997_LI_MAXLEN_VENDOR_ID];
   /**
   ATU-C/ATU-R System Vendor ID (see chapter 7.4.3/7.4.4 in G.997.1)
   The ATU-C/ATU-R System Vendor ID is the Vendor ID as inserted by the
   ATU-C/ATU-R in the Overhead Messages (G.992.3 and G.992.4). It consists of
   8 binary octets, with same format as the ATU-C/ATU-R G.994.1 Vendor ID.
   The ATU-C/ATU-R System Vendor ID should typically identify the ATU-C/ATU-R
   system integrator. In this context, the system integrator usually refers to
   the vendor of the smallest field-replaceable unit. As such, the ATU-C/ATU-R
   System Vendor ID may not be the same as the ATU-C/ATU-R G.994.1 Vendor ID. */
   DSL_OUT DSL_uint8_t SystemVendorID[DSL_G997_LI_MAXLEN_VENDOR_ID];
   /**
   ATU-C/ATU-R Version Number (see chapter 7.4.4.1/7.4.4.2 of G.997.1)
   The ATU-C/ATU-R version number is the version number as inserted by the
   ATU-C/ATU-R in the Overhead Messages (G.992.3 and G.992.4). It is for
   version control and is vendor specific information. It consists of up to
   16 binary octets. */
   DSL_OUT DSL_uint8_t VersionNumber[DSL_G997_LI_MAXLEN_VERSION];
   /**
   ATU-C/ATU-R Serial Number (see chapter 7.4.4.3/7.4.4.4 of G.997.1)
   The ATU-C/ATU-R serial number is the serial number as inserted by the
   ATU-C/ATU-R in the Overhead Messages (G.992.3 and G.992.4). It is vendor
   specific information. It consists of up to 32 ASCII characters.
   Note that the combination of System Vendor ID and serial number creates a
   unique number for each ATU-C/ATU-R. */
   DSL_OUT DSL_uint8_t SerialNumber[DSL_G997_LI_MAXLEN_SERIAL];
   /**
   ATU-C/ATU-R Self-Test Result (see chapter 7.4.4.5/7.4.4.6 of G.997.1)
   This parameter defines the ATU-C/ATU-R self-test result. It is coded as a
   32-bit integer. The most significant octet of the self-test result is 00hex
   if the self-test passed and 01hex if the self-test failed.
   The interpretation of the other octets is vendor discretionary and can be
   interpreted in combination with G.994.1 and system Vendor IDs. */
   DSL_OUT DSL_uint32_t SelfTestResult;
   /**
   ATU-C/ATU-R ADSL Transmission System Capabilities (see chapter
   7.4.4.7/7.4.4.8 of G.997.1)
   This parameter defines the ATU-C/ATU-R transmission system capability list
   of the different coding types. It is coded in a bit map representation with
   the bits defined in chapter 7.3.1.1.1 of G.997.1. This parameter may be
   derived from the handshaking procedures defined in Recommendation G.994.1 */
   DSL_OUT DSL_uint8_t XTSECapabilities[DSL_G997_NUM_XTSE_OCTETS];
} DSL_G997_LineInventoryData_t;

/**
   This structure is used to get the inventory data for both ATU-C and ATU-R.
   See chapter 7.4 in G.997.1.
   This structure has to be used for ioctl \ref DSL_FIO_G997_LINE_INVENTORY_GET

   \remarks This structure will be used for Near-End and Far-End handling.
            Therefore the parameter comments are done for both access
            possibilities.
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (far-end/near-end) the function will
   apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains line inventory configuration settings */
   DSL_OUT DSL_G997_LineInventoryData_t data;
} DSL_G997_LineInventory_t;

/**
   xTU Transmission System Enabling.
   The data within this structure is defined in accordance to chapter 7.3.1.1
   of G.997.1 but the meaning is slightly different because the CO side always
   defines the allowed transmission modes. Nevertheless this configuration can
   be used also on CPE side to restrict the used transmission modes to one of
   the (CO) allowed ones.
   This structure is both used to read (GET), write (SET) configuration and
   to read the current line status in showtime (only one bit is set in this
   case).
   This structure has to be used for ioctl
   - \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET
   - \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_GET
   - \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET

   \note By default all allowed transmission modes should be enabled.
*/
typedef struct
{
   /**
   xTU Transmission System Enabling.
   This configuration parameter defines the transmission system coding types
   to be allowed by the near-end xTU on this line.
   It is coded in a bitmap representation (0 if not allowed, 1 if allowed). */
   DSL_CFG DSL_uint8_t XTSE[DSL_G997_NUM_XTSE_OCTETS];
} DSL_G997_XTUSystemEnablingData_t;

/**
   xTU Transmission System Enabling.
   This structure has to be used for ioctl
   - \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET
   - \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_GET
   - \ref DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET

   \note By default all allowed transmission modes should be enabled.
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains/returns configuration settings */
   DSL_CFG DSL_G997_XTUSystemEnablingData_t data;
} DSL_G997_XTUSystemEnabling_t;


/**
   Power Management State Enabling (PMMode).
   This configuration parameter defines the line states the xTU-C or xTU-R may
   autonomously transition to on this line. It is coded in a bitmap
   representation (0 if not allowed, 1 if allowed) with following definition:
*/
typedef enum
{
   /** L3 state (Idle state) */
   DSL_G997_PMMODE_BIT_L3_STATE = 0x01,
   /** L1/L2 state (Low power state) */
   DSL_G997_PMMODE_BIT_L1_L2_STATE = 0x02
} DSL_G997_PMMode_t;


/**
   This definitions defines all possible power management states.
*/
typedef enum
{
   /**
   Power management state is not available */
   DSL_G997_PMS_NA = -1,
   /**
   L0 - Synchronized
   This Line state (L0) is when the Line has full transmission (i.e.
   showtime). */
   DSL_G997_PMS_L0 = 0,
   /**
   L1 - Power Down Data transmission
   This line state (L1) is when there is transmission on the line but the net
   data rate is reduced (e.g. only for OAM and higher layer connection and
   session control). This state applies to G.992.2 only. */
   DSL_G997_PMS_L1 = 1,
   /**
   L2 - Power Down Data transmission
   This line state (L2) is when there is transmission on the line but the net
   data rate is reduced (e.g. only for OAM and higher layer connection and
   session control). This state applies to G.992.3 and G.992.4 only. */
   DSL_G997_PMS_L2 = 2,
   /**
   L3 - No-power
   This Line state (L3) is when there is No Power transmitted on the line at
   all. */
   DSL_G997_PMS_L3 = 3
} DSL_G997_PowerManagement_t;

/**
   Line power management state (see chapter 7.5.1.2 of G.997.1).
   The Line has four possible power management states, numbered 0 to 3 and
   corresponding as defined in \ref DSL_G997_PowerManagement_t.
*/
typedef struct
{
   /**
   Specifies the power mode that has to be forced on the line. */
   DSL_OUT DSL_G997_PowerManagement_t nPowerManagementStatus;
} DSL_G997_PowerManagementStatusData_t;

/**
   Line power management state.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_POWER_MANAGEMENT_STATUS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains power management status data */
   DSL_OUT DSL_G997_PowerManagementStatusData_t data;
} DSL_G997_PowerManagementStatus_t;

/**
   This parameter represents the last successful transmitted initialization
   state in the downstream/upstream direction in the last full initialization
   performed on the line. Initialization states are defined in the individual
   ADSL Recommendations and are counted from 0 (if G.994.1 is used) or 1
   (if G.994.1 is not used) up to Showtime. This parameter must be interpreted
   along with the ADSL Transmission System.
   This parameter is available only when, after a failed full initialization,
   the line diagnostics procedures are activated on the line. Line diagnostics
   procedures can be activated by the operator of the system (through the Line
   State Forced line configuration parameter) or autonomously by the ATU-C
   or ATU-R.
*/
typedef struct
{
   /**
   Last successful transmitted initialization state */
   DSL_OUT DSL_uint16_t nLastStateTransmitted;
} DSL_G997_LastStateTransmittedData_t;

/**
   This parameter represents the last successful transmitted initialization
   state in the downstream/upstream direction in the last full initialization
   performed on the line.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_LAST_STATE_TRANSMITTED_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains power management status data */
   DSL_OUT DSL_G997_LastStateTransmittedData_t data;
} DSL_G997_LastStateTransmitted_t;

/**
   Bit Allocation per Subcarrier (see chapter 7.5.1.21.1 and 7.5.1.21.2 of G.997.1)
   This structure defines the upstream/downstream bits allocation table per
   subcarrier. It is an array of integer values in the 0 to 15 range for
   subcarriers 0 to NSds.
   The reported bits of subcarriers out of the downstream MEDLEY set shall be
   set to 0.
*/
typedef struct
{
   /**
   Returns the bit allocation for the number of subcarriers (NSC) */
   DSL_OUT DSL_G997_NSCData8_t bitAllocationNsc;
} DSL_G997_BitAllocationNscData_t;

/**
   Bit Allocation per Subcarrier.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_BIT_ALLOCATION_NSC_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains bit allocation status data */
   DSL_OUT DSL_G997_BitAllocationNscData_t data;
} DSL_G997_BitAllocationNsc_t;

/**
   Gain Allocation per Subcarrier (see chapter 7.5.1.29.3 and 7.5.1.29.4 of G.997.1).
   This structure defines the upstream/downstream gains allocation per
   subcarrier. It is an array of integer values in the of 0 to 4093 range
   for subcarriers 0 to NSCds-1.
   The gain value is represented as a multiple of 1/512 on linear scale
   therefore the following range has been defined:
   - Min. value: 20*log(1/512) = -54,2 dB
   - Max. value: 20*log(4093/512) = 18,0 dB
*/
typedef struct
{
   /**
   Returns the gain allocation for the number of subcarriers (NSC) */
   DSL_OUT DSL_G997_NSCData16_t gainAllocationNsc;
} DSL_G997_GainAllocationNscData_t;

/**
   Gain Allocation per Subcarrier.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_GAIN_ALLOCATION_NSC_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains gain allocation status data */
   DSL_OUT DSL_G997_GainAllocationNscData_t data;
} DSL_G997_GainAllocationNsc_t;

/**
   SNR per Subcarrier (see chapter 7.5.1.28.3 and 7.5.1.28.6 of G.997.1).
   This parameter is an array of real values in dB for downstream SNR(f).
   Each array entry represents the SNR(f = i * SNRGds * delta_f) value for a
   particular subcarrier group index i, ranging from 0 to MIN(NSds ,511).
   The SNR(f) is represented as (-32 + snr(i)/2), where snr(i) is an unsigned
   integer in the range from 0 to 254. A special value snr(i)=255 indicates
   that no measurement could be done for this subcarrier group because it is
   out of the passband or that the SNR is out of range to be represented.
*/
typedef struct
{
   /**
   Returns the bit allocation for the number of subcarriers (NSC) */
   DSL_OUT DSL_G997_NSCData8_t snrAllocationNsc;
} DSL_G997_SnrAllocationNscData_t;

/**
   SNR per Subcarrier.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_SNR_ALLOCATION_NSC_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains snr allocation status data */
   DSL_OUT DSL_G997_SnrAllocationNscData_t data;
} DSL_G997_SnrAllocationNsc_t;

/**
   Counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   Number of seconds elapsed in the current 15 min interval. */
   DSL_uint16_t   nCurr15MinTime;
   /**
   Number of seconds elapsed in the current 1 day interval. */
   DSL_uint32_t   nCurr1DayTime;
   /**
   15 minute counters threshold crossing indication.*/
   DSL_uint32_t   n15Min;
   /**
   1 day counters threshold crossing indication.*/
   DSL_uint32_t   n1Day;

} DSL_G997_ThresholdCrossingData_t;


/**
   Direction (ATU-C/ATU-R) specific line failures bit field definitions.
   The meaning of all bits is as follows:
   0: There is NO failure actually present on the line
   1: There is a failure is actually detected on the line
*/
typedef enum
{
   /* Near-end and Far-end failures */
   /* Cleaned. */
   DSL_G997_LINEFAILURE_CLEANED = 0x00000000,
   /**
   Loss-of-power failure
   LPR_NE:
   An LPR failure is declared after 2.5 +- 0.5 s of contiguous near-end LPR
   primitive presence. An LPR failure is cleared after 10 +- 0.5 s of no
   near-end LPR primitive presence.
   LPR_FE:
   A far end Loss of power - LPR-FE failure is declared after the occurrence
   of a far end LPR primitive followed by 2.5 +- 0.5 s of contiguous near end
   LOS defect. A far end LPR failure is cleared after 10 +- 0.5 s of no near end
   LOS defect. */
   DSL_G997_LINEFAILURE_LPR = 0x00000001,
   /**
   Loss-of-frame failure
   A (far-end) LOF failure is declared after 2.5 +- 0.5 s of contiguous
   (RDI) SEF defect, except when an (far-end) LOS defect or failure is present
   (see LOS definition above). A (far-end) LOF failure is cleared when
   (far-end) LOS failure is declared, or after 10 +- 0.5 s of no (RDI) SEF
   defect. */
   DSL_G997_LINEFAILURE_LOF = 0x00000002,
   /**
   Loss-of-signal failure
   A (far-end) LOS failure is declared after 2.5 +- 0.5 s of contiguous (far-end)
   LOS defect, or, if (far-end) LOS defect is present when the criteria for
   (far-end) LOF failure declaration have been met (see LOF definition below).
   A (far-end) LOS failure is cleared after 10 +- 0.5 s of no (far-end) LOS
   defect. */
   DSL_G997_LINEFAILURE_LOS = 0x00000004,
   /**
   Loss of margin failure
   Regrading to the configuration of the minimum noise margin that has been
   set by the CO this failure will be generated in case of minimum margin for
   ATU-C/ATU-R is not guaranteed in the system anymore. */
   DSL_G997_LINEFAILURE_LOM = 0x00000008,
   /**
   Loss of link
   LOL indicates a loss-of-link condition according to RFC2662. */
   DSL_G997_LINEFAILURE_LOL = 0x00000010,
   /**
   Excessive Severe Errors (ESE) Failure
   ESE indicates 10 seconds of ES. */
   DSL_G997_LINEFAILURE_ESE = 0x00000020
} DSL_G997_BF_LineFailures_t;

/**
   Direction specific line failures (see chapter 7.1.1 of G.997.1).
   This Structure is used to read (GET) or write (SET) configuration and to get
   the current status of line failures.
   \note This structure will be used for downstream (Far-End, xTU-R) and
         upstream (Near-End, xTU-C) handling.
         Therefore the parameter comments are done for both access possibilities.
   \note All bits within this bit field have the following meaning:
         0: Line failure event is NOT signalled
         1: Line failure event will be signalled by callback function
*/
typedef struct
{
   /**
   Returns the bit allocation for the number of subcarriers (NSC) */
   DSL_CFG DSL_G997_BF_LineFailures_t nLineFailures;
} DSL_G997_LineFailuresData_t;

/**
   Direction specific line failures (see chapter 7.1.1 of G.997.1).
   This Structure is used to read (GET) or write (SET) configuration and to get
   the current status of line failures.
   This structure has to be used for ioctl
   - \ref DSL_FIO_G997_ALARM_MASK_LINE_FAILURES_CONFIG_SET
   - \ref DSL_FIO_G997_ALARM_MASK_LINE_FAILURES_CONFIG_GET
   - \ref DSL_FIO_G997_LINE_FAILURES_STATUS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains line failure config/status data */
   DSL_CFG DSL_G997_LineFailuresData_t data;
} DSL_G997_LineFailures_t;

/**
   ATM and PTM Data Path failures for far-end and near-end
   (see chapter 7.1.4 'ATM failures' and 7.1.5 'PTM failures' of G.997.1).
   \remarks This structure will be used for downstream and upstream handling.
            Therefore the parameter comments are done for both access
            possibilities.
   \remarks The enums corresponds to bit field definitions.
*/
typedef enum
{
   /* Cleaned. */
   DSL_G997_DATAPATHFAILURE_CLEANED = 0x00000000,
   /**
   No Cell Delineation (NCD(-FE)) failure
   An NCD(-FE) failure is declared when an NCD(-FE) anomaly persists for more
   than 2.5 +- 0.5 s after the start of SHOWTIME. An NCD(-FE) failure terminates
   when no NCD(-FE) anomaly is present for more than 10 +- 0.5 s. */
   DSL_G997_DATAPATHFAILURE_NCD = 0x00000001,
   /**
   Loss of Cell Delineation (LCD(-FE)) failure
   An LCD(-FE) failure is declared when an LCD(-FE) defect persists for more
   than 2.5 +- 0.5 s. An LCD(-FE) failure terminates when no LCD(-FE) defect is
   present for more than 10 +- 0.5 s. */
   DSL_G997_DATAPATHFAILURE_LCD = 0x00000002,
   /**
   Out of Sync (OOS(-FE)) failure
   An OOS failure is declared when an oos-n anomaly persists for more
   than 2.5 + 0.5 s. An OOS failure terminates when no oos-n anomaly
   is present for more than 10 + 0.5 s. */
   DSL_G997_DATAPATHFAILURE_OOS = 0x00000004
} DSL_G997_BF_DataPathFailures_t;

/**
   ATM Data Path failures for far-end and near-end (see chapter 7.1.4 of G.997.1).
   This Structure is used to read (GET) or write (SET) configuration and to get
   the current status of data path failures.
   \note This structure will be used for downstream and upstream handling.
         Therefore the parameter comments are done for both access possibilities.
   \note All bits within this bit field have the following meaning:
         0: Line failure event is NOT signalled
         1: Line failure event will be signalled by callback function
*/
typedef struct
{
   /**
   Returns the bit allocation for the number of subcarriers (NSC) */
   DSL_CFG DSL_G997_BF_DataPathFailures_t nDataPathFailures;
} DSL_G997_DataPathFailuresData_t;

/**
   ATM Data Path failures for far-end and near-end (see chapter 7.1.4 of G.997.1).
   This Structure is used to read (GET) or write (SET) configuration and to get
   the current status of data path failures.
   This structure has to be used for ioctl
   - \ref DSL_FIO_G997_ALARM_MASK_DATA_PATH_FAILURES_CONFIG_SET
   - \ref DSL_FIO_G997_ALARM_MASK_DATA_PATH_FAILURES_CONFIG_GET
   - \ref DSL_FIO_G997_DATA_PATH_FAILURES_STATUS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains data path failure config/status data */
   DSL_CFG DSL_G997_DataPathFailuresData_t data;
} DSL_G997_DataPathFailures_t;

/**
   Defines all possible power management transitions that could be forced on
   the line.
*/
typedef enum
{
   /**
   Force the line to transition from the L3 idle state to the L0 full-on
   state. This transition requires the (short) initialization procedures.
   After reaching the L0 state, the line may transition into or exit from the
   L2 low power state (if L2 state is enabled). If the L0 state is not reached
   (after a vendor discretionary number of retries and/or within a vendor
   discretionary timeout), then an Initialization Failure occurs. Whenever the
   line is in the L3 state, attempts shall be made to transition to the L0 state
   until it is forced into another state through this configuration parameter. */
   DSL_G997_PMSF_L3_TO_L0 = 0,
   /**
   This transition is not possible on CPE side. */
   DSL_G997_PMSF_L0_TO_L2 = 2,
   /**
   Force the line to transition from the L0 full-on or L2 low power state
   to the L3 idle state. This transition requires the (orderly) shutdown
   procedure. After reaching the L3 state, the line shall remain in the L3 idle
   state until it is forced into another state through this configuration
   parameter. */
   DSL_G997_PMSF_LX_TO_L3 = 3
} DSL_G997_PowerManagementStateForce_t;

/**
   Power management state forced (PMSF) (see chapter 7.3.1.1.3).
   This configuration parameter defines the line states to be forced by the
   near-end xTU on this line. It is coded as an integer value with following
   definition.
   Forced line state transitions require the line to enter or exit from the L3
   idle state. These transitions are not restricted by the Line state enabling
   parameter value.
   \note  This configuration parameter maps to the AdminStatus of the line,
          which is part of the GeneralInformationGroup object group specified
          in RFC 2233, and may not need to be duplicated in the ADSL MIB. See
          also RFC2662. The Administrative Status of the line is UP when the
          line is forced to the L0 state and is DOWN when the line is forced to
          the L3 state
*/
typedef struct
{
   /**
   Specifies the power mode that has to be forced on the line. */
   DSL_IN DSL_G997_PowerManagementStateForce_t nPowerManagementState;
} DSL_G997_PowerManagementStateForcedTriggerData_t;

/**
   Power management state forced (PMSF).
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_POWER_MANAGEMENT_STATE_FORCED_TRIGGER
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains power management control data */
   DSL_G997_PowerManagementStateForcedTriggerData_t data;
} DSL_G997_PowerManagementStateForcedTrigger_t;

/**
   Channel status parameters (see chapter 7.5.2 of G.997.1).
   \remarks This structure will be used for downstream and upstream handling.
            Therefore the parameter comments are done for both access possibilities.
*/
typedef struct
{
   /**
   Actual Data Rate.
   This parameter reports the actual net data rate the bearer channel is
   operating at excluding rate in L1 and L2 states. In L1 or L2 states, the
   parameter contains the net data rate in the previous L0 state. The data rate
   is coded in bit/s. */
   DSL_OUT DSL_uint32_t ActualDataRate;
   /**
   Previous Data Rate.
   This parameter reports the previous net data rate the bearer channel was
   operating at just before the latest rate change event occurred excluding all
   transitions between L0 state and L1 or L2 states. A rate change can occur at
   a power management state transition, e.g., at full or short initialization,
   fast retrain, or power down or at a dynamic rate adaptation. The rate is
   coded in bit/s. */
   DSL_OUT DSL_uint32_t PreviousDataRate;
   /**
   Actual Interleaving Delay.
   This parameter is the actual one-way interleaving delay introduced by the
   PMS-TC between the alpha and beta reference points excluding delay in L1 and
   L2 state. In L1 and L2 state, the parameter contains the interleaving delay
   in the previous L0 state. This parameter is derived from the S and D
   parameters as [S*D] / 4 ms, where "S" is the Symbols per codeword, and "D"
   is the "Interleaving Depth" and [x] denotes rounding to the higher integer.
   \attention The Actual Interleaving Delay is coded in multiple of 1/100 ms.
              For example: 25 means 0.25 ms */
   DSL_OUT DSL_uint32_t ActualInterleaveDelay;
   /**
   Actual Impulse Noise Protection.
   This parameter reports the actual impulse noise protection (INP) on the
   bearer channel in the L0 state. In the L1 or L2 state, the parameter
   contains the INP in the previous L0 state. For ADSL, this value is computed
   according to the formula specified in the relevant Recommendation based on
   the actual framing parameters. For G.993.2, the method to report this value
   is according to the INPREPORT parameter. The value is coded in fractions of
   DMT symbols with a granularity of 0.1 symbols. The range is from 0 to 25.4.
   A special value indicates an ACTINP higher than 25.4. */
   DSL_OUT DSL_uint8_t ActualImpulseNoiseProtection;
} DSL_G997_ChannelStatusData_t;

/**
   Channel status parameters.
   This structure has to be used for ioctl \ref DSL_FIO_G997_CHANNEL_STATUS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains channel status data */
   DSL_OUT DSL_G997_ChannelStatusData_t data;
} DSL_G997_ChannelStatus_t;

/**
   Maximum Bit Error Ratio
*/
typedef enum
{
   /** Bit Error Rate of 1E-3 */
   DSL_G997_MAX_BER_3 = 0,
   /** Bit Error Rate of 1E-5 */
   DSL_G997_MAX_BER_5 = 1,
   /** Bit Error Rate of 1E-7 */
   DSL_G997_MAX_BER_7 = 2
} DSL_G997_MaxBER_t;

/**
   Common channel configuration parameters (see chapter 7.3.2.1 of G.997.1).
   These data rate parameters refer to the transmit direction for both the
   ATU-C and the ATU-R and apply to the configuration of an individual upstream
   or downstream bearer channel. The two data rate parameters define the data
   rate minimum and maximum bounds as specified by the operator of the system
   (the operator of the ATU-C). It is assumed that the ATU C and the ATU-R will
   interpret the value set by the operator as appropriate for the specific
   implementation of ADSL between the ATU C and the ATU R in setting the line
   rates. This model defined in this interface makes no assumptions about the
   possible range of these attributes. The Network Management System used by
   the operator to manage the ATU-R and the ATU-C may implement its own limits
   on the allowed values for the desired bit rate parameters based on the
   particulars of the system managed. The definition of such a system is
   outside the scope of this model.
*/
typedef struct
{
   /**
   Minimum Data Rate.
   This parameter specifies the minimum net data rate for the bearer
   channel as desired by the operator of the system. The rate is coded in
   bit/s. */
   DSL_CFG DSL_uint32_t MinDataRate;
   /**
   Maximum Data Rate.
   This parameter specifies the maximum net data rate for the bearer
   channel as desired by the operator of the system. The data rate is coded in
   bit/s. */
   DSL_CFG DSL_uint32_t MaxDataRate;
   /**
   Maximum Interleaving Delay.
   This parameter is the maximum one-way interleaving delay introduced by the
   PMS-TC between the alpha and the beta reference points, in the direction of
   the bearer channel. The one-way interleaving delay is defined in individual
   ADSL Recommendations as  S*D  /4 ms, where "S" is the S factor and "D" is the
   "Interleaving Depth" and  x  denotes rounding to the higher integer.
   The xTU's shall choose the S and D values such that the actual one-way
   interleaving delay (see Actual Interleaving Delay status parameter in
   7.5.2.3) is less or equal than the configured Maximum Interleaving Delay.
   The delay is coded in ms, with the value 0, 1, and 255 special values. The
   value 0 indicates no delay bound is being imposed. The value 1 indicates
   the Fast Latency Path shall be used in the G.992.1 operating mode and S
   and D shall be selected such that S   1 and D = 1 in ITU T Recs G.992.2,
   G.992.3, G.992.4, G.993.2 operating modes. The value 255 indicates a delay
   bound of 1 ms in ITU-T Recs G.993.2.

   \note    A single Maximum Delay value is configured. As a consequence, xTU's
            supporting multiple xDSL Recommendations will use the configured
            value regardless of the operating mode actually being selected at
            line initialization. */
   DSL_CFG DSL_uint16_t MaxIntDelay;
   /**
   Minimum Impulse Noise Protection.
   This parameter specifies the minimum impulse noise protection for the
   bearer channel. The impulse noise protection is expressed in fraction of
   DMT symbols. The value is coded as unsigned integer with steps of 0.5.
   Valid values are:
   + 0 (coded as 0) to switch of the Impulse Noise Protection
   + 0.5 (coded as 1)
   + 1 (coded as 2) to 16 (coded as 32) by step of 1
   \note Values that are defined by the range but not by the G.997.1 (for
         example 5.5) are automatically set the next higher valid value (in
         the example case 6.0) and a warning will be returned in addition.
   \note For different devices and different transmission modes there might be
         restrictions of the values and/or range. In this case the next best
         fitting (higher) value will be chosen internally to program the device
         and a warning will be returned in addition. */
   DSL_CFG DSL_uint8_t MinINP;
   /**
   Maximum Bit Error Ratio.
   This parameter specifies the maximum bit error ratio for the bearer
   channel as desired by the operator of the system. The bit error ratio can
   take the values 1E-3, 1E-5 or 1E-7.
   \note
   ATU's supporting multiple ADSL Recommendations may use or ignore the
   configured value depending on the operating mode actually being selected at
   line initialization. In G.992.3, the ATU's will use the configured value.
   In G.992.1, ATUs operate with the Maximum Bit Error Ratio fixed to 1E-7,
   regardless of the configured value. */
   DSL_CFG DSL_G997_MaxBER_t MaxBER;
} DSL_G997_ChannelConfigData_t;

/**
   This structure has to be used for ioctl.
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains all necessary configuration data */
   DSL_CFG DSL_G997_ChannelConfigData_t data;
} DSL_G997_ChannelConfig_t;

#define DSL_G997_MAX_VALUE_FOR_MIN_INP 32

/**
   Data rate threshold upshift/downshift (see chapter 7.3.2.6.1/7.3.2.6.2
   of G.997.1).
   \attention The implementation within this DSL CPE API is designed to use
              relative values values for upshift/downshift data rates.
              If entering showtime the datarate will be stored and later on
              compared with new data rates in accordance to the upshift/downshift
              margins to decide whether to generate a callback event or not.
*/
typedef struct
{
   /**
   This parameter is a threshold on the net data rate upshift achieved over one
   or more bearer channel data rate adaptations. An upshift rate change alarm
   (event) is triggered when the actual data rate exceeds the data rate at the
   last entry into showtime by more than the threshold. The data rate threshold
   is coded in bit/s.*/
   DSL_CFG DSL_uint32_t nDataRateThresholdUpshift;
   /**
   This parameter is a threshold on the net data rate downshift achieved over
   one or more bearer channel data rate adaptations. A downshift rate change
   alarm (event) is triggered when the actual data rate is below the data rate
   at the last entry into showtime by more than the threshold. The data rate
   threshold is coded in bit/s.*/
   DSL_CFG DSL_uint32_t nDataRateThresholdDownshift;
} DSL_G997_ChannelDataRateThresholdData_t;

/**
   Data rate threshold upshift/downshift.
   This structure has to be used for ioctl
   - \ref DSL_FIO_G997_CHANNEL_DATA_RATE_THRESHOLD_CONFIG_SET
   - \ref DSL_FIO_G997_CHANNEL_DATA_RATE_THRESHOLD_CONFIG_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains channel datarate threshold configuration data */
   DSL_G997_ChannelDataRateThresholdData_t data;
} DSL_G997_ChannelDataRateThreshold_t;

/**
   Datarate threshold crossing alarm (event) types.
*/
typedef enum
{
   /**
   Current datarate has exceeded specified nDataRateThresholdUpshift */
   DSL_G997_DATARATE_THRESHOLD_UPSHIFT = 0,
   /**
   Current datarate has exceeded specified nDataRateThresholdDownshift */
   DSL_G997_DATARATE_THRESHOLD_DOWNSHIFT = 1
} DSL_G997_DataRateThresholdCrossingType_t;

/**
   Data rate shift threshold crossing indication structure.
   Also refer to the description for \ref DSL_G997_ChannelDataRateThresholdData_t
*/
typedef struct
{
   /**
   Specifies the alarm (event) type for data rate threshold crossing type */
   DSL_G997_DataRateThresholdCrossingType_t nDataRateThresholdType;
} DSL_G997_DataRateShiftThresholdCrossingData_t;

/**
   Specifies the possible states for the line transmission status.
*/
typedef enum
{
   /**
   Line transmission state: available */
   DSL_G997_LINE_TRANSMISSION_AVAILABLE = 0,
   /**
   Line transmission state: unavailable */
   DSL_G997_LINE_TRANSMISSION_NOT_AVAILABLE = 1,
   /**
   Line transmission state: not supported */
   DSL_G997_LINE_TRANSMISSION_NOT_SUPPORTED = 2
} DSL_G997_LineTransmission_t;

/**
   Line transmission states (see chapter 7.2.7.1 of G.997.1).
   The transmission state is determined from filtered SES/non SES data.
   The definition of unavailable state is defined in 7.2.1.1.5. An xDSL Line is
   available when it is not unavailable.
*/
typedef struct
{
   /**
   Specifies the current line transmission status according to G.997.1. */
   DSL_OUT DSL_G997_LineTransmission_t nLineTransmissionStatus;
} DSL_G997_LineTransmissionStatusData_t;

/**
   Line transmission status.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_LINE_TRANSMISSION_STATUS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains line transmission status data */
   DSL_OUT DSL_G997_LineTransmissionStatusData_t data;
} DSL_G997_LineTransmissionStatus_t;


/** Maximum number of retries to reach showtime before the event will be generated.
    Each exception that leads to an LINIT value according to
    \ref DSL_G997_LineInit_t will be counted as one try. */
#define DSL_G997_LINIT_RETRIES    3

/**
   Initialization Success/Failure Cause.
*/
typedef enum
{
   /**
   Successful */
   LINIT_SUCCESSFUL = 0,
   /**
   Configuration error.
   This error occurs with inconsistencies in configuration parameters.
   E.g., when the line is initialized in an ADSL Transmission system where
   an ATU does not support the configured Maximum Delay or the configured
   Minimum or Maximum Data Rate for one or more bearer channels. */
   LINIT_CONFIG_ERROR = 1,
   /**
   Configuration not feasible on the line.
   This error occurs if the Minimum Data Rate cannot be reached on the line
   with the Minimum Noise Margin, Maximum PSD level, Maximum Delay and Maximum
   Bit Error Ratio for one or more bearer channels. */
   LINIT_CONFIG_NOT_FEASIBLE = 2,
   /**
   Communication problem.
   This error occurs e.g. due to corrupted messages or bad syntax messages or
   if no common mode can be selected in the G.994.1 handshaking procedure or
   due to a timeout.*/
   LINIT_COMMUNICATION_PROBLEM = 3,
   /**
   No peer ATU detected.
   This error occurs if the peer ATU is not powered or not connected or if the
   line is too long to allow detection of a peer ATU. */
   LINIT_NO_PEER_XTU = 4,
   /**
   Any other or unknown Initialization Failure cause. */
   LINIT_UNKNOWN = 5

} DSL_G997_LineInit_t;

/**
   More detailed Initialization Success/Failure cause.
   This is not standardized in G.997.1.
*/
typedef enum
{
   /**
   Successful or no specific subfailure available */
   LINIT_SUB_NONE = 0,
   /**
   There was additional failure information
   but it does not fit in one of the following classes */
   LINIT_SUB_UNKNOWN = 1,
   /**
   Lint Initialization failed due to a no-common-mode
   situation in the handshake. XTU-C and -R did not indicate an
   operation mode in common. */
   LINIT_SUB_NO_COMMON_MODE = 2
} DSL_G997_LineInitSubStatus_t;

/**
   Initialization Success/Failure Cause (see chapter 7.5.1.3 of G.997.1).
   If the line is forced to the L0 state (or into loop diagnostics mode) and
   an attempt to reach the L0 state (or to successfully complete the loop
   diagnostics procedures) fails (after a vendor discretionary number of
   retries and/or within a vendor discretionary timeout), then an Initialization
   Failure occurs. An Initialization Failure cause and Last Successful
   Transmitted State is given with the Line Initialization Failure
   (see 7.5.1.3). A Line Initialization failure shall be conveyed to the NMS
   by the xTU-R (over the T-/S-interface) after it is detected.
*/
typedef struct
{
   /**
   Specifies the current line transmission status according to G.997.1. */
   DSL_OUT DSL_G997_LineInit_t nLineInitStatus;
   /**
   Specifies the current line transmission substatus*/
   DSL_G997_LineInitSubStatus_t nLineInitSubStatus;
} DSL_G997_LineInitStatusData_t;

/**
   Initialization Success/Failure Cause.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_LINE_INIT_STATUS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains line initialization status data */
   DSL_OUT DSL_G997_LineInitStatusData_t data;
} DSL_G997_LineInitStatus_t;

/**
   Common xTU Line Status (see chapter 7.5.1 of G.997.1).
   \note This structure will be used for downstream and upstream handling.
         Therefore the parameter comments are done for both access possibilities.
*/
typedef struct
{
   /**
   Downstream/Upstream Line Attenuation (see chapter 7.5.1.6/7.5.1.7 of G.997.1).
   This parameter is a integral value over the complete spectrum.
   It is the measured difference in the total power transmitted by the xTU C
   and the total power received by the xTU R over all subcarriers during
   diagnostics mode and initialization. The downstream line attenuation ranges
   per band from 0 to +127 dB with 0.1 dB steps. A special value (1271) indicates
   the line attenuation per band is out of range to be represented,
   unused band is indicated by (-32768) value.
   \note Line Attenuation values are init values which means it will NOT change
         during showtime.
   \note For ADSL systems the value that will be reported if the line is
         disabled reflects the last init value.
   \note For the Danube family US showtime value a special handling exists. This
         handling uses backup value to report LATN parameter in case of
         unsuccessful FW access or OHC access.
         Initially this special backup value reflects the Training LATN. Further
         on each successful FW or OHC access the backup value is updated.*/
   DSL_OUT DSL_int16_t LATN;
   /**
   Downstream/Upstream Signal Attenuation (see chapter 7.5.1.8/7.5.1.9 of G.997.1).
   This parameter is a integral value over the complete spectrum.
   It is the measured difference in the total power transmitted by the xTU C
   and the total power received by the xTU R over all subcarriers during
   showtime. The downstream line attenuation ranges from 0 to +127 dB
   with 0.1 dB steps. A special value (1271) indicates the line attenuation per
   band is out of range to be represented, unused band is indicated
   by (-32768) value.
   \note During showtime, only a subset of the subcarriers may be transmitted
         by the xTU-C, as compared to diagnostics mode and initialization.
         Therefore, the downstream Signal attenuation may be significantly
         lower than the downstream Line attenuation.
   \note For ADSL systems and normal initialization the value that will be
         returned depends on the line state as follows:
         \code
         t1 < t <  (t1 + 15s) : Init value reported
         t >= (t1 + 15s)      : Showtime value reported

         with:
         t1 = time where entering showtime
         t  = time where value is requested from DS API
         \endcode
         The value will set to 0 if leaving showtime whether automatically
         (e.g. exception) or manually (e.g. LineDeactivate).
         Also note that the showtime value will be directly requested from the
         firmware each time it will be requested from DSL API because this value
         might change in showtime.
         If operating in DELT mode according DELT values will be reported.
   \note For the Danube family US showtime value a special handling exists. This
         handling uses backup value to report SATN parameter in case of
         unsuccessful FW access or OHC access.
         Initially this special backup value reflects the Training SATN. Further
         on each successful FW or OHC access the backup value is updated.*/
   DSL_OUT DSL_int16_t SATN;
   /**
   Downstream/Upstream Signal-to-Noise Ratio Margin (see chapter 7.5.1.10/7.5.1.11
   of G.997.1).
   This parameter is a integral value over the complete spectrum.
   The downstream signal to noise ratio margin is the maximum increase in dB of
   the noise power received at the xTU R, such that the BER requirements are met
   for all downstream bearer channels. The downstream SNR margin ranges from
   -64 dB to +63 dB with 0.1 dB steps.
   A special value (-641) indicates the parameter is out of range to be
   represented.
   \note The downstream SNR margin measurement at the xTU R may take
         up to 15 s.
   \note For ADSL systems and normal initialization the value that will be
         returned depends on the line state as follows:
         \code
         t1 < t <  (t1 + 15s) : Init value reported
         t >= (t1 + 15s)      : Showtime value reported

         with:
         t1 = time where entering showtime
         t  = time where value is requested from DS API
         \endcode
         The value will set to 0 if leaving showtime whether automatically
         (e.g. exception) or manually (e.g. LineDeactivate).
         Also note that the showtime value will be directly requested from the
         firmware each time it will be requested from DSL API because this value
         might change in showtime.
         If operating in DELT mode according DELT values will be reported.
   \note For the Danube family US showtime value a special handling exists. This
         handling uses backup value to report SNR parameter in case of
         unsuccessful FW access or OHC access.
         Initially this special backup value reflects the Training SNR. Further
         on each successful FW or OHC access the backup value is updated. */
   DSL_OUT DSL_int16_t SNR;
   /**
   Downstream/Upstream Maximum Attainable Data Rate
   (see chapter 7.5.1.12/7.5.1.13 of G.997.1).
   This parameter indicates the maximum downstream net data rate currently
   attainable by the xTU-C/xTU-R transmitter and the xTU-R/xTU-C receiver.
   The rate is coded in bit/s.
   \note For ADSL systems the value that will be reported if the line is
         disabled  reflects the last showtime/DELT value.
   \note For the Danube family US showtime value a special handling exists. This
         handling uses backup value to report LATN parameter in case of
         unsuccessful FW access or OHC access.
         Initially this special backup value reflects the Training LATN. Further
         on each successful FW or OHC access the backup value is updated. */
   DSL_OUT DSL_uint32_t ATTNDR;
   /**
   Downstream/Upstream Actual Power Spectrum Density
   (see chapter 7.5.1.14/7.5.1.15 of G.997.1).
   This parameter is the average downstream/upstream transmit power spectrum
   density over the used subcarriers (subcarriers to which downstream user data
   are allocated) delivered by the xTU-C at the U-C reference point, at the
   instant of measurement. The power spectrum density level ranges from
   -90 dBm/Hz to 0 dBm/Hz with 0.1 dB steps. A special value (-901) indicates
   the parameter is out of range to be represented.
   \note The downstream actual power spectrum destiny is the sum (in dB) of
         the REFPSDds and RMSGIds.
   \note For ADSL systems the value that will be reported if the line is
         disabled is 0. */
   DSL_OUT DSL_int16_t ACTPS;
   /**
   Downstream/Upstream Actual Aggregate Transmit Power
   (see chapter 7.5.1.16/7.5.1.17 of G.997.1).
   This parameter is the total amount of transmit power delivered by the
   xTU-R/xTU-Upstream at the U-R reference point, at the instant of measurement.
   The total output power level ranges from -31 dBm to +31 dBm with 0.1 dB
   steps.
   A special value (-512) indicates the parameter is out of range to be
   represented.
   \note The downstream/upstream nominal aggregate transmit power may be taken
         as a best estimate of the parameter. */
   DSL_OUT DSL_int16_t ACTATP;
} DSL_G997_LineStatusData_t;

/**
   Common xTU Line Status.
   This structure has to be used for ioctl \ref DSL_FIO_G997_LINE_STATUS_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Specifies for which DELT parameter type (diagnostic/showtime) the function
   will apply */
   DSL_IN DSL_DeltDataType_t nDeltDataType;
   /**
   Structure that contains line status data */
   DSL_OUT DSL_G997_LineStatusData_t data;
} DSL_G997_LineStatus_t;

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   Common xTU Line Status per band (see chapter 7.5.1 of G.997.1).
   \note This structure will be used for downstream and upstream handling.
         Therefore the parameter comments are done for both access possibilities.
*/
typedef struct
{
   /**
   Downstream/Upstream Line Attenuation per band
   (see chapter 7.5.1.6/7.5.1.7 of G.997.1).
   This parameter is defined per usable bands. It is the measured difference
   in the total power transmitted in this band by the xTU C and the total power
   received in this band by the xTU R over all subcarriers of this band during
   diagnostics mode and initialization. The downstream line attenuation ranges
   per band from 0 to +127 dB with 0.1 dB steps. A special value (-1) indicates
   the line attenuation per band is out of range to be represented.
   For ADSL systems, a single parameter is defined as a single downstream
   band is usable.
   All unused band entries will carry 0x8000.
   \note Line Attenuation values are init values which means it will NOT change
         during showtime. */
   DSL_OUT DSL_int16_t LATN[DSL_G997_MAX_NUMBER_OF_BANDS];
   /**
   Downstream/Upstream Signal Attenuation per band
   (see chapter 7.5.1.8/7.5.1.9 of G.997.1).
   This parameter is defined per usable bands. It is the measured difference
   in the total power transmitted in this band by the xTU C and the total power
   received in this band by the xTU R over all subcarriers of this band during
   showtime. The downstream line attenuation per band ranges from 0 to +127 dB
   with 0.1 dB steps. A special value (-1) indicates the line attenuation per
   band is out of range to be represented.
   For ADSL systems, a single parameter is defined as a single downstream
   band is usable.
   \note During showtime, only a subset of the subcarriers may be transmitted
         by the xTU-C, as compared to diagnostics mode and initialization.
         Therefore, the downstream Signal attenuation may be significantly
         lower than the downstream Line attenuation. */
   DSL_OUT DSL_int16_t SATN[DSL_G997_MAX_NUMBER_OF_BANDS];
   /**
   Downstream/Upstream Signal-to-Noise Ratio Margin per band
   (see chapter 7.5.1.10/7.5.1.11 of G.997.1).
   This parameter is defined by usable band. The downstream signal to
   noise ratio margin per band is the maximum increase in dB of the noise
   power received at the xTU R, such that the BER requirements are met
   for all downstream bearer channels of this band. The downstream SNR
   margin per band ranges from -64 dB to +63 dB with 0.1 dB steps.
   A special value (-641) indicates the parameter is out of range to be
   represented.
   \note The downstream SNR margin measurement at the xTU R may take
         up to 15 s. */
   DSL_OUT DSL_int16_t SNR[DSL_G997_MAX_NUMBER_OF_BANDS];
} DSL_G997_LineStatusPerBandData_t;

/**
   Common xTU Line Status per band.
   This structure has to be used for ioctl \ref DSL_FIO_G997_LINE_STATUS_PER_BAND_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains line status data */
   DSL_OUT DSL_G997_LineStatusPerBandData_t data;
} DSL_G997_LineStatusPerBand_t;
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

/**
   G.997.1 specific framing parameter (see chapter 7.5.2.6 of G.997.1/Rev3).
   Structure to return the actual status of various framing parameters.
*/
typedef struct
{
   /**
   Actual size of Reed-Solomon codeword (NFEC).
   This parameter reports the actual Reed-Solomon codeword size used in the
   latency path in which the bearer channel is transported.
   The value is coded in bytes. It ranges from 0 to 255. */
   DSL_OUT DSL_uint16_t nNFEC;
   /**
   Actual number of Reed-Solomon redundancy bytes (RFEC).
   This parameter reports the actual number of Reed-Solomon redundancy bytes
   per codeword used in the latency path in which the bearer channel is
   transported. The value is coded in bytes. It ranges from 0 to 16. The
   value 0 indicates no Reed-Solomon coding.
   G.992.1/2:
   Number of Reed-Solomon bytes. The value corresponds to RF/S for the
   fast path, RI/S for the interleaved path (number of RS bytes per symbol).
   G.992.3/4/5:
   The value corresponds to RP (number of RS bytes per FEC data frame).
   G.993.1:
   The value corresponds to R (number of RS check bytes per code word).
   G.993.2:
   The number of redundancy octets in the RS codeword.  */
   DSL_OUT DSL_uint16_t nRFEC;
   /**
   Actual number of bits per symbol (LSYMB).
   This parameter reports the actual number of bits per symbol assigned to
   the latency path in which the bearer channel is transported. This value
   does not include trellis overhead. The value is coded in bits.
   It ranges from 0 to 65535.
   Maximum possible value is 3825 in case of ADSL2 and 7665 in
   case of ADSL2+.
   This parameter is not valid for G.993.1. */
   DSL_OUT DSL_uint16_t nLSYMB;
   /**
   Actual interleaving depth (INTLVDEPTH).
   This parameter reports the actual depth of the interleaver used in the
   latency path in which the bearer channel is transported. The value
   ranges from 1 to 4096 by step of 1.
   The value 1 indicates no interleaving. */
   DSL_OUT DSL_uint16_t nINTLVDEPTH;
   /**
   Actual interleaving block length (INTLVBLOCK).
   This parameter reports the actual block length of the interleaver used
   in the latency path in which the bearer channel is transported.
   The value ranges from 4 to 255 by step of 1. */
   DSL_OUT DSL_uint16_t nINTLVBLOCK;
   /**
   Actual latency path (LPATH).
   This parameter reports the index of the actual latency path in which the
   bearer is transported. The valid values are 0, 1, 2, and 3.   */
   DSL_OUT DSL_uint8_t nLPATH;
} DSL_G997_FramingParameterStatusData_t;

/**
   G.997.1 specific framing parameter.
   This structure has to be used for ioctl
   \ref DSL_FIO_G997_FRAMING_PARAMETER_STATUS_GET

   The returned data is according to G997.1 V4 chapter 7.5.1.26.1

   H(f) linear representation Scale (HLINSC)

   This parameter is the scale factor to be applied to the Hlin(f)
   values. It is represented as an unsigned integer in the range
   from 1 to 2^16-1. This parameter is only available after a loop
   diagnostic procedure.
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains framing parameter status data */
   DSL_OUT DSL_G997_FramingParameterStatusData_t data;
} DSL_G997_FramingParameterStatus_t;

/**
   This structure is used to read the Hlin scale measured during diagnostic.
*/
typedef struct
{
   /**
   Returns the HLIN scale value */
   DSL_OUT DSL_uint16_t nDeltHlinScale;
} DSL_G997_DeltHlinScaleData_t;

/**
   This structure is used to read the Hlin scale measured during diagnostic.
   This structure has to be used for ioctl \ref DSL_FIO_G997_DELT_HLIN_SCALE_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Specifies for which DELT parameter type (diagnostic/showtime) the function
   will apply */
   DSL_IN DSL_DeltDataType_t nDeltDataType;
   /**
   Structure that contains DELT data for Hlin scale */
   DSL_OUT DSL_G997_DeltHlinScaleData_t data;
} DSL_G997_DeltHlinScale_t;

/**
   This structure is used to read the Hlin per subcarrier measured during
   diagnostic.

   The returned data is according to G997.1 V4 chapter 7.5.1.26.3

   H(f) linear representation (HLINps)

   This parameter is an array of complex values in linear scale for
   Hlin(f). Each array entry represents the Hlin(f = i*HLINGds*.f) value for a
   particular subcarrier group index i, ranging from 0 to MIN(NSds ,511).
   The Hlin(f) is represented as ((HLINSCds/2^15)*((a(i) + j*b(i))/2^15)),
   where a(i) and b(i) are signed integers in the (-2^15 + 1) to (+2^15 - 1)
   range. A special value indicates that no measurement could be done for this
   subcarrier group because it is out of the passband or that the attenuation
   is out of range to be represented. This parameter is only available after a
   loop diagnostic procedure.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the HLIN values for the number of subcarriers (NSC) */
   DSL_OUT DSL_G997_NSCComplexData_t deltHlin;
} DSL_G997_DeltHlinData_t;

/**
   This structure is used to read the Hlin per subcarrier measured during
   diagnostic.
   This structure has to be used for ioctl \ref DSL_FIO_G997_DELT_HLIN_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Specifies for which DELT parameter type (diagnostic/showtime) the function
   will apply */
   DSL_IN DSL_DeltDataType_t nDeltDataType;
   /**
   Structure that contains DELT data for Hlin */
   DSL_OUT DSL_G997_DeltHlinData_t data;
} DSL_G997_DeltHlin_t;

/**
   This structure is used to read the Hlog per subcarrier measured during
   diagnostic or showtime.

   The returned data is according to G997.1 V4 chapter 7.5.1.26.6

   H(f) logarithmic representation (HLOGps)

   This parameter is an array of real values in dB for Hlog(f).
   Each array entry represents the real Hlog(f = i*HLOGGds*.f) value for a
   particular subcarrier group subcarrier index i, ranging from 0 to
   MIN(NSds ,511). The real Hlog(f) value is represented as (6 - m(i)/10),
   where m(i) is an unsigned integer in the range from 0 to 1022.
   A special value indicates that no measurement could be done for this
   subcarrier group because it is out of the passband or that the attenuation
   is out of range to be represented.
*/
typedef struct
{
   /**
   Measurement time  */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the HLOG values for the number of subcarriers (NSC) */
   DSL_OUT DSL_G997_NSCData16_t deltHlog;
} DSL_G997_DeltHlogData_t;

/**
   This structure is used to read the Hlog per subcarrier measured during
   diagnostic or training (in case of selecting showtime values
   nDeltDataType=DSL_DELT_DATA_SHOWTIME).
   This structure has to be used for ioctl \ref DSL_FIO_G997_DELT_HLOG_GET

*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Specifies for which DELT parameter type (diagnostic/showtime) the function
   will apply */
   DSL_IN DSL_DeltDataType_t nDeltDataType;
   /**
   Structure that contains DELT data for Hlog */
   DSL_OUT DSL_G997_DeltHlogData_t data;
} DSL_G997_DeltHlog_t;

/**
   This function is used to read the Signal to Noise Ratio per subcarrier
   measured during diagnostic or showtime.

   The returned SNR values are according to G997.1 V4 chapter  7.5.1.28.3

   Downstream SNR(f) (SNRps)

   This parameter is an array of real values in dB for downstream SNR(f).
   Each array entry represents the SNR(f = i*SNRGds*.f) value for a particular
   subcarrier group index i, ranging from 0 to MIN(NSds ,511). The SNR(f)
   is represented as (-32 + snr(i)/2), where snr(i) is an unsigned integer
   in the range from 0 to 254. A special value indicates that no measurement
   could be done for this subcarrier group because it is out of the passband
   or that the SNR is out of range to be represented.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the SNR values for the number of subcarriers (NSC) */
   DSL_OUT DSL_G997_NSCData8_t deltSnr;
} DSL_G997_DeltSnrData_t;

/**
   This function is used to read the Signal to Noise Ratio per subcarrier
   group measured during showtime or diagnostic.
   This structure has to be used for ioctl \ref DSL_FIO_G997_DELT_HLOG_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Specifies for which DELT parameter type (diagnostic/showtime) the function
   will apply */
   DSL_IN DSL_DeltDataType_t nDeltDataType;
   /**
   Structure that contains DELT data for snr */
   DSL_OUT DSL_G997_DeltSnrData_t data;
} DSL_G997_DeltSnr_t;

/**
   This function is used to read the Quiet Line Noise per subcarrier measured
   during diagnostic or showtime.

   The returned QLN values are according to G997.1 V4 chapter 7.5.1.27.3:

   This parameter is an array of real values in dBm/Hz for downstream QLN(f).
   Each array entry represents the QLN(f = i*QLNGds*.f) value for a
   particular subcarrier group index i, ranging from 0 to MIN(NSds ,511).
   The QLN(f) is represented as (-23 - n(i)/2), where n(i) is an unsigned
   integer in the range from 0 to 254. A special value indicates that no
   measurement could be done for this subcarrier group because it is out
   of the passband or that the noise PSD is out of range to be represented.
*/
typedef struct
{
   /**
   Measurement time */
   DSL_uint16_t nMeasurementTime;
   /**
   Subcarrier group size */
   DSL_uint8_t nGroupSize;
   /**
   Returns the QLN values for the number of subcarriers (NSC) */
   DSL_OUT DSL_G997_NSCData8_t deltQln;
} DSL_G997_DeltQlnData_t;

/**
   This function is used to read the Quiet Line Noise per subcarrier measured
   during diagnostic or training (in case of selecting showtime values
   nDeltDataType=DSL_DELT_DATA_SHOWTIME).
   This structure has to be used for ioctl \ref DSL_FIO_G997_DELT_QLN_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Specifies for which DELT parameter type (diagnostic/showtime) the function
   will apply */
   DSL_IN DSL_DeltDataType_t nDeltDataType;
   /**
   Structure that contains DELT data for qln */
   DSL_OUT DSL_G997_DeltQlnData_t data;
} DSL_G997_DeltQln_t;

/**
   This function is used to read the Quiet Line Noise per subcarrier measured
   during diagnostic.
   This structure has to be used for ioctl \ref DSL_FIO_G997_DELT_QLN_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
} DSL_G997_DeltFreeResources_t;

/** maximum SNMP message length according to G.997.1, chapter 6.4 */
#define DSL_G997_SNMP_MESSAGE_LENGTH 508

/**
   SNMP message data structure. Used to transmit/receive SNMP messages via the
   overhead channel according G.997.1 (see chapter 6.4 and following).
*/
typedef struct
{
   /** Number of used octets within message data. */
   DSL_uint16_t nMessageLength;
   /** Message data */
   DSL_uint8_t nMessageData[DSL_G997_SNMP_MESSAGE_LENGTH];
} DSL_G997_SnmpData_t;

/**
   SNMP message data structure. Used to transmit SNMP messages via the overhead
   channel according G.997.1 (see chapter 6.4 and following).
   This structure has to be used for ioctl
   - \ref DSL_FIO_G997_SNMP_MESSAGE_SEND
   - \ref DSL_FIO_G997_SNMP_MESSAGE_RECEIVE
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains SNMP data */
   DSL_IN_OUT DSL_G997_SnmpData_t data;
} DSL_G997_Snmp_t;

/**
   Rate Adaptation Mode setting.
   \attention The first mode that is defined within G.997.1
              (DSL_G997_RA_MODE_MANUAL) is not available on CPE side.
              Basically the CPE is only able to enable or disable the automatic
              SRA handling.
              Thus please note that the description within the parameter that
              are included within this structure is done from CO point of view.
*/
typedef enum
{
   /**
   Rate automatically selected at startup only and does not change after that.

   At startup:

   The Downstream Minimum Rate parameter specifies the minimum data rate the
   ATU-C transmitter shall operate at for each of the bearer channels, with a
   downstream noise margin which is at least as large as the specified
   Downstream Target Noise Margin, relative to the required BER for each of the
   bearer channels, or better. If the ATU-C fails to achieve the Downstream
   Minimum Data Rate for one of the bearer channels, the ATU-C will fail to
   initialize, and the NMS will be notified. If the ATU-C transmitter is able
   to support a higher downstream data rate at initialization, the excess data
   rate will be distributed amongst the downstream bearer channels according to
   the ratio (0 to 100%) specified by the Rate Adaptation Ratio parameter for
   each bearer channel (adding up to 100% over all bearer channels). When the
   Downstream Maximum Data Rate is achieved in one of the bearer channels, then
   the remaining excess bit rate is assigned to the other bearer channels,
   still according to their relative Rate Adaptation Ratio parameters. As long
   as the downstream data rate is below the Downstream maximum Data Rate for
   one of the bearer channels, data rate increase shall take priority over
   transmit power reduction.

   At showtime:

   During showtime, no downstream data rate adaptation is allowed. The
   downstream data rate, which has been settled during initialization for each
   of the bearer channels, shall be maintained. */
   DSL_G997_RA_MODE_AT_INIT = 2,
   /**
   Data rate is automatically selected at initialization and is continuously
   adapted during operation (showtime). The DYNAMIC Rate Adaptation mode is
   optional. All related configuration parameters are also optional.

   At startup:

   In Mode DSL_G997_RA_MODE_DYNAMIC, the ATU-C shall start up as in Mode
   DSL_G997_RA_MODE_AT_INIT.

   At showtime:

   During showtime, rate adaptation is allowed with respect to the Ratio
   Adaptation Ratio for distributing the excess data rate amongst the bearer
   channels (see Mode DSL_G997_RA_MODE_AT_INIT), and assuring that the
   Downstream Minimum Data Rate remains available at the required BER for each
   of the bearer channels, or better. The downstream data rate can vary between
   the Downstream Minimum Data Rate, and the Downstream Maximum Data Rate.
   Downstream Rate Adaptation is performed when the conditions specified for
   Downstream Upshift Noise Margin and Downstream Upshift Interval - or for
   Downstream Downshift Noise Margin and Downstream Downshift Interval - are
   satisfied.

   This means:

   -  For an Upshift action: Allowed when the downstream noise margin is above
      the Downstream Upshift Noise Margin during Downstream Minimum Time
      Interval for Upshift Rate Adaptation (i.e. upon RAU anomaly).
   -  For a Downshift action: Allowed when the downstream noise margin is below
      the Downstream Downshift Noise Margin during Downstream Minimum Time
      Interval for Downshift Rate Adaptation (i.e. upon RAD anomaly).
   As long as the downstream data rate is below the Downstream Maximum Data
   Rate for one of the bearer channels, data rate increase shall take priority
   over transmit power reduction. */
   DSL_G997_RA_MODE_DYNAMIC = 3,

   /**
   DYNAMIC with SOS 

   Data rate is automatically selected at initialization and may be
   continuously adapted during operation (showtime) by SOS and SRA. 
   The Rate Adaptation mode 4 is optional. In this mode, enabling of SOS 
   and SRA is mandatory.

   At startup:

   In Mode 4, the xTU-C shall start up as in Mode 2.

   At showtime:

   SRA behavior shall be identical as described for Mode 3, 
   unless the actual net data rate is below
   the Minimum net data rate as a result of an SOS procedure.
   Additionally, SOS may be performed, when the conditions specified 
   by the SOS trigger parameters are satisfied. The detailed specification 
   of SOS OLR procedure is in G.993.2.

   If at startup, it is detected that SOS is not supported in the downstream 
   direction by either XTU's, but SRA is supported by both XTU's, 
   the XTU's shall fallback to Mode 3.

   If at startup, it is detected that SOS is not supported in the downstream 
   direction by either XTU's, and SRA is not supported by either XTU's, 
   the XTU's shall fallback to Mode 2. */
   DSL_G997_RA_MODE_DYNAMIC_SOS = 4
} DSL_G997_RA_MODE_t;

/**
   Rate Adaptation configuration parameters (see chapter 7.3.1.4 of G.997.1)
   The following configuration parameters are defined to manage the
   Rate-Adaptive behavior in the transmit direction for both the ATU-C and the
   ATU-R. An ATU-C Rate Adaptation Mode applies to the upstream direction. An
   ATU-R Rate Adaptation Mode applies to the downstream direction.
   \note For the Vinax device the Rata Adaptive mode can be only selected for
         ATU-R (downstream)
*/
typedef struct
{
   /**
   Downstream/Upstream Rate Adaptation Mode (RA-MODE)
   (see chapter 7.3.1.4 of G.997.1).
   This parameter specifies the mode of operation of a rate-adaptive
   ATU-C/ATU-R in the transmit/receive direction. */
   DSL_G997_RA_MODE_t RA_MODE;
} DSL_G997_RateAdaptationConfigData_t;

/**
   Structure for configuration of SRA.
   This structure has to be used for ioctl
   - \ref DSL_FIO_G997_RATE_ADAPTATION_CONFIG_SET
   - \ref DSL_FIO_G997_RATE_ADAPTATION_CONFIG_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which direction (upstream/downstream) the function will
   apply */
   DSL_IN DSL_AccessDir_t nDirection;
   /**
   Structure that contains SRA configuration data */
   DSL_OUT DSL_G997_RateAdaptationConfigData_t data;
} DSL_G997_RateAdaptationConfig_t;

/** @} DRV_DSL_CPE_G997 */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_API_G997_H */
