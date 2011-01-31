/*
 * Configuration file loader, lex header
 *
 * $Id: load_lex.h,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */
#ifndef __LOAD_LEX
#define __LOAD_LEX

#include <stdio.h>

#include "lane.h"
#include "load.h"

#define UNIT 20
#define BOOLEAN 21
#define ATMADDRESS 22
#define STRING 23
#define INTEGER 24
#define LANEDEST 25
#define VARNAME 27
#define VCC 28
#define ERROR 26
#define END 0

typedef struct {
  Bool_t bool;
  int intti;
  AtmAddr_t *atmaddress;
  LaneDestination_t *destaddr;
  char *stringgi;
  LaneVcc_t vcc;
} Ret_t;

extern FILE *yyin;
extern int g_buf_index;
extern Ret_t g_return;

int yylex(void);
#endif
