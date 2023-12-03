#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "utils.h"
#include "memory_utils.h"

#define DAWN_MEM_FILENAME_LEN 20
struct mem_list {
    struct mem_list* next_mem;
    int line;
    char file[DAWN_MEM_FILENAME_LEN];
    char type;
    size_t size;
    void* ptr;
    uint64_t ref;
};

static struct mem_list* mem_base = NULL;
static uint64_t alloc_ref = 0;

void* dawn_memory_alloc(enum dawn_memop type, char* file, int line, size_t nmemb, size_t size, void *ptr)
{
void* ret = NULL;

    switch (type)
    {
    case DAWN_MALLOC:
        ret = malloc(size);
        break;
    case DAWN_REALLOC:
        ret = realloc(ptr, size);
        if (ret != NULL)
/*
GCC v12 has a new Wuse-after-free error. We can not unregister 
the memory before doing a reallocation. The call can fail, so
ptr is never freed. However, the warning can be ignored, since
ptr is only used in the unregister function as a reference to
remove it from our memory auditing.
*/
#if (__GNUC__ >= 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuse-after-free"
            dawn_memory_unregister(DAWN_REALLOC, file, line, ptr);
#pragma GCC diagnostic pop
#else
            dawn_memory_unregister(DAWN_REALLOC, file, line, ptr);
#endif
        break;
    case DAWN_CALLOC:
        ret = calloc(nmemb, size);
        size *= nmemb; // May not be correct allowing for padding but gives a sense of scale
        break;
    default:
        break;
    }

    if (ret != NULL)
        dawn_memory_register(type, file, line, size, ret);

    return ret;
}

void* dawn_memory_register(enum dawn_memop type, char* file, int line, size_t size, void* ptr)
{
    void* ret = NULL;
    struct mem_list* this_log = NULL;
    char type_c = '?';

    switch (type)
    {
    case DAWN_MALLOC:
        type_c = 'M';
        break;
    case DAWN_REALLOC:
        type_c = 'R';
        break;
    case DAWN_CALLOC:
        type_c = 'C';
        break;
    case DAWN_MEMREG:
        type_c = 'X';
        break;
    default:
        dawnlog_warning("mem-audit: Unexpected memory op tag!\n");
        break;
    }

    // Note effort to  register a failed allocation (other code probably wrong as well)
    if (ptr == NULL)
    {
        char* xfile = strrchr(file, '/');

        dawnlog_warning("mem-audit: attempting to register failed allocation (%c@%s:%d)...\n", type_c, xfile ? xfile + 1 : file, line);
        return ret;
    }

    // Insert to linked list with ascending memory reference
    struct mem_list** ipos = &mem_base;
    while (*ipos != NULL && (*ipos)->ptr < ptr)
        ipos = &((*ipos)->next_mem);

    if (*ipos != NULL && (*ipos)->ptr == ptr)
    {
        char* xfile = strrchr(file, '/');

        dawnlog_warning("mem-audit: attempted to register memory already registered (%c@%s:%d)...\n", type_c, xfile ? xfile + 1 : file, line);
    }
    else
    {
        this_log = malloc(sizeof(struct mem_list));

        if (this_log == NULL)
        {
            dawnlog_error("mem-audit: Oh the irony! malloc() failed in dawn_memory_register()!\n");
        }
        else
        {
            // Just use filename - no path
            char *xfile = strrchr(file, '/');

            dawnlog_debug("mem-audit: registering memory (%c@%s:%d)...\n", type_c, xfile ? xfile + 1 : file, line);
            this_log->next_mem = *ipos;
            *ipos = this_log;

            if (xfile != NULL)
                strncpy(this_log->file, xfile + 1, DAWN_MEM_FILENAME_LEN);
            else
                strncpy(this_log->file, "?? UNKNOWN ??", DAWN_MEM_FILENAME_LEN);

            this_log->type = type_c;
            this_log->line = line;
            this_log->ptr = ptr;
            this_log->size = size;
            this_log->ref = alloc_ref++;
        }
    }

    return ret;
}

void dawn_memory_unregister(enum dawn_memop type, char* file, int line, void* ptr)
{
struct mem_list** mem = &mem_base;

    while (*mem != NULL && (*mem)->ptr < ptr)
    {
        mem = &((*mem)->next_mem);
    }

    char* xfile = strrchr(file, '/');

    if (*mem != NULL && (*mem)->ptr == ptr)
    {
        // Just use filename - no path
        dawnlog_debug("mem-audit: unregistering memory (%s:%d -> %c@%s:%d)...\n", xfile ? xfile + 1 : file, line, (*mem)->type, (*mem)->file, (*mem)->line);

        struct mem_list* tmp = *mem;
        *mem = tmp->next_mem;
        free(tmp);
    }
    else
    {
        dawnlog_warning("mem-audit: Releasing memory we hadn't registered (%s:%d)...\n", xfile ? xfile + 1 : file, line);
    }

    return;
}

void dawn_memory_free(enum dawn_memop type, char* file, int line, void* ptr)
{
    dawn_memory_unregister(type, file, line, ptr);

    free(ptr);

    return;
}

void dawn_memory_audit()
{
size_t    total = 0;

    dawnlog_always("mem-audit: Currently recorded allocations...\n");
    for (struct mem_list* mem = mem_base; mem != NULL; mem = mem->next_mem)
    {
        dawnlog_always("mem-audit: %8" PRIu64 "=%c - %s@%d: %zu\n", mem->ref, mem->type, mem->file, mem->line, mem->size);
        total += mem->size;
    }

    char *suffix = "bytes";
    if (total > 128 * 1024)
    {
        total /= 1024;
        suffix = "kbytes";
    }

    dawnlog_always("mem-audit: [End of list: %zu %s]\n", total, suffix);
}
