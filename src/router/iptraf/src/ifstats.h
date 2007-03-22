
/***

ifstats.h - structure definitions for interface counts
	
***/

struct iflist {
    char ifname[8];
    unsigned int encap;
    unsigned long long iptotal;
    unsigned long badtotal;
    unsigned long long noniptotal;
    unsigned long long total;
    unsigned int spanbr;
    unsigned long br;
    float rate;
    float peakrate;
    unsigned int index;
    struct iflist *prev_entry;
    struct iflist *next_entry;
};

struct iftab {
    struct iflist *head;
    struct iflist *tail;
    struct iflist *firstvisible;
    struct iflist *lastvisible;
    WINDOW *borderwin;
    PANEL *borderpanel;
    WINDOW *statwin;
    PANEL *statpanel;
};

struct iftotals {
    unsigned long long total;
    unsigned long long bytestotal;
    unsigned long long total_in;
    unsigned long long total_out;
    unsigned long long bytestotal_out;
    unsigned long long bytestotal_in;
    unsigned long long bcast;
    unsigned long long bcastbytes;

    unsigned long long iptotal;
    unsigned long long ipbtotal;
    unsigned long long iptotal_in;
    unsigned long long iptotal_out;
    unsigned long long ipbtotal_in;
    unsigned long long ipbtotal_out;

    unsigned long long noniptotal;
    unsigned long long nonipbtotal;
    unsigned long long noniptotal_in;
    unsigned long long noniptotal_out;
    unsigned long long nonipbtotal_in;
    unsigned long long nonipbtotal_out;

    unsigned long long tcptotal;
    unsigned long long tcpbtotal;
    unsigned long long tcptotal_in;
    unsigned long long tcptotal_out;
    unsigned long long tcpbtotal_in;
    unsigned long long tcpbtotal_out;

    unsigned long long udptotal;
    unsigned long long udpbtotal;
    unsigned long long udptotal_in;
    unsigned long long udptotal_out;
    unsigned long long udpbtotal_in;
    unsigned long long udpbtotal_out;

    unsigned long long icmptotal;
    unsigned long long icmpbtotal;
    unsigned long long icmptotal_in;
    unsigned long long icmptotal_out;
    unsigned long long icmpbtotal_in;
    unsigned long long icmpbtotal_out;

    unsigned long long othtotal;
    unsigned long long othbtotal;
    unsigned long long othtotal_in;
    unsigned long long othtotal_out;
    unsigned long long othbtotal_in;
    unsigned long long othbtotal_out;

    unsigned long badtotal;
    unsigned int interval;
};
