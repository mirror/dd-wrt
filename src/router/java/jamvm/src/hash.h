/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2011
 * Robert Lougher <rob@jamvm.org.uk>.
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

#include "thread.h"

typedef struct hash_entry {
    void *data;
    int hash;
} HashEntry;

typedef struct hash_table {
    HashEntry *hash_table;
    int hash_size;
    int hash_count;
    VMLock lock;
} HashTable;

extern void resizeHash(HashTable *table, int new_size);
extern void lockHashTable0(HashTable *table, Thread *self);
extern void unlockHashTable0(HashTable *table, Thread *self);

#define initHashTable(table, initial_size, create_lock)                \
{                                                                      \
    table.hash_table = gcMemMalloc(sizeof(HashEntry)*initial_size);    \
    memset(table.hash_table, 0, sizeof(HashEntry)*initial_size);       \
    table.hash_size = initial_size;                                    \
    table.hash_count = 0;                                              \
    if(create_lock)                                                    \
        initVMLock(table.lock);                                        \
}

#define lockHashTable(table)                                           \
    lockHashTable0(&table, threadSelf());

#define unlockHashTable(table)                                         \
    unlockHashTable0(&table, threadSelf());

#define hashTableCount(table)                                          \
    table.hash_count

#define findHashEntry(table, ptr, ptr2, add_if_absent, scavenge,       \
                      locked)                                          \
{                                                                      \
    int hash = HASH(ptr);                                              \
    int i;                                                             \
                                                                       \
    Thread *self;                                                      \
    if(locked) {                                                       \
        self = threadSelf();                                           \
        lockHashTable0(&table, self);                                  \
    }                                                                  \
                                                                       \
    i = hash & (table.hash_size - 1);                                  \
                                                                       \
    for(;;) {                                                          \
        ptr2 = table.hash_table[i].data;                               \
        if((ptr2 == NULL) ||                                           \
                 (COMPARE(ptr, ptr2, hash, table.hash_table[i].hash))) \
            break;                                                     \
                                                                       \
        i = (i+1) & (table.hash_size - 1);                             \
    }                                                                  \
                                                                       \
    if(ptr2) {                                                         \
        ptr2 = FOUND(ptr, ptr2);                                       \
    } else                                                             \
        if(add_if_absent) {                                            \
            table.hash_table[i].hash = hash;                           \
            ptr2 = table.hash_table[i].data = PREPARE(ptr);            \
                                                                       \
            if(ptr2) {                                                 \
                table.hash_count++;                                    \
                if((table.hash_count * 4) > (table.hash_size * 3)) {   \
                    int new_size;                                      \
                    if(scavenge) {                                     \
                        HashEntry *entry = table.hash_table;           \
                        int cnt = table.hash_count;                    \
                        for(; cnt; entry++) {                          \
                            void *data = entry->data;                  \
                            if(data) {                                 \
                                if(SCAVENGE(data)) {                   \
                                    entry->data = NULL;                \
                                    table.hash_count--;                \
                                }                                      \
                                cnt--;                                 \
                            }                                          \
                        }                                              \
                        if((table.hash_count * 3) >                    \
                                        (table.hash_size * 2))         \
                            new_size = table.hash_size*2;              \
                        else                                           \
                            new_size = table.hash_size;                \
                    } else                                             \
                        new_size = table.hash_size*2;                  \
                                                                       \
                    resizeHash(&table, new_size);                      \
                }                                                      \
            }                                                          \
        }                                                              \
                                                                       \
    if(locked)                                                         \
        unlockHashTable0(&table, self);                                \
}

#define deleteHashEntry(table, ptr, locked)                            \
{                                                                      \
    int hash = HASH(ptr);                                              \
    void *ptr2;                                                        \
    int i;                                                             \
                                                                       \
    Thread *self;                                                      \
    if(locked) {                                                       \
        self = threadSelf();                                           \
        lockHashTable0(&table, self);                                  \
    }                                                                  \
                                                                       \
    i = hash & (table.hash_size - 1);                                  \
                                                                       \
    for(;;) {                                                          \
        ptr2 = table.hash_table[i].data;                               \
        if((ptr2 == NULL) ||                                           \
                 (COMPARE(ptr, ptr2, hash, table.hash_table[i].hash))) \
            break;                                                     \
                                                                       \
        i = (i+1) & (table.hash_size - 1);                             \
    }                                                                  \
                                                                       \
    if(ptr2)                                                           \
        table.hash_table[i].data = DELETED;                            \
                                                                       \
    if(locked)                                                         \
        unlockHashTable0(&table, self);                                \
}

#define hashIterate(table)                                             \
{                                                                      \
    HashEntry *_entry = table.hash_table;                              \
    int _cnt = table.hash_count;                                       \
                                                                       \
    while(_cnt) {                                                      \
        void *_data = _entry++->data;                                  \
        if(_data) {                                                    \
            ITERATE(_data);                                            \
            _cnt--;                                                    \
        }                                                              \
    }                                                                  \
}

#define hashIterateP(table)                                            \
{                                                                      \
    HashEntry *entry = table.hash_table;                               \
    int cnt = table.hash_count;                                        \
                                                                       \
    while(cnt) {                                                       \
        void **data_pntr = &entry++->data;                             \
        if(*data_pntr) {                                               \
            ITERATE(data_pntr);                                        \
            cnt--;                                                     \
        }                                                              \
    }                                                                  \
}

#define freeHashTable(table)                                           \
    gcMemFree(table.hash_table)

#define gcFreeHashTable(table)                                         \
    freeHashTable(table)
