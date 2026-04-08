// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024 Christian Marangi <ansuelsmth@gmail.com>
 *
 * This borrow some parse logic from u-boot-env.
 */
#include <linux/nvmem-consumer.h>
#include <linux/nvmem-provider.h>
#include <linux/of.h>
#include <linux/slab.h>

struct ascii_env_match_data {
	const char delim;
};

/*
 * Parse a buffer as an ASCII text with name delimiter value and each pattern separated
 * with a new line char '\n'
 * Example: (delimiter '=')
 *   name=value\nname2=value2\n
 *   2 Cell:
 *   - name: value
 *   - name2: value2
 */
static int ascii_env_parse_cells(struct device *dev, struct nvmem_device *nvmem, uint8_t *buf,
				 size_t data_len, const char delim)
{
	char *var, *value, *eq, *lf, *cr;
	char *data = buf;
	uint incr = 0;

	/*
	 * Warning the inner loop take care of replacing '\n'
	 * with '\0', hence we can use strlen on value.
	 */
	for (var = data; var < data + data_len && *var;
	     var = value + strlen(value) + incr) {
		struct nvmem_cell_info info = {};
		struct device_node *child;
		const char *label;
		incr = 0;

		eq = strchr(var, delim);
		if (!eq)
			break;
		*eq = '\0';
		value = eq + 1;

		/* Replace '\n' with '\0' to use strlen for value */
		lf = strchr(value, '\n');
		if (!lf)
			break;
		*lf = '\0';
		incr++;

		/* For CRLF based env, replace '\r' with '\0' too to use strlen
		 * for value, and increment var by one in loop for next variable */
		cr = strchr(value, '\r');
		if (cr) {
			*cr = '\0';
			incr++;
		}

		info.name = devm_kstrdup(dev, var, GFP_KERNEL);
		if (!info.name)
			return -ENOMEM;
		info.offset = value - data;
		info.bytes = strlen(value);
		info.np = of_get_child_by_name(dev->of_node, info.name);
		for_each_child_of_node(dev->of_node, child) {
			if (!of_property_read_string(child, "label", &label) &&
			    !strncmp(info.name, label, info.bytes))
			    	info.np = child;
			else if (of_node_name_eq(child, info.name))
				info.np = child;
		}

		nvmem_layout_parse_mac_base(&info);

		nvmem_add_one_cell(nvmem, &info);
	}

	return 0;
}

static int ascii_env_add_cells(struct nvmem_layout *layout)
{
	struct nvmem_device *nvmem = layout->nvmem;
	const struct ascii_env_match_data *data;
	struct device *dev = &layout->dev;
	size_t dev_size;
	uint8_t *buf;
	int bytes;
	int ret;

	/* Get the delimiter for name value pattern */
	data = device_get_match_data(dev);

	dev_size = nvmem_dev_size(nvmem);

	buf = kzalloc(dev_size, GFP_KERNEL);
	if (!buf) {
		ret = -ENOMEM;
		goto err_out;
	}

	bytes = nvmem_device_read(nvmem, 0, dev_size, buf);
	if (bytes < 0) {
		ret = bytes;
		goto err_kfree;
	} else if (bytes != dev_size) {
		ret = -EIO;
		goto err_kfree;
	}

	buf[dev_size - 1] = '\0';
	ret = ascii_env_parse_cells(dev, nvmem, buf, dev_size, data->delim);

err_kfree:
	kfree(buf);
err_out:
	return ret;
}

static int ascii_env_probe(struct nvmem_layout *layout)
{
	layout->add_cells = ascii_env_add_cells;

	return nvmem_layout_register(layout);
}

static void ascii_env_remove(struct nvmem_layout *layout)
{
	nvmem_layout_unregister(layout);
}

static const struct ascii_env_match_data ascii_env_eq = {
	.delim = '=',
};

static const struct of_device_id ascii_env_of_match_table[] = {
	{ .compatible = "ascii-eq-delim-env", .data = &ascii_env_eq, },
	{},
};

static struct nvmem_layout_driver ascii_env_layout = {
	.driver = {
		.name = "ascii-env-layout",
		.of_match_table = ascii_env_of_match_table,
	},
	.probe = ascii_env_probe,
	.remove = ascii_env_remove,
};
module_nvmem_layout_driver(ascii_env_layout);

MODULE_AUTHOR("Christian Marangi <ansuelsmth@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(of, ascii_env_of_match_table);
MODULE_DESCRIPTION("NVMEM layout driver for ASCII environment variables");
