/***

ipfrag.h - IP fragmentation hander definitions

***/

struct fragdescent {
    unsigned int min;
    unsigned int max;
    struct fragdescent *prev_entry;
    struct fragdescent *next_entry;
};

struct fragent {
    unsigned long s_addr;
    unsigned int s_port;
    unsigned long d_addr;
    unsigned int d_port;
    unsigned int id;
    unsigned int protocol;
    int firstin;
    time_t starttime;
    struct fragdescent *fragdesclist;
    struct fragdescent *fragdesctail;
    unsigned int bcount;
    struct fragent *prev_entry;
    struct fragent *next_entry;
};

struct fragfreelistent {
    struct fragent *top;
    struct fragfreelist *next_entry;
};


struct fragent *addnewdgram(struct iphdr *packet);

struct fragdescent *addnewhole(struct fragent *frag);

struct fragent *searchfrags(unsigned long saddr, unsigned long daddr,
                            unsigned int protocol, unsigned int id);

void deldgram(struct fragent *ftmp);
void destroyholes(struct fragent *ftmp);

void destroyfraglist(void);

unsigned int processfragment(struct iphdr *packet, unsigned int *sport,
                             unsigned int *dport, int *firstin);
