#ifndef KVCTL_H
#define KVCTL_H

#define VCTL_HEAD_SIZE	8
#define VCTL_ENTRY_LEN	20

typedef struct
{
  char header[4];
  unsigned int entry_num;
} vctl_mheader;

typedef struct
{
  char header[4];
  unsigned int size;
  unsigned int type;
  char majorver[4];
  char minorver[4];
  unsigned char *payload;
} vctl_entry;

typedef struct
{
  unsigned char mac[6];
  unsigned char vlanid;
  unsigned char vlanmap;
} vlaninfo;

#define VCT_VENDORSPEC		0
#define VCT_BOOTLOADER		1
#define VCT_KERNEL		2
#define VCT_VERCTL		3
#define VCT_CURRCONF		4
#define VCT_DEFAULTCONF		5
#define VCT_ROOTFS		6
#define VCT_APP			7
#define VCT_VLAN		8

#endif
