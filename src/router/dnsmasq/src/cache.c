/* dnsmasq is Copyright (c) 2000-2005 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

static struct crec *cache_head, *cache_tail, **hash_table;
static struct crec *dhcp_inuse, *dhcp_spare, *new_chain;
static int cache_inserted, cache_live_freed, insert_error;
static union bigname *big_free;
static int bignames_left, log_queries, cache_size, hash_size;
static int uid;
static char *addrbuff;

static void cache_free(struct crec *crecp);
static void cache_unlink(struct crec *crecp);
static void cache_link(struct crec *crecp);
static char *record_source(struct hostsfile *add_hosts, int index);

void cache_init(int size, int logq)
{
  struct crec *crecp;
  int i;

  if ((log_queries = logq))
    addrbuff = safe_malloc(ADDRSTRLEN);
  else
    addrbuff = NULL;
      
  cache_head = cache_tail = NULL;
  dhcp_inuse = dhcp_spare = NULL;
  new_chain = NULL;
  cache_size = size;
  big_free = NULL;
  bignames_left = size/10;
  uid = 0;

  cache_inserted = cache_live_freed = 0;

  if (cache_size > 0)
    {
      crecp = safe_malloc(size*sizeof(struct crec));
      
      for (i=0; i<size; i++, crecp++)
	{
	  cache_link(crecp);
	  crecp->flags = 0;
	  crecp->uid = uid++;
	}
    }
  
  /* hash_size is a power of two. */
  for (hash_size = 64; hash_size < cache_size/10; hash_size = hash_size << 1);
  hash_table = safe_malloc(hash_size*sizeof(struct crec *));
  for(i=0; i < hash_size; i++)
    hash_table[i] = NULL;
}

static struct crec **hash_bucket(char *name)
{
  unsigned int c, val = 0;
  
  /* don't use tolower and friends here - they may be messed up by LOCALE */
  while((c = (unsigned char) *name++))
    if (c >= 'A' && c <= 'Z')
      val += c + 'a' - 'A';
    else
      val += c;
  
  /* hash_size is a power of two */
  return hash_table + (val & (hash_size - 1));
}

static void cache_hash(struct crec *crecp)
{
  struct crec **bucket = hash_bucket(cache_get_name(crecp));
  crecp->hash_next = *bucket;
  *bucket = crecp;
}
 
static void cache_free(struct crec *crecp)
{
  crecp->flags &= ~F_FORWARD;
  crecp->flags &= ~F_REVERSE;
  crecp->uid = uid++; /* invalidate CNAMES pointing to this. */
  
  if (cache_tail)
    cache_tail->next = crecp;
  else
    cache_head = crecp;
  crecp->prev = cache_tail;
  crecp->next = NULL;
  cache_tail = crecp;
  
  /* retrieve big name for further use. */
  if (crecp->flags & F_BIGNAME)
    {
      crecp->name.bname->next = big_free;
      big_free = crecp->name.bname;
      crecp->flags &= ~F_BIGNAME;
    }
}    

/* insert a new cache entry at the head of the list (youngest entry) */
static void cache_link(struct crec *crecp)
{
  if (cache_head) /* check needed for init code */
    cache_head->prev = crecp;
  crecp->next = cache_head;
  crecp->prev = NULL;
  cache_head = crecp;
  if (!cache_tail)
    cache_tail = crecp;
}

/* remove an arbitrary cache entry for promotion */ 
static void cache_unlink (struct crec *crecp)
{
  if (crecp->prev)
    crecp->prev->next = crecp->next;
  else
    cache_head = crecp->next;

  if (crecp->next)
    crecp->next->prev = crecp->prev;
  else
    cache_tail = crecp->prev;
}

char *cache_get_name(struct crec *crecp)
{
  if (crecp->flags & F_BIGNAME)
    return crecp->name.bname->name;
  else if (crecp->flags & F_DHCP) 
    return crecp->name.namep;
  
  return crecp->name.sname;
}

static int is_outdated_cname_pointer(struct crec *crecp)
{
  struct crec *target = crecp->addr.cname.cache;

  if (!(crecp->flags & F_CNAME))
    return 0;

  if (!target)
    return 1;

  if (crecp->addr.cname.uid == target->uid)
    return 0;

  return 1;
}

static int is_expired(time_t now, struct crec *crecp)
{
  if (crecp->flags & F_IMMORTAL)
    return 0;

  if (difftime(now, crecp->ttd) < 0)
    return 0;

  return 1;
}

static int cache_scan_free(char *name, struct all_addr *addr, time_t now, unsigned short flags)
{
  /* Scan and remove old entries.
     If (flags & F_FORWARD) then remove any forward entries for name and any expired
     entries but only in the same hash bucket as name.
     If (flags & F_REVERSE) then remove any reverse entries for addr and any expired
     entries in the whole cache.
     If (flags == 0) remove any expired entries in the whole cache. 

     In the flags & F_FORWARD case, the return code is valid, and returns zero if the
     name exists in the cache as a HOSTS or DHCP entry (these are never deleted) */
  
  struct crec *crecp, **up;

  if (flags & F_FORWARD)
    {
      for (up = hash_bucket(name), crecp = *up; crecp; crecp = crecp->hash_next)
	if (is_expired(now, crecp) || is_outdated_cname_pointer(crecp))
	  { 
	    *up = crecp->hash_next;
	    if (!(crecp->flags & (F_HOSTS | F_DHCP)))
	      {
		cache_unlink(crecp);
		cache_free(crecp);
	      }
	  } 
	else if ((crecp->flags & F_FORWARD) && 
		 ((flags & crecp->flags & (F_IPV4 | F_IPV6)) || ((crecp->flags | flags) & F_CNAME)) &&
		 hostname_isequal(cache_get_name(crecp), name))
	  {
	    if (crecp->flags & (F_HOSTS | F_DHCP))
	      return 0;
	    *up = crecp->hash_next;
	    cache_unlink(crecp);
	    cache_free(crecp);
	  }
	else
	  up = &crecp->hash_next;
    }
  else
    {
      int i;
#ifdef HAVE_IPV6
      int addrlen = (flags & F_IPV6) ? IN6ADDRSZ : INADDRSZ;
#else
      int addrlen = INADDRSZ;
#endif 
      for (i = 0; i < hash_size; i++)
	for (crecp = hash_table[i], up = &hash_table[i]; crecp; crecp = crecp->hash_next)
	  if (is_expired(now, crecp))
	    {
	      *up = crecp->hash_next;
	      if (!(crecp->flags & (F_HOSTS | F_DHCP)))
		{ 
		  cache_unlink(crecp);
		  cache_free(crecp);
		}
	    }
	  else if (!(crecp->flags & (F_HOSTS | F_DHCP)) &&
		   (flags & crecp->flags & F_REVERSE) && 
		   (flags & crecp->flags & (F_IPV4 | F_IPV6)) &&
		   memcmp(&crecp->addr.addr, addr, addrlen) == 0)
	    {
	      *up = crecp->hash_next;
	      cache_unlink(crecp);
	      cache_free(crecp);
	    }
	  else
	    up = &crecp->hash_next;
    }
  
  return 1;
}

/* Note: The normal calling sequence is
   cache_start_insert
   cache_insert * n
   cache_end_insert

   but an abort can cause the cache_end_insert to be missed 
   in which can the next cache_start_insert cleans things up. */

void cache_start_insert(void)
{
  /* Free any entries which didn't get committed during the last
     insert due to error.
  */
  while (new_chain)
    {
      struct crec *tmp = new_chain->next;
      cache_free(new_chain);
      new_chain = tmp;
    }
  new_chain = NULL;
  insert_error = 0;
}
 
struct crec *cache_insert(char *name, struct all_addr *addr, 
			  time_t now,  unsigned long ttl, unsigned short flags)
{
#ifdef HAVE_IPV6
  int addrlen = (flags & F_IPV6) ? IN6ADDRSZ : INADDRSZ;
#else
  int addrlen = INADDRSZ;
#endif
  struct crec *new;
  union bigname *big_name = NULL;
  int freed_all = flags & F_REVERSE;

  log_query(flags | F_UPSTREAM, name, addr, 0, NULL, 0);

  /* name is needed as workspace by log_query in this case */
  if ((flags & F_NEG) && (flags & F_REVERSE))
    name = NULL;

  /* CONFIG bit no needed except for logging */
  flags &= ~F_CONFIG;

  /* if previous insertion failed give up now. */
  if (insert_error)
    return NULL;

  /* First remove any expired entries and entries for the name/address we
     are currently inserting. Fail is we attempt to delete a name from
     /etc/hosts or DHCP. */
  if (!cache_scan_free(name, addr, now, flags))
    {
      insert_error = 1;
      return NULL;
    }
  
  /* Now get a cache entry from the end of the LRU list */
  while (1) {
    if (!(new = cache_tail)) /* no entries left - cache is too small, bail */
      {
	insert_error = 1;
	return NULL;
      }
    
    /* End of LRU list is still in use: if we didn't scan all the hash
       chains for expired entries do that now. If we already tried that
       then it's time to start spilling things. */
    
    if (new->flags & (F_FORWARD | F_REVERSE))
      { 
	if (freed_all)
	  {
	    cache_scan_free(cache_get_name(new), &new->addr.addr, now, new->flags);
	    cache_live_freed++;
	  }
	else
	  {
	    cache_scan_free(NULL, NULL, now, 0);
	    freed_all = 1;
	  }
	continue;
      }
 
    /* Check if we need to and can allocate extra memory for a long name.
       If that fails, give up now. */
    if (name && (strlen(name) > SMALLDNAME-1))
      {
	if (big_free)
	  { 
	    big_name = big_free;
	    big_free = big_free->next;
	  }
	else if (!bignames_left ||
		 !(big_name = (union bigname *)malloc(sizeof(union bigname))))
	  {
	    insert_error = 1;
	    return NULL;
	  }
	else
	  bignames_left--;
	
      }

    /* Got the rest: finally grab entry. */
    cache_unlink(new);
    break;
  }
  
  new->flags = flags;
  if (big_name)
    {
      new->name.bname = big_name;
      new->flags |= F_BIGNAME;
    }
  if (name)
    strcpy(cache_get_name(new), name);
  else
    *cache_get_name(new) = 0;
  if (addr)
    memcpy(&new->addr.addr, addr, addrlen);
  else
    new->addr.cname.cache = NULL;
  
  new->ttd = now + (time_t)ttl;
  new->next = new_chain;
  new_chain = new;

  return new;
}

/* after end of insertion, commit the new entries */
void cache_end_insert(void)
{
  if (insert_error)
    return;
  
  while (new_chain)
    { 
      struct crec *tmp = new_chain->next;
      /* drop CNAMEs which didn't find a target. */
      if (is_outdated_cname_pointer(new_chain))
	cache_free(new_chain);
      else
	{
	  cache_hash(new_chain);
	  cache_link(new_chain);
	  cache_inserted++;
	}
      new_chain = tmp;
    }
  new_chain = NULL;
}

struct crec *cache_find_by_name(struct crec *crecp, char *name, time_t now, unsigned short prot)
{
  struct crec *ans;

  if (crecp) /* iterating */
    ans = crecp->next;
  else
    {
      /* first search, look for relevant entries and push to top of list
	 also free anything which has expired */
      struct crec *next, **up, **insert = NULL, **chainp = &ans;
         
      for (up = hash_bucket(name), crecp = *up; crecp; crecp = next)
	{
	  next = crecp->hash_next;
	  
	  if (!is_expired(now, crecp) && !is_outdated_cname_pointer(crecp))
	    {
	      if ((crecp->flags & F_FORWARD) && 
		  (crecp->flags & prot) &&
		  hostname_isequal(cache_get_name(crecp), name))
		{
		  if (crecp->flags & (F_HOSTS | F_DHCP))
		    {
		      *chainp = crecp;
		      chainp = &crecp->next;
		    }
		  else
		    {
		      cache_unlink(crecp);
		      cache_link(crecp);
		    }
	      	      
		  /* move all but the first entry up the hash chain
		     this implements round-robin */
		  if (!insert)
		    {
		      insert = up; 
		      up = &crecp->hash_next; 
		    }
		  else
		    {
		      *up = crecp->hash_next;
		      crecp->hash_next = *insert;
		      *insert = crecp;
		      insert = &crecp->hash_next;
		    }
		}
	      else
		/* case : not expired, incorrect entry. */
		up = &crecp->hash_next; 
	    }
	  else
	    {
	      /* expired entry, free it */
	      *up = crecp->hash_next;
	      if (!(crecp->flags & (F_HOSTS | F_DHCP)))
		{ 
		  cache_unlink(crecp);
		  cache_free(crecp);
		}
	    }
	}
	  
      *chainp = cache_head;
    }

  if (ans && 
      (ans->flags & F_FORWARD) &&
      (ans->flags & prot) &&
      hostname_isequal(cache_get_name(ans), name))
    return ans;
  
  return NULL;
}

struct crec *cache_find_by_addr(struct crec *crecp, struct all_addr *addr, 
				time_t now, unsigned short prot)
{
  struct crec *ans;
#ifdef HAVE_IPV6
  int addrlen = (prot == F_IPV6) ? IN6ADDRSZ : INADDRSZ;
#else
  int addrlen = INADDRSZ;
#endif
  
  if (crecp) /* iterating */
    ans = crecp->next;
  else
    {  
      /* first search, look for relevant entries and push to top of list
	 also free anything which has expired */
       int i;
       struct crec **up, **chainp = &ans;
       
       for(i=0; i<hash_size; i++)
	 for (crecp = hash_table[i], up = &hash_table[i]; crecp; crecp = crecp->hash_next)
	   if (!is_expired(now, crecp))
	     {      
	       if ((crecp->flags & F_REVERSE) && 
		   (crecp->flags & prot) &&
		   memcmp(&crecp->addr.addr, addr, addrlen) == 0)
		 {	    
		   if (crecp->flags & (F_HOSTS | F_DHCP))
		     {
		       *chainp = crecp;
		       chainp = &crecp->next;
		     }
		   else
		     {
		       cache_unlink(crecp);
		       cache_link(crecp);
		     }
		 }
	       up = &crecp->hash_next;
	     }
	   else
	     {
	       *up = crecp->hash_next;
	       if (!(crecp->flags & (F_HOSTS | F_DHCP)))
		 {
		   cache_unlink(crecp);
		   cache_free(crecp);
		 }
	     }
       
       *chainp = cache_head;
    }
  
  if (ans && 
      (ans->flags & F_REVERSE) &&
      (ans->flags & prot) &&
      memcmp(&ans->addr.addr, addr, addrlen) == 0)
    return ans;
  
  return NULL;
}

static void add_hosts_entry(struct crec *cache, struct all_addr *addr, int addrlen, 
			    unsigned short flags, int index)
{
  struct crec *lookup = cache_find_by_name(NULL, cache->name.sname, 0, flags & (F_IPV4 | F_IPV6));

  /* Remove duplicates in hosts files. */
  if (lookup && (lookup->flags & F_HOSTS) &&
      memcmp(&lookup->addr.addr, addr, addrlen) == 0)
    free(cache);
  else
    {
      /* Ensure there is only one address -> name mapping (first one trumps) */
      if (cache_find_by_addr(NULL, addr, 0, flags & (F_IPV4 | F_IPV6)))
	flags &= ~F_REVERSE;
      cache->flags = flags;
      cache->uid = index;
      memcpy(&cache->addr.addr, addr, addrlen);
      cache_hash(cache);
    }
}

static void read_hostsfile(char *filename, int opts, char *buff, char *domain_suffix, int index)
{  
  FILE *f = fopen(filename, "r");
  char *line;
  int count = 0, lineno = 0;
  
  if (!f)
    {
      syslog(LOG_ERR, _("failed to load names from %s: %m"), filename);
      return;
    }
    
  while ((line = fgets(buff, MAXDNAME, f)))
    {
      struct all_addr addr;
      char *token = strtok(line, " \t\n\r");
      int addrlen;
      unsigned short flags;
          
      lineno++;

      if (!token || (*token == '#')) 
	continue;

#ifdef HAVE_IPV6      
      if (inet_pton(AF_INET, token, &addr) > 0)
	{
	  flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV4;
	  addrlen = INADDRSZ;
	}
      else if (inet_pton(AF_INET6, token, &addr) > 0)
	{
	  flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV6;
	  addrlen = IN6ADDRSZ;
	}
#else 
     if ((addr.addr.addr4.s_addr = inet_addr(token)) != (in_addr_t) -1)
        {
          flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV4;
          addrlen = INADDRSZ;
	}
#endif
      else
	{
	  syslog(LOG_ERR, _("bad address at %s line %d"), filename, lineno); 
	  continue;
	}

     while ((token = strtok(NULL, " \t\n\r")) && (*token != '#'))
       {
	 struct crec *cache;
	 if (canonicalise(token))
	   {
	     count++;
	     /* If set, add a version of the name with a default domain appended */
	     if ((opts & OPT_EXPAND) && domain_suffix && !strchr(token, '.') && 
		 (cache = malloc(sizeof(struct crec) + 
				 strlen(token)+2+strlen(domain_suffix)-SMALLDNAME)))
	       {
		 strcpy(cache->name.sname, token);
		 strcat(cache->name.sname, ".");
		 strcat(cache->name.sname, domain_suffix);
		 add_hosts_entry(cache, &addr, addrlen, flags, index);
	       }
	     if ((cache = malloc(sizeof(struct crec) + strlen(token)+1-SMALLDNAME)))
	       {
		 strcpy(cache->name.sname, token);
		 add_hosts_entry(cache, &addr, addrlen, flags, index);
	       }
	   }
	 else
	   syslog(LOG_ERR, _("bad name at %s line %d"), filename, lineno); 
       }
    }
  
  fclose(f);

  syslog(LOG_INFO, _("read %s - %d addresses"), filename, count);
}
	    
void cache_reload(int opts, char *buff, char *domain_suffix, struct hostsfile *addn_hosts)
{
  struct crec *cache, **up, *tmp;
  int i;

  cache_inserted = cache_live_freed = 0;
  
  for (i=0; i<hash_size; i++)
    for (cache = hash_table[i], up = &hash_table[i]; cache; cache = tmp)
      {
	tmp = cache->hash_next;
	if (cache->flags & F_HOSTS)
	  {
	    *up = cache->hash_next;
	    free(cache);
	  }
	else if (!(cache->flags & F_DHCP))
	  {
	    *up = cache->hash_next;
	    if (cache->flags & F_BIGNAME)
	      {
		cache->name.bname->next = big_free;
		big_free = cache->name.bname;
	      }
	    cache->flags = 0;
	  }
	else
	  up = &cache->hash_next;
      }
  
  if ((opts & OPT_NO_HOSTS) && !addn_hosts)
    {
      if (cache_size > 0)
	syslog(LOG_INFO, _("cleared cache"));
      return;
    }

  if (!(opts & OPT_NO_HOSTS))
    read_hostsfile(HOSTSFILE, opts, buff, domain_suffix, 0);
  while (addn_hosts)
    {
      read_hostsfile(addn_hosts->fname, opts, buff, domain_suffix, addn_hosts->index);
      addn_hosts = addn_hosts->next;
    }  
} 

void cache_unhash_dhcp(void)
{
  struct crec *tmp, *cache, **up;
  int i;

  for (i=0; i<hash_size; i++)
    for (cache = hash_table[i], up = &hash_table[i]; cache; cache = cache->hash_next)
      if (cache->flags & F_DHCP)
	*up = cache->hash_next;
      else
	up = &cache->hash_next;

  /* prev field links all dhcp entries */
  for (cache = dhcp_inuse; cache; cache = tmp)
    {
      tmp = cache->prev;
      cache->prev = dhcp_spare;
      dhcp_spare = cache;
    }
    
  dhcp_inuse = NULL;
}

void cache_add_dhcp_entry(struct daemon *daemon, char *host_name, 
			  struct in_addr *host_address, time_t ttd) 
{
  struct crec *crec;
  unsigned short flags =  F_DHCP | F_FORWARD | F_IPV4 | F_REVERSE;

  if (!host_name)
    return;

  if ((crec = cache_find_by_name(NULL, host_name, 0, F_IPV4 | F_CNAME)))
    {
      if (crec->flags & F_HOSTS)
	{
	  if (crec->addr.addr.addr.addr4.s_addr != host_address->s_addr)
	    {
	      strcpy(daemon->namebuff, inet_ntoa(crec->addr.addr.addr.addr4));
	      syslog(LOG_WARNING, 
		     _("not giving name %s to the DHCP lease of %s because "
		       "the name exists in %s with address %s"), 
		     host_name, inet_ntoa(*host_address),
		     record_source(daemon->addn_hosts, crec->uid), daemon->namebuff);
	    }
	  return;
	}
      else if (!(crec->flags & F_DHCP))
	cache_scan_free(host_name, NULL, 0, crec->flags & (F_IPV4 | F_CNAME | F_FORWARD));
    }
 
  if ((crec = cache_find_by_addr(NULL, (struct all_addr *)host_address, 0, F_IPV4)))
    {
      if (crec->flags & F_NEG)
	cache_scan_free(NULL, (struct all_addr *)host_address, 0, F_IPV4 | F_REVERSE);
      else
	/* avoid multiple reverse mappings */
	flags &= ~F_REVERSE;
    }

  if ((crec = dhcp_spare))
    dhcp_spare = dhcp_spare->prev;
  else /* need new one */
    crec = malloc(sizeof(struct crec));
  
  if (crec) /* malloc may fail */
    {
      crec->flags = flags;
      if (ttd == 0)
	crec->flags |= F_IMMORTAL;
      else
	crec->ttd = ttd;
      crec->addr.addr.addr.addr4 = *host_address;
      crec->name.namep = host_name;
      crec->prev = dhcp_inuse;
      dhcp_inuse = crec;
      cache_hash(crec);
    }
}



void dump_cache(struct daemon *daemon, time_t now)
{
  syslog(LOG_INFO, _("time %lu, cache size %d, %d/%d cache insertions re-used unexpired cache entries."), 
	 (unsigned long)now, daemon->cachesize, cache_live_freed, cache_inserted); 
  
  if ((daemon->options & (OPT_DEBUG | OPT_LOG)) &&
      (addrbuff || (addrbuff = malloc(ADDRSTRLEN))))
    {
      struct crec *cache ;
      int i;
      syslog(LOG_DEBUG, "Host                                     Address                        Flags     Expires");
    
      for (i=0; i<hash_size; i++)
	for (cache = hash_table[i]; cache; cache = cache->hash_next)
	  {
	    if ((cache->flags & F_NEG) && (cache->flags & F_FORWARD))
	      addrbuff[0] = 0;
	    else if (cache->flags & F_CNAME) 
	      {
		addrbuff[0] = 0;
		addrbuff[ADDRSTRLEN-1] = 0;
		if (!is_outdated_cname_pointer(cache))
		  strncpy(addrbuff, cache_get_name(cache->addr.cname.cache), ADDRSTRLEN);
	      }
#ifdef HAVE_IPV6
	    else if (cache->flags & F_IPV4)
	      inet_ntop(AF_INET, &cache->addr.addr, addrbuff, ADDRSTRLEN);
	    else if (cache->flags & F_IPV6)
	      inet_ntop(AF_INET6, &cache->addr.addr, addrbuff, ADDRSTRLEN);
#else
            else 
	      strcpy(addrbuff, inet_ntoa(cache->addr.addr.addr.addr4));
#endif
	    syslog(LOG_DEBUG, 
#ifdef HAVE_BROKEN_RTC
		   "%-40.40s %-30.30s %s%s%s%s%s%s%s%s%s%s  %lu",
#else
		   "%-40.40s %-30.30s %s%s%s%s%s%s%s%s%s%s  %s",
#endif
		   cache_get_name(cache), addrbuff,
		   cache->flags & F_IPV4 ? "4" : "",
		   cache->flags & F_IPV6 ? "6" : "",
		   cache->flags & F_CNAME ? "C" : "",
		   cache->flags & F_FORWARD ? "F" : " ",
		   cache->flags & F_REVERSE ? "R" : " ",
		   cache->flags & F_IMMORTAL ? "I" : " ",
		   cache->flags & F_DHCP ? "D" : " ",
		   cache->flags & F_NEG ? "N" : " ",
		   cache->flags & F_NXDOMAIN ? "X" : " ",
		   cache->flags & F_HOSTS ? "H" : " ",
#ifdef HAVE_BROKEN_RTC
		   cache->flags & F_IMMORTAL ? 0: (unsigned long)(cache->ttd - now)
#else
	           cache->flags & F_IMMORTAL ? "\n" : ctime(&(cache->ttd)) 
#endif
		   );
	  }  
    } 
}

static char *record_source(struct hostsfile *addn_hosts, int index)
{
  char *source = HOSTSFILE;
  while (addn_hosts)
    { 
      if (addn_hosts->index == index)
	{
	  source = addn_hosts->fname;
	  break;
	}
      addn_hosts = addn_hosts->next;
    }

  return source;
}

void log_query(unsigned short flags, char *name, struct all_addr *addr, 
	       unsigned short type, struct hostsfile *addn_hosts, int index)
{
  char *source;
  char *verb = "is";
  char types[20];
  
  if (!log_queries)
    return;
  
  if (flags & F_NEG)
    {
      if (flags & F_REVERSE)
#ifdef HAVE_IPV6
	inet_ntop(flags & F_IPV4 ? AF_INET : AF_INET6,
		  addr, name, MAXDNAME);
#else
        strcpy(name, inet_ntoa(addr->addr.addr4));  
#endif
	      
      if (flags & F_NXDOMAIN)
	strcpy(addrbuff, "<NXDOMAIN>");
      else
	strcpy(addrbuff, "<NODATA>");
      
      if (flags & F_IPV4)
	strcat(addrbuff, "-IPv4");
      else if (flags & F_IPV6)
	strcat(addrbuff, "-IPv6");
    }
  else if (flags & F_CNAME)
    {
      /* nasty abuse of IPV4 and IPV6 flags */
      if (flags & F_IPV4)
	strcpy(addrbuff, "<MX>");
      else if (flags & F_IPV6)
	strcpy(addrbuff, "<SRV>");
      else if (flags & F_NXDOMAIN)
	strcpy(addrbuff, "<TXT>");
      else
	strcpy(addrbuff, "<CNAME>");
    }
  else
#ifdef HAVE_IPV6
    inet_ntop(flags & F_IPV4 ? AF_INET : AF_INET6,
	      addr, addrbuff, ADDRSTRLEN);
#else
    strcpy(addrbuff, inet_ntoa(addr->addr.addr4));  
#endif
    
  if (flags & F_DHCP)
    source = "DHCP";
  else if (flags & F_HOSTS)
    source = record_source(addn_hosts, index);
  else if (flags & F_CONFIG)
    source = "config";
  else if (flags & F_UPSTREAM)
    source = "reply";
  else if (flags & F_SERVER)
    {
      source = "forwarded";
      verb = "to";
    }
  else if (flags & F_QUERY)
    {
      unsigned int i;
      static const struct {
	unsigned int type;
	const char * const name;
      } typestr[] = {
	{ 1,   "A" },
	{ 2,   "NS" },
	{ 5,   "CNAME" },
	{ 6,   "SOA" },
	{ 10,  "NULL" },
	{ 11,  "WKS" },
	{ 12,  "PTR" },
	{ 13,  "HINFO" },	
        { 15,  "MX" },
	{ 16,  "TXT" },
	{ 22,  "NSAP" },
	{ 23,  "NSAP_PTR" },
	{ 24,  "SIG" },
	{ 25,  "KEY" },
	{ 28,  "AAAA" },
 	{ 33,  "SRV" },
        { 36,  "KX" },
        { 37,  "CERT" },
        { 38,  "A6" },
        { 39,  "DNAME" },
	{ 41,  "OPT" },
	{ 250, "TSIG" },
	{ 251, "IXFR" },
	{ 252, "AXFR" },
        { 253, "MAILB" },
	{ 254, "MAILA" },
	{ 255, "ANY" }
      };
      
      if (type != 0)
        {
          sprintf(types, "query[type=%d]", type); 
          for (i = 0; i < (sizeof(typestr)/sizeof(typestr[0])); i++)
	    if (typestr[i].type == type)
	      sprintf(types,"query[%s]", typestr[i].name);
	}
      source = types;
      verb = "from";
    }
  else
    source = "cached";
  
  if (strlen(name) == 0)
    name = ".";

  if ((flags & F_FORWARD) | (flags & F_NEG))
    syslog(LOG_DEBUG, "%s %s %s %s", source, name, verb, addrbuff);
  else if (flags & F_REVERSE)
    syslog(LOG_DEBUG, "%s %s is %s", source, addrbuff, name);
}

