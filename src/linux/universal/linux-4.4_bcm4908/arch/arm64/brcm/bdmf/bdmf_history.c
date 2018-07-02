/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#include <bdmf_dev.h>
#include <bdmf_session.h>
#include <bdmf_shell.h>

/* Event decoder */
#define HIST_EVENT_NAME_SIZE 32
struct hist_ev
{
    char event[HIST_EVENT_NAME_SIZE];
    int (*decode)(void);
    STAILQ_ENTRY(hist_ev) list;
};
STAILQ_HEAD(hist_ev_list, hist_ev);

typedef enum
{
    histdec_event,      /* Expected event name */
    histdec_parm,       /* Expected event name */
    histdec_rc,         /* Expected rc */
    histdec_out_parm,   /* Expected output parameter */
    histdec_eol,        /* EOL */
    histdec_eof,        /* EOF */
} histdec_state_t;

/* Main hist module's control block */
#define DEC_BUF_LEN     1024
#define DEC_BBUF_LEN    (DEC_BUF_LEN / 2)
struct hist_dev
{
    bdmf_boolean record_on;
    int level;
    /* History buffer */
    uint32_t buf_size;
    uint32_t rec_size;
    char *buffer;
    int overflow;
    /* play-back support */
    struct hist_ev_list ev_list;
    bdmf_file fdec;
    char dec_buf[DEC_BUF_LEN];
    char *pdec;
    char dec_bbuf[DEC_BBUF_LEN];
    char *pbbuf;
    histdec_state_t dec_state;
    int nline;
};
struct hist_dev hist;

#define HIST_MIN_LEFTOVER_SIZE          32

static char *hist_event_name[] = {
    [bdmf_hist_ev_none]              = "*none*",
    [bdmf_hist_ev_new_and_configure] = "bdmf_new_and_configure()",
    [bdmf_hist_ev_new_and_set]       = "bdmf_new_and_set()",
    [bdmf_hist_ev_destroy]           = "bdmf_destroy()",
    [bdmf_hist_ev_link]              = "bdmf_link()",
    [bdmf_hist_ev_unlink]            = "bdmf_unlink()",
    [bdmf_hist_ev_configure]         = "bdmf_configure()",
    [bdmf_hist_ev_mattr_set]         = "bdmf_mattr_set()",
    [bdmf_hist_ev_set_as_num]        = "bdmf_attrelem_set_as_num()",
    [bdmf_hist_ev_set_as_string]     = "bdmf_attrelem_set_as_string()",
    [bdmf_hist_ev_set_as_buf]        = "bdmf_attrelem_set_as_buf()",
    [bdmf_hist_ev_add_as_num]        = "bdmf_attrelem_add_as_num()",
    [bdmf_hist_ev_add_as_string]     = "bdmf_attrelem_add_as_string()",
    [bdmf_hist_ev_add_as_buf]        = "bdmf_attrelem_add_as_buf()",
    [bdmf_hist_ev_delete]            = "bdmf_attrelem_delete()",
};

static const char *_bdmf_hist_ev_name(bdmf_history_event_t ev)
{
    static char *invalid="*invalid*";
    if (ev < sizeof(hist_event_name)/sizeof(char *))
        return hist_event_name[ev];
    return invalid;
}

/*
 * Event recording support
 */

static void _bdmf_hist_overflow(void)
{
    if (!hist.overflow)
    {
        BDMF_TRACE_ERR("BDMF history buffer overflow. Recording stopped.\n");
        hist.overflow = 1;
    }
}

static void _bdmf_hist_vprintf(const char *format, va_list args)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    int size;

    if (hist.overflow || bytes_left < HIST_MIN_LEFTOVER_SIZE) /*check if there is any room for additional writing*/
    {
        _bdmf_hist_overflow();
        return;
    }

    size = vsnprintf(&hist.buffer[hist.rec_size], bytes_left, format, args);
    if (size > bytes_left)
    {
        size = bytes_left;
        _bdmf_hist_overflow();
        return;
    }
    hist.rec_size += size;
}

static void _bdmf_hist_printf(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
static void _bdmf_hist_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    _bdmf_hist_vprintf(format, args);
    va_end(args);
}


#define BDMF_HIST_WRITE_RC(args) \
do { \
    int rc = va_arg(args, int); \
    _bdmf_hist_printf(" # %d\n", rc); \
} while(0)


#define HEX(n) ((n > 9) ? 'A' + n - 9 : '0' + n)

#if 0
static void _bdmf_hist_bin_to_hex(char *data, uint32_t size)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    char *pbuf=&hist.buffer[hist.rec_size];
    char c;
    int hi, lo;

    if (!data)
    {
        if (bytes_left < 2)
        {
            _bdmf_hist_overflow();
            return;
        }
        *(pbuf++) = '-';
        goto done;
    }

    if (size > (bytes_left - 1) / 2)
    {
        _bdmf_hist_overflow();
        if (!bytes_left)
            return;
        size = (bytes_left - 1) / 2;
    }
    while(size--)
    {
        c = *(data++);
        hi = c >> 4;
        lo = c & 0xf;
        *(pbuf++) = HEX(hi);
        *(pbuf++) = HEX(lo);
    }
done:
    *pbuf = 0;
    hist.rec_size += (pbuf - &hist.buffer[hist.rec_size]);
}
#endif

static void _bdmf_hist_encode_index(struct bdmf_object *mo, bdmf_attr_id aid, bdmf_index index)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    char *pbuf=&hist.buffer[hist.rec_size];
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);

    if (hist.overflow || bytes_left < HIST_MIN_LEFTOVER_SIZE) /*check if there is any room for additional writing*/
    {
        _bdmf_hist_overflow();
        return;
    }
    strcpy(pbuf, "-1");
    bdmf_attr_array_index_to_string(mo, attr, index, pbuf, bytes_left);
    hist.rec_size += strlen(pbuf);
}

static void _bdmf_hist_encode_index_ptr(struct bdmf_object *mo, bdmf_attr_id aid, bdmf_index *pindex)
{
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    bdmf_index index;
    if (bdmf_attr_type_is_numeric(attr->index_type))
        index = *pindex;
    else
        index = (bdmf_index)pindex;
    _bdmf_hist_encode_index(mo, aid, index);
}

static void _bdmf_hist_encode_num_or_ref(struct bdmf_object *mo, bdmf_attr_id aid, bdmf_number val)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    char *pbuf=&hist.buffer[hist.rec_size];
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    *pbuf = 0;

    if (hist.overflow || bytes_left < HIST_MIN_LEFTOVER_SIZE) /*check if there is any room for additional writing*/
    {
        _bdmf_hist_overflow();
        return;
    }

    if (attr->type == bdmf_attr_object || attr->type == bdmf_attr_enum || attr->type == bdmf_attr_enum_mask)
    {
        /* Insert space and convert to string value */
        *(pbuf++) = ' '; --bytes_left;
        if (attr->size > sizeof(void *))
            attr->val_to_s(mo, attr, &val, pbuf, bytes_left);
        else
        {
            void *ref_ptr = (void *)((long)val);
            attr->val_to_s(mo, attr, &ref_ptr, pbuf, bytes_left);
        }
        hist.rec_size += strlen(pbuf) + 1;
    }
    else
    {
        _bdmf_hist_printf(" %lld", (long long)val);
    }
}

static void _bdmf_hist_encode_buf(struct bdmf_object *mo, bdmf_attr_id aid, void *buf, uint32_t size)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    char *pbuf=&hist.buffer[hist.rec_size];
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);

    if (hist.overflow || bytes_left < HIST_MIN_LEFTOVER_SIZE) /*check if there is any room for additional writing*/
    {
        _bdmf_hist_overflow();
        return;
    }

    if (!buf)
    {
        _bdmf_hist_printf("-");
        return;
    }
    *pbuf = 0;
    attr->val_to_s(mo, attr, buf, pbuf, bytes_left);
    hist.rec_size += strlen(pbuf);
}

/* start:   bdmf_object
 * end:     none
 */
static void _bdmf_hist_destroy(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        if (!mo->owner)
            return;
        _bdmf_hist_printf("%s %s\n", _bdmf_hist_ev_name(ev), mo->name);
    }
}

/* start:   bdmf_object, bdmf_object
 * end:     rc
 */
static void _bdmf_hist_link_unlink(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo1 = va_arg(args, struct bdmf_object *);
        struct bdmf_object *mo2 = va_arg(args, struct bdmf_object *);
        _bdmf_hist_printf("%s %s %s", _bdmf_hist_ev_name(ev), mo1->name, mo2->name);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   bdmf_object, aid, index, bdmf_number
 * end:     rc [,index]
 */
static void _bdmf_hist_set_as_num(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_attr_id aid = va_arg(args, int);
        struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
        bdmf_index index = va_arg(args, int);
        bdmf_number val = va_arg(args, bdmf_number);
        _bdmf_hist_printf("%s %s %s ", _bdmf_hist_ev_name(ev), mo->name, attr->name);
        _bdmf_hist_encode_index(mo, aid, index);
        _bdmf_hist_encode_num_or_ref(mo, aid, val);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   bdmf_object, aid, index ptr, bdmf_number
 * end:     rc [,index]
 */
static void _bdmf_hist_add_as_num(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
    bdmf_attr_id aid = va_arg(args, int);
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    bdmf_index *pindex = va_arg(args, bdmf_index *);
    bdmf_number val = va_arg(args, bdmf_number);
    int rc = va_arg(args, int);

    BUG_ON((point & bdmf_hist_point_both) != bdmf_hist_point_both);
    _bdmf_hist_printf("%s %s %s ", _bdmf_hist_ev_name(ev), mo->name, attr->name);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_encode_num_or_ref(mo, aid, val);
    pindex = va_arg(args, bdmf_index *);
    _bdmf_hist_printf(" # %d ", rc);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf("\n");
}

/* start:   bdmf_object, aid, index, string
 * end:     rc [,index]
 */
static void _bdmf_hist_set_as_string(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_attr_id aid = va_arg(args, int);
        struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
        bdmf_index index = va_arg(args, int);
        char *val = va_arg(args, char *);
        _bdmf_hist_printf("%s %s %s ", _bdmf_hist_ev_name(ev), mo->name, attr->name);
        _bdmf_hist_encode_index(mo, aid, index);
        _bdmf_hist_printf(" %s", val);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   bdmf_object, aid, index ptr, string
 * end:     rc [,index]
 */
static void _bdmf_hist_add_as_string(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
    bdmf_attr_id aid = va_arg(args, int);
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    bdmf_index *pindex = va_arg(args, bdmf_index *);
    char *val = va_arg(args, char *);
    int rc = va_arg(args, int);

    BUG_ON((point & bdmf_hist_point_both) != bdmf_hist_point_both);

    _bdmf_hist_printf("%s %s %s ", _bdmf_hist_ev_name(ev), mo->name, attr->name);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf(" %s", val);
    pindex = va_arg(args, bdmf_index *);
    _bdmf_hist_printf(" # %d ", rc);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf("\n");
}

/* start:   bdmf_object, aid, index, data
 * end:     rc [,index]
 */
static void _bdmf_hist_set_as_buf(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_attr_id aid = va_arg(args, int);
        struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
        bdmf_index index = va_arg(args, int);
        char *data = va_arg(args, char *);
        uint32_t size = va_arg(args, uint32_t);

        _bdmf_hist_printf("%s %s %s ", _bdmf_hist_ev_name(ev), mo->name, attr->name);
        _bdmf_hist_encode_index(mo, aid, index);
        _bdmf_hist_printf(" ");
        _bdmf_hist_encode_buf(mo, aid, data, size);
    }
    if ((point & bdmf_hist_point_end))
    {
        if (ev == bdmf_hist_ev_set_as_buf)
            BDMF_HIST_WRITE_RC(args);
        else
        {
            int rc = va_arg(args, int);
            bdmf_index index = va_arg(args, bdmf_index);
            _bdmf_hist_printf(" # %d %ld\n", rc, index);
        }

    }
}

/* start:   bdmf_object, aid, index ptr, data
 * end:     rc [,index]
 */
static void _bdmf_hist_add_as_buf(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
    bdmf_attr_id aid = va_arg(args, int);
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    bdmf_index *pindex = va_arg(args, bdmf_index *);
    char *data = va_arg(args, char *);
    uint32_t size = va_arg(args, uint32_t);
    int rc = va_arg(args, int);

    BUG_ON((point & bdmf_hist_point_both) != bdmf_hist_point_both);

    _bdmf_hist_printf("%s %s %s ", _bdmf_hist_ev_name(ev), mo->name, attr->name);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf(" ");
    _bdmf_hist_encode_buf(mo, aid, data, size);
    pindex = va_arg(args, bdmf_index *);
    _bdmf_hist_printf(" # %d ", rc);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf("\n");
}

/* start:   bdmf_object, aid, index
 * end:     rc
 */
static void _bdmf_hist_delete(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_attr_id aid = va_arg(args, int);
        struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
        bdmf_index index = va_arg(args, int);
        _bdmf_hist_printf("%s %s %s ", _bdmf_hist_ev_name(ev), mo->name, attr->name);
        _bdmf_hist_encode_index(mo, aid, index);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   bdmf_object, string
 * end:     rc
 */
static void _bdmf_hist_configure(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        char *str = va_arg(args, char *);
        /* skip temp object used for aggregate attribute/index translation */
        if (! *mo->name)
            return;
        _bdmf_hist_printf("%s %s %s", _bdmf_hist_ev_name(ev), mo->name, str);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   type, parent, string
 * end:     rc
 */
static void _bdmf_hist_new_and_configure(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_type *drv = va_arg(args, struct bdmf_type *);
        struct bdmf_object *owner = va_arg(args, struct bdmf_object *);
        char *str = va_arg(args, char *);
        _bdmf_hist_printf("%s %s %s %s", _bdmf_hist_ev_name(ev),
            drv->name, owner ? owner->name : "*", str);
    }
    if ((point & bdmf_hist_point_end))
    {
        int rc = va_arg(args, int);
        struct bdmf_object *mo = rc ? NULL : va_arg(args, struct bdmf_object *);
        _bdmf_hist_printf(" # %d %s\n", rc, mo ? mo->name : "");
    }
}

static void _bdmf_encode_mattr(struct bdmf_object *mo, bdmf_mattr_t *mattr)
{
    bdmf_mattr_entry_t *entry;
    int i, num_entries;
    
    num_entries = mattr ? mattr->num_entries : 0;
    _bdmf_hist_printf("%d", num_entries);
    for(i=0; i<num_entries; i++)
    {
        struct bdmf_attr *attr;
        entry = &mattr->entries[i];
        attr = bdmf_aid_to_attr(mo->drv, entry->aid);
        _bdmf_hist_printf(" %s ", attr->name);
        _bdmf_hist_encode_index(mo, entry->aid, entry->index);
        /* always encode value as string */
        _bdmf_hist_printf(" %d", bdmf_attr_string);
        switch(entry->val.val_type)
        {
        case bdmf_attr_number: /**< Numeric attribute */
            _bdmf_hist_encode_num_or_ref(mo, entry->aid, entry->val.x.num);
            break;
        case bdmf_attr_string: /**< 0-terminated string */
            _bdmf_hist_printf(" %s", entry->val.x.s);
            break;
        case bdmf_attr_buffer: /**< Buffer with binary data */
            _bdmf_hist_printf(" ");
            _bdmf_hist_encode_buf(mo, entry->aid, entry->val.x.buf.ptr, entry->val.x.buf.len);
            break;
        default:
            _bdmf_hist_printf(" *invalid_type %d", entry->val.val_type);
            break;
        }
    }
}

/* start:   object, mattr
 * end:     rc
 */
static void _bdmf_hist_mattr_set(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_mattr_t *mattr = va_arg(args, bdmf_mattr_t *);
        _bdmf_hist_printf("%s %s ", _bdmf_hist_ev_name(ev), mo->name);
        _bdmf_encode_mattr(mo, mattr);
    }
    if ((point & bdmf_hist_point_end))
    {
        int rc = va_arg(args, int);
        _bdmf_hist_printf(" # %d\n", rc);
    }
}

/* start:   type, parent, mattr
 * end:     rc, object
 */
static void _bdmf_hist_new_and_set(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    static struct bdmf_object mo_tmp = { .name="*new*" };

    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_type *drv = va_arg(args, struct bdmf_type *);
        struct bdmf_object *owner = va_arg(args, struct bdmf_object *);
        bdmf_mattr_t *mattr = va_arg(args, bdmf_mattr_t *);
        mo_tmp.drv = drv;
        _bdmf_hist_printf("%s %s %s ", _bdmf_hist_ev_name(ev),
            mo_tmp.drv->name, owner ? owner->name : "*");
        _bdmf_encode_mattr(&mo_tmp, mattr);
    }
    if ((point & bdmf_hist_point_end))
    {
        int rc = va_arg(args, int);
        struct bdmf_object *mo = rc ? NULL : va_arg(args, struct bdmf_object *);
        _bdmf_hist_printf(" # %d %s\n", rc, mo ? mo->name : "");
    }
}

/* Sometimes bdmf API can call other APIs.
 * Record only the outer call
 */
#define BDMF_HISTORY_HANDLE_POINT(point) \
    do { \
        if ((point & bdmf_hist_point_start)) \
        { \
            ++hist.level; \
            if (hist.level > 1) \
            { \
                if ((point & bdmf_hist_point_end)) \
                    --hist.level; \
                return; \
            } \
        } \
        if ((point & bdmf_hist_point_end)) \
        { \
            BUG_ON(hist.level <= 0); \
            --hist.level; \
            if (hist.level) \
                return; \
        } \
    } while(0)

/* Record built-in history event */
void bdmf_history_bi_event(bdmf_history_event_t ev, bdmf_history_point_t point, ...)
{
    va_list args;

    BDMF_HISTORY_HANDLE_POINT(point);

    if (!hist.record_on)
        return;

    va_start(args, point);
    switch(ev)
    {
    case bdmf_hist_ev_set_as_num:       /**< bdmf_attrelem_set_as_num() */
        _bdmf_hist_set_as_num(ev, point, args);
        break;

    case bdmf_hist_ev_add_as_num:       /**< bdmf_attrelem_add_as_num() */
        _bdmf_hist_add_as_num(ev, point, args);
        break;

    case bdmf_hist_ev_set_as_buf:       /**< bdmf_attrelem_set_as_buf() */
        _bdmf_hist_set_as_buf(ev, point, args);
        break;

    case bdmf_hist_ev_add_as_buf:       /**< bdmf_attrelem_add_as_buf() */
        _bdmf_hist_add_as_buf(ev, point, args);
        break;

    case bdmf_hist_ev_mattr_set:        /**< bdmf_mattr_set() */
        _bdmf_hist_mattr_set(ev, point, args);
        break;

    case bdmf_hist_ev_set_as_string:    /**< bdmf_attrelem_set_as_string() */
        _bdmf_hist_set_as_string(ev, point, args);
        break;

    case bdmf_hist_ev_add_as_string:    /**< bdmf_attrelem_add_as_string() */
        _bdmf_hist_add_as_string(ev, point, args);
        break;

    case bdmf_hist_ev_delete:           /**< bdmf_attrelem_delete() */
        _bdmf_hist_delete(ev, point, args);
        break;

    case bdmf_hist_ev_new_and_configure: /**< bdmf_new_and_configure() */
        _bdmf_hist_new_and_configure(ev, point, args);
        break;

    case bdmf_hist_ev_new_and_set:       /**< bdmf_new_and_set() */
        _bdmf_hist_new_and_set(ev, point, args);
        break;

    case bdmf_hist_ev_destroy:           /**< bdmf_destroy() */
        _bdmf_hist_destroy(ev, point, args);
        break;

    case bdmf_hist_ev_link:              /**< bdmf_link() */
    case bdmf_hist_ev_unlink:            /**< bdmf_unlink() */
        _bdmf_hist_link_unlink(ev, point, args);
        break;

    case bdmf_hist_ev_configure:         /**< bdmf_configure() */
        _bdmf_hist_configure(ev, point, args);
        break;

    default:
        BDMF_TRACE_ERR("history: unknown event %d\n", ev);
        BUG();
    }
    va_end(args);
}

/* Record custom event
 *
 * History event is represented by a string terminated by "\n" \n
 * event_name event_parameters_separated_by_a_single_space # rc optional_results\\n \n
 *
 * \param[in]   event   Event name - must match event in bdmf_history_event_register()
 * \param[in]   point   Recording point: start of event, end of event or both
 * \param[in]   format  printf-like format
 */
void bdmf_history_event(const char *event, bdmf_history_point_t point, const char *format, ...)
{
    va_list args;

    BDMF_HISTORY_HANDLE_POINT(point);

    if (!hist.record_on)
        return;

    if ((point & bdmf_hist_point_start))
        _bdmf_hist_printf("%s ", event);
    va_start(args, format);
    _bdmf_hist_vprintf(format, args);
    va_end(args);

    return;
}

/*
 * Event decoding (re-playing) support
 */

#define HIST_PB_ERROR(format, args...) \
    BDMF_TRACE_ERR("BDMF history pb error in line %d. " format, hist.nline, ## args);

static struct hist_ev *_bdmf_hist_ev_get(const char *event)
{
    struct hist_ev *e;
    STAILQ_FOREACH(e, &hist.ev_list, list)
    {
        if (!strcmp(e->event, event))
            return e;
    }
    return NULL;
}

/** Get numeric argument
 * \param[out]  n       Numeric argument
 * \return 0=OK or error code <0
 */
int bdmf_history_decode_num(bdmf_number *n)
{
    char *num_buf;
    int rc;
    rc = bdmf_history_decode_string(&num_buf);
    if (rc)
        return rc;
    if (!sscanf(num_buf, "%lld", (long long *)n))
    {
        BDMF_TRACE_ERR("history: can't decode number <%s>\n", num_buf);
        return BDMF_ERR_PARSE;
    }
    return 0;
}

/* Get string argument */
static int _bdmf_history_decode_string(char **s)
{
    char c=0;
    int quoted_string=0;
    int rc;
    int size = sizeof(hist.dec_buf) - (hist.pdec - hist.dec_buf);

    *s = hist.pdec;
    if (!hist.fdec)
        return BDMF_ERR_STATE;
    if (hist.dec_state == histdec_eol)
        return BDMF_ERR_PARSE;
    rc = bdmf_file_read(hist.fdec, &c, 1);

    /* Skip leading spaces */
    while (rc > 0 && c == ' ')
        rc = bdmf_file_read(hist.fdec, &c, 1);

    if (c == '"')
    {
        quoted_string = 1;
        rc = bdmf_file_read(hist.fdec, &c, 1);
    }
    while((size > 1) && (rc > 0) &&
        !((c == '\n') || !c || (quoted_string && c == '"') || (!quoted_string && c == ' ')) )
    {
        *(hist.pdec++) = c;
        --size;
        rc = bdmf_file_read(hist.fdec, &c, 1);
    }
    *(hist.pdec++) = 0;
    if (size <= 1)
        return BDMF_ERR_OVERFLOW;

    if (c == '\n')
        hist.dec_state = histdec_eol;
    else if (!rc)
        hist.dec_state = histdec_eof;

    if (rc < 0)
        return BDMF_ERR_IO;

    if (quoted_string && c != '"')
        return BDMF_ERR_PARSE;

    return 0;
}

/* Get string argument */
int bdmf_history_decode_string(char **s)
{
    static char *null_str="";
    if ((hist.dec_state != histdec_parm) && (hist.dec_state != histdec_out_parm))
    {
        *s = null_str;
        return BDMF_ERR_PARSE;
    }
    return _bdmf_history_decode_string(s);
}

/* Get string argument up to '#' terminator */
static int _bdmf_history_decode_string_up_to_rc(char **s)
{
    char c=0;
    int rc;
    int size = sizeof(hist.dec_buf) - (hist.pdec - hist.dec_buf);
    static char *null_str="";
    if ((hist.dec_state != histdec_parm) && (hist.dec_state != histdec_out_parm))
    {
        *s = null_str;
        return BDMF_ERR_PARSE;
    }

    *s = hist.pdec;
    if (!hist.fdec)
        return BDMF_ERR_STATE;
    if (hist.dec_state == histdec_eol)
        return BDMF_ERR_PARSE;
    rc = bdmf_file_read(hist.fdec, &c, 1);
    while((size > 1) && (rc > 0) && !((c == '\n') || (c == '#')) )
    {
        /* Ignore spaces */
        if (c != ' ')
        {
            *(hist.pdec++) = c;
            --size;
        }
        rc = bdmf_file_read(hist.fdec, &c, 1);
    }
    *(hist.pdec++) = 0;
    if (size <= 1)
        return BDMF_ERR_OVERFLOW;

    if (c == '\n')
        hist.dec_state = histdec_eol;
    else if (!rc)
        hist.dec_state = histdec_eof;

    if (rc < 0)
        return BDMF_ERR_IO;

    if (c != '#')
        return BDMF_ERR_PARSE;

    return 0;

}

/** Get binary argument */
int bdmf_history_decode_buf(void **buf, uint32_t *size)
{
    char *hbuf;
    int bb_size = sizeof(hist.dec_bbuf) - (hist.pbbuf - hist.dec_bbuf);
    int rc;
    rc = bdmf_history_decode_string(&hbuf);
    if (rc < 0)
        return rc;

    /* Special handling for NULL passed as buffer */
    if (*hbuf == '-')
    {
        *buf = NULL;
        *size = 0;
        return rc;
    }

    rc = bdmf_strhex(hbuf, (uint8_t *)hist.pbbuf, bb_size);
    if (rc < 0)
        return rc;
    *buf = hist.pbbuf;
    *size = rc;
    hist.pbbuf += rc;
    return 0;
}

/** Get attribute index argument - from string format */
static int _bdmf_history_decode_index(char *sbuf, struct bdmf_object *mo, bdmf_attr_id aid, bdmf_index *pindex)
{
    struct bdmf_attr *attr = &mo->drv->aattr[aid];
    int bb_size = sizeof(hist.dec_bbuf) - (hist.pbbuf - hist.dec_bbuf);
    int rc;

    if ((unsigned)aid >= mo->drv->nattrs)
        return BDMF_ERR_PARM;

    /* Special handling for NULL passed as buffer */
    if (*sbuf == '-' || !strcmp(sbuf, "null"))
    {
        *pindex = BDMF_INDEX_UNASSIGNED;
        return 0;
    }
    if (bb_size < attr->index_size)
        return BDMF_ERR_OVERFLOW;
    rc = bdmf_attr_string_to_array_index(mo, attr, sbuf, hist.pbbuf);
    if (rc < 0)
        return rc;
    if (bdmf_attr_type_is_numeric(attr->index_type))
        *pindex = *(bdmf_index *)hist.pbbuf;
    else
        *pindex = (bdmf_index)hist.pbbuf;
    hist.pbbuf += attr->index_size;
    return 0;
}

/** Decode return code following '#' character. The '#' itself has already been read */
static int _bdmf_history_decode_rc_value(int *res)
{
    char *rc_str;
    char *pend;
    int rc;

    rc = _bdmf_history_decode_string(&rc_str);
    if (rc)
        return rc;

    if (hist.dec_state != histdec_eol)
        hist.dec_state = histdec_out_parm;
    *res = strtol(rc_str, &pend, 10);
    if (pend && *pend)
    {
        HIST_PB_ERROR("Expected numerical res, got %s\n", rc_str);
        return BDMF_ERR_PARSE;
    }
    return 0;

}

/** Decode return code */
int bdmf_history_decode_rc(int *res)
{
    char *out_str;
    int rc;
    rc = _bdmf_history_decode_string(&out_str);
    if (rc)
        return rc;
    if (strcmp(out_str, "#"))
    {
        HIST_PB_ERROR("Expected '#' got %s\n", out_str);
        return BDMF_ERR_PARSE;
    }
    return _bdmf_history_decode_rc_value(res);
}

/** Decode mattr
 * start:   n * { aid, index, type, value }
 */
static int _bdmf_hist_decode_mattr(struct bdmf_object *mo, bdmf_mattr_handle mattr)
{
    bdmf_number nattrs;
    int i;
    int rc = 0;

    rc = bdmf_history_decode_num(&nattrs);
    if (rc)
        return rc;
    if ((unsigned)nattrs > ((bdmf_mattr_t *)mattr)->max_entries)
    {
        HIST_PB_ERROR("Error when parsing mattr. nattrs %d is insane\n", (int)nattrs);
        return BDMF_ERR_PARSE;
    }

    for(i=0; i<(int)nattrs; i++)
    {
        char *attr_name;
        bdmf_attr_id aid;
        bdmf_number type;
        bdmf_index index;
        char *index_buf;

        rc = bdmf_history_decode_string(&attr_name);
        rc = rc ? rc : bdmf_attr_by_name(mo->drv, attr_name, &aid);
        rc = rc ? rc : bdmf_history_decode_string(&index_buf);
        rc = rc ? rc : _bdmf_history_decode_index(index_buf, mo, aid, &index);
        rc = rc ? rc : bdmf_history_decode_num(&type);
        if (rc)
            break;
        switch((bdmf_attr_type_t)type)
        {
        case bdmf_attr_number:
        {
            bdmf_number nval;
            rc = bdmf_history_decode_num(&nval);
            rc = rc ? rc : bdmf_attrelem_set_as_num(mattr, (bdmf_attr_id)aid, index, nval);
        }
        break;

        case bdmf_attr_string:
        {
            char *sval;
            rc = bdmf_history_decode_string(&sval);
            rc = rc ? rc : bdmf_attrelem_set_as_string(mattr, (bdmf_attr_id)aid, index, sval);
        }
        break;

        case bdmf_attr_buffer:
        {
            void *bval;
            uint32_t bsize = 0;
            rc = bdmf_history_decode_buf(&bval, &bsize);
            rc = rc ? rc : bdmf_attrelem_set_as_buf(mattr, (bdmf_attr_id)aid, index, bval, bsize);
        }
        break;

        default:
            rc = BDMF_ERR_PARSE;
            break;
        }

        if (rc)
        {
            HIST_PB_ERROR("Error when parsing mattr attribute #%d\n", i);
            break;
        }
    }
    return rc;
}

/**
 * start:   type, parent, string
 * end:     rc
 */
static int _bdmf_hist_decode_new_and_configure(void)
{
    int rc;
    char *type;
    char *parent;
    char *attrs;
    char *mo_name = "";
    struct bdmf_object *owner = NULL;
    struct bdmf_type *drv = NULL;
    struct bdmf_object *mo = NULL;
    int res;

    rc = bdmf_history_decode_string(&type);
    rc = rc ? rc : bdmf_history_decode_string(&parent);
    rc = rc ? rc : _bdmf_history_decode_string_up_to_rc(&attrs);
    rc = rc ? rc : _bdmf_history_decode_rc_value(&res);
    rc = (rc || res) ? rc : bdmf_history_decode_string(&mo_name);
    if (rc)
        return rc;

    rc = bdmf_type_find_get(type, &drv);
    if (strcmp(parent, "*"))
        rc = rc ? rc : bdmf_find_get_by_name(parent, &owner);
    if (rc)
        rc = BDMF_ERR_HIST_RES_MISMATCH;
    else
        rc = bdmf_new_and_configure(drv, owner, attrs, &mo);

    /* Cleanup */
    if (owner)
        bdmf_put(owner);
    if (drv)
        bdmf_type_put(drv);

    if ((rc != res) || (mo && strcmp(mo->name, mo_name)))
        return BDMF_ERR_HIST_RES_MISMATCH;
    return 0;
}

static int _bdmf_hist_decode_new_and_set(void)
{
    static struct bdmf_object mo_tmp = { .name="*new*" };
    int rc;
    char *type;
    char *parent;
    char *mo_name = "";
    struct bdmf_object *owner = NULL;
    struct bdmf_type *drv = NULL;
    struct bdmf_object *mo = NULL;
    int actual_res=0, res;

    rc = bdmf_history_decode_string(&type);
    rc = rc ? rc : bdmf_history_decode_string(&parent);
    if (rc)
        return rc;

    /* Find stuff. From this point on we must cleanup */
    rc = bdmf_type_find_get(type, &drv);
    if (strcmp(parent, "*"))
        rc = rc ? rc : bdmf_find_get_by_name(parent, &owner);
    if (rc)
    {
        rc = BDMF_ERR_HIST_RES_MISMATCH;
        goto exit;
    }

    /* Now parse mattr and create object */
    {
        BDMF_MATTR(my_mattr, drv);
        mo_tmp.drv = drv;
        rc = _bdmf_hist_decode_mattr(&mo_tmp, my_mattr);
        rc = rc ? rc : bdmf_history_decode_rc(&res);
        rc = (rc || res) ? rc : bdmf_history_decode_string(&mo_name);
        if (rc)
            goto exit;
        actual_res = bdmf_new_and_set(drv, owner, my_mattr, &mo);
    }

    /* Cleanup */
exit:
    if (owner)
        bdmf_put(owner);
    if (drv)
        bdmf_type_put(drv);
    if (rc)
        return rc;
    if ((actual_res != res) || (mo && strcmp(mo->name, mo_name)))
        return BDMF_ERR_HIST_RES_MISMATCH;
    return 0;
}

static int _bdmf_hist_decode_destroy(void)
{
    int rc;
    char *objstr;
    struct bdmf_object *mo = NULL;

    rc = bdmf_history_decode_string(&objstr);
    if (rc)
        return rc;
    rc = bdmf_find_get_by_name(objstr, &mo);
    if (rc)
        return BDMF_ERR_HIST_RES_MISMATCH;
    bdmf_destroy(mo);
    bdmf_put(mo);
    return 0;
}

static int _bdmf_hist_decode_link(void)
{
    int rc;
    char *objstr1, *objstr2;
    struct bdmf_object *mo1=NULL, *mo2=NULL;
    int res;

    rc = bdmf_history_decode_string(&objstr1);
    rc = rc ? rc : bdmf_history_decode_string(&objstr2);
    rc = rc ? rc : bdmf_history_decode_rc(&res);
    if (rc)
        return rc;
    rc = bdmf_find_get_by_name(objstr1, &mo1);
    rc = rc ? rc : bdmf_find_get_by_name(objstr2, &mo2);
    if (rc)
        rc = BDMF_ERR_HIST_RES_MISMATCH;
    else
        rc = bdmf_link(mo1, mo2, NULL);

    /* Cleanup */
    if (mo1)
        bdmf_put(mo1);
    if (mo2)
        bdmf_put(mo2);
    if (rc != res)
        return BDMF_ERR_HIST_RES_MISMATCH;
    return 0;
}

static int _bdmf_hist_decode_unlink(void)
{
    int rc;
    char *objstr1, *objstr2;
    struct bdmf_object *mo1=NULL, *mo2=NULL;
    int res;

    rc = bdmf_history_decode_string(&objstr1);
    rc = rc ? rc : bdmf_history_decode_string(&objstr2);
    rc = rc ? rc : bdmf_history_decode_rc(&res);
    if (rc)
        return rc;
    rc = bdmf_find_get_by_name(objstr1, &mo1);
    rc = rc ? rc : bdmf_find_get_by_name(objstr2, &mo2);
    if (rc)
        rc = BDMF_ERR_HIST_RES_MISMATCH;
    else
        rc = bdmf_unlink(mo1, mo2);

    /* Cleanup */
    if (mo1)
        bdmf_put(mo1);
    if (mo2)
        bdmf_put(mo2);
    if (rc != res)
        return BDMF_ERR_HIST_RES_MISMATCH;
    return 0;
}

static int _bdmf_hist_decode_configure(void)
{
    int rc;
    char *objstr;
    struct bdmf_object *mo=NULL;
    char *attrs;
    int res;

    rc = bdmf_history_decode_string(&objstr);
    rc = rc ? rc : _bdmf_history_decode_string_up_to_rc(&attrs);
    rc = rc ? rc : _bdmf_history_decode_rc_value(&res);
    if (rc)
        return rc;
    rc = bdmf_find_get_by_name(objstr, &mo);
    if (rc)
        rc = BDMF_ERR_HIST_RES_MISMATCH;
    else
        rc = bdmf_configure(mo, attrs);

    /* Cleanup */
    if (mo)
        bdmf_put(mo);
    if (rc != res)
        return BDMF_ERR_HIST_RES_MISMATCH;
    return 0;
}

static int _bdmf_hist_decode_mattr_set(void)
{
    int rc;
    char *objstr;
    struct bdmf_object *mo=NULL;
    int actual_res=0, res;

    rc = bdmf_history_decode_string(&objstr);
    if (rc)
        return rc;
    rc = bdmf_find_get_by_name(objstr, &mo);
    if (rc)
        return BDMF_ERR_HIST_RES_MISMATCH;

    /* From this point on we must do cleanup */

    /* Now parse mattr and create object */
    {
        BDMF_MATTR(my_mattr, mo->drv);
        rc = _bdmf_hist_decode_mattr(mo, my_mattr);
        rc = rc ? rc : bdmf_history_decode_rc(&res);
        if (rc)
            goto exit;
        actual_res = bdmf_mattr_set(mo, my_mattr);
    }

    /* Cleanup */
exit:
    if (mo)
        bdmf_put(mo);
    if (rc)
        return rc;
    if (actual_res != res)
        return BDMF_ERR_HIST_RES_MISMATCH;
    return 0;
}

static int _bdmf_hist_decode_set_add_as_xx(bdmf_attr_type_t as_what, int is_add)
{
    int rc;
    char *objstr;
    struct bdmf_object *mo=NULL;
    char *attr_name;
    bdmf_attr_id aid;
    bdmf_number nval=0;
    char *index_buf, *res_index_buf=NULL;
    bdmf_index index, res_index = 0;
    bdmf_index *pindex;

    char res_index_actual[128] = "";
    char *sval=NULL;
    void *bval=NULL;
    uint32_t bsize = 0;
    int res;
    struct bdmf_attr *attr;

    rc = bdmf_history_decode_string(&objstr);
    rc = rc ? rc : bdmf_history_decode_string(&attr_name);
    rc = rc ? rc : bdmf_history_decode_string(&index_buf);
    if (as_what == bdmf_attr_number)
        rc = rc ? rc : bdmf_history_decode_num(&nval);
    else if (as_what == bdmf_attr_string)
        rc = rc ? rc : bdmf_history_decode_string(&sval);
    else
        rc = rc ? rc : bdmf_history_decode_buf(&bval, &bsize);
    rc = rc ? rc : bdmf_history_decode_rc(&res);
    if (is_add)
        rc = rc ? rc : bdmf_history_decode_string(&res_index_buf);

    /* Done reading. Consult data model */
    rc = rc ? rc : bdmf_find_get_by_name(objstr, &mo);
    rc = rc ? rc : bdmf_attr_by_name(mo->drv, attr_name, &aid);
    rc = rc ? rc : _bdmf_history_decode_index(index_buf, mo, aid, &index);
    if (is_add)
        rc = rc ? rc : _bdmf_history_decode_index(res_index_buf, mo, aid, &res_index);
    else
        res_index = index;

    if (!rc)
    {
        attr = bdmf_aid_to_attr(mo->drv, aid);
        pindex = bdmf_attr_type_is_numeric(attr->index_type) ? &index : (bdmf_index *)index;
        if (as_what == bdmf_attr_number)
        {
            if (is_add)
                rc = bdmf_attrelem_add_as_num(mo, (bdmf_attr_id)aid, pindex, nval);
            else
                rc = bdmf_attrelem_set_as_num(mo, (bdmf_attr_id)aid, index, nval);
        }
        else if (as_what == bdmf_attr_string)
        {
            struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
            if (attr->type == bdmf_attr_object && sval && *sval=='{')
            {
                int sval_len = strlen(sval);
                if (sval_len)
                    sval[sval_len-1] = 0;
                ++sval;
            }
            if (is_add)
                rc = bdmf_attrelem_add_as_string(mo, (bdmf_attr_id)aid, pindex, sval);
            else
                rc = bdmf_attrelem_set_as_string(mo, (bdmf_attr_id)aid, index, sval);
        }
        else
        {
            if (is_add)
                rc = bdmf_attrelem_add_as_buf(mo, (bdmf_attr_id)aid, pindex, bval, bsize);
            else
                rc = bdmf_attrelem_set_as_buf(mo, (bdmf_attr_id)aid, index, bval, bsize);
        }
    }
    if (!rc)
    {
        bdmf_attr_array_index_to_string(mo, attr, index, res_index_actual, sizeof(res_index_actual));
    }

    /* Cleanup */
    if (mo)
        bdmf_put(mo);
    if ((rc != res) || (!rc && res_index_buf && strcmp(res_index_buf, res_index_actual)))
        return BDMF_ERR_HIST_RES_MISMATCH;
    return 0;
}

static int _bdmf_hist_decode_set_as_num(void)
{
    return _bdmf_hist_decode_set_add_as_xx(bdmf_attr_string, 0);
}

static int _bdmf_hist_decode_set_as_string(void)
{
    return _bdmf_hist_decode_set_add_as_xx(bdmf_attr_string, 0);
}

static int _bdmf_hist_decode_set_as_buf(void)
{
    return _bdmf_hist_decode_set_add_as_xx(bdmf_attr_string, 0);
}

static int _bdmf_hist_decode_add_as_num(void)
{
    return _bdmf_hist_decode_set_add_as_xx(bdmf_attr_string, 1);
}

static int _bdmf_hist_decode_add_as_string(void)
{
    return _bdmf_hist_decode_set_add_as_xx(bdmf_attr_string, 1);
}

static int _bdmf_hist_decode_add_as_buf(void)
{
    return _bdmf_hist_decode_set_add_as_xx(bdmf_attr_string, 1);
}

static int _bdmf_hist_decode_delete(void)
{
    int rc;
    char *objstr;
    struct bdmf_object *mo=NULL;
    char *index_buf = NULL, *attr_name = NULL;
    bdmf_attr_id aid;
    bdmf_index index;
    int res;

    rc = bdmf_history_decode_string(&objstr);
    rc = rc ? rc : bdmf_history_decode_string(&attr_name);
    rc = rc ? rc : bdmf_history_decode_string(&index_buf);
    rc = rc ? rc : bdmf_history_decode_rc(&res);
    if (rc)
        return rc;
    rc = bdmf_find_get_by_name(objstr, &mo);
    if (rc)
        return BDMF_ERR_HIST_RES_MISMATCH;
    rc = rc ? rc : bdmf_attr_by_name(mo->drv, attr_name, &aid);
    rc = rc ? rc : _bdmf_history_decode_index(index_buf, mo, aid, &index);
    rc = bdmf_attrelem_delete(mo, (bdmf_attr_id)aid, index);
    /* Cleanup */
    if (mo)
        bdmf_put(mo);
    if (rc != res)
        return BDMF_ERR_HIST_RES_MISMATCH;
    return 0;
}

/*
 * API
 */

/** Init history module
 * \return 0=OK or error code <0
 */
int bdmf_history_module_init(void)
{
    int rc;
    if (hist.ev_list.stqh_last)
        return BDMF_ERR_ALREADY;
    STAILQ_INIT(&hist.ev_list);
    rc = bdmf_history_event_register(hist_event_name[bdmf_hist_ev_new_and_configure],
        _bdmf_hist_decode_new_and_configure);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_new_and_set],
        _bdmf_hist_decode_new_and_set);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_destroy],
        _bdmf_hist_decode_destroy);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_link],
        _bdmf_hist_decode_link);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_unlink],
        _bdmf_hist_decode_unlink);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_configure],
        _bdmf_hist_decode_configure);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_mattr_set],
        _bdmf_hist_decode_mattr_set);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_set_as_num],
        _bdmf_hist_decode_set_as_num);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_set_as_string],
        _bdmf_hist_decode_set_as_string);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_set_as_buf],
        _bdmf_hist_decode_set_as_buf);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_add_as_num],
        _bdmf_hist_decode_add_as_num);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_add_as_string],
        _bdmf_hist_decode_add_as_string);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_add_as_buf],
        _bdmf_hist_decode_add_as_buf);
    rc = rc ? rc : bdmf_history_event_register(hist_event_name[bdmf_hist_ev_delete],
        _bdmf_hist_decode_delete);
    return rc;
}

/** Exit history module
 */
void bdmf_history_module_exit(void)
{
    struct hist_ev *e, *e_tmp;
    bdmf_history_free();
    STAILQ_FOREACH_SAFE(e, &hist.ev_list, list, e_tmp)
    {
        STAILQ_REMOVE_HEAD(&hist.ev_list, list);
        bdmf_free(e);
    }
}

/** Init and start history recording */
int bdmf_history_init(uint32_t size, bdmf_boolean record_on)
{
    if (hist.buffer)
        return BDMF_ERR_ALREADY;
    /* Allocate history structure and buffer */
    hist.buffer = bdmf_calloc(size);
    if (!hist.buffer)
        return BDMF_ERR_NOMEM;
    hist.record_on = record_on;
    hist.buf_size = size;
    return 0;
}

/* Stop history recording */
void bdmf_history_stop(void)
{
    hist.record_on = 0;
    BDMF_TRACE_INFO("history: recording stopped\n");
}

/** Resume history recording */
int bdmf_history_resume(void)
{
    if (!hist.buffer)
        return BDMF_ERR_NOT_SUPPORTED;
    hist.record_on = 1;
    return 0;
}

/** Get history buffer
 * \param[out]  buffer      History buffer pointer
 * \param[out]  size        History buffer size
 * \param[out]  rec_size    Recorded history size
 * \return 0=OK or error code <0
 */
int bdmf_history_get(void **buffer, uint32_t *size, uint32_t *rec_size)
{
    if (!buffer || !size || !rec_size)
        return BDMF_ERR_PARM;
    *buffer = hist.buffer;
    *size = hist.buf_size;
    *rec_size = hist.rec_size;
    return 0;
}

/** Reset history buffer.
 * All recorded history is discarded
 * \return 0=OK or error code <0
 */
void bdmf_history_reset(void)
{
    hist.rec_size = 0;
}

/** Release history buffer.
 * All recorded history is discarded
 * \return 0=OK or error code <0
 */
void bdmf_history_free(void)
{
    if (hist.buffer)
        bdmf_free(hist.buffer);
    memset(&hist, 0, offsetof(struct hist_dev, ev_list));
}

/** Save history buffer
 * \param[in]   fname   History file name
 * \return 0=OK or error code <0
 */
int bdmf_history_save(const char *fname)
{
    uint32_t fmode = BDMF_FMODE_WRONLY | BDMF_FMODE_CREATE | BDMF_FMODE_TRUNCATE;
    bdmf_file f;
    int rc;
    if (!hist.buffer)
        return BDMF_ERR_NOT_SUPPORTED;
    if (!fname)
        return BDMF_ERR_PARM;
    f = bdmf_file_open(fname, fmode);
    if (!f)
    {
        BDMF_TRACE_ERR("history: can't open history file %s for writing\n", fname);
        return BDMF_ERR_IO;
    }
    rc = bdmf_file_write(f, hist.buffer, hist.rec_size);
    bdmf_file_close(f);
    return (rc==hist.rec_size) ? 0 : BDMF_ERR_IO;
}

/** Play-back history file */
int bdmf_history_play(const char *fname, bdmf_boolean stop_on_mismatch)
{
    uint32_t fmode = BDMF_FMODE_RDONLY;
    int rc;
    if (!fname)
        return BDMF_ERR_PARM;
    if (hist.fdec)
        return BDMF_ERR_STATE;
    hist.fdec = bdmf_file_open(fname, fmode);
    if (!hist.fdec)
    {
        BDMF_TRACE_ERR("history: can't open history file %s for reading\n", fname);
        return BDMF_ERR_IO;
    }
    hist.nline = 0;
    do
    {
        char *event = "";
        struct hist_ev *e;
        hist.pdec = hist.dec_buf;
        hist.dec_state = histdec_event;
        hist.pbbuf = hist.dec_bbuf;
        *hist.pdec = 0;
        ++hist.nline;
        rc = _bdmf_history_decode_string(&event);
        if (rc || ! *event)
            break;
        e = _bdmf_hist_ev_get(event);
        if (!e)
        {
            HIST_PB_ERROR("Can't parse event %s\n", event);
            rc = BDMF_ERR_PARSE;
        }
        hist.dec_state = histdec_parm;
        rc = rc ? rc : e->decode();
        if ((rc == BDMF_ERR_HIST_RES_MISMATCH) && !stop_on_mismatch)
        {
            HIST_PB_ERROR("Result mismatch when processing event %s. Ignored\n", event);
            rc = 0;
        }
        if (rc)
        {
            HIST_PB_ERROR("Error \"%s\"\n", bdmf_strerror(rc));
            break;
        }
        /* skip to EOL or EOF */
        while((hist.dec_state != histdec_eol) && (hist.dec_state != histdec_eof))
        {
            char *tmp;
            int tmp_rc;
            tmp_rc = _bdmf_history_decode_string(&tmp);
            if (tmp_rc)
                break;
        }
    } while(hist.dec_state != histdec_eof);

    bdmf_file_close(hist.fdec);

    return rc;
}

/** Register event decoding callback */
int bdmf_history_event_register(const char *event, int (*decode)(void))
{
    struct hist_ev *e;
    e = _bdmf_hist_ev_get(event);
    if (e)
        return BDMF_ERR_ALREADY;
    e = bdmf_alloc(sizeof(*e));
    if (!e)
        return BDMF_ERR_NOMEM;
    strncpy(e->event, event, sizeof(e->event));
    e->decode = decode;
    STAILQ_INSERT_TAIL(&hist.ev_list, e, list);
    return 0;
}

/** Unregister event decoding callback */
void bdmf_history_event_unregister(const char *event)
{
    struct hist_ev *e;
    e = _bdmf_hist_ev_get(event);
    if (!e)
        return;
    STAILQ_REMOVE(&hist.ev_list, e, hist_ev, list);
    bdmf_free(e);
}


#ifdef BDMF_SHELL
/*
 * CLI
 */

/* Init and start history recording
    BDMFMON_MAKE_PARM("size", "History buffer size", BDMFMON_PARM_NUMBER_, 0),
    BDMFMON_MAKE_PARM_ENUM("record_on", "Record mode", bool_table, "on"),
*/
static int bdmf_mon_hist_init(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    uint32_t size = (uint32_t)parm[0].value.number;
    bdmf_boolean record_on = (bdmf_boolean)parm[1].value.number;
    int rc;
    rc = bdmf_history_init(size, record_on);
    return rc;
}

/* Stop history recording
*/
static int bdmf_mon_hist_stop(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    bdmf_history_stop();
    return 0;
}

/* Resume history recording
*/
static int bdmf_mon_hist_resume(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    int rc;
    rc = bdmf_history_resume();
    return rc;
}

/* Save history to file
    BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
*/
static int bdmf_mon_hist_save(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    char *fname = (char *)parm[0].value.string;
    return bdmf_history_save(fname);
}

/* Playback history recording
    BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM_ENUM_DEFVAL("stop", "Stop on result mismatch", yes_no_table, 0, "yes"),
*/
static int bdmf_mon_hist_play(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    char *fname = (char *)parm[0].value.string;
    bdmf_boolean stop_on_mismatch = (bdmf_boolean)parm[1].value.number;
    return bdmf_history_play(fname, stop_on_mismatch);
}

bdmfmon_handle_t bdmf_hist_mon_init(void)
{
    bdmfmon_handle_t bdmf_dir;
    bdmfmon_handle_t hist_dir;

    bdmf_dir=bdmfmon_dir_find(NULL, "bdmf");
    hist_dir = bdmfmon_dir_add(bdmf_dir, "history",
                             "History Recording and Playback",
                             BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_enum_val_t on_off_table[] = {
            { .name="off",   .val=0},
            { .name="on",    .val=1},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("size", "History buffer size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("record_on", "Record mode", on_off_table, 0, "on"),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(hist_dir, "init", bdmf_mon_hist_init,
                      "Init and start history recording",
                      BDMF_ACCESS_ADMIN, NULL, parms);
    }
    {
        bdmfmon_cmd_add(hist_dir, "stop", bdmf_mon_hist_stop,
                      "Stop history recording",
                      BDMF_ACCESS_ADMIN, NULL, NULL);
    }
    {
        bdmfmon_cmd_add(hist_dir, "resume", bdmf_mon_hist_resume,
                      "Resume history recording",
                      BDMF_ACCESS_ADMIN, NULL, NULL);
    }
    {
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(hist_dir, "save", bdmf_mon_hist_save,
                      "Save history to file",
                      BDMF_ACCESS_ADMIN, NULL, parms);
    }
    {
        static bdmfmon_enum_val_t yes_no_table[] = {
            { .name="no",   .val=0},
            { .name="yes",  .val=1},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("stop", "Stop on result mismatch", yes_no_table, 0, "yes"),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(hist_dir, "play", bdmf_mon_hist_play,
                      "Playback history recording",
                      BDMF_ACCESS_ADMIN, NULL, parms);
    }

    return hist_dir;
}

#endif /* #ifdef BDMF_SHELL */
