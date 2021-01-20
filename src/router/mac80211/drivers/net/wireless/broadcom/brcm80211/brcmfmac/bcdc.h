// SPDX-License-Identifier: ISC
/*
 * Copyright (c) 2013 Broadcom Corporation
 */
#ifndef BRCMFMAC_BCDC_H
#define BRCMFMAC_BCDC_H
#define BUS_HEADER_LEN	(16+64)		/* Must be atleast SDPCM_RESERVE
					 * (amount of header tha might be added)
					 * plus any space that might be needed
					 * for bus alignment padding.
					 */

struct brcmf_proto_bcdc_dcmd {
	__le32 cmd;	/* dongle command value */
	__le32 len;	/* lower 16: output buflen;
			 * upper 16: input buflen (excludes header) */
	__le32 flags;	/* flag defns given below */
	__le32 status;	/* status code returned from the device */
};

#ifdef CPTCFG_BRCMFMAC_PROTO_BCDC
int brcmf_proto_bcdc_attach(struct brcmf_pub *drvr);
void brcmf_proto_bcdc_detach(struct brcmf_pub *drvr);
void brcmf_proto_bcdc_txflowblock(struct device *dev, bool state);
void brcmf_proto_bcdc_txcomplete(struct device *dev, struct sk_buff *txp,
				 bool success);
struct brcmf_fws_info *drvr_to_fws(struct brcmf_pub *drvr);
#else


struct brcmf_bcdc {
	u16 reqid;
	u8 bus_header[BUS_HEADER_LEN];
	struct brcmf_proto_bcdc_dcmd msg;
	unsigned char buf[BRCMF_DCMD_MAXLEN];
	struct brcmf_fws_info *fws;
};

static inline int brcmf_proto_bcdc_attach(struct brcmf_pub *drvr) { return 0; }
static inline void brcmf_proto_bcdc_detach(struct brcmf_pub *drvr) {}
static inline struct brcmf_fws_info *drvr_to_fws(struct brcmf_pub *drvr)
{
	struct brcmf_bcdc *bcdc = drvr->proto->pd;

	return bcdc->fws;
}

#endif

#endif /* BRCMFMAC_BCDC_H */
