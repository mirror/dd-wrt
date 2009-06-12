/* 
 * author padraigo
 *
 * simple dictionary 
 */

#include <sys/types.h>

typedef struct bp_note_s {
  uint addr, instr ;
  void *nxt, *prv ;
} bp_node ;

extern bp_node *first;

int ins(uint,uint);
int del(uint);

