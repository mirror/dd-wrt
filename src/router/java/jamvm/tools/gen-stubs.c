/*
 * Copyright (C) 2010, 2011, 2012 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define GEN_STUBS_INC

#include "../jam.h"
#include "../stubs.c"

typedef struct {
    char **list;
    int count;
    int size;
} List;

List static_sigs;
List non_static_sigs;

void *sysMalloc(int size) {
    void *mem = malloc(size);

    if(mem == NULL) {
        perror("Malloc failed\n");
        exit(1);
    }

    return mem;
}

void *sysRealloc(void *addr, int size) {
    void *mem = realloc(addr, size);

    if(mem == NULL) {
        perror("Realloc failed\n");
        exit(1);
    }

    return mem;
}

void sysFree(void *addr) {
    free(addr);
}

void addToList(char *sig, List *list) {
    if(list->count == list->size) {
        list->size += 256;
        list->list = sysRealloc(list->list, list->size * sizeof(char*));
    }

    list->list[list->count++] = sig;
}

void printList(List *list) {
    int i;

    for(i = 0; i < list->count; i++)
        printf("%s\n", list->list[i]);
}

int compare(const void *p1, const void *p2) {
    return strcmp(*(char **)p1, *(char **)p2);
}

void uniqueList(List *list) {
    int i, j;

    if(list->count < 2)
        return;

    qsort(list->list, list->count, sizeof(char*), compare);

    /* Remove duplicates */
    for(i = 0; i < list->count-1; i++) {
        for(j = i + 1; j < list->count && strcmp(list->list[i],
                                                 list->list[j]) == 0; j++)
            free(list->list[j]);

        if(j - i > 1) {
            if(j < list->count)
                memmove(&list->list[i+1], &list->list[j],
                        (list->count-j) * sizeof(char*));
            list->count -= j-i-1;
        }
    }
}

char *checkSigElement(char *sig, int ret_element) {
    switch(*sig) {
        case 'V':
            if(!ret_element)
                return NULL;

        case 'Z':
        case 'B':
        case 'C':
        case 'S':
        case 'I':
        case 'F':
        case 'J':
        case 'D':
            return ++sig;

        case '[':
            return checkSigElement(++sig, FALSE);

        case 'L':
            for(;;) {
                if(*sig == '\0')
                    return NULL;
                if(*sig++ == ';')
                    return sig;
            }

        default:
            return NULL;
    }
}

char *skipSpace(char *pntr) {
    while(isblank(*pntr))
       pntr++;

   return pntr;
}

void parseSignature(char *line) {
    char *pntr, *sig_start;

    line = skipSpace(line);

    /* Ignore empty and comment lines */
    if(*line == '\0' || *line == '#')
        return;

    if(strncmp(line, "static", 6) == 0)
        sig_start = skipSpace(line + 6);
    else
        sig_start = line;

    pntr = sig_start;

    if(*pntr++ != '(')
        goto error;

    /* Check parameters */
    while(*pntr != ')')
        if((pntr = checkSigElement(pntr, FALSE)) == NULL)
            goto error;

    /* Check return type */
    if((pntr = checkSigElement(++pntr, TRUE)) == NULL)
        goto error;

    /* Check for trailling characters */
    if(*skipSpace(pntr) != '\0')
        goto error;

    pntr = convertSig2Simple(sig_start);
    addToList(pntr, sig_start == line ? &non_static_sigs
                                      : &static_sigs);

    return;

error:
    printf("Syntax error in line: %s\n", line);
}

void readSignatures(char *filename) {
    FILE *fd = fopen(filename, "r");
    char *buff = sysMalloc(256);
    int buff_len = 256;
    int pos = 0;

    if(fd == NULL) {
        perror("Couldn't open signatures file for reading");
        exit(1);
    }

    while(fgets(buff + pos, buff_len - pos, fd) != NULL) {
        int len = strlen(buff + pos);

        if(buff[pos + len-1] == '\n') {
            buff[pos + len-1] = '\0';
            pos = 0;

            parseSignature(buff);
        } else {
            buff = sysRealloc(buff, buff_len *= 2);
            pos += len;
        }
    }

    if(pos != 0)
        parseSignature(buff);

    fclose(fd);
    free(buff);
}

char *sigElement2Type(char element) {
    switch(element) {
        case 'B':
        case 'Z':
            return "int8_t";
        case 'C':
            return "uint16_t";
        case 'S':
            return "int16_t";
        case 'J':
            return "int64_t";
        case 'D':
            return "double";
        case 'F':
            return "float";
        case 'V':
            return "void";
        default:
            return "uintptr_t";
    }
}

char *sigElement2StackCast(char element) {
    switch(element) {
        case 'J':
            return "*(int64_t*)ostack = ";
        case 'D':
            return "*(double*)ostack = ";
        case 'F':
            return "*((float*)ostack + IS_BE64) = ";
        case 'V':
            return "";
        default:
            return "*ostack = ";
    }
}

int sigElement2Size(char element) {
    switch(element) {
        case 'J':
        case 'D':
            return 2;
        case 'V':
            return 0;
        default:
            return 1;
    }
}

char *mangleSig(char *sig, int is_static) {
    int len = strlen(sig);
    char *mangled = strdup(sig);
    mangled[len-2] = mangled[0] = '_';

    if(is_static) {
        char *s = sysMalloc(len + 7);
        mangled = strcat(strcpy(s, "static"), mangled);
    }

    return mangled;
}

void writeStubs(FILE *fd, List *list, int profiling) {
    int i;

    for(i = 0; i < list->count; i++) {
        int sp = 0;
        char *pntr;
        char *sig = list->list[i];
        char ret = sig[strlen(sig)-1];
        int is_static = list == &static_sigs;
        char *mangled = mangleSig(sig, is_static);

        fprintf(fd, "/* %s */\n", sig);
        fprintf(fd, "static uintptr_t *%s(Class *class, MethodBlock *mb, "
                    "uintptr_t *ostack) {\n", mangled);

        if(profiling)
            fprintf(fd, "    jni%s_stubs[%d].profile_count++;\n\n",
                        is_static ? "_static" : "", i);

        fprintf(fd, "    if(!initJNILrefs())\n");
        fprintf(fd, "        return NULL;\n\n");

        fprintf(fd, "    %s", sigElement2StackCast(ret));

        if(ret == 'L')
            fprintf(fd, "(uintptr_t)REF_TO_OBJ(");

        fprintf(fd, "(*(%s (*)(void*, void*", sigElement2Type(ret));

        for(pntr = sig + 1; *pntr != ')'; pntr++)
            fprintf(fd, ", %s", sigElement2Type(*pntr));

        fprintf(fd, "))mb->code) (\n\t&jni_env,\n");

        if(is_static)
            fprintf(fd, "\tclass");
        else {
            fprintf(fd, "\t(void*)ostack[0]");
            sp++;
        }
 
        for(pntr = sig + 1; *pntr != ')'; pntr++) {
            if(*pntr == 'F')
                fprintf(fd, ",\n\t*((float *)&ostack[%d] + IS_BE64)", sp);
            else
                fprintf(fd, ",\n\t*(%s *)&ostack[%d]", sigElement2Type(*pntr), sp);
            sp += sigElement2Size(*pntr);
        }

        if(ret == 'L')
            fprintf(fd, ")");

        fprintf(fd, ");\n\n");
        fprintf(fd, "    return ostack + %d;\n", sigElement2Size(*++pntr));

        fprintf(fd, "}\n\n");
    }
}

void writeStubsTable(FILE *fd, List *list, int profiling) {
    int i;
    int is_static = list == &static_sigs;
    char *profile_count_init = profiling ? ", 0" : "";
    char *name = is_static ? "jni_static_stubs" : "jni_stubs";

    fprintf(fd, "\nJNIStub %s[] = {\n", name);

    for(i = 0; i < list->count; i++)
        fprintf(fd, "    {\"%s\", %s%s},\n", list->list[i],
                    mangleSig(list->list[i], is_static),
                    profile_count_init);

    fprintf(fd, "    {NULL, NULL%s}\n", profile_count_init);
    fprintf(fd, "};\n");
}

void writeStubsFile(char *stubs_name, char *sigs_name, int profiling) {
    FILE *fd = fopen(stubs_name, "w");

    if(fd == NULL) {
        perror("Couldn't open stubs file for writing");
        exit(1);
    }

    fprintf(fd, "/* Generated by gen-stubs.c from signature file %s "
                "(profiling %s) */\n\n", sigs_name, profiling ? "on" : "off");

    fprintf(fd, "#include \"jam.h\"\n");
    fprintf(fd, "#include \"stubs.h\"\n");
    fprintf(fd, "#include \"properties.h\"\n");
    fprintf(fd, "#include \"jni-internal.h\"\n");
    fprintf(fd, "\nextern void *jni_env;\n\n");

    fprintf(fd, "/* Static signatures */\n\n");
    writeStubs(fd, &static_sigs, profiling);

    fprintf(fd, "/* Non-static signatures */\n\n");
    writeStubs(fd, &non_static_sigs, profiling);

    writeStubsTable(fd, &static_sigs, profiling);
    writeStubsTable(fd, &non_static_sigs, profiling);

    fclose(fd);
}

int main(int argc, char *argv[]) {
    if(!(argc == 3 || (argc == 4 && strcmp(argv[3], "-profile") == 0))) {
        printf("Usage: %s <input signatures file> <output stubs file> "
               "[-profile]\n", argv[0]);
        return 1;
    }

    readSignatures(argv[1]);

    uniqueList(&static_sigs);
    uniqueList(&non_static_sigs);

    printf("Static signatures: %d\n", static_sigs.count);
    printList(&static_sigs);

    printf("\nNon-static signatures: %d\n", non_static_sigs.count);
    printList(&non_static_sigs);

    writeStubsFile(argv[2], argv[1], argc == 4);
    return 0;
}
