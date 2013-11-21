
struct fsentry {
	char fs[32];
	char mp[64];
	char fstype[16];
	char perms[4];
	struct fsentry *next;
};

struct fsentry *getfsentries(void);
