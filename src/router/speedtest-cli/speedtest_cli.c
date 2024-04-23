/*OB
 * Speedtest-cli
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdlib.h>

#define CONF_SERVER	"http://www.speedtest.net/speedtest-config.php"
#define STATIC_SERVER	"https://www.speedtest.net/api/js/servers"

#define DL_FILE_NUM 5
#define DL_FILE_TIMES 10
#define MAX_FILE_LEN 20
#define UL_FILE_NUM 5
#define UL_FILE_TIMES 10
#define DOWNLOADSIZE 5000000
#define UPLOADSIZE 2000000
static const char *search = NULL;
static int maxsearch = 10;
#define CLOSEST_SERVERS_NUM maxsearch

/* Debug Print */
#define DEBUG_NONE	0x000000
#define DEBUG_INFO	0x000001
static int debug_msg = DEBUG_NONE;

#define SPEEDTEST_INFO(fmt, arg...) \
		do { if (debug_msg & DEBUG_INFO) \
			printf("speedtest_cli >>%s: "fmt, __FUNCTION__, ##arg); } while (0)

#include <curl/curl.h>

static size_t countonly(void *mem, size_t num, size_t n, void *data)
{
	size_t *count = (size_t *)data;
	*count += n * num;
	return n * num;
}

size_t download(char *url, char *filename, int connecttimeout, int maxtimeout)
{
	CURL *hnd;
	hnd = curl_easy_init();
	size_t cnt = 0;
	FILE *out = NULL;
	if (filename) {
		out = fopen(filename, "wb");
		curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, fwrite);
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, out);
	} else {
		curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, countonly);
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &cnt);
	}
	curl_easy_setopt(hnd, CURLOPT_URL, url);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(hnd, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "dd-wrt speedtest");
	if (maxtimeout)
		curl_easy_setopt(hnd, CURLOPT_TIMEOUT_MS, (long)(maxtimeout * 1000));
	if (connecttimeout)
		curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT_MS, (long)(connecttimeout * 1000));

	CURLcode ret = curl_easy_perform(hnd);
	curl_easy_cleanup(hnd);
	if (out)
		fclose(out);
//	SPEEDTEST_INFO("downloaded %ld bytes from %s\n", cnt, url);

	return cnt;
}

size_t upload(char *url, char *filedata, int size, int connecttimeout, int maxtimeout)
{
	size_t cnt = 0;
	CURL *hnd;
	hnd = curl_easy_init();
	curl_off_t len = size;
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, countonly);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &cnt);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, filedata);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, len);
	curl_easy_setopt(hnd, CURLOPT_URL, url);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "dd-wrt speedtest");
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
//	SPEEDTEST_INFO("upload %ld bytes to %s\n", len, url);

	if (maxtimeout)
		curl_easy_setopt(hnd, CURLOPT_TIMEOUT_MS, (long)(maxtimeout * 1000));
	if (connecttimeout)
		curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT_MS, (long)(connecttimeout * 1000));

	CURLcode ret = curl_easy_perform(hnd);
	curl_easy_cleanup(hnd);
	return cnt;
}

static pthread_mutex_t finished_mutex = PTHREAD_MUTEX_INITIALIZER;

static double finished = 0;
static int dl_thread_num = 1;
static int ul_thread_num = 1;
static double time_dl_start;
static double time_ul_start;

typedef struct client_config {
	char *ip;
	char *lat;
	char *lon;
	char *isp;
	char *isprating;
	char *rating;
	char *ispdlavg;
	char *ispulavg;
	char *loggedin;
} client_config_t;

typedef struct server_config {
	char *url;
	char *lat;
	char *lon;
	char *name;
	char *country;
	char *cc;
	char *sponsor;
	char *id;
	char *preferred;
	char *https_functional;
	char *host;
	char *force_ping_select;
	double dist;		/* distance */
	double latency;
	double ping;
} server_config_t;

typedef struct dl_thread_arg {
	char *url;
} dl_thread_arg_t;

typedef struct ul_thread_arg {
	char *url;
	char *ul_file;
	int size;
} ul_thread_arg_t;

static int get_uptime(double *uptime)
{
	FILE *fp;
	char line[256];
	char *time = NULL;

	if (!(fp = fopen("/proc/uptime", "r"))) {
		perror("fopen /proc/uptime");
		return errno;
	}

	if (fgets(line, 255, fp)) {
		fclose(fp);
		if ((time = strtok(line, " ")) != NULL)
			*uptime = atof(time);
		else
			return -1;
	} else {
		fclose(fp);
		return -1;
	}

	return 0;
}

static void client_free(client_config_t * client)
{
	if (client->ip)
		free(client->ip);
	if (client->lat)
		free(client->lat);
	if (client->lon)
		free(client->lon);
	if (client->isp)
		free(client->isp);
	if (client->isprating)
		free(client->isprating);
	if (client->rating)
		free(client->rating);
	if (client->ispdlavg)
		free(client->ispdlavg);
	if (client->ispulavg)
		free(client->ispulavg);
	if (client->loggedin)
		free(client->loggedin);
}

static char *get_str(char *in, char *line)
{
	int line_len, in_len, i, j;
	int val_len = 0, val_index;
	char *val;

	line_len = strlen(line);
	in_len = strlen(in);

	for (i = 1; i < line_len; i++) {
		if ((line[i] == in[0]) && line[i - 1] == ' ' && !strncmp(&line[i], in, in_len)) {
			break;
		}
	}

	if (i == line_len) {
		return NULL;
	}

	j = i + in_len + 1;
	for (; j < line_len; j++) {
		if ((line[j] == '\"') && (val_len == 0)) {
			val_len++;
			val_index = j + 1;
		} else if ((line[j] == '\"') && (val_len != 0)) {
			val_len--;
			val = (char *)malloc(val_len + 1);
			strncpy(val, (char *)&line[val_index], val_len);
			val[val_len] = '\0';
			return val;
		} else if (val_len > 0) {
			val_len++;
		}
	}

	return NULL;
}

static char *get_str_json(char *search, char **p)
{
	char *buf = *p;
	char s[32];
	snprintf(s, sizeof(s), "\"%s\"", search);
	char *look = strstr(buf, s);
	if (!look) {
		return NULL;
	}
	look += strlen(s) + 2;
	char *ret;
	char *orig = ret = malloc(128);
	char *dest = strchr(look, '"');
	if (!dest) {
		return NULL;
	}
	char *i;
	for (i = look; i < dest; i++) {
		if (*i == '\\') {
			continue;
		}
		if ((ret - orig) < 128) {
			*ret = *i;
			ret++;
		}
	}
	*ret = 0;
	*p = i;
	return orig;
}

static int get_speedtest_config(client_config_t * client)
{
	FILE *fp1;
	char line[256];

	SPEEDTEST_INFO(CONF_SERVER "\n");
	download(CONF_SERVER, "/tmp/speedtest-config.php", 0, 0);

	if (!(fp1 = fopen("/tmp/speedtest-config.php", "r"))) {
		perror("fopen /tmp/speedtest-config.php");
		return errno;
	}

	while (fgets(line, 255, fp1)) {
		if (!strncmp(line, "<client", 7)) {
			if ((client->ip = get_str("ip", line)) == NULL) {
				client_free(client);
				return -1;
			}

			if ((client->lat = get_str("lat", line)) == NULL) {
				client_free(client);
				return -1;
			}
			if ((client->lon = get_str("lon", line)) == NULL) {
				client_free(client);
				return -1;
			}
			if ((client->isp = get_str("isp", line)) == NULL) {
				client_free(client);
				return -1;
			}
			if ((client->isprating = get_str("isprating", line)) == NULL) {
				client_free(client);
				return -1;
			}
			if ((client->rating = get_str("rating", line)) == NULL) {
				client_free(client);
				return -1;
			}
			if ((client->ispdlavg = get_str("ispdlavg", line)) == NULL) {
				client_free(client);
				return -1;
			}
			if ((client->ispulavg = get_str("ispulavg", line)) == NULL) {
				client_free(client);
				return -1;
			}
			if ((client->loggedin = get_str("loggedin", line)) == NULL) {
				client_free(client);
				return -1;
			}
			break;
		}
	}
	fclose(fp1);

	unlink("/tmp/speedtest-config.php");

	return 0;
}

static void server_free(server_config_t * server)
{
	if (server->url)
		free(server->url);
	if (server->lat)
		free(server->lat);
	if (server->lon)
		free(server->lon);
	if (server->name)
		free(server->name);
	if (server->country)
		free(server->country);
	if (server->cc)
		free(server->cc);
	if (server->sponsor)
		free(server->sponsor);
	if (server->id)
		free(server->id);
	if (server->preferred)
		free(server->preferred);
	if (server->https_functional)
		free(server->https_functional);
	if (server->host)
		free(server->host);
	if (server->force_ping_select)
		free(server->force_ping_select);
}

static void init_server(server_config_t * server)
{
	memset(server, 0, sizeof(server_config_t));
}

/* calculate distance between client and server */
static double get_distance(client_config_t * client, server_config_t * server)
{
	double lat_c, lat_s, lon_c, lon_s, d, a;
	double dlat, dlon;
	int radius = 6371;	/* earth radius */

	lat_c = atof(client->lat);
	lon_c = atof(client->lon);
	lat_s = atof(server->lat);
	lon_s = atof(server->lon);
	dlat = (lat_s - lat_c) * M_PI / 180;
	dlon = (lon_s - lon_c) * M_PI / 180;

	a = sin(dlat / 2) * sin(dlat / 2) + cos(lat_c) * cos(lat_s) * sin(dlon / 2) * sin(dlon / 2);
	d = radius * 2 * atan2(sqrt(a), sqrt(1 - a));

	return d;
}

/* Determine the 5 nearest speedtest.net servers based on geographic distance */
static int get_nearest_servers(client_config_t * client, server_config_t * servers)
{
	FILE *fp1;
	char line[256];
	server_config_t server;
	int j, k;
	int i;
	char url[128];
	if (search)
		snprintf(url, sizeof(url), "%s?search=%s&limit=%d", STATIC_SERVER, search, maxsearch);
	else
		snprintf(url, sizeof(url), "%s?limit=%d", STATIC_SERVER, maxsearch);
	SPEEDTEST_INFO("%s\n", url);

	download(url, "/tmp/speedtest-servers.php", 0, 0);
	if (!(fp1 = fopen("/tmp/speedtest-servers.php", "r"))) {
		perror("fopen /tmp/speedtest-servers.php");
		return errno;
	}
	init_server(&server);

	fseek(fp1, 0, SEEK_END);
	size_t len = ftell(fp1);
	rewind(fp1);
	char *orig;
	char *buf = orig = malloc(len + 1);
	buf[len] = 0;
	fread(buf, len, 1, fp1);
	fclose(fp1);
	int ccc = 0;
	while (1) {
		if ((server.url = get_str_json("url", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.lat = get_str_json("lat", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.lon = get_str_json("lon", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.name = get_str_json("name", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.country = get_str_json("country", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.cc = get_str_json("cc", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.sponsor = get_str_json("sponsor", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.id = get_str_json("id", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.preferred = get_str_json("preferred", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.https_functional = get_str_json("https_functional", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.host = get_str_json("host", &buf)) == NULL) {
			server_free(&server);
			init_server(&server);
			break;
		}
		if ((server.force_ping_select = get_str_json("force_ping_select", &buf)) == NULL) {
			// ignore
		}

		/* calculate distance between client and server */
		server.dist = get_distance(client, &server);

		for (j = 0; j < CLOSEST_SERVERS_NUM; j++) {
			if (servers[j].url == NULL) {
				servers[j].url = server.url;
				servers[j].lat = server.lat;
				servers[j].lon = server.lon;
				servers[j].name = server.name;
				servers[j].country = server.country;
				servers[j].cc = server.cc;
				servers[j].sponsor = server.sponsor;
				servers[j].id = server.id;
				servers[j].dist = server.dist;
				init_server(&server);
				break;
			} else {
				if (server.dist <= servers[j].dist) {
					server_free(&servers[CLOSEST_SERVERS_NUM - 1]);
					init_server(&servers[CLOSEST_SERVERS_NUM - 1]);
					for (k = CLOSEST_SERVERS_NUM - 1; k > j; k--) {
						servers[k].url = servers[k - 1].url;
						servers[k].lat = servers[k - 1].lat;
						servers[k].lon = servers[k - 1].lon;
						servers[k].name = servers[k - 1].name;
						servers[k].country = servers[k - 1].country;
						servers[k].cc = servers[k - 1].cc;
						servers[k].sponsor = servers[k - 1].sponsor;
						servers[k].id = servers[k - 1].id;
						servers[k].dist = servers[k - 1].dist;
					}
					servers[j].url = server.url;
					servers[j].lat = server.lat;
					servers[j].lon = server.lon;
					servers[j].name = server.name;
					servers[j].country = server.country;
					servers[j].cc = server.cc;
					servers[j].sponsor = server.sponsor;
					servers[j].id = server.id;
					servers[j].dist = server.dist;
					init_server(&server);
					break;
				} else {
					if (j == CLOSEST_SERVERS_NUM - 1) {
						server_free(&server);
						init_server(&server);
					}
				}
			}
		}
	}

	free(orig);
	unlink("/tmp/speedtest-servers.php");
	return 0;
}
double ping(char *addr);

static int get_lowest_latency_server(server_config_t * servers, server_config_t * best_server)
{
	int i, j, len, best;
	char *url = NULL;
	struct timeval tv1, tv2;
	FILE *fp1;
	char line[12];
	double latency[3], lat;

	for (i = 0; i < CLOSEST_SERVERS_NUM; i++) {
		if (!servers[i].url)
			break;
		len = strlen(servers[i].url);
		url = malloc(len - strlen("upload.php") + strlen("latency.txt") + 1);
		strncpy(url, servers[i].url, len - strlen("upload.php"));
		url[len - strlen("upload.php")] = '\0';
		strcat(url, "latency.txt");

		SPEEDTEST_INFO("%s\n", url);
		for (j = 0; j < 3; j++) {
			gettimeofday(&tv1, NULL);
			download(url, "/tmp/latency.txt", 5, 5);
			gettimeofday(&tv2, NULL);

			if (!(fp1 = fopen("/tmp/latency.txt", "r"))) {
				perror("fopen /tmp/latency.txt");
				latency[j] = 3600000000;
				continue;
			}
			fgets(line, sizeof(line), fp1);
			if (!strncmp(line, "test=test", strlen("test=test"))) {
				latency[j] = (double)((tv2.tv_sec - tv1.tv_sec)) * (double)(1000000.0f) + (double)(tv2.tv_usec - tv1.tv_usec);
			} else {
				latency[j] = 3600000000;
			}
			fclose(fp1);
			unlink("/tmp/latency.txt");
			memset(line, 0, sizeof(line));
		}
		servers[i].latency = (latency[0] + latency[1] + latency[2]) / 3;
		free(url);
		url = NULL;
	}

	for (i = 0; i < CLOSEST_SERVERS_NUM; i++) {
		if (!servers[i].url)
			break;
		if (i == 0) {
			best = i;
			lat = servers[i].latency;
		} else {
			if (servers[i].latency < lat) {
				best = i;
				lat = servers[i].latency;
			}
		}
	}

	best_server->url = servers[best].url;
	best_server->lat = servers[best].lat;
	best_server->lon = servers[best].lon;
	best_server->name = servers[best].name;
	best_server->country = servers[best].country;
	best_server->cc = servers[best].cc;
	best_server->sponsor = servers[best].sponsor;
	best_server->id = servers[best].id;
	best_server->dist = servers[best].dist;
	best_server->latency = servers[best].latency;
	best_server->ping = ping(best_server->url);
	return 0;
}

static void *download_thread(void *ptr)
{
	dl_thread_arg_t *in;
	struct stat file_stat;
	double time_diff, time_thread;

	if (get_uptime(&time_thread)) {
		fprintf(stderr, "Error on getting /proc/uptime\n");
		return NULL;
	}
	time_diff = (time_thread - time_dl_start);
	if (time_diff > 10) {
		return NULL;
	}

	in = (dl_thread_arg_t *) ptr;
	size_t cnt = download(in->url, NULL, 0, 0);
	pthread_mutex_lock(&finished_mutex);
	finished += (double)cnt;
	pthread_mutex_unlock(&finished_mutex);

	return NULL;
}

typedef struct THREAD {
	pthread_t q;
	int joined;
} THREAD_T;

static int test_download_speed(server_config_t * best_server)
{
	int i, j, k = 0, ret, url_len, queue_count = 0;
	dl_thread_arg_t download_url[DL_FILE_NUM * DL_FILE_TIMES];
	THREAD_T q[dl_thread_num];
	double duration;
	double time_dl_end;
	FILE *fp_result;
	memset(q, 0, sizeof(q));
	SPEEDTEST_INFO("%s\n", best_server->url);
	best_server->url[strlen(best_server->url) - strlen("speedtest/upload.php")] = '\0';

	for (i = 0; i < DL_FILE_NUM; i++) {
		for (j = 0; j < DL_FILE_TIMES; j++) {
			asprintf(&download_url[k].url, "%sdownload?size=%d", best_server->url, DOWNLOADSIZE);
			k++;
		}
	}

	if (get_uptime(&time_dl_start)) {
		fprintf(stderr, "Error on getting /proc/uptime\n");
		return -1;
	}

	while (1) {
		for (i = 0; i < (DL_FILE_NUM * DL_FILE_TIMES); i++) {
			if (queue_count < dl_thread_num) {
				for (j = 0; j < (dl_thread_num - 1); j++) {
					q[dl_thread_num - j - 1] = q[dl_thread_num - j - 2];
				}
				q[0].joined = 0;
				ret = pthread_create(&q[0].q, NULL, download_thread, (void *)&download_url[i]);
				queue_count++;
			}
			if (queue_count == dl_thread_num) {
				if (i == (DL_FILE_NUM * DL_FILE_TIMES - 1)) {
					/* all task have been put in queue, consume all threads in queue */
					for (j = 0; j < dl_thread_num; j++) {
						q[dl_thread_num - 1 - j].joined = 1;
						pthread_join(q[dl_thread_num - 1 - j].q, NULL);
						queue_count--;
					}

				} else {
					/* consume a thread in queue to provide space for next task */

					q[dl_thread_num - 1].joined = 1;
					pthread_join(q[dl_thread_num - 1].q, NULL);
					queue_count--;
				}
			}
			if (get_uptime(&time_dl_end)) {
				fprintf(stderr, "Error on getting /proc/uptime\n");
				return -1;
			}
			duration = time_dl_end - time_dl_start;
			if (duration > 10.0) {	// limit upload  
				for (j = 0; j < dl_thread_num; j++) {
					if (!q[dl_thread_num - 1 - j].joined)
						pthread_join(q[dl_thread_num - 1 - j].q, NULL);
				}
				goto done;
			}
		}
	}
      done:;
	if (get_uptime(&time_dl_end)) {
		fprintf(stderr, "Error on getting /proc/uptime\n");
		return -1;
	}
	duration = time_dl_end - time_dl_start;
	for (i = 0; i < DL_FILE_NUM * DL_FILE_TIMES; i++) {
		free(download_url[i].url);
	}
	printf("speedtest_cli: Duration = %.2f Download = %.2f Mbit/s (%.2f Kbyte/s)\n", duration, ((finished / 1024 / 1024 / duration) * 8), (finished / 1024 / duration));

	if (!(fp_result = fopen("/tmp/speedtest_download_result", "w"))) {
		perror("fopen /tmp/speedtest_download_result");
		return errno;
	}
	fprintf(fp_result, "%.2f", ((finished / 1024 / 1024 / duration) * 8));
	fclose(fp_result);

	return 0;
}

static void *upload_thread(void *ptr)
{
	ul_thread_arg_t *in;
	struct stat file_stat;
	double time_diff;
	double time_thread;

	if (get_uptime(&time_thread)) {
		fprintf(stderr, "Error on getting /proc/uptime\n");
		return NULL;
	}
	time_diff = time_thread - time_ul_start;
	if (time_diff > 10)
		return NULL;

	in = (ul_thread_arg_t *) ptr;
	size_t size =  upload(in->url, in->ul_file, in->size, 0, 0);

	pthread_mutex_lock(&finished_mutex);
	finished += (double)in->size;
	pthread_mutex_unlock(&finished_mutex);

	return NULL;
}

static int test_upload_speed(server_config_t * best_server)
{
	const char *data = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char *head = "content1=";
	const char *tail = "0123456789ABCDEFGHIJKLMNOPQ";
	double duration;
	double time_ul_end;
	int i, j, data_len, queue_count = 0;
	ul_thread_arg_t upload_arg[UL_FILE_NUM * UL_FILE_TIMES];
	THREAD_T q[ul_thread_num];
	FILE *fp_result;

	SPEEDTEST_INFO("%s\n", best_server->url);
	char *mem;
	size_t datalen = UPLOADSIZE + strlen(head) + strlen(tail);
	char *databuf = mem = malloc(datalen);
	databuf += snprintf(databuf, datalen, "%s", head);
	data_len = (int)round(UPLOADSIZE / strlen(data));
	for (j = 0; j < (data_len - 1); j++) {
		databuf += snprintf(databuf, datalen - ((size_t)databuf - (size_t)mem), "%s", data);
	}
	snprintf(databuf, datalen - ((size_t)databuf - (size_t)mem), "%s", tail);
	for (i = 0; i < (UL_FILE_NUM * UL_FILE_TIMES); i++) {
		asprintf(&upload_arg[i].url, "%supload", best_server->url);
		upload_arg[i].size = (int)round(UPLOADSIZE / strlen(databuf)) * strlen(databuf);
		upload_arg[i].ul_file = mem;
	}
	if (get_uptime(&time_ul_start)) {
		fprintf(stderr, "Error on getting /proc/uptime\n");
		return -1;
	}
	while (1) {
		for (i = 0; i < (UL_FILE_NUM * UL_FILE_TIMES); i++) {
			if (queue_count < ul_thread_num) {
				for (j = 0; j < (ul_thread_num - 1); j++) {
					q[ul_thread_num - j - 1] = q[ul_thread_num - j - 2];
				}
				q[0].joined = 0;
				pthread_create(&q[0].q, NULL, upload_thread, (void *)&upload_arg[i]);
				queue_count++;
			}
        fprintf(stderr, "%d\n",__LINE__);
			if (queue_count == ul_thread_num) {
				if (i == ((UL_FILE_NUM * UL_FILE_TIMES) - 1)) {
					/* all task have been put in queue, consume all threads in queue */
					for (j = 0; j < ul_thread_num; j++) {
						q[ul_thread_num - 1 - j].joined = 1;
						pthread_join(q[ul_thread_num - 1 - j].q, NULL);
						queue_count--;
					}

				} else {
					/* consume a thread in queue to provide space for next task */
					q[ul_thread_num - 1].joined = 1;
					pthread_join(q[ul_thread_num - 1].q, NULL);
					queue_count--;
				}
			}

			if (get_uptime(&time_ul_end)) {
				fprintf(stderr, "Error on getting /proc/uptime\n");
				return -1;
			}
			duration = time_ul_end - time_ul_start;
			if (duration > 10.0) {	// limit upload  
				for (j = 0; j < ul_thread_num; j++) {
					if (!q[ul_thread_num - 1 - j].joined) {
						pthread_join(q[ul_thread_num - 1 - j].q, NULL);
					}
				}
				goto done;
			}

		}
	}
      done:;
	free(mem);
	if (get_uptime(&time_ul_end)) {
		fprintf(stderr, "Error on getting /proc/uptime\n");
		return -1;
	}
	duration = time_ul_end - time_ul_start;
	printf("speedtest_cli: Duration %.2f Upload = %.2f Mbit/s (%.2f Kbyte/s)\n", duration, ((finished / 1024 / 1024 / duration) * 8), (finished / 1024 / duration));

	if (!(fp_result = fopen("/tmp/speedtest_upload_result", "w"))) {
		perror("fopen /tmp/speedtest_upload_result");
		return errno;
	}
	fprintf(fp_result, "%.2f", ((finished / 1024 / 1024 / duration) * 8));
	fclose(fp_result);

	return 0;
}

static void init_client(client_config_t * client)
{
	memset(client, 0, sizeof(client_config_t));
}

static int speedtest(int dl_enable, int ul_enable)
{
	int i;
	client_config_t client;
	server_config_t *servers;
	server_config_t best_server;
	servers = calloc(maxsearch, sizeof(*servers));

	for (i = 0; i < CLOSEST_SERVERS_NUM; i++) {
		init_server(&servers[i]);
	}
	init_server(&best_server);
	init_client(&client);

	if (get_speedtest_config(&client)) {
		fprintf(stderr, "get_speedtest_config error!\n");
		return -1;
	}

	if (get_nearest_servers(&client, servers)) {
		fprintf(stderr, "get_nearest_servers error!\n");
		return -1;
	}

	client_free(&client);

	if (get_lowest_latency_server(servers, &best_server)) {
		fprintf(stderr, "get_lowest_latency_server error!\n");
		return -1;
	}

	FILE *fp = fopen("/tmp/speedtest_name", "wb");
	fprintf(fp, "%s", best_server.name);
	fclose(fp);
	fp = fopen("/tmp/speedtest_country", "wb");
	fprintf(fp, "%s", best_server.country);
	fclose(fp);
	fp = fopen("/tmp/speedtest_sponsor", "wb");
	fprintf(fp, "%s", best_server.sponsor);
	fclose(fp);
	fp = fopen("/tmp/speedtest_latency", "wb");
	fprintf(fp, "%.2f", best_server.ping);
	fclose(fp);

	if (dl_enable == 1) {
		if (test_download_speed(&best_server)) {
			fprintf(stderr, "test_download_speed error!\n");
			return -1;
		}
	}

	if (ul_enable == 1) {
		finished = (double)0;
		if (test_upload_speed(&best_server)) {
			fprintf(stderr, "test_upload_speed error!\n");
			return -1;
		}
	}
	for (i = 0; i < CLOSEST_SERVERS_NUM; i++) {
		server_free(&servers[i]);
	}
	free(servers);
	return 0;
}

static void usage(void)
{
	printf("usage:\n");
	printf("speedtest_cli [options] [1|0](download test enable|disable) ");
	printf("[download thread number] ");
	printf("[1|0](upload test enable|disable) [upload thread number]\n");
	printf("Range of thread number: 1 - 32\n");
	printf("\noptions\n");
	printf("\t-d\n");
	printf("\t\tturn on debug message\n");
	printf("\t\tex. speedtest_cli -d 1 3 1 2 [search] [max results]\n");
}

int main(int argc, char **argv)
{
	int num, i = 1;
	int dl_enable, ul_enable;
	curl_global_init(CURL_GLOBAL_ALL);

	if (argc != 5 && argc != 6 && argc != 7 && argc != 8) {
		usage();
		return 0;
	} else {
		if (!strcmp(argv[i], "-d")) {
				i++;
				debug_msg = DEBUG_INFO;
		} else {
			debug_msg = DEBUG_NONE;
		
		}
		
		if (!strcmp(argv[i], "1")) {
			dl_enable = 1;
		} else if (!strcmp(argv[i], "0")) {
			dl_enable = 0;
		} else {
			usage();
			return 0;
		}
		i++;

		num = atoi(argv[i]);
		if ((num < 1) || (num > 32)) {
			usage();
			return 0;
		} else {
			dl_thread_num = num;
		}
		i++;

		if (!strcmp(argv[i], "1")) {
			ul_enable = 1;
		} else if (!strcmp(argv[i], "0")) {
			ul_enable = 0;
		} else {
			usage();
			return 0;
		}
		i++;

		num = atoi(argv[i]);
		if ((num < 1) || (num > 32)) {
			usage();
			return 0;
		} else {
			ul_thread_num = num;
		}
		i++;
		if (i < argc)
			search = argv[i];
		i++;
		if (i < argc)
			maxsearch = atoi(argv[i]);

	}

	if ((dl_enable == 0) && (ul_enable == 0)) {
		return 0;
	}

	if (speedtest(dl_enable, ul_enable)) {
		fprintf(stderr, "speedtest error\n");
		return -1;
	}

	return 0;
}
