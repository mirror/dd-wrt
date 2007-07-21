#include    <stdio.h>
#include    <string.h>
#include    <fcntl.h>
#include	<time.h>
#include	<unistd.h>
#include	<sys/timeb.h>
#include	<sys/time.h>
#include	<sys/types.h>


#define     TRUE    1
#define     FALSE   0
#define     SEARCH_TIMEOUT                  100
#define     SEARCH_RETRY_CNT                30
#define     MAX_DEVICE_NUM        10

#define     TEMPDIR     "/tmp/moxa"
#define     NPPATH      "/tmp/npreal2"
#define     DRIVERPATH  "/tmp/npreal2/driver"
#define     SBINPATH  "/tmp/npreal2/driver"


#define		NP5210          0x0322
#define		NP5230          0x0312
#define		NP5410          0x0504
#define		NP5430          0x0534
#define		NP5610_8        0x5618
#define		NP5610_16       0x5613


#ifdef _DEBUG_PRINT
    #define DBG_PRINT       printf
#else
    #define DBG_PRINT       if (0) printf
#endif


