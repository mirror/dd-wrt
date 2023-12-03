#include <inttypes.h>
#include <ctype.h>

#include "dawn_iwinfo.h"
#include "memory_utils.h"

#include "datastorage.h"
#include "mac_utils.h"
#include "msghandler.h"
#include "ubus.h"
#include "test_storage.h"

/*** Test Stub Functions - Called by SUT ***/

// It is an accident of Uni* / C history that the below is not stubbed in libc...
int pthread_mutex_trylock(pthread_mutex_t* m)
{
    return EBUSY;  // Tell SUT that it was locked, so no errors, warnings, etc
}

void ubus_set_nr_from_clients(struct kicking_nr* ap_list) {
    printf("ubus_set_nr_from_clients() was called...\n");
}


int ubus_send_beacon_request(client *c, ap *a, int d, int id)
{
    printf("ubus_send_beacon_request() was called...\n");

    return 0;
}

int send_set_probe(struct dawn_mac client_addr)
{
    printf("send_set_probe() was called...\n");
    return 0;
}

int bss_transition_request(uint32_t id, const struct dawn_mac client_addr, struct kicking_nr* neighbor_list, uint32_t duration)
{
    int ret = 0;

    printf("bss_transition_request() was called...\n");

    if (neighbor_list != NULL)
    {
        // Fake a client being disassociated and then rejoining on the recommended neighbor
        client* mc = client_array_get_client(client_addr);
        mc = client_array_delete(mc, true);
        // Originally, there was only one AP, not a list of them; that AP is at the tail of the list
        // Use it to keep the results the same as before
        while (neighbor_list && neighbor_list->next)
            neighbor_list = neighbor_list->next;
        for (int n = 0; n < ETH_ALEN; n++)
            sscanf(neighbor_list->nr_ap->neighbor_report + n * 2, "%2hhx", mc->bssid_addr.u8 + n);
        insert_client_to_array(mc, 0);
        printf("BSS TRANSITION TO " NR_MACSTR "\n", NR_MAC2STR(neighbor_list->nr_ap->neighbor_report));

        // Tell caller not to change the arrays any further
        ret = 1;
    }

    return ret;
}


int wnm_disassoc_imminent(uint32_t id, const struct dawn_mac client_addr, struct kicking_nr* neighbor_list, int threshold, uint32_t duration)
{
int ret = 0;

    printf("wnm_disassoc_imminent() was called...\n");

    if (neighbor_list != NULL)
    {
        // Fake a client being disassociated and then rejoining on the recommended neighbor
        client *mc = client_array_get_client(client_addr);
        mc = client_array_delete(mc, true);
        // Originally, there was only one AP, not a list of them; that AP is at the tail of the list
        // Use it to keep the results the same as before
        while (neighbor_list && neighbor_list->next)
            neighbor_list = neighbor_list->next;
        for (int n=0; n < ETH_ALEN; n++)
            sscanf(neighbor_list->nr_ap->neighbor_report + n*2, "%2hhx", mc->bssid_addr.u8 + n);
        insert_client_to_array(mc, 0);
        printf("BSS TRANSITION TO " NR_MACSTR "\n", NR_MAC2STR(neighbor_list->nr_ap->neighbor_report));

        // Tell caller not to change the arrays any further
        ret = 1;
    }

    return ret;
}

void add_client_update_timer(time_t time)
{
    printf("add_client_update_timer() was called...\n");
}

void del_client_interface(uint32_t id, const struct dawn_mac client_addr, uint32_t reason, uint8_t deauth, uint32_t ban_time)
{
    printf("del_client_interface() was called...\n");
}

int ubus_send_probe_via_network(struct probe_entry_s *probe_entry, bool is_beacon)
{
    printf("send_probe_via_network() was called...\n");
    return 0;
}

int get_rssi_iwinfo(struct dawn_mac client_addr)
{
    printf("get_rssi_iwinfo() was called...\n");
    return 0;
}

int get_expected_throughput_iwinfo(struct dawn_mac client_addr)
{
    printf("get_expected_throughput_iwinfo() was called...\n");
    return 0;
}

int get_bandwidth_iwinfo(struct dawn_mac client_addr, float* rx_rate, float* tx_rate)
{
    *rx_rate = 0.0;
    *tx_rate = 0.0;

    printf("get_bandwidth_iwinfo() was called.  Returning rx=%1f, tx=%1f...\n", (double)*rx_rate, (double)*tx_rate);
    return 0;
}

int send_add_mac(struct dawn_mac client_addr)
{
    printf("send_add_mac() was called...\n");
    return 0;
}

/*** Local Function Prototypes and Related Constants ***/
static int array_auto_helper(int action, int i0, int i1);

#define HELPER_ACTION_ADD 0x1000
#define HELPER_ACTION_DEL 0x2000
#define HELPER_ACTION_STRESS 0x4000
#define HELPER_ACTION_MASK 0x7000

#define HELPER_AP 0x0001
#define HELPER_CLIENT 0x0002
#define HELPER_AUTH_ENTRY 0x0004
#define HELPER_PROBE_ARRAY 0x0008

/*** Local globals ***/
static double base_time;
static double last_time;
static bool first_time = true;

static time_t faketime = 1000;
static bool faketime_auto = true; // true = increment every command; false = scripted updates

/*** Test harness code */
static void time_moves_on()
{
    // Sometimes move fake time on a second - about 1 in 5 chance
    if (((rand() & 0xFF) > 200))
    {
        faketime += 1;
    }

    return;
}

static void set_random_mac(uint8_t *mac)
{
int16_t r1 = rand();
int16_t r2 = rand();
int16_t r3 = rand();

    mac[0] = r1;
    mac[1] = r1 >> 4;
    mac[2] = r2;
    mac[3] = r2 >> 4;
    mac[4] = r3;
    mac[5] = r3 >> 4;

    return;
}

static int array_auto_helper(int action, int i0, int i1)
{
    int m = i0;
    int step = (i0 > i1) ? -1 : 1;
    int ret = 0;

    srand(time(0)); // For faketime incrementing

    int cont = 1;
    while (cont) {
        struct dawn_mac this_mac;

        uint64_t mac_src = m;
    memcpy(&this_mac.u8, &mac_src, ETH_ALEN < sizeof (uint64_t) ? ETH_ALEN : sizeof (uint64_t));
        switch (action & ~HELPER_ACTION_MASK)
        {
        case HELPER_AP:
            ; // Empty statement to allow label before declaration
            if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_ADD)
            {
                ap* ap0 = dawn_malloc(sizeof(struct ap_s));
                ap0->bssid_addr = this_mac;

                insert_to_ap_array(ap0, 0);
            }
            else if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_STRESS) {
                ap* ap0 = dawn_malloc(sizeof(struct ap_s));
                set_random_mac(ap0->bssid_addr.u8);

                insert_to_ap_array(ap0, faketime);
                remove_old_ap_entries(faketime, 10);
                time_moves_on();
            }
            else if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_DEL)
                ap_array_delete(ap_array_get_ap(this_mac));
            break;
        case HELPER_CLIENT:
            ; // Empty statement to allow label before declaration
            if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_ADD)
            {
                client* client0 = dawn_malloc(sizeof(struct client_s));
                client0->bssid_addr = this_mac;
                client0->client_addr = this_mac;

                insert_client_to_array(client0, 0);
            }
            else if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_STRESS) {
                client* client0 = dawn_malloc(sizeof(struct client_s));
                set_random_mac(client0->client_addr.u8);
                set_random_mac(client0->bssid_addr.u8);

                insert_client_to_array(client0, faketime);
                remove_old_client_entries(faketime, 10);
                time_moves_on();
            }
            else if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_DEL)
            {
                client* client0 = client_array_get_client(this_mac);

                if (client0 != NULL && mac_is_equal_bb(this_mac, client0->client_addr))
                    client_array_delete(client0, false);
            }
            break;
        case HELPER_PROBE_ARRAY:
            ; // Empty statement to allow label before declaration
            probe_entry* probe0 = NULL;

            if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_ADD) {
                probe0 = dawn_malloc(sizeof(probe_entry));
                probe0->client_addr = this_mac;
                probe0->bssid_addr = this_mac;

                insert_to_probe_array(probe0, true, true, 0); // TODO: Check bool flags
            }
            else if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_STRESS) {
                probe0 = dawn_malloc(sizeof(probe_entry));
                set_random_mac(probe0->client_addr.u8);
                set_random_mac(probe0->bssid_addr.u8);

                insert_to_probe_array(probe0, true, true, faketime);
                remove_old_probe_entries(faketime, 10);
                time_moves_on();
            }
            else if ((action & HELPER_ACTION_MASK) == HELPER_ACTION_DEL) {
                probe_array_delete(this_mac, this_mac);
            }
            break;
        default:
            printf("HELPER error - which entity?\n");
            ret = -1;
        }

        if (m == i1)
            cont = 0;
        else
            m += step;
    }

    return ret;
}

static int load_u8(uint8_t* v, char* s);
static int load_u8(uint8_t* v, char* s)
{
    int ret = 0;
    sscanf(s, "%" SCNu8, v);
    return ret;
}

static int load_u32(uint32_t* v, char* s);
static int load_u32(uint32_t* v, char* s)
{
    int ret = 0;
    sscanf(s, "%" SCNu32, v);
    return ret;
}

static int load_int(int* v, char* s);
static int load_int(int* v, char* s)
{
    int ret = 0;
    sscanf(s, "%" "d", v);
    return ret;
}

static int load_int_band(int* v, char* s);
static int load_int_band(int* v, char* s)
{
  int ret = 0;
  sscanf(s, "%" "d", v);
  v[1] = v[0];
  return ret;
}

static int load_string(size_t l, char* v, char* s);
static int load_string(size_t l, char* v, char* s)
{
    int ret = 0;
    strncpy(v, s, l);
    return ret;
}

static int load_ssid(uint8_t* v, char* s);
static int load_ssid(uint8_t* v, char* s)
{
    int ret = 0;
    strncpy((char*)v, s, SSID_MAX_LEN);
    return ret;
}

static int load_time(time_t* v, char* s);
static int load_time(time_t* v, char* s)
{
    int ret = 0;
    sscanf(s, "%" SCNi64, (int64_t*)v);  // TODO: Check making portable for target SoC environemnts?
    return ret;
}

static int consume_actions(int argc, char* argv[], int harness_verbosity);

static int consume_actions(int argc, char* argv[], int harness_verbosity)
{
    int ret = 0;
    int args_required = 0; // Suppress compiler warming by assigning initial value

    int curr_arg = 0;

    while (curr_arg < argc && ret == 0)
    {
        if ((strcmp(*argv, "time") == 0) || (strcmp(*argv, "elapsed") == 0) || (strcmp(*argv, "elapsed_msg") == 0)) // "time" is deprecated to avoid confusion with "faketime" commands
        {
            if (strcmp(*argv, "elapsed_msg") == 0)
            {
                args_required = 2;
            }
            else
            {
                args_required = 1;
            }

            if (curr_arg + args_required <= argc)
            {
                struct timespec spec;
                double curr_time;

                //TODO: Check portability for SoC devices when benchmarking?
                clock_gettime(CLOCK_REALTIME, &spec);
                curr_time = spec.tv_sec * 1000.0 + spec.tv_nsec / 1000000.0;

                // First call sets base time for script
                // Later calls report elapsed time since base, and from previous call
                if (first_time)
                {
                    first_time = false;
                    base_time = curr_time;
                    last_time = curr_time;
                }

                if (strcmp(*argv, "elapsed_msg") == 0)
                {
                    printf("Elapsed time (%s): base=%fms, last=%fms\n", *(argv + 1), curr_time - base_time, curr_time - last_time);
                }
                else
                {
                    printf("Elapsed time: base=%fms, last=%fms\n", curr_time - base_time, curr_time - last_time);
                }

                last_time = curr_time;
            }
        }
        else if (strcmp(*argv, "memleak") == 0)
        {
            args_required = 1;

            char* leaky = dawn_malloc(10);
            strcpy(leaky, "TRACKED"); // Force use of memory to avoid unused error

            leaky = malloc(10);
            strcpy(leaky, "UNTRACKED"); // Force use of memory to avoid unused error
        }
        else if (strcmp(*argv, "segv") == 0)
        {
            args_required = 1;

            char *badpointer = 0;
            *badpointer = 0;
        }
        else if (strcmp(*argv, "memaudit") == 0)
        {
            args_required = 1;

            dawn_memory_audit();
        }
        else if (strcmp(*argv, "faketime") == 0)
        {
            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                if (strcmp(*(argv + 1), "auto") == 0)
                {
                    faketime_auto = true;
                }
                else if (strcmp(*(argv + 1), "show") == 0)
                {
                    printf("FAKETIME is currently: %" PRId64 "\n", (int64_t)faketime);
                }
                else
                {
                    args_required = 3;
                    if (curr_arg + args_required <= argc)
                    {
                        if (strcmp(*(argv + 1), "set") == 0)
                        {
                            faketime_auto = false;
                            faketime = atol(argv[2]);
                        }
                        else if (strcmp(*(argv + 1), "add") == 0)
                        {
                            faketime_auto = false;
                            faketime += atol(argv[2]);
                        }
                        else
                        {
                            printf("FAKETIME \"%s\": Unknown or mangled - stopping!\n", *(argv + 1));
                            ret = -1;
                        }
                    }
                }
            }
        }
        else if (strcmp(*argv, "ap_show") == 0)
        {
            args_required = 1;

            print_ap_array();
        }
        else if (strcmp(*argv, "probe_show") == 0)
        {
            args_required = 1;

            if (dawnlog_showing(DAWNLOG_INFO))
                print_probe_array();
        }
        else if (strcmp(*argv, "client_show") == 0)
        {
            args_required = 1;

            print_client_array();
        }
        else if (strcmp(*argv, "ap_add_auto") == 0)
        {
            args_required = 3;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_AP | HELPER_ACTION_ADD, atoi(*(argv + 1)), atoi(*(argv + 2)));
            }
        }
        else if (strcmp(*argv, "ap_del_auto") == 0)
        {
            args_required = 3;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_AP | HELPER_ACTION_DEL, atoi(*(argv + 1)), atoi(*(argv + 2)));
            }
        }
        else if (strcmp(*argv, "ap_stress") == 0)
        {
            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_AP | HELPER_ACTION_STRESS, 1, atoi(*(argv + 1)));
            }
        }
        else if (strcmp(*argv, "probe_add_auto") == 0)
        {
            args_required = 3;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_PROBE_ARRAY | HELPER_ACTION_ADD, atoi(*(argv + 1)), atoi(*(argv + 2)));
            }
        }
        else if (strcmp(*argv, "probe_del_auto") == 0)
        {
            args_required = 3;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_PROBE_ARRAY | HELPER_ACTION_DEL, atoi(*(argv + 1)), atoi(*(argv + 2)));
            }
        }
        else if (strcmp(*argv, "probe_stress") == 0)
        {
            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_PROBE_ARRAY | HELPER_ACTION_STRESS, 1, atoi(*(argv + 1)));
            }
        }
        else if (strcmp(*argv, "client_add_auto") == 0)
        {
            args_required = 3;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_CLIENT | HELPER_ACTION_ADD, atoi(*(argv + 1)), atoi(*(argv + 2)));
            }
        }
        else if (strcmp(*argv, "client_del_auto") == 0)
        {
            args_required = 3;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_CLIENT | HELPER_ACTION_DEL, atoi(*(argv + 1)), atoi(*(argv + 2)));
            }
        }
        else if (strcmp(*argv, "client_stress") == 0)
        {
            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                ret = array_auto_helper(HELPER_CLIENT | HELPER_ACTION_STRESS, 1, atoi(*(argv + 1)));
            }
        }
        else if (strcmp(*argv, "remove_old_ap_entries") == 0)
        {
            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                remove_old_ap_entries(faketime, atol(argv[1]));
            }
        }
        else if (strcmp(*argv, "remove_old_client_entries") == 0)
        {
            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                remove_old_client_entries(faketime, atol(argv[1]));
            }
        }
        else if (strcmp(*argv, "remove_old_probe_entries") == 0)
        {
            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                remove_old_probe_entries(faketime, atol(argv[1]));
            }
        }
        else if (strcmp(*argv, "dawn") == 0) // Load metrics that configure DAWN
        {
            args_required = 1;
            while (ret == 0 && curr_arg + args_required < argc)
            {
                char* fn = *(argv + args_required);

                if (!strcmp(fn, "default"))
                {
                    dawn_metric.ap_weight[0] = 0; // Sum component
                    dawn_metric.ap_weight[1] = 0; // Sum component
                    dawn_metric.ht_support[0] = 10; // Sum component
                    dawn_metric.ht_support[1] = 10; // Sum component
                    dawn_metric.vht_support[0] = 100; // Sum component
                    dawn_metric.vht_support[1] = 100; // Sum component
                    dawn_metric.no_ht_support[0] = 0; // Sum component
                    dawn_metric.no_ht_support[1] = 0; // Sum component
                    dawn_metric.no_vht_support[0] = 0; // Sum component
                    dawn_metric.no_vht_support[1] = 0; // Sum component
                    dawn_metric.rssi[0] = 10; // Sum component
                    dawn_metric.rssi[1] = 10; // Sum component
                    dawn_metric.low_rssi[0] = -500; // Sum component
                    dawn_metric.low_rssi[1] = -500; // Sum component
                    dawn_metric.initial_score[0] = 0; // Sum component
                    dawn_metric.initial_score[1] = 100; // Sum component
                    dawn_metric.chan_util[0] = 0; // Sum component
                    dawn_metric.chan_util[1] = 0; // Sum component
                    dawn_metric.max_chan_util[0] = -500; // Sum component
                    dawn_metric.max_chan_util[1] = -500; // Sum component
                    dawn_metric.rssi_val[0] = -60;
                    dawn_metric.rssi_val[1] = -60;
                    dawn_metric.low_rssi_val[0] = -80;
                    dawn_metric.low_rssi_val[1] = -80;
                    dawn_metric.chan_util_val[0] = 140;
                    dawn_metric.chan_util_val[1] = 140;
                    dawn_metric.max_chan_util_val[0] = 170;
                    dawn_metric.max_chan_util_val[1] = 170;
                    dawn_metric.min_probe_count = 2;
                    dawn_metric.bandwidth_threshold = 6;
                    dawn_metric.use_station_count = 1;
                    dawn_metric.max_station_diff = 1;
                    dawn_metric.eval_probe_req = 1;
                    dawn_metric.eval_auth_req = 1;
                    dawn_metric.eval_assoc_req = 1;
                    dawn_metric.deny_auth_reason = 1;
                    dawn_metric.deny_assoc_reason = 17;
                    dawn_metric.min_number_to_kick = 3;
                    dawn_metric.chan_util_avg_period = 3;
                    dawn_metric.set_hostapd_nr = 1;
                    dawn_metric.disassoc_nr_length = 6;
                    dawn_metric.kicking = 0;
                    dawn_metric.duration = 0;
                    dawn_metric.rrm_mode_mask = WLAN_RRM_CAPS_BEACON_REPORT_PASSIVE |
                                                WLAN_RRM_CAPS_BEACON_REPORT_ACTIVE |
                                                WLAN_RRM_CAPS_BEACON_REPORT_TABLE;
                    dawn_metric.rrm_mode_order[0] = WLAN_RRM_CAPS_BEACON_REPORT_PASSIVE;
                    dawn_metric.rrm_mode_order[1] = WLAN_RRM_CAPS_BEACON_REPORT_ACTIVE;
                    dawn_metric.rrm_mode_order[2] = WLAN_RRM_CAPS_BEACON_REPORT_TABLE;
                }
                else if (!strncmp(fn, "ap_weight=", 10)) load_int_band(dawn_metric.ap_weight, fn + 10);
                else if (!strncmp(fn, "ht_support=", 11)) load_int_band(dawn_metric.ht_support, fn + 11);
                else if (!strncmp(fn, "vht_support=", 12)) load_int_band(dawn_metric.vht_support, fn + 12);
                else if (!strncmp(fn, "no_ht_support=", 14)) load_int_band(dawn_metric.no_ht_support, fn + 14);
                else if (!strncmp(fn, "no_vht_support=", 15)) load_int_band(dawn_metric.no_vht_support, fn + 15);
                else if (!strncmp(fn, "rssi=", 5)) load_int_band(dawn_metric.rssi, fn + 5);
                else if (!strncmp(fn, "low_rssi=", 9)) load_int_band(dawn_metric.low_rssi, fn + 9);
                else if (!strncmp(fn, "freq=", 5)) load_int(&dawn_metric.initial_score[1], fn + 5);
                else if (!strncmp(fn, "chan_util=", 10)) load_int_band(dawn_metric.chan_util, fn + 10);
                else if (!strncmp(fn, "max_chan_util=", 14)) load_int_band(dawn_metric.max_chan_util, fn + 14);
                else if (!strncmp(fn, "rssi_val=", 9)) load_int_band(dawn_metric.rssi_val, fn + 9);
                else if (!strncmp(fn, "low_rssi_val=", 13)) load_int_band(dawn_metric.low_rssi_val, fn + 13);
                else if (!strncmp(fn, "chan_util_val=", 14)) load_int_band(dawn_metric.chan_util_val, fn + 14);
                else if (!strncmp(fn, "max_chan_util_val=", 18)) load_int_band(dawn_metric.max_chan_util_val, fn + 18);
                else if (!strncmp(fn, "min_probe_count=", 16)) load_int(&dawn_metric.min_probe_count, fn + 16);
                else if (!strncmp(fn, "bandwidth_threshold=", 20)) load_int(&dawn_metric.bandwidth_threshold, fn + 20);
                else if (!strncmp(fn, "use_station_count=", 18)) load_int(&dawn_metric.use_station_count, fn + 18);
                else if (!strncmp(fn, "max_station_diff=", 17)) load_int(&dawn_metric.max_station_diff, fn + 17);
                else if (!strncmp(fn, "eval_probe_req=", 15)) load_int(&dawn_metric.eval_probe_req, fn + 15);
                else if (!strncmp(fn, "eval_auth_req=", 14)) load_int(&dawn_metric.eval_auth_req, fn + 14);
                else if (!strncmp(fn, "eval_assoc_req=", 15)) load_int(&dawn_metric.eval_assoc_req, fn + 15);
                else if (!strncmp(fn, "deny_auth_reason=", 17)) load_int(&dawn_metric.deny_auth_reason, fn + 17);
                else if (!strncmp(fn, "deny_assoc_reason=", 18)) load_int(&dawn_metric.deny_assoc_reason, fn + 18);
                else if (!strncmp(fn, "min_number_to_kick=", 19)) load_int(&dawn_metric.min_number_to_kick, fn + 19);
                else if (!strncmp(fn, "chan_util_avg_period=", 21)) load_int(&dawn_metric.chan_util_avg_period, fn + 21);
                else if (!strncmp(fn, "set_hostapd_nr=", 15)) load_int(&dawn_metric.set_hostapd_nr, fn + 15);
                else if (!strncmp(fn, "disassoc_nr_length=", 19)) load_int(&dawn_metric.disassoc_nr_length, fn + 19);
                else if (!strncmp(fn, "kicking=", 8)) load_int(&dawn_metric.kicking, fn + 8);
                else if (!strncmp(fn, "duration=", 9)) load_int(&dawn_metric.duration, fn + 9);
                else if (!strncmp(fn, "rrm_mode=", 9)) dawn_metric.rrm_mode_mask = parse_rrm_mode(dawn_metric.rrm_mode_order, fn + 9);
                else {
                    printf("ERROR: Loading DAWN control metrics, but don't recognise assignment \"%s\"\n", fn);
                    ret = 1;
                }

                if (ret == 0)
                    args_required++;
            }
        }
        else if (strcmp(*argv, "macadd") == 0)
        {

            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                struct dawn_mac mac0;

                hwaddr_aton(argv[1], mac0.u8);
                insert_to_maclist(mac0);
            }
        }
        else if (strcmp(*argv, "macget") == 0)
        {

            args_required = 2;
            if (curr_arg + args_required <= argc)
            {
                struct dawn_mac mac0;

                hwaddr_aton(argv[1], mac0.u8);
                printf("Looking for MAC %s - result %d\n", argv[1], mac_find_entry(mac0) == NULL);
            }
        }
        else if (strcmp(*argv, "ap") == 0)
        {
            ap *ap0 = dawn_malloc(sizeof(struct ap_s));

            ap0->freq = 0;
            ap0->ht_support = 0;
            ap0->vht_support = 0;
            ap0->channel_utilization = 0;
            ap0->time = faketime;
            ap0->station_count = 0;
            memset(ap0->ssid, '*', SSID_MAX_LEN);
            ap0->ssid[SSID_MAX_LEN] = '\0';
            ap0->neighbor_report[0] = 0;
            //ap0->collision_domain = 0;
            //ap0->bandwidth = 0;
            ap0->ap_weight = 0;

            args_required = 1;
            while (ret == 0 && curr_arg + args_required < argc)
            {
                char* fn = *(argv + args_required);

                //TODO: Somehwat hacky parsing of value strings to get us going...
                if (false);  // Hack to allow easy paste of generated code
                else if (!strncmp(fn, "bssid=", 6)) hwaddr_aton(fn + 6, ap0->bssid_addr.u8);
                else if (!strncmp(fn, "freq=", 5)) load_u32(&ap0->freq, fn + 5);
                else if (!strncmp(fn, "ht_sup=", 7)) load_u8(&ap0->ht_support, fn + 7);
                else if (!strncmp(fn, "vht_sup=", 8)) load_u8(&ap0->vht_support, fn + 8);
                else if (!strncmp(fn, "util=", 5)) load_u32(&ap0->channel_utilization, fn + 5);
                else if (!strncmp(fn, "time=", 5)) load_time(&ap0->time, fn + 5);
                else if (!strncmp(fn, "stations=", 9)) load_u32(&ap0->station_count, fn + 9);
                else if (!strncmp(fn, "ssid=", 5)) load_ssid(ap0->ssid, fn + 5);
                else if (!strncmp(fn, "neighbors=", 10)) load_string(NEIGHBOR_REPORT_LEN, ap0->neighbor_report, fn + 10);
                else if (!strncmp(fn, "weight=", 7)) load_u32(&ap0->ap_weight, fn + 7);
                else {
                    printf("ERROR: Loading AP, but don't recognise assignment \"%s\"\n", fn);
                    ret = 1;
                }

                if (ret == 0)
                {
                    args_required++;
                }
            }

            if (ret == 0)
                insert_to_ap_array(ap0, ap0->time);
            else
                dawn_free(ap0);
        }
        else if (strcmp(*argv, "client") == 0)
        {
            client *cl0 = dawn_malloc(sizeof(struct client_s));
            //TODO: NULL test

            memset(cl0->signature, 0, SIGNATURE_LEN);
            cl0->ht_supported = 0;
            cl0->vht_supported = 0;
            cl0->freq = 0;
            cl0->auth = 0;
            cl0->assoc = 0;
            cl0->authorized = 0;
            cl0->preauth = 0;
            cl0->wds = 0;
            cl0->wmm = 0;
            cl0->ht = 0;
            cl0->vht = 0;
            cl0->wps = 0;
            cl0->mfp = 0;
            cl0->time = faketime;
            cl0->aid = 0;
            cl0->kick_count = 0;

            args_required = 1;
            while (ret == 0 && curr_arg + args_required < argc)
            {
                char* fn = *(argv + args_required);

                //TODO: Somewhat hacky parsing of value strings to get us going...
                if (false);  // Hack to allow easy paste of generated code
                else if (!strncmp(fn, "bssid=", 6)) hwaddr_aton(fn + 6, cl0->bssid_addr.u8);
                else if (!strncmp(fn, "client=", 7)) hwaddr_aton(fn + 7, cl0->client_addr.u8);
                else if (!strncmp(fn, "sig=", 4)) load_string(SIGNATURE_LEN, cl0->signature, fn + 4);
                else if (!strncmp(fn, "ht_sup=", 7)) load_u8(&cl0->ht_supported, fn + 7);
                else if (!strncmp(fn, "vht_sup=", 8)) load_u8(&cl0->vht_supported, fn + 8);
                else if (!strncmp(fn, "freq=", 5)) load_u32(&cl0->freq, fn + 5);
                else if (!strncmp(fn, "auth=", 5)) load_u8(&cl0->auth, fn + 5);
                else if (!strncmp(fn, "assoc=", 6)) load_u8(&cl0->assoc, fn + 6);
                else if (!strncmp(fn, "authz=", 6)) load_u8(&cl0->authorized, fn + 6);
                else if (!strncmp(fn, "preauth=", 8)) load_u8(&cl0->preauth, fn + 8);
                else if (!strncmp(fn, "wds=", 4)) load_u8(&cl0->wds, fn + 4);
                else if (!strncmp(fn, "wmm=", 4)) load_u8(&cl0->wmm, fn + 4);
                else if (!strncmp(fn, "ht_cap=", 3)) load_u8(&cl0->ht, fn + 3);
                else if (!strncmp(fn, "vht_cap=", 4)) load_u8(&cl0->vht, fn + 4);
                else if (!strncmp(fn, "wps=", 4)) load_u8(&cl0->wps, fn + 4);
                else if (!strncmp(fn, "mfp=", 4)) load_u8(&cl0->mfp, fn + 4);
                else if (!strncmp(fn, "time=", 5)) load_time(&cl0->time, fn + 5);
                else if (!strncmp(fn, "aid=", 4)) load_u32(&cl0->aid, fn + 4);
                else if (!strncmp(fn, "kick=", 5)) load_u32(&cl0->kick_count, fn + 5);
                else {
                    printf("ERROR: Loading CLIENT, but don't recognise assignment \"%s\"\n", fn);
                    ret = 1;
                }

                if (ret == 0)
                    args_required++;
            }

            if (ret == 0)
            {
                insert_client_to_array(cl0, cl0->time);
            }
        }
        else if (strcmp(*argv, "probe") == 0)
        {
            probe_entry* pr0 = NULL;

            struct dawn_mac bmac;
            struct dawn_mac cmac;
            int key_check = 1 | 2;

            args_required = 1;
            while (ret == 0 && curr_arg + args_required < argc)
            {
                char* fn = *(argv + args_required);

                //TODO: Somewhat hacky parsing of value strings to get us going...
                // bssid and client must be specified first so we can work out if we need a new entry or update an old one
                if (false);  // Hack to allow easy paste of generated code
                else if (!strncmp(fn, "bssid=", 6)) { hwaddr_aton(fn + 6, bmac.u8); key_check ^= ~1; }
                else if (!strncmp(fn, "client=", 7)) { hwaddr_aton(fn + 7, cmac.u8); key_check ^= ~2; }
                else if (!strncmp(fn, "target=", 7)) hwaddr_aton(fn + 7, pr0->target_addr.u8);
                else if (!strncmp(fn, "signal=", 7)) load_u32(&pr0->signal, fn + 7);
                else if (!strncmp(fn, "freq=", 5)) load_u32(&pr0->freq, fn + 5);
                else if (!strncmp(fn, "ht_cap=", 7)) load_u8(&pr0->ht_capabilities, fn + 7);
                else if (!strncmp(fn, "vht_cap=", 8)) load_u8(&pr0->vht_capabilities, fn + 8);
                else if (!strncmp(fn, "time=", 5)) load_time(&pr0->rssi_timestamp, fn + 5);
                else if (!strncmp(fn, "time2=", 5)) load_time(&pr0->rcpi_timestamp, fn + 5);
                else if (!strncmp(fn, "counter=", 8)) load_int(&pr0->counter, fn + 8);
                else if (!strncmp(fn, "deny=", 5)) load_int(&pr0->deny_counter, fn + 5);
                else if (!strncmp(fn, "max_rate=", 9)) load_u8(&pr0->max_supp_datarate, fn + 9);
                else if (!strncmp(fn, "min_rate=", 9)) load_u8(&pr0->min_supp_datarate, fn + 9);
                else if (!strncmp(fn, "rcpi=", 5)) load_u32(&pr0->rcpi, fn + 5);
                else if (!strncmp(fn, "rsni=", 5)) load_u32(&pr0->rsni, fn + 5);
                else {
                    printf("ERROR: Loading PROBE, but don't recognise assignment \"%s\"\n", fn);
                    ret = 1;
                }

                if (key_check == 0)
                {
                    key_check = -1;

                    // See if this entry already exists
                    pr0 = probe_array_get_entry(cmac, bmac);

                    // If not, create and initialise it
                    if (pr0 != NULL)
                    {
                        if (harness_verbosity > 1)
                            printf("probe: updating existing entry...\n");
                        pr0->counter++;
                    }
                    else
                    {
                        if (harness_verbosity > 1)
                            printf("probe: creating new entry...\n");
                        // MUSTDO: Check all new dawn_malloc() returns
                        pr0 = dawn_malloc(sizeof(probe_entry));

                        pr0->bssid_addr = bmac;
                        pr0->client_addr = cmac;

                        memset(pr0->target_addr.u8, 0, ETH_ALEN);
                        pr0->signal = 0;
                        pr0->freq = 0;
                        pr0->ht_capabilities = 0;
                        pr0->vht_capabilities = 0;
                        pr0->rssi_timestamp = faketime;
                        pr0->rcpi_timestamp = 0;
                        pr0->counter = 0;
                        pr0->deny_counter = 0;
                        pr0->max_supp_datarate = 0;
                        pr0->min_supp_datarate = 0;
                        pr0->rcpi = 0;
                        pr0->rsni = 0;

                        insert_to_probe_array(pr0, true, true, pr0->rssi_timestamp);
                    }
                }

                if (ret == 0)
                    args_required++;
            }
        }
        else if (strcmp(*argv, "kick") == 0) // Perform kicking evaluation
        {
            args_required = 3;
            if (curr_arg + args_required <= argc)
            {
                int safety_count = 1000;

                struct dawn_mac kick_mac;
                uint32_t kick_id;

                hwaddr_aton(argv[1], kick_mac.u8);
                load_u32(&kick_id, argv[2]);

                while ((kick_clients(kick_mac, kick_id) != 0) && safety_count--);
            }
        }
        else if (strcmp(*argv, "better_ap_available") == 0)
        {
            args_required = 5;
            if (curr_arg + args_required <= argc)
            {
                struct dawn_mac bssid_mac;
                struct dawn_mac client_mac;
                uint32_t autokick;
                uint32_t with_nr;

                int tr = 9999; // Tamper evident value

                hwaddr_aton(argv[1], bssid_mac.u8);
                hwaddr_aton(argv[2], client_mac.u8);
                load_u32(&autokick, argv[3]);
                load_u32(&with_nr, argv[4]);

                struct kicking_nr *neighbor_list = NULL;

                tr = better_ap_available(ap_array_get_ap(bssid_mac), probe_array_get_entry(client_mac, bssid_mac), 100, with_nr ? &neighbor_list : NULL);

                printf("better_ap_available returned %d (with neighbour report %s)\n", tr, neighbor_list ? neighbor_list->nr_ap->neighbor_report : "NONE");
            }
        }
        else if (strcmp(*argv, "eval_probe_metric") == 0)
        {
            args_required = 3;
            if (curr_arg + args_required <= argc)
            {
                struct dawn_mac client_mac;
                struct dawn_mac bssid_mac;

                hwaddr_aton(argv[2], client_mac.u8);
                hwaddr_aton(argv[1], bssid_mac.u8);

                probe_entry *pr0 = probe_array_get_entry(client_mac, bssid_mac);

                if (pr0 == NULL )
                {
                    printf("eval_probe_metric: Can't find probe entry!\n");
                }
                else
                {
                    ap* ap_entry = ap_array_get_ap(pr0->bssid_addr);

                    int this_metric = eval_probe_metric(pr0, ap_entry);
                    dawnlog_info("Score: %d of:\n", this_metric);
                    print_probe_entry(DAWNLOG_DEBUG, pr0);

                    printf("eval_probe_metric: Returned %d\n", this_metric);
                }

            }
        }
        else
        {
            args_required = 1;

            printf("COMMAND \"%s\": Unknown - stopping!\n", *argv);
            ret = -1;
        }

        curr_arg += args_required;
        if (curr_arg <= argc)
        {
            // Still need to continue consuming args
            argv += args_required;

            if (faketime_auto)
            {
                time_moves_on();

                if (harness_verbosity > 0)
                {
                    if (!(faketime & 1))
                        printf("Faketime: tick %" PRId64 "...\n", (int64_t)faketime);
                    else
                        printf("Faketime: tock %" PRId64 "...\n", (int64_t)faketime);
                }
            }
        }
        else
        {
            // There aren't enough args left to give the parameters of the current action
            printf("Commands are mangled at: \"%s\"!\n", *argv);
            ret = -1;
        }
    }

    return ret;
}

#define MAX_LINE_ARGS 20
static int process_script_line(char* line, size_t len, int harness_verbosity);
static int process_script_line(char* line, size_t len, int harness_verbosity)
{
    int argc = 0;
    char* argv[MAX_LINE_ARGS];
    bool in_white = true;
    bool force_nul = false;

    int ret = 0;

    //printf("%lu: \"%s\"\n", len, line);
    while (len > 0 && !ret)
    {
        if (isblank(*line) || (*line == '\n') || (*line == '\r') || (*line == '#') || force_nul)
        {
            if (*line == '#')
            {
                //printf("Blanking 0x%02X...\n", *line);
                force_nul = true;

                // Untested - might bring parsing to faster end...
                // len = 1;
            }

            //printf("Zapping 0x%02X...\n", *line);
            *line = '\0';
            in_white = true;
        }
        else
        {
            if (in_white)
            {
                //printf("Marking 0x%02X...\n", *line);
                if (argc == MAX_LINE_ARGS)
                {
                    printf("ERROR: Script line exceeds permitted arg count!\n");
                    ret = -1;
                }
                else
                {
                    argv[argc] = line;
                    argc++;

                    in_white = false;
                }
            }
        }

        len--;
        line++;
    }

    if (!ret)
        ret = consume_actions(argc, argv, harness_verbosity);

    return ret;
}

int main(int argc, char* argv[])
{
    FILE* fp = NULL;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    int ret = 0;
    int harness_verbosity = 1;

    dawnlog_dest(DAWNLOG_DEST_STDIO); // Send messages to stderr / stdout

    printf("DAWN datastorage.c test harness...\n\n");

    if ((argc == 1) || !strcmp(*(argv + 1), "help") || !strcmp(*(argv + 1), "--help") || !strcmp(*(argv + 1), "-h"))
    {
        printf("Usage: %s [commands]\n\n", *argv);
        printf("  [action [arg...]]... : Read test actions from command line\n");
        printf("    --script [file]... : Read test script from file(s) (NB: \"-\" is a valid name\n");
        printf("                         indicating STDIN) {-s}\n");
        printf("                     - : Read test script from STDIN (and remaining arguments\n");
        printf("                         as script file names)\n");
        printf("                --help : This help message {-h, help}\n");
        printf("NB: Contents of {braces} indicate equivalent command\n");
    }
    else
    {
        init_mutex();

        // Step past command name on args, ie argv[0]
        argc--;
        argv++;

        if (!strcmp(*argv, "--quiet") || !strcmp(*argv, "-q"))
        {
            harness_verbosity = 0;

            argc--;
            argv++;
        }

        if (!strcmp(*argv, "--verbose") || !strcmp(*argv, "-v"))
        {
            harness_verbosity = 2;

            argc--;
            argv++;
        }

        if (!strcmp(*argv, "--script") || !strcmp(*argv, "-s") || !strcmp(*argv, "-"))
        {
            if (!strcmp(*argv, "--script") || !strcmp(*argv, "-s"))
            {
                argc--;
                argv++;
            }

            // Read script from file[s]
            while (argc > 0 && ret == 0)
            {
                if (!strcmp(*argv, "-"))
                {
                    fp = stdin;
                    if (harness_verbosity > 0)
                        printf("Consuming script from STDIN\n");
                }
                else
                {
                    fp = fopen(*argv, "r");
                    if (fp == NULL)
                    {
                        printf("Error opening script file: %s\n", *argv);
                        ret = -1;
                    }
                    else
                    {
                        if (harness_verbosity > 0)
                            printf("Consuming script file: %s\n", *argv);
                    }
                }

                if (ret == 0)
                {
                    read = getline(&line, &len, fp);
                    dawn_regmem(line);
                    while (!ret && read != -1)
                    {
                        if (harness_verbosity > 0)
                            printf("Processing: %s\n", line);
                        ret = process_script_line(line, read, harness_verbosity);
                        if (!ret)
                            read = getline(&line, &len, fp);
                    }

                    if (fp && fp != stdin)
                    {
                        fclose(fp);
                        fp = NULL;
                    }

                    if (line)
                    {
                        dawn_free(line);
                        line = NULL;
                    }
                }

                argc--;
                argv++;
            }
        }
        else
        {
            // Take direct input on command line
            ret = consume_actions(argc, argv, harness_verbosity);
        }

        destroy_mutex();
    }

    if (harness_verbosity > 0)
        printf("\nDAWN datastorage.c test harness - finshed.  \n");

    return ret;
}
