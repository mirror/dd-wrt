/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This software may be redistributed and/or modified under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 *
 * File: rhine_proc.c
 *
 * Purpose: Functions for dumping meeage by proc
 *
 * Author: Chuang Liang-Shing, AJ Jiang
 *
 * Date: Aug 15, 2003
 *
 */

#include "rhine.h"
#include "rhine_proc.h"

static const char* RHINE_PROC_DIR_NAME="Rhine_Adapters";
static struct proc_dir_entry *rhine_dir = NULL;

static int FunVerRead(char *page, char **start, off_t off, int count, int *eof, void *data);
static int FunStatRead(PRHINE_PROC_ENTRY pInfo,char* buf);
static int FunConfRead(PRHINE_PROC_ENTRY pInfo,char* buf);
static int FunConfWrite(PRHINE_PROC_ENTRY pInfo,const char* buf,unsigned long len);
static int FunRMONRead(PRHINE_PROC_ENTRY pInfo,char* buf);
typedef
enum _proc_conf_type {
    CONF_RX_DESC=0,
    CONF_TX_DESC,
    CONF_RX_THRESH,
    CONF_TX_THRESH,
    CONF_DMA_LEN,
    CONF_SPD_DPX,
    CONF_FLOW_CTRL,
    CONF_WOL_OPTS,
    CONF_ENABLE_TAG,
    CONF_VID_SETTING,
    CONF_VAL_PKT,
} PROC_CONF_TYPE, *PPROC_CONF_TYPE;

static const RHINE_PROC_ENTRY rhine_proc_tab_conf[]={
{"RxDescriptors", RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_RX_DESC,    NULL,   0},
{"TxDescriptors", RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_TX_DESC,    NULL,   0},
{"tx_thresh",     RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_TX_THRESH,  NULL,   0},
{"rx_thresh",     RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_RX_THRESH,  NULL,   0},
{"speed_duplex",  RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_SPD_DPX,    NULL,   0},
{"flow_control",  RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_FLOW_CTRL,  NULL,   0},
{"DMA_length",    RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_DMA_LEN,    NULL,   0},
{"ValPktLen",     RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_VAL_PKT,    NULL,   0},
{"wol_opts",      RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_WOL_OPTS,   NULL,   REV_ID_VT6102_A},
{"enable_tagging",RHINE_PROC_FILE,  FunConfRead, FunConfWrite, CONF_ENABLE_TAG, NULL,   REV_ID_VT6105M_A0},
{"VID_setting",RHINE_PROC_FILE, FunConfRead, FunConfWrite, CONF_VID_SETTING,    NULL,   REV_ID_VT6105M_A0},
{"",              RHINE_PROC_EOT,   NULL,   NULL,   0,  NULL}
};



static const RHINE_PROC_ENTRY rhine_proc_tab_rmon[]={
{"etherStatsDropEvents",            RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_DropEvents, NULL},
{"etherStatsOctets",                RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Octets,     NULL},
{"etherStatsPkts",                  RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Pkts,       NULL},
{"etherStatsBroadcastPkts",         RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_BroadcastPkts,  NULL},
{"etherStatsMulticastPkts",         RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_MulticastPkts,  NULL},
{"etherStatsCRCAlignErrors",        RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_CRCAlignErrors, NULL},
{"etherStatsUndersizePkts",         RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_UndersizePkts,NULL},
{"etherStatsOversizePkts",          RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_OversizePkts,NULL},
{"etherStatsFragments",             RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Fragments,NULL},
{"etherStatsJabbers",               RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Jabbers,NULL},
{"etherStatsCollisions",            RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Collisions, NULL},
{"etherStatsPkts64Octets",          RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Pkts64Octets,   NULL},
{"etherStatsPkts65to127Octets",     RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Pkts65to127Octets,  NULL},
{"etherStatsPkts128to255Octets",    RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Pkts128to255Octets, NULL},
{"etherStatsPkts256to511Octets",    RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Pkts256to511Octets, NULL},
{"etherStatsPkts512to1023Octets",   RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Pkts512to1023Octets,    NULL},
{"etherStatsPkts1024to1518Octets",  RHINE_PROC_READ,
    FunRMONRead, NULL, RMON_Pkts1024to1518Octets,   NULL},
{"",                                RHINE_PROC_EOT,
    NULL,   NULL,   0,  NULL}
};

static const RHINE_PROC_ENTRY rhine_proc_tab[]={
{"conf",    RHINE_PROC_DIR,     NULL,       NULL,   0,  rhine_proc_tab_conf},
{"rmon",    RHINE_PROC_DIR,     NULL,       NULL,   0,  rhine_proc_tab_rmon},
{"statics", RHINE_PROC_READ,    FunStatRead,NULL,   0,  NULL},
{"",    RHINE_PROC_EOT, NULL,   NULL,   0,  NULL}
};

static int rhine_proc_read(char *page, char **start, off_t off, int count,
    int *eof, void *data) {
    PRHINE_PROC_ENTRY pEntry=(PRHINE_PROC_ENTRY) data;
    int             len;

    len=pEntry->read_proc(pEntry, page);

    page[len++]='\n';

    if (len <= off + count)
        *eof = 1;

    *start = page + off;
    len -= off;

    if (len > count)
        len = count;

    if (len < 0)
        len = 0;
    return len;
}

static int rhine_proc_write(struct file *filp, const char* buffer,
        unsigned long count, void *data)
{
    int res=0;
    PRHINE_PROC_ENTRY pEntry=(PRHINE_PROC_ENTRY) data;
    res=pEntry->write_proc(pEntry,buffer,count);
    return res;
}

static void rhine_create_proc_tab(PRHINE_INFO pInfo,
    PRHINE_PROC_ENTRY   pParent,    const RHINE_PROC_ENTRY pTab[]) {

    int i;
    struct proc_dir_entry* ptr;
    PRHINE_PROC_ENTRY pEntry=NULL;
    PRHINE_PROC_ENTRY pRoot=NULL;

    if (pTab==NULL)
        return;

    for (i=0;pTab[i].type!=RHINE_PROC_EOT;i++) {
        //Skip some entries

        if ((pTab[i].byRevId!=0) && (pInfo->hw.byRevId<pTab[i].byRevId)) {
//          printk("pTab.byRevid=%d,%d\n",pTab[i].byRevId,pInfo->byRevId);
            continue;
        }
        pEntry=kmalloc(sizeof(RHINE_PROC_ENTRY),GFP_KERNEL);
        memcpy(pEntry,&pTab[i],sizeof(RHINE_PROC_ENTRY));
        pEntry->siblings=pRoot;
        pRoot=pEntry;

        if (pEntry->type & RHINE_PROC_DIR) {
            ptr=create_proc_entry(pEntry->name,S_IFDIR, pParent->pOsEntry);
            pEntry->pOsParent=pParent->pOsEntry;
            pEntry->pOsEntry=ptr;
            rhine_create_proc_tab(pInfo,pEntry,pTab[i].childs);
        }
        else {
            int flag=S_IFREG;
            if (pEntry->type & RHINE_PROC_READ)
                flag|=S_IRUGO;
            if (pEntry->type & RHINE_PROC_WRITE)
                flag|=S_IWUSR;
            ptr=create_proc_entry(pEntry->name,flag, pParent->pOsEntry);
            if (pEntry->type & RHINE_PROC_READ)
                ptr->read_proc=rhine_proc_read;
            if (pEntry->type & RHINE_PROC_WRITE)
                ptr->write_proc=rhine_proc_write;
            ptr->data=pEntry;
            pEntry->pOsEntry=ptr;
            pEntry->pInfo=pInfo;
            pEntry->pOsParent=pParent->pOsEntry;
        }
    }
    pParent->childs=pRoot;
}

static void rhine_delete_proc_tab(PRHINE_INFO pInfo,
    const PRHINE_PROC_ENTRY pEntry) {

    if (pEntry==NULL)
        return;

    if (pEntry->type & RHINE_PROC_DIR)
        rhine_delete_proc_tab(pInfo,(PRHINE_PROC_ENTRY) pEntry->childs);
    else
        rhine_delete_proc_tab(pInfo,(PRHINE_PROC_ENTRY) pEntry->siblings);

    remove_proc_entry(pEntry->name,pEntry->pOsParent);
    kfree(pEntry);
}

BOOL    rhine_create_proc_entry(PRHINE_INFO pInfo) {
    //PRHINE_INFO pInfo = dev->priv;
    struct net_device *dev = pInfo->dev;
   
    //Create ethX directory as root directory
    pInfo->pProcDir=kmalloc(sizeof(RHINE_PROC_ENTRY),GFP_KERNEL);
    memset(pInfo->pProcDir,0,sizeof(RHINE_PROC_ENTRY));

    pInfo->pProcDir->pOsEntry=
        create_proc_entry(dev->name,S_IFDIR,rhine_dir);
    pInfo->pProcDir->pOsParent=rhine_dir;

    //Create all other directoires according the defined entries on table
    rhine_create_proc_tab(pInfo, pInfo->pProcDir, rhine_proc_tab);
    return TRUE;
}

void    rhine_free_proc_entry(PRHINE_INFO pInfo) {
    //PRHINE_INFO pInfo = dev->priv;
    struct net_device *dev = pInfo->dev;
    
    rhine_delete_proc_tab(pInfo, (PRHINE_PROC_ENTRY) pInfo->pProcDir->childs);
    remove_proc_entry(dev->name, rhine_dir);
    kfree(pInfo->pProcDir);
}

BOOL    rhine_init_proc_fs(PRHINE_INFO pInfo) {
    struct proc_dir_entry* ptr;
    int len=strlen(RHINE_PROC_DIR_NAME);

    if (rhine_dir==NULL) {
        for (rhine_dir = proc_net->subdir; rhine_dir;rhine_dir = rhine_dir->next) {
            if ((rhine_dir->namelen == len) &&
                (!memcmp(rhine_dir->name, RHINE_PROC_DIR_NAME, len)))
                break;
        }

        if (rhine_dir==NULL) {
            rhine_dir=create_proc_entry(RHINE_PROC_DIR_NAME,S_IFDIR,proc_net);
            ptr=create_proc_entry("version",S_IFREG|S_IRUGO,rhine_dir);
            ptr->data=NULL;
            ptr->write_proc=NULL;
            ptr->read_proc=FunVerRead;
        }
    }

    if (rhine_dir==NULL)
        return FALSE;

    return TRUE;
}

void    rhine_free_proc_fs(PRHINE_INFO pInfo) {
    struct proc_dir_entry*  ptr;

//  remove_proc_entry(pInfo->pProcDir, rhine_dir);

    if (rhine_dir==NULL)
        return;

    //Check if other adapters's entry still exist
    for (ptr = rhine_dir->subdir; ptr; ptr = ptr->next) {
        if ((*(ptr->name) != '.') &&
            (strcmp(ptr->name,"version")))
            break;
    }

    if (ptr)
        return;

    remove_proc_entry("version",rhine_dir);
    remove_proc_entry(RHINE_PROC_DIR_NAME,proc_net);

    rhine_dir=NULL;
}

static long atol(const char* ptr,int len) {
    unsigned long l=0;
    while (*ptr!=0 && *ptr!='\n' && len-->0 ) {
        if (*ptr< '0' || *ptr >'9')
            return -1;
        l*=10;
        l+=(*ptr++-'0');
    }
    return l;
}

//----------------------------------
//
static int FunVerRead(char *page, char **start, off_t off, int count,
    int *eof, void *data) {
    int             len;

    len=sprintf(page,"%s",RHINE_VERSION);

    page[len++]='\n';

    if (len <= off + count)
        *eof = 1;

    *start = page + off;
    len -= off;

    if (len > count)
        len = count;

    if (len < 0)
        len = 0;
    return len;
}

static int FunStatRead(PRHINE_PROC_ENTRY pEntry,char* buf) {
    PRHINE_INFO pInfo=pEntry->pInfo;
    int         len=0;

    if (pInfo->hw.byRevId>=REV_ID_VT6105M_A0) {
        int i;
        spin_lock_irq(&pInfo->lock);
        rhine_UpdateMIBCounter(pInfo);
        spin_unlock_irq(&pInfo->lock);
        len+=sprintf(&buf[len],"Hardware MIB Counter:\n");
        for (i=0;i<HW_MIB_SIZE;i++)
            len+=sprintf(&buf[len],"%d:\t%d\n",i+1,pInfo->adwHWMIBCounters[i]);
    }
    len=strlen(buf);
    return len;
}

static int FunConfRead(PRHINE_PROC_ENTRY pEntry,char* buf) {
    PRHINE_INFO pInfo=pEntry->pInfo;
    int len=0;
    PROC_CONF_TYPE  op=pEntry->data;
    switch(op) {
    case CONF_RX_DESC:
        len=sprintf(buf,"%d",pInfo->hw.sOpts.nRxDescs);
        break;

    case CONF_TX_DESC:
        len=sprintf(buf,"%d",pInfo->hw.sOpts.nTxDescs);
        break;
    case CONF_TX_THRESH:
        len=sprintf(buf,"%d",pInfo->hw.sOpts.tx_thresh);
        break;
    case CONF_RX_THRESH:
        len=sprintf(buf,"%d",pInfo->hw.sOpts.rx_thresh);
        break;
    case CONF_SPD_DPX: {
        int mii_status=rhine_check_media_mode(&pInfo->hw);
        int r=0;
        if (mii_status & RHINE_AUTONEG_ENABLE)
            r=0;
        else if ((mii_status & (RHINE_SPEED_100|RHINE_DUPLEX_FULL))
                ==(RHINE_SPEED_100|RHINE_DUPLEX_FULL))
            r=2;
        else if ((mii_status & (RHINE_SPEED_10|RHINE_DUPLEX_FULL))
                ==(RHINE_SPEED_10|RHINE_DUPLEX_FULL))
            r=4;
        else if (mii_status & (RHINE_SPEED_100))
            r=1;
        else if (mii_status & (RHINE_SPEED_10))
            r=3;
        len=sprintf(buf,"%d",r);
        }
        break;
    case CONF_DMA_LEN:
        len=sprintf(buf,"%d",pInfo->hw.sOpts.DMA_length);
        break;
    case CONF_WOL_OPTS:
        len=sprintf(buf,"%d",pInfo->hw.sOpts.wol_opts);
        break;
    case CONF_FLOW_CTRL:
        len=sprintf(buf,"%d",pInfo->hw.sOpts.flow_cntl);
        break;
    case CONF_VAL_PKT:
        len=sprintf(buf,"%d",
            (pInfo->hw.flags & RHINE_FLAGS_VAL_PKT_LEN) ? 1 : 0);
        break;
    case CONF_ENABLE_TAG:
        len=sprintf(buf,"%d",
            (pInfo->hw.flags & RHINE_FLAGS_TAGGING) ? 1 : 0);
        break;
    case CONF_VID_SETTING:
        len=sprintf(buf,"%d",pInfo->hw.sOpts.vid);
        break;
    }
    return len;
}

static int FunConfWrite(PRHINE_PROC_ENTRY pEntry, const char* buf,
    unsigned long len) {
    PRHINE_INFO pInfo=pEntry->pInfo;
    //PMAC_REGS   pMacRegs=pInfo->pMacRegs;
    PROC_CONF_TYPE  op=pEntry->data;
    long l;

    l=atol(buf,len);
    if (l<0)
        return -EINVAL;

    switch(op) {
    case CONF_RX_DESC:
        if (pInfo->hw.flags & RHINE_FLAGS_OPENED)
            return -EACCES;
        if ((l<16)||(l>128))
            return -EINVAL;
        pInfo->hw.sOpts.nRxDescs=l;
        break;
    case CONF_TX_DESC:
        if (pInfo->hw.flags & RHINE_FLAGS_OPENED)
            return -EACCES;
        if ((l<16)||(l>128))
            return -EINVAL;
        pInfo->hw.sOpts.nTxDescs=l;
        break;

    case CONF_TX_THRESH:
        if ((l<0)||(l>4))
            return -EINVAL;
        pInfo->hw.sOpts.tx_thresh=l;
        rhine_set_tx_thresh(&pInfo->hw,pInfo->hw.sOpts.tx_thresh);
        break;
    case CONF_RX_THRESH:
        if ((l<0)||(l>7))
            return -EINVAL;
        pInfo->hw.sOpts.rx_thresh=l;
        rhine_set_rx_thresh(&pInfo->hw,pInfo->hw.sOpts.rx_thresh);
        break;

    case CONF_SPD_DPX: {
        U32 new_status=0;
        if (BYTE_REG_BITS_IS_ON(&pInfo->hw, CFGC_MEDEN, MAC_REG_CFGC)) {
            printk(KERN_INFO
                //"%s: The media mode have been forced by EEPROM utiltiy\n",pInfo->dev->name);
                "The media mode have been forced by EEPROM utiltiy\n");
            return -EPERM;
        }

        switch(l) {
        case 0:
            new_status=RHINE_AUTONEG_ENABLE;
            break;
        case 1:
            new_status=RHINE_SPEED_100;
            break;
        case 2:
            new_status=RHINE_SPEED_100|RHINE_DUPLEX_FULL;
            break;
        case 3:
            new_status=RHINE_SPEED_10;
            break;
        case 4:
            new_status=RHINE_SPEED_10|RHINE_DUPLEX_FULL;
            break;
        }

        rhine_set_media_mode(&pInfo->hw, &pInfo->hw.sOpts);
        }
        break;

    case CONF_DMA_LEN:
        if ((l<0)||(l>7))
            return -EINVAL;
        pInfo->hw.sOpts.DMA_length=l;
        rhine_set_DMA_length(&pInfo->hw,pInfo->hw.sOpts.DMA_length);
        break;

    case CONF_WOL_OPTS:
        if ((l<0)||(l>16))
            return -EINVAL;
        pInfo->hw.sOpts.wol_opts=l;
        if (l==0)
            pInfo->hw.flags &=(~RHINE_FLAGS_WOL_ENABLED);
        else
            pInfo->hw.flags |=RHINE_FLAGS_WOL_ENABLED;
        break;

    case CONF_FLOW_CTRL:
        if (pInfo->hw.flags & RHINE_FLAGS_OPENED)
            return -EACCES;
        if ((l<1)||(l>3))
            return -EINVAL;
        pInfo->hw.sOpts.flow_cntl=l;
        break;

    case CONF_VAL_PKT:
        if (l==0) {
            pInfo->hw.flags &=~RHINE_FLAGS_VAL_PKT_LEN;
            pInfo->hw.sOpts.flags &=~RHINE_FLAGS_VAL_PKT_LEN;
        }
        else if (l==1) {
            pInfo->hw.flags |=RHINE_FLAGS_VAL_PKT_LEN;
            pInfo->hw.sOpts.flags |=RHINE_FLAGS_VAL_PKT_LEN;
        }
        else
            return -EINVAL;
        break;
    case CONF_ENABLE_TAG:
        if (l==0) {
            pInfo->hw.flags &=~RHINE_FLAGS_TAGGING;
            pInfo->hw.sOpts.flags &=~RHINE_FLAGS_TAGGING;
        }
        else if (l==1) {
            pInfo->hw.flags |=RHINE_FLAGS_TAGGING;
            pInfo->hw.sOpts.flags |=RHINE_FLAGS_TAGGING;
        }
        else
            return -EINVAL;
        break;
    case CONF_VID_SETTING:
        if ((l<0) || (l>4094))
            return -EINVAL;
        pInfo->hw.sOpts.vid=l;
        pInfo->hw.flags |=RHINE_FLAGS_TAGGING;
        pInfo->hw.sOpts.flags |=RHINE_FLAGS_TAGGING;
        if (pInfo->hw.byRevId>=REV_ID_VT6105M_A0)
            rhine_init_cam_filter(pInfo);

        break;
    }
    return 0;
}

static void rhine_UpdateRMONStats(PRHINE_INFO pInfo) {
    if (pInfo->hw.byRevId>=REV_ID_VT6105M_A0) {
        spin_lock_irq(&pInfo->lock);
        rhine_UpdateMIBCounter(pInfo);
        spin_unlock_irq(&pInfo->lock);
        pInfo->adwRMONStats[RMON_Collisions]=pInfo->stats.collisions;
        pInfo->adwRMONStats[RMON_CRCAlignErrors]=
            pInfo->adwHWMIBCounters[HW_MIB_RxFCSErrs]
            +pInfo->adwHWMIBCounters[HW_MIB_RxFAErrs];
        pInfo->adwRMONStats[RMON_DropEvents]=
            pInfo->adwHWMIBCounters[HW_MIB_RxNoBuf]
            +pInfo->adwHWMIBCounters[HW_MIB_RxMSDPktErrs];
    }
}

static int FunRMONRead(PRHINE_PROC_ENTRY pEntry,char* buf) {
    PRHINE_INFO     pInfo   =pEntry->pInfo;
    PROC_RMON_TYPE  op      =pEntry->data;
    int len=0;

    if (op<0 || op>RMON_TAB_SIZE)
        return -EINVAL;

    rhine_UpdateRMONStats(pInfo);
    len=sprintf(buf,"%d",pInfo->adwRMONStats[op]);
    return len;
}
