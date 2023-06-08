// Copyright (C) 2002, 2003, 2008, 2012, 2014
//               Enrico Scholz <enrico.scholz@ensc.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "splint.h"

#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include "output.h"
#include "util.h"
#include "wrappers.h"

static int		msg_fd = -1;

void
openMsgfile(/*@in@*//*@null@*/char const *filename)
    /*@globals msg_fd@*/
    /*@modifies msg_fd@*/
{
  if (filename==0 || filename[0]=='\0')        { msg_fd =  2; }
  else if (strcmp(filename, "/dev/null")==0)   { msg_fd = -1; }
  else {
    int		new_fd;

    new_fd = Eopen(filename, O_CREAT|O_WRONLY|O_APPEND, 0400);
    msg_fd = Edup2(new_fd, 2);
    assert(msg_fd==2);

    Eclose(new_fd);
  }
}

void
writeMsgTimestamp()
    /*@globals msg_fd@*/
{
  struct timeval		tv;
  time_t			aux;
  size_t			fill_cnt = 1;

  if (msg_fd==-1) return;

  (void)gettimeofday(&tv, 0);

  {
#ifdef WITH_LOGGING
    char			buffer[16];
    struct tm			tmval;

    (void)localtime_r(&tv.tv_sec, &tmval);
    if (strftime(buffer, sizeof buffer, "%T", &tmval)>0) {
      (void)write(msg_fd, buffer, strlen(buffer));
    }
    else
#endif
    {
      writeUInt(msg_fd, static_cast(unsigned int)(tv.tv_sec));
    }
  }

  aux  = tv.tv_usec;
  aux |= 1; // Prevent 'aux==0' which will cause an endless-loop below
  assert(aux>0);

  while (aux<100000) { ++fill_cnt; aux *= 10; }
  (void)write(msg_fd, ".000000", fill_cnt);
  writeUInt(msg_fd, static_cast(unsigned int)(tv.tv_usec));
}

void
writeUInt(int fd, unsigned int val)
{
  char		buffer[32];
  char		*ptr = &buffer[sizeof(buffer) - 1];

  do {
      /*@-strictops@*/
    *ptr-- = '0' + static_cast(char)(val%10);
      /*@=strictops@*/
    val   /= 10;
  } while (val!=0);

  ++ptr;
  assertDefined(ptr);
  assert(ptr<=&buffer[sizeof(buffer)]);

    /*@-strictops@*/
  (void)write(fd, ptr, &buffer[sizeof(buffer)] - ptr);
    /*@=strictops@*/
}

void
writeMsgUInt(unsigned int val)
    /*@globals msg_fd@*/
{
  if (msg_fd==-1) return;

  writeUInt(msg_fd, val);
}

void
writeMsgStr(char const *msg, size_t len)
    /*@globals msg_fd@*/
{
  if (msg_fd==-1) return;

  (void)write(msg_fd, msg, len);
}

void
writeMsg(char const *msg, size_t len)
    /*@globals msg_fd@*/
{
  if (msg_fd==-1) return;

  writeMsgTimestamp();
  (void)write(msg_fd, ": ", 2);
  (void)write(msg_fd, msg, len);
  (void)write(msg_fd, "\n", 1);
}


  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
