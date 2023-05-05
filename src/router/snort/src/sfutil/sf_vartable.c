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
 * sf_vartable.c
 * 11/17/06
 *
 * Library for managing IP variables.
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "sf_vartable.h"
#include "util.h"

vartable_t * sfvt_alloc_table(void)
{
    vartable_t *table = (vartable_t *)SnortAlloc(sizeof(vartable_t));

    /* ID for recognition of variables with different name, but same content
     * Start at 1, so a value of zero indicates not set.
     * This value should be incremented for each variable that hasn't been
     * identified as an alias of another variable */
    table->id = 1;

    return table;
}

static char * sfvt_expand_value(vartable_t *table, char *value)
{
    char *ptr, *end, *tmp, *ret = NULL;
    int retlen = 0, retsize = 0;
    int escaped = 0;

    if ((table == NULL) || (value == NULL))
        return NULL;

    if (strlen(value) == 0)
        return NULL;

    ptr = value;
    end = value + strlen(value);
    while ((ptr < end) && isspace((int)*ptr))
        ptr++;
    while ((end > ptr) && isspace((int)*(end-1)))
        end--;
    if (ptr == end)
        return NULL;

    tmp = SnortStrndup(ptr, end-ptr);
    if (tmp == NULL)
        return NULL;

    /* Start by allocating the length of the value */
    retsize = strlen(value) + 1;
    ret = (char *)SnortAlloc(retsize);

    ptr = tmp;
    end = tmp + strlen(tmp);
    while (ptr < end)
    {
        if (!escaped && (*ptr == '$'))
        {
            char *varstart, *vartmp;
            int parens = 0;
            sfip_var_t *ipvar;

            ptr++;
            if (ptr >= end)
                goto sfvt_expand_value_error;

            if (*ptr == '(')
            {
                ptr++;
                parens = 1;
            }

            varstart = ptr;
            while (ptr < end)
            {
                if (parens)
                {
                    if (*ptr == ')')
                    {
                        break;
                    }
                }
                else if (!isalnum((int)*ptr) && (*ptr != '_'))
                {
                    break;
                }

                ptr++;
            }

            if (varstart == ptr)
                goto sfvt_expand_value_error;

            vartmp = SnortStrndup(varstart, ptr - varstart);
            if (vartmp == NULL)
                goto sfvt_expand_value_error;

            ipvar = sfvt_lookup_var(table, vartmp);
            free(vartmp);
            if (ipvar == NULL)
                goto sfvt_expand_value_error;

            if (ipvar->value != NULL)
            {
                if ((int)(retlen + strlen(ipvar->value)) >= retsize)
                {
                    char *tmpalloc;

                    retsize = retlen + strlen(ipvar->value) + (end - ptr) + 1;
                    tmpalloc = (char *)SnortAlloc(retsize);
                    memcpy(tmpalloc, ret, retlen);
                    memcpy(tmpalloc + retlen, ipvar->value, strlen(ipvar->value));
                    free(ret);
                    retlen += strlen(ipvar->value);
                    ret = tmpalloc;
                }
            }

            if (parens)
                ptr++;

            continue;
        }

        if (*ptr == '\\')
            escaped = 1;
        else
            escaped = 0;

        ret[retlen++] = *ptr;
        ptr++;
    }

    free(tmp);

    if ((retlen + 1) < retsize)
    {
        char *tmpalloc = (char *)SnortAlloc(retlen + 1);
        memcpy(tmpalloc, ret, retlen);
        free(ret);
        ret = tmpalloc;
    }

    ret[retlen] = 0;
    return ret;

sfvt_expand_value_error:
    free(ret);
    free(tmp);
    return NULL;
}

// XXX this implementation is just used to support
// Snort's underlying implementation better
SFIP_RET sfvt_define(vartable_t *table, char *name, char *value)
{
    char *buf;
    int len;
    sfip_var_t *ipret = NULL;
    SFIP_RET ret;

    if(!name || !value) return SFIP_ARG_ERR;

    len = strlen(name) + strlen(value) + 2;

    if((buf = (char*)malloc(len)) == NULL)
    {
        return SFIP_FAILURE;
    }

    SnortSnprintf(buf, len, "%s %s", name, value);

    ret = sfvt_add_str(table, buf, &ipret);
    if ((ret == SFIP_SUCCESS) || (ret == SFIP_DUPLICATE))
        ipret->value = sfvt_expand_value(table, value);
    free(buf);
    return ret;
}

/* Adds the variable described by "str" to the table "table" */
SFIP_RET sfvt_add_str(vartable_t *table, char *str, sfip_var_t **ipret)
{
    sfip_var_t *var;
    sfip_var_t *swp;
    sfip_var_t *p;
    int ret;
    SFIP_RET status;

    if(!table || !str || !ipret) return SFIP_FAILURE;

    /* Creates the variable */
    if( (var = sfvar_alloc(table, str, &status)) == NULL )
    {
         return status;
    }

    /* If this is an alias of another var, id will be set */
    if (var->id == 0)
        var->id = table->id++;

    *ipret = var;

    /* Insertion sort */

    if(!table->head)
    {
        table->head = var;
        return SFIP_SUCCESS;
    }

    if((ret = strcmp(var->name, table->head->name)) < 0)
    {
        var->next = table->head;
        table->head = var;
        return SFIP_SUCCESS;
    }
    /* Redefinition */
    else if(ret == 0)
    {
        var->next = table->head->next;
        sfvar_free(table->head);
        table->head = var;
        return SFIP_DUPLICATE;
    }

    /* The loop below checks table->head->next->name in the first iteration.
     * Make sure there is a table->head->next first */
    if(!table->head->next)
    {
        table->head->next = var;
        return SFIP_SUCCESS;
    }
    else if(!strcmp(var->name, table->head->next->name))
    {
        var->next = table->head->next->next;
        sfvar_free(table->head->next);
        table->head->next = var;
        return SFIP_DUPLICATE;
    }

    for(p = table->head; p->next; p=p->next)
    {
        if((ret = strcmp(var->name, p->next->name)) < 0)
        {
            swp = p->next;
            p->next = var;
            var->next = swp;

            return SFIP_SUCCESS;
        }
        /* Redefinition */
        else if(ret == 0)
        {
            var->next = p->next->next;
            sfvar_free(p->next);
            p->next = var;
            return SFIP_DUPLICATE;
        }
    }

    p->next = var;
    return SFIP_SUCCESS;
}

/* Adds the variable described by "src" to the variable "dst",
 * using the vartable for looking variables used within "src" */
SFIP_RET sfvt_add_to_var(vartable_t *table, sfip_var_t *dst, char *src)
{
    SFIP_RET ret;

    if(!table || !dst || !src) return SFIP_ARG_ERR;

    if((ret = sfvar_parse_iplist(table, dst, src, 0)) == SFIP_SUCCESS)
        return sfvar_validate(dst);

    return ret;
}

/* Looks up a variable from the table by the variable's name  */
sfip_var_t *sfvt_lookup_var(vartable_t *table, char *name)
{
    sfip_var_t *p;
    int len;
    char *end;

    if(!table || !name) return NULL;

    if(*name == '$') name++;

    /* XXX should I assume there will be trailing garbage or
     * should I automatically find where the variable ends? */
    for(end=name;
        *end && !isspace((int)*end) && *end != '\\' && *end != ']';
        end++) ;
    len = end - name;

    for(p=table->head; len && p; p=p->next)
    {
        int name_len = strlen(p->name);
        if((len == name_len) && !strncmp(p->name, name, len)) return p;
    }

    return NULL;
}

void sfvt_free_table(vartable_t *table)
{
    sfip_var_t *p, *tmp;

    if (!table) return;

    p = table->head;
    while (p)
    {
        tmp = p->next;
        sfvar_free(p);
        p = tmp;
    }
    free(table);
}

/* Prints a table's contents */
void sfip_print_table(FILE *f, vartable_t *table)
{
    sfip_var_t *p;

    if(!f || !table) return;

    fprintf(f, "(Table %p)\n", (void*)table);
    for(p=table->head; p; p=p->next)
    {
        sfvar_print_to_file(f, p);
        puts("");
    }
}

//#define TESTER

#ifdef TESTER
int failures = 0;
#define TEST(x) if(x) printf("\tSuccess: line %d\n", __LINE__);\
                else { printf("\tFAILURE: line %d\n", __LINE__); failures++; }

int main()
{
    vartable_t *table;
    sfip_var_t *var;
    sfcidr_t *ip;

    puts("********************************************************************");
    puts("Testing variable table parsing:");
    table = sfvt_alloc_table();
    /* These are all valid */
    TEST(sfvt_add_str(table, "foo [ 1.2.0.0/16, ffff:dead:beef::0 ] ", &var) == SFIP_SUCCESS);
    TEST(sfvt_add_str(table, " goo [ ffff:dead:beef::0 ] ", &var) == SFIP_SUCCESS);
    TEST(sfvt_add_str(table, " moo [ any ] ", &var) == SFIP_SUCCESS);

    /* Test variable redefine */
    TEST(sfvt_add_str(table, " goo [ 192.168.0.1, 192.168.0.2, 192.168.255.0 255.255.248.0 ] ", &var) == SFIP_DUPLICATE);

    /* These should fail since it's a variable name with bogus arguments */
    TEST(sfvt_add_str(table, " phlegm ", &var) == SFIP_FAILURE);
    TEST(sfvt_add_str(table, " phlegm [", &var) == SFIP_FAILURE);
    TEST(sfvt_add_str(table, " phlegm [ ", &var) == SFIP_FAILURE);
    TEST(sfvt_add_str(table, " phlegm [sdfg ", &var) == SFIP_FAILURE);
    TEST(sfvt_add_str(table, " phlegm [ sdfg, 12.123.1.4.5 }", &var) == SFIP_FAILURE);
    TEST(sfvt_add_str(table, " [ 12.123.1.4.5 ]", &var) == SFIP_FAILURE);
    TEST(sfvt_add_str(table, NULL, &var) == SFIP_FAILURE);
    TEST(sfvt_add_str(table, "", &var) == SFIP_FAILURE);

    puts("");
    puts("********************************************************************");
    puts("Expansions:");
    /* Note: used this way leaks memory */
    printf("\t%s\n", sfvt_alloc_expanded(table, "$foo"));
    printf("\t%s\n", sfvt_alloc_expanded(table, "goo $goo sf sfasdfasdf $moo"));
    printf("\t%s\n", sfvt_alloc_expanded(table, " ssdf $moo $moo asdf $fooadff $foo "));
    printf("\t%s\n", sfvt_alloc_expanded(table, " ssdf $moo $moo\\sdf $foo adff"));

    puts("");
    puts("********************************************************************");
    puts("Containment checks:");
    var = sfvt_lookup(table, "goo");
    ip = sfip_alloc("192.168.248.255");
    TEST(sfvar_ip_in(var, ip) == SFIP_SUCCESS);

    /* Check against the 'any' variable */
    var = sfvt_lookup_var(table, "moo");
    TEST(sfvar_ip_in(var, ip) == SFIP_SUCCESS);

    /* Verify it's not in this variable */
    var = sfvt_lookup_var(table, "foo");
    TEST(sfvar_ip_in(var, ip) == SFIP_FAILURE);

    /* Check boundary cases */
    var = sfvt_lookup_var(table, "goo");
    free_ip(ip);
    ip = sfip_alloc_str("192.168.0.3");
    TEST(sfvar_ip_in(var, ip) == SFIP_FAILURE);
    free_ip(ip);
    ip = sfip_alloc_str("192.168.0.2");
    TEST(sfvar_ip_in(var, ip) == SFIP_SUCCESS);


    puts("");
    puts("********************************************************************");

    printf("\n\tTotal Failures: %d\n", failures);
    return 0;
}
#endif
