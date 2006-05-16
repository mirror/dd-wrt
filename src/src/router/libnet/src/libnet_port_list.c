/*
 *  $Id: libnet_port_list.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_port_list.c - transport layer port list chaining code
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include "../include/libnet.h"


int
libnet_plist_chain_new(struct libnet_plist_chain **head, char *tok_list)
{
    char libnet_plist_legal_tokens[] = "0123456789,- ";
    struct libnet_plist_chain *tmp;
    char *tok;
    int i, j, valid_token, cur_node;

    if (!tok_list)
    {
        return (-1);
    }

    /*
     *  Make sure we have legal tokens.
     */
    for (i = 0; tok_list[i]; i++)
    {
        for (j = 0, valid_token = 0; libnet_plist_legal_tokens[j]; j++)
        {
            if (libnet_plist_legal_tokens[j] == tok_list[i])
            {
                valid_token = 1;
                break;
            }
        }
        if (!valid_token)
        {
#if (__DEBUG)
            libnet_error(LIBNET_ERR_CRITICAL,
                    "libnet_build_plist_chain: illegal token # %d (%c)\n",
                    i + 1,
                    tok_list[i]);
#endif
            *head = NULL;
            return (-1);
        }
    }

    /*
     *  Head node.
     */
    *head = 
        (struct libnet_plist_chain *)malloc(sizeof(struct libnet_plist_chain));

    if (!(*head))
    {
#if (__DEBUG)
        perror("libnet_build_plist_chain: malloc");
#endif
        *head = NULL;
        return(-1);
    }

    tmp = *head;
    tmp->node = cur_node = 0;
    tmp->next = NULL;

    /*
     *  Using strtok successively proved problematic.  We solve this by
     *  calling it once, then manually extracting the elements from the token.
     *  In the case of bport > eport, we swap them.
     */
    for (i = 0; (tok = strtok(!i ? tok_list : NULL, ",")); i = 1, cur_node++)
    {
        /*
         *  The first iteration we will have a head node allocated so we don't
         *  need to malloc().
         */
        if (i)
        {
            tmp->next = (struct libnet_plist_chain *)
                        malloc(sizeof(struct libnet_plist_chain));
            if (!tmp)
            {
#if (__DEBUG)
                perror("libnet_build_plist_chain: malloc");
#endif
                /*
                 *  XXX - potential memory leak if other nodes are allocated
                 *  but not freed.
                 */
                *head = NULL;
                return(-1);
            }
            tmp = tmp->next;
            tmp->node = cur_node;
            tmp->next = NULL;
        }
        tmp->bport = atoi(tok);

        /*
         *  Step past this port number.
         */
        j = 0;
        while (isdigit(tok[j]))
        {
            j++;
        }

        /*
         *  If we have a delimiting dash and are NOT at the end of the token
         *  array, we can assume it's the end port, otherwise if we just have
         *  a dash, we consider it shorthand for `inclusive of all ports up to
         *  65535.  Finally, if we have no dash, we assume this token is a
         *  single port only.
         */
        if (tok[j] == '-')
        {
            tmp->eport = (++j != strlen(tok)) ? atoi(&tok[j]) : 65535;
        }
        else
        {
            tmp->eport = tmp->bport;
        }

        /*
         *  Do we need to swap the values?
         */
        if (tmp->bport > tmp->eport)
        {
            tmp->bport ^= tmp->eport;
            tmp->eport ^= tmp->bport;
            tmp->bport ^= tmp->eport;
        }
    }

    /*
     *  The head node needs to hold the total node count.
     */
    (*head)->node = cur_node;
    return (1);
}


int
libnet_plist_chain_next_pair(struct libnet_plist_chain *p, u_short *bport,
        u_short *eport)
{
    static u_short node_cnt;         /* XXX - reentrancy == no */ 
    u_short tmp_cnt;

    if (!p)
    {
        return (-1);
    }

    /*
     *  We are at the end of the list.
     */
    if (node_cnt == p->node)
    {
        node_cnt = 0;
        *bport = 0;
        *eport = 0;
        return (0);
    }

    for (tmp_cnt = node_cnt; tmp_cnt; tmp_cnt--, p = p->next) ;
    *bport = p->bport;
    *eport = p->eport;
    node_cnt++;
    return (1);
}


int
libnet_plist_chain_dump(struct libnet_plist_chain *p)
{
    if (!p)
    {
        return (-1);
    }

    for (; p; p = p->next)
    {
        if (p->bport == p->eport)
        {
            fprintf(stdout, "%d ", p->bport);
        }
        else
        {
            fprintf(stdout, "%d-%d ", p->bport, p->eport);
        }
    }
    fprintf(stdout, "\n");
    return (1);
}


u_char *
libnet_plist_chain_dump_string(struct libnet_plist_chain *p)
{
    u_char buf[BUFSIZ] = {0};
    int i, j;

    if (!p)
    {
        return (NULL);
    }

    for (i = 0, j = 0; p; p = p->next)
    {
        if (p->bport == p->eport)
        {
            i = sprintf(&buf[j], "%d", p->bport);
        }
        else
        {
            i = sprintf(&buf[j], "%d-%d", p->bport, p->eport);
        }
        j += i;
        if (p->next)
        {
            sprintf(&buf[j++], ",");
        }
    }
    return (strdup(buf));       /* XXX - reentrancy == no */
}


int
libnet_plist_chain_free(struct libnet_plist_chain *p)
{
    u_short i;
    struct libnet_plist_chain *tmp;

    if (!p)
    {
        return (-1);
    }

    for (i = p->node; i; i--)
    {
        tmp = p;
        p = p->next;
        free(tmp);
    }
    p = NULL;
    return (1);
}


/* EOF */
