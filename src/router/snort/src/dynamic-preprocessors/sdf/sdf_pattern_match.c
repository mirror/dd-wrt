/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2009-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "sdf_pattern_match.h"
#include "treenodes.h"
#include "sf_dynamic_preprocessor.h"

/* Main pattern-adding function.
 * Arguments:
 *  head => pointer to top node in PII tree
 *  data => pointer to SDFOptionData struct w/ new pattern
 *  otn => pointer to OptTreeNode struct that this pattern belongs to
 * Return values:
 *  -1: error
 *   1: pattern added successfully
 */
static int AddPiiPattern(sdf_tree_node *head, SDFOptionData *data)
{
    char *pattern = data->pii;
    int i = 0;
    int pattern_added = 0;

    if (head == NULL || pattern == NULL)
        return -1;

    /* If the root has some children, try to fit the pattern under them first. */
    while(i < head->num_children && !pattern_added)
    {
        pattern_added = AddPiiPiece(head->children[i], pattern, data);
        i++;
    }

    /* Otherwise, add a new child to the root node */
    if (!pattern_added)
    {
        AddChild(head, data, data->pii);
        pattern_added = 1;
    }

    return pattern_added;
}

/* Check that the brackets in a pattern match up, and only contain numbers.
 *
 * Arguments:
 *   pii - string containing pattern.
 *
 * Returns: void function. Raises fatal error if there's a problem.
 */
static void ExpandBrackets(char **pii)
{
    char *bracket_index, *new_pii, *endptr, *pii_position;
    unsigned long int new_pii_size, repetitions, total_reps = 0;
    unsigned int num_brackets = 0;

    if (pii == NULL || *pii == NULL)
        return;

    /* Locate first '{' */
    bracket_index = strchr(*pii, '{');

    /* Brackets at the beginning have nothing to modify. */
    if (bracket_index == *pii)
    {
        DynamicPreprocessorFatalMessage("SDF Pattern \"%s\" starts with curly "
            "brackets which have nothing to modify.\n", *pii);
    }

    /* Check for various error cases. Total up the # of bytes needed in new pattern */
    while (bracket_index)
    {
        /* Ignore escaped brackets */
        if ((bracket_index > *pii) && (*(bracket_index-1) == '\\'))
        {
            bracket_index = strchr(bracket_index+1, '{');
            continue;
        }

        /* Check for the case of one bracket set modifying another, i.e. "{3}{4}"
           Note: "\}{4}" is OK */
        if ((bracket_index > (*pii)+1) &&
            (*(bracket_index-1) == '}') &&
            (*(bracket_index-2) != '\\') )
        {
            DynamicPreprocessorFatalMessage("SDF Pattern \"%s\" contains curly "
                "brackets which have nothing to modify.\n", *pii);
        }

        /* Get the number from inside the brackets */
        repetitions = strtoul(bracket_index+1, &endptr, 10);
        if (*endptr != '}' && *endptr != '\0')
        {
            DynamicPreprocessorFatalMessage("SDF Pattern \"%s\" contains curly "
                "brackets with non-digits inside.\n", *pii);
        }
        else if (*endptr == '\0')
        {
            DynamicPreprocessorFatalMessage("SDF Pattern \"%s\" contains "
                "an unterminated curly bracket.\n", *pii);
        }

        /* The brackets look OK. Increase the rep count. */
        if ((bracket_index > (*pii)+1) && (*(bracket_index-2) == '\\'))
            total_reps += (repetitions * 2);
        else
            total_reps += repetitions;

        num_brackets++;

        /* Next bracket */
        bracket_index = strchr(bracket_index+1, '{');
    }

    /* By this point, the brackets all match up. */
    if (num_brackets == 0)
        return;

    /* Allocate the new pii string. */
    new_pii_size = (strlen(*pii) + total_reps - 2*num_brackets + 1);
    new_pii = (char *) calloc(new_pii_size, sizeof(char));
    if (new_pii == NULL)
    {
        DynamicPreprocessorFatalMessage("Failed to allocate memory for "
            "SDF preprocessor.\n");
    }

    /* Copy the PII string, expanding repeated sections. */
    pii_position = *pii;
    while (*pii_position != '\0')
    {
        char repeated_section[3] = {'\0'};
        unsigned long int i, reps = 1;

        repeated_section[0] = pii_position[0];
        pii_position++;

        if (repeated_section[0] == '\\' && pii_position[0] != '\0')
        {
            repeated_section[1] = pii_position[0];
            pii_position++;
        }

        if (pii_position[0] == '{')
        {
            reps = strtoul(pii_position+1, &endptr, 10);
            pii_position = endptr+1;
        }

        /* Channeling "Shlemiel the Painter" here. */
        for (i = 0; i < reps; i++)
        {
            strncat(new_pii, repeated_section, 2);
        }
    }

    /* Switch out the pii strings. */
    free(*pii);
    *pii = new_pii;
}

/* Perform any modifications needed to a pattern string, then add it to the tree. */
int AddPii(sdf_tree_node *head, SDFOptionData *data)
{
    if (head == NULL || data == NULL)
        return -1;

    ExpandBrackets(&(data->pii));

    return AddPiiPattern(head, data);
}

/* Recursive pattern-adding function.
 * Return values:
 *  -1: error
 *   0: pattern did not go in this subtree
 *   1: pattern was added in this subtree
 */
int AddPiiPiece(sdf_tree_node *node, char *new_pattern, SDFOptionData *data)
{
    /* Potential cases:
        1) node->pattern and new_pattern overlap by some number of bytes,
           but both end differently.
           Split the current node then add a second child.
        2) node->pattern is a substring of new_pattern.
           Preserve current node, go on to children.
           If no children exist, add one and stop.
        3) new_pattern is a substring of node->pattern.
           Split the current node, AND add an end-of-pattern marker.
        4) Pattern doesn't fit here at all. Return 0 to caller.
    */

    char *node_pattern_copy;
    uint16_t overlapping_bytes = 0;

    if (node == NULL || new_pattern == NULL || *new_pattern == '\0')
        return -1;

    /* Count the overlapping bytes between
        a) our current node's pattern
        b) the piece of the PII pattern being added here
       Additionally, we advance the pattern ptr to the non-matching part,
       so that only the non-matching part is added to a child node. */
    node_pattern_copy = node->pattern;
    while(*node_pattern_copy != '\0' &&
          *new_pattern != '\0' &&
          *node_pattern_copy == *new_pattern)
    {
        /* Handle escape sequences: either the whole thing matches, or not at all */
        if (*new_pattern == '\\')
        {
            if (*(new_pattern+1) != *(node_pattern_copy+1))
                break;

            /* Don't increment twice if the strings just ended in '\' */
            if (*(new_pattern+1) != '\0')
            {
                new_pattern++;
                node_pattern_copy++;
                overlapping_bytes++;
            }
        }

        new_pattern++;
        node_pattern_copy++;
        overlapping_bytes++;
    }

    if (*node_pattern_copy == '\0' && *new_pattern == '\0')
    {
        /* Patterns completely match */
        uint16_t i;
        int data_added = 0;

        /* Replace old option_data if the sid & gid match.
           The OTN has already been freed out from under us. */
        for (i = 0; i < node->num_option_data; i++)
        {
            if ((node->option_data_list[i]->sid == data->sid) &&
                (node->option_data_list[i]->gid == data->gid))
            {
                free(node->option_data_list[i]->pii);
                free(node->option_data_list[i]);
                node->option_data_list[i] = data;
                data_added = 1;
            }
        }

        /* Otherwise, append the new option_data to the list. */
        if (!data_added)
        {
            SDFOptionData **tmp_realloc_ptr = NULL;

            tmp_realloc_ptr = (SDFOptionData **)
                realloc((void *)node->option_data_list,
                        (node->num_option_data + 1) * sizeof(SDFOptionData *));

            if (tmp_realloc_ptr == NULL)
                DynamicPreprocessorFatalMessage("%s(%d) Could not reallocate "
                    "option_data_list\n", __FILE__, __LINE__);

            node->option_data_list = tmp_realloc_ptr;

            node->option_data_list[node->num_option_data] = data;
            node->num_option_data++;
        }

        return 1;
    }
    else if (*node_pattern_copy == '\0')
    {
        int i;
        /* Current node holds a subset of the pattern. Recurse to the children. */
        for(i = 0; i < node->num_children; i++)
        {
            if (AddPiiPiece(node->children[i], new_pattern, data) == 1)
                return 1;
        }

        /* No children matched, or no children existed. Add the child here. */
        AddChild(node, data, new_pattern);
        return 1;
    }
    else if (*new_pattern == '\0')
    {
        /* pattern is a subset of the current node's pattern */
        SplitNode(node, overlapping_bytes);
        node->num_option_data = 1;

        node->option_data_list = (SDFOptionData **) calloc(1, sizeof(SDFOptionData *));
        if (node->option_data_list == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Could not allocate option_data_list\n",
                    __FILE__, __LINE__);
        }

        node->option_data_list[0] = data;
        return 1;
    }
    else if (overlapping_bytes > 0)
    {
        /* Add the child node */
        SplitNode(node, overlapping_bytes);
        AddChild(node, data, new_pattern);
        return 1;
    }

    /* These patterns don't overlap at all! */
    return 0;
}

int SplitNode(sdf_tree_node *node, uint16_t split_index)
{
    sdf_tree_node *new_node = NULL;

    if (node == NULL)
        return -1;

    if (split_index > strlen(node->pattern))
        return -1;

    /* Create new node for second half of split */
    new_node = (sdf_tree_node *) calloc(1,sizeof(sdf_tree_node));
    if (new_node == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Could not allocate new_node\n",
                __FILE__, __LINE__);
    }

    /* Fill in the new node with the child pointers, pattern, pii ptr */
    new_node->pattern = strdup(node->pattern + split_index);
    if (new_node->pattern == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Could not allocate new_node pattern\n",
                __FILE__, __LINE__);
    }

    new_node->children = node->children;
    new_node->option_data_list = node->option_data_list;
    new_node->num_children = node->num_children;
    new_node->num_option_data = node->num_option_data;

    /* Truncate the pattern of the current node, set child to new node */
    node->children = (sdf_tree_node **) calloc(1,sizeof(sdf_tree_node *));
    if (node->children == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Could not allocate node children\n",
                __FILE__, __LINE__);
    }

    node->children[0] = new_node;
    node->num_children = 1;
    node->option_data_list = NULL;
    node->num_option_data = 0;
    node->pattern[split_index] = '\0';

    return 0;
}


/* Create a new tree node, and add it as a child to the current node. */
sdf_tree_node * AddChild(sdf_tree_node *node, SDFOptionData *data, char *pattern)
{
    sdf_tree_node *new_node = NULL;

    /* Take care not to step on the other children */
    if (node->num_children)
    {
        sdf_tree_node **new_child_ptrs =
            (sdf_tree_node **) calloc(node->num_children+1, sizeof(sdf_tree_node *));

        if (new_child_ptrs == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Could not allocate new child pointers\n",
                    __FILE__, __LINE__);
        }

        memcpy(new_child_ptrs, node->children, (node->num_children * sizeof(sdf_tree_node *)));

        new_node = (sdf_tree_node *) calloc(1,sizeof(sdf_tree_node));
        if (new_node == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Could not allocate new node\n",
                    __FILE__, __LINE__);
        }

        new_child_ptrs[node->num_children] = new_node;

        free(node->children);
        node->children = new_child_ptrs;
        node->num_children++;
    }
    else
    {
        node->children = (sdf_tree_node **)calloc(1,sizeof(sdf_tree_node *));
        if (node->children == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Could not allocate node children\n",
                    __FILE__, __LINE__);
        }

        node->children[0] = (sdf_tree_node *)calloc(1,sizeof(sdf_tree_node));
        if (node->children[0] == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) Could not allocate node children[0]\n",
                    __FILE__, __LINE__);
        }

        node->num_children = 1;
        new_node = node->children[0];
    }

    new_node->pattern = strdup(pattern);
    if (new_node->pattern == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Could not allocate node pattern\n",
                __FILE__, __LINE__);
    }

    new_node->num_option_data = 1;
    new_node->option_data_list = (SDFOptionData **) calloc(1, sizeof(SDFOptionData *));

    if (new_node->option_data_list == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Could not allocate node list\n",
                __FILE__, __LINE__);
    }

    new_node->option_data_list[0] = data;

    return new_node;
}

/* Frees an entire PII tree. */
int FreePiiTree(sdf_tree_node *node)
{
    uint16_t i;

    if (node == NULL)
        return -1;

    for (i = 0; i < node->num_children; i++)
    {
        FreePiiTree(node->children[i]);
    }

    free(node->pattern);
    free(node->children);

    for (i = 0; i < node->num_option_data; i++)
    {
        free(node->option_data_list[i]->pii);
        free(node->option_data_list[i]);
    }

    free(node->option_data_list);
    free(node);

    return 0;
}

/* Returns an sdf_tree_node that matches the pattern */
sdf_tree_node * FindPiiRecursively(sdf_tree_node *node, char *buf, uint16_t *buf_index, uint16_t buflen, 
        SDFConfig *config, uint16_t *partial_index, sdf_tree_node **partial_node)
{
    uint16_t old_buf_index;
    uint16_t pattern_index = *partial_index;
    int node_match = 1;

    *partial_index = 0;
    *partial_node = NULL;

    if (node == NULL || buf == NULL || buflen == 0 || *buf_index >= buflen)
        return NULL;


    /* Save the value of buf_index that was passed in. We revert to this value
       if a pattern is not matched here. Ultimately, it should hold the number
       of bytes matched against a pattern. */
    old_buf_index = *buf_index;

    /* Match pattern buf against current node. Evaluate escape sequences.

       NOTE: node->pattern is a NULL-terminated string, but buf is network data
             and may legitimately contain NULL bytes. */
    while (*buf_index < buflen &&
           *(node->pattern + pattern_index) != '\0' &&
           node_match )
    {
        /* Match a byte at a time. */
        if ( *(node->pattern + pattern_index) == '\\' &&
             *(node->pattern + pattern_index + 1) != '\0' )
        {
            /* Escape sequence found */
            pattern_index++;
            switch ( *(node->pattern + pattern_index) )
            {
                /* Escaped special character */
                case '\\':
                case '{':
                case '}':
                case '?':
                    node_match = (*(buf + *buf_index) == *(node->pattern + pattern_index));
                    break;

                /* \d : match digit */
                case 'd':
                    node_match = isdigit( (int)(*(buf + *buf_index)) );
                    break;
                /* \D : match non-digit */
                case 'D':
                    node_match = !isdigit( (int)(*(buf + *buf_index)) );
                    break;

                /* \w : match alphanumeric */
                case 'w':
                    node_match = isalnum( (int)(*(buf + *buf_index)) );
                    break;
                /* \W : match non-alphanumeric */
                case 'W':
                    node_match = !isalnum( (int)(*(buf + *buf_index)) );
                    break;

                /* \l : match a letter */
                case 'l':
                    node_match = isalpha( (int)(*(buf + *buf_index)) );
                    break;
                /* \L : match a non-letter */
                case 'L':
                    node_match = !isalpha( (int)(*(buf + *buf_index)) );
                    break;
            }
        }
        else
        {
            /* Normal byte */
            node_match = (*(buf + *buf_index) == *(node->pattern + pattern_index));
        }

        /* Handle optional characters */
        if (*(node->pattern + pattern_index + 1) == '?')
        {
            /* Advance past the '?' in the pattern string.
               Only advance in the buffer if we matched the optional char. */
            pattern_index += 2;
            if (node_match)
                (*buf_index)++;
            else
                node_match = 1;
        }
        else
        {
            /* Advance to next byte */
            (*buf_index)++;
            pattern_index++;
        }
    }

    if (node_match)
    {
        int i = 0;
        uint16_t j;
        bool node_contains_matches = false;
        sdf_tree_node *matched_node = NULL;


        if(*buf_index == buflen)
        {
            if( (*(node->pattern + pattern_index) != '\0') ||
            ((strlen(node->pattern) == pattern_index) && node->num_children))
            {
                if( (pattern_index < strlen(node->pattern) ) &&
                    ( (*(node->pattern + pattern_index) == '\\' ) && 
                      (*(node->pattern + pattern_index+1) == 'D') ) )
                {
                    /* Do nothing here, as we found PII which is not ending with '\D'.
                     * This is not a partial match, we found the full PII, so do not 
                     * treat this as partial match.
                     */
                }
                else
                {
                    *partial_index = pattern_index;
                    *partial_node = node;
                    return NULL;
                }
            }
        }

        /* Check the children first. Always err on the side of a larger match. */
        while (i < node->num_children && (matched_node == NULL) && !(*partial_index) )
        {
            matched_node = FindPiiRecursively(node->children[i], buf, buf_index, buflen, config,
                    partial_index, partial_node);
            i++;
        }

        if ((matched_node != NULL) || *partial_index)
            return matched_node;

        /* An sdf_tree_node holds multiple SDFOptionData. It's possible to get
           some with validation funs and some without. Evaluate them independently. */
        for (j = 0; j < node->num_option_data; j++)
        {
            SDFOptionData *option_data = node->option_data_list[j];

            /* Run eval func, return NULL if it exists but fails */
            if (option_data->validate_func != NULL &&
                option_data->validate_func(buf, *buf_index, config) != 1)
            {
                *buf_index = old_buf_index;
                option_data->match_success = 0;
            }
            else
            {
                /* No eval func necessary, or an eval func existed and returned 1 */
                option_data->match_success = 1;
                node_contains_matches = true;
            }
        }

        if (node_contains_matches)
            return node;
    }

    /* No match here. */
    *buf_index = old_buf_index;
    return NULL;
}

/* This function takes a head node, and searches the children for PII.
 *
 * head - Pointer to head node of SDF patttern tree. This contains no pattern.
 * buf  - Buffer to search for patterns
 * buf_index - Pointer to store number of bytes that matched a pattern.
 * buflen - Length of buffer pointed to by buf
 * config - SDF preprocessor configuration.
 *
 * returns: sdf_tree_node ptr for matched pattern, or NULL if no match.
 */
sdf_tree_node * FindPii(const sdf_tree_node *head, char *buf, uint16_t *buf_index, uint16_t buflen,
                        SDFConfig *config, SDFSessionData *session)
{
    uint16_t i;
    uint16_t *partial_index = &(session->part_match_index);
    sdf_tree_node **partial_node = &(session->part_match_node);
    *partial_index = 0;

    if (head == NULL)
        return NULL;

    for (i = 0; i < head->num_children; i++)
    {
        sdf_tree_node * matched_node;
        matched_node = FindPiiRecursively(head->children[i], buf, buf_index, buflen, config, partial_index, partial_node);
        if (matched_node || *partial_index)
            return matched_node;
    }

    return NULL;
}
