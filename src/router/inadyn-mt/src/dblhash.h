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

/*
  Data types for dblhash.cp, a non-extensable double hashing ADT.
*/

#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>

#else

#define HGLOBAL void*

#endif


#define  TABLE_SIZE 1933 
#define  HASH_2_MOD 1931 
#define  LAZY_LIM   .9		/*rehash at 90% deletion (for small tables)*/

#define	EntryHandle	HGLOBAL
#define	EntryType	ClientEntry*

enum EntryKind
{
    legit,
    empty,
    deleted
};

typedef struct ClientEntry
{
	void		*ClientData;

	char		*key;

}ClientEntry;

typedef struct TableEntry
{
	EntryType          entry;

	enum EntryKind     info;

}TableEntry;

typedef struct DblHashTab
{
	TableEntry        *hgTabEntry[TABLE_SIZE];

	float             size;
	float             numDeleted;

}DblHashTab;

#define HashTable HGLOBAL

int createTable(struct DblHashTab **table);
int hashTabInsert(EntryType entry,DblHashTab *table);
EntryType hashTabEntry(char *key,DblHashTab *table);
void destroyTable(DblHashTab **table);
EntryType newHashEntry(void *clientData,char *key);
void dumpTable(DblHashTab *table,char *fileName);
