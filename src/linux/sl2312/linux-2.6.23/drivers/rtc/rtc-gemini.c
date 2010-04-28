/*
 *  Gemini OnChip RTC
 *
 *  Copyright (C) 2009 Janos Laube <janos.dev@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/rtc.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <asm/arch/hardware.h>

#define GEMINI_RTC_EPOCH	1970

static const unsigned char days_in_mo[] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

struct gemini_rtc
{
	struct rtc_device*	rtc_dev;
	void __iomem*		rtc_base;
	int			rtc_irq;
};

enum e_gemini_rtc_offsets
{
	GEMINI_RTC_SECOND	= 0x00,
	GEMINI_RTC_MINUTE	= 0x04,
	GEMINI_RTC_HOUR		= 0x08,
	GEMINI_RTC_DAYS		= 0x0C,
	GEMINI_RTC_ALARM_SECOND	= 0x10,
	GEMINI_RTC_ALARM_MINUTE	= 0x14,
	GEMINI_RTC_ALARM_HOUR	= 0x18,
	GEMINI_RTC_RECORD	= 0x1C,
	GEMINI_RTC_CR		= 0x20
};

static irqreturn_t gemini_rtc_interrupt(int irq, void* dev)
{
	return IRQ_HANDLED;
}

static int gemini_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct gemini_rtc* rtc = dev_get_drvdata(dev);

	unsigned int  days;
	unsigned int  months=0;
	unsigned int  years;
	unsigned int  hrs;
	unsigned int  min;
	unsigned int  sec;
	unsigned int  rtc_record;
	unsigned int  total_sec;
	unsigned int  leap_year;
	unsigned int  i;

	sec = ioread32(rtc->rtc_base + GEMINI_RTC_SECOND);
	min = ioread32(rtc->rtc_base + GEMINI_RTC_MINUTE);
	hrs = ioread32(rtc->rtc_base + GEMINI_RTC_HOUR);
	days = ioread32(rtc->rtc_base + GEMINI_RTC_DAYS);
	rtc_record = ioread32(rtc->rtc_base + GEMINI_RTC_RECORD);

	total_sec = rtc_record + days*86400 + hrs*3600 + min*60 + sec;

	tm->tm_sec = total_sec % 60;
	tm->tm_min = (total_sec/60) % 60;
	tm->tm_hour = (total_sec/3600) % 24;

	years = GEMINI_RTC_EPOCH;
	days  = total_sec/86400;
	while (days > 365)
	{
		leap_year = (!(years % 4) && (years % 100)) || !(years % 400);
		days = days - 365 - leap_year;
		years++;
	}
	leap_year = (!(years % 4) && (years + GEMINI_RTC_EPOCH % 100))
				|| !(years % 400);

	for (i=1;i<=12;i++)
	{
		if (days > (days_in_mo[i] + ((i == 2) && leap_year)))
			days = days - (days_in_mo[i] + ((i == 2) && leap_year));
		else
		{
			months = i;
			break;
		}
	}

	tm->tm_mday = days+1;
	tm->tm_mon  = months-1;
	tm->tm_year = years-1900;
	return 0;
}

static int gemini_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	struct gemini_rtc* rtc = dev_get_drvdata(dev);
	unsigned char mon, day, hrs, min, sec, leap_year;
	unsigned int years;
	unsigned int rtc_record;
	unsigned int rtc_sec,rtc_min,rtc_hour,rtc_day,total_sec;

	years = tm->tm_year + 1900;
	mon = tm->tm_mon + 1;
	day = tm->tm_mday;
	hrs = tm->tm_hour;
	min = tm->tm_min;
	sec = tm->tm_sec;

	if (years < GEMINI_RTC_EPOCH)
		return -EINVAL;
	if (years >= GEMINI_RTC_EPOCH + 178)
		return -EINVAL;

	leap_year = ((!(years % 4) && (years % 100)) || !(years % 400));
	if ((mon > 12) || (day == 0))
		return -EINVAL;
	if (day > (days_in_mo[mon] + ((mon == 2) && leap_year)))
		return -EINVAL;
	if ((hrs >= 24) || (min >= 60) || (sec >= 60))
		return -EINVAL;

	rtc_record = mktime(years,mon,day,hrs,min,sec);
	rtc_record -= mktime(GEMINI_RTC_EPOCH, 1, 1, 0, 0, 0);

	rtc_sec = ioread32(rtc->rtc_base + GEMINI_RTC_SECOND);
	rtc_min = ioread32(rtc->rtc_base + GEMINI_RTC_MINUTE);
	rtc_hour = ioread32(rtc->rtc_base + GEMINI_RTC_HOUR);
	rtc_day = ioread32(rtc->rtc_base + GEMINI_RTC_DAYS);
	total_sec= rtc_day*86400 + rtc_hour*3600 + rtc_min*60 + rtc_sec;

	iowrite32(rtc_record - total_sec, rtc->rtc_base + GEMINI_RTC_RECORD);
	iowrite32(0x01, rtc->rtc_base + GEMINI_RTC_CR);
	return 0;
}

static struct rtc_class_ops gemini_rtc_ops = {
	.read_time     = gemini_rtc_read_time,
	.set_time      = gemini_rtc_set_time,
};

static int __devinit gemini_rtc_probe(struct platform_device *pdev)
{
	struct gemini_rtc *rtc;
	struct device *dev = &pdev->dev;
	struct resource *res;
	int ret;

	rtc = kzalloc(sizeof(*rtc), GFP_KERNEL);
	if (unlikely(!rtc))
		return -ENOMEM;
	platform_set_drvdata(pdev, rtc);

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res)
	{
		ret = -ENODEV;
		goto err;
	}
	rtc->rtc_irq = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
	{
		ret = -ENODEV;
		goto err;
	}
	rtc->rtc_base = devm_ioremap(&pdev->dev, res->start,
		res->end - res->start + 1);

	ret = request_irq(rtc->rtc_irq, gemini_rtc_interrupt,
		IRQF_SHARED, pdev->name, dev);

	if (unlikely(ret))
		goto err;

	rtc->rtc_dev = rtc_device_register(pdev->name, dev, &gemini_rtc_ops,
		THIS_MODULE);
	if (unlikely(IS_ERR(rtc->rtc_dev)))
	{
		ret = PTR_ERR(rtc->rtc_dev);
		goto err;
	}
	return 0;

 err:
	kfree(rtc);
	return ret;
}

static int __devexit gemini_rtc_remove(struct platform_device *pdev)
{
	struct gemini_rtc *rtc = platform_get_drvdata(pdev);
	struct device *dev = &pdev->dev;

	free_irq(rtc->rtc_irq, dev);
	rtc_device_unregister(rtc->rtc_dev);
	platform_set_drvdata(pdev, NULL);
	kfree(rtc);

	return 0;
}

static struct platform_driver gemini_rtc_driver = {
	.driver		= {
		.name	= "rtc-gemini",
		.owner	= THIS_MODULE,
	},
	.probe		= gemini_rtc_probe,
	.remove		= __devexit_p(gemini_rtc_remove),
};

static int __init gemini_rtc_init(void)
{
	return platform_driver_register(&gemini_rtc_driver);
}

static void __exit gemini_rtc_exit(void)
{
	platform_driver_unregister(&gemini_rtc_driver);
}

module_init(gemini_rtc_init);
module_exit(gemini_rtc_exit);

MODULE_AUTHOR("Janos Laube <janos.dev@gmail.com>");
MODULE_ALIAS("platform:rtc-gemini");
MODULE_LICENSE("GPL");
