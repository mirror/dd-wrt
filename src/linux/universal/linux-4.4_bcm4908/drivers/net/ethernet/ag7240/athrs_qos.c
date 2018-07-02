#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include "athrs_qos.h"

int athrs_config_qos(struct eth_cfg_params *ethcfg, int cmd)
{
    struct ath_qops *qmac;
    struct net_device *dev;

    if(cmd != S26_QOS_CTL)
        return -EINVAL;

    cmd = ethcfg->cmd;
    qmac = get_qmac(ethcfg->ad_name);
    dev = get_ndev(ethcfg->ad_name);
    if (((cmd != ETH_SOFT_CLASS) && (qmac->qos_cap == 0))
         && !(cmd == ETH_PORT_ILIMIT
         || cmd == ETH_PORT_ELIMIT
         || cmd == ETH_PORT_EQLIMIT)) {

        printk("QOS capability not enabled\n");
        return -EINVAL;
    }

    netif_carrier_off(dev);
    netif_stop_queue(dev);
    printk(KERN_INFO "Qos Config %x,%x,%s,%x,%x,%x\n",cmd,ethcfg->val,ethcfg->ad_name,ethcfg->portnum,
		ethcfg->vlanid,ethcfg->mac_addr[0]);
    switch(cmd){
        case ETH_SOFT_CLASS:
            if( (ethcfg->val > 0) && (qmac->qos_cap == 0) ) {
                qmac->enable_qos(ethcfg->ad_name);
            }else if((ethcfg->val == 0) && (qmac->qos_cap != 0)){
                qmac->disable_qos(ethcfg->ad_name);
            }         
            break;
        case ETH_PORT_QOS:
            if(ethcfg->val >= 0 && ethcfg->val < 4 ) {
                qmac->reg_rmw_clear(ethcfg->ad_name,ATHR_PRI_CTRL_PORT_0 + (ethcfg->portnum * 256),
                      (ATHR_DA_PRI_EN|ATHR_TOS_PRI_EN|ATHR_VLAN_PRI_EN));
       
                qmac->reg_rmw_set(ethcfg->ad_name,ATHR_PRI_CTRL_PORT_0 + 
                      (ethcfg->portnum * 256),ATHR_PORT_PRI_EN);
       
                qmac->reg_rmw_clear(ethcfg->ad_name,PORT_BASE_VLAN_REGISTER0 + 
                      (ethcfg->portnum * 256),(0x3 << 28));

                qmac->reg_rmw_set(ethcfg->ad_name,PORT_BASE_VLAN_REGISTER0 + 
                      (ethcfg->portnum * 256),((ethcfg->val & 0xf) << 28));
            }
            break;
        case ETH_VLAN_QOS:
            if (ethcfg->val >= 0 && ethcfg->val < 8) {
                int portmap = 0;
                int i;
                python_ioctl_vlan_qos(ethcfg->vlanid, ethcfg->val, &portmap);
                for(i=1; i<5; i++){
                    if((portmap & (0x1 << i)) != 0){
                    qmac->reg_rmw_clear(ethcfg->ad_name,ATHR_PRI_CTRL_PORT_0 + (i * 256),
                         (ATHR_DA_PRI_EN|ATHR_TOS_PRI_EN|ATHR_PORT_PRI_EN));

                    qmac->reg_rmw_set(ethcfg->ad_name,ATHR_PRI_CTRL_PORT_0 + 
                         (i * 256),ATHR_VLAN_PRI_EN);
                    }
                }
            }
            break;
        case ETH_IP_QOS:
            {
                int i;
                for(i=1; i<5; i++){
                    qmac->reg_rmw_clear(ethcfg->ad_name,ATHR_PRI_CTRL_PORT_0 + (i * 256),
                         (ATHR_DA_PRI_EN|ATHR_VLAN_PRI_EN|ATHR_PORT_PRI_EN));

                    qmac->reg_rmw_set(ethcfg->ad_name,ATHR_PRI_CTRL_PORT_0 +
                         (i * 256),ATHR_TOS_PRI_EN);
                }
                if(ethcfg->tos != 0 || ethcfg->val != 0){
                    qmac->reg_rmw_clear(ethcfg->ad_name, ATHR_IP_PRI_MAP0 + (((ethcfg->tos >> 1)/32)<<2),
                         0x3 << ((ethcfg->tos%64) >> 1));

                    qmac->reg_rmw_set(ethcfg->ad_name, ATHR_IP_PRI_MAP0 + (((ethcfg->tos >> 1)/32)<<2),
                         ethcfg->val << ((ethcfg->tos%64) >> 1));
                }
            }
            break;
        case ETH_DA_QOS:
            if (ethcfg->val >= 0 && ethcfg->val < 4) {
                int i=0;
                for(i=1; i<5; i++){
                    qmac->reg_rmw_clear(ethcfg->ad_name,ATHR_PRI_CTRL_PORT_0 + (i * 256),
                        (ATHR_TOS_PRI_EN|ATHR_VLAN_PRI_EN|ATHR_PORT_PRI_EN));
                    qmac->reg_rmw_set(ethcfg->ad_name,ATHR_PRI_CTRL_PORT_0 + 
                         (i * 256),ATHR_DA_PRI_EN);
                }
                python_fdb_add_qos(ethcfg->mac_addr, (ethcfg->val & 0x3),(ethcfg->portnum));
            }
            break;
        case ETH_PORT_ILIMIT:
			{
                int val;
                val = ethcfg->val & 0x7fff;
                qmac->reg_rmw_clear(ethcfg->ad_name, RATE_LIMIT_REGISTER0 + 
                      (ethcfg->portnum * 256), 0x7fff);
                qmac->reg_rmw_set(ethcfg->ad_name, RATE_LIMIT_REGISTER0 +
                      (ethcfg->portnum * 256), val);
			}
            break;
        case ETH_PORT_ELIMIT:
            {
                int val;

                qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT1_REGISTER0 + 
                      (ethcfg->portnum * 256), 0x7fff7fff);
                qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT1_REGISTER0 + 
                      (ethcfg->portnum * 256), 0x7fff7fff);
                qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 + 
                      (ethcfg->portnum * 256), 0x7fff7fff);
                qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 + 
                      (ethcfg->portnum * 256), 0x7fff7fff);

                val = ethcfg->val & 0x7fff;
                if(val != 0x7fff){
                    qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT_REGISTER0 +
                          (ethcfg->portnum * 256), (0x1 << 23));

                    qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 +
                          (ethcfg->portnum * 256), 0x7fff<<16);
                    qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 +
                          (ethcfg->portnum * 256), val<<16);
                }else{
                    qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT_REGISTER0 +
                          (ethcfg->portnum * 256), (0x1 << 23));
 
                    qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 + 
                          (ethcfg->portnum * 256), 0x7fff<<16);
                    qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 +
                          (ethcfg->portnum * 256), val<<16);
                }
            }
            break;	
        case ETH_PORT_EQLIMIT:
            {
                int val;
                int queue_id;
                val = ethcfg->val & 0x7fff;
                queue_id = ethcfg->phy_reg;

                qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT_REGISTER0 +
                      (ethcfg->portnum * 256), (0x1 << 23));
                qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 +
                      (ethcfg->portnum * 256), 0x7fff<<16);
                qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 +
                      (ethcfg->portnum * 256), 0x7fff);

                if (0 == queue_id) {
                    qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT1_REGISTER0 +
                          (ethcfg->portnum * 256), 0x7fff);
                    qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT1_REGISTER0 +
                          (ethcfg->portnum * 256), val);
                } else if (1 == queue_id) {
                    qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT1_REGISTER0 +
                          (ethcfg->portnum * 256), 0x7fff<<16);
                    qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT1_REGISTER0 +
                          (ethcfg->portnum * 256), val<<16);
                } else if (2 == queue_id) {
                    qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 +
                          (ethcfg->portnum * 256), 0x7fff);
                    qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 +
                          (ethcfg->portnum * 256), val);
                } else if (3 == queue_id) {
                    qmac->reg_rmw_clear(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 + 
                          (ethcfg->portnum * 256), 0x7fff<<16);
                    qmac->reg_rmw_set(ethcfg->ad_name,RATE_LIMIT2_REGISTER0 +
                          (ethcfg->portnum * 256), val<<16);
                }			
            }
            break;
        default:
            break;
    }
    
    if(!netif_carrier_ok(dev)) {
        netif_carrier_on(dev);
        netif_start_queue(dev);
    }
    return 0;
}

int athr_register_qos(void *mac)
{
    set_mac_qops(mac);
    return 0;
}   
