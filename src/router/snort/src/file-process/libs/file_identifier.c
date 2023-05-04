/*
 **
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Copyright (C) 2012-2013 Sourcefire, Inc.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License Version 2 as
 **  published by the Free Software Foundation.  You may not use, modify or
 **  distribute this program under any other version of the GNU General
 **  Public License.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  5.25.2012 - Initial Source Code. Hcao
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "sf_types.h"
#include "file_identifier.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "util.h"
#include "mstring.h"
#include "sfghash.h"
#include "file_config.h"
#include "memory_stats.h"

uint32_t memory_used = 0; /*Track memory usage*/

static SFGHASH *identifier_merge_hash = NULL;

typedef struct _IdentifierSharedNode
{
    IdentifierNode *shared_node;  /*the node that is shared*/
    IdentifierNode *append_node;  /*the node that is added*/
} IdentifierSharedNode;

static IdentifierMemoryBlock *id_memory_root = NULL;
static IdentifierMemoryBlock *id_memory_current = NULL;

#ifdef DEBUG_MSGS
static char *file_type_test(void *conf);
#endif

static void identifierMergeHashFree(void)
{
    if (identifier_merge_hash != NULL)
    {
        sfghash_delete(identifier_merge_hash);
        identifier_merge_hash = NULL;
    }
}

static void identifierMergeHashInit(void)
{
    if (identifier_merge_hash != NULL)
        identifierMergeHashFree();

    identifier_merge_hash = sfghash_new(1000, sizeof(IdentifierSharedNode), 0,
            NULL);
    if (identifier_merge_hash == NULL)
    {
        FatalError("%s(%d) Could not create identifier merge hash.\n",
                __FILE__, __LINE__);
    }
}

static inline void *calloc_mem(size_t size)
{
    void *ret;
    IdentifierMemoryBlock *new = NULL;
    ret = SnortPreprocAlloc(1, size, PP_FILE, PP_MEM_CATEGORY_SESSION);
    memory_used += size;
    /*For memory management*/
    size = sizeof(*new);
    new = (IdentifierMemoryBlock *)SnortPreprocAlloc(1, size, PP_FILE, PP_MEM_CATEGORY_SESSION);
    new->mem_block = ret;
    if (!id_memory_root)
    {
        id_memory_root = new;
    }
    else
    {
        id_memory_current->next = new;
    }
    id_memory_current = new;
    memory_used += size;
    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"calloc:  %p (%d).\n", ret, size););
    return ret;
}

static void  set_node_state_shared(IdentifierNode *start)
{
    int i;

    if (!start)
        return;

    if (start->state == ID_NODE_SHARED)
        return;

    if (start->state == ID_NODE_USED)
        start->state = ID_NODE_SHARED;
    else
    {
        start->state = ID_NODE_USED;
    }

    for(i = 0; i < MAX_BRANCH; i++)
    {
        set_node_state_shared(start->next[i]);
    }
}

/*Clone a trie*/
static IdentifierNode *clone_node(IdentifierNode *start)
{
    int index;
    IdentifierNode *new;
    if (!start)
        return NULL;

    new =  calloc_mem(sizeof(*new));

    new->offset = start->offset;
    new->type_id = start->type_id;

    for(index = 0; index < MAX_BRANCH; index++)
    {
        if (start->next[index])
        {
            new->next[index] = start->next[index];
        }

    }
    return new;
}

static void verify_magic_offset(MagicData *parent, MagicData *current)
{
    if ((parent) && (parent->content_len + parent->offset > current->offset))
    {
        ParseError(" Magic content at offset %d overlaps with offset %d.",
                parent->offset, current->offset);

    }

    if ((current->next) &&
            (current->content_len + current->offset > current->next->offset))
    {
        ParseError(" Magic content at offset %d overlaps with offset %d.",
                current->offset, current->next->offset);

    }
}

/* Add a new node to the sorted magic list
 */
static void add_to_sorted_magic(MagicData **head, MagicData *new )
{
    MagicData *current = *head;

    /*Avoid adding the same magic*/
    if (!new || (current == new))
        return;

    if (new->offset < current->offset)
    {
        /*current becomes new head*/
        new->next = current;
        *head = new;
        verify_magic_offset(NULL, new);
        return;
    }

    /*Find the parent for the new magic*/
    while (current)
    {
        MagicData *next = current->next;
        if ((!next) || (new->offset < next->offset))
        {
            /*current is the parent*/
            current->next = new;
            new->next = next;
            verify_magic_offset(current, new);
            return;
        }
        current = next;
    }
}

/* Content magics are sorted based on offset, this
 * will help compile the file magic trio
 */
static void sort_magics(MagicData **head)
{
    MagicData *current = *head;

    /*Add one magic at a time to sorted magics*/
    while (current)
    {
        MagicData *next = current->next;
        current->next = NULL;
        add_to_sorted_magic(head, current);
        current = next;
    }
}

/*Create a trie for the magic*/
static inline IdentifierNode *create_trie_from_magic(MagicData **head, uint32_t type_id)
{
    int i;
    IdentifierNode *current;
    IdentifierNode *root = NULL;
    MagicData *magic;

    if (!head || !(*head)||(0 == (*head)->content_len) || !type_id)
        return NULL;

    sort_magics(head);
    magic = *head;

    current =  calloc_mem(sizeof(*current));
    current->state = ID_NODE_NEW;
    root = current;

    while (magic)
    {
        current->offset = magic->offset;
        for(i = 0; i < magic->content_len; i++)
        {
            IdentifierNode *new = calloc_mem(sizeof(*new));
            new->offset = magic->offset + i + 1;
            new->state = ID_NODE_NEW;
            current->next[magic->content[i]] = new;
            current = new;
        }
        magic = magic->next;
    }

    /*Last node has type name*/
    current->type_id = type_id;
    DEBUG_WRAP( if (DEBUG_FILE & GetDebugLevel()){file_identifiers_print(root);})
    return root;

}

/*This function examines whether to update the trie based on shared state*/

static inline bool updateNext(IdentifierNode *start,IdentifierNode **next_ptr,
        IdentifierNode *append)
{

    IdentifierNode *next = (*next_ptr);
    IdentifierSharedNode sharedIdentifier;
    IdentifierNode *result;

    if (!append || (next == append))
        return false;

    sharedIdentifier.append_node = append;
    sharedIdentifier.shared_node = next;
    if (!next)
    {
        /*reuse the append*/
        *next_ptr = append;
        set_node_state_shared(append);
        return false;
    }
    else if ((result = sfghash_find(identifier_merge_hash, &sharedIdentifier)))
    {
        /*the same pointer has been processed, reuse it*/
        *next_ptr = result;
        set_node_state_shared(result);
        return false;
    }
    else
    {

        if ((start->offset < append->offset) && (next->offset > append->offset))
        {
            /*offset could have gap when non 0 offset is allowed */
            int index;
            IdentifierNode *new = calloc_mem(sizeof(*new));
            sharedIdentifier.shared_node = next;
            sharedIdentifier.append_node = append;
            new->offset = append->offset;

            for(index = 0; index < MAX_BRANCH; index++)
            {
                new->next[index] = next;
            }

            set_node_state_shared(next);
            next = new;
            sfghash_add(identifier_merge_hash, &sharedIdentifier, next);
        }
        else if (next->state == ID_NODE_SHARED)
        {
            /*shared, need to clone one*/
            IdentifierNode *current_next = next;
            sharedIdentifier.shared_node = current_next;
            sharedIdentifier.append_node = append;
            next = clone_node(current_next);
            set_node_state_shared(next);
            sfghash_add(identifier_merge_hash, &sharedIdentifier, next);
        }

        *next_ptr = next;
    }

    return true;
}

/*
 * Append magic to existing trie
 *
 */
static void update_trie(IdentifierNode *start, IdentifierNode *append)
{
    int i;

    if ((!start )||(!append)||(start == append))
        return ;


    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Working on %p -> %p at offset %d.\n",
            start, append, append->offset););

    if (start->offset == append->offset )
    {
        /* when we come here, make sure this tree is not shared
         * Update start trie using append information*/

        if (start->state == ID_NODE_SHARED)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Something is wrong ..."););
        }

        if (append->type_id)
        {
            if(start->type_id)
                LogMessage("Duplicated type definition '%d -> %d at offset %d.\n",
                        start->type_id, append->type_id, append->offset);
            start->type_id = append->type_id;
        }

        for(i = 0; i < MAX_BRANCH; i++)
        {
            if (updateNext(start,&start->next[i], append->next[i]))
            {
                update_trie(start->next[i], append->next[i]);
            }
        }
    }
    else  if (start->offset < append->offset )
    {

        for(i = 0; i < MAX_BRANCH; i++)
        {
            if (updateNext(start,&start->next[i], append))
                update_trie(start->next[i], append);
        }
    }
    else /*This is impossible*/
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Something is wrong ....."););
    }
    return;
}


void file_identifers_update(RuleInfo *rule, void *conf)
{
    IdentifierNode *new;
    FileConfig *file_config = NULL;

    file_config = (FileConfig *) conf;

    if (!file_config->identifier_root)
    {
        file_identifers_init();
        file_config->identifier_root = calloc_mem(sizeof(*file_config->identifier_root));
        file_config->id_memory_root = id_memory_root;
        identifierMergeHashInit();
    }

    new = create_trie_from_magic(&(rule->magics), rule->id);

    update_trie(file_config->identifier_root, new);
    DEBUG_WRAP( if (DEBUG_FILE & GetDebugLevel()){file_type_test(file_config);});
}


void file_identifers_init(void)
{
    memory_used = 0;
    id_memory_root = NULL;
    id_memory_current = NULL;
}


uint32_t file_identifiers_usage(void)
{
    return memory_used;
}

/*
 * This is the main function to find file type
 * Find file type is to traverse the tries.
 * Context is saved to continue file type identification
 * when more data are available
 */
uint32_t file_identifiers_match(uint8_t *buf, int len, FileContext *context)
{
    IdentifierNode * current;
    uint64_t end;

    if ( !context )
        return SNORT_FILE_TYPE_UNKNOWN;

    if ( !buf || len <= 0 )
        return SNORT_FILE_TYPE_CONTINUE;

    if ( !context->file_type_context )
    {
        FileConfig * file_config = (FileConfig *)context->file_config;
        context->file_type_context = file_config->identifier_root;
    }

    current = (IdentifierNode*)context->file_type_context;
    end = context->processed_bytes + len;

    while ( current && ( current->offset >= context->processed_bytes ) )
    {
        /* Found file id, save and continue */
        if ( current->type_id )
            context->file_type_id = current->type_id;

        if ( current->offset >= end )
        {
            /* Save current state */
            context->file_type_context = current;
            return SNORT_FILE_TYPE_CONTINUE;
        }

        /* Move to the next level */
        current = current->next[buf[current->offset - context->processed_bytes]];
    }

    /*Either end of magics or passed the current offset*/
    context->file_type_context = NULL;

    if ( context->file_type_id == SNORT_FILE_TYPE_CONTINUE )
        context->file_type_id = SNORT_FILE_TYPE_UNKNOWN;

    return context->file_type_id;

}


void file_identifiers_free(void *conf)
{
    IdentifierMemoryBlock *id_memory_next;
    FileConfig *file_config = (FileConfig *)conf;

    if (!file_config)
        return;
    /*Release memory used for identifiers*/
    id_memory_current = file_config->id_memory_root;
    while (id_memory_current)
    {
        id_memory_next = id_memory_current->next;
        SnortPreprocFree(id_memory_current->mem_block, sizeof(IdentifierNode), PP_FILE, PP_MEM_CATEGORY_SESSION);
        SnortPreprocFree(id_memory_current, sizeof(IdentifierMemoryBlock), PP_FILE, PP_MEM_CATEGORY_SESSION);
        id_memory_current = id_memory_next;
    }

    file_config->id_memory_root = NULL;
    identifierMergeHashFree();
}

#ifdef DEBUG_MSGS
void file_identifiers_print(IdentifierNode* current)
{
    int i;

    DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Working on pointer %p, offset:%d\n",
            (void *) current, current->offset););

    for (i = 0; i < MAX_BRANCH; i++)
    {
        if (current->next[i])
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Magic number: %x ", i););
            file_identifiers_print(current->next[i]);
        }

    }
    if (current->type_id)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Type: %d\n", current->type_id););
    }

    return;
}

char *file_type_test(void *conf)
{
    uint8_t str[100] = {0x4d, 0x5a, 0x46, 0x38, 0x66, 0x72, 0x65, 0x65, 0};
    unsigned int i;
    uint32_t type_id;

    FileContext *context = SnortPreprocAlloc(1, sizeof (*context), PP_FILE, PP_MEM_CATEGORY_SESSION);
    
    static const char *file_type = "MSEXE";

    printf("Check string:");

    for (i = 0; i < strlen((char*)str); i++)
    {
        printf(" %x", str[i]);
    }
    printf("\n");

    context->file_config = conf;

    type_id = file_identifiers_match(str, strlen((char *)str), context);
    if (SNORT_FILE_TYPE_UNKNOWN == type_id)
    {
        printf("File type is unknown\n");
    }
    else if (SNORT_FILE_TYPE_CONTINUE != type_id)
        printf("File type is: %s (%d)\n",
                file_type_name(conf, type_id), type_id);

    SnortPreprocFree(context, sizeof(FileContext), PP_FILE, PP_MEM_CATEGORY_SESSION);
    return ((char *)file_type);
}
#endif




