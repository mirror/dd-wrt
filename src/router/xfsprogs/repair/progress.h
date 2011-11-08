#ifndef	_XFS_REPAIR_PROGRESS_RPT_H_
#define	_XFS_REPAIR_PROGRESS_RPT_H_

#define PROG_RPT_DEFAULT	(15*60)	 /* default 15 minute report interval */
#define	PHASE_START		0
#define	PHASE_END		1


#define	PROG_FMT_SCAN_AG 	0	/* Phase 2 */

#define	PROG_FMT_AGI_UNLINKED 	1	/* Phase 3 */
#define	PROG_FMT_UNCERTAIN      2
#define	PROG_FMT_PROCESS_INO	3
#define	PROG_FMT_NEW_INODES	4

#define	PROG_FMT_DUP_EXTENT	5	/* Phase 4 */
#define	PROG_FMT_INIT_RTEXT	6
#define	PROG_FMT_RESET_RTBM	7
#define	PROG_FMT_DUP_BLOCKS	8

#define	PROG_FMT_REBUILD_AG	9	/* Phase 5 */

#define	PROG_FMT_TRAVERSAL	10	/* Phase 6 */
#define	PROG_FMT_TRAVERSSUB	11
#define	PROG_FMT_DISCONINODE	12

#define	PROGRESS_FMT_CORR_LINK	13	/* Phase 7 */
#define	PROGRESS_FMT_VRFY_LINK 	14

#define	DURATION_BUF_SIZE	512

extern void init_progress_rpt(void);
extern void stop_progress_rpt(void);
extern void summary_report(void);
extern int  set_progress_msg(int report, __uint64_t total);
extern __uint64_t print_final_rpt(void);
extern char *timestamp(int end, int phase, char *buf);
extern char *duration(int val, char *buf);
extern int do_parallel;

#define	PROG_RPT_INC(a,b) if (ag_stride && prog_rpt_done) (a) += (b)

#endif	/* _XFS_REPAIR_PROGRESS_RPT_H_ */
