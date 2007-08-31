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

static struct crec *cache_head = NULL, *cache_tail = NULL, **hash_table = NULL;
static struct crec *dhcp_spare = NULL, *new_chain = NULL;
static int cache_inserted = 0, cache_live_freed = 0, insert_error;
static union bigname *big_free = NULL;
static int bignames_left, hash_size;
static int uid = 0;
static char *addrbuff = NULL;

/* type->string mapping: this is also used by the name-hash function as a mixing table. */
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
  { 48,  "DNSKEY" },
  { 249, "TKEY" },
  { 250, "TSIG" },
  { 251, "IXFR" },
  { 252, "AXFR" },
  { 253, "MAILB" },
  { 254, "MAILA" },
  { 255, "ANY" }
};

static void cache_free(struct crec *crecp);
static void cache_unlink(struct crec *crecp);
static void cache_link(struct crec *crecp);
static char *record_source(struct hostsfile *add_hosts, int index);
static void rehash(int size);
static void cache_hash(struct crec *crecp);

void cache_init(void)
{
  struct crec *crecp;
  int i;

  if (daemon->options & OPT_LOG)
    addrbuff = safe_malloc(ADDRSTRLEN);
  
  bignames_left = daemon->cachesize/10;
  
  if (daemon->cachesize > 0)
    {
      crecp = safe_malloc(daemon->cachesize*sizeof(struct crec));
      
      for (i=0; i < daemon->cachesize; i++, crecp++)
	{
	  cache_link(crecp);
	  crecp->flags = 0;
	  crecp->uid = uid++;
	}
    }
  
  /* create initial hash table*/
  rehash(daemon->cachesize);
}

/* In most cases, we create the hash table once here by calling this with (hash_table == NULL)
   but if the hosts file(s) are big (some people have 50000 ad-block entries), the table
   will be much too small, so the hosts reading code calls rehash every 1000 addresses, to
   expand the table. */
static void rehash(int size)
{
  struct crec **new, **old, *p, *tmp;
  int i, new_size, old_size;

  /* hash_size is a power of two. */
  for (new_size = 64; new_size < size/10; new_size = new_size << 1);
  
  /* must succeed in getting first instance, failure later is non-fatal */
  if (!hash_table)
    new = safe_malloc(new_size * sizeof(struct crec *));
  else if (new_size <= hash_size || !(new = whine_malloc(new_size * sizeof(struct crec *))))
    return;

  for(i = 0; i < new_size; i++)
    new[i] = NULL;

  old = hash_table;
  old_size = hash_size;
  hash_table = new;
  hash_size = new_size;
  
  if (old)
    {
      for (i = 0; i < old_size; i++)
	for (p = old[i]; p ; p = tmp)
	  {
	    tmp = p->hash_next;
	    cache_hash(p);
	  }
      free(old);
    }
}
  
static struct crec **hash_bucket(char *name)
{
  unsigned int c, val = 017465; /* Barker code - minimum self-correlation in cyclic shift */
  const unsigned char *mix_tab = (const unsigned char*)typestr; 

  while((c = (unsigned char) *name++))
    {
      /* don't use tolower and friends here - they may be messed up by LOCALE */
      if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
      val = ((val << 7) | (val >> (32 - 7))) + (mix_tab[(val + c) & 0x3F] ^ c);
    } 
  
  /* hash_size is a power of two */
  return hash_table + ((val ^ (val >> 16)) & (hash_size - 1));
}

static void cache_hash(struct crec *crecp)
{
  /* maintain an invariant that all entries with F_REVERSE set
     are at the start of the hash-chain  and all non-reverse
     immortal entries are at the end of the hash-chain.
     This allows reverse searches and garbage collection to be optimised */

  struct crec **up = hash_bucket(cache_get_name(crecp));

  if (!(crecp->flags & F_REVERSE))
    {
      while (*up && ((*up)->flags & F_REVERSE))
	up = &((*up)->hash_next); 
      
      if (crecp->flags & F_IMMORTAL)
	while (*up && !((*up)->flags & F_IMMORTAL))
	  up = &((*up)->hash_next);
    }
  crecp->hash_next = *up;
  *up = crecp;
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
  if (!(crecp->flags & F_CNAME))
    return 0;
  
  if (crecp->addr.cname.cache && crecp->addr.cname.uid == crecp->addr.cname.cache->uid)
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
     name exists in the cache as a HOSTS or DHCP entry (these are never deleted)

     We take advantage of the fact that hash chains have stuff in the order <reverse>,<other>,<immortal>
     so that when we hit an entry which isn't reverse and is immortal, we're done. */
 
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
	for (crecp = hash_table[i], up = &hash_table[i]; 
	     crecp && ((crecp->flags & F_REVERSE) || !(crecp->flags & F_IMMORTAL));
	     crecp = crecp->hash_next)
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
  struct crec *new;
  union bigname *big_name = NULL;
  int freed_all = flags & F_REVERSE;

  log_query(flags | F_UPSTREAM, name, addr, 0, NULL, 0);

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
		 !(big_name = (union bigname *)whine_malloc(sizeof(union bigname))))
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
    new->addr.addr = *addr;
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
	 also free anything which has expired. All the reverse entries are at the
	 start of the hash chain, so we can give up when we find the first 
	 non-REVERSE one.  */
       int i;
       struct crec **up, **chainp = &ans;
       
       for (i=0; i<hash_size; i++)
	 for (crecp = hash_table[i], up = &hash_table[i]; 
	      crecp && (crecp->flags & F_REVERSE);
	      crecp = crecp->hash_next)
	   if (!is_expired(now, crecp))
	     {      
	       if ((crecp->flags & prot) &&
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
			    unsigned short flags, int index, int addr_dup)
{
  struct crec *lookup = cache_find_by_name(NULL, cache->name.sname, 0, flags & (F_IPV4 | F_IPV6));
  int i;
  
  /* Remove duplicates in hosts files. */
  if (lookup && (lookup->flags & F_HOSTS) &&
      memcmp(&lookup->addr.addr, addr, addrlen) == 0)
    free(cache);
  else
    {
      /* Ensure there is only one address -> name mapping (first one trumps) 
	 We do this by steam here, first we see if the address is the same as
	 the last one we saw, which eliminates most in the case of an ad-block 
	 file with thousands of entries for the same address.
	 Then we search and bail at the first matching address that came from
	 a HOSTS file. Since the first host entry gets reverse, we know 
	 then that it must exist without searching exhaustively for it. */
     
      if (addr_dup)
	flags &= ~F_REVERSE;
      else
	for (i=0; i<hash_size; i++)
	  {
	    for (lookup = hash_table[i]; lookup; lookup = lookup->hash_next)
	      if ((lookup->flags & F_HOSTS) && 
		  (lookup->flags & flags & (F_IPV4 | F_IPV6)) &&
		  memcmp(&lookup->addr.addr, addr, addrlen) == 0)
		{
		  flags &= ~F_REVERSE;
		  break;
		}
	    if (lookup)
	      break;
	  }
      
      cache->flags = flags;
      cache->uid = index;
      memcpy(&cache->addr.addr, addr, addrlen);
      cache_hash(cache);
    }
}

static int read_hostsfile(char *filename, int opts, char *buff, char *domain_suffix, int index, int cache_size)
{  
  FILE *f = fopen(filename, "r");
  char *line;
  int addr_count = 0, name_count = cache_size, lineno = 0;
  unsigned short flags, saved_flags = 0;
  struct all_addr addr, saved_addr;

  if (!f)
    {
      my_syslog(LOG_ERR, _("failed to load names from %s: %s"), filename, strerror(errno));
      return 0;
    }
    
  while ((line = fgets(buff, MAXDNAME, f)))
    {
      char *token = strtok(line, " \t\n\r");
      int addrlen, addr_dup = 0;
              
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
	  my_syslog(LOG_ERR, _("bad address at %s line %d"), filename, lineno); 
	  continue;
	}

     if (saved_flags == flags && memcmp(&addr, &saved_addr, addrlen) == 0)
       addr_dup = 1;
     else
       {
	 saved_flags = flags;
	 saved_addr = addr;
       }
     
     addr_count++;
     
     /* rehash every 1000 names. */
     if ((name_count - cache_size) > 1000)
       {
	 rehash(name_count);
	 cache_size = name_count;
       }
     
     while ((token = strtok(NULL, " \t\n\r")) && (*token != '#'))
       {
	 struct crec *cache;
	 int fqdn = !!strchr(token, '.');
	 if (canonicalise(token))
	   {
	     /* If set, add a version of the name with a default domain appended */
	     if ((opts & OPT_EXPAND) && domain_suffix && !fqdn && 
		 (cache = whine_malloc(sizeof(struct crec) + 
				       strlen(token)+2+strlen(domain_suffix)-SMALLDNAME)))
	       {
		 strcpy(cache->name.sname, token);
		 strcat(cache->name.sname, ".");
		 strcat(cache->name.sname, domain_suffix);
		 add_hosts_entry(cache, &addr, addrlen, flags, index, addr_dup);
		 addr_dup = 1;
		 name_count++;
	       }
	     if ((cache = whine_malloc(sizeof(struct crec) + strlen(token)+1-SMALLDNAME)))
	       {
		 strcpy(cache->name.sname, token);
		 add_hosts_entry(cache, &addr, addrlen, flags, index, addr_dup);
		 name_count++;
	       }
	   }
	 else
	   my_syslog(LOG_ERR, _("bad name at %s line %d"), filename, lineno); 
       }
    }
  
  fclose(f);
  rehash(name_count);

  my_syslog(LOG_INFO, _("read %s - %d addresses"), filename, addr_count);

  return name_count;
}
	    
void cache_reload(int opts, char *buff, char *domain_suffix, struct hostsfile *addn_hosts)
{
  struct crec *cache, **up, *tmp;
  int i, total_size = daemon->cachesize;

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
      if (daemon->cachesize > 0)
	my_syslog(LOG_INFO, _("cleared cache"));
      return;
    }

  if (!(opts & OPT_NO_HOSTS))
    total_size = read_hostsfile(HOSTSFILE, opts, buff, domain_suffix, 0, total_size);
  while (addn_hosts)
    {
      total_size = read_hostsfile(addn_hosts->fname, opts, buff, domain_suffix, addn_hosts->index, total_size);
      addn_hosts = addn_hosts->next;
    }  
} 

void cache_unhash_dhcp(void)
{
  struct crec *cache, **up;
  int i;

  for (i=0; i<hash_size; i++)
    for (cache = hash_table[i], up = &hash_table[i]; cache; cache = cache->hash_next)
      if (cache->flags & F_DHCP)
	{
	  *up = cache->hash_next;
	  cache->next = dhcp_spare;
	  dhcp_spare = cache;
	}
      else
	up = &cache->hash_next;
}

void cache_add_dhcp_entry(char *host_name, 
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
	      my_syslog(LOG_WARNING, 
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
    dhcp_spare = dhcp_spare->next;
  else /* need new one */
    crec = whine_malloc(sizeof(struct crec));
  
  if (crec) /* malloc may fail */
    {
      crec->flags = flags;
      if (ttd == 0)
	crec->flags |= F_IMMORTAL;
      else
	crec->ttd = ttd;
      crec->addr.addr.addr.addr4 = *host_address;
      crec->name.namep = host_name;
      cache_hash(crec);
    }
}

void dump_cache(time_t now)
{
  my_syslog(LOG_INFO, _("time %lu, cache size %d, %d/%d cache insertions re-used unexpired cache entries."), 
	    (unsigned long)now, daemon->cachesize, cache_live_freed, cache_inserted); 
  
  if ((daemon->options & (OPT_DEBUG | OPT_LOG)) &&
      (addrbuff || (addrbuff = whine_malloc(ADDRSTRLEN))))
    {
      struct crec *cache ;
      int i;
      my_syslog(LOG_DEBUG, "Host                                     Address                        Flags     Expires");
    
      for (i=0; i<hash_size; i++)
	for (cache = hash_table[i]; cache; cache = cache->hash_next)
	  {
	    char *a, *p = daemon->namebuff;
	    p += sprintf(p, "%-40.40s ", cache_get_name(cache));
	    if ((cache->flags & F_NEG) && (cache->flags & F_FORWARD))
	      a = ""; 
	    else if (cache->flags & F_CNAME) 
	      {
		a = "";
		if (!is_outdated_cname_pointer(cache))
		  a = cache_get_name(cache->addr.cname.cache);
	      }
#ifdef HAVE_IPV6
	    else 
	      { 
		a = addrbuff;
		if (cache->flags & F_IPV4)
		  inet_ntop(AF_INET, &cache->addr.addr, addrbuff, ADDRSTRLEN);
		else if (cache->flags & F_IPV6)
		  inet_ntop(AF_INET6, &cache->addr.addr, addrbuff, ADDRSTRLEN);
	      }
#else
            else 
	      a = inet_ntoa(cache->addr.addr.addr.addr4);
#endif
	    p += sprintf(p, "%-30.30s %s%s%s%s%s%s%s%s%s%s  ", a, 
			 cache->flags & F_IPV4 ? "4" : "",
			 cache->flags & F_IPV6 ? "6" : "",
			 cache->flags & F_CNAME ? "C" : "",
			 cache->flags & F_FORWARD ? "F" : " ",
			 cache->flags & F_REVERSE ? "R" : " ",
			 cache->flags & F_IMMORTAL ? "I" : " ",
			 cache->flags & F_DHCP ? "D" : " ",
			 cache->flags & F_NEG ? "N" : " ",
			 cache->flags & F_NXDOMAIN ? "X" : " ",
			 cache->flags & F_HOSTS ? "H" : " ");
#ifdef HAVE_BROKEN_RTC
	    p += sprintf(p, "%lu", cache->flags & F_IMMORTAL ? 0: (unsigned long)(cache->ttd - now));
#else
	    p += sprintf(p, "%s", cache->flags & F_IMMORTAL ? "\n" : ctime(&(cache->ttd)));
	    /* ctime includes trailing \n - eat it */
	    *(p-1) = 0;
#endif
	    my_syslog(LOG_DEBUG, daemon->namebuff);
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
  char *source, *dest = addrbuff;
  char *verb = "is";
  char types[20];
  
  if (!(daemon->options & OPT_LOG))
    return;

  if (addr)
    {
#ifdef HAVE_IPV6
      inet_ntop(flags & F_IPV4 ? AF_INET : AF_INET6,
		addr, addrbuff, ADDRSTRLEN);
#else
      strncpy(addrbuff, inet_ntoa(addr->addr.addr4), ADDRSTRLEN);  
#endif
    }

  if (flags & F_REVERSE)
    {
      dest = name;
      name = addrbuff;
    }
  
  if (flags & F_NEG)
    {
      if (flags & F_NXDOMAIN)
	{
	  if (flags & F_IPV4)
	    dest = "NXDOMAIN-IPv4";
	  else 
	    dest = "NXDOMAIN-IPv6"; 
	}
      else
	{      
	  if (flags & F_IPV4)
	    dest = "NODATA-IPv4";
	  else 
	    dest = "NODATA-IPv6";
	}
    }
  else if (flags & F_CNAME)
    {
      /* nasty abuse of IPV4 and IPV6 flags */
      if (flags & F_IPV4)
	dest = "<MX>";
      else if (flags & F_IPV6)
	dest = "<SRV>";
      else if (flags & F_NXDOMAIN)
	dest = "<TXT>";
      else if (flags & F_BIGNAME)
	dest = "<PTR>";
      else
	dest = "<CNAME>";
    }
    
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

  my_syslog(LOG_DEBUG, "%s %s %s %s", source, name, verb, dest);
}

