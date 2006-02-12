/*
**	BPALogin v2.0 - lightweight portable BIDS2 login client
**	Copyright (c) 1999  Shane Hyde (shyde@trontech.com.au)
** 
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
** 
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/ 

#include "bpalogin.h"
#include "ddns3-client/ctx.h"
#include <bcmnvram.h>

struct session s;
int debug_level = DEFAULT_DEBUG;
char ddnsconffile[256];
int dosyslog = 1;

extern int errno;

int parse_parms(struct session *,char * conffile);
void usage();
void debug(int l,char *s,...);
void noncritical(char *s,...);

void onconnected(int i)
{
    if(strcmp(s.connectedprog,""))
    {
        char buf[500];
		sprintf(buf,"%.500s %d %d %s",
				s.connectedprog,					// the program we're executing.
				s.listenport,						// The port we're listening on
				getpid(),						// Our PID
				inet_ntoa(s.tsmlist_in[s.octmslistindex].sin_addr));	// The IP of server sending us heartbeats

        debug(0,"Executing external command - %s\n",buf);
        system(buf);
    }
	
	/* ddns3 update */
/*	if(s.ddns3_active == 3) */ /* GRRR - config parser broken */
	if(s.ddns3_active > 0 && s.ddns3_active%3 == 0)
	{
		struct ddns3_ctx *dc;
		ddns3_ctx_new(&dc, s.ddns3_server, atoi(s.ddns3_port));

		debug(0,"ddns3: connecting to server %s\n", dc->url);
		if(ddns3_ctx_connect(dc)) {
			noncritical("ddns3: connection failed\n");
		} else {
			debug(0,"ddns3: logging in as '%s'\n", s.ddns3_user);
			if(ddns3_ctx_login(dc, s.ddns3_auth, s.ddns3_user, s.ddns3_pass)) {
				noncritical("ddns3: authentication failed: %s\n", dc->buf);
			} else {
				debug(0,"ddns3: updating '%s' := %s\n", s.ddns3_handle, inet_ntoa(s.localipaddress.sin_addr));
				if(ddns3_ctx_set(dc, s.ddns3_handle, inet_ntoa(s.localipaddress.sin_addr)))
					noncritical("ddns3: error updating IP-Handle: %s\n", dc->buf);
				else
					debug(0,"ddns3: success\n");
				ddns3_ctx_logout(dc);
			}
			ddns3_ctx_disconnect(dc);
		}
		ddns3_ctx_del(&dc);
	}
}

void ondisconnected(int reason)
{
    if(strcmp(s.disconnectedprog,""))
    {
        char buf[500];
        sprintf(buf,"%.500s %d",s.disconnectedprog,reason);

        debug(0,"Executing external command - %s\n",buf);
        system(buf);
    }
}

void critical(char *s)
{
    if(dosyslog)
    syslog(LOG_CRIT,"Critical error: %s\n",s);
    else
    printf("Critical error: %s\n",s);
    closelog();
    exit(1);
}

void debug(int l,char *s,...)
{
    va_list ap;
    va_start(ap,s);
    if(debug_level > l)
    {
        int pri;
        char buf[256];

        switch(l)
        {
        case 0:
            pri = LOG_INFO;
            break;
        case 1:
            pri = LOG_INFO;
            break;
        case 2:
        case 3:
        default:
            pri = LOG_INFO;
            break;
        }
        vsprintf(buf,s,ap);
        if(dosyslog)
        syslog(pri,"%s",buf);
        else
        printf("%s",buf);
    }
    va_end(ap);
}

void noncritical(char *s,...)
{
    char buf[256];

    va_list ap;
    va_start(ap,s);
    vsprintf(buf,s,ap);
    if(dosyslog)
    syslog(LOG_CRIT,buf);
    else
    printf(buf);
    va_end(ap);
}

void onsignal(int i)
{
    debug(1,"Signal caught\n");
    logout(0,&s);
    closesocket(s.listensock);
    s.ondisconnected(0);
    closelog();
    exit(1);
}

void onsignal1(int i)
{
	debug(1,"Signal caught HUP\n");
	logout(0,&s);
	closesocket(s.listensock);
	s.ondisconnected(0);
	closelog();
	exit(1);
}


void onsignal2(int i)
{
	debug(1,"Signal caught TERM\n");
	logout(0,&s);
	closesocket(s.listensock);
	s.ondisconnected(0);
	closelog();
	exit(1);
}



int main(int argc,char* argv[])
{
    int makedaemon = 1;
    char conffile[256];

    int c;

    signal(SIGINT,onsignal);
	signal(SIGHUP,onsignal1);
	signal(SIGTERM,onsignal2);

    strcpy(s.authserver,DEFAULT_AUTHSERVER);
    strcpy(s.authdomain,DEFAULT_AUTHDOMAIN);
    s.authport = DEFAULT_AUTHPORT;
    strcpy(s.username,"");
    strcpy(s.password,"");
    strcpy(s.connectedprog,"");
    strcpy(s.disconnectedprog,"");
    strcpy(conffile,DEFAULT_CONFFILE);
    strcpy(s.localaddress,"");
	s.localport = 5050;
    s.minheartbeat = 60;

	strcpy(ddnsconffile,"");

	/* ddns3 changes */
	s.ddns3_active = 0;
	strcpy(s.ddns3_server,"ns.ddns.nu");
	strcpy(s.ddns3_port,"5000");
	strcpy(s.ddns3_auth,"ddns");

	while(1)
	{
		c = getopt(argc,argv,"c:Dd:");
		if(c == -1)
            break;
		switch(c)
		{
		case 'c':
			strncpy(conffile,optarg,MAXCONFFILE);
	    break;
        case '?':
            usage();
            exit(1);
            break;
        }
    }

	if(!parse_parms(&s,conffile))
	{
        usage();
        exit(1);
    }

    optind = 1;
	while(1)	
	{
		c = getopt(argc,argv,"c:Dd:");
		if(c == -1)
			break;
		switch(c)
		{
        case 'D':
            makedaemon = 0;
            break;
        case 'c':
            break;
        case 'd':
            debug_level = atoi(optarg);
            break;
        case '?':
            break;
        case ':':
            break;
      }
      }

	if(makedaemon && fork())
          exit(0);

    openlog("bpalogin",LOG_PID,LOG_DAEMON);

    if(dosyslog)    
	syslog(LOG_INFO,"BPALogin v2.0 - lightweight portable BIDS2 login client\n");
    else
	printf("BPALogin v2.0 - lightweight portable BIDS2 login client\n");

    if(!strcmp(s.username,""))
    {
        critical("Username has not been set");
	closelog();
        exit(1);
    }
    if(!strcmp(s.password,""))
    {
        critical("Password has not been set");
	closelog();
        exit(1);
    }
    s.debug = debug;
    s.critical = critical;
    s.noncritical = noncritical;
    s.onconnected = onconnected;
    s.ondisconnected = ondisconnected;

    while(mainloop(&s));
    s.ondisconnected(0);

    closelog();
    exit(0);
}

int parse_parms(struct session *s,char * conffile)
{
    char buf[512];
    FILE * f;

    f = fopen(conffile,"rt");
    if(!f)
    {
        debug(0,"Cannot open conf file\n");
        return FALSE;
    }

    while(fgets(buf,400,f) != NULL)
    {
        char parm[100];
        char value[100];

        if(buf[0] == '#')
            continue;

        sscanf(buf,"%s %s",parm,value);    
        debug(2,"Parameter %s set to %s\n",parm,value);

        if(!strcasecmp(parm,"username"))
        {
            strcpy(s->username,value);
        }
        else if(!strcasecmp(parm,"password"))
        {
            strcpy(s->password,value);
        }
        else if(!strcasecmp(parm,"authdomain"))
        {
            strcpy(s->authdomain,".");
            strcat(s->authdomain,value);
        }
        else if(!strcasecmp(parm,"authserver"))
        {
            strcpy(s->authserver,value);
        }
        else if(!strcasecmp(parm,"localaddress"))
        {
            strcpy(s->localaddress,value);
        }
        else if(!strcasecmp(parm,"logging"))
        {
			if(!strcmp("sysout",value)) dosyslog = 0;
			if(!strcmp("syslog",value)) dosyslog = 1;
        }
        else if(!strcasecmp(parm,"debuglevel"))
        {
			int v = atoi(value);
			debug_level = v;	
        }
        else if(!strcasecmp(parm,"minheartbeatinterval"))
        {
			int v = atoi(value);
			s->minheartbeat = v;	
        }
        else if(!strcasecmp(parm,"localport"))
        {
			int v = atoi(value);
			s->localport = v;	
        }
        else if(!strcasecmp(parm,"connectedprog"))
        {
            strcpy(s->connectedprog,value);
        }
        else if(!strcasecmp(parm,"disconnectedprog"))
        {
            strcpy(s->disconnectedprog,value);
        }
		/* ddns3 changes */
		else if(!strcasecmp(parm,"ddns3-server"))
		{
			strcpy(s->ddns3_server,value);
		}
		else if(!strcasecmp(parm,"ddns3-port"))
		{
			strcpy(s->ddns3_port,value);
		}
		else if(!strcasecmp(parm,"ddns3-auth"))
		{
			strcpy(s->ddns3_auth,value);
    }
    
		else if(!strcasecmp(parm,"ddns3-username"))
		{
			strcpy(s->ddns3_user,value);
			s->ddns3_active ++;
		}
		else if(!strcasecmp(parm,"ddns3-password"))
		{
			strcpy(s->ddns3_pass,value);
			s->ddns3_active ++;
		}
		else if(!strcasecmp(parm,"ddns3-handle"))
		{
			strcpy(s->ddns3_handle,value);
			s->ddns3_active ++;
		}
		if(s->ddns3_active == 0 ){
		    if(strcmp(nvram_safe_get("ddns3_username"),"") != 0 ){
			strcpy(s->ddns3_user,nvram_get("ddns3_username"));
			s->ddns3_active ++;
		    }
		    if(strcmp(nvram_safe_get("ddns3_password"),"") != 0 ){
			strcpy(s->ddns3_pass,nvram_get("ddns3_password"));
			s->ddns3_active ++;
		    }
		    if(strcmp(nvram_safe_get("ddns3_handle"),"") != 0 ){
			strcpy(s->ddns3_handle,nvram_get("ddns3_handle"));
			s->ddns3_active ++;
		    }
		}
	}
    fclose(f);
    strcat(s->authserver,s->authdomain);
    return TRUE;
}

void usage()
{
	printf("BPALogin v2.0 - lightweight portable BIDS2 login client\n");
	printf("Copyright (c) 1999-2000 Shane Hyde (shyde@trontech.net)\n");
	printf("\nThis program is *not* a product of Big Pond Advance\n");
	printf("\nThis build of BPALogin contains support for ddns v3\n");
	printf("Copyright (c) 1999-2001 Alan Yates <alany@ay.com.au>\n");
	printf("\nUsage: bpalogin [options], where options are:\n\n");
	printf(" -c conffile          Specifies the config file to read option\n");
	printf("                      settings from (default is /etc/bpalogin.conf)\n");
	printf(" -D                   Dont run bpalogin as a daemon (run in foreground)\n");
	printf("\nNote that command line options override the values in the conffile\n");
	
}

int closesocket(int s)
{
    return close(s);
}

void socketerror(struct session *s, const char * str)
{
    char buf[200];
    sprintf(buf,"%.100s - %.80s",str,strerror(errno));
    s->noncritical(buf);
}
