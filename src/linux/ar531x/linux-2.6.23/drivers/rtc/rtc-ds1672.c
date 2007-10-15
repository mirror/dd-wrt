/*
 * An rtc/i2c driver for the Dallas DS1672
 * Copyright 2005 Alessandro Zummo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/rtc.h>

#define DRV_VERSION "0.1"

/* Addresses to scan: none. This chip cannot be detected. */
static unsigned short normal_i2c[] = { 0x68, I2C_CLIENT_END };

/* Insmod parameters */
I2C_CLIENT_INSMOD;
I2C_CLIENT_MODULE_PARM(hctosys,
	"Set the system time from the hardware clock upon initialization");

/* Registers */

#define DS1672_REG_CNT_BASE	0
#define DS1672_REG_CONTROL	4
#define DS1672_REG_TRICKLE	5


/* Prototypes */
static int ds1672_probe(struct i2c_adapter *adapter, int address, int kind);

/*
 * In the routines that deal directly with the ds1672 hardware, we use
 * rtc_time -- month 0-11, hour 0-23, yr = calendar year-epoch
 * Epoch is initialized as 2000. Time is set to UTC.
 */
static int ds1672_get_datetime(struct i2c_client *client, struct rtc_time *tm, unsigned char reg_base)
{
  unsigned char dt_addr[2] = { 0, reg_base };
	unsigned long time;
  unsigned char buf[8];

  struct i2c_msg msgs[] = {
    { client->addr, 0, 1, dt_addr },  /* setup read ptr */
    { client->addr, I2C_M_RD, 4, buf }, /* read date */
  };

  /* read date registers */
  if ((i2c_transfer(client->adapter, &msgs[0], 2)) != 2) {
    dev_err(&client->dev, "%s: read error\n", __FUNCTION__);
    return -EIO;
  }
	time = buf[3];
  time <<= 8;
  time += buf[2];
  time <<= 8;
  time += buf[1];
  time <<= 8;
  time += buf[0];

	rtc_time_to_tm(time, tm);

  return 0;
}

static int ds1672_set_mmss(struct i2c_client *client, unsigned long secs)
{
	unsigned char buf[5];
	int result = 0;

  buf[0] = (secs & 0xff);
  secs >>= 8;
  buf[1] = (secs & 0xff);
  secs >>= 8;
  buf[2] = (secs & 0xff);
  secs >>= 8;
  buf[3] = (secs & 0xff);

	int i;
  for (i = 0; i < 4; i++) {
    if (i2c_smbus_write_byte_data(client, i, buf[i]) < 0) {
      result = -EIO;
      break;
    }
  }

	return (result);
}

static int ds1672_set_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	unsigned long secs;
/*
	dev_dbg(&client->dev,
		"%s: secs=%d, mins=%d, hours=%d, ",
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__FUNCTION__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);
*/
	rtc_tm_to_time(tm, &secs);

	return ds1672_set_mmss(client, secs);
}

static int ds1672_hctosys(struct i2c_client *client)
{
	int err;
	struct rtc_time tm;
	struct timespec tv;

	if ((err = ds1672_get_datetime(client, &tm, 0)) != 0)
		return err;

	/* IMPORTANT: the RTC only stores whole seconds. It is arbitrary
	 * whether it stores the most close value or the value with partial
	 * seconds truncated. However, it is important that we use it to store
	 * the truncated value. This is because otherwise it is necessary,
	 * in an rtc sync function, to read both xtime.tv_sec and
	 * xtime.tv_nsec. On some processors (i.e. ARM), an atomic read
	 * of >32bits is not possible. So storing the most close value would
	 * slow down the sync API. So here we have the truncated value and
	 * the best guess is to add 0.5s.
	 */

	tv.tv_nsec = NSEC_PER_SEC >> 1;

	rtc_tm_to_time(&tm, &tv.tv_sec);

	do_settimeofday(&tv);

	dev_info(&client->dev,
		"setting the system clock to %d-%02d-%02d %02d:%02d:%02d\n",
		tm.tm_year + 1900, tm.tm_mon + 1,
		tm.tm_mday, tm.tm_hour, tm.tm_min,
		tm.tm_sec);

	return 0;
}

static int ds1672_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	return ds1672_get_datetime(to_i2c_client(dev), tm, 0);
}

static int ds1672_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	return ds1672_set_datetime(to_i2c_client(dev), tm);
}

static int ds1672_rtc_set_mmss(struct device *dev, unsigned long secs)
{
	return ds1672_set_mmss(to_i2c_client(dev), secs);
}

static struct rtc_class_ops ds1672_rtc_ops = {
	.read_time = ds1672_rtc_read_time,
	.set_time = ds1672_rtc_set_time,
	.set_mmss = ds1672_rtc_set_mmss,
};

static int ds1672_attach(struct i2c_adapter *adapter)
{
	dev_dbg(&adapter->dev, "%s\n", __FUNCTION__);

	return i2c_probe(adapter, &addr_data, ds1672_probe);
}

static int ds1672_detach(struct i2c_client *client)
{
	int err;
	struct rtc_device *rtc = i2c_get_clientdata(client);

	dev_dbg(&client->dev, "%s\n", __FUNCTION__);

 	if (rtc)
		rtc_device_unregister(rtc);

	if ((err = i2c_detach_client(client)))
		return err;

	kfree(client);

	return 0;
}

static struct i2c_driver ds1672_driver = {
	.driver		= {
	.owner		= THIS_MODULE,
	.name		= "ds1672",
	},
//	.flags		= I2C_DF_NOTIFY,
	.attach_adapter = &ds1672_attach,
	.detach_client	= &ds1672_detach,
};

static int ds1672_probe(struct i2c_adapter *adapter, int address, int kind)
{
	int err = 0;
	struct i2c_client *client;
	struct rtc_device *rtc;

	dev_dbg(&adapter->dev, "%s\n", __FUNCTION__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit;
	}

	if (!(client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	/* I2C client */
	client->addr = address;
	client->driver = &ds1672_driver;
	client->adapter	= adapter;

	strlcpy(client->name, ds1672_driver.driver.name, I2C_NAME_SIZE);

	/* Inform the i2c layer */
	if ((err = i2c_attach_client(client)))
		goto exit_kfree;

	dev_info(&client->dev, "chip found, driver version " DRV_VERSION "\n");

	rtc = rtc_device_register(ds1672_driver.driver.name, &client->dev,
				&ds1672_rtc_ops, THIS_MODULE);

	if (IS_ERR(rtc)) {
		err = PTR_ERR(rtc);
		dev_err(&client->dev,
			"unable to register the class device\n");
		goto exit_detach;
	}

	i2c_set_clientdata(client, rtc);

	/* If requested, set the system time */
	if (hctosys) {
		if ((err = ds1672_hctosys(client)) < 0)
			dev_err(&client->dev,
				"unable to set the system clock\n");
	}

	return 0;

exit_detach:
	i2c_detach_client(client);

exit_kfree:
	kfree(client);

exit:
	return err;
}

static int __init ds1672_init(void)
{
	return i2c_add_driver(&ds1672_driver);
}

static void __exit ds1672_exit(void)
{
	i2c_del_driver(&ds1672_driver);
}

MODULE_AUTHOR("Alessandro Zummo <a.zummo@towertech.it>");
MODULE_DESCRIPTION("Dallas/Maxim DS1672 timekeeper driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

module_init(ds1672_init);
module_exit(ds1672_exit);
