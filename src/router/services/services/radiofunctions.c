/*
 * radiofunctions.c
 *
 * Copyright (C) 2010 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
extern void radio_off(int idx);
extern void radio_on(int idx);

void start_radio_off(void)
{
	radio_off(-1);
}

void start_radio_on(void)
{
	radio_on(-1);
}

void start_radio_off_0(void)
{
	radio_off(0);
}

void start_radio_on_0(void)
{
	radio_on(0);
}

void start_radio_off_1(void)
{
	radio_off(1);
}

void start_radio_on_1(void)
{
	radio_on(1);
}

void start_radio_off_2(void)
{
	radio_off(2);
}

void start_radio_on_2(void)
{
	radio_on(2);
}

void stop_radio_off(void)
{
}

void stop_radio_on(void)
{
}

void stop_radio_off_0(void)
{
}

void stop_radio_on_0(void)
{
}

void stop_radio_off_1(void)
{
}

void stop_radio_on_1(void)
{
}

void stop_radio_off_2(void)
{
}

void stop_radio_on_2(void)
{
}
