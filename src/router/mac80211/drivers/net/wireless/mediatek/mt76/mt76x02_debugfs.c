// SPDX-License-Identifier: ISC
/*
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 */

#include <linux/debugfs.h>
#include "mt76x02.h"

static int
mt76x02_ampdu_stat_show(struct seq_file *file, void *data)
{
	struct mt76x02_dev *dev = file->private;
	int i, j;

	for (i = 0; i < 4; i++) {
		seq_puts(file, "Length: ");
		for (j = 0; j < 8; j++)
			seq_printf(file, "%8d | ", i * 8 + j + 1);
		seq_puts(file, "\n");
		seq_puts(file, "Count:  ");
		for (j = 0; j < 8; j++)
			seq_printf(file, "%8d | ",
				   dev->mphy.aggr_stats[i * 8 + j]);
		seq_puts(file, "\n");
		seq_puts(file, "--------");
		for (j = 0; j < 8; j++)
			seq_puts(file, "-----------");
		seq_puts(file, "\n");
	}

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt76x02_ampdu_stat);

static int read_txpower(struct seq_file *file, void *data)
{
	struct mt76x02_dev *dev = dev_get_drvdata(file->private);

	seq_printf(file, "Target power: %d\n", dev->target_power);

	mt76_seq_puts_array(file, "Delta", dev->target_power_delta,
			    ARRAY_SIZE(dev->target_power_delta));
	return 0;
}

static int
mt76x02_dfs_stat_show(struct seq_file *file, void *data)
{
	struct mt76x02_dev *dev = file->private;
	struct mt76x02_dfs_pattern_detector *dfs_pd = &dev->dfs_pd;
	int i;

	seq_printf(file, "allocated sequences:\t%d\n",
		   dfs_pd->seq_stats.seq_pool_len);
	seq_printf(file, "used sequences:\t\t%d\n",
		   dfs_pd->seq_stats.seq_len);
	seq_puts(file, "\n");

	for (i = 0; i < MT_DFS_NUM_ENGINES; i++) {
		seq_printf(file, "engine: %d\n", i);
		seq_printf(file, "  hw pattern detected:\t%d\n",
			   dfs_pd->stats[i].hw_pattern);
		seq_printf(file, "  hw pulse discarded:\t%d\n",
			   dfs_pd->stats[i].hw_pulse_discarded);
		seq_printf(file, "  sw pattern detected:\t%d\n",
			   dfs_pd->stats[i].sw_pattern);
	}

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt76x02_dfs_stat);

static int read_agc(struct seq_file *file, void *data)
{
	struct mt76x02_dev *dev = dev_get_drvdata(file->private);

	seq_printf(file, "avg_rssi: %d\n", dev->cal.avg_rssi_all);
	seq_printf(file, "low_gain: %d\n", dev->cal.low_gain);
	seq_printf(file, "false_cca: %d\n", dev->cal.false_cca);
	seq_printf(file, "agc_gain_adjust: %d\n", dev->cal.agc_gain_adjust);

	return 0;
}

static int
mt76_edcca_set(void *data, u64 val)
{
	struct mt76x02_dev *dev = data;
	enum nl80211_dfs_regions region = dev->mt76.region;

	mutex_lock(&dev->mt76.mutex);

	dev->ed_monitor_enabled = !!val;
	dev->ed_monitor = dev->ed_monitor_enabled &&
			  region == NL80211_DFS_ETSI;
	mt76x02_edcca_init(dev);

	mutex_unlock(&dev->mt76.mutex);

	return 0;
}

static int
mt76_edcca_get(void *data, u64 *val)
{
	struct mt76x02_dev *dev = data;

	*val = dev->ed_monitor_enabled;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_edcca, mt76_edcca_get, mt76_edcca_set,
			 "%lld\n");

static ssize_t read_file_turboqam(struct file *file, char __user *user_buf,
			     size_t count, loff_t *ppos)
{
	struct mt76x02_dev *dev = file->private_data;
	struct mt76_dev *mt76dev = &dev->mt76;
	char buf[32];
	unsigned int len;

	len = sprintf(buf, "0x%08x\n", mt76dev->turboqam);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

int
mt76_init_sband_2g(struct mt76_phy *phy, struct ieee80211_rate *rates,
		   int n_rates, bool vht);

static ssize_t write_file_turboqam(struct file *file, const char __user *user_buf,
			     size_t count, loff_t *ppos)
{
	struct mt76x02_dev *dev = file->private_data;
	struct mt76_dev *mt76dev = &dev->mt76;
	struct mt76_phy *mt76phy = &mt76dev->phy;
	unsigned long turboqam;
	char buf[32];
	ssize_t len;

	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	if (kstrtoul(buf, 0, &turboqam))
		return -EINVAL;
		
       mt76dev->turboqam = turboqam;
       if (mt76phy->cap.has_2ghz) {
		if (turboqam)
			mt76_init_sband_2g(mt76phy, mt76x02_rates, ARRAY_SIZE(mt76x02_rates), 1);
		else
			mt76_init_sband_2g(mt76phy, mt76x02_rates, ARRAY_SIZE(mt76x02_rates), 0);
	}
	return count;
}

static const struct file_operations fops_turboqam = {
	.read = read_file_turboqam,
	.write = write_file_turboqam,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static int mt76x02_read_rate_txpower(struct seq_file *s, void *data)
{
	struct mt76x02_dev *dev = dev_get_drvdata(s->private);

	mt76_seq_puts_array(s, "CCK", dev->rate_power.cck,
			    ARRAY_SIZE(dev->rate_power.cck));
	mt76_seq_puts_array(s, "OFDM", dev->rate_power.ofdm,
			    ARRAY_SIZE(dev->rate_power.ofdm));
	mt76_seq_puts_array(s, "HT", dev->rate_power.ht,
			    ARRAY_SIZE(dev->rate_power.ht));
	mt76_seq_puts_array(s, "VHT", dev->rate_power.vht,
			    ARRAY_SIZE(dev->rate_power.vht));
	return 0;
}

void mt76x02_init_debugfs(struct mt76x02_dev *dev)
{
	struct dentry *dir;

	dir = mt76_register_debugfs(&dev->mt76);
	if (!dir)
		return;

	debugfs_create_devm_seqfile(dev->mt76.dev, "xmit-queues", dir,
				    mt76_queues_read);
	debugfs_create_u8("temperature", 0400, dir, &dev->cal.temp);
	debugfs_create_bool("tpc", 0600, dir, &dev->enable_tpc);

	debugfs_create_file("edcca", 0600, dir, dev, &fops_edcca);
	debugfs_create_file("ampdu_stat", 0400, dir, dev, &mt76x02_ampdu_stat_fops);
	debugfs_create_file("dfs_stats", 0400, dir, dev, &mt76x02_dfs_stat_fops);
	debugfs_create_devm_seqfile(dev->mt76.dev, "txpower", dir,
				    read_txpower);

	debugfs_create_devm_seqfile(dev->mt76.dev, "rate_txpower", dir,
				    mt76x02_read_rate_txpower);
	debugfs_create_devm_seqfile(dev->mt76.dev, "agc", dir, read_agc);

	debugfs_create_u32("tx_hang_reset", 0400, dir, &dev->tx_hang_reset);
	debugfs_create_file("turboqam", S_IRUSR | S_IWUSR, dir,
			    dev, &fops_turboqam);
}
EXPORT_SYMBOL_GPL(mt76x02_init_debugfs);
