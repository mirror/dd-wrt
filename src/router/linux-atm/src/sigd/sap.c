/* sap.c - SAP manipulations */
 
/* Written 1996-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <atm.h>
#include <linux/atmdev.h>
#include <linux/atmsvc.h>

#include "atmd.h"
#include "uni.h"
#include "qlib.h"
#include <q.out.h>

#include "proto.h"
#include "sap.h"


#define COMPONENT "SAP"


static int class_compat(const struct atm_trafprm *tx,
  const struct atm_trafprm *rx)
{
    if (tx->traffic_class == ATM_NONE || tx->traffic_class == ATM_ANYCLASS)
	return 1;
    if (rx->traffic_class == ATM_UBR || rx->traffic_class == ATM_ANYCLASS)
	return 1;
	/* don't apply CAC to PCR */
    if (tx->traffic_class != rx->traffic_class) return 0;
	/* ignore special cases like CBR to VBR for now */
    switch (tx->traffic_class) {
	case ATM_CBR:
	    if (!rx->max_pcr || rx->max_pcr == ATM_MAX_PCR) return 1;
	    return tx->min_pcr <= rx->max_pcr;
	    /* Actually, we shouldn't look at min_pcr, because there's no
	       bandwidth negotiation anyway. */
	default:
	    diag(COMPONENT,DIAG_ERROR,"unsupported traffic class %d\n",
	      tx->traffic_class);
	    return 0;
    }
}

int sap_compat(const struct sockaddr_atmsvc *old_addr,
  const struct sockaddr_atmsvc *new_addr,struct sockaddr_atmsvc *res_addr,
  const struct atm_sap *old_sap,const struct atm_sap *new_sap,
  struct atm_sap *res_sap,const struct atm_qos *old_qos,
  const struct atm_qos *new_qos,struct atm_qos *res_qos)
{
    if (atmsvc_addr_in_use(*old_addr) &&
      !atm_equal((struct sockaddr *) old_addr,(struct sockaddr *) new_addr,0,0))
	return 0;
    if (res_qos) *res_qos = *new_qos;
    if (old_qos->txtp.max_sdu && new_qos->txtp.max_sdu &&
      old_qos->txtp.max_sdu > new_qos->txtp.max_sdu) return 0;
    if (new_qos->rxtp.max_sdu && old_qos->rxtp.max_sdu &&
      new_qos->rxtp.max_sdu > old_qos->rxtp.max_sdu) return 0;
    if (!class_compat(&old_qos->rxtp,&new_qos->rxtp) ||
      !class_compat(&new_qos->txtp,&old_qos->txtp)) return 0;
    if (!sap_equal(old_sap,new_sap,
      SXE_COMPATIBLE | SXE_NEGOTIATION | (res_sap ? SXE_RESULT : 0),res_sap))
	return 0;
    return 1;
}


static int encode_blli(Q_DSC *dsc,const struct atm_blli *blli)
{
    if (blli->l2_proto != ATM_L2_NONE) {
	q_assign(dsc,QF_uil2_proto,blli->l2_proto);
	switch (blli->l2_proto) {
	    case ATM_L2_X25_LL:
	    case ATM_L2_X25_ML:
	    case ATM_L2_HDLC_ARM:
	    case ATM_L2_HDLC_NRM:
	    case ATM_L2_HDLC_ABM:
	    case ATM_L2_Q922:
	    case ATM_L2_ISO7776:
		if (blli->l2.itu.mode != ATM_IMD_NONE)
		    q_assign(dsc,QF_l2_mode,blli->l2.itu.mode);
		if (blli->l2.itu.window)
		    q_assign(dsc,QF_window_size,blli->l2.itu.window);
		break;
	    case ATM_L2_USER:
		q_assign(dsc,QF_user_l2,blli->l2.user);
		break;
	    default:
		break;
	}
    }
    if (blli->l3_proto != ATM_L3_NONE) {
	q_assign(dsc,QF_uil3_proto,blli->l3_proto);
	switch (blli->l3_proto) {
	    case ATM_L3_X25:
	    case ATM_L3_ISO8208:
	    case ATM_L3_X223:
		if (blli->l3.itu.mode != ATM_IMD_NONE)
		    q_assign(dsc,QF_l3_mode,blli->l3.itu.mode);
		if (blli->l3.itu.def_size)
		    q_assign(dsc,QF_def_pck_size,blli->l3.itu.def_size);
		if (blli->l3.itu.window)
		    q_assign(dsc,QF_pck_win_size,blli->l3.itu.window);
		break;
	    case ATM_L3_TR9577:
		q_assign(dsc,QF_ipi_high,blli->l3.tr9577.ipi >> 1);
		q_assign(dsc,QF_ipi_low,blli->l3.tr9577.ipi & 1);
		if (blli->l3.tr9577.ipi == NLPID_IEEE802_1_SNAP) {
		    q_write(dsc,QF_oui,blli->l3.tr9577.snap,3);
		    q_write(dsc,QF_pid,blli->l3.tr9577.snap+3,2);
		}
		break;
	    case ATM_L3_USER:
		q_assign(dsc,QF_user_l3,blli->l3.user);
		break;
	    default:
		diag(COMPONENT,DIAG_ERROR,"bad l3_proto (%d)",
		  blli->l3_proto);
		return -EINVAL;
	}
    }
    return 0;
}


int sap_encode(Q_DSC *dsc,const struct sockaddr_atmsvc *addr,
  const struct atm_sap *sap,const struct atm_qos *qos,int uni,int max_rate)
{
    int error,pcr;

    if (*addr->sas_addr.pub)
	q_write(dsc,QF_cdpn_e164,(void *) addr->sas_addr.pub,
	  strlen(addr->sas_addr.pub));
    else if (*addr->sas_addr.prv)
	    q_write(dsc,QF_cdpn_esa,(void *) addr->sas_addr.prv,ATM_ESA_LEN);
	else return -EDESTADDRREQ;
if (qos) {
    if (qos->txtp.traffic_class == ATM_UBR || qos->rxtp.traffic_class ==
      ATM_UBR) {
	q_assign(dsc,QF_best_effort,0);
#if defined(UNI30) || defined(ALLOW_UNI30) || defined(DYNAMIC_UNI)
	if (uni & S_UNI30)
	    q_assign(dsc,QF_trans_cap,ATM_TC_VBR_NRT_R00);
	      /* force presence - UNI 3.0 wants this */
#endif
    }
    if (qos->txtp.traffic_class == ATM_CBR || qos->rxtp.traffic_class ==
      ATM_CBR)
	q_assign(dsc,QF_trans_cap,ATM_TC_CBR);
    switch (qos->txtp.traffic_class) {
	case ATM_NONE:
	    q_assign(dsc,QF_fw_pcr_01,0);
	    break;
	case ATM_UBR:
	    /* fall through */
	case ATM_CBR:
	    /* here's a bit of policy: send the highest value we have */
	    pcr = SELECT_TOP_PCR(qos->txtp);
	    diag(COMPONENT,DIAG_DEBUG,"fwd %d (%d..%d)",pcr,
	      qos->txtp.min_pcr,qos->txtp.max_pcr);
	    if (pcr == ATM_MAX_PCR) pcr = max_rate;
	    q_assign(dsc,QF_fw_pcr_01,pcr);
	    break;
        default:
	    diag(COMPONENT,DIAG_ERROR,"bad TX class (%d)",
	      qos->txtp.traffic_class);
	    return -EINVAL;
    }
    switch (qos->rxtp.traffic_class) {
	case ATM_NONE:
	    q_assign(dsc,QF_bw_pcr_01,0);
	    break;
	case ATM_UBR:
	    /* fall through */
	case ATM_CBR:
	    pcr = SELECT_TOP_PCR(qos->rxtp);
	    diag(COMPONENT,DIAG_DEBUG,"bwd %d (%d..%d)",pcr,
	      qos->rxtp.min_pcr,qos->rxtp.max_pcr);
	    if (pcr == ATM_MAX_PCR) pcr = max_rate;
	    q_assign(dsc,QF_bw_pcr_01,pcr);
	    break;
        default:
	    diag(COMPONENT,DIAG_ERROR,"bad RX class (%d)",
	      qos->rxtp.traffic_class);
	    return -EINVAL;
    }
    if (qos->txtp.max_sdu || qos->rxtp.max_sdu) {
	q_assign(dsc,QF_fw_max_sdu,qos->txtp.max_sdu);
	q_assign(dsc,QF_bw_max_sdu,qos->rxtp.max_sdu);
    }
}
    /* @@@ bearer class ? */
    /* @@@ QOS class ? */
    if (sap->bhli.hl_type != ATM_HL_NONE) {
        q_assign(dsc,QF_hli_type,sap->bhli.hl_type-1);
        switch (sap->bhli.hl_type) {
            case ATM_HL_ISO:
                q_write(dsc,QF_iso_hli,sap->bhli.hl_info,sap->bhli.hl_length);
                break;
            case ATM_HL_USER:
                q_write(dsc,QF_user_hli,sap->bhli.hl_info,sap->bhli.hl_length);
                break;
#if defined(UNI30) || defined(DYNAMIC_UNI)
            case ATM_HL_HLP:
		if (!(uni & S_UNI30)) return -EINVAL;
                q_write(dsc,QF_hlp,sap->bhli.hl_info,4);
                break;
#endif
            case ATM_HL_VENDOR:
		q_write(dsc,QF_hli_oui,sap->bhli.hl_info,3);
		q_write(dsc,QF_app_id, sap->bhli.hl_info+3,4);
                break;
            default:
                diag(COMPONENT,DIAG_ERROR,"bad hl_type (%d)",
		  sap->bhli.hl_type);
		return -EINVAL;
        }
    }
    if (!blli_in_use(sap->blli[0])) return 0;
    q_instance(dsc,QG_blli1);
    error = encode_blli(dsc,sap->blli);
    if (error) return 0;
    if (!blli_in_use(sap->blli[1])) return 0;
    q_instance(dsc,QG_blli2);
    error = encode_blli(dsc,sap->blli+1);
    if (error) return 0;
    if (!blli_in_use(sap->blli[2])) return 1;
    q_instance(dsc,QG_blli3);
    return encode_blli(dsc,sap->blli+2);
}


static void decode_blli(Q_DSC *dsc,struct atm_blli *blli)
{
#define GET(var,field) \
   ({ if (q_present(dsc,field)) blli->var = q_fetch(dsc,field); })

    if (q_present(dsc,QF_uil2_proto)) {
	blli->l2_proto = q_fetch(dsc,QF_uil2_proto);
	GET(l2.itu.mode,QF_l2_mode);
	GET(l2.itu.window,QF_window_size);
	GET(l2.user,QF_user_l2);
    }
    if (q_present(dsc,QF_uil3_proto)) {
	blli->l3_proto = q_fetch(dsc,QF_uil3_proto);
	GET(l3.itu.mode,QF_l3_mode);
	GET(l3.itu.def_size,QF_def_pck_size);
	GET(l3.itu.window,QF_pck_win_size);
	GET(l3.user,QF_user_l3);
	if (q_present(dsc,QF_ipi_high)) {
	    blli->l3.tr9577.ipi = q_fetch(dsc,QF_ipi_high) << 1;
	    if (blli->l3.tr9577.ipi != NLPID_IEEE802_1_SNAP)
		blli->l3.tr9577.ipi |= q_fetch(dsc,QF_ipi_low);
	    else if (!q_present(dsc,QF_oui)) blli->l3.tr9577.ipi |= 1;
		else {
		    q_read(dsc,QF_oui,blli->l3.tr9577.snap,3);
		    q_read(dsc,QF_pid,blli->l3.tr9577.snap+3,2);
		}
	}
    }
#undef GET
}

/*
 * NOTE: the incoming call sap is converted to a "local view" 
 * in preparation for use by a local socket.  Therefore, the
 * forward values are assigned to atm_qos->rxtp and the
 * backward values are assigned to atm_qos->txtp.  This has
 * implications later for sap_compat() when we compare the
 * forward/backward parameters.
 */

unsigned int sap_decode(Q_DSC *dsc,struct sockaddr_atmsvc *addr,
  struct atm_sap *sap,struct atm_qos *qos,int uni)
{
    memset(addr,0,sizeof(*addr));
    memset(sap,0,sizeof(*sap));
    memset(qos,0,sizeof(*qos));
    addr->sas_family = AF_ATMSVC;
    if (q_present(dsc,QF_cdpn_e164))
	(void) q_read(dsc,QF_cdpn_e164,(void *) &addr->sas_addr.pub,
	  ATM_E164_LEN);
    else if (q_present(dsc,QF_cdpn_esa))
	    (void) q_read(dsc,QF_cdpn_esa,(void *) &addr->sas_addr.prv,
	      ATM_ESA_LEN);
    qos->aal = ATM_AAL5; /* default and only choice */
    if (q_present(dsc,QF_aal_type))
	if (q_fetch(dsc,QF_aal_type) != 5) {
	    diag(COMPONENT,DIAG_ERROR,"AAL type %d requested",
	      q_fetch(dsc,QF_aal_type));
#ifdef DYNAMIC_UNI
	    return IE_PROBLEM(
	      uni & S_UNI30 ? ATM_CV_AAL_UNSUPP_OLD : ATM_CV_AAL_UNSUPP_NEW,0);
#else
#ifdef UNI30
	    return IE_PROBLEM(ATM_CV_AAL_UNSUPP_OLD,0);
#else
	    return IE_PROBLEM(ATM_CV_AAL_UNSUPP_NEW,0);
#endif /* UNI30 */
#endif /* DYNAMIC_UNI */
	}
    if (q_present(dsc,QF_best_effort)) {
	qos->txtp.traffic_class = qos->rxtp.traffic_class = ATM_UBR;
	diag(COMPONENT,DIAG_DEBUG,"UBR");
    }
    else {
	qos->txtp.traffic_class = qos->rxtp.traffic_class = ATM_CBR;
	diag(COMPONENT,DIAG_DEBUG,"CBR");
    }
    qos->txtp.max_pcr = qos->rxtp.max_pcr = 0;
    /* unbalanced decoding - always sets upper bound */
    if (q_present(dsc,QF_fw_pcr_01)) {
	qos->rxtp.min_pcr = 0;
	qos->rxtp.max_pcr = q_fetch(dsc,QF_fw_pcr_01);
    }
    if (q_present(dsc,QF_bw_pcr_01)) {
	qos->txtp.min_pcr = 0;
	qos->txtp.max_pcr = q_fetch(dsc,QF_bw_pcr_01);
    }
    if (!qos->txtp.max_pcr) qos->txtp.traffic_class = ATM_NONE;
    if (!qos->rxtp.max_pcr) qos->rxtp.traffic_class = ATM_NONE;
    diag(COMPONENT,DIAG_DEBUG,"fwd %d..%d bwd %d..%d",
      qos->rxtp.min_pcr,qos->rxtp.max_pcr,qos->txtp.min_pcr,
      qos->txtp.max_pcr);
    /* SHOULD ... fail call if anything is missing ... @@@ */
    if (q_present(dsc,QF_bw_max_sdu))
	qos->txtp.max_sdu = q_fetch(dsc,QF_bw_max_sdu);
    if (q_present(dsc,QF_fw_max_sdu))
	qos->rxtp.max_sdu = q_fetch(dsc,QF_fw_max_sdu);
    if (q_present(dsc,QG_bhli)) {
	sap->bhli.hl_type = q_fetch(dsc,QF_hli_type)+1;
	switch (sap->bhli.hl_type) {
	    case ATM_HL_ISO:
		sap->bhli.hl_length = q_length(dsc,QF_iso_hli);
		q_read(dsc,QF_iso_hli,sap->bhli.hl_info,sap->bhli.hl_length);
		break;
	    case ATM_HL_USER:
		sap->bhli.hl_length = q_length(dsc,QF_user_hli);
		q_read(dsc,QF_user_hli,sap->bhli.hl_info,sap->bhli.hl_length);
		break;
	    case ATM_HL_VENDOR:
		sap->bhli.hl_length = 7;
		q_read(dsc,QF_hli_oui,sap->bhli.hl_info,3);
		q_read(dsc,QF_app_id,sap->bhli.hl_info+3,4);
		break;
#if defined(UNI30) || defined(DYNAMIC_UNI)
	    case ATM_HL_HLP:
		if (!(uni & S_UNI30)) {
		    sap->bhli.hl_length = 4;
		    q_read(dsc,QF_hlp,sap->bhli.hl_info,4);
		    break;
		}
		/* fall through */
#endif
	    default:
		diag(COMPONENT,DIAG_ERROR,"unrecognized hl_type %d",
		  sap->bhli.hl_type);
		return IE_PROBLEM(ATM_CV_INVALID_IE,ATM_IE_BHLI);
	}
    }
    if (!q_present(dsc,QG_blli1)) return 0;
    q_instance(dsc,QG_blli1);
    decode_blli(dsc,sap->blli);
    if (!q_present(dsc,QG_blli2)) return 0;
    q_instance(dsc,QG_blli2);
    decode_blli(dsc,sap->blli+1);
    if (!q_present(dsc,QG_blli3)) return 0;
    q_instance(dsc,QG_blli3);
    decode_blli(dsc,sap->blli+2);
    return 0;
}
