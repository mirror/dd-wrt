#include "rtl_phy.h"
#include "rtl8368s_asicdrv.h"
//#include "ag7100_phy.h"
#include "rtl8366rb_api.h"
#include "rtl8366rb_api_ext.h"
#include "rtl8366rb_api_compat.h"

#if 0
int32 rtl_vlanSetup()
{
	rtl8366rb_vlanConfig_t  vlan_cfg;
	
	/* initialize VLAN */
	if (SUCCESS != rtl8366rb_initVlan())
	{
		goto vlan_failed;
	}


	/* all the ports are in the default VLAN 1 after VLAN initialized, 
	 * modify it as follows 
	 * VLAN1 member: port0, port5;
	 * VLAN2 member: port1, port2, port3, port4, port5 
	 */
	//rtl8366s_setVlan(1, 0x21, 0x1f);
	vlan_cfg.vid      = 1;
	vlan_cfg.mbrmsk   = 0x21;
	vlan_cfg.untagmsk = 0x1F;
	vlan_cfg.fid      = 0;

	if (SUCCESS != rtl8366rb_setVlan(&vlan_cfg))
	{
		goto vlan_failed;
	}


	vlan_cfg.vid      = 2;
	vlan_cfg.mbrmsk   = 0x3E;
	vlan_cfg.untagmsk = 0x1F;
	vlan_cfg.fid      = 1;
		
	if (SUCCESS != rtl8366rb_setVlan(&vlan_cfg))
	{
		goto vlan_failed;
	}
	
	/* set PVID for each port */
	if (rtl8366rb_setVlanPVID(PORT0, 1, 0)
		|| rtl8366rb_setVlanPVID(PORT1, 2, 0)
		|| rtl8366rb_setVlanPVID(PORT2, 2, 0)
		|| rtl8366rb_setVlanPVID(PORT3, 2, 0)
		|| rtl8366rb_setVlanPVID(PORT4, 2, 0))
	{
		goto pvid_failed;
	}
	//rtl8366s_setVlanPVID(PORT5, 2, 0);

	return SUCCESS;
	
pvid_failed:
	printk ("set vlan pvid failed\n");
	return FAILED;
	
vlan_failed:
	printk ("set vlan failed\n");
	return FAILED;
	
}

int rtl_getLinkStatus()
{
	uint32 linkStatus;
	uint32 phyNo = 0;
	int linkMap = 0;
	
	for (phyNo = 0; phyNo < PHY_NUM_MAX; phyNo++)
	{
		rtl8366rb_getPHYLinkStatus(phyNo, &linkStatus);
		if (linkStatus)
		{
			linkMap |= (1 << phyNo);
		}
	}

	return linkMap;
	
}
#endif
/******************************************************************************
*
* rtl_phySetup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
rtl_phySetup(int ethUnit)
{
	int phyNo = 0;
	uint32 data = 0;	

//	smi_init();
//	udelay(1000*1000);

	/* read chip ID, it also initialize SMI through first sim_read called added by tiger */
	if (SUCCESS == smi_read(0x5C, &data))
	{
		printf ("chip ID is 0x%08x\n", data);
	}

	if (SUCCESS != rtl8366rb_initChip())
	{
		printf ("initChip failed\n");
		return FAILED;
	}	

#if 0
	rtl8366rb_macConfig_t macConfig;
	macConfig.speed = SPD_1000M;
	macConfig.duplex = FULL_DUPLEX;
	macConfig.link = 1;
	macConfig.txPause = 1;
	macConfig.rxPause = 1;
	macConfig.force = MAC_FORCE;
	
	if (SUCCESS != rtl8366rb_setMac5ForceLink(&macConfig)) 
	{
		printk ("set mac5 failed\n");
		return FAILED;
	}


	phyNo = rtl8368s_getAsicPHYRegs(0, 1, 22, &data);
	printk("rtl8368s_getAsicPHYRegs(0, 1, 22, &data) return is %d, data is 0x%08x\n", phyNo, data);
	
	udelay(100*1000);


	if (SUCCESS != rtl_vlanSetup())
	{
		return FAILED;
	}		


	if (SUCCESS != rtl8366rb_setGreenEthernet(1, 1))
	{
		printk ("+++ set green ethernet failed\n");
		return FAILED;
	}
	
	if (SUCCESS != rtl8368s_setAsicReg(0xC44C, 0x0085))
	{
		printk ("+++ set asic reg failed\n");
		return FAILED;
	}
#endif
	return SUCCESS;
		
}


