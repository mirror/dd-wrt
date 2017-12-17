/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

 /*
 * Qualcomm TSENS Header file
 *
 */

#ifndef __QCOM_TSENS_H
#define __QCOM_TSENS_H

#define TSENS_MAX_SENSORS	11

struct tsens_platform_data {
	uint32_t	slope[TSENS_MAX_SENSORS];
	int		tsens_factor;
	uint32_t	tsens_num_sensor;
	int		tsens_irq;
	void		*tsens_addr;
	void		*tsens_calib_addr;
	struct resource	*res_tsens_mem;
	struct resource	*res_calib_mem;
};

struct tsens_device {
	uint32_t	sensor_num;
};

int32_t tsens_get_temp(struct tsens_device *dev, unsigned long *temp);
int msm_tsens_early_init(struct tsens_platform_data *pdata);

#endif /*QCOM_TSENS_H */
