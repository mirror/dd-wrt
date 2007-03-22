struct isdntabent {
    char ifname[10];
    unsigned int encap;
    struct isdntabent *next_entry;
};

struct isdntab {
    struct isdntabent *head;
    struct isdntabent *tail;
};

void add_isdn_entry(struct isdntab *list, char *ifname, int isdn_fd);
struct isdntabent *isdn_table_lookup(struct isdntab *list, char *ifname,
                                     int isdn_fd);
void destroy_isdn_table(struct isdntab *list);
