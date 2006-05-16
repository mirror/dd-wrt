/*
This is a libary collection
*/

#include "StrUtils.h"
#include <ctype.h>
#include <stdarg.h>

/// std::string before del
/**
Use:
	- Returns the string before del
	- Always takes the first delimiter text "del".
*/
/*
const std::string 
head(const std::string &str, const std::string &del) 
{
	return std::string(str,0, str.find(del));
}
*/

/// std::string before del (char)
/**
Use:
	- Returns the string before del char
	- Always takes the first delimiter char "del".
 */
const std::string 
headc(const std::string &str, const char &c) 
{
	return std::string(str,0, str.find(c));
}

/// std::string before last del
/**
Use:
	- Returns the std::string before del
	- Always takes the last delimiter text "del".
*/
/*
const std::string 
headr(const std::string &str, const std::string &del) 
{
	return std::string(str,0, str.rfind(del));
}
*/


/// std::string after del
/**
Use:
	- Returns the std::string after del
	- Always takes the first delimiter text "del".
*/
/*
const std::string 
tail(const std::string &str, const std::string &del) 
{	
	int delimiter_pos;
	delimiter_pos = str.find(del);	
	if (delimiter_pos == -1) 
	{
		return std::string();
	}	
	int delimiter_end;
	delimiter_end = delimiter_pos + del.length();	
	return std::string(str,delimiter_end, str.length() - delimiter_end);
}
*/

/// std::string after last del
/**
Use:
	- Returns the std::string after del
	- Always takes the last delimiter text "del".
*/
/*
const std::string 
tailr(const std::string &str, const std::string &del) 
{	
	int delimiter_pos;
	delimiter_pos = str.rfind(del);	
	if (delimiter_pos == -1) 
	{
		return std::string();
	}	
	int delimiter_end;
	delimiter_end = delimiter_pos + del.length();	
	return std::string(str,delimiter_end, str.length() - delimiter_end);
}
*/

/// cleans white space
/**
Use:
	- Returns std::string with white space removed from start and end of std::string
*/
const std::string 
trim(const std::string &str) 
{
	return trim_start(trim_end(str));
}

/// cleans white space from start
/**
Use:
	- Returns std::string with white space removed from start of std::string.
*/

const std::string 
trim_start(const std::string &str) 
{
	unsigned long start_pos;
	if (str.empty())
		return std::string("");
	start_pos = str.find_first_not_of(" \n\t");
	if (str.npos == start_pos)
		return std::string("");
	return std::string(str, start_pos);
}

/// cleans white space from end.
/**
Use:
	- Returns std::string with white space removed from end of std::string.
*/
const std::string 
trim_end(const std::string &str) 
{	
	if (str.empty())
		return std::string("");
	return std::string(str, 0,1 + str.find_last_not_of(" \n\t",str.length()));
}
/// compares the end of a std::string
/**
Use:
	- Returns true is str ends with end. false otherwise.
*/
/*
bool 
ends_with(const std::string &str, const std::string &end)
{
	// Note linux rh 6.2 does not support the following line hence alturnative implementation will be provided
	// return (1 == str.compare(str.length() - end.length(), end.length(), str));
	std::string tmp;
	int length_str, length_end;
	length_str = str.length();	 
	length_end = end.length();
	if (length_str < length_end)
		return false;
	tmp.assign(str, length_str -  length_end , length_end);
	return (tmp == end); 
}
*/
/// Change to upper case 
/**
Use:
	- Returns an uper case sring
*/
const std::string 
case_upper(const std::string &str)
{
	std::string output(str);
	int i;
	int length = output.length();
	for (i = 0; i < length; i++)
	{
		output[i] = toupper(str[i]);		
	}
	return output;
}

/// Change to lower case 
/**
Use:
	- Returns an lower case sring
*/
const std::string 
case_lower(const std::string &str)
{
	std::string output(str);
	int i;
	int length = output.length();
	for (i = 0; i < length; i++)
	{
		output[i] = tolower(str[i]);
	}
	return output;
}

/*const std::string 
convert_numtostr(const int input)
{
	char convert_buffer[10];
	sprintf(convert_buffer,"%d",input);
	return std::string(convert_buffer);
}



const std::string format_d(int val)
{
	std::string output;
	int convertor_size = 8;
	char *convertor_ptr;
	int convertor_result;
	convertor_ptr = (char *)malloc(convertor_size);
	if (convertor_ptr == NULL)
	{
		return std::string(NULL);
	}
	do
	{
		convertor_result = snprintf(convertor_ptr, convertor_size, "%d",val);

		if ((convertor_result == -1) || (convertor_result >= convertor_size))
		{
			free(convertor_ptr);
			convertor_size << 1;
			convertor_ptr = (char *)malloc(convertor_size);
			if (convertor_ptr == NULL)
			{		
				return std::string(NULL);
			}
		}
	}
	while ((convertor_result == -1) || (convertor_result >= convertor_size));
	return std::string(convertor_ptr);
}

const std::string format_f(double val)
{
	return "";
}
*/

/// printf to C++ std::string
/**
Use:
	- use as printf
		-# use "fmt" to define the structure of the conversions required.
			- \%d,i int; signed decimal notation. 
			- \%f double; decimal notation of the form [-]mmm,ddd where the number of 
			d's is specified by the presision. the default precision is 6; a precision 
			of 0 will supress the decimal point.
				-# Not yet implemented
			- \%s char *; characters from a std::string are printed until a '\\0' is reached
			or until the number of characters indicated by the precision have been 
			printed.
				-# precision function has not yet been implemented.
	- this function will expand on demand of coders using this library
*/
/*
const std::string format(char *fmt, ...)
{	
	std::string output;
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;
	
	

	va_start(ap, fmt);
	for (p = fmt; *p; p++)
	{
		if (*p != '%')
		{
			output.append(p, 1);
		}
		switch (*++p)
		{
			case 'd':
			case 'i':
				ival = va_arg(ap,int);
				output += format_d(ival);
				break;
			case 'f':
				dval = va_arg(ap,double);
				output += format_f(dval);
				break;
			case 's':				
				sval = va_arg(ap,char *);
				output += sval;
				break;
			default:
				output.append(p, 1);
				break;
		}
	}
	return output;
}

const std::string 
url_encode(const std::string &src)
{
	char character;
	std::string output;
	int length = src.length();
	int counter = 0;
	while (counter < length)
	{
		character = src[counter++];
		switch (character)
		{
			case '/':
				output += "%2F";
				break;
			case '=':
				output += "%3D";
				break;

			case '%':
				output += "%25";
				break;
			case ' ':
				output += "%20";
				break;
			default:
				output += character;
				break;
		}
		
	}
	return output;
}
*/
