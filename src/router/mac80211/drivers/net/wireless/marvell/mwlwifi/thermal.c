/*
 * Copyright (C) 2006-2018, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

/* Description:  This file implements thermal framework related functions. */

#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/thermal.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include "sysadpt.h"
#include "core.h"
#include "hif/fwcmd.h"
#include "thermal.h"

static int
mwl_thermal_get_max_throttle_state(struct thermal_cooling_device *cdev,
				   unsigned long *state)
{
	*state = SYSADPT_THERMAL_THROTTLE_MAX;

	return 0;
}

static int
mwl_thermal_get_cur_throttle_state(struct thermal_cooling_device *cdev,
				   unsigned long *state)
{
	struct mwl_priv *priv = cdev->devdata;

	*state = priv->throttle_state;

	return 0;
}

static int
mwl_thermal_set_cur_throttle_state(struct thermal_cooling_device *cdev,
				   unsigned long throttle_state)
{
	struct mwl_priv *priv = cdev->devdata;

	if (throttle_state > SYSADPT_THERMAL_THROTTLE_MAX) {
		wiphy_warn(priv->hw->wiphy,
			   "throttle state %ld is exceeding the limit %d\n",
			   throttle_state, SYSADPT_THERMAL_THROTTLE_MAX);
		return -EINVAL;
	}
	priv->throttle_state = throttle_state;
	mwl_thermal_set_throttling(priv);

	return 0;
}

static struct thermal_cooling_device_ops mwl_thermal_ops = {
	.get_max_state = mwl_thermal_get_max_throttle_state,
	.get_cur_state = mwl_thermal_get_cur_throttle_state,
	.set_cur_state = mwl_thermal_set_cur_throttle_state,
};

static ssize_t mwl_thermal_show_temp(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct mwl_priv *priv = dev_get_drvdata(dev);
	int ret, temperature;

	ret = mwl_fwcmd_get_temp(priv->hw, &priv->temperature);
	if (ret) {
		wiphy_warn(priv->hw->wiphy, "failed: can't get temperature\n");
		goto out;
	}

	temperature = priv->temperature;
	if (priv->chip_type == MWL8864) {
		/* unit is in fahrenheit for this chipset */
		temperature -= 32;
		temperature *= 10;
		temperature /= 18;
		ret = snprintf(buf, PAGE_SIZE, "%d\n", temperature * 1000);
	} else {
		ret = snprintf(buf, PAGE_SIZE, "%d\n", temperature * 100);
	}

	/* display in millidegree celcius */
out:
	return ret;
}

static SENSOR_DEVICE_ATTR(temp1_input, 0444, mwl_thermal_show_temp,
			  NULL, 0);

static struct attribute *mwl_hwmon_attrs[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	NULL,
};
ATTRIBUTE_GROUPS(mwl_hwmon);

void mwl_thermal_set_throttling(struct mwl_priv *priv)
{
	u32 period, duration, enabled;
	int ret;

	period = priv->quiet_period;
	duration = (period * priv->throttle_state) / 100;
	enabled = duration ? 1 : 0;

	ret = mwl_fwcmd_quiet_mode(priv->hw, enabled, period,
				   duration, SYSADPT_QUIET_START_OFFSET);
	if (ret) {
		wiphy_warn(priv->hw->wiphy,
			   "failed: period %u duarion %u enabled %u ret %d\n",
			    period, duration, enabled, ret);
	}
}

int mwl_thermal_register(struct mwl_priv *priv)
{
	struct thermal_cooling_device *cdev;
	struct device *hwmon_dev;
	int ret;

	cdev = thermal_cooling_device_register("mwlwifi_thermal", priv,
					       &mwl_thermal_ops);
	if (IS_ERR(cdev)) {
		wiphy_err(priv->hw->wiphy,
			  "failed to setup thermal device result: %ld\n",
			  PTR_ERR(cdev));
		return -EINVAL;
	}

	ret = sysfs_create_link(&priv->dev->kobj, &cdev->device.kobj,
				"cooling_device");
	if (ret) {
		wiphy_err(priv->hw->wiphy,
			  "failed to create cooling device symlink\n");
		goto err_cooling_destroy;
	}

	priv->cdev = cdev;
	priv->quiet_period = SYSADPT_QUIET_PERIOD_DEFAULT;

	if (!IS_ENABLED(CONFIG_HWMON))
		return 0;

	hwmon_dev =
		devm_hwmon_device_register_with_groups(priv->dev,
						       "mwlwifi_hwmon", priv,
						       mwl_hwmon_groups);
	if (IS_ERR(hwmon_dev)) {
		wiphy_err(priv->hw->wiphy,
			  "failed to register hwmon device: %ld\n",
			  PTR_ERR(hwmon_dev));
		ret = -EINVAL;
		goto err_remove_link;
	}

	return 0;

err_remove_link:
	sysfs_remove_link(&priv->dev->kobj, "cooling_device");
err_cooling_destroy:
	thermal_cooling_device_unregister(cdev);

	return ret;
}

void mwl_thermal_unregister(struct mwl_priv *priv)
{
	if (priv->chip_type != MWL8897)
		return;

	sysfs_remove_link(&priv->dev->kobj, "cooling_device");
	thermal_cooling_device_unregister(priv->cdev);
}
