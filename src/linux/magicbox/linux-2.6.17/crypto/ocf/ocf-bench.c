/*
 * benchmark in kernel crypto speed
 */


#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <cryptodev.h>

#if 1
#define BENCH_IXP_ACCESS_LIB 1
#endif
#ifdef BENCH_IXP_ACCESS_LIB
#include <IxTypes.h>
#include <IxOsBuffMgt.h>
#include <IxNpeDl.h>
#include <IxCryptoAcc.h>
#include <IxQMgr.h>
#include <IxOsServices.h>
#include <IxOsCacheMMU.h>
#endif

/*
 * support for access lib version 1.4
 */
#ifndef IX_MBUF_PRIV
#define IX_MBUF_PRIV(x) ((x)->priv)
#endif

/*
 * the number of simultaneously active requests
 */
static int request_q_len = 20;
MODULE_PARM(request_q_len, "i");
MODULE_PARM_DESC(request_q_len, "Number of outstanding requests");
/*
 * how many requests we want to have processed
 */
static int request_num = 1024;
MODULE_PARM(request_num, "i");
MODULE_PARM_DESC(request_num, "run for at least this many requests");
/*
 * the size of each request
 */
static int request_size = 1500;
MODULE_PARM(request_size, "i");
MODULE_PARM_DESC(request_size, "size of each request");

/*
 * a structure for each request
 */
typedef struct  {
	struct work_struct work;
#ifdef BENCH_IXP_ACCESS_LIB
	IX_MBUF mbuf;
#endif
	unsigned char *buffer;
} request_t;

static request_t *requests;

static int outstanding;
static int total;

/*************************************************************************/
/*
 * OCF benchmark routines
 */

static uint64_t ocf_cryptoid;
static int ocf_init(void);
static int ocf_cb(struct cryptop *crp);
static void ocf_request(void *arg);

static int
ocf_init(void)
{
	int error;
	struct cryptoini crie, cria;
	struct cryptodesc crda, crde;

	memset(&crie, 0, sizeof(crie));
	memset(&cria, 0, sizeof(cria));
	memset(&crde, 0, sizeof(crde));
	memset(&crda, 0, sizeof(crda));

	cria.cri_alg  = CRYPTO_SHA1_HMAC;
	cria.cri_klen = 20 * 8;
	cria.cri_key  = "0123456789abcdefghij";

	crie.cri_alg  = CRYPTO_3DES_CBC;
	crie.cri_klen = 24 * 8;
	crie.cri_key  = "0123456789abcdefghijklmn";

	crie.cri_next = &cria;

	error = crypto_newsession(&ocf_cryptoid, &crie, 0);
	if (error) {
		printk("crypto_newsession failed %d\n", error);
		return -1;
	}
	return 0;
}

static int
ocf_cb(struct cryptop *crp)
{
	request_t *r = (request_t *) crp->crp_opaque;

	crypto_freereq(crp);
	crp = NULL;

	total++;
	if (total > request_num) {
		outstanding--;
		return 0;
	}

	INIT_WORK(&r->work, ocf_request, r);
	schedule_work(&r->work);
	return 0;
}


static void
ocf_request(void *arg)
{
	request_t *r = arg;
	struct cryptop *crp = crypto_getreq(2);
	struct cryptodesc *crde, *crda;

	if (!crp) {
		outstanding--;
		return;
	}

	crda = crp->crp_desc;
	crde = crda->crd_next;

	crda->crd_skip = 0;
	crda->crd_flags = 0;
	crda->crd_len = request_size;
	crda->crd_inject = request_size;
	crda->crd_alg = CRYPTO_SHA1_HMAC;
	crda->crd_key = "0123456789abcdefghij";
	crda->crd_klen = 20 * 8;

	crde->crd_skip = 0;
	crde->crd_flags = CRD_F_IV_EXPLICIT | CRD_F_ENCRYPT;
	crde->crd_len = request_size;
	crde->crd_inject = request_size;
	crde->crd_alg = CRYPTO_3DES_CBC;
	crde->crd_key = "0123456789abcdefghijklmn";
	crde->crd_klen = 24 * 8;

	crp->crp_ilen = request_size + 64;
	crp->crp_flags = CRYPTO_F_CBIMM;
	crp->crp_buf = (caddr_t) r->buffer;
	crp->crp_callback = ocf_cb;
	crp->crp_sid = ocf_cryptoid;
	crp->crp_opaque = (caddr_t) r;
	crypto_dispatch(crp);
}

/*************************************************************************/
#ifdef BENCH_IXP_ACCESS_LIB
/*************************************************************************/
/*
 * CryptoAcc benchmark routines
 */

static IxCryptoAccCtx ixp_ctx;
static UINT32 ixp_ctx_id;
static IX_MBUF ixp_pri;
static IX_MBUF ixp_sec;
static int ixp_registered = 0;

static void ixp_register_cb(UINT32 ctx_id, IX_MBUF *bufp,
					IxCryptoAccStatus status);
static void ixp_perform_cb(UINT32 ctx_id, IX_MBUF *sbufp, IX_MBUF *dbufp,
					IxCryptoAccStatus status);
static void ixp_request(void *arg);

static int
ixp_init(void)
{
	IxCryptoAccStatus status;

	ixp_ctx.cipherCtx.cipherAlgo = IX_CRYPTO_ACC_CIPHER_3DES;
	ixp_ctx.cipherCtx.cipherMode = IX_CRYPTO_ACC_MODE_CBC;
	ixp_ctx.cipherCtx.cipherKeyLen = 24;
	ixp_ctx.cipherCtx.cipherBlockLen = IX_CRYPTO_ACC_DES_BLOCK_64;
	ixp_ctx.cipherCtx.cipherInitialVectorLen = IX_CRYPTO_ACC_DES_IV_64;
	memcpy(ixp_ctx.cipherCtx.key.cipherKey, "0123456789abcdefghijklmn", 24);

	ixp_ctx.authCtx.authAlgo = IX_CRYPTO_ACC_AUTH_SHA1;
	ixp_ctx.authCtx.authDigestLen = 12;
	ixp_ctx.authCtx.aadLen = 0;
	ixp_ctx.authCtx.authKeyLen = 20;
	memcpy(ixp_ctx.authCtx.key.authKey, "0123456789abcdefghij", 20);

	ixp_ctx.useDifferentSrcAndDestMbufs = 0;
	ixp_ctx.operation = IX_CRYPTO_ACC_OP_ENCRYPT_AUTH ;

	IX_MBUF_MLEN(&ixp_pri)  = IX_MBUF_PKT_LEN(&ixp_pri) = 128;
	IX_MBUF_MDATA(&ixp_pri) = (unsigned char *) kmalloc(128, GFP_ATOMIC);
	IX_MBUF_MLEN(&ixp_sec)  = IX_MBUF_PKT_LEN(&ixp_sec) = 128;
	IX_MBUF_MDATA(&ixp_sec) = (unsigned char *) kmalloc(128, GFP_ATOMIC);

	status = ixCryptoAccCtxRegister(&ixp_ctx, &ixp_pri, &ixp_sec,
			ixp_register_cb, ixp_perform_cb, &ixp_ctx_id);

	if (IX_CRYPTO_ACC_STATUS_SUCCESS == status) {
		while (!ixp_registered)
			schedule();
		return ixp_registered < 0 ? -1 : 0;
	}

	printk("ixp: ixCryptoAccCtxRegister failed %d\n", status);
	return -1;
}

static void
ixp_register_cb(UINT32 ctx_id, IX_MBUF *bufp, IxCryptoAccStatus status)
{
	if (bufp) {
		IX_MBUF_MLEN(bufp) = IX_MBUF_PKT_LEN(bufp) = 0;
		kfree(IX_MBUF_MDATA(bufp));
		IX_MBUF_MDATA(bufp) = NULL;
	}

	if (IX_CRYPTO_ACC_STATUS_WAIT == status)
		return;
	if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
		ixp_registered = 1;
	else
		ixp_registered = -1;
}

static void
ixp_perform_cb(
	UINT32 ctx_id,
	IX_MBUF *sbufp,
	IX_MBUF *dbufp,
	IxCryptoAccStatus status)
{
	request_t *r = NULL;

	total++;
	if (total > request_num) {
		outstanding--;
		return;
	}

	if (!sbufp || !(r = IX_MBUF_PRIV(sbufp))) {
		printk("crappo %p %p\n", sbufp, r);
		outstanding--;
		return;
	}

	INIT_WORK(&r->work, ixp_request, r);
	schedule_work(&r->work);
}

static void
ixp_request(void *arg)
{
	request_t *r = arg;
	IxCryptoAccStatus status;

	memset(&r->mbuf, 0, sizeof(r->mbuf));
	IX_MBUF_MLEN(&r->mbuf) = IX_MBUF_PKT_LEN(&r->mbuf) = request_size + 64;
	IX_MBUF_MDATA(&r->mbuf) = r->buffer;
	IX_MBUF_PRIV(&r->mbuf) = r;
	status = ixCryptoAccAuthCryptPerform(ixp_ctx_id, &r->mbuf, NULL,
			0, request_size, 0, request_size, request_size, r->buffer);
	if (IX_CRYPTO_ACC_STATUS_SUCCESS != status) {
		printk("status1 = %d\n", status);
		outstanding--;
		return;
	}
	return;
}

/*************************************************************************/
#endif /* BENCH_IXP_ACCESS_LIB */
/*************************************************************************/

int
ocfbench_init(void)
{
	int i, jstart, jstop;

	printk("Crypto Speed tests\n");

	requests = kmalloc(sizeof(request_t) * request_q_len, GFP_KERNEL);
	if (!requests) {
		printk("malloc failed\n");
		return -EINVAL;
	}

	for (i = 0; i < request_q_len; i++) {
		/* +64 for return data */
		requests[i].buffer = kmalloc(request_size + 128, GFP_DMA);
		if (!requests[i].buffer) {
			printk("malloc failed\n");
			return -EINVAL;
		}
	}

	/*
	 * OCF benchmark
	 */
	printk("OCF: testing ...\n");
	ocf_init();
	total = outstanding = 0;
	jstart = jiffies;
	for (i = 0; i < request_q_len; i++) {
		outstanding++;
		ocf_request(&requests[i]);
	}
	while (outstanding > 0)
		schedule();
	jstop = jiffies;

	printk("OCF: %d requests of %d bytes in %d jiffies\n", total, request_size,
			jstop - jstart);

#ifdef BENCH_IXP_ACCESS_LIB
	/*
	 * IXP benchmark
	 */
	printk("IXP: testing ...\n");
	ixp_init();
	total = outstanding = 0;
	jstart = jiffies;
	for (i = 0; i < request_q_len; i++) {
		outstanding++;
		ixp_request(&requests[i]);
	}
	while (outstanding > 0)
		schedule();
	jstop = jiffies;

	printk("IXP: %d requests of %d bytes in %d jiffies\n", total, request_size,
			jstop - jstart);
#endif /* BENCH_IXP_ACCESS_LIB */

	for (i = 0; i < request_q_len; i++)
		kfree(requests[i].buffer);
	kfree(requests);
	return -EINVAL; /* always fail to load so it can be re-run quickly ;-) */
}

static void __exit ocfbench_exit(void)
{
}

module_init(ocfbench_init);
module_exit(ocfbench_exit);

MODULE_LICENSE("BSD");
MODULE_AUTHOR("David McCullough <dmccullough@cyberguard.com>");
MODULE_DESCRIPTION("Benchmark various in-kernel crypto speeds");
