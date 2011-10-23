/*
Copyright (C) 2008 Bryan Hoover (bhoover@wecs.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include "errorcode.h"

RC_TYPE init_lang_strings(FILE *pLOG_FILE,char *szLang);
void dealloc_lang_strings();
char *langStr(char *szDest,char *szDefault,int buff_size);
char *str_buff_safe(char *s,unsigned int buff_size);
RC_TYPE re_init_lang_strings(char *lang_file_and_path);
