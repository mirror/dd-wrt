
struct support_list {
	char *name;
	char *log_name;
	char *full_name;
	char *supp_name;
	char *supp_value;
	char *log_level;
};
extern struct support_list supports[];

#define SUPPORT_COUNT 1
