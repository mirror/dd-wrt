#include <stdio.h>
#include <stdlib.h>
#include "util.h"
 void
list_add (set_t ** ptr_list, set_t * ptr_node) 
{
  set_t * ptr_head;
  if (*ptr_list == NULL)
    {
      ptr_node->next = NULL;
      *ptr_list = ptr_node;
      return;
    }
  ptr_head = *ptr_list;
  ptr_node->next = ptr_head;
  *ptr_list = ptr_node;
  return;
}
void
list_cat (set_t ** ptr_dst, set_t ** ptr_src) 
{
  set_t * ptr_node;
  set_t * ptr_prev;
  if (*ptr_src == NULL)
    return;
  if (*ptr_dst == NULL)
    {
      *ptr_dst = *ptr_src;
      return;
    }
  ptr_node = *ptr_dst;
  ptr_prev = NULL;
  while (ptr_node != NULL)
    {
      ptr_prev = ptr_node;
      ptr_node = ptr_node->next;
    }
  ptr_prev->next = *ptr_src;
  return;
}

set_t * list_remove (set_t ** ptr_list) 
{
  set_t * ptr_head;
  set_t * ptr_node;
  if (*ptr_list == NULL)
    return NULL;
  ptr_head = *ptr_list;
  ptr_node = *ptr_list;
  ptr_head = ptr_head->next;
  *ptr_list = ptr_head;
  ptr_node->next = NULL;
  return ptr_node;
}
int
list_remove_node (set_t ** ptr_list, set_t * ptr_remove) 
{
  set_t * ptr_node;
  set_t * ptr_next;
  set_t * ptr_prev;
  ptr_node = *ptr_list;
  ptr_prev = NULL;
  ptr_next = ptr_node->next;
  while (ptr_node != NULL)
    {
      if (ptr_node == ptr_remove)
	break;
      ptr_prev = ptr_node;
      ptr_node = ptr_node->next;
      ptr_next = ptr_node->next;
    }
  if (ptr_node == NULL)
    return -1;
  if ((ptr_prev == NULL) && (ptr_next == NULL))
    {
      *ptr_list = NULL;
      return 0;
    }
  if ((ptr_prev == NULL) && (ptr_next != NULL))
    {
      ptr_remove->next = NULL;
      *ptr_list = ptr_next;
      return 0;
    }
  ptr_remove->next = NULL;
  ptr_prev->next = ptr_next;
  return 0;
}

set_t * list_get_head (set_t ** ptr_list) 
{
  return *ptr_list;
}

set_t * list_get_next (set_t * ptr_list) 
{
  return ptr_list->next;
}


/*
 * Compute the inet checksum
 */ 
  unsigned short
in_cksum (unsigned short *addr, int len) 
{
  int nleft = len;
  int sum = 0;
  unsigned short *w = addr;
  unsigned short answer = 0;
  while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }
  if (nleft == 1)
    {
      *(unsigned char *) (&answer) = *(unsigned char *) w;
      sum += answer;
    }
  sum = (sum >> 16) + (sum & 0xffff);
  answer = ~sum;
  return (answer);
}


