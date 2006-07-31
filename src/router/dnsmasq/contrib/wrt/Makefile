CFLAGS?= -O2

all: dhcp_release.c
	$(CC) $(CFLAGS) $(RPM_OPT_FLAGS) -Wall -W dhcp_release.c -o dhcp_release

clean:
	rm -f *~ *.o core dhcp_release
