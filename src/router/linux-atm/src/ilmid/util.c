/*
 * util.c - Various utility procedures
 *
 * Written by Scott W. Shumate
 * 
 * Copyright (c) 1995-97 All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * I ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIM ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "util.h"
#include "atmd.h"

AsnOidResult AsnOidCompare(AsnOid *o1, AsnOid *o2)
{
  int result;

  if(o1->octetLen > o2->octetLen)
  {
    result = memcmp(o1->octs, o2->octs, o2->octetLen);
    if(result < 0)
      return AsnOidLess;
    else
      return AsnOidGreater;
  } else if(o1->octetLen < o2->octetLen)
  {
    result = memcmp(o1->octs, o2->octs, o1->octetLen);
    if(result > 0)
      return AsnOidGreater;
    else if(result == 0)
      return AsnOidRoot;
    else
      return AsnOidLess;
  } else
  {
    result = memcmp(o1->octs, o2->octs, o1->octetLen);
    if(result < 0)
      return AsnOidLess;
    else if(result == 0)
      return AsnOidEqual;
    else
      return AsnOidGreater;
  }
}

int AsnOidSize(AsnOid *oid)
{
  int n, size;

  for(n = 1, size = 2; n < oid->octetLen; n++)
    if(!(oid->octs[n] & 0x80))
      size++;

  return size;
}


VarBind *AppendVarBind(VarBindList *list)
{
  AsnListNode *newNode;
  VarBind *entry;
  
  newNode  = alloc_t(AsnListNode);
  newNode->data = alloc_t(VarBind);
  newNode->next = NULL;

  if(list->last == NULL)
    {
      newNode->prev = NULL;
      list->first = newNode;
      list->last = newNode;
    }
  else
    {
      newNode->prev = list->last;
      list->last->next = newNode;
      list->last = newNode;
    }
  
  list->curr = newNode;
  list->count++;
  
  return newNode->data;
}

void AppendListNode(VarBindList *list, AsnListNode *node)
{
  node->next = NULL;
  if(list->last == NULL)
    {
      node->prev = NULL;
      list->first = node;
      list->last = node;
    }
  else
    {
      node->prev = list->last;
      list->last->next = node;
      list->last = node;
    }
  
  list->curr = node;
  list->count++;
  
  return;
}
