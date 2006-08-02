#include <iostream>
#include <stdio.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef PLATFORM_freebsd
#include <getopt.h>
#else
#include <unistd.h>
#endif

#include "Kaid.h"
#include "KaiDaemon.h"
#include "ifaddrlist.h"

int shut_the_fuck_up = 0, daemonize = 0;
FILE *logger = NULL;
struct ifaddrlist *iflst;

string crop(string str, char what)
{
	string aux = "";
	for(unsigned int i = 0; i < str.size(); i++)
	{
		if(str[i] != what)
			aux += str[i];
	}
	return aux;
}

int split(vector<string>& v, const string& str, char c)
{
	v.clear();
	string::const_iterator s = str.begin();
	while (true) {
		string::const_iterator begin = s;
	
		while (*s != c && s != str.end()) { ++s; }
	
		v.push_back(string(begin, s));
	
		if (s == str.end()) {
		break;
		}
		if (++s == str.end()) {
			// v.push_back("");
			break;
		}
	}
	return v.size();
}
    

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters)
{
	split(tokens, str, delimiters[0]);
}

void Tokenize(const string& str, vector<string>& tokens, const char& delimiter)
{
	split(tokens, str, delimiter);
}

string  Str(const int val)
{
	char buf[32];
	sprintf(buf, "%d", val);
	return buf;
}

string trim(string source, char const* delims)
{
	string result(source);
	string::size_type index = result.find_last_not_of(delims);
	if(index != string::npos)
		result.erase(++index);
    
	index = result.find_first_not_of(delims);
	if(index != string::npos)
		result.erase(0, index);
	else
		result.erase();
	return result;
}

void debuglog(string facility, string msg)
{
    if((shut_the_fuck_up == 0) || (logger != NULL)) {
        string logline = msg;
        if(!facility.empty()) {
            logline = facility + ": " + logline;
        }
        if(logger) {
			logline += "\n";
            fputs(logline.c_str(), logger);
			fflush(logger);
        }
        else
        {
			if(daemonize == 0) {
				cout << logline << endl;
			} else {
            	syslog(LOG_INFO, logline.c_str());
			}
        }
    }
}

const struct timeval Now()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now;
}

unsigned int TimeDelta(struct timeval tv_start, struct timeval tv_end)
{
	int delta_usec, delta_sec;
	delta_sec = tv_end.tv_sec - tv_start.tv_sec;
	delta_usec = tv_end.tv_usec - tv_start.tv_usec;
	return ((delta_sec * 1000) + (delta_usec / 1000));
}

unsigned int TimeDelta(struct timeval tv_start)
{
	return TimeDelta(tv_start, Now());
}

bool Future(struct timeval tv_check)
{
	struct timeval tv_now=Now();
	return ( tv_check.tv_sec>tv_now.tv_sec ||
		    (tv_check.tv_sec==tv_now.tv_sec && tv_check.tv_usec>tv_now.tv_usec) );
}

bool file_exists(const char* filename)
{
    FILE *f;
    f = fopen(filename, "r");
    if(f) {
        fclose(f);
        return true;
    }
    return false;
}

void usage()
{
    printf("Kai Engine %s (http://www.teamxlink.co.uk)\n\n", VERSION);
    printf("Usage instructions:\n");
    printf("\tkaid [-h] [-d | -o log | -s] [-c file]\n");
    printf("\t-h\t\t= show usage instructions (these)\n");
    printf("\t-d\t\t= fork into the background, daemonize\n");
    printf("\t-o log\t\t= dump logging to 'log' instead of syslog\n");
    printf("\t-s\t\t= silent, *really* quiet\n");
    printf("\t-c file\t\t= use settings found in 'file', default is 'kaid.conf' in current dir\n");
    printf("\t-V\t\t= show version intval (useful for scripters)\n\n");
    printf("NOTE: kaid needs to be run as root (pcap promiscuous mode)\n");
}

char* tstamp()
{
/*
    time_t *t;
    char *n;
    time(t);
    ctime_r(t, n);
    return n;    
*/
	#define TIME_SIZE 40

	const struct tm *tm;
	size_t len;
	time_t now;
	char *s;

	now = time ( NULL );
	tm = localtime ( &now );

	s = new char[TIME_SIZE];

	len = strftime ( s, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

	return s;
	#undef TIME_SIZ
}

int main(int argc, char *argv[])
{
	int c;
    pid_t pid, sid;
    
    string config = CONFIG_FILE;
    
    openlog("kaid", LOG_PID, LOG_DAEMON);
    try
    {
		while((c = getopt (argc, argv, "c:dho:sf:V")) != -1)
        {
			switch(c)
            {
                case 'h':
                {
                    usage();
                    return 0;
                }
				case 'd':
				{
					daemonize = 1;
					break;
				}
                case 's':
                {
                    shut_the_fuck_up = 1;
                    break;
                }
                case 'c':
                {
                    config = optarg;
                    break;
                }
                case 'o':
                {
                    logger = fopen(optarg, "a");
                    if(!logger) {
                        perror("KAID: Error opening logfile!");
                        return 1;
                    }
                    fprintf(logger, "-- Started logging: %s --\n", tstamp());
					break;
                }
                case 'f':
                {
                	printf("KAID: Would use local file for settings");
                	break;
                }
				case 'V':
				{
					string a = crop(VERSION, '.');
					printf("%s\n", a.c_str());
					exit(0);
					break;
				}
            }
        }

		if(geteuid() != 0)
		{
			printf("KAID: You're not root, sorry...\n");
			exit(1);
		}    

		openlog("kaid", LOG_PID, LOG_DAEMON);
		if(daemonize == 1)
		{        
			debuglog("KAID", "Kai Engine is Daemonizing...");
        	pid = fork();
        	if (pid < 0) {
            	exit(EXIT_FAILURE);
        	}
        
        	if (pid > 0) {
            	exit(EXIT_SUCCESS);
        	}
        
        	umask(0);
        
        	sid = setsid();
        	if (sid < 0) {
            	exit(EXIT_FAILURE);
        	}
    
        	close(STDIN_FILENO);
        	close(STDOUT_FILENO);
        	close(STDERR_FILENO);
		}
        Daemon cDaemon(config);
        cDaemon.RunDaemon();
	}
    catch (...)
    {
        debuglog("KAID", "Failed to initialise...");
    }
    if(logger)
        fclose(logger);
    closelog();
}
