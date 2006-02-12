#ifndef ADM6996_H
#define ADM6996_H
#include <typedefs.h>

#define ADM6996_VERSION "3.2 by Nikki Chumakov, 10-Jul-2004"
#define ADM6996_MINOR 99

extern void *bcm947xx_sbh;
#define sbh bcm947xx_sbh

/* Private state per ADM switch */
typedef struct {
	void *sbh;      /* SiliconBackplane handle */
	uint coreidx;     /* Current core index */
	uint8 eecs, eesk, eedi;   /* GPIO mapping */
} adm_info_t;

struct adm_port_stat
{
	int						port;
	unsigned int	link:1,
								speed:2,
							  duplex:1,
								flow:1,
								cable_broken:1,
							  cable_len:2;
								
	unsigned int  rxp_overflow:1, txp_overflow:1,
	              rxb_overflow:1, txb_overflow:1,
		      col_overflow:1, err_overflow:1;
	uint32				rx_packets;
	uint32				tx_packets;
	uint32				rx_bytes;
	uint32				tx_bytes;
	uint32				collisions;
	uint32				errors;
};

struct adm_port_config
{
	unsigned int flow:1,
	             autoneg:1,
							 speed:1,
							 duplex:1,
							 tagging:1,
							 disabled:1,
							 tos:1,
							 use_prio:1,
							 port_prio:2,
							 pvid_0_3:4,
							 fx:1,
							 crossover:1;
	unsigned int mac_lock:1,
							 thrs_ena:1,
	             threshold:3,
							 count_recv:1
							;
	unsigned int pvid_4_11:8;
};

struct adm_gen_config
{
	struct adm_port_config port[6];
	unsigned int drop_on_collisions:1,
							 replace_vid_pvid:1,
					     ipg92:1, trunk:1, far_end_fault:1,
							 storming:1, storm_threshold:2,
							 xcrc:1, aging:1,
							 reg_10_bit_3:1,
							 polarity_error:1,
							 drop_q0:2, drop_q1:2, drop_q2:2, drop_q3:2,
							 /* 0x30 reg */
							 mac_clone_30:1, mii_speed:1, speed_led:1, port_led:1,
							 /* 0x11 reg */
							 mac_clone_11:1, vlan_mode:1,
							 /* 2ch reg */
							 tag_shift:3,
							 reg_2c_bit_11:1,
							 fwd_management_mac1:1,
							 fwd_management_mac2:1,
							 fwd_management_mac3:1,
							 fwd_management_mac4:1,
							 smart_squelch:1
							;
	unsigned int vlan_prio:16, tos_prio:16;
	unsigned short vlan_groups[16];
	
};
	
/* Forward declarations */
adm_info_t *adm_attach(void *sbh); 
void adm_detach(adm_info_t *adm);
void adm_enable_device(adm_info_t *adm, char *vars);
int adm_config_vlan(adm_info_t *adm, char *vars);

int adm6996_register_procfs (adm_info_t *info);
void adm6996_unregister_procfs (adm_info_t *info);

void adm_rreg(adm_info_t *adm, uint8 domain, uint8 addr, uint32* val);
void adm_wreg(adm_info_t *adm, uint8 addr, uint16 val);
void adm_get_ports_stats (int, struct adm_port_stat[]);
void adm_get_gen_config (struct adm_gen_config*);
	
extern adm_info_t* einfo;


#define ADM6996_PFX "adm6996: "
                                                                                
#define PRINTK(format, args...)            \
  printk (ADM6996_PFX format, ## args)
	
#define PRINTK_ERR(format, args...)          \
	  printk (ADM6996_PFX format, ## args)
	
#define PRINTK_INFO(format, args...)           \
	  printk (ADM6996_PFX format, ## args)
	
#define DPRINTK(format, args...) if (debug >= 1)       \
	  printk (ADM6996_PFX format, ## args)
	
#define DPRINTK2(format, args...) if (debug >= 2)      \
	  printk (ADM6996_PFX format, ## args)



							  
#define ADM_IOC_BASE 'a'

#define ADM_GET_CHIPID				_IOR(ADM_IOC_BASE, 0, uint32)
#define ADM_GET_PORTS_NUM			_IOR(ADM_IOC_BASE, 1, int)

#define ADM_GET_PORTS					_IOR(ADM_IOC_BASE, 2, struct adm_port_stat[6])

#endif
