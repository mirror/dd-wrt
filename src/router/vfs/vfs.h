typedef struct ENTRY
{
unsigned char namelen;
char *name; //name of file
unsigned int offset; // offset within file
unsigned int filelen; //
unsigned char *content;
unsigned int curpos;
} entry;

//typedef struct ENTRY entry;

typedef struct HEADER
{
char sign[6]; //DD-WRT
int noe; //number of entries
entry *entries; //offset in file
} header;

entry *vfsopen(char *name,char *mode);
int vfsseek(entry *in,int pos,int use);
int vfsgetc(entry *in);
int vfsread(void *fill,int blocks,int blocksize,entry *in);
int vfsrewind(entry *in);
int vfsclose(entry *in);



