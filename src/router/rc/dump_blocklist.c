#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include <time.h>
#include <arpa/inet.h>


char *rfctime(const time_t timep, char *s)
{
	struct tm tm;
	localtime_r(&timep, &tm);
	strftime(s, 200, "%a, %d %b %Y %H:%M:%S", &tm); // spec for linksys
	return s;
}

int main(int argc, char *argv[])
{
	struct blocklist blocklist_root;
	struct blocklist *entry = blocklist_root.next;
	struct blocklist *last = &blocklist_root;

	FILE *fp = NULL;
	if (jffs_mounted()) {
		fp = fopen("/jffs/blocklist", "rb");
	}
	if (!fp)
		fp = fopen("/tmp/blocklist", "rb");
	if (fp) {
		while (!feof(fp)) {
			last->next = malloc(sizeof(*entry));
			memset(last->next, 0, sizeof(*entry));
			int elems = fread(last->next, sizeof(struct blocklist) - sizeof(void *), 1, fp);
			if (elems < 1) {
				free(last->next);
				last->next = NULL;
				break;
			}
			last = last->next;
		}
		fclose(fp);
	}

	entry = blocklist_root.next;
	fprintf(stdout, "Blocked Clients in Tarpit\n");
	while (entry) {
		char seen[128];
		char end[128];
		char release[128] = { 0 };
		rfctime(entry->seen, seen);
		rfctime(entry->end, end);
		switch (entry->blocked) {
		case -1:
			if (entry->end)
				rfctime(entry->end + ENDTIME, release);
			break;
		case 0:
			if (entry->ip[0] && entry->seen)
				rfctime(entry->seen + ENDTIME, release);
			break;
		}
		fprintf(stdout, "[%45s] Attempts %3d Count %3d State(%2d) %s First Time %32s%s %s\n", entry->ip, entry->attempts,
			entry->count, entry->blocked, entry->blocked == 1 ? "Blocked" : "Open   ", seen,
			entry->blocked == 1 && entry->end ? "\tBlocked Until" : "\tReleased On  ",
			entry->blocked == 1 && entry->end			    ? end :
			(entry->blocked == -1 || entry->blocked == 0) && release[0] ? release :
										      "");
		struct blocklist *old = entry;
		entry = entry->next;
		free(old);
	}
}