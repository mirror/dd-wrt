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

#ifndef _BTNS_DRIVER_H_
#define _BTNS_DRIVER_H_

typedef enum
{
	BTN_NO_OP,
	BTN_PUSH,
	BTN_RELEASE,
	BTN_CHANGE /* Both Push & Release will be monitored */
} BTN_OP;


typedef struct {
        unsigned int btn_id;
        unsigned int btn_push_cntr;
        unsigned int btn_release_cntr;
} BTN, *BTN_PTR;


typedef struct {
	unsigned int btns_number;
        BTN* btns;
} BTNS_STS, *BTNS_STS_PTR;

#define MV_BTNS_NAME       "BTNS"


struct btn_data {
        unsigned int    gpp_id;
        unsigned int    default_gpp_val;
        BTN_OP          btn_op;
        char            *btn_name;
};


/*
 *  done against open of /dev/gpp, to get a cloned descriptor.
 */
#define CIOCWAIT_P       _IOWR('c', 150, BTNS_STS)
#define CIOCNOWAIT_P     _IOWR('c', 151, BTNS_STS)


#endif /* _BTNS_DRIVER_H_ */

