/* dnsmasq is Copyright (c) 2000-2006 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

/* This file has code to fork a helper process which recieves data via a pipe 
   shared with the main process and which is responsible for calling a script when
   DHCP leases change.

   The helper process is forked before the main process drops root, so it retains root 
   privs to pass on to the script. For this reason it tries to be paranoid about 
   data received from the main process, in case that has been compromised. We don't
   want the helper to give an attacker root. In particular, the script to be run is
   not settable via the pipe, once the fork has taken place it is not alterable by the 
   main process.
*/

#ifndef NO_FORK

struct script_data
{
  unsigned char action, hwaddr_len, hwaddr_type;
  unsigned char clid_len, hostname_len, uclass_len, vclass_len;
  struct in_addr addr;
  unsigned int remaining_time;
#ifdef HAVE_BROKEN_RTC
  unsigned int length;
#else
  time_t expires;
#endif
  unsigned char hwaddr[DHCP_CHADDR_MAX];
};

static struct script_data *buf = NULL;
static size_t bytes_in_buf = 0, buf_size = 0;

int create_helper(int event_fd, long max_fd)
{
  pid_t pid;
  int i, pipefd[2];
  struct sigaction sigact;

  if (!daemon->dhcp || !daemon->lease_change_command)
    return -1;
  
  /* create the pipe through which the main program sends us commands,
     then fork our process. By now it's too late to die(), we just log 
     any failure via the main process. */
  if (pipe(pipefd) == -1 || !fix_fd(pipefd[1]) || (pid = fork()) == -1)
    {
      send_event(event_fd, EVENT_PIPE_ERR, errno);
      return -1;
    }

  if (pid != 0)
    {
      close(pipefd[0]); /* close reader side */
      return pipefd[1];
    }

  /* ignore SIGTERM, so that we can clean up when the main process gets hit
     and SIGALRM so that we can use sleep() */
  sigact.sa_handler = SIG_IGN;
  sigact.sa_flags = 0;
  sigemptyset(&sigact.sa_mask);
  sigaction(SIGTERM, &sigact, NULL);
  sigaction(SIGALRM, &sigact, NULL);

  /* close all the sockets etc, we don't need them here */
  for (max_fd--; max_fd > 0; max_fd--)
    if (max_fd != STDOUT_FILENO && max_fd != STDERR_FILENO && 
	max_fd != STDIN_FILENO && max_fd != pipefd[0] && max_fd != event_fd)
      close(max_fd);
  
  /* loop here */
  while(1)
    {
      struct script_data data;
      char *p, *action_str, *hostname = NULL;
      unsigned char *buf = (unsigned char *)daemon->namebuff;

      /* we read zero bytes when pipe closed: this is our signal to exit */ 
      if (!read_write(pipefd[0], (unsigned char *)&data, sizeof(data), 1))
	_exit(0);
      
      if (data.action == ACTION_DEL)
	action_str = "del";
      else if (data.action == ACTION_ADD)
	action_str = "add";
      else if (data.action == ACTION_OLD || data.action == ACTION_OLD_HOSTNAME)
	action_str = "old";
      else
	continue;
	
      /* stringify MAC into dhcp_buff */
      p = daemon->dhcp_buff;
      if (data.hwaddr_type != ARPHRD_ETHER || data.hwaddr_len == 0) 
	p += sprintf(p, "%.2x-", data.hwaddr_type);
      for (i = 0; (i < data.hwaddr_len) && (i < DHCP_CHADDR_MAX); i++)
	{
	  p += sprintf(p, "%.2x", data.hwaddr[i]);
	  if (i != data.hwaddr_len - 1)
	    p += sprintf(p, ":");
	}
      
      /* and CLID into packet */
      if (!read_write(pipefd[0], buf, data.clid_len, 1))
	continue;
      for (p = daemon->packet, i = 0; i < data.clid_len; i++)
	{
	  p += sprintf(p, "%.2x", buf[i]);
	  if (i != data.clid_len - 1) 
	    p += sprintf(p, ":");
	}
      
      /* and expiry or length into dhcp_buff2 */
#ifdef HAVE_BROKEN_RTC
      sprintf(daemon->dhcp_buff2, "%u ", data.length);
#else
      sprintf(daemon->dhcp_buff2, "%lu ", (unsigned long)data.expires);
#endif
      
      if (!read_write(pipefd[0], buf, data.hostname_len + data.uclass_len + data.vclass_len, 1))
	continue;
      
      /* possible fork errors are all temporary resource problems */
      while ((pid = fork()) == -1 && (errno == EAGAIN || errno == ENOMEM))
	sleep(2);
      
      if (pid == -1)
	continue;
	  
      /* wait for child to complete */
      if (pid != 0)
	{
	  /* reap our children's children, if necessary */
	  while (1)
	    {
	      int status;
	      pid_t rc = wait(&status);
	      
	      if (rc == pid)
		{
		  /* On error send event back to main process for logging */
		  if (WIFSIGNALED(status))
		    send_event(event_fd, EVENT_KILLED, WTERMSIG(status));
		  else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		    send_event(event_fd, EVENT_EXITED, WEXITSTATUS(status));
		  break;
		}
	      
	      if (rc == -1 && errno != EINTR)
		break;
	    }
	  
	  continue;
	}
      
      if (data.clid_len != 0)
	setenv("DNSMASQ_CLIENT_ID", daemon->packet, 1);
      else
	unsetenv("DNSMASQ_CLIENT_ID");
      
#ifdef HAVE_BROKEN_RTC
      setenv("DNSMASQ_LEASE_LENGTH", daemon->dhcp_buff2, 1);
      unsetenv("DNSMASQ_LEASE_EXPIRES");
#else
      setenv("DNSMASQ_LEASE_EXPIRES", daemon->dhcp_buff2, 1); 
      unsetenv("DNSMASQ_LEASE_LENGTH");
#endif
      
      if (data.vclass_len != 0)
	{
	  buf[data.vclass_len - 1] = 0; /* don't trust zero-term */
	  /* cannot have = chars in env - truncate if found . */
	  if ((p = strchr((char *)buf, '=')))
	    *p = 0;
	  setenv("DNSMASQ_VENDOR_CLASS", (char *)buf, 1);
	  buf += data.vclass_len;
	}
      else 
	unsetenv("DNSMASQ_VENDOR_CLASS");
      
      if (data.uclass_len != 0)
	{
	  unsigned char *end = buf + data.uclass_len;
	  buf[data.uclass_len - 1] = 0; /* don't trust zero-term */
	  
	  for (i = 0; buf < end;)
	    {
	      size_t len = strlen((char *)buf) + 1;
	      if ((p = strchr((char *)buf, '=')))
		*p = 0;
	      if (strlen((char *)buf) != 0)
		{
		  sprintf(daemon->dhcp_buff2, "DNSMASQ_USER_CLASS%i", i++);
		  setenv(daemon->dhcp_buff2, (char *)buf, 1);
		}
	      buf += len;
	    }
	}
      
      sprintf(daemon->dhcp_buff2, "%u ", data.remaining_time);
      setenv("DNSMASQ_TIME_REMAINING", daemon->dhcp_buff2, 1);
      
      if (data.hostname_len != 0)
	{
	  hostname = (char *)buf;
	  hostname[data.hostname_len - 1] = 0;
	  if (!canonicalise(hostname))
	    hostname = NULL;
	}
      
      if (data.action == ACTION_OLD_HOSTNAME && hostname)
	{
	  setenv("DNSMASQ_OLD_HOSTNAME", hostname, 1);
	  hostname = NULL;
	}
      else
	unsetenv("DNSMASQ_OLD_HOSTNAME");

      /* we need to have the event_fd around if exec fails */
      if ((i = fcntl(event_fd, F_GETFD)) != -1)
	fcntl(event_fd, F_SETFD, i | FD_CLOEXEC);
      close(pipefd[0]);

      p =  strrchr(daemon->lease_change_command, '/');
      execl(daemon->lease_change_command, 
	    p ? p+1 : daemon->lease_change_command,
	    action_str, daemon->dhcp_buff, inet_ntoa(data.addr), hostname, (char*)NULL);
      
      /* failed, send event so the main process logs the problem */
      send_event(event_fd, EVENT_EXEC_ERR, errno);
      _exit(0); 
    }
}

/* pack up lease data into a buffer */    
void queue_script(int action, struct dhcp_lease *lease, char *hostname, time_t now)
{
  unsigned char *p;
  size_t size;
  unsigned int i, hostname_len = 0, clid_len = 0, vclass_len = 0, uclass_len = 0;

  /* no script */
  if (daemon->helperfd == -1)
    return;

  if (lease->vendorclass)
    vclass_len = lease->vendorclass_len;
  if (lease->userclass)
    uclass_len = lease->userclass_len;
  if (lease->clid)
    clid_len = lease->clid_len;
  if (hostname)
    hostname_len = strlen(hostname) + 1;

  size = sizeof(struct script_data) +  clid_len + vclass_len + uclass_len + hostname_len;

  if (size > buf_size)
    {
      struct script_data *new;
      
      /* start with resonable size, will almost never need extending. */
      if (size < sizeof(struct script_data) + 200)
	size = sizeof(struct script_data) + 200;

      if (!(new = whine_malloc(size)))
	return;
      if (buf)
	free(buf);
      buf = new;
      buf_size = size;
    }

  buf->action = action;
  buf->hwaddr_len = lease->hwaddr_len;
  buf->hwaddr_type = lease->hwaddr_type;
  buf->clid_len = clid_len;
  buf->vclass_len = vclass_len;
  buf->uclass_len = uclass_len;
  buf->hostname_len = hostname_len;
  buf->addr = lease->addr;
  memcpy(buf->hwaddr, lease->hwaddr, lease->hwaddr_len);
#ifdef HAVE_BROKEN_RTC 
  buf->length = lease->length;
#else
  buf->expires = lease->expires;
#endif
  buf->remaining_time = (unsigned int)difftime(lease->expires, now);

  p = (unsigned char *)(buf+1);
  if (clid_len != 0)
    {
      memcpy(p, lease->clid, clid_len);
      p += clid_len;
    }
  if (vclass_len != 0)
    {
      memcpy(p, lease->vendorclass, vclass_len);
      p += vclass_len;
    }
  if (uclass_len != 0)
    {
      memcpy(p, lease->userclass, uclass_len);
      p += uclass_len;
    }
  /* substitute * for space */
  for (i = 0; i < hostname_len; i++)
    if ((daemon->options & OPT_LEASE_RO) && hostname[i] == ' ')
      *(p++) = '*';
    else
      *(p++) = hostname[i];
  
  bytes_in_buf = p - (unsigned char *)buf;
}

int helper_buf_empty(void)
{
  return bytes_in_buf == 0;
}

void helper_write(void)
{
  ssize_t rc;

  if (bytes_in_buf == 0)
    return;
  
  if ((rc = write(daemon->helperfd, buf, bytes_in_buf)) != -1)
    {
      if (bytes_in_buf != (size_t)rc)
	memmove(buf, buf + rc, bytes_in_buf - rc); 
      bytes_in_buf -= rc;
    }
  else
    {
      if (errno == EAGAIN || errno == EINTR)
	return;
      bytes_in_buf = 0;
    }
}

#endif


