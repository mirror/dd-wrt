typedef struct {
	unsigned char info[128];   /* Informative text string */
	unsigned char spare0[14];
	struct sun_info {
		unsigned char spare1;
		unsigned char id;
		unsigned char spare2;
		unsigned char flags;
	} infos[8];
	unsigned char spare1[246]; /* Boot information etc. */
	unsigned short rspeed;     /* Disk rotational speed */
	unsigned short pcylcount;  /* Physical cylinder count */
	unsigned short sparecyl;   /* extra sects per cylinder */
	unsigned char spare2[4];   /* More magic... */
	unsigned short ilfact;     /* Interleave factor */
	unsigned short ncyl;       /* Data cylinder count */
	unsigned short nacyl;      /* Alt. cylinder count */
	unsigned short ntrks;      /* Tracks per cylinder */
	unsigned short nsect;      /* Sectors per track */
	unsigned char spare3[4];   /* Even more magic... */
	struct sun_partition {
		u_int32_t start_cylinder;
		u_int32_t num_sectors;
	} partitions[8];
	unsigned short magic;      /* Magic number */
	unsigned short csum;       /* Label xor'd checksum */
} sun_partition;

#define SUN_LABEL_MAGIC          0xDABE
#define SUN_LABEL_MAGIC_SWAPPED  0xBEDA
#define sunlabel(x) ((sun_partition *)x)

typedef struct {
	unsigned int   magic;        /* expect AIX_LABEL_MAGIC */
	/* ... */
} aix_partition;

#define	AIX_LABEL_MAGIC		0xc9c2d4c1
#define	AIX_LABEL_MAGIC_SWAPPED	0xc1d4c2c9
#define aixlabel(x) ((aix_partition *)x)

typedef struct {
	unsigned short magic;
	/* ... */
} mac_partition;

#define MAC_LABEL_MAGIC		0x4552
#define MAC_PARTITION_MAGIC	0x504d
#define MAC_OLD_PARTITION_MAGIC	0x5453
#define maclabel(x) ((mac_partition *)x)
