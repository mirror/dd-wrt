#include <linux/module.h>
#include <linux/random.h>
#include <linux/time.h>
#include <linux/export.h>

#include "pdwext.h"
#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)

#ifdef CPTCFG_MAC80211_TDMA_MESH
#include "mesh.h"

void tdma_mesh_store_path(struct ieee80211_if_tdma *tdma, struct p_originator *o, u16 rval)
{
	memcpy(tdma->mesh_tx[tdma->tmd_pointer].da, o->mac, ETH_ALEN);
	memcpy(tdma->mesh_tx[tdma->tmd_pointer].ra, o->relay, ETH_ALEN);
	tdma->mesh_tx[tdma->tmd_pointer].reach_val = rval;
	tdma->tmd_pointer++;
	if (tdma->tmd_pointer >= TMD_MAX_RECORDS)
		tdma->tmd_pointer = 0;
}

static ssize_t __fmt_path(const struct tmd_record *o, char *buf, int buflen)
{

	return scnprintf(buf, buflen, "%pM %pM %d\n", o->da, o->ra, (int)o->reach_val);
}

ssize_t tdma_mesh_fmt_path(const struct ieee80211_if_tdma *tdma, char *buf, int buflen)
{
	char *p = buf;
	ssize_t sz = 0, sz1;
	int i;

	if (tdma->tmd_pointer > 0) {
		for (i = tdma->tmd_pointer - 1; i >= 0; i--) {
			sz1 = __fmt_path(&tdma->mesh_tx[i], p, buflen);
			buflen -= sz1;
			sz += sz1;
			p += sz1;
		}
	}
	for (i = TMD_MAX_RECORDS - 1; i >= tdma->tmd_pointer; i--) {
		sz1 = __fmt_path(&tdma->mesh_tx[i], p, buflen);
		buflen -= sz1;
		sz += sz1;
		p += sz1;
	}
	return sz;
}

EXPORT_SYMBOL(tdma_mesh_fmt_path);

u16 tdma_mesh_calc_reachability(struct p_originator *o, unsigned intval)
{
	return mm_calc_reachability(o, intval);
}

EXPORT_SYMBOL(tdma_mesh_calc_reachability);

#endif
