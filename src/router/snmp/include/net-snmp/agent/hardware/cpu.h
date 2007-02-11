typedef struct netsnmp_cpu_info_s netsnmp_cpu_info;
extern int cpu_num;

                 /* For rolling averages */
struct netsnmp_cpu_history {
     long user_hist;
     long sys_hist;
     long idle_hist;
     long nice_hist;
     long total_hist;

     long ctx_hist;
     long intr_hist;
     long swpi_hist;
     long swpo_hist;
     long pagei_hist;
     long pageo_hist;
};

struct netsnmp_cpu_info_s {
     int  idx;
                 /* For hrDeviceTable */
     char name[  SNMP_MAXBUF ];
     char descr[ SNMP_MAXBUF ];
     int  status;

                 /* For UCD cpu stats */
     long user_ticks;
     long nice_ticks;
     long sys_ticks;
     long idle_ticks;
     long wait_ticks;
     long kern_ticks;
     long intrpt_ticks;
     long sirq_ticks;

     long total_ticks;
     long sys2_ticks;  /* For non-atomic system counts */

                 /* For paging-related UCD stats */
              /* XXX - Do these belong elsewhere ?? */
              /* XXX - Do Not Use - Subject to Change */
     long pageIn;
     long pageOut;
     long swapIn;
     long swapOut;
     long nInterrupts;
     long nCtxSwitches;

     struct netsnmp_cpu_history *history;

     netsnmp_cpu_info *next;
};


    /*
     * Possibly not all needed ??
     */
netsnmp_cpu_info *netsnmp_cpu_get_first(  void );
netsnmp_cpu_info *netsnmp_cpu_get_next( netsnmp_cpu_info* );
netsnmp_cpu_info *netsnmp_cpu_get_byIdx(  int,   int );
netsnmp_cpu_info *netsnmp_cpu_get_byName( char*, int );

netsnmp_cache *netsnmp_cpu_get_cache( void );
int netsnmp_cpu_load( void );
