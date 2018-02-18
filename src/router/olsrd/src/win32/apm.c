/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifdef _WIN32

#include "apm.h"
#include "defs.h"
#include <stdio.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef interface

int
apm_init(void)
{
  struct olsr_apm_info ApmInfo;

  OLSR_PRINTF(3, "Initializing APM\n");

  if (apm_read(&ApmInfo) < 0)
    return -1;

  apm_printinfo(&ApmInfo);

  return 0;
}

void
apm_printinfo(struct olsr_apm_info *ApmInfo)
{
  OLSR_PRINTF(5, "APM info:\n\tAC status %d\n\tBattery percentage %d%%\n\n", ApmInfo->ac_line_status, ApmInfo->battery_percentage);
}

int
apm_read(struct olsr_apm_info *ApmInfo)
{
#if !defined WINCE
  SYSTEM_POWER_STATUS PowerStat;

  memset(ApmInfo, 0, sizeof(struct olsr_apm_info));

  if (!GetSystemPowerStatus(&PowerStat))
    return 0;

  ApmInfo->ac_line_status = (PowerStat.ACLineStatus == 1) ? OLSR_AC_POWERED : OLSR_BATTERY_POWERED;

  ApmInfo->battery_percentage = (PowerStat.BatteryLifePercent <= 100) ? PowerStat.BatteryLifePercent : 0;

  return 1;
#else /* !defined WINCE */
  return 0;
#endif /* !defined WINCE */
}

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
