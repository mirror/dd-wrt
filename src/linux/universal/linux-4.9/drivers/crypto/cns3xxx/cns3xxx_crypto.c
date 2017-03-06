/*
 * Support for CNS3XXX crypto engine which can be found on some CNS34xx
 * boards.
 *
 * License: GPLv2
 *
 */
#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <crypto/internal/hash.h>
#include <crypto/sha.h>

#include "cns3xxx_crypto.h"
//#include "cavium_common.h"

#define CNS3XXX_CRYPTO	"CNS3XXX-CRYPTO:"
#define MAX_HW_HASH_SIZE	0xFFFF

#define CNS3XXX_DECRYPT		0
#define CNS3XXX_ENCRYPT		1


void crypto_int(int, void *);


enum engine_status {
	ENGINE_IDLE,
	ENGINE_BUSY,
	ENGINE_W_DEQUEUE,
};

/**
 * struct req_progress - used for every crypt request
 * @src_sg_it:		sg iterator for src
 * @dst_sg_it:		sg iterator for dst
 * @sg_src_left:	bytes left in src to process (scatter list)
 * @src_start:		offset to add to src start position (scatter list)
 * @crypt_len:		length of current hw crypt/hash process
 * @hw_nbytes:		total bytes to process in hw for this request
 * @copy_back:		whether to copy data back (crypt) or not (hash)
 * @sg_dst_left:	bytes left dst to process in this scatter list
 * @dst_start:		offset to add to dst start position (scatter list)
 * @hw_processed_bytes:	number of bytes processed by hw (request).
 *
 * sg helper are used to iterate over the scatterlist. Since the size of the
 * SRAM may be less than the scatter size, this struct struct is used to keep
 * track of progress within current scatterlist.
 */
struct req_progress {
	struct sg_mapping_iter src_sg_it;
	struct sg_mapping_iter dst_sg_it;
	void (*complete) (void);
	void (*process) (int is_first);

	/* src mostly */
	int sg_src_left;
	int src_start;
	int crypt_len;
	int hw_nbytes;
	/* dst mostly */
	int copy_back;
	int sg_dst_left;
	int dst_start;
	int hw_processed_bytes;
};

struct crypto_priv {
	u8 in_buf[4096+256];
	u8 out_buf[4096+256];
	void __iomem *reg;
	void __iomem *sram;

	int irq;
	struct task_struct *queue_th;

	/* the lock protects queue and eng_st */
	spinlock_t lock;
	struct crypto_queue queue;
	enum engine_status eng_st;
	struct crypto_async_request *cur_req;
	struct req_progress p;
	int max_req_size;
	int has_sha1;
	int has_hmac_sha1;
	int src_num_sgs;
	int dst_num_sgs;
};

static struct crypto_priv *cpg;

struct cns3xxx_ctx {
	u8 aes_enc_key[AES_KEY_LEN];
	u32 aes_dec_key[8];
	int key_len;
	u32 need_calc_aes_dkey;
};

struct accel_config {
        int enc;
        AesType aes_type;
        u16 msg_len;
        n1_scatter_buffer inv;
        n1_scatter_buffer outv;
};


enum crypto_op {
	COP_AES_ECB,
	COP_AES_CBC,
};

struct cns3xxx_req_ctx {
	enum crypto_op op;
	int decrypt;
};

enum hash_op {
	COP_SHA1,
	COP_HMAC_SHA1
};

struct N1_Dev {
        struct N1_Dev *next;
        int id;
        int bus;
        int dev;
        int func;
        void *data;
};


u64 ctx;
static int module_count = 0;
static struct N1_Dev *n1_list = NULL;
u64  n1_ssl_alloc_context(void *);
void n1_ssl_dealloc_context(void *,u64);
u32 n1_KernAes (void *, int, AesType,u64, u16,
         n1_scatter_buffer *inv,
         n1_scatter_buffer *outv,
         CallBackFn cb, void *cb_data);

static u64 (*p_alloc_context)(void *) = NULL;
static void (*p_dealloc_context)(void *, u64) = NULL;
static u32 (*p_KernAes)(void *, int, AesType, u64, u16, n1_scatter_buffer *, n1_scatter_buffer *, CallBackFn, void *)=NULL;

static int cns3xxx_setkey_aes(struct crypto_ablkcipher *cipher, const u8 *key,
		unsigned int len)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct cns3xxx_ctx *ctx = crypto_tfm_ctx(tfm);

	switch (len) {
	case AES_KEYSIZE_128:
	case AES_KEYSIZE_192:
	case AES_KEYSIZE_256:
		break;
	default:
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	ctx->key_len = len;
	//ctx->need_calc_aes_dkey = 1;
	ctx->need_calc_aes_dkey = 0;

	memcpy(ctx->aes_enc_key, key, AES_KEY_LEN);
	return 0;
}

static void copy_src_to_buf(struct req_progress *p, char *dbuf, int len)
{
	int ret;
	void *sbuf;
	int copied = 0;


	while (1) {
		if (!p->sg_src_left) {
			ret = sg_miter_next(&p->src_sg_it);
			BUG_ON(!ret);
			p->sg_src_left = p->src_sg_it.length;
			p->src_start = 0;
		}

		sbuf = p->src_sg_it.addr + p->src_start;

		if (p->sg_src_left <= len - copied) {
			memcpy(dbuf + copied, sbuf, p->sg_src_left);
			copied += p->sg_src_left;
			p->sg_src_left = 0;
			if (copied >= len)
				break;
		} else {
			int copy_len = len - copied;
			memcpy(dbuf + copied, sbuf, copy_len);
			p->src_start += copy_len;
			p->sg_src_left -= copy_len;
			break;
		}
	}
}

static void setup_data_in(int key_len)
{
	struct req_progress *p = &cpg->p;

	
	int data_in_sram =
	    min(p->hw_nbytes - p->hw_processed_bytes, cpg->max_req_size);

	//printk("\n hw_nbytes = %d hw_p_bytes = %d max_req_size = %d p->crypt_len= %d\n",p->hw_nbytes, p->hw_processed_bytes, cpg->max_req_size, p->crypt_len);

	copy_src_to_buf(p, cpg->in_buf + IN_DATA_P(key_len) + p->crypt_len,
			data_in_sram - p->crypt_len);
	p->crypt_len = data_in_sram;
}

static void cns3xxx_process_current_q(int first_block)
{
	struct ablkcipher_request *req = ablkcipher_request_cast(cpg->cur_req);
	struct cns3xxx_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct cns3xxx_req_ctx *req_ctx = ablkcipher_request_ctx(req);
	struct sec_accel_config op;
	struct accel_config in_op;
	int i;

	switch (req_ctx->op) {
	case COP_AES_ECB:
		op.config = CFG_OP_CRYPT_ONLY | CFG_ENCM_AES | CFG_ENC_MODE_ECB;
		break;
	case COP_AES_CBC:
	default:
		op.config = CFG_OP_CRYPT_ONLY | CFG_ENCM_AES | CFG_ENC_MODE_CBC;
		
		//if (first_block)
		{
			memcpy(cpg->in_buf + IN_DATA_IV_P, req->info, 16);
		}
		break;
	}

        switch (ctx->key_len) {
        case AES_KEYSIZE_128:
                in_op.aes_type = AES_128;

        //printk("AES 128\n");
                break;
        case AES_KEYSIZE_192:
                in_op.aes_type = AES_192;
        //printk("AES 192\n");
                break;
        case AES_KEYSIZE_256:
                in_op.aes_type = AES_256;
        //printk("AES 256\n");
                break;
        }


	if (req_ctx->decrypt) {
		in_op.enc = 0;
		memcpy(cpg->in_buf + IN_DATA_KEY_P, ctx->aes_enc_key,
				ctx->key_len);
	} else {
		in_op.enc = 1;
                memcpy(cpg->in_buf + IN_DATA_KEY_P, ctx->aes_enc_key,
                                ctx->key_len);
	}


	setup_data_in(ctx->key_len);
	in_op.msg_len = cpg->p.crypt_len;

       	in_op.inv.bufcnt = cpg->src_num_sgs;
	for(i=0;i< cpg->src_num_sgs;i++)
	{
		in_op.inv.bufptr[i] = (u32 *)cpg->in_buf;
        	in_op.inv.bufsize[i] = in_op.msg_len;
	}

       	in_op.outv.bufcnt = cpg->dst_num_sgs;
        for(i=0;i< cpg->dst_num_sgs;i++)
        {
                in_op.outv.bufptr[i] = (u32 *)cpg->out_buf;
                in_op.outv.bufsize[i] = in_op.msg_len;
        }

	/* GO */

// dump input data
#if 0
	if (req_ctx->decrypt) {
        	printk("Key Pattern =");
        	for(i=0;i < 48;i++)
                	printk("%2x",cpg->in_buf[i]);
		if(((i+1)%16)==0)
			printk("\n");
	}
#endif


        if (p_KernAes != NULL)
        {
                (*p_KernAes)(n1_list->data, in_op.enc, in_op.aes_type, 0, in_op.msg_len, &in_op.inv, &in_op.outv,crypto_int, NULL); //(void *)csp1_request); 

        }
        else
                printk(KERN_CRIT "kernel_shim: Csp1EncryptAes: symbol_get(n1_EncryptAes) failed\n");

	/*
	 * XXX: add timer if the interrupt does not occur for some mystery
	 * reason
	 */
}

static void cns3xxx_crypto_algo_completion(void)
{
	struct ablkcipher_request *req = ablkcipher_request_cast(cpg->cur_req);
	struct cns3xxx_req_ctx *req_ctx = ablkcipher_request_ctx(req);

	//printk("%s %d\n",__func__,__LINE__);

	sg_miter_stop(&cpg->p.src_sg_it);
	sg_miter_stop(&cpg->p.dst_sg_it);

	if (req_ctx->op != COP_AES_CBC)
		return ;

	memcpy(req->info, cpg->in_buf + IN_DATA_IV_P, 16);
}

static void dequeue_complete_req(void)
{
	struct crypto_async_request *req = cpg->cur_req;
	void *buf;
	int ret;

	cpg->p.hw_processed_bytes += cpg->p.crypt_len;
	if (cpg->p.copy_back) {
		int need_copy_len = cpg->p.crypt_len;
		int sram_offset = 0;
		do {
			int dst_copy;

			if (!cpg->p.sg_dst_left) {
				ret = sg_miter_next(&cpg->p.dst_sg_it);
				BUG_ON(!ret);
				cpg->p.sg_dst_left = cpg->p.dst_sg_it.length;
				cpg->p.dst_start = 0;
			}

			buf = cpg->p.dst_sg_it.addr;
			buf += cpg->p.dst_start;

			dst_copy = min(need_copy_len, cpg->p.sg_dst_left);

			memcpy(buf, cpg->out_buf, dst_copy);
			sram_offset += dst_copy;
			cpg->p.sg_dst_left -= dst_copy;
			need_copy_len -= dst_copy;
			cpg->p.dst_start += dst_copy;
		} while (need_copy_len > 0);
	}

	cpg->p.crypt_len = 0;

	BUG_ON(cpg->eng_st != ENGINE_W_DEQUEUE);
	if (cpg->p.hw_processed_bytes < cpg->p.hw_nbytes) {

		/* process next scatter list entry */
		cpg->eng_st = ENGINE_BUSY;
		cpg->p.process(0);
	} else {
		cpg->p.complete();
		cpg->eng_st = ENGINE_IDLE;
		local_bh_disable();
		req->complete(req, 0);
		local_bh_enable();
	}
}

static int count_sgs(struct scatterlist *sl, unsigned int total_bytes)
{
	int i = 0;
	size_t cur_len;

	while (1) {
		cur_len = sl[i].length;
		++i;
		if (total_bytes > cur_len)
			total_bytes -= cur_len;
		else
			break;
	}

	return i;
}

static void cns3xxx_start_new_crypt_req(struct ablkcipher_request *req)
{
	struct req_progress *p = &cpg->p;
	int num_sgs;

	cpg->cur_req = &req->base;
	memset(p, 0, sizeof(struct req_progress));
	p->hw_nbytes = req->nbytes;
	p->complete = cns3xxx_crypto_algo_completion;
	p->process = cns3xxx_process_current_q;
	p->copy_back = 1;

	num_sgs = count_sgs(req->src, req->nbytes);
	cpg->src_num_sgs = num_sgs;
	sg_miter_start(&p->src_sg_it, req->src, num_sgs, SG_MITER_FROM_SG);
	

	num_sgs = count_sgs(req->dst, req->nbytes);
	cpg->dst_num_sgs = num_sgs;
	sg_miter_start(&p->dst_sg_it, req->dst, num_sgs, SG_MITER_TO_SG);

	//printk("src_num_sgs = %d dst_num_sgs = %d\n", cpg->src_num_sgs, cpg->dst_num_sgs);
	cns3xxx_process_current_q(1);
}

static int queue_manag(void *data)
{
	cpg->eng_st = ENGINE_IDLE;
	do {
		struct crypto_async_request *async_req = NULL;
		struct crypto_async_request *backlog;

		__set_current_state(TASK_INTERRUPTIBLE);

		if (cpg->eng_st == ENGINE_W_DEQUEUE)
			dequeue_complete_req();

		spin_lock_irq(&cpg->lock);
		if (cpg->eng_st == ENGINE_IDLE) {
			backlog = crypto_get_backlog(&cpg->queue);
			async_req = crypto_dequeue_request(&cpg->queue);
			if (async_req) {
				BUG_ON(cpg->eng_st != ENGINE_IDLE);
				cpg->eng_st = ENGINE_BUSY;
			}
		}
		spin_unlock_irq(&cpg->lock);

		if (backlog) {
			backlog->complete(backlog, -EINPROGRESS);
			backlog = NULL;
		}

		if (async_req) {
			/*if (async_req->tfm->__crt_alg->cra_type !=
			    &crypto_ahash_type) {*/
				struct ablkcipher_request *req =
				    container_of(async_req,
						 struct ablkcipher_request,
						 base);
				cns3xxx_start_new_crypt_req(req);
			/*} else {
				struct ahash_request *req =
				    ahash_request_cast(async_req);
				cns3xxx_start_new_hash_req(req);
			}*/
			async_req = NULL;
		}

		schedule();

	} while (!kthread_should_stop());


	return 0;
}

static int cns3xxx_handle_req(struct crypto_async_request *req)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&cpg->lock, flags);
	ret = crypto_enqueue_request(&cpg->queue, req);
	spin_unlock_irqrestore(&cpg->lock, flags);
	wake_up_process(cpg->queue_th);
	return ret;
}

static int cns3xxx_enc_aes_cbc(struct ablkcipher_request *req)
{
	struct cns3xxx_req_ctx *req_ctx = ablkcipher_request_ctx(req);

	req_ctx->op = COP_AES_CBC;
	req_ctx->decrypt = 0;

	return cns3xxx_handle_req(&req->base);
}

static int cns3xxx_dec_aes_cbc(struct ablkcipher_request *req)
{
	//struct cns3xxx_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct cns3xxx_req_ctx *req_ctx = ablkcipher_request_ctx(req);

	req_ctx->op = COP_AES_CBC;
	req_ctx->decrypt = 1;

	//compute_aes_dec_key(ctx);
	return cns3xxx_handle_req(&req->base);
}

static int cns3xxx_cra_init(struct crypto_tfm *tfm)
{
	tfm->crt_ablkcipher.reqsize = sizeof(struct cns3xxx_req_ctx);
	return 0;
}

void crypto_int(int a, void *p)
{
	BUG_ON(cpg->eng_st != ENGINE_BUSY);
	cpg->eng_st = ENGINE_W_DEQUEUE;
	wake_up_process(cpg->queue_th);
}

struct crypto_alg cns3xxx_aes_alg_cbc = {
	.cra_name		= "cbc(aes)",
	.cra_driver_name	= "cns3xxx-cbc-aes",
	.cra_priority	= 300,
	.cra_flags	= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize	= AES_BLOCK_SIZE,
	.cra_ctxsize	= sizeof(struct cns3xxx_ctx),
	.cra_alignmask	= 0,
	.cra_type	= &crypto_ablkcipher_type,
	.cra_module	= THIS_MODULE,
	.cra_init	= cns3xxx_cra_init,
	.cra_u		= {
		.ablkcipher = {
			.ivsize		=	AES_BLOCK_SIZE,
			.min_keysize	=	AES_MIN_KEY_SIZE,
			.max_keysize	=	AES_MAX_KEY_SIZE,
			.setkey		=	cns3xxx_setkey_aes,
			.encrypt	=	cns3xxx_enc_aes_cbc,
			.decrypt	=	cns3xxx_dec_aes_cbc,
		},
	},
};


int 
Csp1ConfigDevice(void)
{
    void * n1_ssl_config_device(void);
    void * (*func)(void);

    func = symbol_get(n1_ssl_config_device);
    if (!func) {
        printk(KERN_CRIT "n1_ssl_config_device: symbol_get failed\n");
        return -1;
    }
	
    n1_list = (struct N1_Dev *)(*func)();

    if (n1_list == NULL) {
        printk(KERN_CRIT "No Cavium devices detected in the system\n");
   	symbol_put(n1_ssl_config_device);
        return -1;
    }
    module_count++;
    if (module_count == 1) {
	if (p_alloc_context == NULL) {
		p_alloc_context = symbol_get(n1_ssl_alloc_context);
	}
	if (p_dealloc_context == NULL) {
		p_dealloc_context = symbol_get(n1_ssl_dealloc_context);
	}
        if (p_KernAes == NULL) {
                p_KernAes = symbol_get(n1_KernAes);
        }
   }
   symbol_put(n1_ssl_config_device);

   return 0;
}

u64 Csp1AllocContext(void)
{
	if (!p_alloc_context)
	{
    		if (n1_list == NULL)
		 {
       	 		printk(KERN_CRIT " Cavium device is not initialized1\n");
			return (u64)0;
		}
		p_alloc_context = symbol_get(n1_ssl_alloc_context);
		if (!p_alloc_context) {
        		printk(KERN_CRIT "kernel_shim: Csp1AllocContext: symbol_get(Csp1AllocContext) failed\n");
			return (u64)0;	
		}
	}

	return (*p_alloc_context)(n1_list->data);
}

void 
Csp1FreeContext(void)
{
	if (!p_dealloc_context)
	{
    		if (n1_list == NULL)
		 {
       	 		printk(KERN_CRIT " Cavium device is not initialized\n");
			return;
		}
		p_dealloc_context = symbol_get(n1_ssl_dealloc_context);
		if (!p_dealloc_context) {
        		printk(KERN_CRIT "kernel_shim: Csp1FreeContext: symbol_get(Csp1FreeContext) failed\n");
			return;	
		}
	}

	(*p_dealloc_context)(n1_list->data, ctx);
}

void
Csp1UnconfigDevice(void)
{

	void * n1_ssl_unconfig_device(void *,u64);
	void * (*func)(void *, u64);	

	Csp1FreeContext();

	func = symbol_get(n1_ssl_unconfig_device);
	if (!func) {
        	printk(KERN_CRIT "Csp1UnConfigDevice: symbol_get failed\n");
				return;
    	}
   
    	(*func)(n1_list->data, 0);

    	symbol_put(n1_ssl_unconfig_device);

	module_count--;
	if(module_count == 0)
	{
    		n1_list = NULL;
		if (p_alloc_context) {
			symbol_put(n1_ssl_alloc_context);
			p_alloc_context = NULL;
		}
		if (p_dealloc_context) {
			symbol_put(n1_ssl_dealloc_context);
			p_dealloc_context = NULL;
		}
		if (p_KernAes) {
			symbol_put(n1_KernAes);
			p_KernAes = NULL;
		}
	}
}



static int cns3xxx_probe(struct platform_device *pdev)
{
	struct crypto_priv *cp;
	int ret;

	Csp1ConfigDevice();
	ctx = Csp1AllocContext();

	if (cpg) {
		printk(KERN_ERR CNS3XXX_CRYPTO "Second crypto dev?\n");
		return -EEXIST;
	}


	cp = kzalloc(sizeof(*cp), GFP_KERNEL);
	if (!cp)
		return -ENOMEM;

	spin_lock_init(&cp->lock);
	crypto_init_queue(&cp->queue, 50);
	
	cp->max_req_size = 1024;

	platform_set_drvdata(pdev, cp);
	cpg = cp;

	cp->queue_th = kthread_run(queue_manag, cp, "cns3xxx_crypto");
	if (IS_ERR(cp->queue_th)) {
		ret = PTR_ERR(cp->queue_th);
		goto err_thread;
	}

	/*ret = request_irq(irq, crypto_int, IRQF_DISABLED, dev_name(&pdev->dev),
			cp);
	if (ret)
		goto err_unmap_sram;*/


	/*ret = crypto_register_alg(&cns3xxx_aes_alg_ecb);
	if (ret)
		goto err_reg;*/

	ret = crypto_register_alg(&cns3xxx_aes_alg_cbc);
	if (ret)
		goto err_unreg_cbc;
/*
	ret = crypto_register_ahash(&cns3xxx_sha1_alg);
	if (ret == 0)
		cpg->has_sha1 = 1;
	else
		printk(KERN_WARNING MV_CESA "Could not register sha1 driver\n");

	ret = crypto_register_ahash(&mv_hmac_sha1_alg);
	if (ret == 0) {
		cpg->has_hmac_sha1 = 1;
	} else {
		printk(KERN_WARNING MV_CESA
		       "Could not register hmac-sha1 driver\n");
	}
*/
	return 0;
err_unreg_cbc:
	crypto_unregister_alg(&cns3xxx_aes_alg_cbc);
err_thread:
	kthread_stop(cp->queue_th);
	iounmap(cp->reg);
	kfree(cp);
	cpg = NULL;
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int cns3xxx_remove(struct platform_device *pdev)
{
	struct crypto_priv *cp = platform_get_drvdata(pdev);

	//crypto_unregister_alg(&cns3xxx_aes_alg_ecb);
	Csp1UnconfigDevice();
	crypto_unregister_alg(&cns3xxx_aes_alg_cbc);
	/*if (cp->has_sha1)
		crypto_unregister_ahash(&cns3xxx_sha1_alg);
	if (cp->has_hmac_sha1)
		crypto_unregister_ahash(&cns3xxx_hmac_sha1_alg);*/
	kthread_stop(cp->queue_th);
	kfree(cp);
	cpg = NULL;
	return 0;
}

static struct platform_driver cns3xxx_crypto = {
	.probe		= cns3xxx_probe,
	.remove		= cns3xxx_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "cns3xxx_crypto",
	},
};
MODULE_ALIAS("platform:cns3xxx_crypto");

static int __init cns3xxx_crypto_init(void)
{
	return platform_driver_register(&cns3xxx_crypto);
}
module_init(cns3xxx_crypto_init);

static void __exit cns3xxx_crypto_exit(void)
{
	platform_driver_unregister(&cns3xxx_crypto);
}
module_exit(cns3xxx_crypto_exit);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("Support for CNS3XXX cryptographic engine");
MODULE_LICENSE("GPL");
