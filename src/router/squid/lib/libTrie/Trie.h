/*
 * Copyright (C) 1996-2024 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef SQUID_LIB_LIBTRIE_TRIE_H
#define SQUID_LIB_LIBTRIE_TRIE_H

#include "TrieNode.h"
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

class TrieCharTransform;

/* TODO: parameterize this to be more generic -
* i.e. M-ary internal node sizes etc
*/

class Trie
{

public:
    Trie(TrieCharTransform *aTransform = nullptr);
    ~Trie();
    Trie (Trie const &);
    Trie &operator= (Trie const &);

    /* Find an exact match in the trie.
    * If found, return the private data.
    * If not found, return NULL.
    */
    inline void *find (char const *, size_t);
    /* find any element of the trie in the buffer from the
    * beginning thereof
    */
    inline void *findPrefix (char const *, size_t);

    /* Add a string.
    * returns false if the string is already
    * present or cannot be added.
    */

    bool add(char const *, size_t, void *);

private:
    TrieNode *head;

    /* transfor each 8 bits in the element */
    TrieCharTransform *transform;
};

void *
Trie::find (char const *aString, size_t theLength)
{
    if (head)
        return head->find (aString, theLength, transform, false);

    return nullptr;
}

void *
Trie::findPrefix (char const *aString, size_t theLength)
{
    if (head)
        return head->find (aString, theLength, transform, true);

    return nullptr;
}

#endif /* SQUID_LIB_LIBTRIE_TRIE_H */

