/*
 *       adm6996.c  -- ADM6996L linux interface driver.
 *	Copyright (c) 2004 Nikki Chumakov (nikki@gattaca.ru)
 */
#define ADMDRIVER
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>

#include <typedefs.h>
#include <sbconfig.h>
#include <sbpci.h>
#include <sbutils.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <bcmenet47xx.h>

#include "adm6996.h"

static int verbose = 0;

static struct adm_dev_t {
	void    *sbh;   /* sb utils handle */
	char*				vars;
	int					vars_size;
	bcmenetregs_t regsva;
	adm_info_t* info;
} adm_dev ;

#ifdef MODULE
static int debug = 0;

MODULE_AUTHOR("Nikki Chumakov");
MODULE_DESCRIPTION("ADM6996L Interface Driver");
MODULE_LICENSE("GPL");
MODULE_PARM(debug, "i");

EXPORT_NO_SYMBOLS;
#endif // MODULE

int adm6996_init(void);
#ifdef MODULE
int init_module(void);
void cleanup_module(void);
int adm6996_cleanup (void);
#endif

#ifdef MODULE
int init_module (void)
{
	int ret;

	if (verbose) 
		printk ("Loading module adm6996 ...\n");
	ret = adm6996_init ();
	if (verbose)
		printk ("Done\n");
	return (ret);
}

void cleanup_module (void)
{
	if (verbose)
		printk ("Unloading module adm6996 ... ");

	(void) adm6996_cleanup ();

	if (verbose)
		printk ("done\n");
}
int adm6996_cleanup (void)
{
#ifdef CONFIG_PROC_FS
	cleanup_sysctl ();
  adm6996_unregister_procfs (0);
#endif

	
	adm_detach (adm_dev.info);		
	sb_detach(adm_dev.sbh);
	return (0);
}

#endif

adm_info_t* einfo;
int adm6996_init (void)
{
	int err = 0;

	printk(KERN_INFO "ADM6996L Driver version %s\n", ADM6996_VERSION);
		
	einfo = adm_dev.info = adm_attach (sbh);
#ifdef CONFIG_PROC_FS
  err = adm6996_register_procfs (adm_dev.info);
	if (err)
		return (err);
	init_sysctl ();
#endif
			
	return (0);
}


