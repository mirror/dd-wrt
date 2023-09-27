#if defined(HAVE_PCRE_H)
#include <pcre.h>
#elif defined(HAVE_REGEX_H)
#include <sys/types.h>
#include <regex.h>
#endif

struct _conf_mount_list;

extern struct _conf_mount_list *ignoremount_list;

void init_hw_fsys(void);
void shutdown_hw_fsys( void );

typedef struct _conf_mount_list {
#if defined(HAVE_PCRE_H)
    pcre                    *regex_ptr;
#elif defined(HAVE_REGEX_H)
    regex_t                 *regex_ptr;
#endif
    char                    *name;
    struct _conf_mount_list *next;
} conf_mount_list;
