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

#include "olsr.h"
#include "defs.h"
#include "olsr_cookie.h"
#include "log.h"

#include <assert.h>

/* Root directory of the cookies we have in the system */
static struct olsr_cookie_info *cookies[COOKIE_ID_MAX] = { 0 };

/*
 * Allocate a cookie for the next available cookie id.
 */
struct olsr_cookie_info *
olsr_alloc_cookie(const char *cookie_name, olsr_cookie_type cookie_type)
{
  struct olsr_cookie_info *ci;
  int ci_index;

  /*
   * Look for an unused index.
   * For ease of troubleshooting (non-zero patterns) we start at index 1.
   */
  for (ci_index = 1; ci_index < COOKIE_ID_MAX; ci_index++) {
    if (!cookies[ci_index]) {
      break;
    }
  }

  /* 1 <= ci_index <= COOKIE_ID_MAX */

  if (ci_index == COOKIE_ID_MAX) {
    olsr_exit("No more cookies available", EXIT_FAILURE);
    return NULL;
  }

  ci = calloc(1, sizeof(struct olsr_cookie_info));
  cookies[ci_index] = ci;

  /* Now populate the cookie info */
  ci->ci_id = ci_index;
  ci->ci_type = cookie_type;
  if (cookie_name) {
    ci->ci_name = strdup(cookie_name);
  }

  /* Init the free list */
  if (cookie_type == OLSR_COOKIE_TYPE_MEMORY) {
    list_head_init(&ci->ci_free_list);
  }

  return ci;
}

/*
 * Free a cookie that is no longer being used.
 */
void
olsr_free_cookie(struct olsr_cookie_info *ci)
{
  struct list_node *memory_list;

  /* Mark the cookie as unused */
  cookies[ci->ci_id] = NULL;

  /* Free name if set */
  if (ci->ci_name) {
    free(ci->ci_name);
  }

  /* Flush all the memory on the free list */
  if (ci->ci_type == OLSR_COOKIE_TYPE_MEMORY) {
    while (!list_is_empty(&ci->ci_free_list)) {
      memory_list = ci->ci_free_list.next;
      list_remove(memory_list);
      free(memory_list);
    }
  }

  free(ci);
}

/*
 * Flush all cookies. This is really only called upon shutdown.
 */
void
olsr_delete_all_cookies(void)
{
  int ci_index;

  /*
   * Walk the full index range and kill 'em all.
   */
  for (ci_index = 1; ci_index < COOKIE_ID_MAX; ci_index++) {
    if (!cookies[ci_index]) {
      continue;
    }
    olsr_free_cookie(cookies[ci_index]);
  }
}

/*
 * Set the size for fixed block allocations.
 * This is only allowed for memory cookies.
 */
void
olsr_cookie_set_memory_size(struct olsr_cookie_info *ci, size_t size)
{
  if (!ci) {
    return;
  }

  assert(ci->ci_type == OLSR_COOKIE_TYPE_MEMORY);
  ci->ci_size = size;
}

/*
 * Basic sanity checking for a passed-in cookie-id.
 */
static bool
olsr_cookie_valid(olsr_cookie_t cookie_id)
{
  if ((cookie_id < COOKIE_ID_MAX) && cookies[cookie_id]) {
    return true;
  }
  return false;
}

/*
 * Increment usage state for a given cookie.
 */
void
olsr_cookie_usage_incr(olsr_cookie_t cookie_id)
{
  if (olsr_cookie_valid(cookie_id)) {
    cookies[cookie_id]->ci_usage++;
    cookies[cookie_id]->ci_changes++;
  }
}

/*
 * Decrement usage state for a given cookie.
 */
void
olsr_cookie_usage_decr(olsr_cookie_t cookie_id)
{
  if (olsr_cookie_valid(cookie_id)) {
    cookies[cookie_id]->ci_usage--;
    cookies[cookie_id]->ci_changes++;
  }
}

/*
 * Return a cookie name.
 * Mostly used for logging purposes.
 */
char *
olsr_cookie_name(olsr_cookie_t cookie_id)
{
  static char unknown[] = "unknown";

  if (olsr_cookie_valid(cookie_id)) {
    return (cookies[cookie_id])->ci_name;
  }

  return unknown;
}

/*
 * Allocate a fixed amount of memory based on a passed in cookie type.
 */
void *
olsr_cookie_malloc(struct olsr_cookie_info *ci)
{
  void *ptr;
  struct olsr_cookie_mem_brand *branding;
  struct list_node *free_list_node;

#ifdef OLSR_COOKIE_DEBUG
  bool reuse = false;
#endif /* OLSR_COOKIE_DEBUG */

  /*
   * Check first if we have reusable memory.
   */
  if (!ci->ci_free_list_usage) {

    /*
     * No reusable memory block on the free_list.
     */
    ptr = calloc(1, ci->ci_size + sizeof(struct olsr_cookie_mem_brand));

    if (!ptr) {
      char buf[1024];
      snprintf(buf, sizeof(buf), "%s: out of memory: %s", ci->ci_name, strerror(errno));
      olsr_exit(buf, EXIT_FAILURE);
    }
    assert(ptr);
  } else {

    /*
     * There is a memory block on the free list.
     * Carve it out of the list, and clean.
     */
    free_list_node = ci->ci_free_list.next;
    list_remove(free_list_node);
    ptr = (void *)free_list_node;
    memset(ptr, 0, ci->ci_size);
    ci->ci_free_list_usage--;
#ifdef OLSR_COOKIE_DEBUG
    reuse = true;
#endif /* OLSR_COOKIE_DEBUG */
  }

  /*
   * Now brand mark the end of the memory block with a short signature
   * indicating presence of a cookie. This will be checked against
   * When the block is freed to detect corruption.
   */
  branding = (struct olsr_cookie_mem_brand *)ARM_NOWARN_ALIGN(((unsigned char *)ptr + ci->ci_size));
  memcpy(&branding->cmb_sig[0], "cookie", 6);
  branding->cmb_id = ci->ci_id;

  /* Stats keeping */
  olsr_cookie_usage_incr(ci->ci_id);

#ifdef OLSR_COOKIE_DEBUG
  OLSR_PRINTF(1, "MEMORY: alloc %s, %p, %u bytes%s\n", ci->ci_name, ptr, ci->ci_size, reuse ? ", reuse" : "");
#endif /* OLSR_COOKIE_DEBUG */

  return ptr;
}

/*
 * Free a memory block owned by a given cookie.
 * Run some corruption checks.
 */
void
olsr_cookie_free(struct olsr_cookie_info *ci, void *ptr)
{
  struct olsr_cookie_mem_brand *branding;
  struct list_node *free_list_node;

#ifdef OLSR_COOKIE_DEBUG
  bool reuse = false;
#endif /* OLSR_COOKIE_DEBUG */

  branding = (struct olsr_cookie_mem_brand *)ARM_NOWARN_ALIGN(((unsigned char *)ptr + ci->ci_size));

  /*
   * Verify if there has been a memory overrun, or
   * the wrong owner is trying to free this.
   */
  assert(!memcmp(&branding->cmb_sig, "cookie", 6));
  assert(branding->cmb_id == ci->ci_id);

  /* Kill the brand */
  memset(branding, 0, sizeof(*branding));

  /*
   * Rather than freeing the memory right away, try to reuse at a later
   * point. Keep at least ten percent of the active used blocks or at least
   * ten blocks on the free list.
   */
  if ((ci->ci_free_list_usage < COOKIE_FREE_LIST_THRESHOLD) || (ci->ci_free_list_usage < ci->ci_usage / COOKIE_FREE_LIST_THRESHOLD)) {

    free_list_node = (struct list_node *)ptr;
    list_node_init(free_list_node);
    list_add_before(&ci->ci_free_list, free_list_node);
    ci->ci_free_list_usage++;
#ifdef OLSR_COOKIE_DEBUG
    reuse = true;
#endif /* OLSR_COOKIE_DEBUG */
  } else {

    /*
     * No interest in reusing memory.
     */
    free(ptr);
  }

  /* Stats keeping */
  olsr_cookie_usage_decr(ci->ci_id);

#ifdef OLSR_COOKIE_DEBUG
  OLSR_PRINTF(1, "MEMORY: free %s, %p, %u bytes%s\n", ci->ci_name, ptr, ci->ci_size, reuse ? ", reuse" : "");
#endif /* OLSR_COOKIE_DEBUG */

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
