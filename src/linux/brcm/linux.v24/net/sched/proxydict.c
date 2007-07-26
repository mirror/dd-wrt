#ifndef __KERNEL__
#include <string.h>
#include <netinet/in.h>
#endif

#include "proxyremap.h"
#include "proxydict.h"


/*--------------------------------------------------------------------------
Implementation.
*/

// Hash function
#define hash_fnc(m,server,port,proto) \
 (((proto)*7+(server)*13+(port)*5)%m->hash_size)

// Size of hash table given maximal number of connections:
#define hash_size_max_con(max_con) (2*(max_con))

// The memory area we maintain:
typedef struct {
  int hash_size;
  int max_con;
  int cur_con;
  
  int free_first;
  
  // Then we have:
  //   int hash_table[hash_size];
  //   int next[max_con];
  //   ProxyRemapBlock info[max_con];
  //
  // The idea is the following:
  //   Given a connection we map it by hash_fnc into hash_table. This gives an 
  //   index in next which contains a -1 terminated linked list of connections 
  //   mapping to that hash value.
  //
  //   The entries in next not allocated is also in linked list where 
  //   the first free index is free_first.
} memory;  

#define Memory(m)     ((memory*)m)
#define Hash_table(m) ((int*)(((char*)m)+sizeof(memory)))
#define Next(m)       ((int*)(((char*)m)+sizeof(memory)+     \
                       sizeof(int)*((memory*)m)->hash_size))
#define Info(m)       ((ProxyRemapBlock*)(((char*)m)+                          \
                                           sizeof(memory)+                     \
                                           sizeof(int)*((memory*)m)->hash_size+\
					   sizeof(int)*((memory*)m)->max_con   \
					  ))

int proxyGetMemSize(int max_con) {
  return sizeof(memory)+
         sizeof(int)*hash_size_max_con(max_con)+
	 sizeof(int)*max_con+
	 sizeof(ProxyRemapBlock)*max_con;
}

void proxyInitMem(void* data, int max_con) {
  // Init m:
  memory* m=Memory(data);
  m->max_con=max_con;
  m->cur_con=0;
  m->hash_size=hash_size_max_con(max_con);

  {
    // Get pointers:
    int* hash_table=Hash_table(data);
    int* next=Next(data);
    int i;
  
    // Init the hash table:
    for(i=0; i<m->hash_size; i++) hash_table[i]=-1;
  
    // Init the free-list
    for(i=0; i<m->max_con; i++) next[i]=i+1;
    m->free_first=0;
  }
}  
  
int proxyGetCurConn(void* data) {
  return Memory(data)->cur_con;
}

int proxyGetMaxConn(void* data) {
  return Memory(data)->max_con;
}

ProxyRemapBlock* proxyLookup(void* data, unsigned ipaddr, unsigned short port, char proto) {    
  memory* m=Memory(data);
  int* hash_table=Hash_table(m);
  int* next=Next(m);
  ProxyRemapBlock* info=Info(m);
  int i;
  
  for(i=hash_table[hash_fnc(m,ipaddr,port,proto)]; i!=-1; i=next[i]) {
    if(info[i].proto==proto &&
       info[i].sport==port &&
       info[i].saddr==ipaddr) return &info[i];
  }
       
  return 0;
}    

int proxyConsumeBlock(void* data, ProxyRemapBlock* blk) {
  memory* m=Memory(data);
  int* hash_table=Hash_table(m);
  int* next=Next(m);
  ProxyRemapBlock* info=Info(m);
  int hash=hash_fnc(m,blk->saddr,blk->sport,blk->proto);
  int foo;
  
  if(blk->open) {
    if(m->cur_con == m->max_con) return -1;
    
    // Insert the block at a free entry:
    info[m->free_first]=*blk;
    m->cur_con++;

    foo=next[m->free_first];
    
    // And insert it in the hash tabel:
    next[m->free_first]=hash_table[hash];
    hash_table[hash]=m->free_first;
    m->free_first=foo;
  } else {
    int* toupdate;
    
    // Find the block
    for(toupdate=&hash_table[hash]; 
        *toupdate!=-1; 
	toupdate=&next[*toupdate]) {
      if(info[*toupdate].proto==blk->proto &&
         info[*toupdate].sport==blk->sport &&
         info[*toupdate].saddr==blk->saddr) break;
    }
    if(*toupdate==-1) return -1;

    foo=*toupdate;
    
    // Delete it from the hashing list:    
    *toupdate=next[*toupdate];
    
    // And put it on the free list:
    next[foo]=m->free_first;
    m->free_first=foo;

    m->cur_con--;
  }
  
  return 0;
}
