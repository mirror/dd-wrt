#ifndef _pokerdb_h_
#define _pokerdb_h_

#define P_CREDIT 500
#define P_LIMIT 1500

struct pokeruser {
	unsigned int fieldlen;
	unsigned int usersize;
	unsigned char *user;
	unsigned int familysize;
	unsigned char *family;
	unsigned int userid;
	int coins;
	unsigned int credit;
	unsigned int buys;
	unsigned int credits;
	unsigned int buys_today;
} __attribute__((packed));

struct pokerdb {
	unsigned int usercount;
	struct pokeruser *users;
} __attribute__((packed));

struct pokerdb *loadpokerdb(void);
void writepokerdb(struct pokerdb *db);
void freepokerdb(struct pokerdb *db);

#endif
