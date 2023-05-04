/*
 **
 **  reload.c
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License Version 2 as
 **  published by the Free Software Foundation.  You may not use, modify or
 **  distribute this program under any other version of the GNU General
 **  Public License.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GETTID
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>

#include "reload.h"
#include "snort.h"
#include "parser.h"
#include "sfcontrol_funcs.h"
#include "sfcontrol.h"
#include "mpse.h"
#include "sp_flowbits.h"
#include "sp_dynamic.h"
#include "rate_filter.h"
#include "file_service_config.h"
#include "so_rule_mem_adjust.h"
#include "file_service.h"
#ifdef INTEL_SOFT_CPM
#include "intel-soft-cpm.h"
#endif
#ifdef HAVE_MALLOC_TRIM
#include <malloc.h>
#endif
#ifdef SIDE_CHANNEL
# include "sidechannel.h"
#endif
#ifdef SNORT_RELOAD
#include "sftarget_reader.h"
#endif
#ifdef REG_TEST
#include "reg_test.h"
#endif

extern volatile int attribute_reload_thread_running;
extern int reload_attribute_table_flags;

#if defined(SNORT_RELOAD)

void ReloadFreeAdjusters(SnortConfig* sc)
{
    ReloadAdjustEntry* rae;

    while ((rae = sc->raEntry))
    {
        sc->raEntry = rae->raNext;
        if (rae->raUserFreeFunc && rae->raUserData) 
            rae->raUserFreeFunc(rae->raUserData);
        free(rae);
    }
    if ((rae = sc->raSessionEntry))
    {
        if (rae->raUserFreeFunc && rae->raUserData)
            rae->raUserFreeFunc(rae->raUserData);
        free(rae);
        sc->raSessionEntry = NULL;
    }
}

static ReloadAdjustEntry* ReloadAdjustRegisterAlloc(const char* raName, tSfPolicyId raPolicyId,
                                                    ReloadAdjustFunc raFunc, void* raUserData,
                                                    ReloadAdjustUserFreeFunc raUserFreeFunc)
{
    static const char* raUnspecified = "Unspecified";
    ReloadAdjustEntry* rae;

    if ((rae = (ReloadAdjustEntry*)calloc(1, sizeof*rae)))
    {
        rae->raName = raName ? raName : raUnspecified;
        rae->raPolicyId = raPolicyId; 
        rae->raFunc = raFunc;
        rae->raUserData = raUserData;
        rae->raUserFreeFunc = raUserFreeFunc;
        LogMessage("Registered %s to adjust for reload.\n", rae->raName); 
    }
    return rae;
}

int ReloadAdjustRegister(SnortConfig* sc, const char* raName, tSfPolicyId raPolicyId,
                         ReloadAdjustFunc raFunc, void* raUserData,
                         ReloadAdjustUserFreeFunc raUserFreeFunc)
{
    ReloadAdjustEntry* rae;

    rae = ReloadAdjustRegisterAlloc(raName, raPolicyId, raFunc, raUserData, raUserFreeFunc);
    if (!rae)
        return -1;
    rae->raNext = sc->raEntry;
    sc->raEntry = rae;
    return 0;
}

int ReloadAdjustSessionRegister(SnortConfig* sc, const char* raName, tSfPolicyId raPolicyId,
                                ReloadAdjustFunc raFunc, void* raUserData,
                                ReloadAdjustUserFreeFunc raUserFreeFunc)
{
    ReloadAdjustEntry* rae;

    rae = ReloadAdjustRegisterAlloc(raName, raPolicyId, raFunc, raUserData, raUserFreeFunc);
    if (!rae)
        return -1;
    sc->raSessionEntry = rae; 
    return 0;
}

#endif

#if defined(SNORT_RELOAD) && !defined(WIN32) 
volatile bool reloadInProgress = false;
static pthread_mutex_t reload_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#if defined(SNORT_RELOAD) && !defined(WIN32)
static volatile int snort_swapped = 0;
SnortConfig *snort_conf_new = NULL;
static SnortConfig *snort_conf_old = NULL;
#endif

bool SnortDynamicLibsChanged(void)
{
#ifdef SNORT_RELOAD
    if (detection_lib_changed)
        return true;
#endif
    return false;
}

void CheckForReload(void)
{
#if defined(SNORT_RELOAD) && !defined(WIN32)

    /* Check for a new configuration */
    if (snort_reload)
    {
        PostConfigFuncNode *idxPlugin;
        snort_reload = 0;

        /* There was an error reloading.  A non-reloadable configuration
         * option changed */
        if (snort_conf_new == NULL)
        {
#ifdef RELOAD_ERROR_FATAL
            CleanExit(1);
#else
            Restart();
#endif
        }

        snort_conf_old = snort_conf;
        snort_conf = snort_conf_new;
        snort_conf_new = NULL;

        FileServiceInstall();

        ScRestoreInternalLogLevel();

        SwapPreprocConfigurations(snort_conf);

        /* Need to do this here because there is potentially outstanding
         * state data pointing to the previous configuration.  A race
         * condition is created if these are free'd in the reload thread
         * where a double free could occur. */
        FreeSwappedPreprocConfigurations(snort_conf);

#ifdef INTEL_SOFT_CPM
        if (snort_conf->fast_pattern_config->search_method == MPSE_INTEL_CPM)
            IntelPmActivate(snort_conf);
#endif

        snort_swapped = 1;
         setNapRuntimePolicy( getParserPolicy(snort_conf) );
         setIpsRuntimePolicy( getParserPolicy(snort_conf) );

        /* Do any reload for plugin data */
        idxPlugin = plugin_reload_funcs;
        while(idxPlugin)
        {
            idxPlugin->func(snort_conf, SIGHUP, idxPlugin->arg);
            idxPlugin = idxPlugin->next;
        }

    }
#endif
}

#if defined(SNORT_RELOAD) && !defined(WIN32)
static int VerifyLibInfos(DynamicLibInfo *old_info, DynamicLibInfo *new_info)
{
    if ((old_info != NULL) && (new_info != NULL))
    {
        unsigned i;

        if (old_info->type != new_info->type)
        {
            FatalError("%s(%d) Incompatible library types.\n",
                       __FILE__, __LINE__);
        }

        if (old_info->count != new_info->count)
            return -1;

        for (i = 0; i < old_info->count; i++)
        {
            unsigned j;
            DynamicLibPath *old_path = old_info->lib_paths[i];

            for (j = 0; j < new_info->count; j++)
            {
                DynamicLibPath *new_path = new_info->lib_paths[j];

                if ((strcmp(old_path->path, new_path->path) == 0) &&
                    (old_path->ptype == new_path->ptype))
                {
                   if (old_path->last_mod_time != new_path->last_mod_time)
                       return -1;

                    break;
                }
            }

            if (j == new_info->count)
                return -1;
        }
    }
    else if (old_info != new_info)
    {
        return -1;
    }

    return 0;
}

#if defined(SFLINUX) || defined(WRLINUX)
static int VerifyDynamicLibInfo(DynamicLibInfo *old_info, DynamicLibInfo *new_info)
{
    if ((old_info != NULL) && (new_info != NULL))
    {
        if (old_info->type != new_info->type)
        {
            FatalError("%s(%d) Incompatible library types.\n",
                       __FILE__, __LINE__);
        }
        if (strcmp(old_info->lib_paths[0]->path, new_info->lib_paths[0]->path) != 0)
        {
             return -1;
        }
    } 
    else if (old_info != new_info)
    {
        return -1;
    }

    return 0;
}
#endif

static int VerifyOutputs(SnortConfig *old_config, SnortConfig *new_config)
{
    OutputConfig *old_output_config, *new_output_config;
    int old_outputs = 0, new_outputs = 0;

    /* Get from output_configs to see if output has changed */
    for (old_output_config = old_config->output_configs;
         old_output_config != NULL;
         old_output_config = old_output_config->next)
    {
        old_outputs++;
    }

    for (new_output_config = new_config->output_configs;
         new_output_config != NULL;
         new_output_config = new_output_config->next)
    {
        new_outputs++;
    }

    if (new_outputs != old_outputs)
    {
        ErrorMessage("Snort Reload: Any change to any output "
                     "configurations requires a restart.\n");
        return -1;
    }

    for (old_output_config = old_config->output_configs;
         old_output_config != NULL;
         old_output_config = old_output_config->next)
    {

        for (new_output_config = new_config->output_configs;
                new_output_config != NULL;
                new_output_config = new_output_config->next)
        {
            if (strcasecmp(old_output_config->keyword, new_output_config->keyword) == 0)
            {
                if ((old_output_config->opts != NULL) &&
                        (new_output_config->opts != NULL) &&
                        (strcasecmp(old_output_config->opts, new_output_config->opts) == 0))
                {
                    new_outputs++;
                    break;
                }
                else if (old_output_config->opts == NULL &&
                        new_output_config->opts == NULL)
                {
                    new_outputs++;
                    break;
                }
            }

        }

        old_outputs++;
    }

    if (new_outputs != old_outputs)
    {
        ErrorMessage("Snort Reload: Any change to any output "
                     "configurations requires a restart.\n");
        return -1;
    }

    /* Check user defined rule type outputs */
    for (old_output_config = old_config->rule_type_output_configs;
         old_output_config != NULL;
         old_output_config = old_output_config->next)
    {
        old_outputs++;
    }

    for (new_output_config = new_config->rule_type_output_configs;
         new_output_config != NULL;
         new_output_config = new_output_config->next)
    {
        new_outputs++;
    }

    if (new_outputs != old_outputs)
    {
        ErrorMessage("Snort Reload: Any change to any output "
                     "configurations requires a restart.\n");
        return -1;
    }

    /* Do user defined rule type outputs as well */
    for (old_output_config = old_config->rule_type_output_configs;
         old_output_config != NULL;
         old_output_config = old_output_config->next)
    {
        for (new_output_config = new_config->rule_type_output_configs;
             new_output_config != NULL;
             new_output_config = new_output_config->next)
        {
            if (strcasecmp(old_output_config->keyword,
                           new_output_config->keyword) == 0)
            {
                if (old_output_config->opts && new_output_config->opts)
                {
                    if (strcasecmp(old_output_config->opts,
                                   new_output_config->opts) == 0)
                    {
                        new_outputs++;
                        break;
                    }
                }
                if (!old_output_config->opts && !new_output_config->opts)
                {
                    new_outputs++;
                    break;
                }
            }
        }

        old_outputs++;
    }

    if (new_outputs != old_outputs)
    {
        ErrorMessage("Snort Reload: Any change to any output "
                     "configurations requires a restart.\n");
        return -1;
    }

    return 0;
}

static int VerifyReload(SnortConfig *sc)
{
    if (sc == NULL)
        return -1;

#ifdef TARGET_BASED
    {
        SnortPolicy *p1 = sc->targeted_policies[getDefaultPolicy()];
        SnortPolicy *p2 = snort_conf->targeted_policies[getDefaultPolicy()];

        if ((p1->target_based_config.args != NULL) &&
            (p2->target_based_config.args != NULL))
        {
            if (strcasecmp(p1->target_based_config.args,
                           p2->target_based_config.args) != 0)
            {
                ErrorMessage("Snort Reload: Changing the attribute table "
                             "configuration requires a restart.\n");
                return -1;
            }
        }
    }
#endif

    if ((snort_conf->alert_file != NULL) && (sc->alert_file != NULL))
    {
        if (strcasecmp(snort_conf->alert_file, sc->alert_file) != 0)
        {
            ErrorMessage("Snort Reload: Changing the alert file "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->alert_file != sc->alert_file)
    {
        ErrorMessage("Snort Reload: Changing the alert file "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->asn1_mem != sc->asn1_mem)
    {
        ErrorMessage("Snort Reload: Changing the asn1 memory configuration "
                     "requires a restart.\n");
        return -1;
    }

    if ((sc->bpf_filter == NULL) && (sc->bpf_file != NULL))
        sc->bpf_filter = read_infile(sc->bpf_file);

    if ((sc->bpf_filter != NULL) && (snort_conf->bpf_filter != NULL))
    {
        if (strcasecmp(snort_conf->bpf_filter, sc->bpf_filter) != 0)
        {
            ErrorMessage("Snort Reload: Changing the bpf filter configuration "
                         "requires a restart.\n");
            return -1;
        }
    }
    else if (sc->bpf_filter != snort_conf->bpf_filter)
    {
        ErrorMessage("Snort Reload: Changing the bpf filter configuration "
                     "requires a restart.\n");
        return -1;
    }

#ifdef ACTIVE_RESPONSE
    if ( sc->respond_attempts != snort_conf->respond_attempts ||
         sc->respond_device != snort_conf->respond_device )
    {
        ErrorMessage("Snort Reload: Changing config response "
                     "requires a restart.\n");
        return -1;
    }
#endif

    if ((snort_conf->chroot_dir != NULL) &&
        (sc->chroot_dir != NULL))
    {
        if (strcasecmp(snort_conf->chroot_dir, sc->chroot_dir) != 0)
        {
            ErrorMessage("Snort Reload: Changing the chroot directory "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->chroot_dir != sc->chroot_dir)
    {
        ErrorMessage("Snort Reload: Changing the chroot directory "
                     "configuration requires a restart.\n");
        return -1;
    }

    if ((snort_conf->run_flags & RUN_FLAG__DAEMON) !=
        (sc->run_flags & RUN_FLAG__DAEMON))
    {
        ErrorMessage("Snort Reload: Changing to or from daemon mode "
                     "requires a restart.\n");
        return -1;
    }

    if ((snort_conf->interface != NULL) && (sc->interface != NULL))
    {
        if (strcasecmp(snort_conf->interface, sc->interface) != 0)
        {
            ErrorMessage("Snort Reload: Changing the interface "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->interface != sc->interface)
    {
        ErrorMessage("Snort Reload: Changing the interface "
                     "configuration requires a restart.\n");
        return -1;
    }

    /* Orig log dir because a chroot might have changed it */
    if ((snort_conf->orig_log_dir != NULL) &&
        (sc->orig_log_dir != NULL))
    {
        if (strcasecmp(snort_conf->orig_log_dir, sc->orig_log_dir) != 0)
        {
            ErrorMessage("Snort Reload: Changing the log directory "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->orig_log_dir != sc->orig_log_dir)
    {
        ErrorMessage("Snort Reload: Changing the log directory "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->no_log != sc->no_log)
    {
        ErrorMessage("Snort Reload: Changing from log to no log or vice "
                     "versa requires a restart.\n");
        return -1;
    }

    if ((snort_conf->run_flags & RUN_FLAG__NO_PROMISCUOUS) !=
        (sc->run_flags & RUN_FLAG__NO_PROMISCUOUS))
    {
        ErrorMessage("Snort Reload: Changing to or from promiscuous mode "
                     "requires a restart.\n");
        return -1;
    }


#ifdef PERF_PROFILING
    if ((snort_conf->profile_rules.num != sc->profile_rules.num) ||
        (snort_conf->profile_rules.sort != sc->profile_rules.sort) ||
        (snort_conf->profile_rules.append != sc->profile_rules.append))
    {
        ErrorMessage("Snort Reload: Changing rule profiling number, sort "
                     "or append configuration requires a restart.\n");
        return -1;
    }

    if ((snort_conf->profile_rules.filename != NULL) &&
        (sc->profile_rules.filename != NULL))
    {
        if (strcasecmp(snort_conf->profile_rules.filename,
                       sc->profile_rules.filename) != 0)
        {
            ErrorMessage("Snort Reload: Changing the rule profiling filename "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->profile_rules.filename !=
             sc->profile_rules.filename)
    {
        ErrorMessage("Snort Reload: Changing the rule profiling filename "
                     "configuration requires a restart.\n");
        return -1;
    }

    if ((snort_conf->profile_preprocs.num !=  sc->profile_preprocs.num) ||
        (snort_conf->profile_preprocs.sort != sc->profile_preprocs.sort) ||
        (snort_conf->profile_preprocs.append != sc->profile_preprocs.append))
    {
        ErrorMessage("Snort Reload: Changing preprocessor profiling number, "
                     "sort or append configuration requires a restart.\n");
        return -1;
    }

    if ((snort_conf->profile_preprocs.filename != NULL) &&
        (sc->profile_preprocs.filename != NULL))
    {
        if (strcasecmp(snort_conf->profile_preprocs.filename,
                       sc->profile_preprocs.filename) != 0)
        {
            ErrorMessage("Snort Reload: Changing the preprocessor profiling "
                         "filename configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->profile_preprocs.filename !=
             sc->profile_preprocs.filename)
    {
        ErrorMessage("Snort Reload: Changing the preprocessor profiling "
                     "filename configuration requires a restart.\n");
        return -1;
    }
#endif

    if (snort_conf->group_id != sc->group_id)
    {
        ErrorMessage("Snort Reload: Changing the group id "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->user_id != sc->user_id)
    {
        ErrorMessage("Snort Reload: Changing the user id "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->pkt_snaplen != sc->pkt_snaplen)
    {
        ErrorMessage("Snort Reload: Changing the packet snaplen "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->threshold_config->memcap !=
        sc->threshold_config->memcap)
    {
        ErrorMessage("Snort Reload: Changing the threshold memcap "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->rate_filter_config->memcap !=
        sc->rate_filter_config->memcap)
    {
        ErrorMessage("Snort Reload: Changing the rate filter memcap "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->detection_filter_config->memcap !=
        sc->detection_filter_config->memcap)
    {
        ErrorMessage("Snort Reload: Changing the detection filter memcap "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (VerifyLibInfos(snort_conf->dyn_engines, sc->dyn_engines) == -1)
    {
        ErrorMessage("Snort Reload: Any change to the dynamic engine "
                     "configuration requires a restart.\n");
        return -1;
    }

#if defined(SFLINUX) || defined(WRLINUX)
    if (VerifyDynamicLibInfo(snort_conf->dyn_rules, sc->dyn_rules) == -1)
#else
    if (VerifyLibInfos(snort_conf->dyn_rules, sc->dyn_rules) == -1)
#endif
    {
        LogMessage("Snort Reload: Dynamic detection libs have changed.\n");
        detection_lib_changed = 1;
    }

    if (VerifyLibInfos(snort_conf->dyn_preprocs, sc->dyn_preprocs) == -1)
    {
        ErrorMessage("Snort Reload: Any change to the dynamic preprocessor "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->so_rule_memcap != sc->so_rule_memcap)
    {
        AdjustSoRuleMemory(sc, snort_conf);
    }

    if (VerifyOutputs(snort_conf, sc) == -1)
        return -1;

#ifdef SIDE_CHANNEL
    if (SideChannelVerifyConfig(sc) != 0)
    {
        ErrorMessage("Snort Reload: Changing the side channel configuration requires a restart.\n");
        return -1;
    }
#endif

    if (ScMaxIP6Extensions() != sc->max_ip6_extensions)
    {
	ErrorMessage("Snort Reload: Changing Max IP6 extensions "
		     "configuration requires a restart.\n");
	return -1;
    }

    return 0;
}

static SnortConfig * ReloadConfig(void)
{
    SnortConfig *sc;

    if (ScSuppressConfigLog())
        ScSetInternalLogLevel(INTERNAL_LOG_LEVEL__ERROR);

#ifdef HAVE_MALLOC_TRIM
    malloc_trim(0);
#endif

    sc = ParseSnortConf();

    sc = MergeSnortConfs(snort_cmd_line_conf, sc);

    sc->reloadPolicyFlag = 1;

#ifdef PERF_PROFILING
    /* Parse profiling here because of file option and potential
     * dependence on log directory */
    {
        char *opts = NULL;
        int in_table;

        in_table = sfghash_find2(sc->config_table,
                                 CONFIG_OPT__PROFILE_PREPROCS, (void *)&opts);
        if (in_table)
            ConfigProfilePreprocs(sc, opts);

        in_table = sfghash_find2(sc->config_table,
                                 CONFIG_OPT__PROFILE_RULES, (void *)&opts);
        if (in_table)
            ConfigProfileRules(sc, opts);
    }
#endif

    if (VerifyReload(sc) == -1)
    {
        SnortConfFree(sc);
        return NULL;
    }

    DAQ_UpdateTunnelBypass(sc);

    if (sc->output_flags & OUTPUT_FLAG__USE_UTC)
        sc->thiszone = 0;
    else
        sc->thiszone = gmt2local(0);

#ifdef TARGET_BASED
    SFAT_ReloadCheck(sc);
#endif

    /* Preprocessors will have a reload callback */
    ConfigurePreprocessors(sc, 1);

    FlowbitResetCounts();
    ParseRules(sc);
    RuleOptParseCleanup();

    /* Check if Dynamic detection libs have changed */
    if (!detection_lib_changed) 
    {
        pthread_mutex_lock(&dynamic_rules_lock);
        ReloadDynamicRules(sc);
        pthread_mutex_unlock(&dynamic_rules_lock);
    } 
    else 
    {
        ReloadDynamicDetectionLibs(sc);
        InitDynamicDetectionPlugins(sc);
    }

    /* Handles Fatal Errors itself. */
    SnortEventqNew(sc->event_queue_config, sc->event_queue);

    detection_filter_print_config(sc->detection_filter_config);
    RateFilter_PrintConfig(sc->rate_filter_config);
    print_thresholding(sc->threshold_config, 0);
    PrintRuleOrder(sc->rule_lists);

    SetRuleStates(sc);

    if (file_sevice_config_verify(snort_conf, sc) == -1)
    {
        SnortConfFree(sc);
        return NULL;
    }

    if (VerifyReloadedPreprocessors(sc))
    {
        SnortConfFree(sc);
        return NULL;
    }

    if (CheckPreprocessorsConfig(sc))
    {
        SnortConfFree(sc);
        return NULL;
    }

    FilterConfigPreprocessors(sc);
    PostConfigPreprocessors(sc);
    
    sc->udp_ips_port_filter_list = ParseIpsPortList(sc, IPPROTO_UDP);

    /* Need to do this after dynamic detection stuff is initialized, too */
    FlowBitsVerify();

    if ((sc->file_mask != 0) && (sc->file_mask != snort_conf->file_mask))
        umask(sc->file_mask);

    /* Transfer any user defined rule type outputs to the new rule list */
    {
        RuleListNode *cur = snort_conf->rule_lists;

        for (; cur != NULL; cur = cur->next)
        {
            RuleListNode *new = sc->rule_lists;

            for (; new != NULL; new = new->next)
            {
                if (strcasecmp(cur->name, new->name) == 0)
                {
                    OutputFuncNode *alert_list = cur->RuleList->AlertList;
                    OutputFuncNode *log_list = cur->RuleList->LogList;

                    sc->head_tmp = new->RuleList;

                    for (; alert_list != NULL; alert_list = alert_list->next)
                    {
                        AddFuncToOutputList(sc, alert_list->func,
                                            OUTPUT_TYPE__ALERT, alert_list->arg);
                    }

                    for (; log_list != NULL; log_list = log_list->next)
                    {
                        AddFuncToOutputList(sc, log_list->func,
                                            OUTPUT_TYPE__LOG, log_list->arg);
                    }

                    sc->head_tmp = NULL;
                    break;
                }
            }
        }
    }

    /* XXX XXX Can't do any output plugins */
    //PostConfigInitPlugins(sc->plugin_post_config_funcs);

    fpCreateFastPacketDetection(sc);

#ifdef PPM_MGR
    PPM_PRINT_CFG(&sc->ppm_cfg);
#endif

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
    // This is needed when PPM is disabled and enabling snort-engine debugs
    if (!ppm_tpu)
       ppm_tpu = (PPM_TICKS)get_ticks_per_usec();
#endif

    // Restores the configured logging level, if it was suppressed earlier
    ScRestoreInternalLogLevel();

    return sc;
}

static void updatePeriodicCheck(void)
{
    PeriodicCheckFuncNode *checkFunc;

    /* reset preprocessors */
    checkFunc = periodic_check_funcs;
    while (checkFunc != NULL)
    {
        if ( 0 == checkFunc->time_left )
        {
            checkFunc->func(-1, checkFunc->arg);
            checkFunc->time_left = checkFunc->period;
            //LogMessage("        --== Share Memory! ==--\n");
        }
        else
            checkFunc->time_left--;

        checkFunc = checkFunc->next;
    }

}

void * ReloadConfigThread(void *data)
{
    sigset_t mtmask;

    /* Don't handle any signals here */
    sigfillset(&mtmask);
    pthread_sigmask(SIG_BLOCK, &mtmask, NULL);

    snort_reload_thread_pid = gettid();
    snort_reload_thread_created = 1;

    while (snort_initializing)
        nanosleep(&thread_sleep, NULL);

    while (!snort_exiting)
    {
        if (reload_signal != reload_total)
        {
            int reload_failed = 0;

#ifdef CONTROL_SOCKET
            pthread_mutex_lock(&reload_mutex);
            if (!reloadInProgress)
            {
                reloadInProgress = true;
                pthread_mutex_unlock(&reload_mutex);
#endif
                reload_total++;

                LogMessage("\n");
                LogMessage("        --== Reloading Snort ==--\n");
                LogMessage("\n");

                snort_conf_new = ReloadConfig();

                // Restore Log level if we suppressed it earlier

                if (snort_conf_new == NULL)
                    reload_failed = 1;
                snort_reload = 1;

                while (!snort_swapped && !snort_exiting)
                    nanosleep(&thread_sleep, NULL);

                snort_swapped = 0;

                SnortConfFree(snort_conf_old);
                snort_conf_old = NULL;

#ifdef INTEL_SOFT_CPM
                if (snort_conf->fast_pattern_config->search_method != MPSE_INTEL_CPM)
                    IntelPmStopInstance();
#endif

                if (snort_exiting && !reload_failed)
                {
                    /* This will load the new preprocessor configurations and
                     * free the old ones, so any preprocessor cleanup that
                     * requires a configuration will be using the new one
                     * unless it relies on old configurations that are still
                     * attached to existing sessions. */
                    SwapPreprocConfigurations(snort_conf);
                    FreeSwappedPreprocConfigurations(snort_conf);
#if defined (SIDE_CHANNEL) && defined (REG_TEST)
                    if (snort_conf && snort_conf->file_config)
                      FileSSConfigFree(snort_conf->file_config);
#endif

#ifdef CONTROL_SOCKET
                    reloadInProgress = false;
#endif
                    /* Get out of the loop and exit */
                    break;
                }

                if (!reload_failed)
                {
#if defined(TARGET_BASED) && !defined(WIN32)
		    if(!IsAdaptiveConfigured())
		    {
			if(attribute_reload_thread_running)
			{
			    ReloadAttributeThreadStop();
			    reload_attribute_table_flags = 0;
#ifdef REG_TEST
			    if(REG_TEST_FLAG_RELOAD & getRegTestFlags())
			    {
				printf("Adaptive profile disabled, attribute table cleared\n");
			    }
#endif
			}
			SFAT_Cleanup();
			SnortAddSignal(SIGNAL_SNORT_READ_ATTR_TBL, SigNoAttributeTableHandler, 1);
		    }
		    else if(IsAdaptiveConfigured() && ScDisableAttrReload(snort_conf))
		    {
			if(attribute_reload_thread_running)
			{
			    ReloadAttributeThreadStop();
			    SFAT_CleanPrevConfig();
		        }
			SnortAddSignal(SIGNAL_SNORT_READ_ATTR_TBL, SigNoAttributeTableHandler, 1);
		    }
		    else if(IsAdaptiveConfigured() && !ScDisableAttrReload(snort_conf))
		    {
			SnortAddSignal(SIGNAL_SNORT_READ_ATTR_TBL,SigAttributeTableReloadHandler,0);
		    }
#endif
		    while (snort_conf->raEntry)
		    {
		        sleep(1);
		    }

                    LogMessage("\n");
                    LogMessage("        --== Reload Complete ==--\n");
                    LogMessage("\n");
                }
#ifdef CONTROL_SOCKET
                reloadInProgress = false;
            }
            else
                pthread_mutex_unlock(&reload_mutex);
#endif
        }
        /* Use the maintenance thread for periodic check*/
        updatePeriodicCheck();

        sleep(1);
    }

    pthread_exit((void *)0);
}
#endif

#if defined(SNORT_RELOAD) && !defined(WIN32) && defined(CONTROL_SOCKET)

static int ControlSocketReloadConfig(uint16_t type, const uint8_t *data, uint32_t length, void **new_config,
                                     char *statusBuf, int statusBuf_len)
{
    SnortConfig *new_sc;

    pthread_mutex_lock(&reload_mutex);
    while (reloadInProgress || SnortIsInitializing())
    {
        sleep(1);
    }
    reloadInProgress = true;
    pthread_mutex_unlock(&reload_mutex);

    LogMessage("\n");
    LogMessage("        --== Reloading Snort ==--\n");
    LogMessage("\n");

    new_sc = ReloadConfig();
    if (new_sc == NULL)
    {
        reloadInProgress = false;
        return -1;
    }
    *new_config = (void *)new_sc;
    return 0;
}

static int ControlSocketReloadSwap(uint16_t type, void *new_config, void **old_config)
{
    PostConfigFuncNode *idxPlugin = NULL;

    *old_config = (void *)snort_conf;
    snort_conf = (SnortConfig *)new_config;
    FileServiceInstall();
    SwapPreprocConfigurations(snort_conf);

#ifdef INTEL_SOFT_CPM
    if (snort_conf->fast_pattern_config->search_method == MPSE_INTEL_CPM)
        IntelPmActivate(snort_conf);
#endif

    /* Do any reload for plugin data */
    idxPlugin = plugin_reload_funcs;
    while(idxPlugin)
    {
        idxPlugin->func(snort_conf, SIGHUP, idxPlugin->arg);
        idxPlugin = idxPlugin->next;
    }
    if (snort_conf->raSessionEntry) 
    {
        /* Session is always the first to adjust */
        snort_conf->raSessionEntry->raNext = snort_conf->raEntry; 
        snort_conf->raEntry = snort_conf->raSessionEntry;
        snort_conf->raSessionEntry = NULL;
    }
    setNapRuntimePolicy( getParserPolicy(snort_conf) );
    setIpsRuntimePolicy( getParserPolicy(snort_conf) );
#if defined(REG_TEST)  && defined(PPM_MGR)
    if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
    {
       static char prev_ppm_cfg ;
       if (snort_conf->ppm_cfg.rule_log & PPM_LOG_MESSAGE)
       {
           if (! ( prev_ppm_cfg & PPM_LOG_MESSAGE) )
            {
               printf("ppm rule-log set to PPM_LOG_MESSAGE\n");
               prev_ppm_cfg |= PPM_LOG_MESSAGE;
            }
       }

       if( snort_conf->ppm_cfg.rule_log & PPM_LOG_ALERT )
       {
          if ( ! ( prev_ppm_cfg & PPM_LOG_ALERT) )
          {
             printf("ppm rule-log set to PPM_LOG_ALERT\n");
             prev_ppm_cfg |= PPM_LOG_ALERT;
          }
       }
    }
#endif
    return 0;
}

static void ControlSocketReloadFree(uint16_t type, void *old_config, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    SnortConfig *old_sc = (SnortConfig *)old_config;

    FreeSwappedPreprocConfigurations(snort_conf);

    SnortConfFree(old_sc);

#ifdef INTEL_SOFT_CPM
    if (snort_conf->fast_pattern_config->search_method != MPSE_INTEL_CPM)
        IntelPmStopInstance();
#endif

#if defined(TARGET_BASED) && !defined(WIN32)
    if(!IsAdaptiveConfigured())
    {
	if(attribute_reload_thread_running)
	{
	    ReloadAttributeThreadStop();
	    reload_attribute_table_flags = 0;
#ifdef REG_TEST
	    if(REG_TEST_FLAG_RELOAD & getRegTestFlags())
	    {
		printf("Adaptive profile disabled, attribute table cleared\n");
	    }
#endif
	}
	SFAT_Cleanup();
	SnortAddSignal(SIGNAL_SNORT_READ_ATTR_TBL, SigNoAttributeTableHandler, 1);
    }
    else if(IsAdaptiveConfigured() && ScDisableAttrReload(snort_conf))
    {
	if(attribute_reload_thread_running)
	{
	    ReloadAttributeThreadStop();
	    SFAT_CleanPrevConfig();
	}
	SnortAddSignal(SIGNAL_SNORT_READ_ATTR_TBL, SigNoAttributeTableHandler, 1);
    }
    else if(IsAdaptiveConfigured() && !ScDisableAttrReload(snort_conf))
    {
	SnortAddSignal(SIGNAL_SNORT_READ_ATTR_TBL,SigAttributeTableReloadHandler,0);
    }
#endif

    while (snort_conf->raEntry && !snort_exiting )
        sleep(1);

    if( !snort_conf->raEntry )
    {
        LogMessage("\n");
        LogMessage("        --== Reload Complete ==--\n");
        LogMessage("\n");
    }
    else
    {
        LogMessage("            Reloading Aborted \n");
    }

    reloadInProgress = false;
}

#endif

void ReloadControlSocketRegister(void)
{
#if defined(SNORT_RELOAD) && !defined(WIN32) && defined(CONTROL_SOCKET)
    if (ControlSocketRegisterHandler(CS_TYPE_RELOAD, &ControlSocketReloadConfig, &ControlSocketReloadSwap, &ControlSocketReloadFree))
    {
        LogMessage("Failed to register the reload control handler.\n");
    }
#endif
}


