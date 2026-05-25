/* ISC license. */

#include <spawn.h>

int main (void)
{
  pid_t pid ;
  posix_spawn_file_actions_t actions ;
  posix_spawnattr_t attr ;
  char *const argv[2] = { "/bin/true", 0 } ;
  char *const envp[1] = { 0 } ;
  posix_spawnattr_setflags(&attr, 0) ;
  posix_spawn_file_actions_init(&actions) ;
  return posix_spawn(&pid, argv[0], 0, 0, argv, envp) ;
}
