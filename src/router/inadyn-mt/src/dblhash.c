/*
Copyright (C) 2007 Bryan Hoover (bhoover@wecs.com)

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
/*
  Non-extensable double hashing ADT.  Uses a client defined/passed function
  to get the key (number).  That is, hash entries are pointers of type void.
  What the pointers point to is a function of the client's implementation,
  etc.  As such, the mentioned client function is used to dereference, derive
  and return the key.

	Author: Bryan Hoover (bhoover@wecs.com)
	Date:	February 2008
	
	History:
		- April 1994	- first implementation.
		- August 1995	- translated from Pascal to C
		- Feb 2008		- removed delete, rehash, and copy functions
						- removed Win 3.1 GlobalAlloc related calls
						- minor semantic, syntax changes
						- removed client key deref callbacks
*/

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "dblhash.h"
#include "safe_mem.h"

static int hashKey(char *s)
{
	#define			MAX_KEY_LEN 128

	int				i;
	int				sLen;
	int				retVal=0;


	sLen=strlen(s);


	for (i=0;i<sLen && i<MAX_KEY_LEN;i++)

		retVal=(retVal + (unsigned char) s[i]);


	return retVal;
}

static int hash_1(int key)
{

	return (key%TABLE_SIZE);
}

static int hash_2(int key)
{

	/*	must not evaluate to zero
	*/
	return (HASH_2_MOD-(key%HASH_2_MOD));
}

static int find(char *key,DblHashTab *table)
{
	TableEntry	*tab;

	int			secondHash;
	int			attemptNum=0;
	int			curHash;
	unsigned int	key_to_num;


	key_to_num=hashKey(key);

	secondHash=hash_2(key_to_num);

	curHash=hash_1(key_to_num);


	for(;;)
	{

		if(!(tab=table->hgTabEntry[curHash]))

			return 0;

		if(tab->info==empty)     /*not found or new entry*/
		{
			return curHash;
		}
		/*
		  ask this next so we avoid dereferencing an undefined ptr
		*/
		if(tab->info!=legit)		/*deleted and no more room*/
		{
			if(attemptNum<TABLE_SIZE)

				attemptNum++;

			else
			{
				return 0;
			}
		}
		else
			if(!(strcmp(tab->entry->key,key)))   /*duplicate ins or found*/
			{
				return curHash;
			}
			else if(attemptNum<TABLE_SIZE)

				attemptNum++;

			else
			{
				return 0;
			}

		curHash=((attemptNum*secondHash)%TABLE_SIZE);
	}
}

int createTable(DblHashTab **table)
{
	int			i;
	DblHashTab	*thisTable;


	thisTable=safe_malloc(sizeof(DblHashTab));


	for(i=0;i<TABLE_SIZE;i++)
	{

		thisTable->hgTabEntry[i]=safe_malloc(sizeof(TableEntry));


		thisTable->hgTabEntry[i]->info=empty;

		thisTable->hgTabEntry[i]->entry=NULL;
	}


	thisTable->size=0;

	thisTable->numDeleted=0;


	*table=thisTable;


	return 1;
}

void destroyTable(DblHashTab **table)
{
	int			i;
	DblHashTab	*thisTable;


	thisTable=*table;


	for (i=0;i<TABLE_SIZE;i++) {

		if (thisTable->hgTabEntry[i]->entry) {

			free(thisTable->hgTabEntry[i]->entry->key);

			free(thisTable->hgTabEntry[i]->entry);
		}


		free(thisTable->hgTabEntry[i]);
	}


	free(thisTable);

	*table=NULL;
}

EntryType hashTabEntry(char *key,DblHashTab *table)
{
	TableEntry		*tab;
	int               pos;
	EntryType         retVal=NULL;


	pos=find(key,table);

	if(pos)
	{
		tab=table->hgTabEntry[pos];

		if((tab->info==legit))

			retVal=tab->entry;
	}

	return retVal;
}

int hashTabInsert(EntryType entry,DblHashTab *table)
{
	TableEntry		*tab;
	int               tabPos;


	if(table->size==TABLE_SIZE)

		return 0;                   /*error condition*/

	else
	{
		if(!(tabPos=find(entry->key, table)))

			return 0;

		else
		{
			tab=table->hgTabEntry[tabPos];


			if(tab->info==empty)

				table->size++;


			tab->entry=entry;

			tab->info=legit;


			return 1;
		}
	}
}

EntryType newHashEntry(void *clientData,char *key)
{

	EntryType  hashTabEntry;


	hashTabEntry=safe_malloc(sizeof(ClientEntry));

	hashTabEntry->ClientData=clientData;

	hashTabEntry->key=safe_malloc(strlen(key)+1);

	strcpy(hashTabEntry->key,key);


	return hashTabEntry;
}
