/*
 * Copyright (c) 2009 Dave Gamble
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SRC_USR_UTIL_CJSON_H_
#define SRC_USR_UTIL_CJSON_H_

/* cJSON Types: */
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

typedef enum valuenum_flags {
	VALUENUM_INT = (1 << 0),
	VALUENUM_UINT = (1 << 1),
	VALUENUM_DBL = (1 << 2),
} valuenum_flags;

/* The cJSON structure: */
typedef struct cJSON {
	/*
	 * next/prev allow you to walk array/object chains.
	 * Alternatively, use GetArraySize/GetArrayItem/GetObjectItem
	 */
	struct cJSON *next;
	struct cJSON *prev;
	/* An array or object item will have a child pointer pointing to a chain
	 * of the items in the array/object.
	 */
	struct cJSON *child;

	/* The type of the item, as above. */
	int type;

	/* The item's string, if type==cJSON_String */
	char *valuestring;
	/* The item's number, if type==cJSON_Number */
	int valueint;
	unsigned int valueuint;
	double valuedouble;
	valuenum_flags numflags;

	/*
	 * The item's name string, if this item is the child of,
	 * or is in the list of subitems of an object.
	 */
	char *string;
} cJSON;

/*
 * Supply a block of JSON, and this returns a cJSON object you can interrogate.
 * Call cJSON_Delete when finished.
 */
cJSON *cJSON_Parse(const char *value);
/* Delete a cJSON entity and all subentities. */
void cJSON_Delete(cJSON *c);

/* Returns the number of items in an array (or object). */
int cJSON_GetArraySize(cJSON *array);
/* Returns item number "item" from array "array". NULL if unsuccessful. */
cJSON *cJSON_GetArrayItem(cJSON *array, int item);
/* Get item "string" from object. Case insensitive. */
cJSON *cJSON_GetObjectItem(cJSON *object, const char *string);

/*
 * For analysing failed parses. This returns a pointer to the parse error.
 * You'll probably need to look a few chars back to make sense of it.
 * Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds.
 */
const char *cJSON_GetErrorPtr(void);

/*
 * ParseWithOpts allows you to require (and check) that the JSON is
 * null-terminated, and to retrieve the pointer to the final byte parsed.
 */
cJSON *cJSON_ParseWithOpts(const char *value,
		const char **return_parse_end, int require_null_terminated);

#endif /* SRC_USR_UTIL_CJSON_H_ */
