#include <php_config.h>
#if ZTS
# define THREADSAFE 1
#endif
#if !ZEND_DEBUG
# define NDEBUG
#endif
/* discourage foolishness */
#define sqlite3_temp_directory sqlite3_temp_directory_unsafe_except_in_minit
