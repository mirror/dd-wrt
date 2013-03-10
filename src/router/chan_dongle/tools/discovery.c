#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#include <asterisk.h>
#include <asterisk/linkedlists.h>		/* AST_LIST_ENTRY() */

#include "mutils.h"
#include "tty.h"

struct dev_descr {
	AST_LIST_ENTRY (dev_descr) entry;

	int		busnum;
	char		devpath[40];
	int		configuration;
	int		interfaceno;
	char		port[80];
};

typedef AST_LIST_HEAD_NOLOCK(, dev_descr) dev_list_t;

static const char sys_driver[] = "/sys/bus/usb/drivers";
static const char port_number[] = "port_number";

#/* */
char * read_result(int fd)
{
	unsigned total = 0;
	char buf[4096];
	char * found;
	int readed;
	
	while(1) {
		readed = read(fd, buf + total, sizeof(buf) - total);
		if(readed <= 0)
			return NULL;
		total += readed;
//		fprintf(stdout, "%*s", readed, buf);
		if((found = memmem(buf, total, "\r\nOK\r\n", 6)) != NULL) {
			found[0] = 0;
			return strdup(buf);
		} else if((found = memmem(buf, total, "\r\nERROR\r\n", 9)) != NULL) {
			return NULL;
		}
	}
	return NULL;
}

#/* */
unsigned count_lines(const char * lines)
{
	unsigned lno = 1;
	const char * str = lines;
	
	while(1) {
		const char * x = strstr(str, "\r\n");
		if(x == NULL)
			break;
		str = x + 2;
		lno++;
	}

	return lno;
}

#/* */
char ** read_results(int fd, int * argc)
{
	int arg;
	char * str;
	char * x;
	char ** argv;
	int lno;
	
	char * lines = read_result(fd);

	if(!lines) {
		argc = 0;
		return NULL;
	}

	lno = count_lines(lines);
	argv = malloc((lno + 1) * sizeof(lines));
	
	for(arg = 0, str = lines; arg < lno; ) {
		x = strstr(str, "\r\n");
		if(x == NULL)
			break;
		x[0] = 0;
		if(str[0] != 0)
			argv[arg++] = strdup(str);
		str = x + 2;
	}
	free(lines);
	argv[arg] = NULL;

	*argc = arg;
	return argv;
}

#/* */
void free_results(char ** argv)
{
	if(argv) {
		unsigned idx;
		for(idx = 0; argv[idx]; idx++) {
			free(argv[idx]);
		}
		free(argv);
	}
}

#/* */
int discovery_port(struct dev_descr * descr, const char * name)
{
	char pname[PATH_MAX];
	struct dirent * entry;
	struct stat statb;

	DIR * dir = opendir(name);
	if(dir) {
		while((entry = readdir(dir)) != NULL) {
			snprintf(pname, sizeof(pname), "%s/%s/%s", name, entry->d_name, port_number);
			if(stat(pname, &statb) == 0) {
				snprintf(descr->port, sizeof(descr->port), "/dev/%s", entry->d_name);
				return 1;
			}
		}
		closedir(dir);
	}
	return 0;
}

#/* */
char * get_info_item(char ** argv, const char * name, unsigned len)
{
	for(; argv[0]; argv++) {
		if(strncmp(argv[0], name, len) == 0) {
			char * found = argv[0] + len;
			while(found[0] == ' ')
				found++;
			return strdup(found);
		}
	}
	return NULL;
}

#/* */
int get_info(const char * port, char ** manu, char ** model, char ** imei, char ** imsi)
{
	int fd = opentty(port);
	*manu = *model = *imei = *imsi = 0;
	if(fd >= 0) {
		static const char ati[] = "ATI\r";
		static const char cimi[] = "AT+CIMI\r";
		char **argv;
		int argc;
		
		write_all(fd, ati, STRLEN(ati));
		argv = read_results(fd, &argc);
		if(argv) {
			static const char s_manufactorer[] = "Manufacturer:";
			static const char s_model[] = "Model:";
			static const char s_imei[] = "IMEI:";
			

			*manu = get_info_item(argv, s_manufactorer, STRLEN(s_manufactorer));
			*model = get_info_item(argv, s_model, STRLEN(s_model));
			*imei = get_info_item(argv, s_imei, STRLEN(s_imei));

			free_results(argv);
		}
		
		write_all(fd, cimi, STRLEN(cimi));
		argv = read_results(fd, &argc);
		if(argv) {
			int x = 0;
			if(strncmp(argv[x], cimi, STRLEN(cimi)) == 0)
				x++;
			*imsi = strdup(argv[x]);
			free_results(argv);
		}
		closetty(port, fd);
	}

	return *manu ||  *model || *imei || *imsi;
}

#/* */
int discovery_all(dev_list_t * devs)
{
	struct dev_descr * dev;

	AST_LIST_TRAVERSE(devs, dev, entry) {
		if(dev->interfaceno == 0) {
			char * manu, * model, * imei, * imsi;
			fprintf(stdout, "Bus: %d Dev: %s Conf: %d\n", dev->busnum, dev->devpath, dev->configuration);
			if(get_info(dev->port, &manu, &model, &imei, &imsi)) {
				fprintf(stdout, "Manufacturer: %s  Model: %s IMEI: %s IMSI: %s\n", manu, model, imei, imsi);
				free(manu);
				free(model);
				free(imei);
				free(imsi);
			}
		}
		fprintf(stdout, "\tInterface: %d Port: %s\n", dev->interfaceno, dev->port);
	}
	return 0;
}

/*
 * /sys/bus/usb/drivers/option
 *  1-1:1.0
 *  1-1:1.1
 *  1-1:1.2
 *
*/

#/* */
void discovery_driver(const char * driver)
{
	char realname[PATH_MAX];
	char name[PATH_MAX];
	char * name2;
	DIR * dir;
	struct dirent * entry;
	int len;
	dev_list_t devs;

	len = snprintf(name, sizeof(name), "%s/%s", sys_driver, driver);
	dir = opendir(name);
	if(dir) {
		while((entry = readdir(dir)) != NULL) {
			struct dev_descr * descr = malloc(sizeof(*descr));
			/* check numbers */
//			if(entry->d_type == &&  sscanf(entry->d_name, "%u:%u-%u:%u", &id1, &id2, &id3, &id4) == 4)
			if(sscanf(entry->d_name, "%d-%39[^:]:%d.%d", &descr->busnum, descr->devpath, &descr->configuration, &descr->interfaceno) == 4)
			{
//				fprintf(stdout, "device %s\n", entry->d_name);
				snprintf(name + len, sizeof(name) - len, "/%s", entry->d_name);

				if(realpath(name, realname) != NULL)
					name2 = realname;
				else {
					name2 = name;
				}

				if(discovery_port(descr, name2)) {
					AST_LIST_INSERT_TAIL(&devs, descr, entry);
					continue;
				}
			}
			free(descr);
		}
		closedir(dir);
	}

	discovery_all(&devs);
}

#/* */
int main(int argc, char * argv[])
{
	int arg;

	discovery_driver("option");
	for(arg = 1; arg < argc; ++arg)
		discovery_driver(argv[arg]);
	return 0;
}
