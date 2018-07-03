/* These define should be sync with bcmiqosd's content
 * 10/21/15, Sinclair
 */

#ifndef BCM_IQOS_DEF_H
#define BCM_IQOS_DEF_H

/* Folder */
#define IQOS_RUNTIME_FOLDER             "/tmp/trend"
#define IQOS_SRC_FOLDER                 "/usr/sbin"
#define RAMFS_CONFMTD_DIR               "/tmp/media/nand"

/* File */
#ifdef BRCM_GENERIC_IQOS
#define IQOS_QOS_CONF_FILE              "qos.conf"
#define IQOS_TMP_QOS_CONF_FILE          "qos.conf.tmp"
#else   /* BRCM_GENERIC_IQOS */
#define IQOS_QOS_CONF_FILE              "qosd.conf"
#define IQOS_TMP_QOS_CONF_FILE          "qosd.conf.tmp"
#endif  /* BRCM_GENERIC_IQOS */
#define IQOS_SETUP_SH_FILE              "setup.sh"
#define IQOS_QOS_SH_FILE                "qos.sh"
#define IQOS_INITIALIZER_FILE           "sample.bin"
#define IQOS_RULE_FILE                  "rule.trf"
#define IQOS_RULE_AGENT_FILE            "tdts_rule_agent"
#define IQOS_DEV_MAPPING_TBL_FILE       "TmToNtgr_dev_mapping"

/* Noted that, according to BRCM, the modules below doesn't provide any
 * analogical meaning between BRCM's version & Trend's version.
 * They are substantially different in the architecture. (CSP #975037)
 * */
#ifdef BRCM_GENERIC_IQOS
#define IQOS_IDP_MOD_FILE               "tdts.ko"
#define IQOS_UDB_MOD_FILE               "tdts_udb.ko"
#define IQOS_FW_MOD_FILE                "tdts_udbfw.ko"
#else   /* BRCM_GENERIC_IQOS */
#define IQOS_IDP_MOD_FILE               "IDP.ko"
#define IQOS_UDB_MOD_FILE               "tc_cmd.ko"
#define IQOS_FW_MOD_FILE                "bw_forward.ko"
#endif  /* BRCM_GENERIC_IQOS */

/* Path */
#define IQOS_QOS_CONF_PATH          IQOS_RUNTIME_FOLDER"/"IQOS_QOS_CONF_FILE
#define IQOS_TMP_QOS_CONF_PATH      IQOS_RUNTIME_FOLDER"/"IQOS_TMP_QOS_CONF_FILE
#define IQOS_SETUP_SH_PATH          IQOS_RUNTIME_FOLDER"/"IQOS_SETUP_SH_FILE
#define IQOS_QOS_SH_PATH            IQOS_RUNTIME_FOLDER"/"IQOS_QOS_SH_FILE
#define IQOS_INITIALIZER_PATH       IQOS_RUNTIME_FOLDER"/"IQOS_INITIALIZER_FILE
#define IQOS_RULE_PATH              IQOS_RUNTIME_FOLDER"/"IQOS_RULE_FILE
#define IQOS_RULE_AGENT_PATH        IQOS_RUNTIME_FOLDER"/"IQOS_RULE_AGENT_FILE

#define IQOS_DEV_MAPPING_TBL_PATH   IQOS_RUNTIME_FOLDER"/"IQOS_DEV_MAPPING_TBL_FILE

/* Command */
/* These two commands won't rm/insert the module. 
 * They just pause/trigger the TC rule adding service of IQOS modules*/
#ifdef BRCM_GENERIC_IQOS
#define IQOS_SYS_CMD_ENABLE_QOS_SERVICE     IQOS_INITIALIZER_FILE" -a set_qos_on"
#define IQOS_SYS_CMD_DISABLE_QOS_SERVICE    IQOS_INITIALIZER_FILE" -a set_qos_off"
#else   /* BRCM_GENERIC_IQOS */
#define IQOS_SYS_CMD_ENABLE_QOS_SERVICE     "echo start > /proc/iqos_ctrl"
#define IQOS_SYS_CMD_DISABLE_QOS_SERVICE    "echo stop > /proc/iqos_ctrl"
#endif  /* BRCM_GENERIC_IQOS */

#endif      /* BCM_IQOS_DEF_H */
