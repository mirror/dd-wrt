#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

__pid_t
__vfork __P ((void))
{
  return fork();
}

weak_alias (__vfork, vfork)
