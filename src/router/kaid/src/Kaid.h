#ifndef KAID_H
#define KAID_H

#include <vector>
#include <string>
#include <iterator>

#ifndef PLATFORM_freebsd
#include <locale>
#endif

#include <string>
#include <algorithm>
#include <cctype>

#include <sys/time.h>

#define	AUTHOR				"Mineiro"
#if defined(PLATFORM_macosx_jaguar)
#define PLATFORM			"Mac OSX"
#elif defined(PLATFORM_freebsd)
#define PLATFORM			"FreeBSD"
#elif defined(OPENWRT)
#define PLATFORM			"KaiStation"
#else
#define PLATFORM			"Linux"
#endif

using namespace std;

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ");
void Tokenize(const string& str, vector<string>& tokens, const char& delimiter = (char)1);
string  Str(const int val);
void debuglog(string facility, string msg);
const struct timeval Now();
unsigned int TimeDelta(struct timeval tv_start, struct timeval tv_end);
unsigned int TimeDelta(struct timeval tv_start);
bool Future(struct timeval tv_check);
bool file_exists(const char* filename);
char* tstamp();
string crop(string str, char what);

#endif
