/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 1998-2013 Sourcefire, Inc.
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

/*
 * Adam Keeton
 * sf_ipvar.c
 * 11/17/06
 *
 * Library for IP variables.
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "util.h"
#include "sf_ipvar.h"
#include "sf_vartable.h"

#define LIST_OPEN '['
#define LIST_CLOSE ']'

static SFIP_RET sfvar_list_compare(sfip_node_t *, sfip_node_t *);
static inline void sfip_node_free ( sfip_node_t * );
static inline void sfip_node_freelist ( sfip_node_t * );


static inline sfip_var_t *_alloc_var(void)
{
    return (sfip_var_t*)calloc(1, sizeof(sfip_var_t));
}

void sfvar_free(sfip_var_t *var)
{
    if(!var) return;

    if(var->name) free(var->name);

    if(var->value) free(var->value);

    if(var->mode == SFIP_LIST)
    {
        sfip_node_freelist(var->head);
        sfip_node_freelist(var->neg_head);
    }
    else if(var->mode == SFIP_TABLE)
    {
        // XXX
    }

    free(var);
}

sfip_node_t *sfipnode_alloc(char *str, SFIP_RET *status)
{
    sfip_node_t *ret;

    if(!str)
    {
        if(status)
            *status = SFIP_ARG_ERR;
        return NULL;
    }

    if( (ret = (sfip_node_t*)calloc(1, sizeof(sfip_node_t))) == NULL )
    {
        if(status)
            *status = SFIP_ALLOC_ERR;
         return NULL;
    }

    /* Check if this string starts with a '!', if so,
     * then the node needs to be negated */
    if(*str == '!')
    {
        str++;
        ret->flags |= SFIP_NEGATED;
    }

    /* Check if this is an "any" */
    if(!strncasecmp(str, "any", 3))
    {
        /* Make sure they're not doing !any, which is meaningless */
        if(ret->flags & SFIP_NEGATED)
        {
            if(status)
                *status = SFIP_ARG_ERR;
            free(ret);
            return NULL;
        }

        ret->flags |= SFIP_ANY;

        if( (ret->ip = sfip_alloc("0.0.0.0", status)) == NULL )
        {
            /* Failed to parse this string, so free and return */
            if(status)
                *status = SFIP_ALLOC_ERR;

            free(ret);
            return NULL;
        }

        if(status)
            *status = SFIP_SUCCESS;

#if 0
        if( (ret->ip = sfip_alloc("0.0.0.0", NULL)) == NULL)
        {
            if(status)
                *status = SFIP_FAILURE;
            free(ret);
            return NULL;
        }
#endif

    }
    else if( (ret->ip = sfip_alloc(str, status)) == NULL )
    {
        /* Failed to parse this string, so free and return */
        if(status)
             *status = SFIP_INET_PARSE_ERR;
        free(ret);
        return NULL;
    }

    /* Check if this is a negated, zero'ed IP (equivalent of a "!any") */
    if(!sfip_is_set(ret->ip) && (ret->flags & SFIP_NEGATED))
    {
        if(status)
            *status = SFIP_NOT_ANY;
        free(ret->ip);
        free(ret);
        return NULL;
    }

    return ret;
}

static inline void sfip_node_free ( sfip_node_t *node )
{
    if ( !node )
        return;

    if ( node->ip )
        sfip_free(node->ip);

    free(node);
}

static inline void sfip_node_freelist ( sfip_node_t *root )
{
    sfip_node_t *node;

    if ( !root )
        return;

    for ( node = root; node; node = root  )
    {
        root = root->next;
        sfip_node_free(node);
    }
}

sfip_node_t* MergeLists(sfip_node_t *list1, sfip_node_t* list2, uint16_t list1_len, uint16_t list2_len, uint16_t *merge_len)
{
    SFIP_RET ret = SFIP_SUCCESS;
    sfip_node_t* listHead = NULL, *merge_list = NULL, *tmp = NULL, *node = NULL;
    uint16_t num_nodes = 0;

    if(!list1 && !list2)
    {
        *merge_len = 0;
        return NULL;
    }
    if (list1 == NULL)
    {
        *merge_len = list2_len;
        return list2;
    }
    if (list2 == NULL)
    {
        *merge_len = list1_len;
        return list1;
    }

    /*Both lists are sorted and not NULL. If list1 or list2 contains "any", free the other list*/
    if(list1->flags & SFIP_ANY)
    {
        *merge_len = list1_len;
        while(list2)
        {
            tmp = list2->next;
            sfip_node_free(list2);
            list2 = tmp;
        }
        return list1;
    }

    if(list2->flags & SFIP_ANY)
    {
        *merge_len = list2_len;
        while(list1)
        {
            tmp = list1->next;
            sfip_node_free(list1);
            list1 = tmp;
        }
        return list2;
    }

    /*Iterate till one of the list is NULL. Append each node to merge_list*/
    while (list1 && list2)
    {
        ret = sfip_cidr_compare(list1->ip, list2->ip);
        if(ret == SFIP_LESSER)
        {
            node = list1;
            list1 = list1->next;
            list1_len--;
        }
        else if(ret == SFIP_GREATER)
        {
            node = list2;
            list2 = list2->next;
            list2_len--;
        }
        else if(ret == SFIP_EQUAL)
        {
            node = list1;
            list1 = list1->next;
            /*Free the duplicate node*/
            tmp = list2->next;
            sfip_node_free(list2);
            list2 = tmp;

            list1_len--;
            list2_len--;
        }

        if(!merge_list)
        {
            merge_list = node;
            listHead = node;
        }
        else
        {
            merge_list->next = node;
            merge_list = merge_list->next;
        }
        num_nodes++;
    }

    /*list2 is NULL. Append list1*/
    if(list1 != NULL)
    {
        merge_list->next = list1;
        num_nodes += list1_len;
    }
    /*list1 is NULL. Append list2*/
    if(list2 != NULL)
    {
        merge_list->next = list2;
        num_nodes += list2_len;
    }

    *merge_len = num_nodes;
    return listHead;
}

/* Deep copy of src added to dst */
/*src is sorted. Iterate over every node in dst and do sfvar_add_node*/
SFIP_RET sfvar_add(sfip_var_t *dst, sfip_var_t *src)
{
    sfip_var_t *copiedvar;

    if(!dst || !src) return SFIP_ARG_ERR;

    if((copiedvar = sfvar_deep_copy(src)) == NULL)
    {
        return SFIP_ALLOC_ERR;
    }

    dst->head = MergeLists(dst->head, copiedvar->head, dst->head_count, copiedvar->head_count, &dst->head_count);
    dst->neg_head = MergeLists(dst->neg_head, copiedvar->neg_head, dst->neg_head_count, copiedvar->neg_head_count, &dst->neg_head_count);

    free(copiedvar);

    return SFIP_SUCCESS;
}

SFIP_RET sfvar_add_node(sfip_var_t *var, sfip_node_t *node, int negated)
{
    sfip_node_t *p, *swp, *tmp;
    sfip_node_t **head;
    uint16_t *count;

    if(!var || !node) return SFIP_ARG_ERR;

    /* XXX */
    /* As of this writing, 11/20/06, nodes are always added to
     * the list, regardless of the mode (list or table). */

    if(negated)
    {
        head = &var->neg_head;
        count = &var->neg_head_count;
    }
    else
    {
        head = &var->head;
        count = &var->head_count;
    }


    if(!(*head))
    {
        *head = node;
        ++*count;
        return SFIP_SUCCESS;
    }

    /*If head node is any, do not add anything else*/
    if((*head)->flags & SFIP_ANY)
    {
        sfip_node_free(node);
        return SFIP_SUCCESS;
    }

    /* "Anys" should always be inserted first */
    /* Otherwise, check if this IP is less than the head's IP */
    SFIP_RET node_cmp_ret = SFIP_SUCCESS;
    if(node->flags & SFIP_ANY)
    {
        if((*head)->flags & SFIP_ANY)
        {
            sfip_node_free(node);
            return SFIP_SUCCESS;
        }
        else
        {
            /*Free the list when adding any*/
            while(*head)
            {
                tmp = (*head)->next;
                sfip_node_free(*head);
                *head = tmp;
            }
            *head = node;
            *count = 1;
            return SFIP_SUCCESS;
        }
    }
    else
    {
        node_cmp_ret = sfip_cidr_compare(node->ip, (*head)->ip);
        if(node_cmp_ret == SFIP_EQUAL)
        {
            sfip_node_free(node);
            return SFIP_SUCCESS;
        }
        else if(node_cmp_ret == SFIP_LESSER)
        {
            node->next = *head;
            *head = node;
            ++*count;
            return SFIP_SUCCESS;
        }
    }

    /* If we're here, the head node was lesser than the new node */
    /* Before searching the list, verify there is atleast two nodes.
     * (This saves an extra check during the loop below) */
    if(!(*head)->next)
    {
        (*head)->next = node;
        ++*count;
        return SFIP_SUCCESS;
    }

    /* Insertion sort */
    for(p = *head; p->next; p=p->next)
    {
        node_cmp_ret = sfip_cidr_compare(node->ip, p->next->ip);
        if(node_cmp_ret == SFIP_EQUAL)
        {
            sfip_node_free(node);
            return SFIP_SUCCESS;
        }
        else if(node_cmp_ret == SFIP_LESSER)
        {
            swp = p->next;
            p->next = node;
            node->next = swp;
            ++*count;

            return SFIP_SUCCESS;
        }
    }

    p->next = node;
    ++*count;

    return SFIP_SUCCESS;

    /* XXX Insert new node into routing table */
//    sfrt_add(node->ip,
}

static SFIP_RET sfvar_list_compare(sfip_node_t *list1, sfip_node_t *list2)
{
    sfip_node_t *tmp, *tmp2;

    if ((list1 == NULL) && (list2 == NULL))
        return SFIP_EQUAL;

    /* Lists are ordered and of equal size */
    for (tmp = list1, tmp2 = list2; (tmp != NULL) && (tmp2 != NULL); tmp = tmp->next, tmp2 = tmp2->next)
    {
        if ((sfip_cidr_compare(tmp->ip, tmp2->ip) != SFIP_EQUAL))
        {
            return SFIP_FAILURE;
        }
    }
    return SFIP_EQUAL;
}

/* Check's if two variables have the same nodes */
SFIP_RET sfvar_compare(const sfip_var_t *one, const sfip_var_t *two)
{
    /* If both NULL, consider equal */
    if(!one && !two)
        return SFIP_EQUAL;

    /* If one NULL and not the other, consider unequal */
    if((one && !two) || (!one && two))
        return SFIP_FAILURE;

    if (sfvar_is_alias(one, two))
        return SFIP_EQUAL;

    if(one->head_count != two->head_count)
        return SFIP_FAILURE;

    if (sfvar_list_compare(one->head, two->head) == SFIP_FAILURE)
        return SFIP_FAILURE;

    if(one->neg_head_count != two->neg_head_count)
        return SFIP_FAILURE;

    if (sfvar_list_compare(one->neg_head, two->neg_head) == SFIP_FAILURE)
        return SFIP_FAILURE;

    return SFIP_EQUAL;
}

/* Support function for sfvar_parse_iplist.  Used to
 * correctly match up end brackets.
 *  (Can't just do strchr(str, ']') because of the
 *  [a, [b], c] case, and can't do strrchr because
 *  of the [a, [b], [c]] case) */
static char *_find_end_token(char *str)
{
    int stack = 0;

    for(; *str; str++)
    {
        if(*str == LIST_OPEN)
            stack++;
        else if(*str == LIST_CLOSE)
            stack--;

        if(!stack)
        {
            return str;
        }
    }

    return NULL;
}

/* Support function for sfvar_parse_iplist.
 *  Negates a node */
static void _negate_node(sfip_node_t *node)
{
    if(node->addr_flags & SFIP_NEGATED)
    {
        node->addr_flags &= ~SFIP_NEGATED;
        node->flags &= ~SFIP_NEGATED;
    }
    else
    {
        node->addr_flags |= SFIP_NEGATED;
        node->flags |= SFIP_NEGATED;
    }
}

/* Support function for sfvar_parse_iplist.
 *  Negates a variable */
static void _negate_lists(sfip_var_t *var)
{
    sfip_node_t *node;
    sfip_node_t *temp;
    uint16_t temp_count;

    for(node = var->head; node; node=node->next)
        _negate_node(node);

    for(node = var->neg_head; node; node=node->next)
        _negate_node(node);

    /* Swap lists */
    temp = var->head;
    var->head = var->neg_head;
    var->neg_head = temp;

    /*Swap the counts*/
    temp_count = var->neg_head_count;
    var->neg_head_count = var->head_count;
    var->head_count = temp_count;
}

SFIP_RET sfvar_parse_iplist(vartable_t *table, sfip_var_t *var,
                           char *str, int negation)
{
    char *end, *tok;
    SFIP_RET ret;
    int neg_ip;

    if(!var || !table || !str)
        return SFIP_ARG_ERR;

    while(*str)
    {
        /* Skip whitespace and leading commas */
        if(isspace((int)*str) || *str == ',')
        {
            str++;
            continue;
        }

        neg_ip = 0;

        /* Handle multiple negations */
        for(; *str == '!'; str++)
             neg_ip = !neg_ip;

        /* Find end of this token */
        for(end = str+1;
           *end && !isspace((int)*end) && *end != LIST_CLOSE && *end != ',';
            end++) ;

        tok = SnortStrndup(str, end - str);

        if(*str == LIST_OPEN)
        {
            char *list_tok;

            /* Find end of this list */
            if((end = _find_end_token(str)) == NULL)
            {
                /* No trailing bracket found */
                free(tok);
                return SFIP_UNMATCHED_BRACKET;
            }

            str++;
            list_tok = SnortStrndup(str, end - str);

            if((ret = sfvar_parse_iplist(table, var, list_tok,
                           negation ^ neg_ip)) != SFIP_SUCCESS)
            {
                free(list_tok);
                free(tok);
                return ret;
            }

            free(list_tok);
        }
        else if(*str == '$')
        {
            sfip_var_t *tmp_var;
            sfip_var_t *copy_var;

            if((tmp_var = sfvt_lookup_var(table, tok)) == NULL)
            {
                free(tok);
                return SFIP_LOOKUP_FAILURE;
            }

            copy_var = sfvar_deep_copy(tmp_var);
            /* Apply the negation */
            if(negation ^ neg_ip)
            {
                /* Check for a negated "any" */
                if(copy_var->head && copy_var->head->flags & SFIP_ANY)
                {
                    free(tok);
                    sfvar_free(copy_var);
                    return SFIP_NOT_ANY;
                }

                /* Check if this is a negated, zero'ed IP (equivalent of a "!any") */
                if(copy_var->head && !sfip_is_set(copy_var->head->ip))
                {
                    free(tok);
                    sfvar_free(copy_var);
                    return SFIP_NOT_ANY;
                }

                _negate_lists(copy_var);
            }

            sfvar_add(var, copy_var);
            sfvar_free(copy_var);
        }
        else if(*str == LIST_CLOSE)
        {
            /* This should be the last character, if not, then this is an
             * invalid extra closing bracket */
            if(!(*(str+1)))
            {
                free(tok);
                return SFIP_SUCCESS;
            }

            free(tok);
            return SFIP_UNMATCHED_BRACKET;
        }
        else
        {
            sfip_node_t *node;

            /* Skip leading commas */
            for(; *str == ','; str++) ;

            /* Check for a negated "any" */
            if(negation ^ neg_ip && !strcasecmp(tok, "any"))
            {
                free(tok);
                return SFIP_NOT_ANY;
            }

            /* This should be an IP address! */
            /* Allocate new node for this string and add it to "ret" */
            if((node = sfipnode_alloc(tok, &ret)) == NULL)
            {
                free(tok);
                return ret;
            }

            if(negation ^ neg_ip)
            {
                _negate_node(node);
            }

            /* Check if this is a negated, zero'ed IP (equivalent of a "!any") */
            if(!sfip_is_set(node->ip) && (node->flags & SFIP_NEGATED))
            {
                sfip_node_free(node);
                free(tok);
                return SFIP_NOT_ANY;
            }

            ret = sfvar_add_node(var, node, negation ^ neg_ip);

            if(ret != SFIP_SUCCESS )
            {
                free(tok);
                return ret;
            }
        }

        free(tok);
        if(*end)
            str = end + 1;
        else break;
    }

    return SFIP_SUCCESS;
}

SFIP_RET sfvar_validate(sfip_var_t *var)
{
    sfip_node_t *idx, *neg_idx;

    if(!var->head || !var->neg_head)
        return SFIP_SUCCESS;

    for(idx = var->head; idx; idx = idx->next)
    {
        for(neg_idx = var->neg_head; neg_idx; neg_idx = neg_idx->next)
        {
            /* A smaller netmask means "less specific" */
            if((sfip_bits(neg_idx->ip) <= sfip_bits(idx->ip)) &&
                /* Verify they overlap */
                (sfip_contains(neg_idx->ip, &idx->ip->addr) == SFIP_CONTAINS))
            {
                return SFIP_CONFLICT;
            }
        }
    }

    return SFIP_SUCCESS;
}

sfip_var_t * sfvar_create_alias(const sfip_var_t *alias_from, const char *alias_to)
{
    sfip_var_t *ret;

    if ((alias_from == NULL) || (alias_to == NULL))
        return NULL;

    ret = sfvar_deep_copy(alias_from);
    if (ret == NULL)
        return NULL;

    ret->name = SnortStrdup(alias_to);
    ret->id = alias_from->id;

    return ret;
}

int sfvar_is_alias(const sfip_var_t *one, const sfip_var_t *two)
{
    if ((one == NULL) || (two == NULL))
        return 0;

    if ((one->id != 0) && (one->id == two->id))
        return 1;
    return 0;
}

/* Allocates and returns a new variable, described by "variable". */
sfip_var_t *sfvar_alloc(vartable_t *table, char *variable, SFIP_RET *status)
{
    sfip_var_t *ret, *tmpvar;
    char *str, *end, *tmp;
    SFIP_RET stat;
    bool invalidvar = true;

    if(!variable || !(*variable))
    {
        if(status)
            *status = SFIP_ARG_ERR;
        return NULL;
    }

    if( (ret = _alloc_var()) == NULL )
    {
        if(status)
            *status = SFIP_ALLOC_ERR;
        return NULL;
    }

    /* Extract and save the variable's name */
    /* Start by skipping leading whitespace or line continuations: '\' */
    for(str = variable ; *str && (isspace((int)*str) || *str == '\\'); str++) ;
    if (*str == 0)  /* Didn't get anything */
    {
        if (status)
            *status = SFIP_ARG_ERR;

        sfvar_free(ret);
        return NULL;
    }

    /* Find the end of the name */
    for(end = str; *end && !isspace((int)*end) && *end != '\\'; end++)
    {
	if(isalpha((int)*end))
		invalidvar = false;
    }

    if(invalidvar)
    {
	if(status)
	  *status = SFIP_INVALID_VAR;

	sfvar_free(ret);
	return NULL;
    }

    if(!isalnum((int)*str) && *str != '$' && *str != '!' && *str != '_')
    {
        if(status)
            *status = SFIP_ARG_ERR;

        sfvar_free(ret);
        return NULL;
    }

    /* Set the new variable's name/key */
    if((ret->name = SnortStrndup(str, end - str)) == NULL)
    {
        if(status)
            *status = SFIP_ALLOC_ERR;

        sfvar_free(ret);
        return NULL;
    }

    /* End points to the end of the name.  Skip past it and any whitespace
     * or potential line continuations */
    str = end;
    for (; (*str != 0) && (isspace((int)*str) || (*str == '\\')); str++);
    if (*str == 0)  /* Didn't get anything */
    {
        if (status)
            *status = SFIP_ARG_ERR;

        sfvar_free(ret);
        return NULL;
    }

    /* Trim off whitespace and line continuations from the end of the string */
    end = (str + strlen(str)) - 1;
    for (; (end > str) && (isspace((int)*end) || (*end == '\\')); end--);
    end++;

    /* See if this is just an alias */
    tmp = SnortStrndup(str, end - str);
    tmpvar = sfvt_lookup_var(table, tmp);
    free(tmp);
    if (tmpvar != NULL)
    {
        sfip_var_t *aliased = sfvar_create_alias(tmpvar, ret->name);
        if (aliased != NULL)
        {
            if (status != NULL)
                *status = SFIP_SUCCESS;

            sfvar_free(ret);
            return aliased;
        }
    }

    /* Everything is treated as a list, even if it's one element that's not
     * surrounded by brackets */
    stat = sfvar_parse_iplist(table, ret, str, 0);
    if (status != NULL)
        *status = stat;

    if (stat != SFIP_SUCCESS)
    {
        sfvar_free(ret);
        return NULL;
    }

    if(ret->head &&
            (ret->head->flags & SFIP_ANY && ret->head->flags & SFIP_NEGATED))
    {
        if(status)
            *status = SFIP_NOT_ANY;

        sfvar_free(ret);
        return NULL;
    }

    if(sfvar_validate(ret) == SFIP_CONFLICT)
    {
        if(status)
            *status = SFIP_CONFLICT;

        sfvar_free(ret);
        return NULL;
    }

    return ret;
}

static inline sfip_node_t *_sfvar_deep_copy_list(const sfip_node_t *idx)
{
    sfip_node_t *ret, *temp, *prev;

    ret = temp = NULL;

    for( ; idx; idx = idx->next)
    {
        prev = temp;

        if( (temp = (sfip_node_t*)calloc(1, sizeof(sfip_node_t))) == NULL )
        {
            sfip_node_freelist(ret);
            return NULL;
        }
        if( (temp->ip = (sfcidr_t*)calloc(1, sizeof(sfcidr_t))) == NULL )
        {
            sfip_node_freelist(ret);
            free(temp);
            return NULL;
        }

        temp->flags = idx->flags;
        temp->addr_flags = idx->addr_flags;

        /* If it's an "any", there may be no IP object */
        if(idx->ip)
            memcpy(temp->ip, idx->ip, sizeof(sfcidr_t));

        if(prev)
            prev->next = temp;
        else
            ret = temp;
    }
    return ret;
}

sfip_var_t *sfvar_deep_copy(const sfip_var_t *var)
{
    sfip_var_t *ret;

    if(!var)
        return NULL;

    ret = (sfip_var_t*)SnortAlloc(sizeof(sfip_var_t));

    ret->mode = var->mode;
    ret->head = _sfvar_deep_copy_list(var->head);
    ret->neg_head = _sfvar_deep_copy_list(var->neg_head);
    ret->head_count = var->head_count;
    ret->neg_head_count = var->neg_head_count;

    return ret;
}


/* Support function for sfvar_ip_in  */
static inline int _sfvar_ip_in4(sfip_var_t *var, sfaddr_t *ip)
{
    int match;
    sfip_node_t *pos_idx, *neg_idx;

    match = 0;

    pos_idx = var->head;
    neg_idx = var->neg_head;

    if(!pos_idx)
    {
        for( ; neg_idx; neg_idx = neg_idx->next)
        {
            if(sfaddr_family(&neg_idx->ip->addr) != AF_INET)
                continue;

            if(sfip_fast_cont4(neg_idx->ip, ip))
            {
                return 0;
            }
        }

        return 1;
    }

    while(pos_idx)
    {
        if(neg_idx)
        {
            if(sfaddr_family(&neg_idx->ip->addr) == AF_INET &&
                sfip_fast_cont4(neg_idx->ip, ip))
            {
                return 0;
            }

            neg_idx = neg_idx->next;
        }
        /* No more potential negations.  Check if we've already matched. */
        else if(match)
        {
            return 1;
        }

        if(!match)
        {
            if(sfip_is_set(pos_idx->ip))
            {
                if(sfaddr_family(&pos_idx->ip->addr) == AF_INET &&
                   sfip_fast_cont4(pos_idx->ip, ip))
                {
                    match = 1;
                }
                else
                {
                    pos_idx = pos_idx->next;
                }
            }
            else
            {
                match = 1;
            }
        }
    }

    return 0;
}

/* Support function for sfvar_ip_in  */
static inline int _sfvar_ip_in6(sfip_var_t *var, sfaddr_t *ip)
{
    int match;
    sfip_node_t *pos_idx, *neg_idx;

    match = 0;

    pos_idx = var->head;
    neg_idx = var->neg_head;

    if(!pos_idx)
    {
        for( ; neg_idx; neg_idx = neg_idx->next)
        {
            if(sfaddr_family(&neg_idx->ip->addr) != AF_INET6)
                continue;

            if(sfip_fast_cont6(neg_idx->ip, ip))
            {
                return 0;
            }
        }

        return 1;
    }

    while(pos_idx)
    {
        if(neg_idx)
        {
            if(sfaddr_family(&neg_idx->ip->addr) == AF_INET6 &&
                sfip_fast_cont6(neg_idx->ip, ip))
            {
                return 0;
            }

            neg_idx = neg_idx->next;
        }
        /* No more potential negations.  Check if we've already matched. */
        else if(match)
        {
            return 1;
        }

        if(!match)
        {
            if(sfip_is_set(pos_idx->ip))
            {

                if(sfaddr_family(&pos_idx->ip->addr) == AF_INET6 &&
                   sfip_fast_cont6(pos_idx->ip, ip))
                {
                     match = 1;
                }
                else
                {
                    pos_idx = pos_idx->next;
                }
            }
            else
            {
                match = 1;
            }
        }
    }

    return 0;
}
/* Returns SFIP_SUCCESS if ip is contained in 'var', SFIP_FAILURE otherwise */
/* If either argument is NULL, SFIP_ARG_ERR is returned. */
int sfvar_ip_in(sfip_var_t *var, sfaddr_t *ip)
{
    if(!var || !ip)
        return 0;

#if 0
    if(var->mode == SFIP_TABLE)
    {
        // XXX
    }
    else
    {
#endif
        /* Since this is a performance-critical function it uses different
         * codepaths for IPv6 and IPv4 traffic, rather than the dual-stack
         * functions. */

        if(sfaddr_family(ip) == AF_INET)
        {
            return _sfvar_ip_in4(var, ip);
        }
        else
        {
            return _sfvar_ip_in6(var, ip);
        }
#if 0
    }
#endif
}

static char buffer[1024];
void sfip_set_print(const char *prefix, sfip_node_t *p)
{
    int ret;
    for(; p; p = p->next)
    {
        buffer[0] = '\0';
        if(!p->ip) continue;
        if(p->flags & SFIP_NEGATED)
        {
            if (p->ip->bits != 128)
            {
                if (sfaddr_family(&p->ip->addr) == AF_INET6)
                {
                    ret = SnortSnprintfAppend(buffer, sizeof(buffer), "!%s/%d", sfip_to_str(&p->ip->addr), p->ip->bits);
                }
                else
                {
                    ret = SnortSnprintfAppend(buffer, sizeof(buffer), "!%s/%d", sfip_to_str(&p->ip->addr), p->ip->bits < 96 ? -1 : p->ip->bits - 96);
                }
            }
            else
            {
                ret = SnortSnprintfAppend(buffer, sizeof(buffer), "!%s", sfip_to_str(&p->ip->addr));
            }
            if (ret != SNORT_SNPRINTF_SUCCESS)
                return;
        }
        else
        {
            if (p->ip->bits != 128)
            {
                if (sfaddr_family(&p->ip->addr) == AF_INET6)
                {
                    ret = SnortSnprintfAppend(buffer, sizeof(buffer), "%s/%d", sfip_to_str(&p->ip->addr), p->ip->bits);
                }
                else
                {
                    ret = SnortSnprintfAppend(buffer, sizeof(buffer), "%s/%d", sfip_to_str(&p->ip->addr), p->ip->bits < 96 ? -1 : p->ip->bits - 96);
                }
            }
            else
            {
                ret = SnortSnprintfAppend(buffer, sizeof(buffer), "%s", sfip_to_str(&p->ip->addr));
            }
            if (ret != SNORT_SNPRINTF_SUCCESS)
                return;
        }
        if (prefix)
            LogMessage("%s%s\n", prefix, buffer);
        else
            LogMessage("%s\n", buffer);
    }
}

void sfvar_print(const char *prefix, sfip_var_t *var)
{
   if (!var || !var->head)
   {
       return;
   }

    if(var->mode == SFIP_LIST)
    {
        if(var->head->flags & SFIP_ANY)
        {
            if (prefix)
                LogMessage("%sany\n", prefix);
            else
                LogMessage("any\n");
        }
        else
        {
            sfip_set_print(prefix, var->head);
        }
    }
    else if(var->mode == SFIP_TABLE)
    {
        // XXX
    }
}

void sfip_set_print_to_file(FILE *f, sfip_node_t *p)
{
     for(; p; p = p->next)
     {
         if(!p->ip) continue;
         if(p->flags & SFIP_NEGATED)
             fprintf(f, "\t!%s\n", sfip_to_str(&p->ip->addr));
         else
             fprintf(f, "\t %s\n", sfip_to_str(&p->ip->addr));
     }
}

/* Prints the variable "var" to the file descriptor 'f' */
void sfvar_print_to_file(FILE *f, sfip_var_t *var)
{
    if(!f) return;

    if(!var || !var->head)
    {
        fprintf(f, "[no variable]\n");
        return;
    }

    fprintf(f, "Name: %s\n", var->name);

    if(var->mode == SFIP_LIST)
    {
        if(var->head->flags & SFIP_ANY)
            fprintf(f, "\t%p: <any>\n", (void*)var->head);
        else
        {
            sfip_set_print_to_file(f, var->head);
        }
    }
    else if(var->mode == SFIP_TABLE)
    {
        // XXX
    }
}

int sfvar_flags(sfip_node_t *node)
{
    if(node) return node->flags;
    return -1;
}

/* XXX The unit tests for this code are performed within sf_vartable.c */
#if 0

int main()
{
    sfip_vtable *table;
    sfip_var_t *var;
    sfcidr_t *ip;

    /* Test parsing */
    /* Allowable arguments:
     *      { <ip>[, <ip>, ... , <ip> }
     * Where an IP can be in CIDR notation, or be specified with a netmask.
     * IPs may also be negated with '!' */
    puts("********************************************************************");
    puts("Testing parsing:");
    var = sfvar_str(" {   1.2.3.4/8,  5.5.5.5 255.255.255.0, any} ");
    sfip_print_var(stdout, var);
    sfvar_free(var);
    puts("");
    var = sfvar_str(" {   1.2.3.4,  ffff::3/127, 0.0.0.1} ");
    sfip_print_var(stdout, var);
    ip = sfip_alloc("1.2.3.5");
    printf("(need more of these) 'in': %d\n", sfip_in(var, ip));
    puts("also, use 'sfip_in' for the unit tests");
    puts("");

    return 0;
}

#endif

