/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JANSSON_H
#define JANSSON_H

#include <stdio.h>
#include <stdlib.h>  /* for size_t */
#include <stdarg.h>

#include "jansson_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* version */

#define JANSSON_MAJOR_VERSION  2
#define JANSSON_MINOR_VERSION  12
#define JANSSON_MICRO_VERSION  0

/* Micro version is omitted if it's 0 */
#define JANSSON_VERSION  "2.12"

/* Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
   for numeric comparisons, e.g. #if JANSSON_VERSION_HEX >= ... */
#define JANSSON_VERSION_HEX  ((JANSSON_MAJOR_VERSION << 16) |   \
                              (JANSSON_MINOR_VERSION << 8)  |   \
                              (JANSSON_MICRO_VERSION << 0))

/* If __atomic or __sync builtins are available the library is thread
 * safe for all read-only functions plus reference counting. */
#if JSON_HAVE_ATOMIC_BUILTINS || JSON_HAVE_SYNC_BUILTINS
#define JANSSON_THREAD_SAFE_REFCOUNT 1
#endif

#if defined(__GNUC__) || defined(__clang__)
#define JANSSON_ATTRS(...) __attribute__((__VA_ARGS__))
#else
#define JANSSON_ATTRS(...)
#endif

/* types */

typedef enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INTEGER,
    JSON_REAL,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL
} json_type;

typedef struct json_t {
    json_type type;
    volatile size_t refcount;
} json_t;

#ifndef JANSSON_USING_CMAKE /* disabled if using cmake */
#if JSON_INTEGER_IS_LONG_LONG
#ifdef _WIN32
#define JSON_INTEGER_FORMAT "I64d"
#else
#define JSON_INTEGER_FORMAT "lld"
#endif
typedef long long json_int_t;
#else
#define JSON_INTEGER_FORMAT "ld"
typedef long json_int_t;
#endif /* JSON_INTEGER_IS_LONG_LONG */
#endif


#define json_object dd_json_object
#define json_array dd_json_array
#define json_string dd_json_string
#define json_stringn dd_json_stringn
#define json_string_nocheck dd_json_nocheck
#define json_stringn_nocheck dd_jsonn_nocheck
#define json_integer dd_json_integer
#define json_real dd_json_real
#define json_true dd_json_true
#define json_false dd_json_false
#define json_null dd_json_null


#define json_typeof(json)      ((json)->type)
#define json_is_object(json)   ((json) && json_typeof(json) == JSON_OBJECT)
#define json_is_array(json)    ((json) && json_typeof(json) == JSON_ARRAY)
#define json_is_string(json)   ((json) && json_typeof(json) == JSON_STRING)
#define json_is_integer(json)  ((json) && json_typeof(json) == JSON_INTEGER)
#define json_is_real(json)     ((json) && json_typeof(json) == JSON_REAL)
#define json_is_number(json)   (json_is_integer(json) || json_is_real(json))
#define json_is_true(json)     ((json) && json_typeof(json) == JSON_TRUE)
#define json_is_false(json)    ((json) && json_typeof(json) == JSON_FALSE)
#define json_boolean_value     json_is_true
#define json_is_boolean(json)  (json_is_true(json) || json_is_false(json))
#define json_is_null(json)     ((json) && json_typeof(json) == JSON_NULL)

/* construction, destruction, reference counting */

json_t *json_object(void);
json_t *json_array(void);
json_t *json_string(const char *value);
json_t *json_stringn(const char *value, size_t len);
json_t *json_string_nocheck(const char *value);
json_t *json_stringn_nocheck(const char *value, size_t len);
json_t *json_integer(json_int_t value);
json_t *json_real(double value);
json_t *json_true(void);
json_t *json_false(void);
#define json_boolean(val)      ((val) ? json_true() : json_false())
json_t *json_null(void);

/* do not call JSON_INTERNAL_INCREF or JSON_INTERNAL_DECREF directly */
#if JSON_HAVE_ATOMIC_BUILTINS
#define JSON_INTERNAL_INCREF(json) __atomic_add_fetch(&json->refcount, 1, __ATOMIC_ACQUIRE)
#define JSON_INTERNAL_DECREF(json) __atomic_sub_fetch(&json->refcount, 1, __ATOMIC_RELEASE)
#elif JSON_HAVE_SYNC_BUILTINS
#define JSON_INTERNAL_INCREF(json) __sync_add_and_fetch(&json->refcount, 1)
#define JSON_INTERNAL_DECREF(json) __sync_sub_and_fetch(&json->refcount, 1)
#else
#define JSON_INTERNAL_INCREF(json) (++json->refcount)
#define JSON_INTERNAL_DECREF(json) (--json->refcount)
#endif

static JSON_INLINE
json_t *json_incref(json_t *json)
{
    if(json && json->refcount != (size_t)-1)
        JSON_INTERNAL_INCREF(json);
    return json;
}

#define json_delete dd_json_delete

/* do not call json_delete directly */
void json_delete(json_t *json);

static JSON_INLINE
void json_decref(json_t *json)
{
    if(json && json->refcount != (size_t)-1 && JSON_INTERNAL_DECREF(json) == 0)
        json_delete(json);
}

#if defined(__GNUC__) || defined(__clang__)
static JSON_INLINE
void json_decrefp(json_t **json)
{
    if(json) {
        json_decref(*json);
	*json = NULL;
    }
}

#define json_auto_t json_t __attribute__((cleanup(json_decrefp)))
#endif


/* error reporting */

#define JSON_ERROR_TEXT_LENGTH    160
#define JSON_ERROR_SOURCE_LENGTH   80

typedef struct json_error_t {
    int line;
    int column;
    int position;
    char source[JSON_ERROR_SOURCE_LENGTH];
    char text[JSON_ERROR_TEXT_LENGTH];
} json_error_t;

enum json_error_code {
    json_error_unknown,
    json_error_out_of_memory,
    json_error_stack_overflow,
    json_error_cannot_open_file,
    json_error_invalid_argument,
    json_error_invalid_utf8,
    json_error_premature_end_of_input,
    json_error_end_of_input_expected,
    json_error_invalid_syntax,
    json_error_invalid_format,
    json_error_wrong_type,
    json_error_null_character,
    json_error_null_value,
    json_error_null_byte_in_key,
    json_error_duplicate_key,
    json_error_numeric_overflow,
    json_error_item_not_found,
    json_error_index_out_of_range
};

static JSON_INLINE enum json_error_code dd_json_error_code(const json_error_t *e) {
    return (enum json_error_code)e->text[JSON_ERROR_TEXT_LENGTH - 1];
}


/* getters, setters, manipulation */
#define json_object_seed dd_json_object_seed
#define json_object_size dd_json_object_size
#define json_object_get dd_json_object_get
#define json_object_set_new dd_json_object_set_new
#define json_object_set_new_nocheck dd_json_object_set_new_nocheck
#define json_object_del dd_json_object_del
#define json_object_clear dd_json_object_clear
#define json_object_update dd_json_object_update
#define json_object_update_existing dd_json_object_update_existing
#define json_object_update_missing dd_json_object_update_missing
#define json_object_iter dd_json_object_iter
#define json_object_iter_at dd_json_object_iter_at
#define json_object_key_to_iter dd_json_object_key_to_iter
#define json_object_iter_next dd_json_object_iter_next
#define json_object_iter_key dd_json_object_iter_key
#define json_object_iter_value dd_json_object_iter_value
#define json_object_iter_set_new dd_json_object_iter_set_new


void json_object_seed(size_t seed);
size_t json_object_size(const json_t *object);
json_t *json_object_get(const json_t *object, const char *key) JANSSON_ATTRS(warn_unused_result);
int json_object_set_new(json_t *object, const char *key, json_t *value);
int json_object_set_new_nocheck(json_t *object, const char *key, json_t *value);
int json_object_del(json_t *object, const char *key);
int json_object_clear(json_t *object);
int json_object_update(json_t *object, json_t *other);
int json_object_update_existing(json_t *object, json_t *other);
int json_object_update_missing(json_t *object, json_t *other);
void *json_object_iter(json_t *object);
void *json_object_iter_at(json_t *object, const char *key);
void *json_object_key_to_iter(const char *key);
void *json_object_iter_next(json_t *object, void *iter);
const char *json_object_iter_key(void *iter);
json_t *json_object_iter_value(void *iter);
int json_object_iter_set_new(json_t *object, void *iter, json_t *value);

#define json_object_foreach(object, key, value) \
    for(key = json_object_iter_key(json_object_iter(object)); \
        key && (value = json_object_iter_value(json_object_key_to_iter(key))); \
        key = json_object_iter_key(json_object_iter_next(object, json_object_key_to_iter(key))))

#define json_object_foreach_safe(object, n, key, value)     \
    for(key = json_object_iter_key(json_object_iter(object)), \
            n = json_object_iter_next(object, json_object_key_to_iter(key)); \
        key && (value = json_object_iter_value(json_object_key_to_iter(key))); \
        key = json_object_iter_key(n), \
            n = json_object_iter_next(object, json_object_key_to_iter(key)))

#define json_array_foreach(array, index, value) \
	for(index = 0; \
		index < json_array_size(array) && (value = json_array_get(array, index)); \
		index++)

static JSON_INLINE
int json_object_set(json_t *object, const char *key, json_t *value)
{
    return json_object_set_new(object, key, json_incref(value));
}

static JSON_INLINE
int json_object_set_nocheck(json_t *object, const char *key, json_t *value)
{
    return json_object_set_new_nocheck(object, key, json_incref(value));
}

static JSON_INLINE
int json_object_iter_set(json_t *object, void *iter, json_t *value)
{
    return json_object_iter_set_new(object, iter, json_incref(value));
}

#define json_array_size dd_json_array_size
#define json_array_get dd_json_array_get
#define json_array_set_new dd_json_array_set_new
#define json_array_append_new dd_json_array_append_new
#define json_array_insert_new dd_json_array_insert_new
#define json_array_remove dd_json_array_remove
#define json_array_clear dd_json_array_clear
#define json_array_extend dd_json_array_extend

size_t json_array_size(const json_t *array);
json_t *json_array_get(const json_t *array, size_t index) JANSSON_ATTRS(warn_unused_result);
int json_array_set_new(json_t *array, size_t index, json_t *value);
int json_array_append_new(json_t *array, json_t *value);
int json_array_insert_new(json_t *array, size_t index, json_t *value);
int json_array_remove(json_t *array, size_t index);
int json_array_clear(json_t *array);
int json_array_extend(json_t *array, json_t *other);

static JSON_INLINE
int json_array_set(json_t *array, size_t ind, json_t *value)
{
    return json_array_set_new(array, ind, json_incref(value));
}

static JSON_INLINE
int json_array_append(json_t *array, json_t *value)
{
    return json_array_append_new(array, json_incref(value));
}

static JSON_INLINE
int json_array_insert(json_t *array, size_t ind, json_t *value)
{
    return json_array_insert_new(array, ind, json_incref(value));
}

#define json_string_value dd_json_string_value
#define json_string_length dd_json_string_length
#define json_integer_value dd_json_integer_value
#define json_real_value dd_json_real_value
#define json_number_value dd_json_number_value
#define json_string_set dd_json_string_set
#define json_string_setn dd_json_string_setn
#define json_string_set_nocheck dd_json_string_set_nocheck
#define json_string_setn_nocheck dd_json_string_setn_nocheck
#define json_integer_set dd_json_integer_set
#define json_real_set dd_json_real_set
const char *json_string_value(const json_t *string);
size_t json_string_length(const json_t *string);
json_int_t json_integer_value(const json_t *integer);
double json_real_value(const json_t *real);
double json_number_value(const json_t *json);

int json_string_set(json_t *string, const char *value);
int json_string_setn(json_t *string, const char *value, size_t len);
int json_string_set_nocheck(json_t *string, const char *value);
int json_string_setn_nocheck(json_t *string, const char *value, size_t len);
int json_integer_set(json_t *integer, json_int_t value);
int json_real_set(json_t *real, double value);

/* pack, unpack */

#define json_pack dd_json_pack
#define json_pack_ex dd_json_pack_ex
#define json_vpack_ex dd_json_vpack_ex

json_t *json_pack(const char *fmt, ...) JANSSON_ATTRS(warn_unused_result);
json_t *json_pack_ex(json_error_t *error, size_t flags, const char *fmt, ...) JANSSON_ATTRS(warn_unused_result);
json_t *json_vpack_ex(json_error_t *error, size_t flags, const char *fmt, va_list ap) JANSSON_ATTRS(warn_unused_result);

#define JSON_VALIDATE_ONLY  0x1
#define JSON_STRICT         0x2

#define json_unpack dd_unjson_unpack
#define json_unpack_ex dd_json_unpack_ex
#define json_vunpack_ex dd_json_vunpack_ex
#define json_vsprintf dd_json_vsprintf
#define json_sprintf dd_json_sprintf

int json_unpack(json_t *root, const char *fmt, ...);
int json_unpack_ex(json_t *root, json_error_t *error, size_t flags, const char *fmt, ...);
int json_vunpack_ex(json_t *root, json_error_t *error, size_t flags, const char *fmt, va_list ap);

/* sprintf */

json_t *json_sprintf(const char *fmt, ...) JANSSON_ATTRS(warn_unused_result, format(printf, 1, 2));
json_t *json_vsprintf(const char *fmt, va_list ap) JANSSON_ATTRS(warn_unused_result, format(printf, 1, 0));



/* equality */
#define json_equal dd_json_equal

int json_equal(const json_t *value1, const json_t *value2);


/* copying */
#define json_copy dd_json_copy
#define json_deep_copy dd_json_deep_copy

json_t *json_copy(json_t *value) JANSSON_ATTRS(warn_unused_result);
json_t *json_deep_copy(const json_t *value) JANSSON_ATTRS(warn_unused_result);


/* decoding */

#define JSON_REJECT_DUPLICATES  0x1
#define JSON_DISABLE_EOF_CHECK  0x2
#define JSON_DECODE_ANY         0x4
#define JSON_DECODE_INT_AS_REAL 0x8
#define JSON_ALLOW_NUL          0x10


typedef size_t (*json_load_callback_t)(void *buffer, size_t buflen, void *data);

#define json_loads dd_json_loads
#define json_loadb dd_json_loadb
#define json_loadf dd_json_loadf
#define json_loadfd dd_json_loadfd
#define json_load_file dd_json_load_file
#define json_load_callback dd_json_load_callback

json_t *json_loads(const char *input, size_t flags, json_error_t *error) JANSSON_ATTRS(warn_unused_result);
json_t *json_loadb(const char *buffer, size_t buflen, size_t flags, json_error_t *error) JANSSON_ATTRS(warn_unused_result);
json_t *json_loadf(FILE *input, size_t flags, json_error_t *error) JANSSON_ATTRS(warn_unused_result);
json_t *json_loadfd(int input, size_t flags, json_error_t *error) JANSSON_ATTRS(warn_unused_result);
json_t *json_load_file(const char *path, size_t flags, json_error_t *error) JANSSON_ATTRS(warn_unused_result);
json_t *json_load_callback(json_load_callback_t callback, void *data, size_t flags, json_error_t *error) JANSSON_ATTRS(warn_unused_result);


/* encoding */

#define JSON_MAX_INDENT         0x1F
#define JSON_INDENT(n)          ((n) & JSON_MAX_INDENT)
#define JSON_COMPACT            0x20
#define JSON_ENSURE_ASCII       0x40
#define JSON_SORT_KEYS          0x80
#define JSON_PRESERVE_ORDER     0x100
#define JSON_ENCODE_ANY         0x200
#define JSON_ESCAPE_SLASH       0x400
#define JSON_REAL_PRECISION(n)  (((n) & 0x1F) << 11)
#define JSON_EMBED              0x10000

typedef int (*json_dump_callback_t)(const char *buffer, size_t size, void *data);

#define json_dumps dd_json_dumps
#define json_dumpb dd_json_dumpb
#define json_dumpf dd_json_dumpf
#define json_dumpfd dd_json_dumpfd
#define json_dump_file dd_json_dump_file
#define json_dump_callback dd_json_dump_callback
 
char *json_dumps(const json_t *json, size_t flags) JANSSON_ATTRS(warn_unused_result);
size_t json_dumpb(const json_t *json, char *buffer, size_t size, size_t flags);
int json_dumpf(const json_t *json, FILE *output, size_t flags);
int json_dumpfd(const json_t *json, int output, size_t flags);
int json_dump_file(const json_t *json, const char *path, size_t flags);
int json_dump_callback(const json_t *json, json_dump_callback_t callback, void *data, size_t flags);

/* custom memory allocation */


typedef void *(*json_malloc_t)(size_t);
typedef void (*json_free_t)(void *);

#define json_set_alloc_funcs dd_json_set_alloc_funcs
#define json_get_alloc_funcs dd_json_get_alloc_funcs
void json_set_alloc_funcs(json_malloc_t malloc_fn, json_free_t free_fn);
void json_get_alloc_funcs(json_malloc_t *malloc_fn, json_free_t *free_fn);

#ifdef __cplusplus
}
#endif

#endif
