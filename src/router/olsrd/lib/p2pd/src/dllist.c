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


