/*
 *	Filters: Trie for prefix sets
 *
 *	Copyright 2009 Ondrej Zajicek <santiago@crfreenet.org>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Trie for prefix sets
 *
 * We use a (compressed) trie to represent prefix sets. Every node
 * in the trie represents one prefix (&addr/&plen) and &plen also
 * indicates the index of the bit in the address that is used to
 * branch at the node. If we need to represent just a set of
 * prefixes, it would be simple, but we have to represent a
 * set of prefix patterns. Each prefix pattern consists of
 * &ppaddr/&pplen and two integers: &low and &high, and a prefix
 * &paddr/&plen matches that pattern if the first MIN(&plen, &pplen)
 * bits of &paddr and &ppaddr are the same and &low <= &plen <= &high.
 *
 * We use a bitmask (&accept) to represent accepted prefix lengths
 * at a node. As there are 33 prefix lengths (0..32 for IPv4), but
 * there is just one prefix of zero length in the whole trie so we 
 * have &zero flag in &f_trie (indicating whether the trie accepts
 * prefix 0.0.0.0/0) as a special case, and &accept bitmask
 * represents accepted prefix lengths from 1 to 32.
 *
 * There are two cases in prefix matching - a match when the length
 * of the prefix is smaller that the length of the prefix pattern,
 * (&plen < &pplen) and otherwise. The second case is simple - we
 * just walk through the trie and look at every visited node
 * whether that prefix accepts our prefix length (&plen). The
 * first case is tricky - we don't want to examine every descendant
 * of a final node, so (when we create the trie) we have to propagate
 * that information from nodes to their ascendants.
 *
 * Suppose that we have two masks (M1 and M2) for a node. Mask M1
 * represents accepted prefix lengths by just the node and mask M2
 * represents accepted prefix lengths by the node or any of its
 * descendants. Therefore M2 is a bitwise or of M1 and children's
 * M2 and this is a maintained invariant during trie building.
 * Basically, when we want to match a prefix, we walk through the trie,
 * check mask M1 for our prefix length and when we came to
 * final node, we check mask M2.
 *
 * There are two differences in the real implementation. First,
 * we use a compressed trie so there is a case that we skip our
 * final node (if it is not in the trie) and we came to node that
 * is either extension of our prefix, or completely out of path
 * In the first case, we also have to check M2.
 *
 * Second, we really need not to maintain two separate bitmasks.
 * Checks for mask M1 are always larger than &applen and we need
 * just the first &pplen bits of mask M2 (if trie compression
 * hadn't been used it would suffice to know just $applen-th bit),
 * so we have to store them together in &accept mask - the first
 * &pplen bits of mask M2 and then mask M1.
 *
 * There are four cases when we walk through a trie:
 *
 * - we are in NULL
 * - we are out of path (prefixes are inconsistent)
 * - we are in the wanted (final) node (node length == &plen)
 * - we are beyond the end of path (node length > &plen)
 * - we are still on path and keep walking (node length < &plen)
 *
 * The walking code in trie_match_prefix() is structured according to
 * these cases.
 */

#include "nest/bird.h"
#include "lib/string.h"
#include "conf/conf.h"
#include "filter/filter.h"

/**
 * f_new_trie
 *
 * Allocates and returns a new empty trie.
 */
struct f_trie *
f_new_trie(linpool *lp)
{
  struct f_trie * ret;
  ret = lp_allocz(lp, sizeof(struct f_trie));
  ret->lp = lp;
  return ret;
}

static inline struct f_trie_node *
new_node(struct f_trie *t, int plen, ip_addr paddr, ip_addr pmask, ip_addr amask)
{
  struct f_trie_node *n = lp_allocz(t->lp, sizeof(struct f_trie_node));
  n->plen = plen;
  n->addr = paddr;
  n->mask = pmask;
  n->accept = amask;
  return n;
}

static inline void
attach_node(struct f_trie_node *parent, struct f_trie_node *child)
{
  parent->c[ipa_getbit(child->addr, parent->plen) ? 1 : 0] = child;
}

/**
 * trie_add_prefix
 * @t: trie to add to
 * @px: prefix address
 * @plen: prefix length
 * @l: prefix lower bound 
 * @h: prefix upper bound
 *
 * Adds prefix (prefix pattern) @px/@plen to trie @t.  @l and @h are lower
 * and upper bounds on accepted prefix lengths, both inclusive.
 * 0 <= l, h <= 32 (128 for IPv6).
 */

void
trie_add_prefix(struct f_trie *t, ip_addr px, int plen, int l, int h)
{
  if (l == 0)
    t->zero = 1;
  else
    l--;

  if (h < plen)
    plen = h;

  ip_addr amask = ipa_xor(ipa_mkmask(l), ipa_mkmask(h));
  ip_addr pmask = ipa_mkmask(plen);
  ip_addr paddr = ipa_and(px, pmask);
  struct f_trie_node *o = NULL;
  struct f_trie_node *n = &t->root;

  while(n)
    {
      ip_addr cmask = ipa_and(n->mask, pmask);

      if (ipa_compare(ipa_and(paddr, cmask), ipa_and(n->addr, cmask)))
	{
	  /* We are out of path - we have to add branching node 'b'
	     between node 'o' and node 'n', and attach new node 'a'
	     as the other child of 'b'. */
	  int blen = ipa_pxlen(paddr, n->addr);
	  ip_addr bmask = ipa_mkmask(blen);
	  ip_addr baddr = ipa_and(px, bmask);

	  /* Merge accept masks from children to get accept mask for node 'b' */
	  ip_addr baccm = ipa_and(ipa_or(amask, n->accept), bmask);

	  struct f_trie_node *a = new_node(t, plen, paddr, pmask, amask);
	  struct f_trie_node *b = new_node(t, blen, baddr, bmask, baccm);
	  attach_node(o, b);
	  attach_node(b, n);
	  attach_node(b, a);
	  return;
	}

      if (plen < n->plen)
	{
	  /* We add new node 'a' between node 'o' and node 'n' */
	  amask = ipa_or(amask, ipa_and(n->accept, pmask));
	  struct f_trie_node *a = new_node(t, plen, paddr, pmask, amask);
	  attach_node(o, a);
	  attach_node(a, n);
	  return;
	}
	
      if (plen == n->plen)
	{
	  /* We already found added node in trie. Just update accept mask */
	  n->accept = ipa_or(n->accept, amask);
	  return;
	}

      /* Update accept mask part M2 and go deeper */
      n->accept = ipa_or(n->accept, ipa_and(amask, n->mask));

      /* n->plen < plen and plen <= 32 (128) */
      o = n;
      n = n->c[ipa_getbit(paddr, n->plen) ? 1 : 0];
    }

  /* We add new tail node 'a' after node 'o' */
  struct f_trie_node *a = new_node(t, plen, paddr, pmask, amask);
  attach_node(o, a);
}

/**
 * trie_match
 * @t: trie
 * @px: prefix address
 * @plen: prefix length
 *
 * Tries to find a matching prefix pattern in the trie such that
 * prefix @px/@plen matches that prefix pattern. Returns 1 if there
 * is such prefix pattern in the trie.
 */
int
trie_match_prefix(struct f_trie *t, ip_addr px, int plen)
{
  ip_addr pmask = ipa_mkmask(plen);
  ip_addr paddr = ipa_and(px, pmask);

  if (plen == 0)
    return t->zero;

  int plentest = plen - 1;
  struct f_trie_node *n = &t->root;

  while(n)
    {
      ip_addr cmask = ipa_and(n->mask, pmask);

      /* We are out of path */
      if (ipa_compare(ipa_and(paddr, cmask), ipa_and(n->addr, cmask)))
	return 0;

      /* Check accept mask */
      if (ipa_getbit(n->accept, plentest))
	return 1;

      /* We finished trie walk and still no match */
      if (plen <= n->plen)
	return 0;

      /* Choose children */
      n =  n->c[(ipa_getbit(paddr, n->plen)) ? 1 : 0];
    }

  return 0;
}

static int
trie_node_same(struct f_trie_node *t1, struct f_trie_node *t2)
{
  if ((t1 == NULL) && (t2 == NULL))
    return 1;

  if ((t1 == NULL) || (t2 == NULL))
    return 0;

  if ((t1->plen != t2->plen) ||
      (! ipa_equal(t1->addr, t2->addr)) ||
      (! ipa_equal(t1->accept, t2->accept)))
    return 0;

  return trie_node_same(t1->c[0], t2->c[0]) && trie_node_same(t1->c[1], t2->c[1]);
}

/**
 * trie_same
 * @t1: first trie to be compared
 * @t2: second one
 *
 * Compares two tries and returns 1 if they are same
 */
int
trie_same(struct f_trie *t1, struct f_trie *t2)
{
  return (t1->zero == t2->zero) && trie_node_same(&t1->root, &t2->root);
}

static void
trie_node_format(struct f_trie_node *t, buffer *buf)
{
  if (t == NULL)
    return;

  if (ipa_nonzero(t->accept))
    buffer_print(buf, "%I/%d{%I}, ", t->addr, t->plen, t->accept);

  trie_node_format(t->c[0], buf);
  trie_node_format(t->c[1], buf);
}

/**
 * trie_format
 * @t: trie to be formatted
 * @buf: destination buffer
 *
 * Prints the trie to the supplied buffer.
 */
void
trie_format(struct f_trie *t, buffer *buf)
{
  buffer_puts(buf, "[");

  if (t->zero)
    buffer_print(buf, "%I/%d", IPA_NONE, 0);
  trie_node_format(&t->root, buf);

  /* Undo last separator */
  if (buf->pos[-1] != '[')
    buf->pos -= 2;

  buffer_puts(buf, "]");
}
