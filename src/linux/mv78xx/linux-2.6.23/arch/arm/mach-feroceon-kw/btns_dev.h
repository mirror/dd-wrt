/*******************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/

#ifndef _BTNS_DEV_H_
#define _BTNS_DEV_H_

/* rd6281 */
#define WPS_GPP 46
#define WPS_BTN (1 << WPS_GPP)

/* rd6192 tact switch */
#define UP_GPP		30
#define DOWN_GPP	34
#define LEFT_GPP	32
#define RIGHT_GPP	31
#define PRESSED_GPP	25

struct btns_platform_data {
        unsigned int    btns_num;
        struct btn_data *btns_data_arr;
};

#endif /* _BTNS_DEV_H_ */

