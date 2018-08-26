#include <stdio.h>
#include <locale.h>

#include <slang.h>

#include <wctype.h>

int main (int argc, char **argv)
{
   char * locale;
   int i;
#define VARIABLE(xxx) unsigned int sys_##xxx=0, sl_##xxx = 0
   VARIABLE(lower);
   VARIABLE(alpha);
   VARIABLE(upper);
   VARIABLE(graph);
   VARIABLE(print);
   VARIABLE(space);
   VARIABLE(alnum);
   VARIABLE(digit);
   VARIABLE(xdigit);
   VARIABLE(cntrl);
   VARIABLE(punct);

   if (NULL == (locale = setlocale (LC_ALL, "en_US.UTF-8")))
     {
	fprintf (stderr, "setlocale failed\n");
	exit (1);
     }

   (void) SLutf8_enable (1);
   fprintf (stdout, "locale = %s\n", locale);

   for (i = 0; i <= 0x10FFFD; i++)
     {
#define TEST(xxx) \
   if ((0 == isw##xxx(i)) != (0 == SLwchar_is##xxx(i))) \
       fprintf (stdout, "Failed " #xxx "(0x%X) : 0x%X != 0x%X\n", \
		   i, isw##xxx(i), SLwchar_is##xxx(i))

	if (towupper (i) != SLwchar_toupper (i))
	  fprintf (stdout, "towupper (0x%X) failed 0x%X vs 0x%lX\n",
		   i, towupper(i), SLwchar_toupper(i));
	if (towlower (i) != SLwchar_tolower (i))
	  fprintf (stdout, "towlower (0x%X) failed 0x%X vs 0x%lX\n",
		   i, towlower(i), SLwchar_tolower(i));
#if 0
#undef TEST
#define TEST(xxx) \
     sys_##xxx += (0 != isw##xxx(i)); sl_##xxx += (0 != SLwchar_is##xxx(i))

	TEST(alpha);
	TEST(print);
	TEST(alnum);
	TEST(graph);
	TEST(lower);
	TEST(punct);
#endif
	TEST(lower);
	TEST(upper);
	TEST(cntrl);
	TEST(space);
	TEST(xdigit);
	TEST(digit);
     }

#define PRINT(xxx) \
   fprintf (stdout, #xxx ": %7u %7u\n", sys_##xxx, sl_##xxx)

   PRINT(lower);
   PRINT(alpha);
   PRINT(digit);
   PRINT(print);
   PRINT(alnum);
   PRINT(cntrl);
   PRINT(graph);
   PRINT(punct);
   PRINT(space);
   PRINT(upper);
   PRINT(xdigit);

   return 0;
}
