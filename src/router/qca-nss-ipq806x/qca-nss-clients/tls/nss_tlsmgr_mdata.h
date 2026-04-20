/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE
 **************************************************************************
 */

/**
 * @file nss_tlsmgr_mdata.h
 *	NSS TLS common metadata definitions.
 */

#ifndef _NSS_TLSMGR_MDATA_H_
#define _NSS_TLSMGR_MDATA_H_

#define NSS_TLSMGR_MDATA_VER  0x01  /**< Metadata version. */
#define NSS_TLSMGR_MDATA_MAGIC  0x8e8f  /**< Metadata magic. */

#define NSS_TLSMGR_MDATA_FRAG_MAX 4
#define NSS_TLSMGR_MDATA_REC_MAX 4
#define NSS_TLSMGR_MAX_IN_FRAGS (NSS_TLSMGR_MDATA_REC_MAX * NSS_TLSMGR_MDATA_FRAG_MAX)  /**< Maximum input fragments. */
#define NSS_TLSMGR_MAX_OUT_FRAGS (NSS_TLSMGR_MDATA_REC_MAX * NSS_TLSMGR_MDATA_FRAG_MAX)	/**< Maximum output fragments. */

#define NSS_TLSMGR_MDATA_FLAG_CCS 0x1
#define NSS_TLSMGR_MDATA_FLAG_ERROR 0x2

#define NSS_TLSMGR_MDATA_FRAG_FLAG_FIRST 0x1
#define NSS_TLSMGR_MDATA_FRAG_FLAG_LAST 0x2

/**
 * nss_tlsmgr_mdata_frag
 *	TLS fragment information.
 */
struct nss_tlsmgr_mdata_frag {
	uint32_t addr;		/**< Adress of data fragment. */
	uint16_t len;		/**< Length of data fragment. */
	uint16_t flags;		/**< Reserved. */
};

/**
 * nss_tlsmgr_mdata_rec
 *	TLS record information.
 */
struct nss_tlsmgr_mdata_rec {
	uint16_t rec_len;					/**< Length of payload in this record. */
	uint8_t rec_type;					/**< Type of record. */
	uint8_t error; 						/* Transform error codes */
	uint8_t in_frags;					/**< Number of input fragments. */
	uint8_t out_frags;					/**< Number of output fragments. */
	uint8_t res[2]; 					/**< Reserved. */
	struct nss_tlsmgr_mdata_frag in[NSS_TLSMGR_MDATA_FRAG_MAX];	/**< Input fragments. */
	struct nss_tlsmgr_mdata_frag out[NSS_TLSMGR_MDATA_FRAG_MAX];	/**< Output fragments. */
};

/**
 * nss_tlsmgr_mdata
 *	TLS metadata.
 */
struct nss_tlsmgr_mdata {
	uint8_t  ver;		/**< Metadata version. */
	uint8_t rec_cnt;	/**< Total number of records. */
	uint8_t in_frag_cnt;	/**< Total number of input fragments. */
	uint8_t out_frag_cnt;	/**< Total number of output fragments. */
	uint32_t flags;		/**< Transformation flags. */
	uint8_t res[2];		/**< Reserved. */
	uint16_t magic;         /**< Meta data magic. */
	struct nss_tlsmgr_mdata_rec rec[NSS_TLSMGR_MDATA_REC_MAX];	/**< Records structure. */
};

/*
 * nss_tlsmgr_mdata_get_ver()
 *	Get TLS metadata Version.
 */
static inline uint8_t nss_tlsmgr_mdata_get_ver(struct nss_tlsmgr_mdata *ntm)
{
	return ntm->ver;
}

/*
 * nss_tlsmgr_mdata_get_magic()
 *	Get TLS metadata Magic.
 */
static inline uint16_t nss_tlsmgr_mdata_get_magic(struct nss_tlsmgr_mdata *ntm)
{
	return ntm->magic;
}

/*
 * nss_tlsmgr_mdata_get_rec_cnt()
 *	Get TLS metadata record count.
 */
static inline uint8_t nss_tlsmgr_mdata_get_rec_cnt(struct nss_tlsmgr_mdata *ntm)
{
	return ntm->rec_cnt;
}

/*
 * nss_tlsmgr_mdata_get_rec()
 *	Get start of record.
 */
static inline struct nss_tlsmgr_mdata_rec *nss_tlsmgr_mdata_get_rec(struct nss_tlsmgr_mdata *ntm, uint8_t idx)
{
	return &ntm->rec[idx];
}

/*
 * nss_tlsmgr_mdata_get_rec_len()
 *	Get record length.
 */
static inline uint16_t nss_tlsmgr_mdata_rec_get_rec_len(struct nss_tlsmgr_mdata_rec *ntmr)
{
	return ntmr->rec_len;
}

/*
 * nss_tlsmgr_mdata_get_rec_type()
 *	Get record type.
 */
static inline uint8_t nss_tlsmgr_mdata_rec_get_rec_type(struct nss_tlsmgr_mdata_rec *ntmr)
{
	return ntmr->rec_type;
}

/*
 * nss_tlsmgr_mdata_rec_get_error()
 *	Get error status from metadata.
 */
static inline uint8_t nss_tlsmgr_mdata_rec_get_error(struct nss_tlsmgr_mdata_rec *ntmr)
{
	return ntmr->error;
}

/*
 * nss_tlsmgr_mdata_rec_get_in_frags()
 *	Get total number of input fragments.
 */
static inline uint8_t nss_tlsmgr_mdata_rec_get_in_frags(struct nss_tlsmgr_mdata_rec *ntmr)
{
	return ntmr->in_frags;
}

/*
 * nss_tlsmgr_mdata_rec_get_out_frags()
 *	Get total number of output fragments.
 */
static inline uint8_t nss_tlsmgr_mdata_rec_get_out_frags(struct nss_tlsmgr_mdata_rec *ntmr)
{
	return ntmr->out_frags;
}

/*
 * nss_tlsmgr_mdata_rec_get_in_frag()
 *	Get start of input fragment.
 */
static inline struct nss_tlsmgr_mdata_frag *nss_tlsmgr_mdata_rec_get_in_frag(struct nss_tlsmgr_mdata_rec *ntmr)
{
	return ntmr->in;
}

/*
 * nss_tlsmgr_mdata_rec_get_out_frag()
	int error;
 *	Get start of output fragment.
 */
static inline struct nss_tlsmgr_mdata_frag *nss_tlsmgr_mdata_rec_get_out_frag(struct nss_tlsmgr_mdata_rec *ntmr)
{
	return ntmr->out;
}

/*
 * nss_tlsmgr_mdata_rec_verify_idx()
 *	Check if record index is out of range.
 */
static inline bool nss_tlsmgr_mdata_rec_verify_idx(uint8_t idx)
{
	return idx > NSS_TLSMGR_MDATA_REC_MAX;
}

/*
 * nss_tlsmgr_mdata_frag_is_overflow()
 *	Check if fragment index is out of range.
 */
static inline bool nss_tlsmgr_mdata_frag_verify_count(uint8_t frag_cnt)
{
	return frag_cnt > NSS_TLSMGR_MDATA_FRAG_MAX;
}

/**
 * @}
 */
#endif /* _NSS_TLSMGR_MDATA_H_ */


