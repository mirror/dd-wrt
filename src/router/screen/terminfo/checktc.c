#include <stdio.h>

char *CL, *CM, *CS, *SR;
int CO, LI, AM, XN;

char *tgetstr(), *getenv();
void PutStr(), CPutStr(), CCPutStr(), GotoPos(), RETURN();

main()
{
  char *term, *s;
  char tcbuf[1024];
  char tcstr[1024], *tp;

  if ((term = getenv("TERM")) == 0)
    {
      fprintf(stderr, "No $TERM set\n");
      exit(1);
    }
  switch (tgetent(tcbuf, term))
    {
    case -1:
      fprintf(stderr, "Could not open termcap file\n");
      exit(1);
    case 0:
      fprintf(stderr, "I don't know what a '%s' terminal is.\n", term);
      exit(1);
    }
  tp = tcstr;
  if ((CL = tgetstr("cl", &tp)) == 0)
    {
      fprintf(stderr, "cl capability required\n");
      exit(1);
    }
  if ((CM = tgetstr("cm", &tp)) == 0)
    {
      fprintf(stderr, "cm capability required\n");
      exit(1);
    }
  
  if ((s = getenv("COLUMNS")))
      CO = atoi(s);
  if ((s = getenv("LINES")))
      LI = atoi(s);
  if (CO == 0)
      CO = tgetnum("co");
  if (LI == 0)
      LI = tgetnum("li");
  if (CO == 0)
      CO = 80;
  if (LI == 0)
      LI = 24;
  GotoPos(5, 1);
  printf("******* cl capability does not work !!! *******");
  GotoPos(5, 2);
  PutStr(CL);
  printf("******* cl capability does not home cursor *******");
  GotoPos(0, 0);
  printf("                                                  ");
  GotoPos(5, 4);
  printf("******* cm capability does not work !!! *******");
  GotoPos(5, 4);
  printf("                                               ");
  GotoPos(CO/2-12, LI/2);
  printf("Your terminal size is");
  GotoPos(CO/2-3, LI/2+1);
  printf("%dx%d", CO, LI);
  GotoPos(CO/2-2, 0);
  printf("top");
  GotoPos(CO/2-3, LI-1);
  printf("bottom");
  GotoPos(0, LI/2-2);printf("l");
  GotoPos(0, LI/2-1);printf("e");
  GotoPos(0, LI/2+0);printf("f");
  GotoPos(0, LI/2+1);printf("t");
  GotoPos(CO-1, LI/2-2);printf("r");
  GotoPos(CO-1, LI/2-1);printf("i");
  GotoPos(CO-1, LI/2+0);printf("g");
  GotoPos(CO-1, LI/2+1);printf("h");
  GotoPos(CO-1, LI/2+2);printf("t");
  GotoPos(CO/2-15, LI/2+3);
  RETURN();
  AM = tgetflag("am");
  printf("Termcap: terminal does %sauto-wrap", AM ? "" : "not ");
  GotoPos(0, 5);
  if (AM)
    {
      printf(" am capability set, but terminal does not wrap");
      GotoPos(CO-1, 3);
    }
  else
    {
      printf(" am capability not set, but terminal does wrap");
      GotoPos(CO-1, 4);
    }
  printf("  \n                                                  ");
  GotoPos(0, 10);
  RETURN();
  if (AM)
    {
      XN = tgetflag("xn");
      printf("Termcap: terminal has %smagic margins", XN ? "" : "no ");
      GotoPos(0, 5);
      if ((XN = tgetflag("xn")))
	{
	  printf(" xn capability set, but terminal has no magic-margins");
	  GotoPos(CO-1, 4);
	}
      else
	{
	  printf(" xn capability not set, but terminal has magic-margins");
	  GotoPos(CO-1, 3);
	}
      printf(" \n");
      printf("                                                       ");
      GotoPos(0, 10);
      RETURN();
      if (XN)
        {
          GotoPos(0, 6);
	  printf(" last col in last row is not usable");
          GotoPos(CO-1, LI-1);
	  printf(" ");
          GotoPos(0, 6);
	  printf("                                          ");
	  GotoPos(0, 0);
	  printf("testing magic margins in last row");
	  GotoPos(0, 10);
	  RETURN();
        }
    }
  if ((CS = tgetstr("cs", &tp)))
    {
      printf("Termcap: terminal has scrollregions");
      GotoPos(0, 5);
      printf(" cs capability set, but doesn't work");
      CCPutStr(CS, 4, 5);
      GotoPos(0, 5);
      printf("\n\n");
      CCPutStr(CS, 0, LI-1);
      GotoPos(0, 10);
      RETURN();
    }
  if ((SR = tgetstr("sr", &tp)))
    {
      GotoPos(0, 5);
      printf(" sr capability set, but doesn't work");
      GotoPos(0, 0);
      PutStr(SR);
      GotoPos(0, 6);
      printf("                                    ");
      GotoPos(0, 0);
      printf("Termcap: terminal can scroll backwards");
      GotoPos(0, 10);
      RETURN();
    }
}

void
putcha(c)
char c;
{
  putchar(c);
}

void
PutStr(s)
char *s;
{
  tputs(s, 1, putcha);
  fflush(stdout);
}

void CPutStr(s, c)
char *s;
int c;
{
  tputs(tgoto(s, 0, c), 1, putcha);
  fflush(stdout);
}

void CCPutStr(s, x, y)
char *s;
int x, y;
{
  tputs(tgoto(s, y, x), 1, putcha);
  fflush(stdout);
}

void GotoPos(x,y)
int x,y;
{
  tputs(tgoto(CM, x, y), 1, putcha);
  fflush(stdout);
}

void
RETURN()
{
  printf("Press <RETURN> to continue");
  fflush(stdout);
  while(getchar() != '\n');
  PutStr(CL);
}
