/***

ethdesc.c	- Ethernet host description management module

Copyright (c) Gerard Paul Java 1998

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

***/

#include "descrec.h"

#define WITHETCETHERS 1
#define WITHOUTETCETHERS 0

void etherr(void);
void loaddesclist(struct desclist *list, unsigned int linktype,
                  int withethers);
void savedesclist(struct desclist *list, unsigned int linktype);
void displayethdescs(struct desclist *list, WINDOW * win);
void destroydesclist(struct desclist *list);
void operate_descselect(struct desclist *list, struct desclistent **node,
                        WINDOW * win, int *aborted);
void selectdesc(struct desclist *list, struct desclistent **node,
                int *aborted);
void descdlg(struct descrec *rec, char *initaddr, char *initdesc,
             int *aborted);
void addethdesc(struct desclist *list);
void editethdesc(struct desclist *list);
void delethdesc(struct desclist *list);
void ethdescmgr(unsigned int linktype);
