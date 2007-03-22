/*
 * promisc.h - definitions for promiscuous state save/recovery
 *
 * Thanks to Holger Friese 
 * <evildead@bs-pc5.et-inf.fho-emden.de> for the base patch.
 * Applied it, but then additional issues came up and I ended up doing more
 * than slight modifications.  struct iflist is becoming way too large for
 * comfort and for something as little as this.
 */

struct promisc_params {
    char ifname[8];
    int saved_state;
    int state_valid;
};

struct promisc_states {
    struct promisc_params params;
    struct promisc_states *next_entry;
};

void init_promisc_list(struct promisc_states **list);
void save_promisc_list(struct promisc_states *list);
void load_promisc_list(struct promisc_states **list);
void srpromisc(int mode, struct promisc_states *promisc_list);
void destroy_promisc_list(struct promisc_states **list);
