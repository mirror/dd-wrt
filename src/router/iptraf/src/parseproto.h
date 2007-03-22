#define RANGE_OK            0
#define COMMA_EXPECTED      1
#define INVALID_RANGE        2
#define OUT_OF_RANGE        4
#define NO_MORE_TOKENS      5

void get_next_protorange(char *src, char **cptr,
                         unsigned int *proto1, unsigned int *proto2,
                         int *parse_result, char **badtokenptr);
int validate_ranges(char *src, int *parse_result, char **bptr);
