/*
 *  Copyright (C) 2004-06 Luca Deri <deri@ntop.org>
 *
 *  		          http://www.ntop.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "nprobe.h"

#ifdef HAVE_MYSQL

extern V9TemplateId *v9TemplateList[TEMPLATE_LIST_LEN];
static MYSQL mysql;
u_char db_initialized = 0;

/* If you need to add a key to the table
   then add the the V9 name of the field
   to the array below 
*/
static char *db_keys[] = {
  "FIRST_SWITCHED",
  "LAST_SWITCHED",
  "IPV4_SRC_ADDR",
  "IPV4_DST_ADDR",
  "L4_SRC_PORT",
  "L4_DST_PORT",
  NULL
};

/* ***************************************************** */

int exec_sql_query(char *sql, u_char dump_error_if_any) {
  //traceEvent(TRACE_ERROR, "====> %s", sql);

  if(!db_initialized) {
    static char shown_msg = 0;

    if(!shown_msg) {
      traceEvent(TRACE_ERROR, "MySQL error: DB not yet initialized");
      traceEvent(TRACE_ERROR, "Please use the %s command line option", MYSQL_OPT);
      shown_msg = 1;
    }
    return(-2);
  }

  if(mysql_query(&mysql, sql)) {
    if(dump_error_if_any)
      traceEvent(TRACE_ERROR, "MySQL error: %s", mysql_error(&mysql));
    return(-1);
  } else
    return(0);
}

/* ***************************************************** */

char* get_last_db_error() {
  return((char*)mysql_error(&mysql));
}

/* ***************************************************** */

int init_database(char *db_host, char* user, char *pw, char *db_name) {
  char sql[2048];

  db_initialized = 0;

  if(mysql_init(&mysql) == NULL) {
    traceEvent(TRACE_ERROR, "Failed to initialize MySQL connection");
    return(-1);
  } else
    traceEvent(TRACE_INFO, "MySQL initialized");

  if(!mysql_real_connect(&mysql, db_host, user, pw, NULL, 0, NULL, 0)){
    traceEvent(TRACE_ERROR, "Failed to connect to MySQL: %s [%s:%s:%s:%s]\n",
	       mysql_error(&mysql), db_host, user, pw, db_name);
    return(-2);
  } else
    traceEvent(TRACE_INFO, "Succesfully connected to MySQL [%s:%s:%s:%s]",
	       db_host, user, pw, db_name);

  db_initialized = 1;

  /* *************************************** */

  snprintf(sql, sizeof(sql), "CREATE DATABASE IF NOT EXISTS %s", db_name);
  if(exec_sql_query(sql, 0) != 0) {
    traceEvent(TRACE_ERROR, "MySQL error: %s\n", get_last_db_error());
    return(-3);
  }

  if(mysql_select_db(&mysql, db_name)) {
    traceEvent(TRACE_ERROR, "MySQL error: %s\n", get_last_db_error());
    return(-4);
  }

  /* *************************************** */

  /* NetFlow */
  snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS `flows` ("
	   "`idx` int(11) NOT NULL auto_increment,"
	   "UNIQUE KEY `idx` (`idx`)"
	   ") ENGINE=MyISAM DEFAULT CHARSET=latin1");

  if(exec_sql_query(sql, 0) != 0) {
    traceEvent(TRACE_ERROR, "MySQL error: %s\n", get_last_db_error());
    return(-5);
  }

  return(0);
}

/* ************************************************ */

int init_db_table() {
  char sql[2048];
  int i, j;

  traceEvent(TRACE_INFO, "Scanning templates");

  for(i=0; i<TEMPLATE_LIST_LEN; i++) {
    if(v9TemplateList[i] != NULL) {
#ifdef DEBUG
	 traceEvent(TRACE_INFO, "Found [%20s][%d bytes]",
	 v9TemplateList[i]->templateName, v9TemplateList[i]->templateLen);
#endif

      if(v9TemplateList[i]->templateLen <= 4) {
	char *sql_type;

	if(v9TemplateList[i]->templateLen <= 1)
	  sql_type = "tinyint(4)";
	else if(v9TemplateList[i]->templateLen <= 2)
	  sql_type = "mediumint(6)";
	else if(v9TemplateList[i]->templateLen <= 4)
	  sql_type = "bigint(20)";

	snprintf(sql, sizeof(sql), "ALTER TABLE `flows` ADD `%s` %s NOT NULL default '0'",
		 v9TemplateList[i]->templateName, sql_type);
      } else {
	snprintf(sql, sizeof(sql), "ALTER TABLE `flows` ADD `%s` varchar(%d) NOT NULL default ''",
		 v9TemplateList[i]->templateName, 2*v9TemplateList[i]->templateLen);
      }

      if(exec_sql_query(sql, 0) != 0) {
#ifdef DEBUG
	traceEvent(TRACE_ERROR, "MySQL error: %s\n", get_last_db_error());
#endif
      } else {
	for(j=0; db_keys[j] != NULL; j++)
	  if(!strcmp(v9TemplateList[i]->templateName, db_keys[j])) {
	    snprintf(sql, sizeof(sql), "ALTER TABLE `flows` ADD INDEX (`%s`)",
		     v9TemplateList[i]->templateName);
	    
	    if(exec_sql_query(sql, 0) != 0) {
#ifdef DEBUG
	      traceEvent(TRACE_ERROR, "MySQL error: %s\n", get_last_db_error());
#endif
	    }
	    break;
	  }
      }
    } else
      break;
  }
}

/* ************************************************ */

void dump_flow2db(char *buffer, u_int32_t buffer_len) {
  char sql_a[2048] = { 0 }, sql_b[2048] = { 0 }, sql[4096] = { 0 }, buf[128];
  int i, pos = 0;

  // traceEvent(TRACE_INFO, "dump_flow2db()");

  strcpy(sql_a, "INSERT INTO `flows` (");
  strcpy(sql_b, "VALUES (");

  for(i=0; i<TEMPLATE_LIST_LEN; i++) {
    if(v9TemplateList[i] != NULL) {
#ifdef DEBUG
      traceEvent(TRACE_INFO, "Found [%20s][%d bytes]",
		 v9TemplateList[i]->templateName, v9TemplateList[i]->templateLen);
#endif

      if(i > 0) {
	strcat(sql_a, ", ");
	strcat(sql_b, ", ");
      }

      buf[0] = '\0';
      strcat(sql_a, v9TemplateList[i]->templateName);

      if(v9TemplateList[i]->templateLen <= 4) {
	u_int8_t a = 0, b = 0, c = 0, d = 0;
	u_int32_t val;
	char *sql_type;

	if(v9TemplateList[i]->templateLen == 1) {
	  sql_type = "tinyint(4)";
	  d = buffer[pos];
	  pos += 1;
	} else if(v9TemplateList[i]->templateLen == 2) {
	  sql_type = "mediumint(6)";
	  c = buffer[pos], d = buffer[pos+1];
	  pos += 2;
	} else if(v9TemplateList[i]->templateLen == 3) {
	  sql_type = "mediumint(6)";
	  b = buffer[pos], c = buffer[pos+1], d = buffer[pos+2];
	  pos += 3;
	} else if(v9TemplateList[i]->templateLen == 4) {
	  sql_type = "bigint(20)";
	  a = buffer[pos], b = buffer[pos+1], c = buffer[pos+2], d = buffer[pos+3];
	  pos += 4;
	}

	a &= 0xFF, b &= 0xFF, c &= 0xFF, d &= 0xFF;
	val =  (a << 24) + (b << 16) + (c << 8) + d;

	snprintf(buf, sizeof(buf), "'%u'", val );	

	/*
	  snprintf(sql, sizeof(sql), "ALTER TABLE `flows` ADD `%s` varchar(%d) NOT NULL default ''",
	  v9TemplateList[i]->templateName, v9TemplateList[i]->templateLen);
	*/

	// traceEvent(TRACE_INFO, "%X", val);
      } else {
	int len = 0, k, j;
	
	buf[0] = '\'';
	buf[1] = '\0';
	
	for(k = 0, j = 1; k<v9TemplateList[i]->templateLen; k++) {
	  snprintf(&buf[j], sizeof(buf)-j, "%02X", buffer[k]);
	  j += 2;
	}
	
	buf[j] = '\'';          
	buf[j+1] = '\0';
      }
      
      strcat(sql_b, buf);
    }
  }
  
  strcat(sql_a, ")");
  strcat(sql_b, ")");

  snprintf(sql, sizeof(sql), "%s %s", sql_a, sql_b);
  //traceEvent(TRACE_INFO, sql);
  exec_sql_query(sql, 1);
}
#endif
