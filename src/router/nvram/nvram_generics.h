#include <malloc.h>

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
	const char *value = BCMINIT(nvram_get)(name);
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
	const char *value = BCMINIT(nvram_get)(name);
	return (value && strcmp(value, invmatch));
}


char *nvram_prefix_get(const char *name, const char *prefix)
{
char p[64];
sprintf(p,"%s_%s\n",prefix,name);
return nvram_safe_get(p);
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
if(!nvram_get(name) || strcmp(nvram_get(name), value))
	nvram_set(name, value); 
}
