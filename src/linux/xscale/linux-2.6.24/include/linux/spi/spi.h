/* 
 *  linux/include/linux/spi/spi.h 
 * 
 *  Copyright (C) 2001 Russell King, All Rights Reserved. 
 *  Copyright (C) 2002 Compaq Computer Corporation, All Rights Reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License. 
 * 
 * Derived from l3.h by Jamey Hicks 
 */ 
#ifndef _LINUX_SPI_H 
#define _LINUX_SPI_H
 
#include <linux/module.h>
#include <linux/types.h>
#include <linux/spi/spi-id.h>
#include <linux/device.h>       /* for struct device */
#include <asm/semaphore.h>

/* --- General options ------------------------------------------------ */

struct spi_msg;
struct spi_algorithm;
struct spi_adapter;
struct spi_client;
struct spi_driver;
struct spi_client_address_data;

/*
 * SPI Message - used for pure spi transaction, also from /dev interface
 */
#define SPI_M_WR        	0x0000
#define SPI_M_RD        	0x0001
#define SPI_M_NOSTART   	0x4000
#define SPI_M_REV_DIR_ADDR      0x2000
 
struct spi_msg {
    __u16 addr;     /* slave address                        */
    __u16 flags;
    __u16 len;              /* msg length                           */
    __u8 *buf;              /* pointer to msg data                  */
};
						  
/* To determine what functionality is present */
						  
#define SPI_FUNC_SPI                  0x00000001

/* ----- commands for the ioctl like spi_command call:
 * note that additional calls are defined in the algorithm and hw
 *      dependent layers - these can be listed here, or see the
 *      corresponding header files.
 */
/* -> bit-adapter specific ioctls       */
#define SPI_RETRIES     0x0701  /* number of times a device address     */
                                /* should be polled when not            */
                                /* acknowledging                        */
#define SPI_TIMEOUT     0x0702  /* set timeout - call with int          */

/*
 * The master routines are the ones normally used to transmit data to devices
 * on a bus (or read from them). Apart from two basic transfer functions to
 * transmit one message at a time, a more complex version can be used to
 * transmit an arbitrary number of messages without interruption.
 */
extern int spi_master_send(struct spi_client *,const char* ,int);
extern int spi_master_recv(struct spi_client *,char* ,int);
     
/* Transfer num messages.
 */
extern int spi_transfer(struct spi_adapter *adap, struct spi_msg *msgs, int num);
      
/*
 * Some adapter types may support slave behaviuor.
 * This is not tested/implemented yet and will change in the future.
 */
extern int spi_slave_send(struct spi_client *,char*,int);
extern int spi_slave_recv(struct spi_client *,char*,int);
	 


#define SPI_MAJOR      98 
 
extern struct bus_type spi_bus_type; 
 
/* 
 * A driver is capable of handling one or more physical devices present on 
 * SPI adapters. 
 */ 
 
struct spi_ops { 
       int (*open) (struct spi_driver *); 
       int (*command) (struct spi_driver *, int cmd, void *arg); 
       void (*close) (struct spi_driver *); 
}; 
 
typedef char *spi_ids_t[]; 
#define SPI_ID_ANY     "* ANY *" 
#define SPI_IDS_TABLE_BEGIN static spi_ids_t spi_devices_supported = { 
#define SPI_IDS_TABLE_END ,NULL }; 
#define SPI_IDS &spi_devices_supported 
 

/*
 * A driver is capable of handling one or more physical devices present on
 * SPI adapters. This information is used to inform the driver of adapter
 * events.
*/

struct spi_driver {
        struct module *owner;
	char name[32];
	int id;
	unsigned int class;
	unsigned int flags;             /* div., see below              */

	/* Notifies the driver that a new bus has appeared. This routine
	 * can be used by the driver to test if the bus meets its conditions
	 * & seek for the presence of the chip(s) it supports. If found, it
	 * registers the client(s) that are on the bus to the spi admin. via
	 * spi_attach_client.
	 */
	int (*attach_adapter)(struct spi_adapter *);
	int (*detach_adapter)(struct spi_adapter *);

	/* tells the driver that a client is about to be deleted & gives it
	 * the chance to remove its private data. Also, if the client struct
	 * has been dynamically allocated by the driver in the function above,
	 * it must be freed here.
	 */
	int (*detach_client)(struct spi_client *);

	/* a ioctl like command that can be used to perform specific functions
	 * with the device.
	 */
	int (*command)(struct spi_client *client,unsigned int cmd, void *arg);
	
	struct device_driver driver;
	struct list_head list;
};
#define to_spi_driver(d) container_of(d, struct spi_driver, driver)

#define SPI_NAME_SIZE   50

/*
 * spi_client identifies a single device (i.e. chip) that is connected to an
 * spi bus. The behaviour is defined by the routines of the driver. This
 * function is mainly used for lookup & other admin. functions.
 */
struct spi_client {
        unsigned int flags;             /* div., see below              */
	unsigned int addr;              /* chip address - NOTE: 7bit    */
	                                /* addresses are stored in the  */
                                        /* _LOWER_ 7 bits of this char  */
        struct spi_adapter *adapter;    /* the adapter we sit on        */
        struct spi_driver *driver;      /* and our access routines      */
        int usage_count;                /* How many accesses currently  */
                                        /* to the client                */
        struct device dev;              /* the device structure         */
        struct list_head list;
        char name[SPI_NAME_SIZE];
        struct completion released;
};
#define to_spi_client(d) container_of(d, struct spi_client, dev)

static inline void *spi_get_clientdata (struct spi_client *dev)
{
    return dev_get_drvdata (&dev->dev);
}

static inline void spi_set_clientdata (struct spi_client *dev, void *data)
{
    dev_set_drvdata (&dev->dev, data);
}

#define SPI_DEVNAME(str)        .name = str

static inline char *spi_clientname(struct spi_client *c)
{
    return &c->name[0];
}

																										       
#define SELECT         0x01 
#define UNSELECT       0x02 
#define BEFORE_READ    0x03 
#define BEFORE_WRITE   0x04 
#define AFTER_READ     0x05 
#define AFTER_WRITE    0x06 
 
/*
 * The following structs are for those who like to implement new bus drivers:
 * spi_algorithm is the interface to a class of hardware solutions which can
 * be addressed using the same bus algorithms - i.e. bit-banging or the IXP465
 * to name two of the most common.
 */
struct spi_algorithm {
	char name[32];                          /* textual description  */
	unsigned int id;
		     
        /* If an adapter algorithm can't do SPI-level access, set master_xfer
	   to NULL. */
	int (*master_xfer)(struct spi_adapter *adap, struct spi_msg *msgs,
			   int num);
		       
	/* --- these optional/future use for some adapter types.*/
	int (*slave_send)(struct spi_adapter *,char*,int);
	int (*slave_recv)(struct spi_adapter *,char*,int);

	/* --- ioctl like call to set div. parameters. */
	int (*algo_control)(struct spi_adapter *, unsigned int, unsigned long);

	/* To determine what the adapter supports */
	u32 (*functionality) (struct spi_adapter *);
};
																										        
/* 
 * spi_adapter is the structure used to identify a physical SPI device along 
 * with the access algorithms necessary to access it. 
 */ 
struct spi_adapter { 
        struct module *owner;
	unsigned int id;/* == is algo->id | hwdep.struct->id,           */
		        /* for registered values see below              */
	unsigned int class;
	struct spi_algorithm *algo;/* the algorithm to access the bus   */
	void *algo_data;

	/* --- administration stuff. */
	int (*client_register)(struct spi_client *);
	int (*client_unregister)(struct spi_client *);

        /* data fields that are valid for all devices   */
	struct semaphore bus_lock;
	struct semaphore clist_lock;

	int timeout;
	int retries;
	struct device dev;              /* the adapter device */
	struct class_device class_dev;  /* the class device */

#ifdef CONFIG_PROC_FS
	/* No need to set this when you initialize the adapter          */
	int inode;
#endif /* def CONFIG_PROC_FS */

        int nr;
        struct list_head clients;
        struct list_head list;
        char name[SPI_NAME_SIZE];
        struct completion dev_released;
        struct completion class_dev_released;
};

#define dev_to_spi_adapter(d) container_of(d, struct spi_adapter, dev)
#define class_dev_to_spi_adapter(d) container_of(d, struct spi_adapter, class_dev)

static inline void *spi_get_adapdata (struct spi_adapter *dev)
{
    return dev_get_drvdata (&dev->dev);
}
	
static inline void spi_set_adapdata (struct spi_adapter *dev, void *data)
{
    dev_set_drvdata (&dev->dev, data);
}
		

/*flags for the driver struct: */
#define SPI_DF_NOTIFY   0x01            /* notify on bus (de/a)ttaches  */

/*flags for the client struct: */
#define SPI_CLIENT_ALLOW_USE            0x01    /* Client allows access */
#define SPI_CLIENT_ALLOW_MULTIPLE_USE   0x02    /* Allow multiple access-locks */
 
/* ----- functions exported by spi.o */

/* administration...
 */
extern int spi_add_adapter(struct spi_adapter *);
extern int spi_del_adapter(struct spi_adapter *);
 
extern int spi_add_driver(struct spi_driver *);
extern int spi_del_driver(struct spi_driver *);
 
extern int spi_attach_client(struct spi_client *);
extern int spi_detach_client(struct spi_client *);
 
extern int spi_control(struct spi_client *,unsigned int, unsigned long);

extern int spi_adapter_id(struct spi_adapter *adap);
extern struct spi_adapter* spi_get_adapter(int id);
extern void spi_put_adapter(struct spi_adapter *adap);
		 
extern int spi_transfer(struct spi_adapter *, struct spi_msg msgs[], int); 
extern int spi_write(struct spi_adapter *, int, const char *, int); 
extern int spi_read(struct spi_adapter *, int, char *, int); 

/* Return the functionality mask */
static inline u32 spi_get_functionality(struct spi_adapter *adap)
{
    return adap->algo->functionality(adap);
}
	
/* Return 1 if adapter supports everything we need, 0 if not. */
static inline int spi_check_functionality(struct spi_adapter *adap, u32 func)
{
    return (func & spi_get_functionality(adap)) == func;
}
		
#endif                         /* _LINUX_SPI_H */ 
