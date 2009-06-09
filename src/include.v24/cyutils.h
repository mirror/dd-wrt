#include "code_pattern.h"
#include "../../opt/build.h"


#ifdef HAVE_POWERNOC
#define CYBERTAN_VERSION	"POWERNOC v23"
#elif HAVE_WTS
#define CYBERTAN_VERSION	"WTS v23"
#elif HAVE_OMNI
#define CYBERTAN_VERSION	"OMNI Wifi v23"
#elif HAVE_DLS
#define CYBERTAN_VERSION	"DLS v23"
#elif HAVE_SKYTRON
#define CYBERTAN_VERSION	"SKYTRON v23"
#elif HAVE_GGEW
#define CYBERTAN_VERSION	"NMN 1.6 (" BUILD_DATE ")"
#elif HAVE_NEWMEDIA
#define CYBERTAN_VERSION	"NMN 1.5 (" BUILD_DATE ")"
#elif HAVE_SKYTEL
#define CYBERTAN_VERSION	"ST54G v23"
#elif HAVE_34TELECOM
#define CYBERTAN_VERSION	"MiuraBasic v23"
#elif HAVE_FON
#define CYBERTAN_VERSION	"FON powered by DD-WRT v23 (" BUILD_DATE ")"
#elif HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
#define CYBERTAN_VERSION	"2.1 (" BUILD_DATE ")"
#else
#define CYBERTAN_VERSION	"MAKSAT 1.5 (" BUILD_DATE ")"
#endif
#elif HAVE_TMK
#define CYBERTAN_VERSION	"KMT-WAS 1.6 (" BUILD_DATE ")"
#elif HAVE_XIOCOM
#define CYBERTAN_VERSION	"XOS 1.8.1 (" BUILD_DATE ")"
#elif HAVE_BUFFALO
#define CYBERTAN_VERSION	"DD-WRT v24SP1b3-" DEFAULT_COUNTRY_CODE " (" BUILD_DATE ")"
#else
#define CYBERTAN_VERSION	"DD-WRT v24-sp2 (" BUILD_DATE ")"
#endif

#define LANG_VERSION		"v1.00.00"
#define MINOR_VERSION		""
#define SERIAL_NUMBER		"000"
#define PMON_BOOT_VERSION	"v1.8"
#define CFE_BOOT_VERSION	"v2.4"
#define CFES_BOOT_VERSION	"v3.5"

#define BOOT_IPADDR "192.168.1.1"
#define BOOT_NETMASK "255.255.255.0"

#define SUPPORT_4712_CHIP	0x0001
#define SUPPORT_INTEL_FLASH	0x0002
#define SUPPORT_5325E_SWITCH	0x0004
#define SUPPORT_4704_CHIP	0x0008
#define SUPPORT_5352E_CHIP	0x0010
#define SUPPORT_5354G_CHIP	0x0020

struct code_header {
	char magic[4];
	char res1[4];	// for extra magic
	char fwdate[3];
	char fwvern[3];
	char id[4];	// U2ND
	char hw_ver;    // 0) for 4702, 1) for 4712, 2) for 4712L, 3) for 4704
	char res2;
	unsigned short flags;
	unsigned char res3[10];
} ;

//#ifdef MULTILANG_SUPPORT
struct lang_header {
        char magic[4];
        char res1[4];   // for extra magic
        char fwdate[3];
        char fwvern[3];
        char id[4];     // U2ND
        char hw_ver;    // 0: for 4702, 1: for 4712
	char res2;
        unsigned long len;
        unsigned char res3[8];
} ;
//#endif

struct boot_header {
	char magic[3];
	char res[29];
};

/***************************************
 * define upnp misc                    *
 ***************************************/
  #define URL			"http://www.dd-wrt.com/"
  #define DEV_FRIENDLY_NAME	MODEL_NAME
  #define DEV_MFR		"NewMedia-NET GmbH"
  #define DEV_MFR_URL		URL
  #define DEV_MODEL_DESCRIPTION	"Internet Gateway Device"
  #define DEV_MODEL		MODEL_NAME
  #define DEV_MODEL_NO		CYBERTAN_VERSION
  #define DEV_MODEL_URL		URL

/***************************************
 * define Parental Control link        *
 ***************************************/
#if LOCALE == EUROPE
  #define	SIGN_UP_URL	"http://pcsvc.ourlinksys.com/eu/language.jsp"
  #define	MORE_INFO_URL	"http://www.linksys.com/pcsvc/eu/info_eu.asp"
  #define	ADMIN_URL	"http://pcsvc.ourlinksys.com/en"
#elif LOCALE == GERMANY
  #define	SIGN_UP_URL	"http://pcsvc.ourlinksys.com/de/trial.asp"
  #define	MORE_INFO_URL	"http://www.linksys.com/pcsvc/de/info_de.asp"
  #define	ADMIN_URL	"http://pcsvc.ourlinksys.com/de/admin.asp"
#elif LOCALE == FRANCE
  #define	SIGN_UP_URL	"http://pcsvc.ourlinksys.com/fr/trial.asp"
  #define	MORE_INFO_URL	"http://www.linksys.com/pcsvc/fr/info_fr.asp"
  #define	ADMIN_URL	"http://pcsvc.ourlinksys.com/fr/admin.asp"
#else
  #define	SIGN_UP_URL	"http://pcsvc.ourlinksys.com/us/trial.asp"
  #define	MORE_INFO_URL	"http://www.linksys.com/pcsvc/info.asp"
  #define	ADMIN_URL	"http://pcsvc.ourlinksys.com/us/admin.asp"
#endif

/***************************************
 * define PPTP info		       * 
 ***************************************/
#define	PPTP_VENDOR	"Linksys"
#define PPTP_HOSTNAME	""

/***************************************
 * define L2TP info		       *
 ***************************************/
#define	L2TP_VENDOR	"Linksys"
#define L2TP_HOSTNAME	""

