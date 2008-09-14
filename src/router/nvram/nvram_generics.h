#include <malloc.h>
#include <stdarg.h>

#define cprintf(fmt, args...)

/*
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)
*/

void nvram_store_collection(char *name,char *buf)
{
char *chain;
char *n;
int size=strlen(buf);
int chaincount=size/1024;
int c=0;
int i,offset;
offset=0;
cprintf("chaincount = %d\n",chaincount);
for (i=0;i<chaincount;i++)
    {
    n=malloc(strlen(name)+16);
    sprintf(n,"%s%d",name,c++);
    cprintf("get chain name %s\n",n);
    chain=malloc(1025);
    memcpy(chain,&buf[offset],1024);
    chain[1024]=0;
    cprintf("story chain %s\n",chain);
    nvram_set(n,chain);
    offset+=1024;
    free(n);
    free(chain);
    }
int rest=size%1024;
if (rest)
{
n=malloc(strlen(name)+16);
sprintf(n,"%s%d",name,c);
cprintf("chainname = %s, malloc = %d\n",n,rest+16);
chain=malloc(rest+1);
memcpy(chain,&buf[offset],rest);
chain[rest]=0;
nvram_set(n,chain);
cprintf("free mem\n");
free(n);
free(chain);
}
}
/*
    do not forget to free the returned result
*/
char *nvram_get_collection(char *name)
{
char *chains=NULL;
int offset=0;
int c=0;
char n[65];

sprintf(n,"%s%d",name,c++);
cprintf("name = %s\n",n);
while(nvram_get(n)!=NULL)
    {
    char *chain=nvram_get(n);
    cprintf("chain = %s\n",chain);
    if (chains==NULL)
	chains=malloc(strlen(chain)+1);
    chains=(char*)realloc(chains,chains!=NULL?strlen(chains)+strlen(chain)+1:strlen(chain)+1);
    cprintf("alloc okay\n");
    memcpy(&chains[offset],chain,strlen(chain));
    offset+=strlen(chain);
    chains[offset]=0;
    cprintf("copy %s\n",chains);
    sprintf(n,"%s%d",name,c++);
    }
return chains;
}
/*
 * Match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal
 *		to match or FALSE otherwise
 */
int
nvram_match(char *name, char *match) {
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

/*
 * Inversely match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string
 *		equal to invmatch or FALSE otherwise
 */
int
nvram_invmatch(char *name, char *invmatch) {
	const char *value = nvram_get(name);
	return (value && strcmp(value, invmatch));
}


char *nvram_prefix_get(const char *name, const char *prefix)
{
char p[64];
sprintf(p,"%s_%s",prefix,name);
return nvram_safe_get(p);
}
int nvram_prefix_match(const char *name, const char *prefix,char *match)
{
char p[64];
sprintf(p,"%s_%s",prefix,name);
return nvram_match(p,match);
}

int nvram_nmatch(char *match,const char *fmt,...)
{
char varbuf[64];
va_list args;
va_start(args, (char*)fmt);
vsnprintf(varbuf, sizeof(varbuf), fmt, args);
va_end(args);
return nvram_match(varbuf,match);
}

char *nvram_nget(const char *fmt,...)
{
char varbuf[64];
va_list args;
va_start(args, (char*)fmt);
vsnprintf(varbuf, sizeof(varbuf), fmt, args);
va_end(args);
return nvram_safe_get(varbuf);
}
char *nvram_nset(char *value,const char *fmt,...)
{
char varbuf[64];
va_list args;
va_start(args, (char*)fmt);
vsnprintf(varbuf, sizeof(varbuf), fmt, args);
va_end(args);
return nvram_set(varbuf,value);
}

char *nvram_safe_get(const char *name)
{
return nvram_get(name)?:"";
} 

void nvram_safe_unset(const char *name)
{
if(nvram_get(name)) 
	nvram_unset(name); 
}

void nvram_safe_set(const char *name, char *value)
{
if(!nvram_get(name) || strcmp(nvram_safe_get(name), value))
	nvram_set(name, value); 
}
int nvram_default_match (char *var, char *match, char *def)
{
  char *v = nvram_get (var);
  if (v == NULL || strlen (v) == 0)
    {
      nvram_set (var, def);
      return !strcmp(match,def);
    }
  return nvram_match (var, match);
}
char *nvram_default_get (char *var, char *def)
{
  char *v = nvram_get (var);
  if (v == NULL || strlen (v) == 0)
    {
      nvram_set (var, def);
      return def;
    }
  return nvram_safe_get (var);
}

void fwritenvram(char *var,FILE *fp)
{
int i;
if (fp==NULL)
    return;
      char *host_key = nvram_safe_get (var);
      int len = strlen(host_key);
      for (i=0;i<len;i++)
     	  if (host_key[i] != 0x0D)
	    fprintf (fp, "%c", host_key[i]);
}
void writenvram(char *var,char *file)
{
int i;
FILE *fp=fopen(file,"wb");
if (fp==NULL)
    return;
fwritenvram(var,fp);
fclose(fp);
}
