#include <malloc.h>

void nvram_store_collection(char *name,char *buf)
{
char *chain;
char *n;
int size=strlen(name);
int chaincount=size/1024;
int c=0;
int i,offset;
offset=0;
for (i=0;i<chaincount;i++)
    {
    n=malloc(strlen(name)+16);
    sprintf(n,"%s%d",name,c++);
    chain=malloc(1025);
    memcpy(chain,&buf[offset],1024);
    chain[1024]=0;
    nvram_set(n,chain);
    offset+=1024;
    free(n);
    free(chain);
    }
int rest=size%1024;
n=malloc(strlen(name)+16);
sprintf(n,"%s%d",name,c);
chain=malloc(rest);
memcpy(chain,&buf[offset],rest);
chain[rest]=0;
nvram_set(n,chain);
free(n);
free(chain);
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
while(nvram_get(n)!=NULL)
    {
    char *chain=nvram_get(n);
    chains=(char*)realloc(chains,chains!=NULL?strlen(chains)+strlen(chain)+1:strlen(chain+1));
    memcpy(chains[offset],chain,strlen(chain));
    offset+=strlen(chain);
    chains[offset]=0;
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
