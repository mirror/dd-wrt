/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

promisc.c	- handles the promiscuous mode flag for the Ethernet/FDDI/
              Token Ring interfaces

***/

#include "iptraf-ng-compat.h"

#include "ifaces.h"
#include "error.h"
#include "promisc.h"
#include "dirs.h"

#define PROMISC_MSG_MAX 80

void init_promisc_list(struct promisc_states **list)
{
	FILE *fd;
	char buf[IFNAMSIZ];
	struct promisc_states *ptmp;
	struct promisc_states *tail = NULL;

	*list = NULL;
	fd = open_procnetdev();

	while (get_next_iface(fd, buf, sizeof(buf))) {
		if (strcmp(buf, "") != 0) {
			ptmp = xmalloc(sizeof(struct promisc_states));
			strcpy(ptmp->params.ifname, buf);

			if (*list == NULL) {
				*list = ptmp;
			} else
				tail->next_entry = ptmp;

			tail = ptmp;
			ptmp->next_entry = NULL;

			/*
			 * Retrieve and save interface flags
			 */

			if ((strncmp(buf, "eth", 3) == 0)
			    || (strncmp(buf, "ra", 2) == 0)
			    || (strncmp(buf, "fddi", 4) == 0)
			    || (strncmp(buf, "tr", 2) == 0)
			    || (strncmp(buf, "ath", 3) == 0)
			    || (strncmp(buf, "bnep", 4) == 0)
			    || (strncmp(buf, "ni", 2) == 0)
			    || (strncmp(buf, "tap", 3) == 0)
			    || (strncmp(buf, "dummy", 5) == 0)
			    || (strncmp(buf, "br", 2) == 0)
			    || (strncmp(buf, "vmnet", 5) == 0)
			    || (strncmp(ptmp->params.ifname, "wvlan", 4) == 0)
			    || (strncmp(ptmp->params.ifname, "lec", 3) == 0)) {
				int flags = dev_get_flags(buf);

				if (flags < 0) {
					write_error("Unable to obtain interface parameters for %s",
						buf);
					ptmp->params.state_valid = 0;
				} else {
					ptmp->params.saved_state = flags;
					ptmp->params.state_valid = 1;
				}
			}
		}
	}
}

/*
 * Save interfaces and their states to a temporary file.  Used only by the
 * first IPTraf instance.  Needed in case there are subsequent, simultaneous 
 * instances of IPTraf, which may still need promiscuous mode even after
 * the first instance exits.  These subsequent instances will need to restore
 * the promiscuous state from this file.
 */

void save_promisc_list(struct promisc_states *list)
{
	int fd;
	struct promisc_states *ptmp = list;

	fd = open(PROMISCLISTFILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd < 0) {
		write_error("Unable to save interface flags");
		return;
	}

	while (ptmp != NULL) {
		write(fd, &(ptmp->params), sizeof(struct promisc_params));
		ptmp = ptmp->next_entry;
	}

	close(fd);
}

/*
 * Load promiscuous states into list
 */

void load_promisc_list(struct promisc_states **list)
{
	int fd;
	struct promisc_states *ptmp = NULL;
	struct promisc_states *tail = NULL;
	int br;

	fd = open(PROMISCLISTFILE, O_RDONLY);

	if (fd < 0) {
		write_error("Unable to retrieve saved interface flags");
		*list = NULL;
		return;
	}

	do {
		ptmp = xmalloc(sizeof(struct promisc_states));
		br = read(fd, &(ptmp->params), sizeof(struct promisc_params));

		if (br > 0) {
			if (tail != NULL)
				tail->next_entry = ptmp;
			else
				*list = ptmp;

			ptmp->next_entry = NULL;
			tail = ptmp;
		} else
			free(ptmp);
	} while (br > 0);

	close(fd);
}

/*
 * Set/restore interface promiscuous mode.
 */

void srpromisc(int mode, struct promisc_states *list)
{
	struct promisc_states *ptmp;

	ptmp = list;

	while (ptmp != NULL) {
		if (((strncmp(ptmp->params.ifname, "eth", 3) == 0)
		     || (strncmp(ptmp->params.ifname, "fddi", 4) == 0)
		     || (strncmp(ptmp->params.ifname, "tr", 2) == 0)
		     || (strncmp(ptmp->params.ifname, "ra", 2) == 0)
		     || (strncmp(ptmp->params.ifname, "ath", 3) == 0)
		     || (strncmp(ptmp->params.ifname, "wvlan", 4) == 0)
		     || (strncmp(ptmp->params.ifname, "lec", 3) == 0))
		    && (ptmp->params.state_valid)) {
			if (mode) {
				/* set promiscuous */
				int r = dev_set_promisc(ptmp->params.ifname);
				if(r < 0)
					write_error("Failed to set promiscuous mode on %s", ptmp->params.ifname);
			} else {
				/* restore saved state */
				if (ptmp->params.saved_state & IFF_PROMISC)
					/* was promisc, so leave it as is */
					continue;
				/* wasn't promisc, clear it */
				int r = dev_clear_promisc(ptmp->params.ifname);
				if(r < 0)
					write_error("Failed to clear promiscuous mode on %s", ptmp->params.ifname);
			}
		}
		ptmp = ptmp->next_entry;
	}
}

void destroy_promisc_list(struct promisc_states **list)
{
	struct promisc_states *ptmp = *list;
	struct promisc_states *ctmp;

	if (ptmp != NULL)
		ctmp = ptmp->next_entry;

	while (ptmp != NULL) {
		free(ptmp);
		ptmp = ctmp;
		if (ctmp != NULL)
			ctmp = ctmp->next_entry;
	}
}
