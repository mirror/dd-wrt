/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/vpd/V4/vpdcheck.c#4 $
 * Project:	Gigabit Ethernet Adapters, Diagnostic Tool
 * Version:	$Revision: #4 $, $Change: 4281 $
 * Date:	$DateTime: 2010/11/05 11:58:07 $
 * Purpose:	VPD content structure check
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef SK_DIAG
#include "h/skdrv1st.h"
#include "h/lm80.h"
#include "h/skdrv2nd.h"
#else
#define w_print	printf
#define FALSE	0
#define TRUE	(!FALSE)
#endif

#define VPD_LARGE_ITEM_TYPE 1
#define VPD_SMALL_ITEM_TYPE 0

#define VPD_ITEM_TYPE(pVR) (((pVR)->type_name_len & 0x80)>>7)

#define VPD_RESOURCE_NAME(pVR)								\
	((VPD_ITEM_TYPE(pVR) == VPD_LARGE_ITEM_TYPE) ?			\
		VPD_LARGE_ITEM_NAME(pVR) : VPD_SMALL_ITEM_NAME(pVR))

#define VPD_RESOURCE_SIZE(pVR)								\
	((VPD_ITEM_TYPE(pVR) == VPD_LARGE_ITEM_TYPE) ?			\
		VPD_LARGE_ITEM_SIZE(pVR) : VPD_SMALL_ITEM_SIZE(pVR))

#define VPD_RESOURCE_DATA(pVR)								\
	((VPD_ITEM_TYPE(pVR) == VPD_LARGE_ITEM_TYPE) ?			\
		(pVR)->large_item.data : (pVR)->small_item.data)

#define VPD_RESOURCE_HDR_SIZE(pVR)	((VPD_ITEM_TYPE(pVR) == VPD_LARGE_ITEM_TYPE) ? 3 : 1)
#define VPD_LARGE_ITEM_SIZE(pVR)	((pVR)->large_item.lsb+(pVR)->large_item.msb*256)
#define VPD_LARGE_ITEM_NAME(pVR)	((pVR)->type_name_len & 0x7F)

#define VPD_SMALL_ITEM_SIZE(pVR)	((pVR)->type_name_len & 0x07)
#define VPD_SMALL_ITEM_NAME(pVR)	(((pVR)->type_name_len & 0x78)>>3)

#define ID_STR	0x02
#define VPD_R	0x10
#define VPD_W	0x11
#define END_TAG	0x0F

#pragma pack(1)

typedef struct {
	unsigned char type_name_len;
	union {
		struct {
			unsigned char data[7];
		} small_item;
		struct {
			unsigned char lsb;
			unsigned char msb;
			unsigned char data[65536];
		} large_item;
	};
} VPD_RESOURCE;

typedef struct {
	unsigned short keyword;
	unsigned char length;
	unsigned char data[255];
} VPD_HEADER;

#pragma pack()

int is_resource_name_ok(unsigned char res_name)
{
	const unsigned char DEFINED_ITEM_NAMES[]={ ID_STR, VPD_R, VPD_W, END_TAG };
	unsigned int i;

	for (i = 0; i < sizeof(DEFINED_ITEM_NAMES); i++) {
		if (res_name == DEFINED_ITEM_NAMES[i]) {
			return (TRUE);
		}
	}
	return (FALSE);
}

unsigned char compute_chksum(unsigned char *data, int len, unsigned char chksum)
{
	while (len--) {
		chksum += *data++;
	}
	return (chksum);
}

int is_vpd_kw_ok(VPD_HEADER **pvph, int ro, unsigned char *chksum, int *last_kw)
{
	unsigned char *KEYWORDS_RO[] = {
		(unsigned char *)"PN",
		(unsigned char *)"EC",
		(unsigned char *)"FG",
		(unsigned char *)"LC",
		(unsigned char *)"MN",
		(unsigned char *)"PG",
		(unsigned char *)"SN",
		(unsigned char *)"Vx",
		(unsigned char *)"CP",
		(unsigned char *)"RV",
		(unsigned char *)0
	};
	unsigned char *KEYWORDS_RW[] = {
		(unsigned char *)"Vx",
		(unsigned char *)"Yx",
		(unsigned char *)"YA",
		(unsigned char *)"RW",
		(unsigned char *)0
	};
	unsigned char **p;
	int i = 0, kwok = FALSE;

	*last_kw = FALSE;
	p = KEYWORDS_RO;

	if (!ro) {
		p=KEYWORDS_RW;
	}

	while (p[i]) {
		if (*(unsigned short *)p[i]==(*pvph)->keyword) {
			kwok = TRUE;
			break;
		}
		if (p[i][1]=='x' && p[i][0]==((unsigned char *)&((*pvph)->keyword))[0]) {
			if (isalnum(((unsigned char *)&((*pvph)->keyword))[1])) {
				kwok = TRUE;
				break;
			}
		}
		i++;
	}

	if (!kwok) {
		w_print("\nUknown VPD KEYWORD \"%c%c\" (0x%01x 0x%01x) found!\n",
			((unsigned char *)&((*pvph)->keyword))[0],
			((unsigned char *)&((*pvph)->keyword))[1],
			((unsigned char *)&((*pvph)->keyword))[0],
			((unsigned char *)&((*pvph)->keyword))[1]);
		return (FALSE);
	}

	switch ((*pvph)->keyword) {
	case 'C' + 256 * 'P':
		*chksum = compute_chksum((unsigned char *)(*pvph), (*pvph)->length + 3,
			*chksum);
		break;
	case 'R' + 256 * 'V':
		*chksum = compute_chksum((unsigned char *)(*pvph), 4, *chksum);
		if (*chksum!=0) {
			w_print("\nVPD computed checksum 0x%02x different "
				"from exp. checksum 0x00!\n", *chksum);
			return (FALSE);
		}
		*last_kw = TRUE;
		break;
	case 'R' + 256 * 'W':
		*last_kw = TRUE;
		break;
	default:
		for (i = 0; i < (*pvph)->length; i++) {
			if (!isalnum((*pvph)->data[i]) && !isspace((*pvph)->data[i]) &&
				!ispunct((*pvph)->data[i])) {

				w_print("\nKEYWORD \"%c%c\" data is not 100%% alpha-numeric!\n",
					((unsigned char *)&((*pvph)->keyword))[0],
					((unsigned char *)&((*pvph)->keyword))[1]);
				return (FALSE);
			}
		}
		*chksum = compute_chksum((unsigned char *)(*pvph), (*pvph)->length + 3,
			*chksum);
		break;
	}

	(*pvph) = (VPD_HEADER *)((unsigned char *)*pvph + (*pvph)->length + 3);
	return (TRUE);
}

int is_vpd_r_content_ok(unsigned char *res_data, int len, unsigned char *chksum)
{
	VPD_HEADER *vph = (VPD_HEADER *)res_data;
	int last_kw = 0;

	while ((unsigned char *)vph - res_data < len) {
		if (last_kw==TRUE) {
			w_print("\nOne of the VPD_R resource size or the 'RV' "
				"keyword length is incorrect!\n");
			return (FALSE);
		}

		if (!is_vpd_kw_ok(&vph, 1, chksum, &last_kw)) {
			return (FALSE);
		}
	}

	if ((unsigned char *)vph - res_data != len) {
		w_print("\n'RV' keyword not found!\n");
		w_print("\nSum of VPD keywords lengths %d is different from VPD_R "
			"resource length %d!\n", (unsigned char *)vph - res_data, len);
			return (FALSE);
	}
	return (TRUE);
}

int is_vpd_w_content_ok(unsigned char *res_data, int len)
{
	VPD_HEADER *vph = (VPD_HEADER *)res_data;
	int last_kw = 0;
	unsigned char unused_chksum;

	while ((unsigned char *)vph - res_data < len) {
		if (last_kw==TRUE) {
			w_print("\nOne of the VPD_W resource size or the 'RW' "
				"keyword length is incorrect!\n");
			return (FALSE);
		}

		if (!is_vpd_kw_ok(&vph,0,&unused_chksum,&last_kw)) {
			return (FALSE);
		}
	}

	if ((unsigned char *)vph - res_data != len) {
		w_print("\n'RW' keyword not found!\n");
		w_print("\nSum of VPD keywords lengths %d is different from VPD_W "
			"resource length %d!\n", (unsigned char *)vph - res_data, len);
		return (FALSE);
	}
	return (TRUE);
}

int is_resource_content_ok(VPD_RESOURCE *pvr, unsigned char *chksum)
{
	switch (VPD_RESOURCE_NAME(pvr)) {
	case ID_STR:
		*chksum = compute_chksum((unsigned char *)pvr, VPD_RESOURCE_SIZE(pvr) +
			VPD_RESOURCE_HDR_SIZE(pvr), *chksum);
		/*Nothing to check*/
		return (TRUE);
	case VPD_R:
		*chksum = compute_chksum((unsigned char *)pvr,
			VPD_RESOURCE_HDR_SIZE(pvr), *chksum);
		return (is_vpd_r_content_ok(VPD_RESOURCE_DATA(pvr),
			VPD_RESOURCE_SIZE(pvr), chksum));
	case VPD_W:
		return (is_vpd_w_content_ok(VPD_RESOURCE_DATA(pvr),
			VPD_RESOURCE_SIZE(pvr)));
	case END_TAG:
		if (VPD_RESOURCE_SIZE(pvr)!=0) {
		w_print("\nEND TAG resource size is not 0!\n");
			return (FALSE);
		}
		return (TRUE);
	default:
		w_print("\nUnexpected resource name 0x%x!\n", VPD_RESOURCE_NAME(pvr));
	}
	return (FALSE);
}

int is_resource_ok(VPD_RESOURCE **ppvr, unsigned char *chksum)
{
	VPD_RESOURCE *pvr=(VPD_RESOURCE *)*ppvr;

	if (!is_resource_name_ok(VPD_RESOURCE_NAME(pvr))) {
		w_print("\nWrong resource name 0x%x found!\n",VPD_RESOURCE_NAME(pvr));
		return (FALSE);
	}

	if (!is_resource_content_ok(pvr,chksum)) {
		w_print("\nWrong resource content detected!\n");
		return (FALSE);
	}

	if (VPD_RESOURCE_NAME(*ppvr) == END_TAG) {
		*ppvr = 0;
	}
	else {
		*ppvr = (VPD_RESOURCE *)((unsigned char *)pvr +
			VPD_RESOURCE_SIZE(pvr) + VPD_RESOURCE_HDR_SIZE(pvr));
	}
	return (TRUE);
}

unsigned char *load_file(char *fn, unsigned long *size)
{
	unsigned char *vpdr = 0;
	FILE *f;

	*size = 0;

	f = fopen(fn, "rb");

	if (f) {
		fseek(f, 0, SEEK_END);
		*size = ftell(f);
		fseek(f, 0, SEEK_SET);

		vpdr = (unsigned char *)malloc(*size);
		if (vpdr) {
			fread(vpdr, *size, 1, f);
		}
		fclose(f);
	}
	else {
		w_print("Can not open file \"%s\": ", fn);
		perror("");
	}
	return (vpdr);
}

VPD_RESOURCE *find_vpd_data(unsigned char *buf, unsigned long len)
{
	VPD_RESOURCE *vpdr = (VPD_RESOURCE *)buf;

	while (len) {
		if (is_resource_name_ok(VPD_RESOURCE_NAME(vpdr))) {
			unsigned long vpdr_size = VPD_RESOURCE_SIZE(vpdr) +
				VPD_RESOURCE_HDR_SIZE(vpdr);

			if (vpdr_size < len) {
				VPD_RESOURCE *next_vpdr =
					(VPD_RESOURCE *)(((unsigned char *)vpdr) + vpdr_size);

				if (is_resource_name_ok(VPD_RESOURCE_NAME(next_vpdr))) {
					/* we might have found the VPD */
					return (vpdr);
				}
			}
		}
		vpdr = (VPD_RESOURCE *)(((unsigned char *)vpdr)+1);
		len--;
	}
	return (0);
}

int vpd_data_valid(VPD_RESOURCE *vpdr)
{
	unsigned char chksum = 0;

	while (vpdr) {
		if (!is_resource_ok(&vpdr,&chksum)) {
			return (FALSE);
		}
	}
	return (TRUE);
}

int check_vpd_buffer(unsigned char *file_buf, unsigned long len)
{
	VPD_RESOURCE *vpdr = 0;

	vpdr = find_vpd_data(file_buf, len);

	if (!vpdr) {
		w_print("\nUnable to find valid VPD!\n");
		return (1);
	}

	if (!vpd_data_valid(vpdr)) {
		return (2);
	}
	return (0);
}

int check_vpd_file(char *fn)
{
	unsigned char *file_buf = 0;
	unsigned long len = 0;
	int ret = 0;

	file_buf = load_file(fn, &len);

	if (file_buf) {
		ret = check_vpd_buffer(file_buf, len);
	}
	else {
		return (3);
	}

	free(file_buf);
	return (ret);
}

#ifdef TEST
int main(int argc, char **args)
{
	if (argc!=2) {
		printf("Usage: %s <raw vpd file>\n", args[0]);
	}
	return (check_vpd_file(args[1]));
}
#endif
