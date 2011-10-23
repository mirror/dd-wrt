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
		February 2008 - unicode, language strings parsing, handling related
		June/July 2009 - adapted searchedLangFile call to searchedProgFile.
  
*/

#define MODULE_TAG "LANG: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include "errorcode.h"
#include <stdlib.h>

#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <wctype.h>
#include "lang.h"
#include "path.h"
#include "unicode_util.h"
#include "dyndns.h"
#include "debug_if.h"
#include "dblhash.h"
#include "safe_mem.h"


typedef struct LANG_STR {

	char	*lang_str_ndx;
	char	*lang_str;

}LANG_STR;

typedef struct LANG_STR_OBJECT {

	LANG_STR	**p_ls;

	int			size;

}LANG_STR_OBJECT;

static LANG_STR_OBJECT *global_lang_str=NULL;

DblHashTab	*global_lang_str_hash=NULL;


static RC_TYPE push_in_buffer(char* p_src, int src_len, char *p_buffer, int* p_act_len, int max_len)
{
	if (*p_act_len + src_len > max_len)
	{
		return RC_FILE_IO_OUT_OF_BUFFER;
	}
	memcpy(p_buffer + *p_act_len,p_src, src_len);
	*p_act_len += src_len;
	return RC_OK;
}

typedef enum
{
    NEW_LINE,
    COMMENT,
    DATA,
    ESCAPE,
    QUOTE_END,
    QUOTE_CONTINUE,
    QUOTE_CONTINUED,
    TRANSLATION_STR

} PARSER_STATE;

typedef struct
{
	FILE *p_file;
	PARSER_STATE state;
} OPTION_FILE_PARSER;

static RC_TYPE parser_init(OPTION_FILE_PARSER *p_cfg, FILE *p_file)
{
	memset(p_cfg, 0, sizeof(*p_cfg));
	p_cfg->state = NEW_LINE;
	p_cfg->p_file = p_file;
	return RC_OK;
}

static RC_TYPE do_parser_read_option(OPTION_FILE_PARSER *p_cfg,
                                     char *p_buffer,
                                     int maxlen,
                                     char *ch,
                                     int *count,
                                     int *parse_end)
{

	RC_TYPE rc = RC_OK;


	switch (p_cfg->state)
	{
	case NEW_LINE:

		if (!(strcmp(ch,":")))
		{

			p_cfg->state=TRANSLATION_STR;

			return rc;
		}

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{
			return rc;
		}

		if (!(strcmp(ch,"\\")))
		{
			p_cfg->state = ESCAPE;

			return rc;
		}

		if (!(strcmp(ch,"#"))) /*comment*/
		{
			p_cfg->state = COMMENT;

			return rc;
		}

		if ((strcmp(ch," ")) && (strcmp(ch,"	")))
		{
			if (strcmp(ch,"\""))
			{

				return RC_CMD_PARSER_INVALID_OPTION;
			}

			p_cfg->state = DATA;

			return rc;
		}

		/*skip actual leading  spaces*/
		return rc;

	case TRANSLATION_STR:

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{
			return rc;
		}

		if ((strcmp(ch," ")) && (strcmp(ch,"	")))
		{
			if (strcmp(ch,"\""))
			{

				return RC_CMD_PARSER_INVALID_OPTION;
			}

			/*signal translation string*/
			push_in_buffer(":", 1, p_buffer, count, maxlen);

			*parse_end=1;

			p_cfg->state = DATA;

			return rc;
		}

		/*skip actual leading  spaces*/
		return rc;

	case COMMENT:

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{
			p_cfg->state = NEW_LINE;
		}

		/*skip comments*/
		return rc;

	case QUOTE_CONTINUED:

		if (!(strcmp(ch,"\"")))
		{

			p_cfg->state=DATA;

			return rc;
		}

		if (!(strcmp(ch," ")) || !(strcmp(ch,"	")))
		{

			return rc;
		}

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{

			return rc;
		}

		if (!(strcmp(ch,"\\")))
		{

			return rc;
		}


		return RC_CMD_PARSER_INVALID_OPTION;


	case QUOTE_CONTINUE:

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{
			p_cfg->state = QUOTE_CONTINUED;

			return rc;
		}

		if (!(strcmp(ch," ")) || !(strcmp(ch,"	")))
		{

			return rc;
		}

		if (!(strcmp(ch,"\"")))
		{

			p_cfg->state = DATA;

			return rc;
		}

		if (!(strcmp(ch,"\\")))
		{

			return rc;
		}

		return RC_CMD_PARSER_INVALID_OPTION;

	case QUOTE_END:

		if (!(strcmp(ch,":")))
		{

			p_cfg->state=TRANSLATION_STR;

			*parse_end=1;

			return rc;
		}


		if (!(strcmp(ch,"\\")))
		{

			p_cfg->state=QUOTE_CONTINUE;

			return rc;
		}

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{

			p_cfg->state=NEW_LINE;

			*parse_end=1;

			return rc;
		}

		if ((strcmp(ch," ")) && (strcmp(ch,"	")))
		{

			return RC_CMD_PARSER_INVALID_OPTION;
		}

		return rc;


	case ESCAPE:


		if (!(strcmp(ch,"n")))
		{

			rc = push_in_buffer("\n", strlen("\n"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"r")))
		{

			rc = push_in_buffer("\r", strlen("\r"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"t")))
		{

			rc = push_in_buffer("\t", strlen("\t"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"b")))
		{

			rc = push_in_buffer("\b", strlen("\b"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"f")))
		{

			rc = push_in_buffer("\f", strlen("\f"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"a")))
		{

			rc = push_in_buffer("\a", strlen("\a"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"\\")))
		{

			rc = push_in_buffer("\\", strlen("\\"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"?")))
		{

			rc = push_in_buffer("\?", strlen("\?"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"'")))
		{

			rc = push_in_buffer("\'", strlen("\'"), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"\"")))
		{

			rc = push_in_buffer("\"", strlen("\""), p_buffer, count, maxlen);
		}

		if (!(strcmp(ch,"0")))
		{

			rc = push_in_buffer("\0", strlen("\0"), p_buffer, count, maxlen);
		}

		p_cfg->state = DATA;

		return rc;

	case DATA:

		if (!(strcmp(ch,"\\")))
		{

			p_cfg->state = ESCAPE;

			return rc;
		}

		if (!(strcmp(ch,"\"")))
		{

			p_cfg->state = QUOTE_END;

			return rc;
		}


		/*actual data*/

		return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);

	default:

		return rc;
	}
}

static RC_TYPE parser_utf_8_read_option(OPTION_FILE_PARSER *p_cfg, char *p_buffer, int maxlen)
{
	RC_TYPE	rc = RC_OK;
	int		parse_end = 0;
	int		count = 0;
	char	c_buff[7];


	*p_buffer = 0;


	while(!parse_end)
	{

		memset(c_buff,0,7);

		if (!(utf_read_utf_8(c_buff,p_cfg->p_file)))
		{
			if (feof(p_cfg->p_file))
			{
				break;
			}

			rc = RC_FILE_IO_READ_ERROR;

			break;
		}

		rc=do_parser_read_option(p_cfg,p_buffer,maxlen,c_buff,&count,&parse_end);

		if (!(rc == RC_OK))

			return rc;
	}


	rc = push_in_buffer("\0",1,p_buffer,&count,maxlen);


	return rc;
}

static char *searchedLangFile(FILE *pLOG_FILE,char **dest,char *langFileName)
{ 
	char *res = searchedProgFile(pLOG_FILE,dest,"inadyn-mt","lang",3,langFileName);
	if (!res)
	  return searchedProgFile(pLOG_FILE,dest,"inadyn-mt", "lang",3,DYNDNS_DEFAULT_LANG_FILE);
	else
	  return res;
}

char *lang_code(char *szCode,char *szLocale)
{

	char	tmp_buff[512];
	char	*under_score;


	if (!(szLocale))

		return strcpy(szCode,"en");


	if (!(under_score=strchr(szLocale,'_')))

		return strcpy(szCode,"en");



	memset(tmp_buff,0,512);


	return str_to_lwr(strncpy(szCode,strncpy(tmp_buff,szLocale,under_score-szLocale),2));
}

RC_TYPE do_init_lang_strings(FILE *pLOG_FILE,char *lang_file_and_path)
{

	RC_TYPE				rc = RC_OK;
	FILE				*p_file=NULL;
	char				*p_tmp_buffer=NULL;
	const				int buffer_size=512*UTF_8_SIZE;
	OPTION_FILE_PARSER	parser;
	LANG_STR			**p_langArray=NULL;
	int					is_translation_str=0;
	int					is_duplicate=0;


	do
	{
		p_tmp_buffer = safe_malloc(buffer_size);
		if (!p_tmp_buffer)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}


		if (lang_file_and_path) 

			p_file = utf_fopen(lang_file_and_path, "r");


		if (!(p_file))

		{
			DBG_PRINTF((LOG_ERR,"W:" MODULE_TAG "Cannot open language file.  Will use english defaults, or default override (--lang_file <path/file_name>...)\n"));
			rc = RC_FILE_IO_OPEN_ERROR;
			break;
		}

		if (pLOG_FILE)

			os_file_printf(pLOG_FILE,"I:" MODULE_TAG "Openned language file, %s...\n",lang_file_and_path);


		if (utf_is_bom_8(p_file))

			fseek(p_file,3,0);


		if ((rc = parser_init(&parser, p_file)) != RC_OK)
		{
			break;
		}

		if (!(feof(p_file))) {

			global_lang_str=(LANG_STR_OBJECT *) safe_malloc(sizeof(LANG_STR_OBJECT));

			global_lang_str->p_ls=NULL;

			global_lang_str->size=0;

			createTable(&global_lang_str_hash);
		}


		/*	read default string (index), translation
			string pairs, one at a time -- a ':' token 
			signals next string is translation string
		*/

		while(!feof(p_file))
		{
			rc = parser_utf_8_read_option(&parser,p_tmp_buffer, buffer_size);

			if (rc != RC_OK)
			{
				dealloc_lang_strings();

				DBG_PRINTF((LOG_ERR,"W:" MODULE_TAG "Error parsing language file.  Will use english defaults, or default override (--lang_file <path/file_name>)...\n"));

				break;
			}


			if (!(strcmp(":",p_tmp_buffer)))

				is_translation_str=1;

			else {

				if (is_translation_str && !(is_duplicate)) {

					if (strlen(p_tmp_buffer) && global_lang_str->size) {

						free(global_lang_str->p_ls[global_lang_str->size-1]->lang_str);

						global_lang_str->p_ls[global_lang_str->size-1]->lang_str=(char *) safe_malloc(strlen(p_tmp_buffer)+1);

						strcpy(global_lang_str->p_ls[global_lang_str->size-1]->lang_str,p_tmp_buffer);
					}

					is_translation_str=0;
				}
				else {

					if (!strlen(p_tmp_buffer))
					{
						break;
					}

					if (hashTabEntry(p_tmp_buffer,global_lang_str_hash))

						is_duplicate=1;
					else
					{

						is_duplicate=0;

						p_langArray=(LANG_STR **) safe_realloc(global_lang_str->p_ls,(global_lang_str->size+1) *
						                                       sizeof(LANG_STR *));

						global_lang_str->p_ls=p_langArray;

						global_lang_str->p_ls[global_lang_str->size]=(LANG_STR *) safe_malloc(sizeof(LANG_STR));

						global_lang_str->p_ls[global_lang_str->size]->lang_str_ndx=(char *) safe_malloc(strlen(p_tmp_buffer)+1);

						global_lang_str->p_ls[global_lang_str->size]->lang_str=(char *) safe_malloc(strlen(p_tmp_buffer)+1);

						/*	let index consist of default output string (first x number of characters --
							see hashKey()), and lang string follows it in lang file (if translation 
							present, otherwise, use default output string).
						*/

						strcpy(global_lang_str->p_ls[global_lang_str->size]->lang_str_ndx,p_tmp_buffer);

						strcpy(global_lang_str->p_ls[global_lang_str->size]->lang_str,p_tmp_buffer);



						hashTabInsert(newHashEntry(global_lang_str->p_ls[global_lang_str->size],
						                           global_lang_str->p_ls[global_lang_str->size]->lang_str_ndx),
						              global_lang_str_hash);


						global_lang_str->size++;
					}
				}
			}
		}

	}
	while(0);


	if (p_file)
	{
		fclose(p_file);
	}
	if (p_tmp_buffer)
	{
		free(p_tmp_buffer);
	}

	if (global_lang_str && pLOG_FILE)

		os_file_printf(pLOG_FILE,"I:" MODULE_TAG "Took in %d language strings from language file...\n",global_lang_str->size);


	return rc;
}

RC_TYPE init_lang_strings(FILE *pLOG_FILE,char *szLocale)
{
	char				*lang_file_and_path=NULL;
	char				langFile[7];
	RC_TYPE				rc=RC_OK;


	DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Language file search locale:  %s...\n",szLocale));

	memset(langFile,0,7);


	/*pass to langFile function, 2 character language code, with appended, .lng*/

	searchedLangFile(pLOG_FILE,&lang_file_and_path,strcat(lang_code(langFile,szLocale),".lng"));


	rc=do_init_lang_strings(pLOG_FILE,lang_file_and_path);

	free(lang_file_and_path);


	return rc;
}

void dealloc_lang_strings()
{

	int i;


	if (global_lang_str) {

		if (global_lang_str->p_ls) {

			for (i=0;i<global_lang_str->size;i++) {

				free(global_lang_str->p_ls[i]->lang_str_ndx);

				free(global_lang_str->p_ls[i]->lang_str);


				free(global_lang_str->p_ls[i++]);
			}

			free(global_lang_str->p_ls);

			global_lang_str->p_ls=NULL;
		}

		free(global_lang_str);

		global_lang_str=NULL;


		destroyTable(&global_lang_str_hash);
	}
}

RC_TYPE re_init_lang_strings(char *lang_file_and_path)
{

	dealloc_lang_strings();


	return do_init_lang_strings(NULL,lang_file_and_path);
}

/*truncate s, to buff_size if len >= buff_size
*/
char *str_buff_safe(char *s,unsigned int buff_size)
{

	if (!(strlen(s) < buff_size))

		utf_truncate(s,buff_size);


	return s;
}

char *langStr(char *szDest,char *szDefault,int buff_size)
{

	EntryType	hashEntry;


	if (!(global_lang_str))

		strcpy(szDest,str_buff_safe(szDefault,buff_size));

	else {

		hashEntry=hashTabEntry(szDefault,global_lang_str_hash);

		if (hashEntry)

			strcpy(szDest,str_buff_safe((char *) ((LANG_STR *) hashEntry->ClientData)->lang_str,buff_size));

		else

			strcpy(szDest,str_buff_safe(szDefault,buff_size));
	}


	return szDest;
}
