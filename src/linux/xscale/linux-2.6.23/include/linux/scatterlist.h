#ifndef _LINUX_SCATTERLIST_H
#define _LINUX_SCATTERLIST_H

#include <asm/scatterlist.h>
#include <linux/mm.h>
#include <linux/string.h>

static inline void sg_set_buf(struct scatterlist *sg, const void *buf,
			      unsigned int buflen)
{
	sg->page = virt_to_page(buf);
	sg->offset = offset_in_page(buf);
	sg->length = buflen;
}

static inline void sg_init_one(struct scatterlist *sg, const void *buf,
			       unsigned int buflen)
{
	memset(sg, 0, sizeof(*sg));
	sg_set_buf(sg, buf, buflen);
}
static inline void sg_set_page(struct scatterlist *sg, struct page *page,
			       unsigned int len, unsigned int offset)
{
sg->page = page;
sg->offset = offset;
sg->length = len;
}

static inline struct page *sg_page(struct scatterlist *sg)
{
    return sg->page;
}
static inline void sg_init_table(struct scatterlist *sg,int num)
{
    memset(sg,0,sizeof(*sg)*num);
}
#endif /* _LINUX_SCATTERLIST_H */
