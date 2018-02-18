/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include <stdlib.h>

#include "olsr_types.h"
#include "dllist.h"

/*------------------------------------------------------------------------------
 * Description : appends a node to the list specified by the head and tail
 *               elements
 * Parameters  : head - pointer to the head of the list
 *               tail - pointer to the tail of the list
 *               data - pointer to the data to store in the list
 * Returns     : pointer to the newly created element in the list
 * Uses data   : none
 *------------------------------------------------------------------------------
 */
struct node * append_node(struct node ** head, struct node ** tail, void * data)
{
  struct node * new = calloc(1, sizeof(struct node));

  if (*head == NULL) {
    *head = new;
  } else {
    new->prev = *tail;
    (*tail)->next = new;
  }

  new->data = data;
  *tail = new;

  return new;
}

/*------------------------------------------------------------------------------
 * Description : removes the specified element from the list specified by the
 *               head and tail elements
 * Parameters  : head - pointer to the head of the list
 *               tail - pointer to the tail of the list
 *               node - the element to remove from the list
 *               free_data - indicator whether to free the content of the data
 *               element
 * Returns     : nothing
 * Uses data   : none
 *------------------------------------------------------------------------------
 */
void remove_node(struct node ** head, struct node **tail, struct node * node, bool free_data)
{
  struct node * curr = NULL;

  for (curr = *head; curr; curr = curr->next) {
    if (curr == node) {
      // Now we found the proper node so we can remove it

      if (free_data)
        free(curr->data);

      if (curr == *head) {
        // Head node
        *head = curr->next;
      } else if (curr == *tail) {
        // Tail node
        *tail = curr->prev;
      } else {
        // Middle node
        curr->prev->next = curr->next;
        curr->next->prev = curr->prev;
      }

      if (*head != NULL)
        (*head)->prev = NULL;

      if (*tail != NULL)
        (*tail)->next = NULL;

      if (curr != NULL) {
        curr->next = curr->prev = NULL;
        free(curr);
      }
      break; // Bail out if we handled a remove
    }
  }
}

/*------------------------------------------------------------------------------
 * Description : clears the entire list specified by the head and tail elements
 * Parameters  : head - pointer to the head of the list
 *               tail - pointer to the tail of the list
 *               free_data - indicator whether to free the data pointer
 * Returns     : nothing
 * Uses data   : none
 *------------------------------------------------------------------------------
 */
void clear_list(struct node **head, struct node **tail, bool free_data)
{
  while (*head)
    remove_node(head, tail, *head, free_data);
}


