/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2013-2014
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Header file for configuration module
  */

#ifndef GOT_CONF_H
#define GOT_CONF_H

#include "addressing.h"
#include "array.h"
#include "reference.h"
#include "sources.h"

extern void CNF_Initialise(int restarted, int client_only);
extern void CNF_Finalise(void);

extern void CNF_EnablePrint(void);

extern char *CNF_GetRtcDevice(void);

extern void CNF_ReadFile(const char *filename);
extern void CNF_ParseLine(const char *filename, int number, char *line);

extern void CNF_CreateDirs(uid_t uid, gid_t gid);

extern void CNF_CheckReadOnlyAccess(void);

extern void CNF_AddInitSources(void);
extern void CNF_AddSources(void);
extern void CNF_AddBroadcasts(void);
extern void CNF_AddRefclocks(void);

extern void CNF_ReloadSources(void);

extern int CNF_GetAcquisitionPort(void);
extern int CNF_GetNTPPort(void);
extern char *CNF_GetDriftFile(int *interval);
extern char *CNF_GetLogDir(void);
extern char *CNF_GetDumpDir(void);
extern int CNF_GetLogBanner(void);
extern int CNF_GetLogMeasurements(int *raw);
extern int CNF_GetLogSelection(void);
extern int CNF_GetLogStatistics(void);
extern int CNF_GetLogTracking(void);
extern int CNF_GetLogRtc(void);
extern int CNF_GetLogRefclocks(void);
extern int CNF_GetLogTempComp(void);
extern char *CNF_GetKeysFile(void);
extern char *CNF_GetRtcFile(void);
extern int CNF_GetManualEnabled(void);
extern ARR_Instance CNF_GetOpenCommands(void);
extern int CNF_GetCommandPort(void);
extern int CNF_GetRtcOnUtc(void);
extern int CNF_GetRtcSync(void);
extern void CNF_GetMakeStep(int *limit, double *threshold);
extern void CNF_GetMaxChange(int *delay, int *ignore, double *offset);
extern double CNF_GetLogChange(void);
extern void CNF_GetMailOnChange(int *enabled, double *threshold, char **user);
extern int CNF_GetNoClientLog(void);
extern unsigned long CNF_GetClientLogLimit(void);
extern void CNF_GetFallbackDrifts(int *min, int *max);
extern void CNF_GetBindAddress(int family, IPAddr *addr);
extern void CNF_GetBindAcquisitionAddress(int family, IPAddr *addr);
extern void CNF_GetBindCommandAddress(int family, IPAddr *addr);
extern char *CNF_GetBindNtpInterface(void);
extern char *CNF_GetBindAcquisitionInterface(void);
extern char *CNF_GetBindCommandInterface(void);
extern char *CNF_GetBindCommandPath(void);
extern int CNF_GetNtpDscp(void);
extern char *CNF_GetNtpSigndSocket(void);
extern char *CNF_GetPidFile(void);
extern REF_LeapMode CNF_GetLeapSecMode(void);
extern char *CNF_GetLeapSecTimezone(void);
extern char *CNF_GetLeapSecList(void);

/* Value returned in ppm, as read from file */
extern double CNF_GetMaxUpdateSkew(void);
extern double CNF_GetMaxClockError(void);
extern double CNF_GetMaxDrift(void);
extern double CNF_GetCorrectionTimeRatio(void);
extern double CNF_GetMaxSlewRate(void);
extern double CNF_GetClockPrecision(void);

extern SRC_AuthSelectMode CNF_GetAuthSelectMode(void);
extern double CNF_GetMaxDistance(void);
extern double CNF_GetMaxJitter(void);
extern double CNF_GetReselectDistance(void);
extern double CNF_GetStratumWeight(void);
extern double CNF_GetCombineLimit(void);

extern int CNF_AllowLocalReference(int *stratum, int *orphan, double *distance, double *activate,
                                   double *wait_synced, double *wait_unsynced);

extern void CNF_SetupAccessRestrictions(void);

extern int CNF_GetSchedPriority(void);
extern int CNF_GetLockMemory(void);

extern int CNF_GetNTPRateLimit(int *interval, int *burst, int *leak, int *kod);
extern int CNF_GetNtsRateLimit(int *interval, int *burst, int *leak);
extern int CNF_GetCommandRateLimit(int *interval, int *burst, int *leak);
extern void CNF_GetSmooth(double *max_freq, double *max_wander, int *leap_only);
extern void CNF_GetTempComp(char **file, double *interval, char **point_file, double *T0, double *k0, double *k1, double *k2);

extern char *CNF_GetUser(void);

extern int CNF_GetMaxSamples(void);
extern int CNF_GetMinSamples(void);

extern int CNF_GetMinSources(void);

extern double CNF_GetRtcAutotrim(void);
extern char *CNF_GetHwclockFile(void);

extern int CNF_GetInitSources(void);
extern double CNF_GetInitStepThreshold(void);

typedef enum {
  CNF_HWTS_RXFILTER_ANY,
  CNF_HWTS_RXFILTER_NONE,
  CNF_HWTS_RXFILTER_NTP,
  CNF_HWTS_RXFILTER_PTP,
  CNF_HWTS_RXFILTER_ALL,
} CNF_HwTs_RxFilter;

typedef struct {
  char *name;
  int minpoll;
  int maxpoll;
  int min_samples;
  int max_samples;
  int nocrossts;
  CNF_HwTs_RxFilter rxfilter;
  double precision;
  double tx_comp;
  double rx_comp;
} CNF_HwTsInterface;

extern int CNF_GetHwTsInterface(unsigned int index, CNF_HwTsInterface **iface);
extern double CNF_GetHwTsTimeout(void);

extern int CNF_GetPtpPort(void);
extern int CNF_GetPtpDomain(void);

extern int CNF_GetRefresh(void);

extern ARR_Instance CNF_GetNtsAeads(void);
extern char *CNF_GetNtsDumpDir(void);
extern char *CNF_GetNtsNtpServer(void);
extern int CNF_GetNtsServerCertAndKeyFiles(const char ***certs, const char ***keys);
extern int CNF_GetNtsServerPort(void);
extern int CNF_GetNtsServerProcesses(void);
extern int CNF_GetNtsServerConnections(void);
extern int CNF_GetNtsRefresh(void);
extern int CNF_GetNtsRotate(void);
extern int CNF_GetNtsTrustedCertsPaths(const char ***paths, uint32_t **ids);
extern int CNF_GetNoSystemCert(void);
extern int CNF_GetNoCertTimeCheck(void);

#endif /* GOT_CONF_H */
