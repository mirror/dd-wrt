#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include <time.h>
#include <arpa/inet.h>

static struct blocklist blocklist_root;

static void init_blocklist(void)
{
	struct blocklist *entry = blocklist_root.next;
	struct blocklist *last = &blocklist_root;
	if (entry) {
		while (entry) {
			last = entry;
			entry = entry->next;
			free(last);
		}
		blocklist_root.next = NULL;
		entry = blocklist_root.next;
		last = &blocklist_root;
	}

	FILE *fp = NULL;
	if (jffs_mounted()) {
		fp = fopen("/jffs/blocklist", "rb");
	}
	if (!fp)
		fp = fopen("/tmp/blocklist", "rb");
	if (fp) {
		while (!feof(fp)) {
			last->next = malloc(sizeof(*entry));
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
}

int main(int argc, char *argv[])
{
	init_blocklist();

	struct blocklist *entry = blocklist_root.next;
	while (entry) {
		fprintf(stdout, "blocklist entry [%s]\t\tAttempts %d\tState(%d) %s\tFirst Time (%ld) %s\tBlocked Until (%ld) %s\n", entry->ip,
			entry->attempts, entry->blocked, entry->blocked == 1 ? "Blocked" : "Open", entry->seen,ctime(&entry->seen),
			entry->end, ctime(&entry->end));
		entry = entry->next;
	}
}