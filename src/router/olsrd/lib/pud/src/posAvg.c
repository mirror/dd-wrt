#include "posAvg.h"

/* Plugin includes */

/* OLSR includes */
#include "olsr.h"

/* System includes */
#include <assert.h>
#include <nmea/sentence.h>

/* Defines */

#define LISTSIZE(x)			(((x)->entriesMaxCount) + 1) /* always valid */
#define NEWESTINDEX(x)		((x)->newestEntryIndex) /* always valid */
#define WRAPINDEX(x, i)		((i) % LISTSIZE(x)) /* always valid for i>=0 */
#define INCOMINGINDEX(x)	WRAPINDEX(x, (NEWESTINDEX(x) + 1)) /* always valid */
#define OLDESTINDEX(x)		(((x)->entriesCount > 1) ? WRAPINDEX(x, (INCOMINGINDEX(x) + LISTSIZE(x) - (x)->entriesCount)) : NEWESTINDEX(x)) /* always valid */

/**
 Flush/empty the position average list

 @param positionAverageList
 The position average list
 */
void flushPositionAverageList(PositionAverageList * positionAverageList) {
	assert (positionAverageList != NULL);

	positionAverageList->entriesCount = 0;
	memset(&positionAverageList->counters, 0,
			sizeof(positionAverageList->counters));

	nmea_zero_INFO(&positionAverageList->positionAverageCumulative.nmeaInfo);
	nmea_zero_INFO(&positionAverageList->positionAverage.nmeaInfo);
}

/**
 Initialise the position average list: allocate memory for the entries and
 reset fields.

 @param positionAverageList
 The position average list
 @param maxEntries
 The maximum number of entries in the list (the number of entries that should
 be averaged)

 @return
 - false on failure
 - true otherwise
 */
bool initPositionAverageList(PositionAverageList * positionAverageList,
		unsigned long long maxEntries) {
	void * p;

	if (positionAverageList == NULL) {
		return false;
	}
	if (maxEntries < 2) {
		return false;
	}

	p = olsr_malloc((maxEntries + 1) * sizeof(PositionUpdateEntry),
			"PositionAverageEntry entries for PositionAverageList (PUD)");
	if (p == NULL) {
		return false;
	}

	positionAverageList->entriesMaxCount = maxEntries;
	positionAverageList->entries = p;
	positionAverageList->newestEntryIndex = 0;

	flushPositionAverageList(positionAverageList);

	return true;
}

/**
 Clean up the position average list: free memory and reset fields.

 @param positionAverageList
 The position average list
 */
void destroyPositionAverageList(PositionAverageList * positionAverageList) {
	assert (positionAverageList != NULL);

	flushPositionAverageList(positionAverageList);

	if (positionAverageList->entries != NULL) {
		free(positionAverageList->entries);
		positionAverageList->entries = NULL;
	}

	positionAverageList->entriesMaxCount = 0;
	positionAverageList->newestEntryIndex = 0;
}

/**
 Get the entry for a certain type of position update.

 @param positionAvgList
 The position average list
 @param positionType
 The type of the position of the average entry

 @return
 A pointer to the requested position update entry
 */
PositionUpdateEntry * getPositionAverageEntry(
		PositionAverageList * positionAvgList,
		AverageEntryPositionType positionType) {
	PositionUpdateEntry * r = NULL;

	switch (positionType) {
		case OLDEST:
			assert(positionAvgList->entriesCount >= positionAvgList->entriesMaxCount);
			r = &positionAvgList->entries[OLDESTINDEX(positionAvgList)];
			break;

		case INCOMING:
			r = &positionAvgList->entries[INCOMINGINDEX(positionAvgList)];
			break;

		case NEWEST:
			r = &positionAvgList->entries[NEWESTINDEX(positionAvgList)];
			break;

		case AVERAGECUMULATIVE:
			r = &positionAvgList->positionAverageCumulative;
			break;

		case AVERAGE:
			r = &positionAvgList->positionAverage;
			break;

		default:
			r = NULL;
			break;
	}

	return r;
}

/**
 Update position average mask and fix counters for a new entry or for an entry
 that is/will be removed. Update the respective counters when the smask of the
 entry has the corresponding flag set. The fix counters count the fix values
 separately.

 @param positionAverageList
 The position average list
 @param entry
 The entry to update the counters from
 @param add
 True when updating the counters for a new entry, false for an entry that
 is/will be removed
 */
static void updateCounters(PositionAverageList * positionAverageList,
		PositionUpdateEntry * entry, bool add) {
	PositionUpdateCounters * counters = &positionAverageList->counters;
#ifndef NDEBUG
	unsigned long long maxCount = positionAverageList->entriesMaxCount;
#endif
	int amount = (add ? 1 : -1);

	/* smask */
	if ((entry->nmeaInfo.smask & GPGGA) != 0) {
		assert(add ? (counters->gpgga < maxCount):(counters->gpgga > 0));
		counters->gpgga += amount;
	}
	if ((entry->nmeaInfo.smask & GPGSA) != 0) {
		assert(add ? (counters->gpgsa < maxCount):(counters->gpgsa > 0));
		counters->gpgsa += amount;
	}
	if ((entry->nmeaInfo.smask & GPGSV) != 0) {
		assert(add ? (counters->gpgsv < maxCount):(counters->gpgsv > 0));
		counters->gpgsv += amount;
	}
	if ((entry->nmeaInfo.smask & GPRMC) != 0) {
		assert(add ? (counters->gprmc < maxCount):(counters->gprmc > 0));
		counters->gprmc += amount;
	}
	if ((entry->nmeaInfo.smask & GPVTG) != 0) {
		assert(add ? (counters->gpvtg < maxCount):(counters->gpvtg > 0));
		counters->gpvtg += amount;
	}

	/* sig */
	if (nmea_INFO_has_field(entry->nmeaInfo.smask, SIG)) {
		if (entry->nmeaInfo.sig == NMEA_SIG_HIGH) {
			assert(add ? (counters->sigHigh < maxCount):(counters->sigHigh > 0));
			counters->sigHigh += amount;
		} else if (entry->nmeaInfo.sig == NMEA_SIG_MID) {
			assert(add ? (counters->sigMid < maxCount):(counters->sigMid > 0));
			counters->sigMid += amount;
		} else if (entry->nmeaInfo.sig == NMEA_SIG_LOW) {
			assert(add ? (counters->sigLow < maxCount):(counters->sigLow > 0));
			counters->sigLow += amount;
		} else {
			assert(add ? (counters->sigBad < maxCount):(counters->sigBad > 0));
			counters->sigBad += amount;
		}
	}

	/* fix */
	if (nmea_INFO_has_field(entry->nmeaInfo.smask, FIX)) {
		if (entry->nmeaInfo.fix == NMEA_FIX_3D) {
			assert(add ? (counters->fix3d < maxCount):(counters->fix3d > 0));
			counters->fix3d += amount;
		} else if (entry->nmeaInfo.fix == NMEA_FIX_2D) {
			assert(add ? (counters->fix2d < maxCount):(counters->fix2d > 0));
			counters->fix2d += amount;
		} else {
			assert(add ? (counters->fixBad < maxCount):(counters->fixBad > 0));
			counters->fixBad += amount;
		}
	}
}

/**
 Determine the new smask, sig and fix of the average position based on the
 counters. The relevant smask bits (like GPGGA) are only set when all entries
 in the average list have that bit set. The sig and fix will be set to the
 lowest/worst value of all entries and will only be set to the highest/best
 value when all entries in the average list are set to the highest/best value.

 @param positionAverageList
 The position average list
 */
static void determineCumulativeSmaskSigFix(
		PositionAverageList * positionAverageList) {
	PositionUpdateEntry * cumulative =
			&positionAverageList->positionAverageCumulative;
	PositionUpdateCounters * counters = &positionAverageList->counters;
	unsigned long long count = positionAverageList->entriesCount;

	/* smask */
	cumulative->nmeaInfo.smask = 0;

	if (counters->gpgga >= count) {
		cumulative->nmeaInfo.smask |= GPGGA;
	}

	if (counters->gpgsa >= count) {
		cumulative->nmeaInfo.smask |= GPGSA;
	}

	if (counters->gpgsv >= count) {
		cumulative->nmeaInfo.smask |= GPGSV;
	}

	if (counters->gprmc >= count) {
		cumulative->nmeaInfo.smask |= GPRMC;
	}

	if (counters->gpvtg >= count) {
		cumulative->nmeaInfo.smask |= GPVTG;
	}

	/* sig */
	cumulative->nmeaInfo.sig = NMEA_SIG_BAD;
	if (nmea_INFO_has_field(cumulative->nmeaInfo.smask, SIG)) {
		if (counters->sigBad == 0) {
			if (counters->sigHigh >= count) {
				cumulative->nmeaInfo.sig = NMEA_SIG_HIGH;
			} else if (counters->sigMid > 0) {
				cumulative->nmeaInfo.sig = NMEA_SIG_MID;
			} else if (counters->sigLow > 0) {
				cumulative->nmeaInfo.sig = NMEA_SIG_LOW;
			}
		}
	}

	/* fix */
	cumulative->nmeaInfo.fix = NMEA_FIX_BAD;
	if (nmea_INFO_has_field(cumulative->nmeaInfo.smask, FIX)) {
		if (counters->fixBad == 0) {
			if (counters->fix3d >= count) {
				cumulative->nmeaInfo.fix = NMEA_FIX_3D;
			} else if (counters->fix2d > 0) {
				cumulative->nmeaInfo.fix = NMEA_FIX_2D;
			}
		}
	}
}

/**
 Add/remove a position update entry to/from the average position list, updates
 the counters, adjusts the entriesCount and redetermines the cumulative
 smask, sig and fix.

 @param positionAverageList
 The position average list
 @param entry
 The entry to add/remove
 @param add
 True when the entry must be added to the list, false when it must be removed
 */
static void addOrRemoveEntryToFromCumulativeAverage(
		PositionAverageList * positionAverageList, PositionUpdateEntry * entry,
		bool add) {
	PositionUpdateEntry * cumulative =
			&positionAverageList->positionAverageCumulative;

	if (!add) {
		assert(positionAverageList->entriesCount >= positionAverageList->entriesMaxCount);
		assert(entry == getPositionAverageEntry(positionAverageList, OLDEST));

		/* do not touch smask */

		/* do not touch utc */

		/* do not touch sig */
		/* do not touch fix */

		/* do not touch satinfo */
	} else {
		assert(positionAverageList->entriesCount < positionAverageList->entriesMaxCount);
		assert(entry == getPositionAverageEntry(positionAverageList, INCOMING));

		/* smask at the end */

		/* use the latest utc */
		cumulative->nmeaInfo.utc = entry->nmeaInfo.utc;

		/* sig at the end */
		/* fix at the end */

		/* use the latest satinfo */
		cumulative->nmeaInfo.satinfo = entry->nmeaInfo.satinfo;
	}

	/* PDOP, HDOP, VDOP */
	cumulative->nmeaInfo.PDOP += add ? entry->nmeaInfo.PDOP
			: -entry->nmeaInfo.PDOP;
	cumulative->nmeaInfo.HDOP += add ? entry->nmeaInfo.HDOP
			: -entry->nmeaInfo.HDOP;
	cumulative->nmeaInfo.VDOP += add ? entry->nmeaInfo.VDOP
			: -entry->nmeaInfo.VDOP;

	/* lat, lon */
	cumulative->nmeaInfo.lat += add ? entry->nmeaInfo.lat
			: -entry->nmeaInfo.lat;
	cumulative->nmeaInfo.lon += add ? entry->nmeaInfo.lon
			: -entry->nmeaInfo.lon;

	/* elv, speed, direction, declination */
	cumulative->nmeaInfo.elv += add ? entry->nmeaInfo.elv
			: -entry->nmeaInfo.elv;
	cumulative->nmeaInfo.speed += add ? entry->nmeaInfo.speed
			: -entry->nmeaInfo.speed;
	cumulative->nmeaInfo.direction += add ? entry->nmeaInfo.direction
			: -entry->nmeaInfo.direction;
	cumulative->nmeaInfo.declination += add ? entry->nmeaInfo.declination
			: -entry->nmeaInfo.declination;

	positionAverageList->entriesCount += (add ? 1 : -1);

	updateCounters(positionAverageList, entry, add);
	determineCumulativeSmaskSigFix(positionAverageList);
}

/**
 Update the average position from the cumulative average position. Basically
 divide all relevant cumulative values by the number of entries in the list.

 @param positionAverageList
 The position average list
 */
static void updatePositionAverageFromCumulative(
		PositionAverageList * positionAverageList) {
	double divider = positionAverageList->entriesCount;

	positionAverageList->positionAverage = positionAverageList->positionAverageCumulative;

	/* smask: use from cumulative average */

	/* utc: use from cumulative average */

	/* sig: use from cumulative average */
	/* fix: use from cumulative average */

	if (divider > 1.0) {
		positionAverageList->positionAverage.nmeaInfo.PDOP /= divider;
		positionAverageList->positionAverage.nmeaInfo.HDOP /= divider;
		positionAverageList->positionAverage.nmeaInfo.VDOP /= divider;

		positionAverageList->positionAverage.nmeaInfo.lat /= divider;
		positionAverageList->positionAverage.nmeaInfo.lon /= divider;

		positionAverageList->positionAverage.nmeaInfo.elv /= divider;
		positionAverageList->positionAverage.nmeaInfo.speed /= divider;
		positionAverageList->positionAverage.nmeaInfo.direction /= divider;
		positionAverageList->positionAverage.nmeaInfo.declination /= divider;
	}

	/* satinfo: use from average */
}

/**
 Add a new (incoming) position update to the position average list

 @param positionAverageList
 The position average list
 @param newEntry
 The new (incoming) position update (must be the same as the one returned from
 the function getPositionAverageEntryForIncoming:INCOMING)
 */
void addNewPositionToAverage(PositionAverageList * positionAverageList,
		PositionUpdateEntry * newEntry) {
	assert (positionAverageList != NULL);
	assert (newEntry == getPositionAverageEntry(positionAverageList, INCOMING));

	if (positionAverageList->entriesCount
			>= positionAverageList->entriesMaxCount) {
		/* list is full, so first remove the oldest from the average */
		addOrRemoveEntryToFromCumulativeAverage(positionAverageList,
				getPositionAverageEntry(positionAverageList, OLDEST), false);
	}

	/* now just add the new position */
	addOrRemoveEntryToFromCumulativeAverage(positionAverageList, newEntry, true);

	/* update the place where the new entry is stored */
	positionAverageList->newestEntryIndex
			= WRAPINDEX(positionAverageList, NEWESTINDEX(positionAverageList) + 1);

	/* update average position */
	updatePositionAverageFromCumulative(positionAverageList);
}
