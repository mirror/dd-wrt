// types
//typedef unsigned int uint32;

// consts
static volatile unsigned char *gpioaddr_input = NULL;
static volatile unsigned char *gpioaddr_output = NULL;
static volatile unsigned char *gpioaddr_enable = NULL;

// function prototypes
static int mmc_open(struct inode *inode, struct file *filp);
static int mmc_release(struct inode *inode, struct file *filp);
static int mmc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#if 0
static int mmc_check_media_change(kdev_t dev);
static int mmc_revalidate(kdev_t dev);
#endif

// inline functs
#define GET_RESULT_DO(shift_do) result |= (  ((shift_do)>=0)?( ((*l_gpioaddr_input) & SD_DO) << (shift_do) ):( ((*l_gpioaddr_input) & SD_DO) >> (-(shift_do)) )  );
#define SET_DI(bit_di,shift_di) di = (  ((shift_di)>=0)?( (data_out & bit_di) >> (shift_di) ):( (data_out & bit_di) << (-(shift_di)) )  );

// module global params
static int mp_max_init_tries = 30000;
module_param(mp_max_init_tries, int, 0);
MODULE_PARM_DESC(mp_max_init_tries, "Number of max CMD0 sent in card init loop.");
static int mp_max_pwr_on_clocks = 80;
module_param(mp_max_pwr_on_clocks, int, 0);
MODULE_PARM_DESC(mp_max_pwr_on_clocks, "Number of max clocks sent to power on card.");

// global vars
/* we have only one device */
static unsigned int hd_sizes[64];
static unsigned int hd_blocksizes[64];
static unsigned int hd_hardsectsizes[64];
static unsigned int hd_maxsect[64];
static struct hd_struct hd[64];
static int mmc_media_detect = 0;
static int mmc_media_changed = 1;
static unsigned char port_state = 0x00;
static unsigned char ps_di, ps_di_clk, ps_clk;
const unsigned char NOT_DI_NOT_CLK = (~SD_DI) & (~SD_CLK);
const unsigned char DI_CLK = SD_DI | SD_CLK;

// half period in CPU cycles of a frequency of 380KHz
// it is used to limit CLK frequency during MMC init phase
// is calibrated to the current CPU during module startup
static cycles_t hfp_380khz = 0;
// time reference of the last clock sent (only usefull if last clock was sent
// lower than 21,47 sec ago at 200MHz (cycles_t is a 32bit integer)
static cycles_t last_clk = 0;

// global structs
static struct block_device_operations mmc_bdops =  {
	open: mmc_open,
	release: mmc_release,
	ioctl: mmc_ioctl,
#if 0
	check_media_change: mmc_check_media_change,
	revalidate: mmc_revalidate,
#endif
};

static struct gendisk hd_gendisk = {
	major:		MAJOR_NR,
	major_name:	DEVICE_NAME,
	minor_shift:	6,
	max_p:		64,
	part:		hd,
	sizes:		hd_sizes,
	fops:		&mmc_bdops,
};

