#ifndef BTCONFIG_H
#define BTCONFIG_H

extern size_t cfg_req_slice_size;

#define MAX_METAINFO_FILESIZ	4194304
#define cfg_max_slice_size 131072
#define cfg_req_queue_length 64
#define MAX_PF_LEN 8
#define PEER_ID_LEN 20
#define PEER_PFX "-CT1304-"

extern size_t cfg_cache_size;

extern size_t cfg_max_peers;
extern size_t cfg_min_peers;

extern int cfg_listen_port;
extern int cfg_max_listen_port;
extern int cfg_min_listen_port;

extern time_t cfg_seed_hours;

extern int cfg_max_bandwidth;

// arguments global value
extern char *arg_metainfo_file;
extern char *arg_bitfield_file;
extern char *arg_save_as;
extern char *arg_user_agent;

extern unsigned char arg_flg_force_seed_mode;
extern unsigned char arg_flg_check_only;
extern unsigned char arg_flg_exam_only;
extern unsigned char arg_flg_make_torrent;

extern size_t arg_piece_length;
extern char *arg_announce;
#endif
