#ifndef _PUD_POSAVG_H_
#define _PUD_POSAVG_H_

/* Plugin includes */

/* OLSR includes */

/* System includes */
#include <nmea/info.h>
#include <stdbool.h>

/** Stores an nmeaINFO entry, used in the averaging */
typedef struct _PositionUpdateEntry {
		nmeaINFO nmeaInfo; /**< the position information */
} PositionUpdateEntry;

/**
 Counts the number of GPxxx based entries. Some parameters in nmeaINFO are
 dependent on different GPxxx NMEA sentences. These counters are used to
 determine which information would be valid for the average position.
 Also counts the fix values.
 */
typedef struct _PositionUpdateCounters {
		/* smask */
		unsigned long long gpgga; /**< the number of GPGGA based entries */
		unsigned long long gpgsa; /**< the number of GPGSA based entries */
		unsigned long long gpgsv; /**< the number of GPGSV based entries */
		unsigned long long gprmc; /**< the number of GPRMC based entries */
		unsigned long long gpvtg; /**< the number of GPVTG based entries */

		/* sig */
		unsigned long long sigBad; /**< the number of entries with a bad sig */
		unsigned long long sigLow; /**< the number of entries with a low sig */
		unsigned long long sigMid; /**< the number of entries with a mid sig */
		unsigned long long sigHigh; /**< the number of entries with a high sig */

		/* fix */
		unsigned long long fixBad; /**< the number of entries with a bad fix */
		unsigned long long fix2d; /**< the number of entries with a 2D fix */
		unsigned long long fix3d; /**< the number of entries with a 3D fix */
} PositionUpdateCounters;

/**
 A list of position updates that are used to determine the average position.

 The list uses 1 extra entry: when 5 entries have to be averaged then the list
 will have 6 entries. The 6th entry is used for the incoming entry, so that
 it is already in the list (together with the old entry that will be removed
 from the average) and does not need to be copied into the list. This is for
 better performance.

 The list is a circular list.
 This means that there is a gap/unused entry in the list between the
 newest entry and the oldest entry, which is the 'incoming entry'.

 Note that 'positionAverageCumulative' stores cumulative values for parameters
 for which an average is calculated. The reason is to minimise the number of
 calculations to be performed.
 */
typedef struct _PositionAverageList {
		unsigned long long entriesMaxCount; /**< the maximum number of entries in the list */
		PositionUpdateEntry * entries; /**< the list entries */

		unsigned long long entriesCount; /**< the number of entries in the list */
		unsigned long long newestEntryIndex; /**< index of the newest entry in the list (zero-based) */
		PositionUpdateCounters counters; /**< the counters */

		PositionUpdateEntry positionAverageCumulative; /**< the average position with cumulative values */
		PositionUpdateEntry positionAverage; /**< the average position */
} PositionAverageList;

/**
 Enumeration describing the type of an entry position in the average list
 */
typedef enum _AverageEntryPositionType {
	OLDEST, NEWEST, INCOMING, AVERAGECUMULATIVE, AVERAGE
} AverageEntryPositionType;

bool initPositionAverageList(PositionAverageList * positionAverageList,
		unsigned long long maxEntries);
void flushPositionAverageList(PositionAverageList * positionAverageList);
void destroyPositionAverageList(PositionAverageList * positionAverageList);

PositionUpdateEntry * getPositionAverageEntry(
		PositionAverageList * positionAverageList,
		AverageEntryPositionType positionType);

void addNewPositionToAverage(PositionAverageList * positionAverageList,
		PositionUpdateEntry * newEntry);

#endif /* _PUD_POSAVG_H_ */
