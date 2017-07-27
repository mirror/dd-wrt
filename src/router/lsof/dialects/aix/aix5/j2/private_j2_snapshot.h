/*
 * j2_snapshot.h -- lsof private copy
 *
 * Needed for:
 *
 *	AIX 5.2, because this header file is missing and j2_inode.h #includes
 *	    it.  The dummy snapshotObject structure definition is needed by
 *          some releases of AIX 5.2 and above, but the structure's size does
 *	    not affect lsof's use of the JFS2 inode structure.
 */

#if	!defined(_H_J2_SNAPSHOT)
#define	_H_J2_SNAPSHOT
struct snapshotObject {
	uint64 d1;
};
#endif	/* !defined(_H_J2_SNAPSHOT) */
