#include "posAvg.h"

/* Plugin includes */

/* OLSR includes */
#include "olsr.h"

/* System includes */
#include <assert.h>
#include <math.h>
#include <string.h>
#include <nmea/info.h>
#include <nmea/sentence.h>
#include <nmea/gmath.h>

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
	memset(&positionAverageList->positionAverageCumulative.track, 0, sizeof(positionAverageList->positionAverageCumulative.track));
	memset(&positionAverageList->positionAverageCumulative.mtrack, 0, sizeof(positionAverageList->positionAverageCumulative.mtrack));
	memset(&positionAverageList->positionAverageCumulative.magvar, 0, sizeof(positionAverageList->positionAverageCumulative.magvar));

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
 Update position average present, smask and fix counters for a new entry or for
 an entry that is/will be removed. Update the respective counters when the smask
 of the entry has the corresponding flag set. The fix counters count the fix
 values separately.

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
	uint32_t present = entry->nmeaInfo.present;
	int smask = entry->nmeaInfo.smask;
#ifndef NDEBUG
	unsigned long long maxCount = positionAverageList->entriesMaxCount;
#endif
	int amount = (add ? 1 : -1);

	/* present */
	if ((present & SMASK) != 0) {
		assert(add ? (counters->smask < maxCount):(counters->smask > 0));
		counters->smask += amount;
	}
	if ((present & UTCDATE) != 0) {
		assert(add ? (counters->utcdate < maxCount):(counters->utcdate > 0));
		counters->utcdate += amount;
	}
	if ((present & UTCTIME) != 0) {
		assert(add ? (counters->utctime < maxCount):(counters->utctime > 0));
		counters->utctime += amount;
	}
	if ((present & SIG) != 0) {
		assert(add ? (counters->sig < maxCount):(counters->sig > 0));
		counters->sig += amount;
	}
	if ((present & FIX) != 0) {
		assert(add ? (counters->fix < maxCount):(counters->fix > 0));
		counters->fix += amount;
	}
	if ((present & PDOP) != 0) {
		assert(add ? (counters->pdop < maxCount):(counters->pdop > 0));
		counters->pdop += amount;
	}
	if ((present & HDOP) != 0) {
		assert(add ? (counters->hdop < maxCount):(counters->hdop > 0));
		counters->hdop += amount;
	}
	if ((present & VDOP) != 0) {
		assert(add ? (counters->vdop < maxCount):(counters->vdop > 0));
		counters->vdop += amount;
	}
	if ((present & LAT) != 0) {
		assert(add ? (counters->lat < maxCount):(counters->lat > 0));
		counters->lat += amount;
	}
	if ((present & LON) != 0) {
		assert(add ? (counters->lon < maxCount):(counters->lon > 0));
		counters->lon += amount;
	}
	if ((present & ELV) != 0) {
		assert(add ? (counters->elv < maxCount):(counters->elv > 0));
		counters->elv += amount;
	}
	if ((present & SPEED) != 0) {
		assert(add ? (counters->speed < maxCount):(counters->speed > 0));
		counters->speed += amount;
	}
	if ((present & TRACK) != 0) {
		assert(add ? (counters->track < maxCount):(counters->track > 0));
		counters->track += amount;
	}
	if ((present & MTRACK) != 0) {
		assert(add ? (counters->mtrack < maxCount):(counters->mtrack > 0));
		counters->mtrack += amount;
	}
	if ((present & MAGVAR) != 0) {
		assert(add ? (counters->magvar < maxCount):(counters->magvar > 0));
		counters->magvar += amount;
	}
	if ((present & SATINUSECOUNT) != 0) {
		assert(add ? (counters->satinusecount < maxCount):(counters->satinusecount > 0));
		counters->satinusecount += amount;
	}
	if ((present & SATINUSE) != 0) {
		assert(add ? (counters->satinuse < maxCount):(counters->satinuse > 0));
		counters->satinuse += amount;
	}
	if ((present & SATINVIEW) != 0) {
		assert(add ? (counters->satinview < maxCount):(counters->satinview > 0));
		counters->satinview += amount;
	}

	/* smask */
	if ((smask & GPGGA) != 0) {
		assert(add ? (counters->gpgga < maxCount):(counters->gpgga > 0));
		counters->gpgga += amount;
	}
	if ((smask & GPGSA) != 0) {
		assert(add ? (counters->gpgsa < maxCount):(counters->gpgsa > 0));
		counters->gpgsa += amount;
	}
	if ((smask & GPGSV) != 0) {
		assert(add ? (counters->gpgsv < maxCount):(counters->gpgsv > 0));
		counters->gpgsv += amount;
	}
	if ((smask & GPRMC) != 0) {
		assert(add ? (counters->gprmc < maxCount):(counters->gprmc > 0));
		counters->gprmc += amount;
	}
	if ((smask & GPVTG) != 0) {
		assert(add ? (counters->gpvtg < maxCount):(counters->gpvtg > 0));
		counters->gpvtg += amount;
	}

	/* sig */
	if (nmea_INFO_is_present(present, SIG)) {
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
	if (nmea_INFO_is_present(present, FIX)) {
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
static void determineCumulativePresentSmaskSigFix(
		PositionAverageList * positionAverageList) {
	PositionUpdateEntry * cumulative =
			&positionAverageList->positionAverageCumulative;
	PositionUpdateCounters * counters = &positionAverageList->counters;
	unsigned long long count = positionAverageList->entriesCount;

	/* present */
	cumulative->nmeaInfo.present = 0;

	if (counters->smask >= count) {
		cumulative->nmeaInfo.present |= SMASK;
	}
	if (counters->utcdate >= count) {
		cumulative->nmeaInfo.present |= UTCDATE;
	}
	if (counters->utctime >= count) {
		cumulative->nmeaInfo.present |= UTCTIME;
	}
	if (counters->sig >= count) {
		cumulative->nmeaInfo.present |= SIG;
	}
	if (counters->fix >= count) {
		cumulative->nmeaInfo.present |= FIX;
	}
	if (counters->pdop >= count) {
		cumulative->nmeaInfo.present |= PDOP;
	}
	if (counters->hdop >= count) {
		cumulative->nmeaInfo.present |= HDOP;
	}
	if (counters->vdop >= count) {
		cumulative->nmeaInfo.present |= VDOP;
	}
	if (counters->lat >= count) {
		cumulative->nmeaInfo.present |= LAT;
	}
	if (counters->lon >= count) {
		cumulative->nmeaInfo.present |= LON;
	}
	if (counters->elv >= count) {
		cumulative->nmeaInfo.present |= ELV;
	}
	if (counters->speed >= count) {
		cumulative->nmeaInfo.present |= SPEED;
	}
	if (counters->track >= count) {
		cumulative->nmeaInfo.present |= TRACK;
	}
	if (counters->mtrack >= count) {
		cumulative->nmeaInfo.present |= MTRACK;
	}
	if (counters->magvar >= count) {
		cumulative->nmeaInfo.present |= MAGVAR;
	}
	if (counters->satinusecount >= count) {
		cumulative->nmeaInfo.present |= SATINUSECOUNT;
	}
	if (counters->satinuse >= count) {
		cumulative->nmeaInfo.present |= SATINUSE;
	}
	if (counters->satinview >= count) {
		cumulative->nmeaInfo.present |= SATINVIEW;
	}

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
	if (nmea_INFO_is_present(cumulative->nmeaInfo.present, SIG)) {
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
	if (nmea_INFO_is_present(cumulative->nmeaInfo.present, FIX)) {
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
 * Calculate angle components
 *
 * @param components a pointer to the components structure
 * @param angle a pointer to the angle (in degrees) from which to calculate
 * the components. Set to NULL when the angle is not present in the input data.
 *
 */
static void calculateAngleComponents(AngleComponents * components, double * angle) {
	if (!components)
		return;

	if (!angle) {
		components->x = 0.0;
		components->y = 0.0;
		return;
	}

	components->x = cos(nmea_degree2radian(*angle));
	components->y = sin(nmea_degree2radian(*angle));
}

/**
 * Calculate angle from its components
 *
 * @param components a pointer to the components structure
 * @return angle the angle (in degrees)
 */
static double calculateAngle(AngleComponents * components) {
	if (!components)
		return 0;

	return nmea_radian2degree(atan2(components->y, components->x));
}

/**
 * Add the src angle components to the dst angle components (accumulate)
 *
 * @param dst a pointer to the destination components structure
 * @param src a pointer to the source components structure
 * @param add true to add, false to subtract
 */
static void addAngleComponents(AngleComponents * dst, AngleComponents * src, bool add) {
	if (!dst || !src)
		return;

	dst->x += add ? src->x : -src->x;
	dst->y += add ? src->y : -src->y;
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

		/* do not touch present */
		/* do not touch smask */

		/* do not touch utc */

		/* do not touch sig */
		/* do not touch fix */

		/* do not touch satinfo */
	} else {
		assert(positionAverageList->entriesCount < positionAverageList->entriesMaxCount);
		assert(entry == getPositionAverageEntry(positionAverageList, INCOMING));

		/* present at the end */
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

	/* elv, speed */
	cumulative->nmeaInfo.elv += add ? entry->nmeaInfo.elv
			: -entry->nmeaInfo.elv;
	cumulative->nmeaInfo.speed += add ? entry->nmeaInfo.speed
			: -entry->nmeaInfo.speed;

	/* track, mtrack, magvar */
	cumulative->nmeaInfo.track += add ? entry->nmeaInfo.track : -entry->nmeaInfo.track;
	addAngleComponents(&cumulative->track, &entry->track, add);

	cumulative->nmeaInfo.mtrack += add ? entry->nmeaInfo.mtrack : -entry->nmeaInfo.mtrack;
	addAngleComponents(&cumulative->mtrack, &entry->mtrack, add);

	cumulative->nmeaInfo.magvar += add ? entry->nmeaInfo.magvar : -entry->nmeaInfo.magvar;
	addAngleComponents(&cumulative->magvar, &entry->magvar, add);

	/* adjust list count */
	positionAverageList->entriesCount += (add ? 1 : -1);

	updateCounters(positionAverageList, entry, add);
	determineCumulativePresentSmaskSigFix(positionAverageList);
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

		positionAverageList->positionAverage.nmeaInfo.track = calculateAngle(&positionAverageList->positionAverageCumulative.track);
		positionAverageList->positionAverage.nmeaInfo.mtrack = calculateAngle(&positionAverageList->positionAverageCumulative.mtrack);
		positionAverageList->positionAverage.nmeaInfo.magvar = calculateAngle(&positionAverageList->positionAverageCumulative.magvar);
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

	/* calculate the angle components */
	calculateAngleComponents(&newEntry->track, nmea_INFO_is_present(newEntry->nmeaInfo.present, TRACK) ? &newEntry->nmeaInfo.track : NULL);
	calculateAngleComponents(&newEntry->mtrack, nmea_INFO_is_present(newEntry->nmeaInfo.present, MTRACK) ? &newEntry->nmeaInfo.mtrack : NULL);
	calculateAngleComponents(&newEntry->magvar, nmea_INFO_is_present(newEntry->nmeaInfo.present, MAGVAR) ? &newEntry->nmeaInfo.magvar : NULL);

	/* now just add the new position */
	addOrRemoveEntryToFromCumulativeAverage(positionAverageList, newEntry, true);

	/* update the place where the new entry is stored */
	positionAverageList->newestEntryIndex
			= WRAPINDEX(positionAverageList, NEWESTINDEX(positionAverageList) + 1);

	/* update average position */
	updatePositionAverageFromCumulative(positionAverageList);
}
