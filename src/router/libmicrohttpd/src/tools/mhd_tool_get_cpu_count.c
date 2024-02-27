/*
 This file is part of GNU libmicrohttpd
  Copyright (C) 2023 Evgeny Grin (Karlson2k)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file tools/mhd_tool_get_cpu_count.c
 * @brief  Implementation of functions to detect the number of available
 *         CPU cores.
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include "mhd_tool_get_cpu_count.h"
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */
#ifdef HAVE_SYS__CPUSET_H
#  include <sys/_cpuset.h>
#endif /* HAVE_SYS_PARAM_H */
#ifdef HAVE_STDDEF_H
#  include <stddef.h>
#endif /* HAVE_STDDEF_H */
#ifdef HAVE_STRING_H
#  include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_SYS_SYSCTL_H
#  include <sys/sysctl.h>
#endif /* HAVE_SYS_SYSCTL_H */
#ifdef HAVE_FEATURES_H
#  include <features.h>
#endif /* HAVE_FEATURES_H */
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_VXCPULIB_H
#  include <vxCpuLib.h>
#endif
#ifdef HAVE_WINDOWS_H
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#  endif /* ! WIN32_LEAN_AND_MEAN */
#  include <windows.h>
#  ifndef ALL_PROCESSOR_GROUPS
#    define ALL_PROCESSOR_GROUPS 0xFFFFu
#  endif /* ALL_PROCESSOR_GROUPS */
#elif defined(_WIN32) && ! defined (__CYGWIN__)
#  error Windows headers are required for Windows build
#endif /* HAVE_WINDOWS_H */
#ifdef HAVE_SCHED_H
#  include <sched.h>
#endif /* HAVE_SCHED_H */
#ifdef HAVE_SYS_CPUSET_H
#  include <sys/cpuset.h>
#endif /* HAVE_SYS_CPUSET_H */
#ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
#endif /* HAVE_STDBOOL_H */

#if ! defined(HAS_DECL_CPU_SETSIZE) && ! defined(CPU_SETSIZE)
#  define CPU_SETSIZE (1024)
#  define CPU_SETSIZE_SAFE (64)
#else  /* HAS_DECL_CPU_SETSIZE || CPU_SETSIZE */
#  define CPU_SETSIZE_SAFE CPU_SETSIZE
#endif /* HAS_DECL_CPU_SETSIZE || CPU_SETSIZE */

/* Check and fix possible missing macros */
#if ! defined(HAS_DECL_CTL_HW) && defined(CTL_HW)
#  define HAS_DECL_CTL_HW 1
#endif /* ! HAS_DECL_CTL_HW && CTL_HW */

#if ! defined(HAS_DECL_HW_NCPUONLINE) && defined(HW_NCPUONLINE)
#  define HAS_DECL_HW_NCPUONLINE 1
#endif /* ! HAS_DECL_HW_NCPUONLINE && HW_NCPUONLINE */

#if ! defined(HAS_DECL_HW_AVAILCPU) && defined(HW_AVAILCPU)
#  define HAS_DECL_HW_AVAILCPU 1
#endif /* ! HAS_DECL_HW_AVAILCPU && HW_AVAILCPU */

#if ! defined(HAS_DECL_HW_NCPU) && defined(HW_NCPU)
#  define HAS_DECL_HW_NCPU 1
#endif /* ! HAS_DECL_HW_NCPU && HW_NCPU */

#if ! defined(HAS_DECL__SC_NPROCESSORS_ONLN) && defined(_SC_NPROCESSORS_ONLN)
#  define HAS_DECL__SC_NPROCESSORS_ONLN 1
#endif /* ! HAS_DECL__SC_NPROCESSORS_ONLN && _SC_NPROCESSORS_ONLN */

#if ! defined(HAS_DECL__SC_NPROC_ONLN) && defined(_SC_NPROC_ONLN)
#  define HAS_DECL__SC_NPROC_ONLN 1
#endif /* ! HAS_DECL__SC_NPROC_ONLN && _SC_NPROC_ONLN */

#if ! defined(HAS_DECL__SC_CRAY_NCPU) && defined(_SC_CRAY_NCPU)
#  define HAS_DECL__SC_CRAY_NCPU 1
#endif /* ! HAS_DECL__SC_CRAY_NCPU && _SC_CRAY_NCPU */

#if ! defined(HAS_DECL__SC_NPROCESSORS_CONF) && defined(_SC_NPROCESSORS_CONF)
#  define HAS_DECL__SC_NPROCESSORS_CONF 1
#endif /* ! HAVE_DECL__SC_NPROCESSORS_CONF && _SC_NPROCESSORS_CONF */

/* Forward declarations */

static int
mhd_tool_get_sys_cpu_count_sysctl_ (void);

/**
 * Detect the number of logical CPU cores available for the process by
 * sched_getaffinity() (and related) function.
 * @return the number of detected logical CPU cores or
 *         -1 if failed to detect (or this function unavailable).
 */
static int
mhd_tool_get_proc_cpu_count_sched_getaffinity_ (void)
{
  int ret = -1;
#if defined(HAVE_SCHED_GETAFFINITY) && defined(HAVE_GETPID)
  /* Glibc style */
  if (0 >= ret)
  {
    cpu_set_t cur_set;
    if (0 == sched_getaffinity (getpid (), sizeof (cur_set), &cur_set))
    {
#ifdef HAVE_CPU_COUNT
      ret = CPU_COUNT (&cur_set);
#else  /* ! HAVE_CPU_COUNT */
      unsigned int i;
      ret = 0;
      for (i = 0; i < CPU_SETSIZE_SAFE; ++i)
      {
        if (CPU_ISSET (i, &cur_set))
          ++ret;
      }
      if (0 == ret)
        ret = -1;
#endif /* ! HAVE_CPU_COUNT */
    }
  }
#ifdef HAVE_CPU_COUNT_S
  if (0 >= ret)
  {
    /* Use 256 times larger size than size for default maximum CPU number.
       Hopefully it would be enough even for exotic situations. */
    static const unsigned int set_size_cpus = 256 * CPU_SETSIZE;
    const size_t set_size_bytes = CPU_ALLOC_SIZE (set_size_cpus);
    cpu_set_t *p_set;

    p_set = CPU_ALLOC (set_size_cpus);
    if (NULL != p_set)
    {
      if (0 == sched_getaffinity (getpid (), set_size_bytes, p_set))
      {
#ifndef MHD_FUNC_CPU_COUNT_S_GETS_CPUS
        ret = CPU_COUNT_S (set_size_bytes, p_set);
#else  /* MHD_FUNC_CPU_COUNT_S_GETS_CPUS */
        ret = CPU_COUNT_S (set_size_cpus, p_set);
#endif /* MHD_FUNC_CPU_COUNT_S_GETS_CPUS */
      }
      CPU_FREE (p_set);
    }
  }
#endif /* HAVE_CPU_COUNT_S */
#endif /* HAVE_SCHED_GETAFFINITY && HAVE_GETPID */
  if (0 >= ret)
    return -1;
  return ret;
}


/**
 * Detect the number of logical CPU cores available for the process by
 * cpuset_getaffinity() function.
 * @return the number of detected logical CPU cores or
 *         -1 if failed to detect (or this function unavailable).
 */
static int
mhd_tool_get_proc_cpu_count_cpuset_getaffinity_ (void)
{
  int ret = -1;
#if defined(HAVE_CPUSET_GETAFFINITY)
  /* FreeBSD style */
  if (0 >= ret)
  {
    cpuset_t cur_mask;
    /* The should get "anonymous" mask/set. The anonymous mask is always
       a subset of the assigned set (which is a subset of the root set). */
    if (0 == cpuset_getaffinity (CPU_LEVEL_WHICH, CPU_WHICH_PID, (id_t) -1,
                                 sizeof (cur_mask), &cur_mask))
    {
#ifdef HAVE_CPU_COUNT
      ret = CPU_COUNT (&cur_mask);
#else  /* ! HAVE_CPU_COUNT */
      unsigned int i;
      ret = 0;
      for (i = 0; i < CPU_SETSIZE_SAFE; ++i)
      {
        if (CPU_ISSET (i, &cur_mask))
          ++ret;
      }
      if (0 == ret)
        ret = -1;
#endif /* ! HAVE_CPU_COUNT */
    }
  }
#ifdef HAVE_CPU_COUNT_S
  if (0 >= ret)
  {
    /* Use 256 times larger size than size for default maximum CPU number.
       Hopefully it would be enough even for exotic situations. */
    static const unsigned int mask_size_cpus = 256 * CPU_SETSIZE;
    const size_t mask_size_bytes = CPU_ALLOC_SIZE (mask_size_cpus);
    cpuset_t *p_mask;

    p_mask = CPU_ALLOC (mask_size_cpus);
    if (NULL != p_mask)
    {
      if (0 == cpuset_getaffinity (CPU_LEVEL_WHICH, CPU_WHICH_PID, (id_t) -1,
                                   mask_size_bytes, p_mask))
      {
#ifndef MHD_FUNC_CPU_COUNT_S_GETS_CPUS
        ret = CPU_COUNT_S (mask_size_bytes, p_mask);
#else  /* MHD_FUNC_CPU_COUNT_S_GETS_CPUS */
        ret = CPU_COUNT_S (mask_size_cpus, p_mask);
#endif /* MHD_FUNC_CPU_COUNT_S_GETS_CPUS */
      }
      CPU_FREE (p_mask);
    }
  }
#endif /* HAVE_CPU_COUNT_S */
#endif /* HAVE_CPUSET_GETAFFINITY */
  if (0 >= ret)
    return -1;
  return ret;
}


/**
 * Detect the number of logical CPU cores available for the process by
 * sched_getaffinity_np() (and related) function.
 * @return the number of detected logical CPU cores or
 *         -1 if failed to detect (or this function unavailable).
 */
static int
mhd_tool_get_proc_cpu_count_sched_getaffinity_np_ (void)
{
  int ret = -1;
#if defined(HAVE_SCHED_GETAFFINITY_NP) && defined(HAVE_GETPID)
  /* NetBSD style */
  cpuset_t *cpuset_ptr;
  cpuset_ptr = cpuset_create ();
  if (NULL != cpuset_ptr)
  {
    if (0 == sched_getaffinity_np (getpid (), cpuset_size (cpuset_ptr),
                                   cpuset_ptr))
    {
      cpuid_t cpu_num;
#if defined(HAVE_SYSCONF) && defined(HAVE_DECL__SC_NPROCESSORS_CONF)
      unsigned int max_num = 0;
      long sc_value;
      sc_value = sysconf (_SC_NPROCESSORS_ONLN);
      if (0 < sc_value)
        max_num = (unsigned int) sc_value;
      if (0 < max_num)
      {
        ret = 0;
        for (cpu_num = 0; cpu_num < max_num; ++cpu_num)
          if (0 < cpuset_isset (cpu_num, cpuset_ptr))
            ++ret;
      }
      else /* Combined with the next 'if' */
#endif /* HAVE_SYSCONF && HAVE_DECL__SC_NPROCESSORS_CONF */
      if (1)
      {
        int res;
        cpu_num = 0;
        ret = 0;
        do
        {
          res = cpuset_isset (cpu_num++, cpuset_ptr);
          if (0 < res)
            ++ret;
        } while (0 <= res);
      }
#ifdef __NetBSD__
      if (0 == ret)
      {
        /* On NetBSD "unset" affinity (exactly zero CPUs) means
           "all CPUs are available". */
        ret = mhd_tool_get_sys_cpu_count_sysctl_ ();
      }
#endif /* __NetBSD__ */
    }
    cpuset_destroy (cpuset_ptr);
  }
#endif /* HAVE_SCHED_GETAFFINITY_NP && HAVE_GETPID */
  if (0 >= ret)
    return -1;
  return ret;
}


/**
 * Detect the number of logical CPU cores available for the process by
 * W32 API functions.
 * @return the number of detected logical CPU cores or
 *         -1 if failed to detect (or this function unavailable).
 */
static int
mhd_tool_get_proc_cpu_count_w32_ (void)
{
  int ret = -1;
#if defined(_WIN32) && ! defined(__CYGWIN__)
  /* W32 Native */
  /**
   * Maximum used number of CPU groups.
   * Improvement: Implement dynamic allocation when it would be reasonable
   */
#define MHDT_MAX_GROUP_COUNT 128
  /**
   * The count of logical CPUs as returned by GetProcessAffinityMask()
   */
  int count_by_proc_aff_mask;
  count_by_proc_aff_mask = -1;
  if (1)
  {
    DWORD_PTR proc_aff;
    DWORD_PTR sys_aff;

    if (GetProcessAffinityMask (GetCurrentProcess (), &proc_aff, &sys_aff))
    {
      /* Count all set bits */
      for (count_by_proc_aff_mask = 0; 0 != proc_aff; proc_aff &= proc_aff - 1)
        ++count_by_proc_aff_mask;
    }
  }
  if (0 < count_by_proc_aff_mask)
  {
    HMODULE k32hndl;
    k32hndl = LoadLibraryA ("kernel32.dll");
    if (NULL != k32hndl)
    {
      typedef BOOL (WINAPI *GPGA_PTR)(HANDLE hProcess,
                                      PUSHORT GroupCount,
                                      PUSHORT GroupArray);
      GPGA_PTR ptrGetProcessGroupAffinity;
      ptrGetProcessGroupAffinity =
        (GPGA_PTR) (void *) GetProcAddress (k32hndl,
                                            "GetProcessGroupAffinity");
      if (NULL == ptrGetProcessGroupAffinity)
      {
        /* Windows version before Win7 */
        /* No processor groups supported, the process affinity mask gives full picture */
        ret = count_by_proc_aff_mask;
      }
      else
      {
        /* Windows version Win7 or later */
        /* Processor groups are supported */
        USHORT arr_elements = MHDT_MAX_GROUP_COUNT;
        USHORT groups_arr[MHDT_MAX_GROUP_COUNT]; /* Hopefully should be enough */
        /* Improvement: Implement dynamic allocation when it would be reasonable */
        /**
         * Exactly one processor group is assigned to the process
         */
        bool single_cpu_group_assigned; /**< Exactly one processor group is assigned to the process */
        struct mhdt_GR_AFFINITY
        {
          KAFFINITY Mask;
          WORD Group;
          WORD Reserved[3];
        };
        typedef BOOL (WINAPI *GPDCSM_PTR)(HANDLE Process,
                                          struct mhdt_GR_AFFINITY *CpuSetMasks,
                                          USHORT CpuSetMaskCount,
                                          USHORT *RequiredMaskCount);
        GPDCSM_PTR ptrGetProcessDefaultCpuSetMasks;
        bool win_fe_or_later;
        bool cpu_set_mask_assigned;

        single_cpu_group_assigned = false;
        if (ptrGetProcessGroupAffinity (GetCurrentProcess (), &arr_elements,
                                        groups_arr))
        {
          if (1 == arr_elements)
          {
            /* Exactly one processor group assigned to the process */
            single_cpu_group_assigned = true;
#if 0 /* Disabled code */
            /* The value returned by GetThreadGroupAffinity() is not relevant as
               for the new threads the process affinity mask is used. */
            ULONG_PTR proc_aff2;
            typedef BOOL (WINAPI *GTGA_PTR)(HANDLE hThread,
                                            struct mhdt_GR_AFFINITY *
                                            GroupAffinity);
            GTGA_PTR ptrGetThreadGroupAffinity;
            ptrGetThreadGroupAffinity =
              (GTGA_PTR) (void *) GetProcAddress (k32hndl,
                                                  "GetThreadGroupAffinity");
            if (NULL != ptrGetThreadGroupAffinity)
            {
              struct mhdt_GR_AFFINITY thr_gr_aff;
              if (ptrGetThreadGroupAffinity (GetCurrentThread (), &thr_gr_aff))
                proc_aff2 = (ULONG_PTR) thr_gr_aff.Mask;
            }
#endif /* Disabled code */
          }
        }
        ptrGetProcessDefaultCpuSetMasks =
          (GPDCSM_PTR) (void *) GetProcAddress (k32hndl,
                                                "GetProcessDefaultCpuSetMasks");
        if (NULL != ptrGetProcessDefaultCpuSetMasks)
        {
          /* This is Iron Release / Codename Fe
             (also know as Windows 11 and Windows Server 2022)
             or later version */
          struct mhdt_GR_AFFINITY gr_affs[MHDT_MAX_GROUP_COUNT]; /* Hopefully should be enough */
          /* Improvement: Implement dynamic allocation when it would be reasonable */
          USHORT num_elm;

          win_fe_or_later = true;

          if (ptrGetProcessDefaultCpuSetMasks (GetCurrentProcess (), gr_affs,
                                               sizeof (gr_affs)
                                               / sizeof (gr_affs[0]), &num_elm))
          {
            if (0 == num_elm)
            {
              /* No group mask set */
              cpu_set_mask_assigned = false;
            }
            else
              cpu_set_mask_assigned = true;
          }
          else
            cpu_set_mask_assigned = true; /* Assume the worst case */
        }
        else
        {
          win_fe_or_later = false;
          cpu_set_mask_assigned = false;
        }
        if (! win_fe_or_later)
        {
          /* The OS is not capable of distributing threads across different
             processor groups. Results reported by GetProcessAffinityMask()
             are relevant for the main processor group for the process. */
          ret = count_by_proc_aff_mask;
        }
        else
        {
          /* The of is capable of automatic threads distribution across
             processor groups. */
          if (cpu_set_mask_assigned)
          {
            /* Assigned Default CpuSet Masks combines with "classic"
               affinity in the not fully clear way. The combination
               is not documented and this functionality could be changed
               any moment. */
            ret = -1;
          }
          else
          {
            if (! single_cpu_group_assigned)
            {
              /* This is a multi processor group process on Win11 (or later).
                 Each processor group may have different affinity and
                 the OS has not API to get it.
                 For example, affinity to the main processor group could be
                 assigned by SetProcessAffinityMask() function, which converts
                 the process to the single-processor-group type, but if
                 SetThreadGroupAffinity() is called later and bind the thread
                 to another processor group, the process becomes multi-processor-
                 group again, however the initial affinity mask is still used
                 for the initial (main) processor group. There is no API to read
                 it.
                 It is also possible that processor groups have different number
                 of processors. */
              ret = -1;
            }
            else
            {
              /* Single-processor-group process on Win11 (or later) without
                 assigned Default CpuSet Masks. */
              ret = count_by_proc_aff_mask;
            }
          }
        }
      }
      FreeLibrary (k32hndl);
    }
  }
#endif /* _WIN32 && ! __CYGWIN__ */
  if (0 >= ret)
    return -1;
  return ret;
}


/**
 * Detect the number of logical CPU cores available for the process.
 * The number of cores available for this process could be different from
 * value of cores available on the system. The OS may have limit on number
 * assigned/allowed cores for single process and process may have limited
 * CPU affinity.
 * @return the number of logical CPU cores available for the process or
 *         -1 if failed to detect
 */
int
mhd_tool_get_proc_cpu_count (void)
{
  int res;

#if defined(__linux__) || defined(__GLIBC__)
  /* On Linux kernel try first 'sched_getaffinity()' as it should be
     the native API.
     Also try it first on other kernels if Glibc is used. */
  res = mhd_tool_get_proc_cpu_count_sched_getaffinity_ ();
  if (0 < res)
    return res;

  res = mhd_tool_get_proc_cpu_count_cpuset_getaffinity_ ();
  if (0 < res)
    return res;
#else  /* ! __linux__ && ! __GLIBC__ */
  /* On non-Linux kernels 'cpuset_getaffinity()' could be the native API,
     while 'sched_getaffinity()' could be implemented in compatibility layer. */
  res = mhd_tool_get_proc_cpu_count_cpuset_getaffinity_ ();
  if (0 < res)
    return res;

  res = mhd_tool_get_proc_cpu_count_sched_getaffinity_ ();
  if (0 < res)
    return res;
#endif /* ! __linux__ && ! __GLIBC__ */

  res = mhd_tool_get_proc_cpu_count_sched_getaffinity_np_ ();
  if (0 < res)
    return res;

  res = mhd_tool_get_proc_cpu_count_w32_ ();
  if (0 < res)
    return res;

  return -1;
}


/**
 * Detect the number of processors by special API functions
 * @return number of processors as returned by special API functions or
 *         -1 in case of error or special API functions unavailable
 */
static int
mhd_tool_get_sys_cpu_count_special_api_ (void)
{
  int ret = -1;
#ifdef HAVE_PSTAT_GETDYNAMIC
  if (0 >= ret)
  {
    /* HP-UX things */
    struct pst_dynamic psd_data;
    memset ((void *) &psd_data, 0, sizeof (psd_data));
    if (1 == pstat_getdynamic (&psd_data, sizeof (psd_data), (size_t) 1, 0))
    {
      if (0 < psd_data.psd_proc_cnt)
        ret = (int) psd_data.psd_proc_cnt;
    }
  }
#endif /* HAVE_PSTAT_GETDYNAMIC */
#ifdef HAVE_VXCPUENABLEDGET
  if (0 >= ret)
  {
    /* VxWorks */
    cpuset_t enb_set;
    enb_set = vxCpuEnabledGet ();
    /* Count set bits */
    for (ret = 0; 0 != enb_set; enb_set &= enb_set - 1)
      ++ret;
  }
#endif /* HAVE_VXCPUENABLEDGET */
#if defined(_WIN32) && ! defined (__CYGWIN__)
  if (0 >= ret)
  {
    /* Native W32 */
    HMODULE k32hndl;
    k32hndl = LoadLibraryA ("kernel32.dll");
    if (NULL != k32hndl)
    {
      typedef DWORD (WINAPI *GAPC_PTR)(WORD GroupNumber);
      GAPC_PTR ptrGetActiveProcessorCount;
      /* Available on W7 or later */
      ptrGetActiveProcessorCount =
        (GAPC_PTR) (void *) GetProcAddress (k32hndl, "GetActiveProcessorCount");
      if (NULL != ptrGetActiveProcessorCount)
      {
        DWORD res;
        res = ptrGetActiveProcessorCount (ALL_PROCESSOR_GROUPS);
        ret = (int) res;
        if (res != (DWORD) ret)
          ret = -1; /* Overflow */
      }
    }
    if ((0 >= ret) && (NULL != k32hndl))
    {
      typedef void (WINAPI *GNSI_PTR)(SYSTEM_INFO *pSysInfo);
      GNSI_PTR ptrGetNativeSystemInfo;
      /* May give incorrect (low) result on versions from W7 to W11
         when more then 64 CPUs are available */
      ptrGetNativeSystemInfo =
        (GNSI_PTR) (void *) GetProcAddress (k32hndl, "GetNativeSystemInfo");
      if (NULL != ptrGetNativeSystemInfo)
      {
        SYSTEM_INFO sysInfo;

        memset ((void *) &sysInfo, 0, sizeof (sysInfo));
        ptrGetNativeSystemInfo (&sysInfo);
        ret = (int) sysInfo.dwNumberOfProcessors;
        if (sysInfo.dwNumberOfProcessors != (DWORD) ret)
          ret = -1; /* Overflow */
      }
    }
    if (NULL != k32hndl)
      FreeLibrary (k32hndl);
  }
  if (0 >= ret)
  {
    /* May give incorrect (low) result on versions from W7 to W11
       when more then 64 CPUs are available */
    SYSTEM_INFO sysInfo;
    memset ((void *) &sysInfo, 0, sizeof (sysInfo));
    GetSystemInfo (&sysInfo);
    ret = (int) sysInfo.dwNumberOfProcessors;
    if (sysInfo.dwNumberOfProcessors != (DWORD) ret)
      ret = -1; /* Overflow */
  }
#endif /* _WIN32 && ! __CYGWIN__ */
  if (0 >= ret)
    return -1;
  return ret;
}


/**
 * Detect the number of processors by sysctl*() functions in reliable way.
 *
 * This function uses reliable identificators that coreponds to actual
 * number of CPU cores online currently.
 * @return number of processors as returned by 'sysctl*' functions or
 *         -1 in case of error or the number cannot be detected
 *         by these functions
 */
static int
mhd_tool_get_sys_cpu_count_sysctl_ (void)
{
  int ret = -1;
  /* Do not use sysctl() function on GNU/Linux even if
     sysctl() is available */
#ifndef __linux__
#ifdef HAVE_SYSCTLBYNAME
  if (0 >= ret)
  {
    size_t value_size = sizeof (ret);
    /* Darwin: The number of available logical CPUs */
    if ((0 != sysctlbyname ("hw.logicalcpu", &ret, &value_size,
                            NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
  if (0 >= ret)
  {
    size_t value_size = sizeof (ret);
    /* FreeBSD: The number of online CPUs */
    if ((0 != sysctlbyname ("kern.smp.cpus", &ret, &value_size,
                            NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
  if (0 >= ret)
  {
    size_t value_size = sizeof (ret);
    /* Darwin: The current number of CPUs available to run threads */
    if ((0 != sysctlbyname ("hw.activecpu", &ret, &value_size,
                            NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
  if (0 >= ret)
  {
    size_t value_size = sizeof (ret);
    /* OpenBSD, NetBSD: The number of online CPUs */
    if ((0 != sysctlbyname ("hw.ncpuonline", &ret, &value_size,
                            NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
  if (0 >= ret)
  {
    size_t value_size = sizeof (ret);
    /* Darwin: The old/alternative name for "hw.activecpu" */
    if ((0 != sysctlbyname ("hw.availcpu", &ret, &value_size,
                            NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
#endif /* HAVE_SYSCTLBYNAME */
#if defined(HAVE_SYSCTL) && \
  defined(HAS_DECL_CTL_HW) && \
  defined(HAS_DECL_HW_NCPUONLINE)
  if (0 >= ret)
  {
    /* OpenBSD, NetBSD: The number of online CPUs */
    int mib[2] = {CTL_HW, HW_NCPUONLINE};
    size_t value_size = sizeof (ret);
    if ((0 != sysctl (mib, 2, &ret, &value_size, NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
#endif /* HAVE_SYSCTL && HAS_DECL_CTL_HW && HAS_DECL_HW_NCPUONLINE */
#if defined(HAVE_SYSCTL) && \
  defined(HAS_DECL_CTL_HW) && \
  defined(HAS_DECL_HW_AVAILCPU)
  if (0 >= ret)
  {
    /* Darwin: The MIB name for "hw.activecpu" */
    int mib[2] = {CTL_HW, HW_AVAILCPU};
    size_t value_size = sizeof (ret);
    if ((0 != sysctl (mib, 2, &ret, &value_size, NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
#endif /* HAVE_SYSCTL && HAS_DECL_CTL_HW && HAS_DECL_HW_AVAILCPU */
#endif /* ! __linux__ */
  if (0 >= ret)
    return -1;
  return ret;
}


/**
 * Detect the number of processors by sysctl*() functions, using fallback way.
 *
 * This function uses less reliable (compared to
 * #mhd_tool_get_sys_cpu_count_sysctl_()) ways to detect the number of
 * available CPU cores and may return values corresponding to the number of
 * physically available (but possibly not used by the kernel) CPU logical cores.
 * @return number of processors as returned by 'sysctl*' functions or
 *         -1 in case of error or the number cannot be detected
 *         by these functions
 */
static int
mhd_tool_get_sys_cpu_count_sysctl_fallback_ (void)
{
  int ret = -1;
  /* Do not use sysctl() function on GNU/Linux even if
     sysctl() is available */
#ifndef __linux__
#ifdef HAVE_SYSCTLBYNAME
  if (0 >= ret)
  {
    size_t value_size = sizeof (ret);
    /* FreeBSD, OpenBSD, NetBSD, Darwin (and others?): The number of CPUs */
    if ((0 != sysctlbyname ("hw.ncpu", &ret, &value_size,
                            NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
#endif /* HAVE_SYSCTLBYNAME */
#if defined(HAVE_SYSCTL) && \
  defined(HAS_DECL_CTL_HW) && \
  defined(HAS_DECL_HW_NCPU)
  if (0 >= ret)
  {
    /* FreeBSD, OpenBSD, NetBSD, Darwin (and others?): The number of CPUs */
    int mib[2] = {CTL_HW, HW_NCPU};
    size_t value_size = sizeof (ret);
    if ((0 != sysctl (mib, 2, &ret, &value_size, NULL, 0))
        || (sizeof (ret) != value_size))
      ret = -1;
  }
#endif /* HAVE_SYSCTL && HAS_DECL_CTL_HW && HAS_DECL_HW_NCPU */
#endif /* ! __linux__ */
  if (0 >= ret)
    return -1;
  return ret;
}


/**
 * Detect the number of processors by sysconf() function in reliable way.
 *
 * This function uses reliable identificators that coreponds to actual
 * number of CPU cores online currently.
 * @return number of processors as returned by 'sysconf' function or
 *         -1 in case of error or 'sysconf' unavailable
 */
static int
mhd_tool_get_sys_cpu_count_sysconf_ (void)
{
  int ret = -1;
#if defined(HAVE_SYSCONF) && \
  (defined(HAS_DECL__SC_NPROCESSORS_ONLN) || defined(HAS_DECL__SC_NPROC_ONLN))
  long value = -1;
#ifdef HAS_DECL__SC_NPROCESSORS_ONLN
  if (0 >= value)
    value = sysconf (_SC_NPROCESSORS_ONLN);
#endif /* HAS_DECL__SC_NPROCESSORS_ONLN */
#ifdef HAS_DECL__SC_NPROC_ONLN
  if (0 >= value)
    value = sysconf (_SC_NPROC_ONLN);
#endif /* HAS_DECL__SC_NPROC_ONLN */
  if (0 >= value)
    return -1;
  ret = (int) value;
  if ((long) ret != value)
    return -1; /* Overflow */
#endif /* HAVE_SYSCONF &&
          (HAS_DECL__SC_NPROCESSORS_ONLN || HAS_DECL__SC_NPROC_ONLN) */
  return ret;
}


/**
 * Detect the number of processors by sysconf() function, using fallback way.
 *
 * This function uses less reliable (compared to
 * #mhd_tool_get_sys_cpu_count_sysconf_()) ways to detect the number of
 * available CPU cores and may return values corresponding to the number of
 * physically available (but possibly not used by the kernel) CPU logical cores.
 * @return number of processors as returned by 'sysconf' function or
 *         -1 in case of error or 'sysconf' unavailable
 */
static int
mhd_tool_get_sys_cpu_count_sysconf_fallback_ (void)
{
  int ret = -1;
#if defined(HAVE_SYSCONF) && \
  (defined(HAS_DECL__SC_CRAY_NCPU) || defined(HAS_DECL__SC_NPROCESSORS_CONF))
  long value = -1;
#ifdef HAS_DECL__SC_CRAY_NCPU
  if (0 >= value)
    value = sysconf (_SC_CRAY_NCPU);
#endif /* HAS_DECL__SC_CRAY_NCPU */
#ifdef HAS_DECL__SC_NPROCESSORS_CONF
  if (0 >= value)
    value = sysconf (_SC_NPROCESSORS_CONF);
#endif /* HAS_DECL__SC_NPROCESSORS_CONF */
  if (0 >= value)
    return -1;
  ret = (int) value;
  if ((long) ret != value)
    return -1; /* Overflow */
#endif /* HAVE_SYSCONF &&
          (HAS_DECL__SC_CRAY_NCPU || HAS_DECL__SC_NPROCESSORS_CONF) */
  return ret;
}


/**
 * Try to detect the number of logical CPU cores available for the system.
 * The number of available logical CPU cores could be changed any time due to
 * CPU hotplug.
 * @return the number of logical CPU cores available,
 *         -1 if failed to detect.
 */
int
mhd_tool_get_system_cpu_count (void)
{
  int res;

  /* Try specialised APIs first */
  res = mhd_tool_get_sys_cpu_count_special_api_ ();
  if (0 < res)
    return res;

  /* Try sysctl*(). This is typically a direct interface to
     kernel values. */
  res = mhd_tool_get_sys_cpu_count_sysctl_ ();
  if (0 < res)
    return res;

  /* Try sysconf() as the last resort as this is a generic interface
     which can be implemented by parsing system files. */
  res = mhd_tool_get_sys_cpu_count_sysconf_ ();
#if ! defined(__linux__) && ! defined(__GLIBC__)
  if (0 < res)
    return res;
#else  /* __linux__ || __GLIBC__ */
  if (2 < res)
    return res;
  if (0 < res)
  {
    /* '1' or '2' could a be fallback number.
     * See get_nprocs_fallback() in glibc
       sysdeps/unix/sysv/linux/getsysstats.c */

    int proc_cpu_count;

    proc_cpu_count = mhd_tool_get_proc_cpu_count ();
    if (proc_cpu_count == res)
    {
      /* The detected number of CPUs available for the process
         is equal to the detected number of system CPUs.
         Assume detected number is correct. */
      return res;
    }
  }
#endif /* __linux__ || __GLIBC__  */

  /* Try available fallbacks */

  res = mhd_tool_get_sys_cpu_count_sysctl_fallback_ ();
  if (0 < res)
    return res;

  res = mhd_tool_get_sys_cpu_count_sysconf_fallback_ ();
#if ! defined(__linux__) && ! defined(__GLIBC__)
  if (0 < res)
    return res;
#else  /* __linux__ || __GLIBC__ */
  if (2 < res)
    return res;
#endif /* __linux__ || __GLIBC__  */

  return -1; /* Cannot detect */
}
