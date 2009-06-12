//==========================================================================
//
//      tests/sysctl1.c
//
//      Simple test of sysclt API
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from FreeBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    andrew lunn
// Contributors: andrew lunn
// Date:         2003-06-28
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <network.h>
#include <sys/sysctl.h>

#include <cyg/infra/testcase.h>
#ifndef NELEM
#define NELEM(x) sizeof(x)/sizeof(*x)
#endif

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

static int mib_name[] =     { CTL_DEBUG, 1 }; 
static int mib_next[] =     { CTL_DEBUG, 2 };
static int mib_name2oid[] = { CTL_DEBUG, 3 };
static int mib_oidfmt[] =   { CTL_DEBUG, 4 };
static int mib_name_debug_name[] = { CTL_DEBUG, 1, CTL_DEBUG, 1 }; 
static int mib[10];
static char name2oid[] = "sysctl.name2oid";
void
net_test(cyg_addrword_t q)
{
    char oldbuff[128];
    size_t oldbuffsize;
    char pbuff[128], name[64], *p;
    int *new_oid;
    size_t num_elem;
    int ret, i, oid_kind; 

    diag_printf("Start sysctl1 test\n");

    //    init_all_network_interfaces();

    /* Test the OID to name function of sysctl*/
    oldbuffsize = sizeof(oldbuff);
    ret = sysctl(mib_name_debug_name, NELEM(mib_name_debug_name), 
                 oldbuff, &oldbuffsize, NULL, 0);
    if (ret == -1) CYG_TEST_FAIL("sysclt(mib_name) failed");
    CYG_TEST_INFO(oldbuff);
    CYG_TEST_PASS_FAIL(!strcmp(oldbuff, "sysctl.name"), "sysctl.name");

    /* Test the name to OID function of sysclt */
    oldbuffsize = sizeof(oldbuff);
    ret = sysctl(mib_name2oid, NELEM(mib_name2oid), oldbuff, &oldbuffsize, 
                 name2oid, sizeof(name2oid));
    if (ret == -1) CYG_TEST_FAIL("sysclt(mib_name) failed");
    CYG_TEST_PASS_FAIL(((ret == 8 ) && 
                        (((int *)&oldbuff)[0] == CTL_DEBUG) &&
                        (((int *)&oldbuff)[1] == 3)), 
                       "sysctl.name2oid");
    
    /* Walk the table using the next function of sysclt */
    num_elem = NELEM(mib_next)+1;
    new_oid = mib + NELEM(mib_next);
    mib[2] = 0;
    do {
      memcpy(mib,mib_next,sizeof(mib_next));
      oldbuffsize = sizeof(mib) - sizeof(mib_next);
      ret = sysctl(mib, num_elem, new_oid, &oldbuffsize, NULL, 0);
      if (ret == -1) {
        if (errno != ENOENT) {
          CYG_TEST_FAIL_FINISH("sysclt(mib_name) failed");
        } else {
          break;
        }
      }
      p = pbuff;
      num_elem = NELEM(mib_next) + (ret / 4);
      i=0;
      while (ret > 0) {
        p+=diag_sprintf(p, "%d ",new_oid[i++]);
        ret -= sizeof(int);
      }
      /* Get the name of this oid */
      oldbuffsize = sizeof(name);
      memcpy(mib,mib_name,sizeof(mib_name));
      ret = sysctl(mib, num_elem, name, &oldbuffsize, NULL, 0);
      if (ret == -1) CYG_TEST_FAIL("sysclt(mib2name) failed");
      name[ret] = 0;
      p += diag_sprintf(p, "= %s (", name);

      /* Get the find and format */
      memcpy(mib,mib_oidfmt,sizeof(mib_oidfmt));
      ret = sysctl(mib, num_elem, name, &oldbuffsize, NULL, 0);
      if (ret == -1) CYG_TEST_FAIL("sysclt(mib2name) failed");
      oid_kind = *(int *)name;
      switch (oid_kind & CTLTYPE) {
      case CTLTYPE_NODE:
        p += diag_sprintf(p, "NODE");
        break;
      case CTLTYPE_INT:
        p += diag_sprintf(p, "INT");
        break;
      case CTLTYPE_STRING:
        p += diag_sprintf(p, "STRING");
        break;
      case CTLTYPE_QUAD:
        p += diag_sprintf(p, "QUAD");
        break;
      case CTLTYPE_STRUCT:
        p += diag_sprintf(p, "STRUCT");
        break;
      case CTLTYPE_UINT:
        p += diag_sprintf(p, "UINT");
        break;
      case CTLTYPE_LONG:
        p += diag_sprintf(p, "LONG");
        break;
      case CTLTYPE_ULONG:
        p += diag_sprintf(p, "ULONG");
        break;
      default:
        p += diag_sprintf(p,"Unknown type! (%d)", oid_kind & CTLTYPE);
      }
      if (oid_kind & CTLFLAG_RD) 
        p += diag_sprintf(p," Read");
      if (oid_kind & CTLFLAG_WR) 
        p += diag_sprintf(p," Write");
      if (oid_kind & CTLFLAG_NOLOCK) 
        p += diag_sprintf(p," Nolock");
      if (oid_kind & CTLFLAG_ANYBODY) 
        p += diag_sprintf(p," Anybody");
      if (oid_kind & CTLFLAG_SECURE) 
        p += diag_sprintf(p," Secure");
      if (oid_kind & CTLFLAG_PRISON) 
        p += diag_sprintf(p," Prison");
      if (oid_kind & CTLFLAG_DYN) 
        p += diag_sprintf(p," Dynamic");
      
      p += diag_sprintf(p," )");

      p += diag_sprintf(p,"{%d}", num_elem);
      CYG_TEST_INFO(pbuff);
    } while (ret != -1);

    /* Tests for sysctlnametomib */
    num_elem = NELEM(mib);
    ret = sysctlnametomib(name2oid, mib,&num_elem);
    if (ret == -1) CYG_TEST_FAIL("sysctlnametomib failed");
    CYG_TEST_PASS_FAIL(((num_elem == 2 ) && 
                        (((int *)&oldbuff)[0] == CTL_DEBUG) &&
                        (((int *)&oldbuff)[1] == 3)), 
                       "sysctlnametooid1");

    /* This time with too small a results buffer */
    num_elem = 1;
    ret = sysctlnametomib(name2oid, mib,&num_elem);
    CYG_TEST_PASS_FAIL((ret == -1) && (errno = ENOMEM), 
                       "sysctlnametooid2");
    /* This time with an unknown name */
    num_elem = NELEM(mib);
    ret = sysctlnametomib("unknown.unknown", mib,&num_elem);
    CYG_TEST_PASS_FAIL((ret == -1) && (errno = ENOENT), 
                       "sysctlnametooid3");

    /* Tests for sysctlbyname */
    oldbuffsize = sizeof(oldbuff);
    ret = sysctlbyname("sysctl.name2oid", oldbuff, &oldbuffsize, 
                 name2oid, sizeof(name2oid));
    if (ret == -1) CYG_TEST_FAIL("sysclt(mib_name) failed");
    CYG_TEST_PASS_FAIL(((ret == 8 ) && 
                        (((int *)&oldbuff)[0] == CTL_DEBUG) &&
                        (((int *)&oldbuff)[1] == 3)), 
                       "sysctlbyname");

    CYG_TEST_EXIT ("sysctl1 exit");
}

void
cyg_start(void)
{
    CYG_TEST_INIT();

    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(CYGPKG_NET_THREAD_PRIORITY-4,// Priority - just a number
                      net_test,                 // entry
                      0,                        // entry parameter
                      "Loopback ping  test",    // Name
                      &stack[0],                // Stack
                      STACK_SIZE,               // Size
                      &thread_handle,           // Handle
                      &thread_data              // Thread data structure
            );
    cyg_thread_resume(thread_handle);           // Start it
    cyg_scheduler_start();
}

