/*
 *
 * LECS configuration database loading
 *
 * $Id: lecs_load.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <stdio.h>
#include <stdlib.h>

/* Local includes */
#include "lecs_load.h"
#include "lecs_db.h"
#include "ldb.h"
#include "lecs.h"

int
load_db(const char *filename)
{
  int ret = 0;     /* silence gcc 2.7.2.1 */
  int readnew=1;
  Elan_t *elan =NULL;
  char message[300];

  yyin = fopen(filename,"r");
  if(!yyin) {
    sprintf(message,"Can't open configuration file '%s' ",filename);
    perror(message);    
    return -1;
  }

  while(1) {
    if (readnew) 
      ret = yylex();
    readnew=1;

    switch(ret) {
    case ELAN_NAME:
      elan = new_elan(g_return);
      break;
    case TYPE:
      if (!elan) {
	printf("No ELAN for TYPE; line %d\n", g_lineno);
	break;
      }
      ret = yylex();
      if (ret == TYPE_ETHERNET) {
	elan->type = LE_MAX_FRAME_1516;
      } else if (ret == TYPE_TR) {
	elan->type = LE_LAN_TYPE_802_3;
      } else {
	printf("Invalid type; line %d\n", g_lineno);
      }
      break;
    case TYPE_ETHERNET:
    case TYPE_TR:
      printf("Invalid type placement; line %d\n", g_lineno);
      break;
    case MAX_FRAME:
      if (!elan) {
	printf("No ELAN for Max frame size; line %d\n", g_lineno);
	break;
      }
      ret = yylex();
      switch (ret) {
      case MF_1516:
	elan->max_frame = LE_MAX_FRAME_1516;
	break;
      case MF_4544:
	elan->max_frame = LE_MAX_FRAME_4544;
	break;
      case MF_9234:
	elan->max_frame = LE_MAX_FRAME_9234;
	break;
      case MF_18190:
	elan->max_frame = LE_MAX_FRAME_18190;
	break;
      default:
	printf("Invalid max frame size %d\n", g_lineno);
	break;
      }
      break;
    case MF_1516:
    case MF_4544:
    case MF_9234:
    case MF_18190:
      printf("Invalid max frame size placement; line %d\n", g_lineno);
      break;
    case LES_ADDR:
      if (!elan) {
	printf("No ELAN for LES; line %d\n",g_lineno);
	break;
      }
      ret = yylex();
      if (ret != ADDRESS_ATM) {
	printf("ATM address for LES missing; line %d; ret:%d\n",g_lineno,ret);
	readnew=0;
      } else
	if (add_les(elan, g_return)<0) {
	  printf("Couldn't read LES address; line %d\n", g_lineno);
	}
      break;
    case ADDRESS_ATM:
      if (!elan) {
	set_lecs_addr(g_return);	
	break;
      }
      if (add_atm(elan, g_return)<0)
	printf("Couldn't read LEC address; line %d\n", g_lineno);
      break;
    case DEFAULT:
      if (!elan) {
	printf("No ELAN to set as a default; line %d\n",g_lineno);
	break;
      }
      set_default(elan);
      break;
    case ERROR:
      printf("Error reading database file; line %d\n", g_lineno);
      while(ret==ERROR)
	ret=yylex();
      readnew=0;
      break;
    default:
      fclose(yyin);
      return 0;
    }
  }
}
