/* ioctl_query.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/ioctl.h"


/**
 * can_ioctl_query - query for driver version and features
 * @canuser: pointer to the user/client state structure
 * @what:    select which feature or version is queried
 *
 * Return Value: returns value for queried characteristic or -EINVAL
 *	if there is no reply to the query.
 */

int can_ioctl_query(struct canuser_t *canuser, unsigned long what)
{
	switch(what){
		case CAN_DRV_QRY_BRANCH:
			/* returns driver branch value - "LINC" for LinCAN driver */
			return CAN_DRV_BRANCH;

		case CAN_DRV_QRY_VERSION:
			/* returns driver version as (major<<16) | (minor<<8) | patch */
			return CAN_DRV_VER;

		case CAN_DRV_QRY_MSGFORMAT:
			/* format of canmsg_t structure */
			#ifdef CAN_MSG_VERSION_2
			  return 2;
			#else /*CAN_MSG_VERSION_2*/
			  return 1;
			#endif /*CAN_MSG_VERSION_2*/
	}

	return -EINVAL;
}
