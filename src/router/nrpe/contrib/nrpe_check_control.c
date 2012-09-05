#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_CHARS	1024
#define SERVICE_COUNT	12

#define COMMAND_FILE	"/usr/local/nagios/var/rw/nagios.cmd"
#define SERVICES_FILE	"/usr/local/nagios/etc/services.cfg"

int main(int argc, char *argv[])
{
  char check_name[MAX_CHARS];
  char ent_type[MAX_CHARS];
  char input_buffer[MAX_CHARS];
  char host_name[MAX_CHARS];
  char service_name[MAX_CHARS];
  char state[MAX_CHARS];
  char state_type[MAX_CHARS];
  char temp_input[MAX_CHARS];
  char temp_string[MAX_CHARS];
  char test_host[MAX_CHARS];

  char *temp_var;

  FILE *command_fp;
  FILE *services_fp;

  int attempt;
  int i;

  time_t current_time;

  strcpy(state,argv[1]);
  strcpy(state_type,argv[2]);
  attempt=atoi(argv[3]);
  strcpy(host_name,argv[4]);

  if(strcmp(state,"OK") == 0)
  {
    services_fp=fopen(SERVICES_FILE,"r");
    command_fp=fopen(COMMAND_FILE,"a");
    while((fgets(input_buffer,MAX_CHARS-1,services_fp)) != NULL)
    {
      if(input_buffer[0]=='#' || input_buffer[0]=='\x0' || input_buffer[0]=='\n' || input_buffer[0]=='\r')
      {
        continue;
      }
      else
      {
        strcpy(temp_input,input_buffer);
        strcpy(temp_string,strtok(temp_input,"="));
        strcpy(ent_type,strtok(temp_string,"["));
        if(strcmp(ent_type,"service") == 0)
        {
          strcpy(test_host,strtok(NULL,"]"));
          if(strcmp(test_host,host_name) == 0)
          {
            temp_var=strtok(input_buffer,"=");
            strcpy(service_name,strtok(NULL,";")); 
            for(i=1;i<=SERVICE_COUNT;i++)
            {
              temp_var=strtok(NULL,";");
            }
            strcpy(check_name,strtok(temp_var,"!"));
            if(strcmp(check_name,"check_nrpe") == 0)
            {
              time(&current_time);
              fprintf(command_fp,"[%lu] ENABLE_SVC_CHECK;%s;%s\n",current_time,host_name,service_name);
            }
          }
        }
      }
    }
    fclose(command_fp);
    fclose(services_fp);
  }
  else if(strcmp(state,"CRITICAL") == 0)
  {
    if(attempt == 3)
    {
      services_fp=fopen(SERVICES_FILE,"r");
      command_fp=fopen(COMMAND_FILE,"a");
      while((fgets(input_buffer,MAX_CHARS-1,services_fp)) != NULL)
      {
        if(input_buffer[0]=='#' || input_buffer[0]=='\x0' || input_buffer[0]=='\n' || input_buffer[0]=='\r')
        {
          continue;
        }
        else
        {
          strcpy(temp_input,input_buffer);
          strcpy(temp_string,strtok(temp_input,"="));
          strcpy(ent_type,strtok(temp_string,"["));
          if(strcmp(ent_type,"service") == 0)
          {
            strcpy(test_host,strtok(NULL,"]"));
            if(strcmp(test_host,host_name) == 0)
            {
              temp_var=strtok(input_buffer,"=");
              strcpy(service_name,strtok(NULL,";")); 
              for(i=1;i<=SERVICE_COUNT;i++)
              {
                temp_var=strtok(NULL,";");
              }
              strcpy(check_name,strtok(temp_var,"!"));
              if(strcmp(check_name,"check_nrpe") == 0)
              {
                time(&current_time);
                fprintf(command_fp,"[%lu] DISABLE_SVC_CHECK;%s;%s\n",current_time,host_name,service_name);
              }
            }
          }
        }
      }
      fclose(command_fp);
      fclose(services_fp);
    }
  }
  return 0;
}
