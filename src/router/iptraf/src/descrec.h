/*
 * LAN description record definitions
 */

struct descrec {
    char address[13];
    char desc[65];
};

struct desclistent {
    struct descrec rec;
    struct desclistent *prev_entry;
    struct desclistent *next_entry;
};

struct desclist {
    struct desclistent *head;
    struct desclistent *tail;
};
