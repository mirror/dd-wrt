/*
 * 
 * ELAN Configuration db load header
 *
 * $Id: lecs_db.h,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 * 
 */
#ifndef LECS_DB_H
#define LECS_DB_H

#define ELAN_NAME 20
#define ADDRESS_ATM 21
#define ERROR 22
#define LES_ADDR 23
#define DEFAULT 24
#define TYPE 25
#define MAX_FRAME 26
#define TYPE_ETHERNET 27
#define TYPE_TR 28
#define MF_1516 29
#define MF_4544 30
#define MF_9234 31
#define MF_18190 32

extern FILE *yyin;
extern char *g_return;
extern unsigned int g_lineno;

int yylex(void);
#endif /* LECS_DB_H */
