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

	History:
		- February 2008 - path, file utility routines
		= June/July 2009 - abstracted out program meta
		file search routine and related changes from
		language file search routine and related.
		fixed possible path truncation in cross_platform_cwd
		routine.
  
*/

#define MODULE_TAG "PATH: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>

#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <wctype.h>
#include "path.h"
#include "unicode_util.h"
#include "os.h"
#include "debug_if.h"
#include "safe_mem.h"


char *str_to_lwr(char *s)
{

	return utf_8_strtolwr(s);
}

char *cross_platform_cwd(char **dir)
{

	#define	 MAX_CWD	4096

#ifdef _WIN32
	#define	BUFF_INC	128

	unsigned int		buff_size=128;
#else
	#define BUFF_INC	128*UTF_8_SIZE

	unsigned int		buff_size=128*UTF_8_SIZE;
#endif

#ifndef _WIN32

	*dir=safe_malloc(buff_size);


	while (!(getcwd(*dir,buff_size))) {

		buff_size+=BUFF_INC;

		free(*dir);

		if (buff_size>MAX_CWD) {

			*dir=NULL;

			return NULL;
		}

		*dir=safe_malloc(buff_size);
	}
#else

	if (!(isWinNT()))

  #ifndef UNICOWS

	{

		*dir=safe_malloc(buff_size+1);


		while (!(_getcwd(*dir,buff_size))) {

			buff_size+=BUFF_INC;

			free(*dir);

			if (buff_size>MAX_CWD) {

				*dir=NULL;

				return NULL;
			}

			*dir=safe_malloc(buff_size+1);
		}

		_strlwr(*dir);
	}

  #else

	;

  #endif

  #ifndef UNICOWS

	else

  #endif

	{

		wchar_t	*utf_16;

		utf_16=safe_malloc(buff_size*sizeof(wchar_t)+sizeof(wchar_t));


		while (!(_wgetcwd(utf_16,buff_size))) {

			free(utf_16);

			buff_size+=BUFF_INC;

			if (buff_size>MAX_CWD) {

				*dir=NULL;

				return NULL;
			}

			utf_16=safe_malloc(buff_size*sizeof(wchar_t)+sizeof(wchar_t));
		}


		utf_16_to_8(utf_malloc_16_to_8(dir,utf_16),_wcslwr(utf_16));


		free(utf_16);
	}

#endif 


	return *dir;
}

#ifdef _WIN32

char *nt_console_name2(char **destName,char *srcName)
{

	int		buff_len;
	int		alloc_ret;


	buff_len=utf_8_len(srcName)+1; /*add 1 for appended dir separater*/


	if (!(isWinNT())) {

#ifndef UNICOWS

		while (1) {

			*destName=safe_malloc(buff_len+1);


			alloc_ret=GetShortPathName(srcName,*destName,buff_len+1);

			if (!(alloc_ret)) {

				strcpy(*destName,"");

				break;

			}
			else

				if (alloc_ret<=buff_len)

					break;

				else {

					buff_len=alloc_ret;

					free(*destName);
				}
		}
	}

#else

		;
	}

#endif

#ifndef UNICOWS

	else

#endif

	{
		wchar_t *utf_16_src;
		wchar_t *utf_16_dest;
		char	*thisDest=NULL;


		utf_8_to_16(utf_malloc_8_to_16(&utf_16_src,srcName),srcName);

		SetLastError(0);


		while(1) {

			alloc_ret=buff_len*2+sizeof(wchar_t);

			*destName=safe_malloc(alloc_ret);

			utf_16_dest=safe_malloc(alloc_ret);

			memset(utf_16_dest,0,alloc_ret);


			alloc_ret=GetShortPathNameW(utf_16_src,utf_16_dest,buff_len+1);


			if (!(alloc_ret)) {

				strcpy(*destName,"");

				break;
			}
			else

				if (alloc_ret<=buff_len) {

					strcpy(*destName,utf_16_to_8(utf_malloc_16_to_8(&thisDest,utf_16_dest),utf_16_dest));

					free(thisDest);

					break;

				}
				else {

					buff_len=alloc_ret;

					free(*destName);
					free(utf_16_dest);
				}
		}

		free(utf_16_src);
		free(utf_16_dest);
	}


	return *destName;
}

char *nt_console_name(char **destName,char *path,char *srcName)
{

	char	*tmp_buff;



	tmp_buff=safe_malloc(strlen(path)+strlen(srcName)+2);


	nt_console_name2(destName,strcat(strcat(strcpy(tmp_buff,path),"\\"),srcName));



	free(tmp_buff);


	return *destName;
}

#endif

int create_file2(char *fullPath)
{

	FILE *f=NULL;


	f=utf_fopen(fullPath,"w");

	if (!(f))

		return 0;
	else
		
		fclose(f);


	return 1;
}

int create_file(char *path,char *srcName)
{

	char	*tmp_buff;
	int	ret_val;



	tmp_buff=safe_malloc(strlen(path)+strlen(srcName)+2);


	ret_val=create_file2(strcat(strcat(strcpy(tmp_buff,path),DIR_DELIM_STR),srcName));



	free(tmp_buff);


	return ret_val;
}

/*
returns start position of subS, within s, or NULL if subS is not within s
*/
unsigned int subStr(char *s,char *subS)
{

	char			*p_s=s;
	unsigned int	len_subS;


	if (!(s && subS))

		return 0;

	len_subS=strlen(subS);

	while(1) {

		if ((strlen(p_s)<len_subS))

			return 0;

		else

			if (strncmp(p_s,subS,len_subS))

				p_s++;

			else

				return p_s-s+1;

	}
}

char *cat_root(char *root,char *ancestor)
{

	if (!(strcmp(root+strlen(root)-1,DIR_DELIM_STR)))

		strcat(root,ancestor);

	else

		strcat(strcat(root,DIR_DELIM_STR),ancestor);


	return root;
}

char *path_ancestor(char *a,char *path,int numAnc)
{

	int		i;
	char	*path_buff;
	char	*p_match;


	if (strlen(path)<=ROOT_LEN)

		return NULL;


	path_buff=safe_malloc(strlen(path)+1);

	strcpy(path_buff,path);


	for (i=0;i<numAnc;i++) {  /*back up numAnc number of parents*/

		p_match=strrchr(path_buff,DIR_DELIM);


		if (p_match)

			path_buff[(p_match-path_buff)]=0;
	}

	if (p_match)

		strcpy(a,path_buff);

	else

		a=NULL;


	free(path_buff);


	return a;
}

#ifdef _WIN32

int is_file_name(char *s)
{


	WIN32_FIND_DATA	FindFileData;
	char			*dest;
	int			retVal=0;


	memset(&FindFileData,0,sizeof(WIN32_FIND_DATA));

	dest=safe_malloc(strlen(s)+1);



	retVal=(!(FindFirstFile(nt_console_name2(&dest,s),&FindFileData)==INVALID_HANDLE_VALUE));


	free(dest);


	if (retVal)

		return (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));


	return retVal;
}


char *modulePath(FILE *pLOG_FILE,char **dir)
{

	unsigned int	path_len=128;


	if (!(isWinNT()))

#ifndef UNICOWS

		while (1) {

			*dir=safe_malloc(path_len+1);


			if (!(GetModuleFileName(NULL,*dir,path_len))) {

				free(*dir);

				return NULL;
			}

			if (is_file_name(*dir))

				break;


			free(*dir);

			path_len+=128;
		}
#else

		;
#endif

#ifndef UNICOWS

	else

#endif

	{
		char *utf_8;

		path_len=256;


		while (1) {

			*dir=safe_malloc(path_len+sizeof(wchar_t));

			memset(*dir,0,path_len);


			if (!(GetModuleFileNameW(NULL,(wchar_t *) *dir,path_len/2))) {

				free(*dir);

				return NULL;
			}


			utf_16_to_8(utf_malloc_16_to_8(&utf_8,(wchar_t *) *dir),(wchar_t *) *dir);

			free(*dir);

			*dir=safe_malloc(strlen(utf_8)+1);

			strcpy(*dir,utf_8);

			free(utf_8);


			if (is_file_name(*dir))

				break;


			free(*dir);

			path_len+=256;
		}
	}


	path_ancestor(*dir,*dir,1);


	return *dir;
}

#endif

int is_file(char *name)
{

	FILE		*p_f=NULL;


#ifdef _WIN32

	return is_file_name(name);

#else

	if (!(p_f=fopen(name,"r")))

		return 0;


	fclose(p_f);


	return 1;

#endif

}

char *get_file(char *path,char *fileName)
{
	/*
		Allocate, check exist, and return file name, path pointer.
	*/

	char *a_path=NULL;	


	a_path=safe_malloc(strlen(path)+strlen(fileName)+1);

	if (!(is_file(strcat(strcpy(a_path,path),fileName)))) {


		free(a_path); a_path=NULL;	
	}


	return a_path;
}

char *progMetaFile(FILE *pLOG_FILE,char **dest,char *path,int path_len,char *fileName,char *root_dir,char *in_dir)
{

	/*look for file in path, starting from current directory, and then backing up to program root, root_dir

	  return full path if we find it

	  otherwise reuturn NULL
	*/

#ifdef _WIN32

	int		is_wide=0;
	wchar_t	*utf_16=NULL;
#endif

	char	*relative_lang=NULL;
	char	*prog_root=NULL;
	char	*dir=NULL;
	FILE    *fp=NULL;


	if (!(root_dir))

		return NULL;

	
	dir=safe_malloc(strlen(in_dir)+strlen(path)+strlen(fileName)+2); /*add 1 for dir separater*/

	strcpy(dir,in_dir);

	relative_lang=safe_malloc(strlen(path)+strlen(fileName)+1);

	prog_root=safe_malloc(strlen(root_dir)+strlen(path)+strlen(fileName)+2);


	strcpy(relative_lang,path);

#ifdef _WIN32

	str_to_lwr(nt_console_name(&dir,dir,""));
#endif 


	strcat(relative_lang,fileName);


	/*if we're not at or below our supposed root directory (root_dir), return NULL*/

	if (!(subStr(dir,root_dir))) {

		free(dir); free(relative_lang); free(prog_root);

		return NULL;
	}


	DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Attempting open program meta file %s...\n",relative_lang));


	strcpy(prog_root,root_dir);

	cat_root(dir,relative_lang);

	cat_root(prog_root,relative_lang);


	DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Starting search at %s...\n",dir));

	if (pLOG_FILE)

		os_file_printf(pLOG_FILE,"I:" MODULE_TAG "Starting search at %s...\n",dir);


	/*conditional compiles concered with whether linux, or WinNT, or Win32s with or without uncows.lib/dll -- this
	  concerns whether have unicocde path, file name

	  if linux, utf-8, can proceed as normal

	  if windows, set boolean true if win nt, or win32s with unicows, and false if win32s without unicows.  will
	  branch relative to path/file handling based on this bolean.
	*/

#ifndef _WIN32

	while (!(fp=utf_fopen(dir,"r"))) {

#else

	fp=utf_fopen(dir,"r");



	while (!(fp)) {

#endif

		if (!(strcmp(prog_root,dir))) {

			free(dir); free(relative_lang); free(prog_root);

			return NULL;
		}


		path_ancestor(dir,dir,path_len); /*back up to relative_lang's ancestor*/

		/*try relative path/filename from current directory*/
		strcat(strcat(dir,DIR_DELIM_STR),relative_lang);

#ifdef _WIN32

		fp=utf_fopen(dir,"r");

#endif

	}



	fclose(fp);

	DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Found program meta file:  %s...\n",dir));

	if (pLOG_FILE)

		os_file_printf(pLOG_FILE,"I:" MODULE_TAG "Found program meta file:  %s...\n",dir);



	*dest=safe_malloc(strlen(dir)+1);

	strcpy(*dest,dir);


	free(dir); free(relative_lang); free(prog_root);


	return *dest;
}

char *searchedProgFile(FILE *pLOG_FILE,char **dest,char *root,char *path,int path_len,char *langFileName)
{

	/*
		Search for a file from ./, /etc, /usr/local/share /user/local/etc plus 
		*path, up to *root.
	*/

	char	*a_path=NULL;
	char	*cwd=NULL;
	char	*normed_path=NULL;

	*dest=NULL;

	normed_path=safe_malloc(strlen(path)+2);
	strcat(strcpy(normed_path,path),DIR_DELIM_STR);

	cross_platform_cwd(&cwd);

#ifdef _WIN32

	if (!(progMetaFile(pLOG_FILE,dest,normed_path,path_len,langFileName,str_to_lwr(ancestor_path(&a_path,root,cwd)),cwd))) {

		free(a_path); free(cwd); a_path=NULL; cwd=NULL;


		modulePath(pLOG_FILE,&cwd);


		progMetaFile(pLOG_FILE,dest,normed_path,path_len,langFileName,str_to_lwr(ancestor_path(&a_path,root,cwd)),cwd);
	}

#else

	{

		char	*prefixed_path=NULL;


		if (!(progMetaFile(pLOG_FILE,dest,normed_path,path_len,langFileName,ancestor_path(&a_path,root,cwd),cwd))) {


			free(a_path); a_path=NULL;
			

			prefixed_path=safe_malloc(strlen("/etc/")+strlen(root)+strlen(normed_path)+2);
            
			strcat(strcat(strcat(strcpy(prefixed_path,"/etc/"),root),"/"),normed_path);
	/*
		TODO:	should search relative to where executing from (/bin, /usr/local/bin, should map to 
			/etc, /usr/local/etc respectively
	*/

			if (!(*dest=get_file(prefixed_path,langFileName))) {
	
				prefixed_path=safe_realloc(prefixed_path,strlen("/usr/share/")+strlen(root)+strlen(normed_path)+2);

				strcat(strcat(strcat(strcpy(prefixed_path,"/usr/share/"),root),"/"),normed_path);


				if (!(*dest=get_file(prefixed_path,langFileName))) {

					prefixed_path=safe_realloc(prefixed_path,strlen("/usr/local/etc/")+strlen(root)+strlen(normed_path)+2);

					strcat(strcat(strcat(strcpy(prefixed_path,"/usr/local/etc/"),root),"/"),normed_path);


					*dest=get_file(prefixed_path,langFileName);
				}
			}

			free(prefixed_path);
		}	
	}

#endif

	free(cwd);
	free(a_path);
	free(normed_path);

	if (*dest)
		
		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Found program meta file:  %s...\n",*dest));

	return *dest;
}

char *ancestor_path(char **a_path,char *ancestor,char *path)
{

	char		*a_path_buff;
	char		*p_a_path_buff;

#ifdef _WIN32

	char		*nt_cons_name;
#endif

	int			anc_len=strlen(ancestor);


	if (!(path))

		return NULL;


	/*check for root*/

	if (anc_len==ROOT_LEN) {

#ifndef _WIN32

		if (!(strstr(path,ancestor)-path))

#else

		if (!(strstr(str_to_lwr(path),str_to_lwr(ancestor))-path))

#endif

			return ancestor;

		else

			return NULL;
	}

	a_path_buff=safe_malloc(strlen(path)+1);

	strcpy(a_path_buff,path);

	p_a_path_buff=a_path_buff;


	while (1) {

#ifdef _WIN32


		if (strlen(nt_console_name(&nt_cons_name,a_path_buff,ancestor))) {

			strcpy(a_path_buff,nt_cons_name);

			break;
		}

#else

		{
			char	*delim_match;


			delim_match=strrchr(a_path_buff,DIR_DELIM);

			if (subStr(delim_match,ancestor))

				break;
		}

#endif

		p_a_path_buff=path_ancestor(a_path_buff,a_path_buff,1);


		if (!(p_a_path_buff))

			break;
	}

	if (!(p_a_path_buff))

		*a_path=NULL;

	else {

		*a_path=safe_malloc(strlen(p_a_path_buff)+1);

		strcpy(*a_path,p_a_path_buff);
	}


	free(a_path_buff);


#ifdef _WIN32

	free(nt_cons_name);

#endif

	return *a_path;
}
