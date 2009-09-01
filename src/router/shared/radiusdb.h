#ifndef _radiusdb_h_
#define _radiusdb_h_

struct radiususer {
	unsigned int fieldlen;
	unsigned int usersize;
	unsigned char *user;
	unsigned int passwordsize;
	unsigned char *passwd;
	unsigned int downstream;
	unsigned int upstream;
//more fields can be added in future
};

struct radiusdb {
	unsigned int usercount;
	struct radiususer *users;
};

#endif
