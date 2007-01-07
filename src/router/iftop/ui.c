/*
 * ui.c:
 *
 */

#include <sys/types.h>

#include <ctype.h>
#include <curses.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/wait.h>

#include "addr_hash.h"
#include "serv_hash.h"
#include "iftop.h"
#include "resolver.h"
#include "sorted_list.h"
#include "options.h"
#include "screenfilter.h"

#define HOSTNAME_LENGTH 256

#define HISTORY_DIVISIONS   3

#define HELP_TIME 2

#define HELP_MESSAGE \
"Host display:                          General:\n"\
" n - toggle DNS host resolution         P - pause display\n"\
" s - toggle show source host            h - toggle this help display\n"\
" d - toggle show destination host       b - toggle bar graph display\n"\
" t - cycle line display mode            B - cycle bar graph average\n"\
"                                        T - toggle cummulative line totals\n"\
"Port display:                           j/k - scroll display\n"\
" N - toggle service resolution          f - edit filter code\n"\
" S - toggle show source port            l - set screen filter\n"\
" D - toggle show destination port       L - lin/log scales\n"\
" p - toggle port display                ! - shell command\n"\
"                                        q - quit\n"\
"Sorting:\n"\
" 1/2/3 - sort by 1st/2nd/3rd column\n"\
" < - sort by source name\n"\
" > - sort by dest name\n"\
" o - freeze current order\n"\
"\n"\
"iftop, version " IFTOP_VERSION 


/* 2, 10 and 40 seconds */
int history_divs[HISTORY_DIVISIONS] = {1, 5, 20};

#define UNIT_DIVISIONS 4
char* unit_bits[UNIT_DIVISIONS] =  { "b", "Kb", "Mb", "Gb"};
char* unit_bytes[UNIT_DIVISIONS] =  { "B", "KB", "MB", "GB"};

typedef struct host_pair_line_tag {
    addr_pair ap;
    double long total_recv;
    double long total_sent;
    double long recv[HISTORY_DIVISIONS];
    double long sent[HISTORY_DIVISIONS];
} host_pair_line;


extern hash_type* history;
extern int history_pos;
extern int history_len;

extern options_t options ;

void ui_finish();

hash_type* screen_hash;
hash_type* service_hash;
sorted_list_type screen_list;
host_pair_line totals;
int peaksent, peakrecv, peaktotal;

#define HELP_MSG_SIZE 80
int showhelphint = 0;
int persistenthelp = 0;
time_t helptimer = 0;
char helpmsg[HELP_MSG_SIZE];
int dontshowdisplay = 0;

/*
 * Compare two screen lines based on bandwidth.  Start comparing from the 
 * specified column
 */
int screen_line_bandwidth_compare(host_pair_line* aa, host_pair_line* bb, int start_div) {
    int i;
    for(i = start_div; i < HISTORY_DIVISIONS; i++) {
        if(aa->recv[i] + aa->sent[i] != bb->recv[i] + bb->sent[i]) {
            return(aa->recv[i] + aa->sent[i] < bb->recv[i] + bb->sent[i]);
        }
    }
    return 1;
}

/*
 * Compare two screen lines based on hostname / IP.  Fall over to compare by
 * bandwidth.
 */
int screen_line_host_compare(struct in_addr* a, struct in_addr* b, host_pair_line* aa, host_pair_line* bb) {
    char hosta[HOSTNAME_LENGTH], hostb[HOSTNAME_LENGTH];
    int r;

    /* This isn't overly efficient because we resolve again before 
       display. */
    if (options.dnsresolution) {
        resolve(a, hosta, HOSTNAME_LENGTH);
        resolve(b, hostb, HOSTNAME_LENGTH);
    }
    else {
        strcpy(hosta, inet_ntoa(*a));
        strcpy(hostb, inet_ntoa(*b));
    }

    r = strcmp(hosta, hostb);

    if(r == 0) {
        return screen_line_bandwidth_compare(aa, bb, 2);
    }
    else {
        return (r > 0);
    }


}

int screen_line_compare(void* a, void* b) {
    host_pair_line* aa = (host_pair_line*)a;
    host_pair_line* bb = (host_pair_line*)b;
    if(options.sort == OPTION_SORT_DIV1) {
      return screen_line_bandwidth_compare(aa, bb, 0);
    }
    else if(options.sort == OPTION_SORT_DIV2) {
      return screen_line_bandwidth_compare(aa, bb, 1);
    }
    else if(options.sort == OPTION_SORT_DIV3) {
      return screen_line_bandwidth_compare(aa, bb, 2);
    }
    else if(options.sort == OPTION_SORT_SRC) {
      return screen_line_host_compare(&(aa->ap.src), &(bb->ap.src), aa, bb);
    }
    else if(options.sort == OPTION_SORT_DEST) {
      return screen_line_host_compare(&(aa->ap.dst), &(bb->ap.dst), aa, bb);
    }

    return 1;
}

void readable_size(float n, char* buf, int bsize, int ksize, int bytes) {

    int i = 0;
    float size = 1;

    /* Convert to bits? */
    if(bytes == 0) { 
      n *= 8;
    }

    while(1) {
      if(n < size * 1000 || i >= UNIT_DIVISIONS - 1) {
        snprintf(buf, bsize, " %4.0f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]); 
        break;
      }
      i++;
      size *= ksize;
      if(n < size * 10) {
        snprintf(buf, bsize, " %4.2f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]); 
        break;
      }
      else if(n < size * 100) {
        snprintf(buf, bsize, " %4.1f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]); 
        break;
      }
  }
}


/* Barchart scales. */
static struct {
    int max, interval;
} scale[] = {
        {      64000,     10 },     /* 64 kbit/s */
        {     128000,     10 },
        {     256000,     10 },
        {    1000000,     10 },     /* 1 Mbit/s */
        {   10000000,     10 },     
        {  100000000,    100 },
        { 1000000000,    100 }      /* 1 Gbit/s */
    };
static int rateidx = 0, wantbiggerrate;

static int get_bar_interval(float bandwidth) {
    int i = 10;
    if(bandwidth > 100000000) {
        i = 100;
    }
    return i;
}

static float get_max_bandwidth() {
    float max;
    if(options.max_bandwidth > 0) {
        max = options.max_bandwidth;
    }
    else {
        max = scale[rateidx].max;
    }
    return max;
}

/* rate in bits */
static int get_bar_length(const int rate) {
    float l;
    if (rate <= 0)
        return 0;
    if (rate > scale[rateidx].max)
        wantbiggerrate = 1;
    if(options.log_scale) {
        l = log(rate) / log(get_max_bandwidth());
    }
    else {
        l = rate / get_max_bandwidth();
    }
    return (l * COLS);
}

static void draw_bar_scale(int* y) {
    float i;
    float max,interval;
    max = get_max_bandwidth();
    interval = get_bar_interval(max);
    if(options.showbars) {
        float stop;
        /* Draw bar graph scale on top of the window. */
        move(*y, 0);
        clrtoeol();
        mvhline(*y + 1, 0, 0, COLS);
        /* i in bytes */

        if(options.log_scale) {
            i = 1.25;
            stop = max / 8;
        }
        else {
            i = max / (5 * 8);
            stop = max / 8;
        }

        /* for (i = 1.25; i * 8 <= max; i *= interval) { */
        while(i <= stop) {
            char s[40], *p;
            int x;
            /* This 1024 vs 1000 stuff is just plain evil */
            readable_size(i, s, sizeof s, options.log_scale ? 1000 : 1024, 0);
            p = s + strspn(s, " ");
            x = get_bar_length(i * 8);
            mvaddch(*y + 1, x, ACS_BTEE);
            if (x + strlen(p) >= COLS)
                x = COLS - strlen(p);
            mvaddstr(*y, x, p);

            if(options.log_scale) {
                i *= interval;
            }
            else {
                i += max / (5 * 8);
            }
        }
        mvaddch(*y + 1, 0, ACS_LLCORNER);
        *y += 2;
    }
    else {
        mvhline(*y, 0, 0, COLS);
        *y += 1;
    }
}

int history_length(const int d) {
    if (history_len < history_divs[d])
        return history_len * RESOLUTION;
    else
        return history_divs[d] * RESOLUTION;
}

void draw_line_total(float sent, float recv, int y, int x, option_linedisplay_t linedisplay, int bytes) {
    char buf[10];
    float n;
    switch(linedisplay) {
        case OPTION_LINEDISPLAY_TWO_LINE:
          draw_line_total(sent, recv, y, x, OPTION_LINEDISPLAY_ONE_LINE_SENT, bytes);
          draw_line_total(sent, recv, y+1, x, OPTION_LINEDISPLAY_ONE_LINE_RECV, bytes);
          break;
        case OPTION_LINEDISPLAY_ONE_LINE_SENT:
          n = sent;
          break;
        case OPTION_LINEDISPLAY_ONE_LINE_RECV:
          n = recv;
          break;
        case OPTION_LINEDISPLAY_ONE_LINE_BOTH:
          n = recv + sent;
          break;
    }
    if(linedisplay != OPTION_LINEDISPLAY_TWO_LINE) {
        readable_size(n, buf, 10, 1024, bytes);
        mvaddstr(y, x, buf);
    }
}

void draw_bar(float n, int y) {
    int L;
    mvchgat(y, 0, -1, A_NORMAL, 0, NULL);
    L = get_bar_length(8 * n);
    if (L > 0)
        mvchgat(y, 0, L + 1, A_REVERSE, 0, NULL);
}

void draw_line_totals(int y, host_pair_line* line, option_linedisplay_t linedisplay) {
    int j;
    int x = (COLS - 8 * HISTORY_DIVISIONS);

    for(j = 0; j < HISTORY_DIVISIONS; j++) {
        draw_line_total(line->sent[j], line->recv[j], y, x, linedisplay, options.bandwidth_in_bytes);
        x += 8;
    }
    
    if(options.showbars) {
      switch(linedisplay) {
        case OPTION_LINEDISPLAY_TWO_LINE:
          draw_bar(line->sent[options.bar_interval],y);
          draw_bar(line->recv[options.bar_interval],y+1);
          break;
        case OPTION_LINEDISPLAY_ONE_LINE_SENT:
          draw_bar(line->sent[options.bar_interval],y);
          break;
        case OPTION_LINEDISPLAY_ONE_LINE_RECV:
          draw_bar(line->recv[options.bar_interval],y);
          break;
        case OPTION_LINEDISPLAY_ONE_LINE_BOTH:
          draw_bar(line->recv[options.bar_interval] + line->sent[options.bar_interval],y);
          break;
      }
    }
}

void draw_totals(host_pair_line* totals) {
    /* Draw rule */
    int y = LINES - 4;
    int j;
    char buf[10];
    int x = (COLS - 8 * HISTORY_DIVISIONS);
    y++;
    draw_line_totals(y, totals, OPTION_LINEDISPLAY_TWO_LINE);
    y += 2;
    for(j = 0; j < HISTORY_DIVISIONS; j++) {
        readable_size((totals->sent[j] + totals->recv[j]) , buf, 10, 1024, options.bandwidth_in_bytes);
        mvaddstr(y, x, buf);
        x += 8;
    }
}

extern history_type history_totals;

void screen_list_init() {
    screen_list.compare = &screen_line_compare;
    sorted_list_initialise(&screen_list);
}

void screen_list_clear() {
    sorted_list_node* nn = NULL;
    peaksent = peakrecv = peaktotal = 0;
    while((nn = sorted_list_next_item(&screen_list, nn)) != NULL) {
        free(nn->data);
    }
    sorted_list_destroy(&screen_list);
}

void calculate_totals() {
    int i;

    /**
     * Calculate peaks and totals
     */
    for(i = 0; i < HISTORY_LENGTH; i++) {
        int j;
        int ii = (HISTORY_LENGTH + history_pos - i) % HISTORY_LENGTH;

        for(j = 0; j < HISTORY_DIVISIONS; j++) {
            if(i < history_divs[j]) {
                totals.recv[j] += history_totals.recv[ii];
                totals.sent[j] += history_totals.sent[ii];
            }
        }

        if(history_totals.recv[i] > peakrecv) {
            peakrecv = history_totals.recv[i];
        }
        if(history_totals.sent[i] > peaksent) {
            peaksent = history_totals.sent[i];
        }
        if(history_totals.recv[i] + history_totals.sent[i] > peaktotal) {
            peaktotal = history_totals.recv[i] + history_totals.sent[i];	
        }
    }
    for(i = 0; i < HISTORY_DIVISIONS; i++) {
      int t = history_length(i);
      totals.recv[i] /= t;
      totals.sent[i] /= t;
    }
}

void make_screen_list() {
    hash_node_type* n = NULL;
    while(hash_next_item(screen_hash, &n) == HASH_STATUS_OK) {
        host_pair_line* line = (host_pair_line*)n->rec;
        int i;
        for(i = 0; i < HISTORY_DIVISIONS; i++) {
          line->recv[i] /= history_length(i);
          line->sent[i] /= history_length(i);
        }

        /* Don't make a new, sorted screen list if order is frozen
         */
        if(!options.freezeorder) {
            sorted_list_insert(&screen_list, line);
        } 
	 
    }
}

/*
 * Zeros all data in the screen hash, but does not remove items.
 */
void screen_hash_clear() {
    hash_node_type* n = NULL;
    while(hash_next_item(screen_hash, &n) == HASH_STATUS_OK) {
        host_pair_line* hpl = (host_pair_line*)n->rec;
        memset(hpl->recv, 0, sizeof(hpl->recv));
        memset(hpl->sent, 0, sizeof(hpl->sent));
    }
}

void analyse_data() {
    hash_node_type* n = NULL;

    if(options.paused == 1) {
      return;
    }

    // Zero totals
    memset(&totals, 0, sizeof totals);

    if(options.freezeorder) {
      screen_hash_clear();
    }
    else {
      screen_list_clear();
      hash_delete_all(screen_hash);
    }

    while(hash_next_item(history, &n) == HASH_STATUS_OK) {
        history_type* d = (history_type*)n->rec;
        host_pair_line* screen_line;
	union {
	    host_pair_line **h_p_l_pp;
	    void **void_pp;
	} u_screen_line = { &screen_line };
        addr_pair ap;
        int i;
        int tsent, trecv;
        tsent = trecv = 0;


        ap = *(addr_pair*)n->key;

        /* Aggregate hosts, if required */
        if(options.aggregate_src) {
            ap.src.s_addr = 0;
        }
        if(options.aggregate_dest) {
            ap.dst.s_addr = 0;
        }

        /* Aggregate ports, if required */
        if(options.showports == OPTION_PORTS_DEST || options.showports == OPTION_PORTS_OFF) {
            ap.src_port = 0;
        }
        if(options.showports == OPTION_PORTS_SRC || options.showports == OPTION_PORTS_OFF) {
            ap.dst_port = 0;
        }
        if(options.showports == OPTION_PORTS_OFF) {
            ap.protocol = 0;
        }

	
        if(hash_find(screen_hash, &ap, u_screen_line.void_pp) == HASH_STATUS_KEY_NOT_FOUND) {
            screen_line = xcalloc(1, sizeof *screen_line);
            hash_insert(screen_hash, &ap, screen_line);
            screen_line->ap = ap;
        }
        
	screen_line->total_sent += d->total_sent;
	screen_line->total_recv += d->total_recv;

        for(i = 0; i < HISTORY_LENGTH; i++) {
            int j;
            int ii = (HISTORY_LENGTH + history_pos - i) % HISTORY_LENGTH;

            for(j = 0; j < HISTORY_DIVISIONS; j++) {
                if(i < history_divs[j]) {
                    screen_line->recv[j] += d->recv[ii];
                    screen_line->sent[j] += d->sent[ii];
                }
            }
        }

    }

    make_screen_list();

    
    calculate_totals();

}

void sprint_host(char * line, struct in_addr* addr, unsigned int port, unsigned int protocol, int L) {
    char hostname[HOSTNAME_LENGTH];
    char service[HOSTNAME_LENGTH];
    char* s_name;
    union {
        char **ch_pp;
        void **void_pp;
    } u_s_name = { &s_name };

    ip_service skey;
    int left;
    if(addr->s_addr == 0) {
        sprintf(hostname, " * ");
    }
    else {
        if (options.dnsresolution)
            resolve(addr, hostname, L);
        else
            strcpy(hostname, inet_ntoa(*addr));
    }
    left = strlen(hostname);

    if(port != 0) {
      skey.port = port;
      skey.protocol = protocol;
      if(options.portresolution && hash_find(service_hash, &skey, u_s_name.void_pp) == HASH_STATUS_OK) {
        snprintf(service, HOSTNAME_LENGTH, ":%s", s_name);
      }
      else {
        snprintf(service, HOSTNAME_LENGTH, ":%d", port);
      }
    }
    else {
      service[0] = '\0';
    }


    sprintf(line, "%-*s", L, hostname);
    if(left > (L - strlen(service))) {
        left = L - strlen(service);
        if(left < 0) {
           left = 0;
        }
    }
    sprintf(line + left, "%-*s", L-left, service);
}



void ui_print() {
    sorted_list_node* nn = NULL;
    char host1[HOSTNAME_LENGTH], host2[HOSTNAME_LENGTH];
    static char *line;
    static int lcols;
    int y = 0;

    if (dontshowdisplay)
        return;

    if (!line || lcols != COLS) {
        xfree(line);
        line = calloc(COLS + 1, 1);
    }

    /* 
     * erase() is faster than clear().  Dunno why we switched to 
     * clear() -pdw 24/10/02
     */
    erase();

    draw_bar_scale(&y);

    if(options.showhelp) {
      mvaddstr(y,0,HELP_MESSAGE);
    }
    else {
      int i = 0;

      while(i < options.screen_offset && ((nn = sorted_list_next_item(&screen_list, nn)) != NULL)) {
        i++;
      }

      /* Screen layout: we have 2 * HISTORY_DIVISIONS 6-character wide history
       * items, and so can use COLS - 12 * HISTORY_DIVISIONS to print the two
       * host names. */

      if(i == 0 || nn != NULL) {
        while((y < LINES - 5) && ((nn = sorted_list_next_item(&screen_list, nn)) != NULL)) {
            int x = 0, L;


            host_pair_line* screen_line = (host_pair_line*)nn->data;

            if(y < LINES - 5) {
                L = (COLS - 8 * HISTORY_DIVISIONS - 4) / 2;
                if(options.show_totals) {
                    L -= 4;    
                }
                if(L > HOSTNAME_LENGTH) {
                    L = HOSTNAME_LENGTH;
                }

                sprint_host(host1, &(screen_line->ap.src), screen_line->ap.src_port, screen_line->ap.protocol, L);
                sprint_host(host2, &(screen_line->ap.dst), screen_line->ap.dst_port, screen_line->ap.protocol, L);
                if(!screen_filter_match(host1) && !screen_filter_match(host2)) {
                  continue;
                }

                mvaddstr(y, x, host1);
                x += L;

                switch(options.linedisplay) {
                  case OPTION_LINEDISPLAY_TWO_LINE:
                    mvaddstr(y, x, " => ");
                    mvaddstr(y+1, x, " <= ");
                    break;
                  case OPTION_LINEDISPLAY_ONE_LINE_BOTH:
                    mvaddstr(y, x, "<=> ");
                    break;
                  case OPTION_LINEDISPLAY_ONE_LINE_SENT:
                    mvaddstr(y, x, " => ");
                    break;
                  case OPTION_LINEDISPLAY_ONE_LINE_RECV:
                    mvaddstr(y, x, " <= ");
                    break;
                }

                x += 4;


                mvaddstr(y, x, host2);
                
                if(options.show_totals) {
                    draw_line_total(screen_line->total_sent, screen_line->total_recv, y, COLS - 8 * (HISTORY_DIVISIONS + 1), options.linedisplay, 1);
                }

                draw_line_totals(y, screen_line, options.linedisplay);

            }
            if(options.linedisplay == OPTION_LINEDISPLAY_TWO_LINE) {
              y += 2;
            }
            else {
              y += 1;
            }
        }
      }
    }


    y = LINES - 3;
    
    mvhline(y-1, 0, 0, COLS);

    mvaddstr(y, 0, "TX: ");
    mvaddstr(y+1, 0, "RX: ");
    mvaddstr(y+2, 0, "TOTAL: ");

    /* Cummulative totals */
    mvaddstr(y, 16, "cumm: ");

    readable_size(history_totals.total_sent, line, 10, 1024, 1);
    mvaddstr(y, 22, line);

    readable_size(history_totals.total_recv, line, 10, 1024, 1);
    mvaddstr(y+1, 22, line);

    readable_size(history_totals.total_recv + history_totals.total_sent, line, 10, 1024, 1);
    mvaddstr(y+2, 22, line);

    /* peak traffic */
    mvaddstr(y, 32, "peak: ");

    readable_size(peaksent / RESOLUTION, line, 10, 1024, options.bandwidth_in_bytes);
    mvaddstr(y, 39, line);

    readable_size(peakrecv / RESOLUTION, line, 10, 1024, options.bandwidth_in_bytes);
    mvaddstr(y+1, 39, line);

    readable_size(peaktotal / RESOLUTION, line, 10, 1024, options.bandwidth_in_bytes);
    mvaddstr(y+2, 39, line);

    mvaddstr(y, COLS - 8 * HISTORY_DIVISIONS - 8, "rates:");

    draw_totals(&totals);


    if(showhelphint) {
      mvaddstr(0, 0, " ");
      mvaddstr(0, 1, helpmsg);
      mvaddstr(0, 1 + strlen(helpmsg), " ");
      mvchgat(0, 0, strlen(helpmsg) + 2, A_REVERSE, 0, NULL);
    }
    move(LINES - 1, COLS - 1);
    
    refresh();

    /* Bar chart auto scale */
    if (wantbiggerrate && options.max_bandwidth == 0) {
        ++rateidx;
        wantbiggerrate = 0;
    }
}

void ui_tick(int print) {
  if(print) {
    ui_print();
  }
  else if(showhelphint && (time(NULL) - helptimer > HELP_TIME) && !persistenthelp) {
    showhelphint = 0;
    ui_print();
  }
}

void ui_curses_init() {
    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();       /* don't echo input */
    halfdelay(2);
}

void showhelp(const char * s) {
  strncpy(helpmsg, s, HELP_MSG_SIZE);
  showhelphint = 1;
  helptimer = time(NULL);
  persistenthelp = 0;
  tick(1);
}

void ui_init() {
    char msg[20];
    ui_curses_init();
    
    erase();

    screen_list_init();
    screen_hash = addr_hash_create();

    service_hash = serv_hash_create();
    serv_hash_initialise(service_hash);

    snprintf(msg,20,"Listening on %s",options.interface);
    showhelp(msg);


}


void showportstatus() {
  if(options.showports == OPTION_PORTS_ON) {
    showhelp("Port display ON");
  }
  else if(options.showports == OPTION_PORTS_OFF) {
    showhelp("Port display OFF");
  }
  else if(options.showports == OPTION_PORTS_DEST) {
    showhelp("Port display DEST");
  }
  else if(options.showports == OPTION_PORTS_SRC) {
    showhelp("Port display SOURCE");
  }
}


void ui_loop() {
    /* in edline.c */
    char *edline(int linenum, const char *prompt, const char *initial);
    /* in iftop.c */
    char *set_filter_code(const char *filter);

    extern sig_atomic_t foad;

    while(foad == 0) {
        int i;
        i = getch();
        switch (i) {
            case 'q':
                foad = 1;
                break;

            case 'n':
                if(options.dnsresolution) {
                    options.dnsresolution = 0;
                    showhelp("DNS resolution off");
                }
                else {
                    options.dnsresolution = 1;
                    showhelp("DNS resolution on");
                }
                tick(1);
                break;

            case 'N':
                if(options.portresolution) {
                    options.portresolution = 0;
                    showhelp("Port resolution off");
                }
                else {
                    options.portresolution = 1;
                    showhelp("Port resolution on");
                }
                tick(1);
                break;

            case 'h':
            case '?':
                options.showhelp = !options.showhelp;
                tick(1);
                break;

            case 'b':
                if(options.showbars) {
                    options.showbars = 0;
                    showhelp("Bars off");
                }
                else {
                    options.showbars = 1;
                    showhelp("Bars on");
                }
                tick(1);
                break;

            case 'B':
                options.bar_interval = (options.bar_interval + 1) % 3;
                if(options.bar_interval == 0) {
                    showhelp("Bars show 2s average");
                }
                else if(options.bar_interval == 1) { 
                    showhelp("Bars show 10s average");
                }
                else {
                    showhelp("Bars show 40s average");
                }
                ui_print();
                break;
            case 's':
                if(options.aggregate_src) {
                    options.aggregate_src = 0;
                    showhelp("Show source host");
                }
                else {
                    options.aggregate_src = 1;
                    showhelp("Hide source host");
                }
                break;
            case 'd':
                if(options.aggregate_dest) {
                    options.aggregate_dest = 0;
                    showhelp("Show dest host");
                }
                else {
                    options.aggregate_dest = 1;
                    showhelp("Hide dest host");
                }
                break;
            case 'S':
                /* Show source ports */
                if(options.showports == OPTION_PORTS_OFF) {
                  options.showports = OPTION_PORTS_SRC;
                }
                else if(options.showports == OPTION_PORTS_DEST) {
                  options.showports = OPTION_PORTS_ON;
                }
                else if(options.showports == OPTION_PORTS_ON) {
                  options.showports = OPTION_PORTS_DEST;
                }
                else {
                  options.showports = OPTION_PORTS_OFF;
                }
                showportstatus();
                break;
            case 'D':
                /* Show dest ports */
                if(options.showports == OPTION_PORTS_OFF) {
                  options.showports = OPTION_PORTS_DEST;
                }
                else if(options.showports == OPTION_PORTS_SRC) {
                  options.showports = OPTION_PORTS_ON;
                }
                else if(options.showports == OPTION_PORTS_ON) {
                  options.showports = OPTION_PORTS_SRC;
                }
                else {
                  options.showports = OPTION_PORTS_OFF;
                }
                showportstatus();
                break;
            case 'p':
                options.showports = 
                  (options.showports == OPTION_PORTS_OFF)
                  ? OPTION_PORTS_ON
                  : OPTION_PORTS_OFF;
                showportstatus();
                // Don't tick here, otherwise we get a bogus display
                break;
            case 'P':
                if(options.paused) {
                    options.paused = 0;
                    showhelp("Display unpaused");
                }
                else {
                    options.paused = 1;
                    showhelp("Display paused");
                    persistenthelp = 1;
                }
                break;
            case 'o':
                if(options.freezeorder) {
                    options.freezeorder = 0;
                    showhelp("Order unfrozen");
                }
                else {
                    options.freezeorder = 1;
                    showhelp("Order frozen");
                    persistenthelp = 1;
                }
                break;
            case '1':
                options.sort = OPTION_SORT_DIV1;
                showhelp("Sort by col 1");
                break;
            case '2':
                options.sort = OPTION_SORT_DIV2;
                showhelp("Sort by col 2");
                break;
            case '3':
                options.sort = OPTION_SORT_DIV3;
                showhelp("Sort by col 3");
                break;
            case '<':
                options.sort = OPTION_SORT_SRC;
                showhelp("Sort by source");
                break;
            case '>':
                options.sort = OPTION_SORT_DEST;
                showhelp("Sort by dest");
                break;
            case 'j':
                options.screen_offset++;
                ui_print();
                break;
            case 'k':
                if(options.screen_offset > 0) {
                  options.screen_offset--;
                  ui_print();
                }
                break;
            case 't':
                options.linedisplay = (options.linedisplay + 1) % 4;
                switch(options.linedisplay) {
                  case OPTION_LINEDISPLAY_TWO_LINE:
                    showhelp("Two lines per host");
                    break;
                  case OPTION_LINEDISPLAY_ONE_LINE_SENT:
                    showhelp("Sent traffic only");
                    break;
                  case OPTION_LINEDISPLAY_ONE_LINE_RECV:
                    showhelp("Received traffic only");
                    break;
                  case OPTION_LINEDISPLAY_ONE_LINE_BOTH:
                    showhelp("One line per host");
                    break;
                }
                ui_print();
                break;
            case 'f': {
                char *s;
                dontshowdisplay = 1;
                if ((s = edline(0, "Net filter", options.filtercode))) {
                    char *m;
                    if (s[strspn(s, " \t")] == 0) {
                        /* Empty filter; set to NULL. */
                        xfree(s);
                        s = NULL;
                    }
                    if (!(m = set_filter_code(s))) {
                        xfree(options.filtercode);
                        options.filtercode = s;
                        /* -lpcap will write junk to stderr; we do our best to
                         * erase it.... */
                        move(COLS - 1, LINES - 1);
                        wrefresh(curscr);
                        showhelp("Installed new filter");
                    } else {
                        showhelp(m);
                        xfree(s);
                    }
                }
                dontshowdisplay = 0;
                ui_print();
                break;
            }
            case 'l': {
#ifdef HAVE_REGCOMP
                char *s;
                dontshowdisplay = 1;
                if ((s = edline(0, "Screen filter", options.screenfilter))) {
                    if(!screen_filter_set(s)) {
                        showhelp("Invalid regexp");
                    }
                }
                dontshowdisplay = 0;
                ui_print();
#else
                showhelp("Sorry, screen filters not supported on this platform")
#endif
                break;
            }
            case '!': {
#ifndef NO_SYSTEM
                char *s;
                dontshowdisplay = 1;
                if ((s = edline(0, "Command", "")) && s[strspn(s, " \t")]) {
                    int i, dowait = 0;
                    erase();
                    refresh();
                    endwin();
                    errno = 0;
                    i = system(s);
                    if (i == -1 || (i == 127 && errno != 0)) {
                        fprintf(stderr, "system: %s: %s\n", s, strerror(errno));
                        dowait = 1;
                    } else if (i != 0) {
                        if (WIFEXITED(i))
                            fprintf(stderr, "%s: exited with code %d\n", s, WEXITSTATUS(i));
                        else if (WIFSIGNALED(i))
                            fprintf(stderr, "%s: killed by signal %d\n", s, WTERMSIG(i));
                        dowait = 1;
                    }
                    ui_curses_init();
                    if (dowait) {
                        fprintf(stderr, "Press any key....");
                        while (getch() == ERR);
                    }
                    erase();
                    xfree(s);
                }
                dontshowdisplay = 0;
#else
                showhelp("Sorry, subshells have been disabled.");
#endif
                break;
            }
            case 'T':
                options.show_totals = !options.show_totals;
                if(options.show_totals) {
                    showhelp("Show cummulative totals");
                }
                else {
                    showhelp("Hide cummulative totals");
                }
                ui_print();
                break;
            case 'L':
                options.log_scale = !options.log_scale;
                showhelp(options.log_scale ? "Logarithmic scale" : "Linear scale");
                ui_print();
                break;
            case KEY_CLEAR:
            case 12:    /* ^L */
                wrefresh(curscr);
                break;
            case ERR:
                break;
            default:
                showhelp("Press H or ? for help");
                break;
        }
        tick(0);
    }
}

void ui_finish() {
    endwin();
}
