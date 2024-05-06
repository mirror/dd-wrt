#ifndef SRC_USR_ARGP_USERSPACE_TYPES_H_
#define SRC_USR_ARGP_USERSPACE_TYPES_H_

#include <stdbool.h>

bool show_csv_header(bool no_headers, bool csv);
bool show_footer(bool no_headers, bool csv);
void print_table_separator(int junk, ...);

#endif /* SRC_USR_ARGP_USERSPACE_TYPES_H_ */
