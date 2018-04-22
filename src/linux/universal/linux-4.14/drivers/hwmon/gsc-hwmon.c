/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2018 Gateworks Corporation
 *
 * This driver registers Linux HWMON attributes for GSC ADC's
 */
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/mfd/gsc.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>

#include <linux/platform_data/gsc_hwmon.h>

#define GSC_HWMON_MAX_TEMP_CH	16
#define GSC_HWMON_MAX_IN_CH	16

#define GSC_HWMON_RESOLUTION	12
#define GSC_HWMON_VREF		2500

struct gsc_hwmon_data {
	struct gsc_dev *gsc;
	struct device *dev;
	struct gsc_hwmon_platform_data *pdata;
	const struct gsc_hwmon_channel *temp_ch[GSC_HWMON_MAX_TEMP_CH];
	const struct gsc_hwmon_channel *in_ch[GSC_HWMON_MAX_IN_CH];
	u32 temp_config[GSC_HWMON_MAX_TEMP_CH + 1];
	u32 in_config[GSC_HWMON_MAX_IN_CH + 1];
	struct hwmon_channel_info temp_info;
	struct hwmon_channel_info in_info;
	const struct hwmon_channel_info *info[4];
	struct hwmon_chip_info chip;
};

static ssize_t show_pwm_auto_point_temp(struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	struct gsc_hwmon_data *hwmon = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	u8 reg = hwmon->pdata->fan_base + (2 * attr->index);
	u8 regs[2];
	int ret;

	ret = regmap_bulk_read(hwmon->gsc->regmap_hwmon, reg, regs, 2);
	if (ret)
		return ret;

	ret = regs[0] | regs[1] << 8;
	return sprintf(buf, "%d\n", ret * 10);
}

static ssize_t set_pwm_auto_point_temp(struct device *dev,
				       struct device_attribute *devattr,
				       const char *buf, size_t count)
{
	struct gsc_hwmon_data *hwmon = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	u8 reg = hwmon->pdata->fan_base + (2 * attr->index);
	u8 regs[2];
	long temp;
	int err;

	if (kstrtol(buf, 10, &temp))
		return -EINVAL;

	temp = clamp_val(temp, 0, 10000);
	temp = DIV_ROUND_CLOSEST(temp, 10);

	regs[0] = temp & 0xff;
	regs[1] = (temp >> 8) & 0xff;
	err = regmap_bulk_write(hwmon->gsc->regmap_hwmon, reg, regs, 2);
	if (err)
		return err;

	return count;
}

static ssize_t show_pwm_auto_point_pwm(struct device *dev,
				       struct device_attribute *devattr,
				       char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);

	return sprintf(buf, "%d\n", 255 * (50 + (attr->index * 10)) / 100);
}

static SENSOR_DEVICE_ATTR(pwm1_auto_point1_pwm, S_IRUGO,
			  show_pwm_auto_point_pwm, NULL, 0);
static SENSOR_DEVICE_ATTR(pwm1_auto_point1_temp, S_IRUGO | S_IWUSR,
			  show_pwm_auto_point_temp, set_pwm_auto_point_temp, 0);

static SENSOR_DEVICE_ATTR(pwm1_auto_point2_pwm, S_IRUGO,
			  show_pwm_auto_point_pwm, NULL, 1);
static SENSOR_DEVICE_ATTR(pwm1_auto_point2_temp, S_IRUGO | S_IWUSR,
			  show_pwm_auto_point_temp, set_pwm_auto_point_temp, 1);

static SENSOR_DEVICE_ATTR(pwm1_auto_point3_pwm, S_IRUGO,
			  show_pwm_auto_point_pwm, NULL, 2);
static SENSOR_DEVICE_ATTR(pwm1_auto_point3_temp, S_IRUGO | S_IWUSR,
			  show_pwm_auto_point_temp, set_pwm_auto_point_temp, 2);

static SENSOR_DEVICE_ATTR(pwm1_auto_point4_pwm, S_IRUGO,
			  show_pwm_auto_point_pwm, NULL, 3);
static SENSOR_DEVICE_ATTR(pwm1_auto_point4_temp, S_IRUGO | S_IWUSR,
			  show_pwm_auto_point_temp, set_pwm_auto_point_temp, 3);

static SENSOR_DEVICE_ATTR(pwm1_auto_point5_pwm, S_IRUGO,
			  show_pwm_auto_point_pwm, NULL, 4);
static SENSOR_DEVICE_ATTR(pwm1_auto_point5_temp, S_IRUGO | S_IWUSR,
			  show_pwm_auto_point_temp, set_pwm_auto_point_temp, 4);

static SENSOR_DEVICE_ATTR(pwm1_auto_point6_pwm, S_IRUGO,
			  show_pwm_auto_point_pwm, NULL, 5);
static SENSOR_DEVICE_ATTR(pwm1_auto_point6_temp, S_IRUGO | S_IWUSR,
			  show_pwm_auto_point_temp, set_pwm_auto_point_temp, 5);

static struct attribute *gsc_hwmon_attributes[] = {
	&sensor_dev_attr_pwm1_auto_point1_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point1_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point2_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point2_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point3_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point3_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point4_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point4_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point5_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point5_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point6_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point6_temp.dev_attr.attr,
	NULL
};

static const struct attribute_group gsc_hwmon_group = {
	.attrs = gsc_hwmon_attributes,
};
__ATTRIBUTE_GROUPS(gsc_hwmon);

static int
gsc_hwmon_read(struct device *dev, enum hwmon_sensor_types type, u32 attr,
	       int channel, long *val)
{
	struct gsc_hwmon_data *hwmon = dev_get_drvdata(dev);
	const struct gsc_hwmon_channel *ch;
	int sz, ret;
	u8 buf[3];

	switch (type) {
	case hwmon_in:
		ch = hwmon->in_ch[channel];
		break;
	case hwmon_temp:
		ch = hwmon->temp_ch[channel];
		break;
	default:
		return -EOPNOTSUPP;
	}

	if (hwmon->gsc->type == gsc_v3)
		sz = 2;
	else
		sz = (ch->type == type_voltage) ? 3 : 2;
	ret = regmap_bulk_read(hwmon->gsc->regmap_hwmon, ch->reg, buf, sz);
	if (ret)
		return ret;

	*val = 0;
	while (sz-- > 0)
		*val |= (buf[sz] << (8 * sz));

	switch (ch->type) {
	case type_temperature:
		if (*val > 0x8000)
			*val -= 0xffff;
		break;
	case type_voltage_raw:
		*val = clamp_val(*val, 0, BIT(GSC_HWMON_RESOLUTION));
		/* scale based on ref voltage and resolution */
		*val *= GSC_HWMON_VREF;
		*val /= BIT(GSC_HWMON_RESOLUTION);
		/* scale based on optional voltage divider */
		if (ch->vdiv[0] && ch->vdiv[1]) {
			*val *= (ch->vdiv[0] + ch->vdiv[1]);
			*val /= ch->vdiv[1];
		}
		/* adjust by offset */
		*val += ch->voffset;
		break;
	case type_voltage:
		/* no adjustment needed */
		break;
	}

	return 0;
}

static int
gsc_hwmon_read_string(struct device *dev, enum hwmon_sensor_types type,
		      u32 attr, int channel, const char **buf)
{
	struct gsc_hwmon_data *hwmon = dev_get_drvdata(dev);

	switch (type) {
	case hwmon_in:
		*buf = hwmon->in_ch[channel]->name;
		break;
	case hwmon_temp:
		*buf = hwmon->temp_ch[channel]->name;
		break;
	default:
		return -ENOTSUPP;
	}

	return 0;
}

static umode_t
gsc_hwmon_is_visible(const void *_data, enum hwmon_sensor_types type, u32 attr,
		     int ch)
{
	switch (type) {
	case hwmon_temp:
		return S_IRUGO;
	case hwmon_in:
		return S_IRUGO;
		break;
	default:
		return -EOPNOTSUPP;
	}
}

static const struct hwmon_ops gsc_hwmon_ops = {
	.is_visible = gsc_hwmon_is_visible,
	.read = gsc_hwmon_read,
	.read_string = gsc_hwmon_read_string,
};

static struct gsc_hwmon_platform_data *
gsc_hwmon_get_devtree_pdata(struct device *dev)
{
	struct gsc_hwmon_platform_data *pdata;
	struct gsc_hwmon_channel *ch;
	struct fwnode_handle *child;
	const char *type;
	int nchannels;

	nchannels = device_get_child_node_count(dev);
	if (nchannels == 0)
		return ERR_PTR(-ENODEV);

	pdata = devm_kzalloc(dev,
			     sizeof(*pdata) + nchannels * sizeof(*ch),
			     GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);
	ch = (struct gsc_hwmon_channel *)(pdata + 1);
	pdata->channels = ch;
	pdata->nchannels = nchannels;

	device_property_read_u32(dev, "gw,fan-base", &pdata->fan_base);

	/* allocate structures for channels and count instances of each type */
	device_for_each_child_node(dev, child) {
		if (fwnode_property_read_string(child, "label", &ch->name)) {
			dev_err(dev, "channel without label\n");
			fwnode_handle_put(child);
			return ERR_PTR(-EINVAL);
		}
		if (fwnode_property_read_u32(child, "reg", &ch->reg)) {
			dev_err(dev, "channel without reg\n");
			fwnode_handle_put(child);
			return ERR_PTR(-EINVAL);
		}
		if (fwnode_property_read_string(child, "type", &type)) {
			dev_err(dev, "channel without type\n");
			fwnode_handle_put(child);
			return ERR_PTR(-EINVAL);
		}
		if (!strcasecmp(type, "gw,hwmon-temperature"))
			ch->type = type_temperature;
		else if (!strcasecmp(type, "gw,hwmon-voltage"))
			ch->type = type_voltage;
		else if (!strcasecmp(type, "gw,hwmon-voltage-raw"))
			ch->type = type_voltage_raw;
		else {
			dev_err(dev, "channel without type\n");
			fwnode_handle_put(child);
			return ERR_PTR(-EINVAL);
		}

		fwnode_property_read_u32(child, "gw,voltage-offset",
			&ch->voffset);
		fwnode_property_read_u32_array(child, "gw,voltage-divider",
			ch->vdiv, ARRAY_SIZE(ch->vdiv));
		ch++;
	}

	return pdata;
}

static int gsc_hwmon_probe(struct platform_device *pdev)
{
	struct gsc_dev *gsc = dev_get_drvdata(pdev->dev.parent);
	struct device *dev = &pdev->dev;
	struct gsc_hwmon_platform_data *pdata = dev_get_platdata(dev);
	struct gsc_hwmon_data *hwmon;
	const struct attribute_group **groups;
	int i, i_in, i_temp;

	if (!pdata) {
		pdata = gsc_hwmon_get_devtree_pdata(dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	}

	hwmon = devm_kzalloc(dev, sizeof(*hwmon), GFP_KERNEL);
	if (!hwmon)
		return -ENOMEM;
	hwmon->gsc = gsc;
	hwmon->pdata = pdata;

	for (i = 0, i_in = 0, i_temp = 0; i < hwmon->pdata->nchannels; i++) {
		const struct gsc_hwmon_channel *ch = &pdata->channels[i];

		switch (ch->type) {
		case type_temperature:
			if (i_temp == GSC_HWMON_MAX_TEMP_CH) {
				dev_err(gsc->dev, "too many temp channels\n");
				return -EINVAL;
			}
			hwmon->temp_ch[i_temp] = ch;
			hwmon->temp_config[i_temp] = HWMON_T_INPUT |
						     HWMON_T_LABEL;
			i_temp++;
			break;
		case type_voltage:
		case type_voltage_raw:
			if (i_in == GSC_HWMON_MAX_IN_CH) {
				dev_err(gsc->dev, "too many input channels\n");
				return -EINVAL;
			}
			hwmon->in_ch[i_in] = ch;
			hwmon->in_config[i_in] =
				HWMON_I_INPUT | HWMON_I_LABEL;
			i_in++;
			break;
		default:
			dev_err(gsc->dev, "invalid type: %d\n", ch->type);
			return -EINVAL;
		}
	}

	/* setup config structures */
	hwmon->chip.ops = &gsc_hwmon_ops;
	hwmon->chip.info = hwmon->info;
	hwmon->info[0] = &hwmon->temp_info;
	hwmon->info[1] = &hwmon->in_info;
	hwmon->temp_info.type = hwmon_temp;
	hwmon->temp_info.config = hwmon->temp_config;
	hwmon->in_info.type = hwmon_in;
	hwmon->in_info.config = hwmon->in_config;

	groups = pdata->fan_base ? gsc_hwmon_groups : NULL;
	hwmon->dev = devm_hwmon_device_register_with_info(dev,
							  KBUILD_MODNAME, hwmon,
							  &hwmon->chip, groups);
	return PTR_ERR_OR_ZERO(hwmon->dev);
}

static const struct of_device_id gsc_hwmon_of_match[] = {
	{ .compatible = "gw,gsc-hwmon", },
	{}
};

static struct platform_driver gsc_hwmon_driver = {
	.driver = {
		.name = KBUILD_MODNAME,
		.of_match_table = gsc_hwmon_of_match,
	},
	.probe = gsc_hwmon_probe,
};

module_platform_driver(gsc_hwmon_driver);

MODULE_AUTHOR("Tim Harvey <tharvey@gateworks.com>");
MODULE_DESCRIPTION("GSC hardware monitor driver");
MODULE_LICENSE("GPL v2");
