/*
 *  Host Resources MIB - other device implementation - hr_other.c
 *
 */

#include <net-snmp/net-snmp-config.h>

#include "host_res.h"
#include "hr_other.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif


void            Init_HR_CPU(void);
int             Get_Next_HR_CPU(void);
const char     *describe_cpu(int);

void            Init_HR_CoProc(void);
int             Get_Next_HR_CoProc(void);
const char     *describe_coproc(int);

void
init_hr_other(void)
{
    init_device[HRDEV_PROC] = Init_HR_CPU;
    next_device[HRDEV_PROC] = Get_Next_HR_CPU;
    device_descr[HRDEV_PROC] = describe_cpu;

    init_device[HRDEV_COPROC] = Init_HR_CoProc;
    next_device[HRDEV_COPROC] = Get_Next_HR_CoProc;
    device_descr[HRDEV_COPROC] = describe_coproc;
}


static int      done_CPU;

void
Init_HR_CPU(void)
{
    done_CPU = 0;
}

int
Get_Next_HR_CPU(void)
{
    /*
     * Assumes a single CPU system
     * I think it's safe to assume at least one! 
     */
    if (done_CPU != 1) {
        done_CPU = 1;
        return (HRDEV_PROC << HRDEV_TYPE_SHIFT);
    } else
        return -1;
}

const char     *
describe_cpu(int idx)
{
#ifdef _SC_CPU_VERSION
    int             result;

    result = sysconf(_SC_CPU_VERSION);
    switch (result) {
    case CPU_HP_MC68020:
        return (" Motorola MC68020 ");
    case CPU_HP_MC68030:
        return (" Motorola MC68030 ");
    case CPU_HP_MC68040:
        return (" Motorola MC68040 ");
    case CPU_PA_RISC1_0:
        return (" HP PA-RISC 1.0 ");
    case CPU_PA_RISC1_1:
        return (" HP PA-RISC 1.1 ");
    case CPU_PA_RISC1_2:
        return (" HP PA-RISC 1.2 ");
    case CPU_PA_RISC2_0:
        return (" HP PA-RISC 2.0 ");
    default:
        return ("An electronic chip with an HP label");

    }
#else
    return ("An electronic chip that makes the computer work.");
#endif
}



static int      done_coProc;

void
Init_HR_CoProc(void)
{
    done_coProc = 0;
}

int
Get_Next_HR_CoProc(void)
{
    /*
     * How to identify the presence of a co-processor ? 
     */

    if (done_coProc != 1) {
        done_coProc = 1;
        return (HRDEV_COPROC << HRDEV_TYPE_SHIFT);
    } else
        return -1;
}


const char     *
describe_coproc(int idx)
{
    return ("Guessing that there's a floating point co-processor");
}
