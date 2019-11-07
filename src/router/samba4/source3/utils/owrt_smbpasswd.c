/*
 * Copyright (C) John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675
 * Mass Ave, Cambridge, MA 02139, USA.  */

#include "includes.h"
#include <endian.h>

void E_md4hash(const char *passwd, uchar p16[16])
{
	int len;
	smb_ucs2_t wpwd[129];
	int i;

	len = strlen(passwd);
	for (i = 0; i < len; i++) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
		wpwd[i] = (unsigned char)passwd[i];
#else
		wpwd[i] = (unsigned char)passwd[i];
		wpwd[i] <<= 8;
#endif
	}
	wpwd[i] = 0;

	len = len * sizeof(unsigned short);
	mdfour(p16, (unsigned char *)wpwd, len);
	ZERO_STRUCT(wpwd);
}

/**
 * Creates the DES forward-only Hash of the users password in DOS ASCII charset
 * @param passwd password in 'unix' charset.
 * @param p16 return password hashed with DES, caller allocated 16 byte buffer
 * @return False if password was > 14 characters, and therefore may be incorrect, otherwise True
 * @note p16 is filled in regardless
 */
 
bool E_deshash(const char *passwd, uchar p16[16])
{
	int ret = 1;
	char dospwd[256+2];
	int i;
	int len;
	
	/* Password must be converted to DOS charset - null terminated, uppercase. */
//	push_ascii(dospwd, passwd, sizeof(dospwd), STR_UPPER|STR_TERMINATE);
	len = strlen(passwd);
	for (i = 0; i < len; i++) {
		char c = passwd[i];
		if (islower(c)) c = toupper(c);
		dospwd[i] = c;
	}
	dospwd[i] = 0;
       
	/* Only the fisrt 14 chars are considered, password need not be null terminated. */
	E_P16((const unsigned char *)dospwd, p16);

	if (strlen(dospwd) > 14) {
		ret = 0;
	}

	memset(dospwd, 0, sizeof(dospwd));
	// ZERO_STRUCT(dospwd);	

	return ret;
}



/* returns -1 if user is not present in /etc/passwd*/
int find_uid_for_user(char *user)
{
	char t[256];
	FILE *fp = fopen("/etc/passwd", "r");
	int ret = -1;

	if(!fp)
	{
		printf("failed to open /etc/passwd");
		goto out;
	}

	while(!feof(fp))
	{
		if(fgets(t, 255, fp))
		{
			char *p1, *p2;
			p1 = strchr(t, ':');
			if(p1 && (p1 - t == strlen(user)) && (strncmp(t, user, strlen(user))) == 0)
			{
				p1 = strchr(t, ':');
				if(!p1)
					goto out;
				p2 = strchr(++p1, ':');
				if(!p2)
					goto out;
				p1 = strchr(++p2, ':');
				if(!p1)
					goto out;
				*p1 = '\0';
				ret = atoi(p2);
				goto out;
			}
		}
	}
	printf("No valid user found in /etc/passwd\n");

out:
	if(fp)
		fclose(fp);
	return ret;
}

void insert_user_in_smbpasswd(char *user, char *line)
{
	char t[256];
	FILE *fp = fopen("/var/samba/smbpasswd", "r+");

	if(!fp)
	{
		printf("failed to open /var/samba/smbpasswd");
		goto out;
	}

	while(!feof(fp))
	{
		if(fgets(t, 255, fp))
		{
			char *p;
			p = strchr(t, ':');
			if(p && (p - t == strlen(user)) && (strncmp(t, user, strlen(user))) == 0)
			{
				fseek(fp, -strlen(line), SEEK_CUR);
				break;
			}
		}
	}

	fprintf(fp, line);

out:
	if(fp)
		fclose(fp);
}

void delete_user_from_smbpasswd(char *user)
{
	char t[256];
	FILE *fp = fopen("/var/samba/smbpasswd", "r+");

	if(!fp)
	{
		printf("failed to open /var/samba/smbpasswd");
		goto out;
	}

	while(!feof(fp))
	{
		if(fgets(t, 255, fp))
		{
			char *p;
			p = strchr(t, ':');
			if(p && (p - t == strlen(user)) && (strncmp(t, user, strlen(user))) == 0)
			{
				off_t r_pos, w_pos;
				char t2[256];
				r_pos = ftell(fp);
				w_pos = r_pos;
				w_pos -= strlen(t);
				while(fgets(t2, 256, fp))
				{
					fseek(fp,w_pos,SEEK_SET);
					fputs(t2, fp);
					r_pos += strlen(t2);
					w_pos += strlen(t2);
					fseek(fp,r_pos,SEEK_SET);
				}
				ftruncate(fileno(fp), w_pos);
				break;
			}
		}
	}

out:
	if(fp)
		fclose(fp);
}

int main(int argc, char **argv)
{
	unsigned uid;
	uchar new_nt_p16[NT_HASH_LEN];
	uchar new_lanman_p16[LM_HASH_LEN];
	int g;
	int smbpasswd_present;
	char smbpasswd_line[256];
	char *s;

	if(argc != 3)
	{
		printf("usage for openwrt_smbpasswd - \n\t%s USERNAME PASSWD\n\t%s -del USERNAME\n", argv[0], argv[0]);
		exit(1);
	}
	if(strcmp(argv[1], "-del") == 0)
	{
		printf("deleting user %s\n", argv[2]);
		delete_user_from_smbpasswd(argv[2]);
		return 0;
	}
	uid = find_uid_for_user(argv[1]);
	if(uid == -1)
		exit(2);

	E_md4hash(argv[2], new_nt_p16);

	if (!E_deshash(argv[2], new_lanman_p16)) {
		/* E_deshash returns false for 'long' passwords (> 14
		   DOS chars).  This allows us to match Win2k, which
		   does not store a LM hash for these passwords (which
		   would reduce the effective password length to 14 */

		memset(new_lanman_p16, 0, LM_HASH_LEN);
	}


	s = smbpasswd_line;
	s += snprintf(s, 256 - (s - smbpasswd_line), "%s:%u:", argv[1], uid);
	for(g = 0; g < 16; g++)
		s += snprintf(s, 256 - (s - smbpasswd_line), "%02X", new_lanman_p16[g]);
	s += snprintf(s, 256 - (s - smbpasswd_line), ":");
	
	for(g = 0; g < 16; g++)
		s += snprintf(s, 256 - (s - smbpasswd_line), "%02X", new_nt_p16[g]);
	snprintf(s, 256 - (s - smbpasswd_line), ":[UX         ]:LCT-00000001:\n");

	insert_user_in_smbpasswd(argv[1], smbpasswd_line);

	return 0;
}
