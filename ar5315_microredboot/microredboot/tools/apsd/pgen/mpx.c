void 
print_hex_string(char* buf, int len)
{
    int i;

    if (len==0) { printf("<empty string>"); return; }

    for (i = 0; i < len; i++) {
        printf("%02x ", *((unsigned char *)buf + i));
	if ((i&0xf)==15) printf("\n");
    }
    if ((i&0xf))
	printf("\n");
}


void 
print_c_hex_string(char* buf, int len)
{
    int i;

    if (len==0) { printf("<empty string>"); return; }

    for (i = 0; i < len; i++) {
	if (i<len-1)
        	printf("0x%02x,", *((unsigned char *)buf + i));
	else
        	printf("0x%02x", *((unsigned char *)buf + i));
	if ((i&0xf)==15) printf("\n");
    }
    if ((i&0xf)!=1)
	printf("\n");
}


void 
print_vera_hex_string(char* cname, char *buf, int len)
{
    int i;

    if (len==0) { printf("<empty string>"); return; }

    for (i = 0; i < len; i++) {
	if (i<len-1)
        	printf("%s[%d]='h%02x, ", cname, i, *((unsigned char *)buf + i));
	else
        	printf("%s[%d]='h%02x", cname, i, *((unsigned char *)buf + i));
	if ((i&0x3)==3) printf("\n");
    }
    if ((i&0xf)!=1)
	printf("\n");
}

void
mpx(m, buf, len)
char *m, *buf; int len;
{
	printf("%s", m);
	print_hex_string(buf, len);
}

void
cmpx(m, buf, len)
char *m, *buf; int len;
{
	printf("\nunsigned char %s[] ={\n", m);
	print_c_hex_string(buf, len);
	printf("\n};\n");
}
void
kmpx(m, buf, len)
char *m, *buf; int len;
{
	printf("\nunsigned char %skey[] ={\n", m);
	print_c_hex_string(buf, len);
	printf("\n};\n");
}
void
smpx(s1, m, s2, buf, len)
char *s1, *s2, *m, *buf; int len;
{
	printf(s1, m);
	print_c_hex_string(buf, len);
	printf(s2);
}

void
vera_mpx(m, buf, len)
char *m, *buf; int len;
{
	print_vera_hex_string(m, buf, len);
}

void
swbx(buf, len)
char *buf; int len;
{
char tmp;
int i;
	for(i=0; i<len; i=i+2) {
		tmp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = tmp;
	}
}

