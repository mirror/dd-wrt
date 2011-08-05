/*
 * dlist_test.c
 *
 * Copyright (C) 2003 Eric J Bohm
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */


/* Double linked list implementation tester.
 */
#include "dlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// create some dlists put nodes in and out
// use dump
// try list with del_func and without
// use insert, unshift, push
// use pop push mark
// use prev next

typedef struct simple {
  char label[80];
  int number;
} Simple;


typedef struct complex {
  int cnumber;
  Simple *sthing;
} Complex;

void complex_silly_multiply_by_two( void *a);
int complex_equal(void *a, void *b);
int complex_less(void *a, void *b);
int complex_greater(void *a, void *b);
int complex_comp(void *a, void *b);
void simple_dump(Dlist *);
void simple_dump_rev(Dlist *);
void complex_dump(Dlist *);
void complex_dump_rev(Dlist *);
void complex_out(void *);
void complex_del(void *);
Complex *complex_maker(int,int,char *);
int complex_filter(void *a);

Simple *simple_maker(int ,char *);

int main (int argc,char *argv[])
{
  Dlist *list;
  Simple *s1,*s2,*s3,*stemp;
  Complex *c1,*c2,*c3, *c4, *ctemp, *cfound;
  while(1)
    {
	s1=simple_maker(1,"one");
	s2=simple_maker(2,"two");
	s3=simple_maker(3,"three");
      if((list=dlist_new(sizeof(Simple)))==NULL)
	{
	   fprintf(stderr,"ERR dlist_new fail\n");
	  return(2);
	 }
      dlist_push(list,s1);
      dlist_push(list,s2);
      dlist_push(list,s3);
      printf("count is %ld\n",list->count);
      simple_dump(list);
      simple_dump_rev(list);
      stemp=dlist_pop(list);
      printf("popped %s\n",stemp->label);
      simple_dump(list);
      printf("pushed\n");
      dlist_push(list,s3);
      simple_dump(list);
      stemp=dlist_shift(list);
      printf("shifted %s\n",stemp->label);
      simple_dump(list);
      printf("unshifted\n");
      dlist_unshift(list,stemp);
      simple_dump(list);
      dlist_destroy(list);
      c1=complex_maker(1,1,"one");
      c2=complex_maker(2,2,"two");
      c3=complex_maker(3,3,"three");
      if((list=dlist_new_with_delete(sizeof(Complex),complex_del))==NULL)
	{
		fprintf(stderr,"ERR dlist_new fail\n");
		return(2);
	}
	if(dlist_insert_sorted(list,c1,complex_less)==NULL)
	{
		fprintf(stderr,"ERR dlist_insert fail\n");
		return(2);
	}
	printf("sorted insert 1\n");
	if(dlist_insert_sorted(list,c3,complex_less)==NULL)
	{
		fprintf(stderr,"ERR dlist_insert fail\n");
		return(2);
	}
	if(dlist_insert_sorted(list,c2,complex_less)==NULL)
	{
		fprintf(stderr,"ERR dlist_insert fail\n");
		return(2);
	}
	printf("sorted insert 2\n");
	printf("ascending sorted output\n");
	complex_dump(list);
	dlist_transform(list,complex_silly_multiply_by_two);
	printf("transform multi by 2 output\n");
	complex_dump(list);
	ctemp=complex_maker(6,6,"three");
	if((cfound=(Complex *) dlist_find_custom(list,ctemp,complex_equal))!=NULL)

	{
		printf("found %d as %d in list\n",ctemp->cnumber,cfound->cnumber);
	} else {
		printf("ERROR find failed on %d \n",ctemp->cnumber);
		return(3);
	}
	complex_del(ctemp);
	dlist_destroy(list);
	c1=complex_maker(1,1,"one");
	c2=complex_maker(2,2,"two");
	c3=complex_maker(3,3,"three");
      if((list=dlist_new_with_delete(sizeof(Complex),complex_del))==NULL)
	{
	   fprintf(stderr,"ERR dlist_new fail\n");
	  return(2);
	 }
	if(dlist_insert_sorted(list,c1,complex_greater)==NULL)
	{
	  fprintf(stderr,"ERR dlist_insert fail\n");
	  return(2);
	}
	printf("greater sorted insert 1\n");
	if(dlist_insert_sorted(list,c3,complex_greater)==NULL)
	{
	   fprintf(stderr,"ERR dlist_insert fail\n");
	   return(2);
	 }
	printf("greater sorted insert 3\n");
	if(dlist_insert_sorted(list,c2,complex_greater)==NULL)
	{
	   fprintf(stderr,"ERR dlist_insert fail\n");
	   return(2);
	 }
	printf("greater sorted insert 2\n");
	printf("descending sorted output using transform\n");
	dlist_transform(list,complex_out);
      dlist_destroy(list);
      c1=complex_maker(1,1,"one");
      c2=complex_maker(2,2,"two");
      c3=complex_maker(3,3,"three");
      c4=complex_maker(4,4,"four");
      if((list=dlist_new_with_delete(sizeof(Complex),complex_del))==NULL)
	{
	  fprintf(stderr,"ERR dlist_new fail\n");
	  return(2);
	}
      dlist_push(list,c2);
      dlist_push(list,c1);
      dlist_push(list,c4);
      dlist_push(list,c3);
      printf("unsorted custom\n");
      complex_dump(list);
      printf("unsorted custom reversed\n");
      complex_dump_rev(list);
      dlist_sort_custom(list,complex_comp);
      printf("custom sorted output\n");
      complex_dump(list);

	dlist_filter_sort(list, complex_filter, complex_comp);
	printf("custom filtered and sorted output\n");
	complex_dump(list);
      dlist_destroy(list);
    }
  return(0);
}

void simple_dump (Dlist *list)
{
  Simple *thisone;
  printf("count  %ld \n",list->count);
  dlist_for_each_data(list,thisone,Simple)
    {
      printf("label %s number %d \n",thisone->label,thisone->number);
    }

}
void simple_dump_rev (Dlist *list)
{
  Simple *thisone;
  printf("rev count  %ld \n",list->count);
  dlist_for_each_data_rev(list,thisone,Simple)
    {
      printf("label %s number %d \n",thisone->label,thisone->number);
    }
}

Simple * simple_maker(int snumber,char *label)
{
	Simple *stemp;
	if((stemp=malloc(sizeof(Simple)))==NULL)
	{
		fprintf(stderr,"ERR malloc fail\n");
		return(NULL);
	}
	stemp->number=snumber;
	strcpy(stemp->label,label);
	return(stemp);
}

Complex * complex_maker(int cnumber, int snumber, char* label)
{
	Complex *ctemp;
	if((ctemp=malloc(sizeof(Complex)))==NULL)
	{
		fprintf(stderr,"ERR malloc fail\n");
		return(NULL);
	}
	ctemp->cnumber=cnumber;
	ctemp->sthing=simple_maker(snumber,label);
	return(ctemp);
}

void complex_out(void *data)
{
	Complex *thisone=(Complex *)data;
	printf("cnumber %d label %s number %d \n",thisone->cnumber,thisone->sthing->label,thisone->sthing->number);
}

/**
 * return 1 if a==b, else 0
 */
int complex_equal(void *a, void *b)
{
	if((((Complex *)a)->cnumber==((Complex *)b)->cnumber)
	    && (((Complex *)a)->sthing->number==
		((Complex *)b)->sthing->number)
	    &&  strcmp(((Complex *)a)->sthing->label,
		((Complex *)b)->sthing->label)==0)
		return(1);
	return(0);
}

/** for sorting
 * return 1 if a<b, else 0
 */
int complex_less(void *a, void *b)
{
	return( ((Complex *)a)->cnumber <  ((Complex *)b)->cnumber );
}

/** for sorting
 * return 1 if a>b, else 0
 */
int complex_greater(void *a, void *b)
{
	return( ((Complex *)a)->cnumber >  ((Complex *)b)->cnumber );
}

int complex_comp(void *a, void *b)
{
	return( ((Complex *)a)->cnumber -  ((Complex *)b)->cnumber );
}

void complex_silly_multiply_by_two( void *a)
{
	((Complex *)a)->cnumber=((Complex *)a)->cnumber*2;
	((Complex *)a)->sthing->number=((Complex *)a)->sthing->number*2;
}

void complex_dump (Dlist *list)
{
  Complex *thisone;
  dlist_start(list);
  printf("count  %ld \n",list->count);
  dlist_for_each_data(list,thisone,Complex)
    {
      printf("cnumber %d label %s number %d \n",thisone->cnumber,thisone->sthing->label,thisone->sthing->number);
    }

}

void complex_dump_rev (Dlist *list)
{
  Complex *thisone;
  dlist_start(list);
  printf("count  %ld \n",list->count);
  dlist_for_each_data_rev(list,thisone,Complex)
    {
      printf("cnumber %d label %s number %d \n",thisone->cnumber,thisone->sthing->label,thisone->sthing->number);
    }

}

void complex_del (void *item)
{
  Complex *corpse=item;
  printf("freeing complex\n");
  free(corpse->sthing);
  free(corpse);
}

int complex_filter(void *a)
{
	Complex *c = (Complex *)a;

	if (c->cnumber >= 2 && c->cnumber <= 3)
		return 1;
	else
		return 0;
}
