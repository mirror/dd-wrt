/* dnsmasq is Copyright (c) 2000-2007 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

#ifdef HAVE_TFTP

static struct tftp_file *check_tftp_fileperm(ssize_t *len);
static void free_transfer(struct tftp_transfer *transfer);
static ssize_t tftp_err(int err, char *packet, char *mess, char *file);
static ssize_t tftp_err_oops(char *packet, char *file);
static ssize_t get_block(char *packet, struct tftp_transfer *transfer);
static char *next(char **p, char *end);

#define OP_RRQ  1
#define OP_WRQ  2
#define OP_DATA 3
#define OP_ACK  4
#define OP_ERR  5
#define OP_OACK 6

#define ERR_NOTDEF 0
#define ERR_FNF    1
#define ERR_PERM   2
#define ERR_FULL   3
#define ERR_ILL    4

void tftp_request(struct listener *listen, time_t now)
{
  ssize_t len;
  char *packet = daemon->packet;
  char *filename, *mode, *p, *end, *opt;
  struct sockaddr_in addr, peer;
  struct msghdr msg;
  struct cmsghdr *cmptr;
  struct iovec iov;
  struct ifreq ifr;
  int is_err = 1, if_index = 0;
  struct iname *tmp;
  struct tftp_transfer *transfer;

  union {
    struct cmsghdr align; /* this ensures alignment */
#ifdef HAVE_LINUX_NETWORK
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#else
    char control[CMSG_SPACE(sizeof(struct sockaddr_dl))];
#endif
  } control_u; 

  msg.msg_controllen = sizeof(control_u);
  msg.msg_control = control_u.control;
  msg.msg_flags = 0;
  msg.msg_name = &peer;
  msg.msg_namelen = sizeof(peer);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  iov.iov_base = packet;
  iov.iov_len = daemon->packet_buff_sz;

  /* we overwrote the buffer... */
  daemon->srv_save = NULL;

  if ((len = recvmsg(listen->tftpfd, &msg, 0)) < 2)
    return;
  
  if (daemon->options & OPT_NOWILD)
    addr = listen->iface->addr.in;
  else
    {
      addr.sin_addr.s_addr = 0;
      
#if defined(HAVE_LINUX_NETWORK)
      for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	if (cmptr->cmsg_level == SOL_IP && cmptr->cmsg_type == IP_PKTINFO)
	  {
	    addr.sin_addr = ((struct in_pktinfo *)CMSG_DATA(cmptr))->ipi_spec_dst;
	    if_index = ((struct in_pktinfo *)CMSG_DATA(cmptr))->ipi_ifindex;
	  }
      if (!(ifr.ifr_ifindex = if_index) || 
	  ioctl(listen->tftpfd, SIOCGIFNAME, &ifr) == -1)
	return;
      
#elif defined(IP_RECVDSTADDR) && defined(IP_RECVIF)
      for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVDSTADDR)
	  addr.sin_addr = *((struct in_addr *)CMSG_DATA(cmptr));
	else if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVIF)
	  if_index = ((struct sockaddr_dl *)CMSG_DATA(cmptr))->sdl_index;
      
      if (if_index == 0 || !if_indextoname(if_index, ifr.ifr_name))
	return;
      
#endif
      
      if (addr.sin_addr.s_addr == 0)
	return;
      
      if (!iface_check(AF_INET, (struct all_addr *)&addr.sin_addr, 
		       &ifr, &if_index))
	return;
      
      /* allowed interfaces are the same as for DHCP */
      for (tmp = daemon->dhcp_except; tmp; tmp = tmp->next)
	if (tmp->name && (strcmp(tmp->name, ifr.ifr_name) == 0))
	  return;
      
    }
  
  /* tell kernel to use ephemeral port */
  addr.sin_port = 0;
  addr.sin_family = AF_INET;
#ifdef HAVE_SOCKADDR_SA_LEN
  addr.sin_len = sizeof(addr);
#endif
  
  if (!(transfer = whine_malloc(sizeof(struct tftp_transfer))))
    return;
  
  if ((transfer->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
      free(transfer);
      return;
    }
  
  transfer->peer = peer;
  transfer->timeout = now + 2;
  transfer->backoff = 1;
  transfer->block = 1;
  transfer->blocksize = 512;
  transfer->file = NULL;
  transfer->opt_blocksize = transfer->opt_transize = 0;

  if (bind(transfer->sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1 ||
      !fix_fd(transfer->sockfd))
    {
      free_transfer(transfer);
      return;
    }

  p = packet + 2;
  end = packet + len;

  if (ntohs(*((unsigned short *)packet)) != OP_RRQ ||
      !(filename = next(&p, end)) ||
      !(mode = next(&p, end)) ||
      strcasecmp(mode, "octet") != 0)
    len = tftp_err(ERR_ILL, packet, _("unsupported request from %s"), inet_ntoa(peer.sin_addr));
  else
    {
      while ((opt = next(&p, end)))
	{
	  if (strcasecmp(opt, "blksize") == 0 &&
	      (opt = next(&p, end)) &&
	      !(daemon->options & OPT_TFTP_NOBLOCK))
	    {
	      transfer->blocksize = atoi(opt);
	      if (transfer->blocksize < 1)
		transfer->blocksize = 1;
	      if (transfer->blocksize > (unsigned)daemon->packet_buff_sz - 4)
		transfer->blocksize = (unsigned)daemon->packet_buff_sz - 4;
	      transfer->opt_blocksize = 1;
	      transfer->block = 0;
	    }
	  
	  if (strcasecmp(opt, "tsize") == 0 && next(&p, end))
	    {
	      transfer->opt_transize = 1;
	      transfer->block = 0;
	    }
	}

      strcpy(daemon->namebuff, "/");
      if (daemon->tftp_prefix)
	{
	  if (daemon->tftp_prefix[0] == '/')
	    daemon->namebuff[0] = 0;
	  strncat(daemon->namebuff, daemon->tftp_prefix, MAXDNAME);
	  if (daemon->tftp_prefix[strlen(daemon->tftp_prefix)-1] != '/')
	    strncat(daemon->namebuff, "/", MAXDNAME);

	  if (daemon->options & OPT_TFTP_APREF)
	    {
	      size_t oldlen = strlen(daemon->namebuff);
	      struct stat statbuf;
	      
	      strncat(daemon->namebuff, inet_ntoa(peer.sin_addr), MAXDNAME);
	      strncat(daemon->namebuff, "/", MAXDNAME);
	      
	      /* remove unique-directory if it doesn't exist */
	      if (stat(daemon->namebuff, &statbuf) == -1 || !S_ISDIR(statbuf.st_mode))
		daemon->namebuff[oldlen] = 0;
	    }
		
	  /* Absolute pathnames OK if they match prefix */
	  if (filename[0] == '/')
	    {
	      if (strstr(filename, daemon->namebuff) == filename)
		daemon->namebuff[0] = 0;
	      else
		filename++;
	    }
	}
      else if (filename[0] == '/')
	daemon->namebuff[0] = 0;
      strncat(daemon->namebuff, filename, MAXDNAME);
      daemon->namebuff[MAXDNAME-1] = 0;

      /* check permissions and open file */
      if ((transfer->file = check_tftp_fileperm(&len)))
	{
	  if ((len = get_block(packet, transfer)) == -1)
	    len = tftp_err_oops(packet, daemon->namebuff);
	  else
	    is_err = 0;
	}
    }
  
  while (sendto(transfer->sockfd, packet, len, 0, 
		(struct sockaddr *)&peer, sizeof(peer)) == -1 && errno == EINTR);
  
  if (is_err)
    free_transfer(transfer);
  else
    {
      my_syslog(LOG_INFO, _("TFTP sent %s to %s"), daemon->namebuff, inet_ntoa(peer.sin_addr));
      transfer->next = daemon->tftp_trans;
      daemon->tftp_trans = transfer;
    }
}
 
static struct tftp_file *check_tftp_fileperm(ssize_t *len)
{
  char *packet = daemon->packet, *namebuff = daemon->namebuff;
  struct tftp_file *file;
  struct tftp_transfer *t;
  uid_t uid = geteuid();
  struct stat statbuf;
  int fd = -1;

  /* trick to ban moving out of the subtree */
  if (daemon->tftp_prefix && strstr(namebuff, "/../"))
    goto perm;
  
  if ((fd = open(namebuff, O_RDONLY)) == -1)
    {
      if (errno == ENOENT)
	{
	  *len = tftp_err(ERR_FNF, packet, _("file %s not found"), namebuff);
	  return NULL;
	}
      else if (errno == EACCES)
	goto perm;
      else
	goto oops;
    }
  
  /* stat the file descriptor to avoid stat->open races */
  if (fstat(fd, &statbuf) == -1)
    goto oops;
  
  /* running as root, must be world-readable */
  if (uid == 0)
    {
      if (!(statbuf.st_mode & S_IROTH))
	goto perm;
    }
  /* in secure mode, must be owned by user running dnsmasq */
  else if ((daemon->options & OPT_TFTP_SECURE) && uid != statbuf.st_uid)
    goto perm;
      
  /* If we're doing many tranfers from the same file, only 
     open it once this saves lots of file descriptors 
     when mass-booting a big cluster, for instance. 
     Be conservative and only share when inode and name match
     this keeps error messages sane. */
  for (t = daemon->tftp_trans; t; t = t->next)
    if (t->file->dev == statbuf.st_dev && 
	t->file->inode == statbuf.st_ino &&
	strcmp(t->file->filename, namebuff) == 0)
      {
	close(fd);
	t->file->refcount++;
	return t->file;
      }
  
  if (!(file = whine_malloc(sizeof(struct tftp_file) + strlen(namebuff) + 1)))
    {
      errno = ENOMEM;
      goto oops;
    }

  file->fd = fd;
  file->size = statbuf.st_size;
  file->dev = statbuf.st_dev;
  file->inode = statbuf.st_ino;
  file->refcount = 1;
  strcpy(file->filename, namebuff);
  return file;
  
 perm:
  errno = EACCES;
  *len =  tftp_err(ERR_PERM, packet, _("cannot access %s: %s"), namebuff);
  if (fd != -1)
    close(fd);
  return NULL;

 oops:
  *len =  tftp_err_oops(packet, namebuff);
  if (fd != -1)
    close(fd);
  return NULL;
}

void check_tftp_listeners(fd_set *rset, time_t now)
{
  struct tftp_transfer *transfer, *tmp, **up;
  ssize_t len;
  
  struct ack {
    unsigned short op, block;
  } *mess = (struct ack *)daemon->packet;
  
  /* Check for activity on any existing transfers */
  for (transfer = daemon->tftp_trans, up = &daemon->tftp_trans; transfer; transfer = tmp)
    {
      tmp = transfer->next;
      
      if (FD_ISSET(transfer->sockfd, rset))
	{
	  /* we overwrote the buffer... */
	  daemon->srv_save = NULL;
	  
	  if ((len = recv(transfer->sockfd, daemon->packet, daemon->packet_buff_sz, 0)) >= (ssize_t)sizeof(struct ack))
	    {
	      if (ntohs(mess->op) == OP_ACK && ntohs(mess->block) == (unsigned short)transfer->block) 
		{
		  /* Got ack, ensure we take the (re)transmit path */
		  transfer->timeout = now;
		  transfer->backoff = 0;
		  transfer->block++;
		}
	      else if (ntohs(mess->op) == OP_ERR)
		{
		  char *p = daemon->packet + sizeof(struct ack);
		  char *end = daemon->packet + len;
		  char *err = next(&p, end);
		  /* Sanitise error message */
		  if (!err)
		    err = "";
		  else
		    {
		      char *q, *r;
		      for (q = r = err; *r; r++)
			if (isprint(*r))
			  *(q++) = *r;
		      *q = 0;
		    }
		  my_syslog(LOG_ERR, _("TFTP error %d %s received from %s"),
			    (int)ntohs(mess->block), err, 
			    inet_ntoa(transfer->peer.sin_addr));	
		  
		  /* Got err, ensure we take abort */
		  transfer->timeout = now;
		  transfer->backoff = 100;
		}
	    }
	}
      
      if (difftime(now, transfer->timeout) >= 0.0)
	{
	  int endcon = 0;

	  /* timeout, retransmit */
	  transfer->timeout += 1 + (1<<transfer->backoff);
	  	  
	  /* we overwrote the buffer... */
	  daemon->srv_save = NULL;
	 
	  if ((len = get_block(daemon->packet, transfer)) == -1)
	    {
	      len = tftp_err_oops(daemon->packet, transfer->file->filename);
	      endcon = 1;
	    }
	  else if (++transfer->backoff > 5)
	    {
	      /* don't complain about timeout when we're awaiting the last
		 ACK, some clients never send it */
	      if (len != 0)
		my_syslog(LOG_ERR, _("TFTP failed sending %s to %s"), 
			  transfer->file->filename, inet_ntoa(transfer->peer.sin_addr));
	      len = 0;
	    }
	  
	  if (len != 0)
	    while(sendto(transfer->sockfd, daemon->packet, len, 0, 
			 (struct sockaddr *)&transfer->peer, sizeof(transfer->peer)) == -1 && errno == EINTR);
	  
	  if (endcon || len == 0)
	    {
	      /* unlink */
	      *up = tmp;
	      free_transfer(transfer);
	      continue;
	    }
	}

      up = &transfer->next;
    }
}

static void free_transfer(struct tftp_transfer *transfer)
{
  close(transfer->sockfd);
  if (transfer->file && (--transfer->file->refcount) == 0)
    {
      close(transfer->file->fd);
      free(transfer->file);
    }
  free(transfer);
}

static char *next(char **p, char *end)
{
  char *ret = *p;
  size_t len;

  if (*(end-1) != 0 || 
      *p == end ||
      (len = strlen(ret)) == 0)
    return NULL;

  *p += len + 1;
  return ret;
}

static ssize_t tftp_err(int err, char *packet, char *message, char *file)
{
  struct errmess {
    unsigned short op, err;
    char message[];
  } *mess = (struct errmess *)packet;
  ssize_t ret = 4;
  char *errstr = strerror(errno);
 
  mess->op = htons(OP_ERR);
  mess->err = htons(err);
  ret += (snprintf(mess->message, 500,  message, file, errstr) + 1);
  if (err != ERR_FNF)
    my_syslog(LOG_ERR, "TFTP %s", mess->message);
  
  return  ret;
}

static ssize_t tftp_err_oops(char *packet, char *file)
{
  return tftp_err(ERR_NOTDEF, packet, _("cannot read %s: %s"), file);
}

/* return -1 for error, zero for done. */
static ssize_t get_block(char *packet, struct tftp_transfer *transfer)
{
  if (transfer->block == 0)
    {
      /* send OACK */
      char *p;
      struct oackmess {
	unsigned short op;
	char data[];
      } *mess = (struct oackmess *)packet;
      
      p = mess->data;
      mess->op = htons(OP_OACK);
      if (transfer->opt_blocksize)
	{
	  p += (sprintf(p, "blksize") + 1);
	  p += (sprintf(p, "%d", transfer->blocksize) + 1);
	}
      if (transfer->opt_transize)
	{
	  p += (sprintf(p,"tsize") + 1);
	  p += (sprintf(p, "%u", (unsigned int)transfer->file->size) + 1);
	}

      return p - packet;
    }
  else
    {
      /* send data packet */
      struct datamess {
	unsigned short op, block;
	unsigned char data[];
      } *mess = (struct datamess *)packet;
      
      off_t offset = transfer->blocksize * (transfer->block - 1);
      size_t size = transfer->file->size - offset; 
      
      if (offset > transfer->file->size)
	return 0; /* finished */
      
      if (size > transfer->blocksize)
	size = transfer->blocksize;
      
      lseek(transfer->file->fd, offset, SEEK_SET);
      
      mess->op = htons(OP_DATA);
      mess->block = htons((unsigned short)(transfer->block));
      
      if (!read_write(transfer->file->fd, mess->data, size, 1))
	return -1;
      else
	return size + 4;
    }
}

#endif
