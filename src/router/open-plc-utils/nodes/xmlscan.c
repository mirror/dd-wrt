/*====================================================================*
 *
 *   xmlscan.c - markup scanner;
 *
 *   node.h
 *
 *   scan XML source and create a parse tree;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef XMLSCAN_SOURCE
#define XMLSCAN_SOURCE

/*====================================================================*
 *  system header files;
 *--------------------------------------------------------------------*/

#include <string.h>
#include <ctype.h>

/*====================================================================*
 *  custom header files;
 *--------------------------------------------------------------------*/

#include "../nodes/node.h"
#include "../tools/number.h"
#include "../tools/error.h"

/*====================================================================*
 *
 *   char * advance (char * string, unsigned * line);
 *
 *   discard whitespace and count newlines up to the next meaningful
 *   character;
 *
 *   this function is critical to the XML parsing engine because it
 *   ensures that node strings are NUL terminated and line counts
 *   are accurate;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char * advance (char * string, unsigned * lineno)

{
	while (isspace (*string))
	{
		if (*string == '\n')
		{
			(*lineno)++;
		}
		*string++ = (char)(0);
	}
	return (string);
}


/*====================================================================*
 *
 *   char * discard (char * string, unsigned * line);
 *
 *   discard current character; advance to next character;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char * discard (char * string, unsigned * lineno)

{
	*string++ = (char)(0);
	string = advance (string, lineno);
	return (string);
}


/*====================================================================*
 *
 *   char * nmtoken (char * string);
 *
 *   collect nmtoken as per w3c xml 1.0 specification;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char * nmtoken (char * string)

{
	while (isalnum (*string) || (*string == '-') || (*string == '_') || (*string == '.') || (*string == ':'))
	{
		string++;
	}
	return (string);
}


/*====================================================================*
 *
 *   char * content (char * string, char quote, unsigned * line);
 *
 *   collect literal string; discard quotes; preserve whitespace;
 *   count newlines;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char * content (char * string, char quote, unsigned * lineno)

{
	if (*string == quote)
	{
		*string++ = (char)(0);
	}
	while (*string)
	{
		if (*string == quote)
		{
			break;
		}
		if (*string++ == '\n')
		{
			(*lineno)++;
		}
	}
	if (*string == quote)
	{
		*string++ = (char)(0);
	}
	return (string);
}


/*====================================================================*
 *
 *   char * collect (char * string);
 *
 *   collect entity; an entity consists of non-blank characters
 *   excluding common tag punctuation;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char * collect (char * string)

{
	while (*string)
	{
		if (*string == '<')
		{
			break;
		}
		if (*string == '=')
		{
			break;
		}
		if (*string == '/')
		{
			break;
		}
		if (*string == '?')
		{
			break;
		}
		if (*string == '>')
		{
			break;
		}
		if (isspace (*string))
		{
			break;
		}
		string++;
	}
	return (string);
}


/*====================================================================*
 *
 *   static char * comment (char * string, unsigned * line);
 *
 *   collect comment;
 *   preserve delimiters;
 *   preserve whitespace;
 *   count newlines;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char * comment (char * string, unsigned * lineno)

{
	string++;
	if (*string == '-')
	{
		while (*string == '-')
		{
			string++;
		}
		while ((*string) && (*string != '-'))
		{
			while ((*string) && (*string != '-'))
			{
				if (*string == '\n')
				{
					(*lineno)++;
				}
				string++;
			}
			string++;
		}
		while (*string == '-')
		{
			string++;
		}
	}
	return (string);
}


/*====================================================================*
 *
 *   char * literal (char * string, char quote, unsigned * line);
 *
 *   collect literal;
 *   preserve delimiters;
 *   preserve whitespace;
 *   count newlines;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char * literal (char *string, char quote, unsigned * lineno)

{
	if (*string == quote)
	{
		*string++ = (char)(0);
	}
	while (*string)
	{
		if (*string == quote)
		{
			break;
		}
		if (*string == '\n')
		{
			(*lineno)++;
		}
		string++;
	}
	if (*string == quote)
	{
		*string++ = (char)(0);
	}
	return (string);
}


/*====================================================================*
 *
 *   char * context (char * string, signed c, unsigned *line);
 *
 *   collect context;
 *   preserve delimiters;
 *   preserve whitespace;
 *   count newlines;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static char * context (char *string, signed c, unsigned * lineno)

{
	string++;
	while (*string)
	{
		if (*string == (char)(c))
		{
			string++;
			break;
		}
		if (*string == '{')
		{
			string = context (string, '}', lineno);
			continue;
		}
		if (*string == '(')
		{
			string = context (string, ')', lineno);
			continue;
		}
		if (*string == '[')
		{
			string = context (string, ']', lineno);
			continue;
		}
		if ((*string == '\"') || (*string == '\''))
		{
			string = literal (string, *string, lineno);
			continue;
		}
		if (*string == '\n')
		{
			(*lineno)++;
		}
		string++;
	}
	return (string);
}


/*====================================================================*
 *
 *   void xmlscan (NODE * node);
 *
 *   node.h
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

signed xmlscan (NODE * node)

{
	NODE * section = node;
	NODE * element;
	NODE * attribute;
	NODE * value;
	char prefix = (char)(0);
	char suffix = (char)(0);
	char * string = node->text;
	unsigned lineno = 1;
	if (!section)
	{
		error (1, EFAULT, "section is null");
	}
	if (!string)
	{
		error (1, EFAULT, "string is null");
	}
	while (*string)
	{
		if (*string == '<')
		{
			prefix = '<';
			suffix = '>';
			string = discard (string, &lineno);
			if ((*string == '/') || (*string == '?') || (*string == '!'))
			{
				prefix = *string;
				string = discard (string, &lineno);
			}
			element = xmlnode (section);
			element->line = lineno;
			element->type = NODE_ELEM;
			element->text = string;
			if (isalpha (*string))
			{
				string = nmtoken (string);
			}
			else if (*string == '-')
			{
				string = comment (string, &lineno);
			}
			else if (*string == '[')
			{
				string = context (string, ']', &lineno);
			}
			else
			{
				string = collect (string);
			}
			string = advance (string, &lineno);
			while ((*string) && (*string != '<') && (*string != '/') && (*string != '?') && (*string != '>'))
			{
				attribute = xmlnode (element);
				attribute->line = lineno;
				attribute->type = NODE_ATTR;
				attribute->text = string;
				if (isalpha (*string))
				{
					string = nmtoken (string);
				}
				else if (*string == '-')
				{
					string = comment (string, &lineno);
				}
				else if (*string == '[')
				{
					string = context (string, ']', &lineno);
				}
				else if ((*string == '\"') || (*string == '\''))
				{
					string = content (string, *string, &lineno);
					attribute->text++;
				}
				else
				{
					string = collect (string);
				}
				string = advance (string, &lineno);
				if (*string == '=')
				{
					string = discard (string, &lineno);
					value = xmlnode (attribute);
					value->line = lineno;
					value->type = NODE_VALU;
					value->text = string;
					if ((*string == '\"') || (*string == '\''))
					{
						string = content (string, *string, &lineno);
						value->text++;
					}
					else
					{
						string = collect (string);
					}
					string = advance (string, &lineno);
				}
			}
			if ((*string == '/') || (*string == '?'))
			{
				suffix = *string;
				string = discard (string, &lineno);
			}
		}
		else if (*string == '>')
		{
			string = discard (string, &lineno);
			if (prefix == '!')
			{
				element->type = NODE_SGML;
			}
			else if (prefix == '?')
			{
				element->type = NODE_INST;
			}
			else if (suffix == '?')
			{
			}
			else if (prefix == '/')
			{
				element->type = NODE_ETAG;
				if (element->below)
				{
					error (1, 0, "Element </%s> on line %d has attributes or content.", element->text, element->line);
				}
				if (strcmp (section->text, element->text))
				{
					error (1, 0, "Element <%s> on line %d teminated by </%s> on line %d", section->text, section->line, element->text, element->line);
				}
				if (section->above)
				{
					section = section->above;
				}
			}
			else if (suffix == '/')
			{
			}
			else
			{
				section = element;
			}
		}
		else
		{
			signed space = 0;
			char * output = string;
			NODE * segment = xmlnode (section);
			segment->line = lineno;
			segment->type = NODE_DATA;
			segment->text = string;
			while (*string)
			{
				if (*string == '<')
				{
					break;
				}
				if (isspace (*string))
				{
					string = advance (string, &lineno);
					space++;
					continue;
				}
				if (space)
				{
					*output++ = ' ';
					space--;
				}
				*output++ = *string++;
			}
			if (output < string)
			{
				*output = (char)(0);
			}
		}
	}
	return (0);
}


#endif

