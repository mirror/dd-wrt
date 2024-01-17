/*
 * voltage.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#ifdef HAVE_VOLT
EJ_VISIBLE void ej_get_voltage(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_LAGUNA
	FILE *fp = fopen("/sys/bus/i2c/devices/0-0029/in0_input", "rb");
#elif HAVE_NEWPORT
	FILE *fp = fopen("/sys/class/hwmon/hwmon0/in1_input", "rb");
#elif HAVE_VENTANA
	FILE *fp = fopen("/sys/bus/i2c/devices/0-0029/in0_input", "rb");
#else
	FILE *fp = fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028/volt", "rb");
	if (!fp)
		fp = fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028/in1_input", "rb");
#endif
	if (fp == NULL) {
		websWrite(wp, "%s", live_translate(wp, "status_router.notavail")); // no
		// i2c
		// lm75
		// found
		return;
	}
	int temp;

	fscanf(fp, "%d", &temp);
	fclose(fp);
	// temp*=564;
	int high = temp / 1000;
	int low = (temp - (high * 1000)) / 100;

	websWrite(wp, "%d.%d Volt", high, low); // no i2c lm75 found
}

EJ_VISIBLE void ej_show_voltage(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "status_router.inpvolt", NULL);
	websWrite(wp, "<span id=\"voltage\">");
	ej_get_voltage(wp, argc, argv);
	websWrite(wp, "</span>&nbsp;\n");
	websWrite(wp, "</div>\n");
}
#endif
