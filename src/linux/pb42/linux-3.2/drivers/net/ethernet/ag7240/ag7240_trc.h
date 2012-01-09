/*
 * Copyright (c) 2008, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _AG7240_TRC_h
#define _AG7240_TRC_h

#define TRC_SIZE    256

typedef struct {
    u32     unit;
    int     line;
    u32     val;
    char    comment[32];

}ag7240_trc_entry_t;

typedef struct {
    int                 cur;
    ag7240_trc_entry_t  entry[TRC_SIZE];
}ag7240_trc_t;

#ifdef CONFIG_AG7240_USE_TRC

#define ag7240_trc_init()  do {                 \
    int i;                                      \
    printk(MODULE_NAME": TRACE ENABLED\n");     \
    mac->tb.cur = 0;                            \
    for(i = 0; i < TRC_SIZE; i++) {             \
        mac->tb.entry[i].line = 0;              \
        mac->tb.entry[i].val = 0x7f;            \
        mac->tb.entry[i].comment[0] = '\0';     \
    }                                           \
}while(0);

#define ag7240_trc_new(_x,_y)     do {               \
    unsigned long flags;                             \
    spin_lock_irqsave(&mac->mac_lock, flags);        \
    mac->tb.entry[mac->tb.cur].unit = mac->mac_unit; \
    mac->tb.entry[mac->tb.cur].line = __LINE__;      \
    mac->tb.entry[mac->tb.cur].val  = (u32)(_x);     \
    if (_y)                                          \
       strncpy(mac->tb.entry[mac->tb.cur].comment, (_y), sizeof(mac->tb.entry[mac->tb.cur].comment)); \
    else                                             \
       mac->tb.entry[mac->tb.cur].comment[0] = '\0'; \
    if (mac->tb.cur == (TRC_SIZE - 1))               \
        mac->tb.cur = 0;                             \
    else                                             \
        mac->tb.cur ++;                              \
    spin_unlock_irqrestore(&mac->mac_lock, flags);   \
}while(0);

#define ag7240_trc ag7240_trc_new

#define ag7240_trc_dump()      do {                         \
    unsigned long flags;                                    \
    spin_lock_irqsave(&mac->mac_lock, flags);               \
    int i, cur = mac->tb.cur;                               \
    printk(MODULE_NAME": head %d tail %d\n", mac->mac_txring.ring_head, mac->mac_txring.ring_tail);\
    for(i = 0; i < TRC_SIZE; i++) {                         \
        if (mac->tb.entry[cur].line) {                      \
            printk("%d %d %08x %s\n",                       \
                mac->tb.entry[cur].unit,                    \
                mac->tb.entry[cur].line,                    \
                mac->tb.entry[cur].val,                     \
                mac->tb.entry[cur].comment);                \
            mac->tb.entry[cur].line = 0;                    \
        }                                                   \
        if (cur == (TRC_SIZE - 1))                          \
            cur = 0;                                        \
        else                                                \
            cur++ ;                                         \
    }                                                       \
    spin_unlock_irqrestore(&mac->mac_lock, flags);          \
}while(0);

#else

#define ag7240_trc_init()
#define ag7240_trc_new(_x,_y) 
#define ag7240_trc(_x, _y)
#define ag7240_trc_dump() 

#endif

#endif

