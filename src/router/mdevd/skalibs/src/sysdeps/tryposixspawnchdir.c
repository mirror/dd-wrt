/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _XPG4_2
#define _XPG4_2
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <spawn.h>

int main (void)
{
  pid_t pid ;
  posix_spawn_file_actions_t actions ;
  posix_spawnattr_t attr ;
  char *const argv[2] = { "/bin/true", 0 } ;
  char *const envp[1] = { 0 } ;
  posix_spawn_file_actions_init(&actions) ;
  posix_spawn_file_actions_addchdir(&actions, "/") ;
  posix_spawn_file_actions_addfchdir(&actions, -1) ;
  return posix_spawn(&pid, argv[0], 0, 0, argv, envp) ;
}
