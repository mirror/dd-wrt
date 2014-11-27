#ifdef RAISENTINET3
#include <stdlib.h>
#include <string.h>

char *str_r_trim(char *str)
{ char *attuale;
  for (attuale = str + strlen(str) - 1; (*attuale == ' ' || *attuale == '\t') && (attuale >= str); --attuale) 
      ;
  *(++attuale) = '\0'; 
  return(str); 
}
char *str_l_trim(char *stringa)
{ char *attuale, *in_no_b; 
  for (in_no_b = stringa; *in_no_b == ' ' || *in_no_b == '\t'; ++in_no_b) 
      ;
  for (attuale = stringa; *in_no_b != '\0'; ++attuale, ++in_no_b) 
      *attuale = *in_no_b;
  *attuale = '\0'; 
  return(stringa); 
}

char *TogliCommenti(char *str)
{ char *p;
  p=strchr (str, '#');
  if (p != NULL)
      p[0]='\0';
  return(str);
}
char *TogliSpazi(char *str)
{ str_r_trim(str);
  str_l_trim(str);
  return(str);
}

char *SeparaLabelValore(char *str)
{ char *p;
  p=strchr (str, '\n');
  if (p != NULL)
      p[0]='\0';
  p=strchr (str, '=');
  if (p != NULL)
      p[0]='\0';
  else
      return(str);
  return(++p);
}

int LeggiFileConfig(void)
{
	FILE *fp;
	dg_mode = 0;
	fp = fopen("/etc/discovery.conf", "r");
	if (fp == NULL)
	{
		dg_mode = 1; 
		return(1);
		//fprintf(stderr,"ERROR: cannot open file /etc/discovery.conf\n");
		//exit(1);
	}else
	{
		while (!feof(fp))
		{
			if (!fgets(UrlServer, 80, fp)) return(1);
			TogliCommenti(UrlServer);
			UrlServerVal=SeparaLabelValore(UrlServer);
			TogliCommenti(UrlServerVal);
			TogliSpazi(UrlServer);
			TogliSpazi(UrlServerVal);
			if (!strcmp(UrlServer,"UrlServer"))
			{
				/*printf("Il file è stato aperto label->%s\n",UrlServer);
				  printf("Il file è stato aperto valore->%s\n",UrlServerVal);*/
				break;
			}
		}
		fclose(fp);
		return(0);
	}
}
void InviaRisultati(risultatoScan *risultatoHost, int pingscan)
{
	char *wrapper = NULL;
	char cmd_wget[4000];
	extern char *UrlServerVal, *zona; 

	wrapper = getenv("NMAP_WRAPPER");
	if (!wrapper || !*wrapper)
		wrapper = strdup("nmap_wrapper");

	if (pingscan == 1) { //solo host discovery
		risultatoHost->OpenPort = (char *)malloc(2*sizeof(char));  
		sprintf(risultatoHost->OpenPort,"%s","-");
	}	
	//printf("\nUrl=%s zona=%s Ip=%s Dns=%s Mac=%s MacVendor=%s OS=%s\n OpenPort---\n%s---\n", UrlServerVal, zona, risultatoHost->Ip, risultatoHost->Dns, risultatoHost->Mac, risultatoHost->MacVendor, risultatoHost->OS, risultatoHost->OpenPort);

	if (!dg_mode)
		sprintf(cmd_wget,"wget -q --no-check-certificate --delete-after --post-data='submit=submit&add=%s'  '%s/include/audit/admin_pc_add_discovery.php?info=%s^^^%s^^^%s^^^%s^^^%s^^^%s'",risultatoHost->OpenPort,UrlServerVal,zona,risultatoHost->Ip,risultatoHost->Dns,risultatoHost->Mac,risultatoHost->MacVendor,risultatoHost->OS);
	else
		sprintf(cmd_wget,"%s -q --no-check-certificate --delete-after --post-data='submit=submit&add=%s'  'http://127.0.0.1/include/audit/admin_pc_add_discovery.php?info=%s^^^%s^^^%s^^^%s^^^%s'",wrapper,risultatoHost->OpenPort,risultatoHost->Ip,risultatoHost->Dns,risultatoHost->Mac,risultatoHost->MacVendor,risultatoHost->OS);

	printf(">>>--\n%s\n",cmd_wget);
	system(cmd_wget);

	free(risultatoHost->Dns); free(risultatoHost->Mac); 
	free(risultatoHost->MacVendor); free(risultatoHost->OS); free(risultatoHost->OpenPort);
	free(risultatoHost);
}	  
#endif /* RAISENTINET3 */
