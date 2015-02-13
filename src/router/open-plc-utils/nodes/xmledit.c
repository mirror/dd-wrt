/*====================================================================*
 *
 *   xmledit.c -
 *
 *   node.h
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef XMLEDIT_SOURCE
#define XMLEDIT_SOURCE

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

/*====================================================================*
 *  custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/number.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../nodes/node.h"

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define XML_BAD_NUMBER 1
#define XML_BAD_OFFSET 2
#define XML_BAD_EXTENT 3

/*====================================================================*
 *   variables;
 *--------------------------------------------------------------------*/

static char const * member = "";
static char const * string = "";
static unsigned offset = 0;
static unsigned length = 0;
static bool series = false;

/*====================================================================*
 *
 *   void position (size_t extent);
 *
 *   sanity check offset and extent before editing memory;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static void position (size_t extent)

{
	if (!length)
	{
		error (XML_BAD_EXTENT, EPERM, "%s has no length", member);
	}
	if (offset > extent)
	{
		error (XML_BAD_OFFSET, EPERM, "%s offset of 0x%04X exceeds " DATA_OBJECT " offset of 0x%04X", member, offset, (unsigned int) extent);
	}
	if ((offset + length) > extent)
	{
		error (XML_BAD_EXTENT, EPERM, "%s length of %u bytes exceeds " DATA_OBJECT " length of " SIZE_T_SPEC " bytes", member, length, extent); 
	}
	return;
}


/*====================================================================*
 *
 *   signed xmlinteger (NODE const * node, unsigned radix);
 *
 *   convert numeric string to an unsigned integer; all string digits
 *   string digits must be valid for the specifid radix; radix can be
 *   1 through 16 but 2, 8, 10 and 16 are the only sensible choices;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static unsigned xmlinteger (NODE const * node, unsigned radix)

{
	unsigned digit;
	unsigned value = 0;
	while ((digit = todigit (*string)) < radix)
	{
		value *= radix;
		value += digit;
		string++;
	}
	if (*string)
	{
		error (XML_BAD_NUMBER, EPERM, "%s %s is not numeric", member, node->text);
	}
	return (value);
}


/*====================================================================*
 *
 *   void xmlstring (void * memory, size_t extent);
 *
 *   xmlstring is expressed as character text;  truncate string and
 *   pad memory with NULs as needed;
 *
 *   per the schema, an series cannot have a string member;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static void xmlstring (void * memory, size_t extent)

{
	char * buffer = (char *)(memory);
	if (series)
	{
		error (XML_BAD_NUMBER, ENOTSUP, "%s found inside struct", member);
	}
	if (length)
	{
		while (length > 1)
		{
			if (isprint (*string))
			{
				buffer [offset] = *string++;
			}
			else
			{
				buffer [offset] = (char)(0);
			}
			offset++;
			length--;
		}
		buffer [offset] = (char)(0);
		offset++;
		length--;
	}
	return;
}


/*====================================================================*
 *
 *   void xmlmemory (void * memory, size_t extent);
 *
 *   xmlmemory is a hexadecimal string of variable extent; an empty
 *   string increments offset and decrements length but nothing is
 *   written to the memory;
 *
 *   per the schema, if xmlmemory is not inside an series then it must
 *   match the object extent;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static void xmlmemory (void * memory, size_t extent)

{
	uint8_t * buffer = (uint8_t *)(memory);
	if (!*string)
	{
		offset++;
		length--;
	}
	while ((*string) && (length))
	{
		uint8_t msb = todigit (*string++);
		uint8_t lsb = todigit (*string++);
		if ((msb > 0x0F) || (lsb > 0x0F))
		{
			error (XML_BAD_NUMBER, EINVAL, "%s value is not hexadecimal", member);
		}
		buffer [offset] = (msb << 4) + lsb;
		offset++;
		length--;
	}
	if ((length) && (!series))
	{
		error (XML_BAD_NUMBER, EINVAL, "%s value is too short", member);
	}
	if (*string)
	{
		error (XML_BAD_NUMBER, EINVAL, "%s value is too long", member);
	}
	return;
}


/*====================================================================*
 *
 *   void xmlnumber (void * memory, size_t extent);
 *
 *   xmlnumber is a decimal integer string of variable length; the
 *   value cannot exceed length bytes and offset is incremented by
 *   length bytes;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static void xmlnumber (void * memory, size_t extent)

{
	uint64_t number = 0;
	uint64_t maximum = 0;
	maximum = ~maximum;
	if (length < sizeof (number))
	{
		maximum <<= (length << 3);
		maximum = ~maximum;
	}
	while (isdigit (*string))
	{
		number *= 10;
		number += *string - '0';
		if (number > maximum)
		{
			error (XML_BAD_NUMBER, EINVAL, "%s value exceeds %u bytes", member, length); 
		}
		string++;
	}
	if (*string)
	{
		error (XML_BAD_NUMBER, EINVAL, "%s value is not decimal", member);
	}
	memcpy ((uint8_t *)(memory) + offset, &number, length);
	offset += length;
	length -= length;
	return;
}


/*====================================================================*
 *
 *   void xmlbyte (void * memory, unsigned extent);
 *
 *   xmlbyte is a decimal integer string of variable extent; the
 *   value cannot exceed 255; an empty string increments offset and
 *   decrements length but nothing is written to the memory;
 *
 *   per the schema, if xmlbyte is not inside an series then it must
 *   it must match the object extent which must be 1 by implication;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static void xmlbyte (void * memory, size_t extent)

{
	if (*string)
	{
		uint16_t number = 0;
		while (isdigit (*string))
		{
			number *= 10;
			number += *string - '0';
			if (number > 0xFF)
			{
				error (XML_BAD_NUMBER, EINVAL, "%s value exceeds 8 bits", member);
			}
			string++;
		}
		if (*string)
		{
			error (XML_BAD_NUMBER, EINVAL, "%s value is not decimal", member);
		}
		memcpy ((uint8_t *)(memory) + offset, &number, sizeof (uint8_t));
	}
	offset++;
	length--;
	if ((length) && (!series))
	{
		error (XML_BAD_NUMBER, EINVAL, "%s is too short", member);
	}
	return;
}


/*====================================================================*
 *
 *   static char const * xmlcontent1 (struct node const * node);
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char const * xmlcontent1 (struct node const * node)

{
	if (node)
	{
		node = node->below;
	}
	while (node)
	{
		if (node->type == NODE_DATA)
		{
			return (node->text);
		}
		node = node->after;
	}
	return ("");
}


/*====================================================================*
 *
 *   char const * xmlvalue1 (struct node const * node);
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

char const * xmlvalue1 (struct node const * node)

{
	if (node)
	{
		node = node->below;
	}
	while (node)
	{
		if (node->type == NODE_VALU)
		{
			return (node->text);
		}
		node = node->after;
	}
	return ("");
}


/*====================================================================*
 *
 *   static char const * xmlattribute1 (struct node const * node, char const * name);
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char const * xmlattribute1 (struct node const * node, char const * name)

{
	if (node)
	{
		node = node->below;
	}
	while (node)
	{
		if (node->type == NODE_ATTR)
		{
			if (!strcmp (node->text, name))
			{
				name = xmlvalue1 (node);
				return (name);
			}
		}
		node=node->after;
	}
	return ("");
}


/*====================================================================*
 *
 *   signed xmledit (struct node const * node, void * memory, size_t extent);
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

signed xmledit (struct node const * node, void * memory, size_t extent)

{
	if (node)
	{
		node = node->below;
	}
	while (node)
	{
		if (node->type == NODE_ELEM)
		{
			if (!strcmp (node->text, DATA_MEMBER))
			{
				member = xmlattribute1 (node, DATA_NAME);
				offset = (unsigned)(-1);
				length = (unsigned)(-1);
				series = false;
			}
			else if (!strcmp (node->text, DATA_OFFSET))
			{
				string = xmlcontent1 (node);
				offset = xmlinteger (node, 16);
			}
			else if (!strcmp (node->text, DATA_LENGTH))
			{
				string = xmlcontent1 (node);
				length = xmlinteger (node, 10);
			}
			else if (!strcmp (node->text, DATA_STRUCT))
			{
				series = true;
			}
			else if (!strcmp (node->text, DATA_STRING))
			{
				string = xmlcontent1 (node);
				position (extent);
				xmlstring (memory, extent);
			}
			else if (!strcmp (node->text, DATA_MEMORY))
			{
				string = xmlcontent1 (node);
				position (extent);
				xmlmemory (memory, extent);
			}
			else if (!strcmp (node->text, DATA_HUGE))
			{
				length = sizeof (uint64_t);
				position (extent);
				string = xmlcontent1 (node);
				xmlnumber (memory, extent);
			}
			else if (!strcmp (node->text, DATA_LONG))
			{
				length = sizeof (uint32_t);
				position (extent);
				string = xmlcontent1 (node);
				xmlnumber (memory, extent);
			}
			else if (!strcmp (node->text, DATA_WORD))
			{
				length = sizeof (uint16_t);
				position (extent);
				string = xmlcontent1 (node);
				xmlnumber (memory, extent);
			}
			else if (!strcmp (node->text, DATA_BYTE))
			{
				position (extent);
				string = xmlcontent1 (node);
				xmlbyte (memory, extent);
			}
			xmledit (node, memory, extent);
		}
		node = node->after;
	}
	return (0);
}


/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

