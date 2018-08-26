% This file illustrates how to read a binary file into a structure.  In this
% case, the file is the Unix utmp file.
%
% Note that the format of the utmp file will vary with the OS.  The format
% encoded here is for glibc Linux, but even that may be version-dependent.

variable format, size, fp, buf;

variable is_glibc = 1;

#ifeval is_glibc
typedef struct
{
   ut_type, ut_pid, ut_line, ut_id,
     ut_user, ut_host, ut_exit, ut_session, ut_tv, ut_addr
} UTMP_Type;
% The ut_tv is a timeval structure which has the format: l2
% Also the ut_exit field is a struct of h2
format = pad_pack_format ("h i S32 S4 S32 S256 h2 l l2 k4 x20");
#else
typedef struct
{
   ut_type, ut_pid, ut_line, ut_id,
     ut_time, ut_user, ut_host, ut_addr
} UTMP_Type;
format = pad_pack_format ("h i S12 S2 l S8 S16 l");
#endif

size = sizeof_pack (format);
vmessage ("Sizeof of utmp line: %d bytes", size);

define print_utmp (u)
{
   () = fprintf (stdout, "%-16s %-12s %-16s %s\n",
		 u.ut_user, u.ut_line, u.ut_host,
#ifeval is_glibc
		 ctime (u.ut_tv[0])
#else
		 ctime (u.ut_time)
#endif
		 );
}

variable Utmp_File;
foreach (["/var/run/utmp", "/var/log/utmp"])
{
   Utmp_File = ();
   fp = fopen (Utmp_File, "rb");
   if (fp != NULL)
     break;
}

if (fp == NULL) error ("Unable to open utmp file");

() = fprintf (stdout, "%-16s %-12s %-16s %s\n",
	      "USER", "TTY", "FROM", "LOGIN@");

variable U = @UTMP_Type;

while (size == fread_bytes (&buf, size, fp))
{
   set_struct_fields (U, unpack (format, buf));
   print_utmp (U);
}

() = fclose (fp);

