
/*
 * HTTP Info plugin for the olsr.org OLSR daemon
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: admin_interface.c,v 1.6 2005/05/29 12:47:41 br1 Exp $
 */

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */


#include "olsrd_httpinfo.h"
#include "olsr_cfg.h"
#include "admin_html.h"
#include "admin_interface.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
build_admin_body(char *buf, olsr_u32_t bufsize)
{
  int size = 0, i = 0;

  while(admin_frame[i] && strcmp(admin_frame[i], "<!-- BASICSETTINGS -->\n"))
    {
      size += sprintf(&buf[size], admin_frame[i]);
      i++;
    }
  
  if(!admin_frame[i])
    return size;


  size += sprintf(&buf[size], "<tr>\n");

  size += sprintf(&buf[size], admin_basic_setting_int,
		  "Debug level:", "debug_level", 2, olsr_cnf->debug_level);
  size += sprintf(&buf[size], admin_basic_setting_float,
		  "Pollrate:", "pollrate", 4, olsr_cnf->pollrate);
  size += sprintf(&buf[size], admin_basic_setting_string,
		  "TOS:", "tos", 6, "TBD");

  size += sprintf(&buf[size], "</tr>\n");
  size += sprintf(&buf[size], "<tr>\n");

  size += sprintf(&buf[size], admin_basic_setting_int,
		  "TC redundancy:", "tc_redundancy", 1, olsr_cnf->tc_redundancy);
  size += sprintf(&buf[size], admin_basic_setting_int,
		  "MPR coverage:", "mpr_coverage", 1, olsr_cnf->mpr_coverage);
  size += sprintf(&buf[size], admin_basic_setting_int,
		  "Willingness:", "willingness", 1, olsr_cnf->willingness);

  size += sprintf(&buf[size], "</tr>\n");
  size += sprintf(&buf[size], "<tr>\n");

  if(olsr_cnf->use_hysteresis)
    {
      size += sprintf(&buf[size], admin_basic_setting_float,
		      "Hyst scaling:", "hyst_scaling", 4, olsr_cnf->hysteresis_param.scaling);

      size += sprintf(&buf[size], admin_basic_setting_float,
		      "Lower thr:", "hyst_lower", 4, olsr_cnf->hysteresis_param.thr_low);
      size += sprintf(&buf[size], admin_basic_setting_float,
		      "Upper thr:", "hyst_upper", 4, olsr_cnf->hysteresis_param.thr_high);
    }
  else
    {
      size += sprintf(&buf[size], "<td>Hysteresis disabled</td>\n");
    }

  size += sprintf(&buf[size], "</tr>\n");
  size += sprintf(&buf[size], "<tr>\n");
  
  if(olsr_cnf->lq_level)
    {
      size += sprintf(&buf[size], admin_basic_setting_int,
		      "LQ level:", "lq_level", 1, olsr_cnf->lq_level);
      size += sprintf(&buf[size], admin_basic_setting_int,
		      "LQ winsize:", "lq_wsize", 2, olsr_cnf->lq_wsize);
    }
  else
    {
      size += sprintf(&buf[size], "<td>LQ disabled</td>\n");
    }


  size += sprintf(&buf[size], "</tr>\n");
  size += sprintf(&buf[size], "<tr>\n");

  size += sprintf(&buf[size], "</tr>\n");
  
  i++;

  while(admin_frame[i] && strcmp(admin_frame[i], "<!-- HNAENTRIES -->\n"))
    {
      size += sprintf(&buf[size], admin_frame[i]);
      i++;
    }

  if(!admin_frame[i] || !admin_frame[i+1])
    return size;

  i++;

  if((olsr_cnf->ip_version == AF_INET) && (olsr_cnf->hna4_entries))
    {
      struct hna4_entry *hna4;
      
      for(hna4 = olsr_cnf->hna4_entries; hna4; hna4 = hna4->next)
	{
	  size += sprintf(&buf[size], admin_frame[i], 
			  olsr_ip_to_string((union olsr_ip_addr *)&hna4->net),
			  olsr_ip_to_string((union olsr_ip_addr *)&hna4->netmask),
			  olsr_ip_to_string((union olsr_ip_addr *)&hna4->net),
			  olsr_ip_to_string((union olsr_ip_addr *)&hna4->netmask));
	}
    }
  else if((olsr_cnf->ip_version == AF_INET6) && (olsr_cnf->hna6_entries))
    {
      struct hna6_entry *hna6;
	
      for(hna6 = olsr_cnf->hna6_entries; hna6; hna6 = hna6->next)
	{
	  size += sprintf(&buf[size], admin_frame[i], 
			  olsr_ip_to_string((union olsr_ip_addr *)&hna6->net),
			  "TBD"/*hna6->prefix_len*/);
	}
    }
  
  i++;

  while(admin_frame[i])
    {
      size += sprintf(&buf[size], admin_frame[i]);
      i++;
    }
  
  return size;
}


#ifdef ADMIN_INTERFACE

int
process_param(char *key, char *value)
{
  static olsr_u32_t curr_hna_net;
  static olsr_bool curr_hna_ok = OLSR_FALSE;

  if(!strcmp(key, "debug_level"))
    {
      int ival = atoi(value);
      if((ival < 0) || (ival > 9))
	return -1;

      olsr_cnf->debug_level = ival;
      return 1;
    }

  if(!strcmp(key, "tc_redundancy"))
    {
      int ival = atoi(value);
      if((ival < 0) || (ival > 3))
	return -1;

      olsr_cnf->tc_redundancy = ival;
      return 1;
    }

  if(!strcmp(key, "mpr_coverage"))
    {
      int ival = atoi(value);
      if(ival < 0)
	return -1;

      olsr_cnf->mpr_coverage = ival;
      return 1;
    }

  if(!strcmp(key, "willingness"))
    {
      int ival = atoi(value);
      if((ival < 0) || (ival > 7))
	return -1;

      olsr_cnf->willingness = ival;
      return 1;
    }

  if(!strcmp(key, "lq_level"))
    {
      int ival = atoi(value);
      if((ival < 0) || (ival > 2))
	return -1;

      olsr_cnf->lq_level = ival;
      return 1;
    }

  if(!strcmp(key, "lq_wsize"))
    {
      int ival = atoi(value);
      if((ival < 0) || (ival > 10))
	return -1;

      olsr_cnf->lq_wsize = ival;
      return 1;
    }

  if(!strcmp(key, "hyst_scaling"))
    {
      float fval = 1.1;
      sscanf(value, "%f", &fval);
      if((fval < 0.0) || (fval > 1.0))
	return -1;

      printf("HYST SCALING: %f\n", fval);
      olsr_cnf->hysteresis_param.scaling = fval;
      return 1;
    }

  if(!strcmp(key, "hyst_scaling"))
    {
      float fval = 1.1;
      sscanf(value, "%f", &fval);
      if((fval < 0.0) || (fval > 1.0))
	return -1;

      olsr_cnf->hysteresis_param.scaling = fval;
      return 1;
    }

  if(!strcmp(key, "hyst_lower"))
    {
      float fval = 1.1;
      sscanf(value, "%f", &fval);
      if((fval < 0.0) || (fval > 1.0))
	return -1;

      olsr_cnf->hysteresis_param.thr_low = fval;
      return 1;
    }

  if(!strcmp(key, "hyst_upper"))
    {
      float fval = 1.1;
      sscanf(value, "%f", &fval);
      if((fval < 0.0) || (fval > 1.0))
	return -1;

      olsr_cnf->hysteresis_param.thr_high = fval;
      return 1;
    }

  if(!strcmp(key, "pollrate"))
    {
      float fval = 1.1;
      sscanf(value, "%f", &fval);
      if((fval < 0.0) || (fval > 1.0))
	return -1;

      olsr_cnf->pollrate = fval;
      return 1;
    }


  if(!strcmp(key, "hna_new_net"))
    {
      struct in_addr in;

      if(inet_aton(value, &in) == 0)
	{
	  fprintf(stderr, "Failed converting new HNA net %s\n", value);
	  return -1;
	}
      curr_hna_ok = OLSR_TRUE;
      curr_hna_net = in.s_addr;
      return 1;
    }

  if(!strcmp(key, "hna_new_netmask"))
    {
      struct in_addr in;

      if(!curr_hna_ok)
	return -1;

      curr_hna_ok = OLSR_FALSE;

      if(inet_aton(value, &in) == 0)
	{
	  fprintf(stderr, "Failed converting new HNA netmask %s\n", value);
	  return -1;
	}
      add_local_hna4_entry((union olsr_ip_addr *)&curr_hna_net,
			   (union hna_netmask *)&in.s_addr);
      
      return 1;
    }

  if(!strncmp(key, "del_hna", 7) && !strcmp(value, "on"))
    {
      struct in_addr net, mask;
      char ip_net[16], ip_mask[16];
      int seperator = 0;

      while(key[7 + seperator] != '*')
	seperator++;

      strncpy(ip_net, &key[7], seperator);
      ip_net[seperator] = 0;
      strncpy(ip_mask, &key[7 + seperator + 1], 16);
      olsr_printf(1, "Deleting HNA %s/%s\n", ip_net, ip_mask);

      if(inet_aton(ip_net, &net) == 0)
	{
	  fprintf(stderr, "Failed converting HNA net %s for deletion\n", ip_net);
	  return -1;
	}

      if(inet_aton(ip_mask, &mask) == 0)
	{
	  fprintf(stderr, "Failed converting HNA netmask %s for deletion\n", ip_mask);
	  return -1;
	}

      remove_local_hna4_entry((union olsr_ip_addr *)&net.s_addr,
			      (union hna_netmask *)&mask.s_addr);

      return 1;
    }

  return 0;
#if 0
  { 1, admin_basic_setting_string, "TOS:", "tos", 6, "TBD" },
#endif
}

int
process_set_values(char *data, olsr_u32_t data_size, char *buf, olsr_u32_t bufsize)
{
  int size = 0;
  int i, val_start, key_start;

  printf("Dynamic Data: %s\n", data);

  size += sprintf(buf, "<html>\n<head><title>olsr.org httpinfo plugin</title></head>\n<body>\n");

  key_start = 0;
  val_start = 0;

  for(i = 0; i < data_size; i++)
    {
      if(data[i] == '=')
	{
	  data[i] = '\0';
	  val_start = i + 1;
	}

      if(data[i] == '&')
	{
	  data[i] = '\0';
	  if(!process_param(&data[key_start], &data[val_start]))
	    {
	      size += sprintf(&buf[size], "<h2>FAILED PROCESSING!</h2><br>Key: %s Value: %s<br>\n", 
			      &data[key_start], &data[val_start]);
	      return -1;
	    }

	  printf("Key: %s\nValue: %s\n", 
		 &data[key_start], &data[val_start]);
	  key_start = i + 1;
	}
    }  

  if(!process_param(&data[key_start], &data[val_start]))
    {
      size += sprintf(&buf[size], "<b>FAILED PROCESSING!</b><br>Key: %s Value: %s<br>\n", 
		      &data[key_start], &data[val_start]);
      return -1;
    }

  printf("Key: %s\nValue: %s\n", 
	 &data[key_start], &data[val_start]);

  size += sprintf(&buf[size], "<h2>UPDATE SUCESSFULL!</h2><br>Press BACK and RELOAD in your browser to return to the plugin<br>\n</body>\n</html>\n");
  size += sprintf(&buf[size], "\n</body>\n</html>\n");

  return size;
}
#endif
