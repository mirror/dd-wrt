/* $Id: gpsutils.c 4108 2006-12-08 11:48:31Z esr $ */
/* gpsutils.c -- code shared between low-level and high-level interfaces */
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "gpsd_config.h"
#include "gpsd.h"

#define MONTHSPERYEAR	12		/* months per calendar year */

void gps_clear_fix(/*@out@*/struct gps_fix_t *fixp)
/* stuff a fix structure with recognizable out-of-band values */
{
    fixp->time = NAN;
    fixp->mode = MODE_NOT_SEEN;
    fixp->latitude = fixp->longitude = NAN;
    fixp->track = NAN;
    fixp->speed = NAN;
    fixp->climb = NAN;
    fixp->altitude = NAN;
    fixp->ept = NAN;
    fixp->eph = NAN;
    fixp->epv = NAN;
    fixp->epd = NAN;
    fixp->eps = NAN;
    fixp->epc = NAN;
}

unsigned int gps_valid_fields(/*@in@*/struct gps_fix_t *fixp)
{
    unsigned int valid = 0;

    if (isnan(fixp->time) == 0)
	valid |= TIME_SET;
    if (fixp->mode != MODE_NOT_SEEN)
	valid |= MODE_SET;
    if (isnan(fixp->latitude) == 0 && isnan(fixp->longitude) == 0)
	valid |= LATLON_SET;
    if (isnan(fixp->altitude) == 0)
	valid |= ALTITUDE_SET;
    if (isnan(fixp->track) == 0)
	valid |= TRACK_SET;
    if (isnan(fixp->speed) == 0)
	valid |= SPEED_SET;
    if (isnan(fixp->climb) == 0)
	valid |= CLIMB_SET;
    if (isnan(fixp->ept) == 0)
	valid |= TIMERR_SET;
    if (isnan(fixp->eph) == 0)
	valid |= HERR_SET;
    if (isnan(fixp->epv) == 0)
	valid |= VERR_SET;
    if (isnan(fixp->epd) == 0)
	valid |= TRACKERR_SET;
    if (isnan(fixp->eps) == 0)
	valid |= SPEEDERR_SET;
    if (isnan(fixp->epc) == 0)
	valid |= CLIMBERR_SET;
    return valid;
}

char *gps_show_transfer(int transfer)
{
/*@ -statictrans @*/
    static char showbuf[100];
    showbuf[0] = '\0';
    if ((transfer & TIME_SET)!=0)
	(void)strlcat(showbuf, "time,", sizeof(showbuf));
    if ((transfer & LATLON_SET)!=0)
	(void)strlcat(showbuf, "latlon,", sizeof(showbuf));
    if ((transfer & MODE_SET)!=0)
	(void)strlcat(showbuf, "mode,", sizeof(showbuf));
    if ((transfer & ALTITUDE_SET)!=0)
	(void)strlcat(showbuf, "altitude,", sizeof(showbuf));
    if ((transfer & TRACK_SET)!=0)
	(void)strlcat(showbuf, "track,", sizeof(showbuf));
    if ((transfer & SPEED_SET)!=0)
	(void)strlcat(showbuf, "speed,", sizeof(showbuf));
    if ((transfer & CLIMB_SET)!=0)
	(void)strlcat(showbuf, "climb,", sizeof(showbuf));
    if ((transfer & TIMERR_SET)!=0)
	(void)strlcat(showbuf, "timerr,", sizeof(showbuf));
    if ((transfer & HERR_SET)!=0)
	(void)strlcat(showbuf, "herr,", sizeof(showbuf));
    if ((transfer & VERR_SET)!=0)
	(void)strlcat(showbuf, "verr,", sizeof(showbuf));
    if ((transfer & SPEEDERR_SET)!=0)
	(void)strlcat(showbuf, "speederr,", sizeof(showbuf));
    if ((transfer & CLIMBERR_SET)!=0)
	(void)strlcat(showbuf, "climberr,", sizeof(showbuf));
    if (strlen(showbuf)>0)
	showbuf[strlen(showbuf)-1] = '\0';
    return showbuf;
/*@ +statictrans @*/
}

void gps_merge_fix(/*@ out @*/struct gps_fix_t *to,
		   gps_mask_t transfer,
		   /*@ in @*/struct gps_fix_t *from)
/* merge new data into an old fix */
{
    if ((transfer & TIME_SET)!=0)
	to->time = from->time;
    if ((transfer & LATLON_SET)!=0) {
	to->latitude = from->latitude;
	to->longitude = from->longitude;
    }
    if ((transfer & MODE_SET)!=0)
	to->mode = from->mode;
    if ((transfer & ALTITUDE_SET)!=0)
	to->altitude = from->altitude;
    if ((transfer & TRACK_SET)!=0)
	to->track = from->track;
    if ((transfer & SPEED_SET)!=0)
	to->speed = from->speed;
    if ((transfer & CLIMB_SET)!=0)
	to->climb = from->climb;
    if ((transfer & TIMERR_SET)!=0)
	to->ept = from->ept;
    if ((transfer & HERR_SET)!=0)
	to->eph = from->eph;
    if ((transfer & VERR_SET)!=0)
	to->epv = from->epv;
    if ((transfer & SPEEDERR_SET)!=0)
	to->eps = from->eps;
    if ((transfer & CLIMBERR_SET)!=0)
	to->epc = from->epc;
}

double timestamp(void) 
{
    struct timeval tv; 
    (void)gettimeofday(&tv, NULL); 
    /*@i1@*/return(tv.tv_sec + tv.tv_usec*1e-6);
}

time_t mkgmtime(register struct tm *t)
/* struct tm to seconds since Unix epoch */
{
    register int year;
    register time_t result;
    static const int cumdays[MONTHSPERYEAR] =
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    /*@ +matchanyintegral @*/
    year = 1900 + t->tm_year + t->tm_mon / MONTHSPERYEAR;
    result = (year - 1970) * 365 + cumdays[t->tm_mon % MONTHSPERYEAR];
    result += (year - 1968) / 4;
    result -= (year - 1900) / 100;
    result += (year - 1600) / 400;
    result += t->tm_mday - 1;
    result *= 24;
    result += t->tm_hour;
    result *= 60;
    result += t->tm_min;
    result *= 60;
    result += t->tm_sec;
    /*@ -matchanyintegral @*/
    return (result);
}

double iso8601_to_unix(/*@in@*/char *isotime)
/* ISO8601 UTC to Unix UTC */
{
    char *dp = NULL;
    double usec;
    struct tm tm;

    /*@i1@*/dp = strptime(isotime, "%Y-%m-%dT%H:%M:%S", &tm);
    if (*dp == '.')
	usec = strtod(dp, NULL);
    else
	usec = 0;
    return (double)mkgmtime(&tm) + usec;
}

/*@observer@*/char *unix_to_iso8601(double fixtime, /*@ out @*/char isotime[], int len)
/* Unix UTC time to ISO8601, no timezone adjustment */
{
    struct tm when;
    double integral, fractional;
    time_t intfixtime;
    size_t slen;

    fractional = modf(fixtime, &integral);
    intfixtime = (time_t)integral;
    (void)gmtime_r(&intfixtime, &when);

    (void)strftime(isotime, 28, "%Y-%m-%dT%H:%M:%S", &when);
    slen = strlen(isotime);
    (void)snprintf(isotime + slen, (size_t)len, "%.1f", fractional);
    /*@ -aliasunique @*/
    (void)memcpy(isotime+slen, isotime+slen+1, strlen(isotime+slen+1));
    /*@ -aliasunique @*/
    (void)strlcat(isotime, "Z", 28);
    return isotime;
}

/*
 * The 'week' part of GPS dates are specified in weeks since 0000 on 06 
 * January 1980, with a rollover at 1024.  At time of writing the last 
 * rollover happened at 0000 22 August 1999.  Time-of-week is in seconds.
 *
 * This code copes with both conventional GPS weeks and the "extended"
 * 15-or-16-bit version with no wraparound that appears in Zodiac
 * chips and is supposed to appear in the Geodetic Navigation
 * Information (0x29) packet of SiRF chips.  Some SiRF firmware versions
 * (notably 231) actually ship the wrapped 10-bit week, despite what
 * the protocol reference claims.
 *
 * Note: This time will need to be corrected for leap seconds.
 */
#define GPS_EPOCH	315964800		/* GPS epoch in Unix time */
#define SECS_PER_WEEK	(60*60*24*7)		/* seconds per week */
#define GPS_ROLLOVER	(1024*SECS_PER_WEEK)	/* rollover period */

double gpstime_to_unix(int week, double tow)
{
    double fixtime;

    if (week >= 1024)
	fixtime = GPS_EPOCH + (week * SECS_PER_WEEK) + tow;
    else {
	time_t now, last_rollover;
	(void)time(&now);
	last_rollover = GPS_EPOCH+((now-GPS_EPOCH)/GPS_ROLLOVER)*GPS_ROLLOVER;
	/*@i@*/fixtime = last_rollover + (week * SECS_PER_WEEK) + tow;
    }
    return fixtime;
}

void unix_to_gpstime(double unixtime, /*@out@*/int *week, /*@out@*/double *tow)
{
    unixtime -= GPS_EPOCH;
    *week = (int)(unixtime / SECS_PER_WEEK);
    *tow = fmod(unixtime, SECS_PER_WEEK);
}

#define Deg2Rad(n)	((n) * DEG_2_RAD)

static double CalcRad(double lat)
/* earth's radius of curvature in meters at specified latitude.*/
{
    const double a = 6378.137;
    const double e2 = 0.081082 * 0.081082;
    // the radius of curvature of an ellipsoidal Earth in the plane of a
    // meridian of latitude is given by
    //
    // R' = a * (1 - e^2) / (1 - e^2 * (sin(lat))^2)^(3/2)
    //
    // where a is the equatorial radius,
    // b is the polar radius, and
    // e is the eccentricity of the ellipsoid = sqrt(1 - b^2/a^2)
    //
    // a = 6378 km (3963 mi) Equatorial radius (surface to center distance)
    // b = 6356.752 km (3950 mi) Polar radius (surface to center distance)
    // e = 0.081082 Eccentricity
    double sc = sin(Deg2Rad(lat));
    double x = a * (1.0 - e2);
    double z = 1.0 - e2 * sc * sc;
    double y = pow(z, 1.5);
    double r = x / y;

    return r * 1000.0;	// Convert to meters
}

double earth_distance(double lat1, double lon1, double lat2, double lon2)
/* distance in meters between two points specified in degrees. */
{
    double x1 = CalcRad(lat1) * cos(Deg2Rad(lon1)) * sin(Deg2Rad(90-lat1));
    double x2 = CalcRad(lat2) * cos(Deg2Rad(lon2)) * sin(Deg2Rad(90-lat2));
    double y1 = CalcRad(lat1) * sin(Deg2Rad(lon1)) * sin(Deg2Rad(90-lat1));
    double y2 = CalcRad(lat2) * sin(Deg2Rad(lon2)) * sin(Deg2Rad(90-lat2));
    double z1 = CalcRad(lat1) * cos(Deg2Rad(90-lat1));
    double z2 = CalcRad(lat2) * cos(Deg2Rad(90-lat2));
    double a = (x1*x2 + y1*y2 + z1*z2)/pow(CalcRad((lat1+lat2)/2),2);
    // a should be in [1, -1] but can sometimes fall outside it by
    // a very small amount due to rounding errors in the preceding
    // calculations.  This is prone to happen when the argument points
    // are very close together.  Return NaN so calculations trying
    // to use this will also blow up.
    if (fabs(a) > 1) 
	return NAN;
    else
	return CalcRad((lat1+lat2) / 2) * acos(a);
}

/*****************************************************************************

Carl Carter of SiRF supplied this algorithm for computing DOPs from 
a list of visible satellites...

For satellite n, let az(n) = azimuth angle from North and el(n) be elevation.
Let:

    a(k, 1) = sin az(k) * cos el(k)
    a(k, 2) = cos az(k) * cos el(k)
    a(k, 3) = sin el(k)

Then form the line-of-sight matrix A for satellites used in the solution:

    | a(1,1) a(1,2) a(1,3) 1 |
    | a(2,1) a(2,2) a(2,3) 1 |
    |   :       :      :   : |
    | a(n,1) a(n,2) a(n,3) 1 |

And its transpose A~:

    |a(1, 1) a(2, 1) .  .  .  a(n, 1) |
    |a(1, 2) a(2, 2) .  .  .  a(n, 2) |
    |a(1, 3) a(2, 3) .  .  .  a(n, 3) |
    |    1       1   .  .  .     1    |

Compute the covariance matrix (A~*A)^-1, which is guaranteed symmetric:

    | s(x)^2    s(x)*s(y)  s(x)*s(z)  s(x)*s(t) | 
    | s(x)*s(y) s(y)^2     s(y)*s(z)  s(y)*s(t) |
    | s(z)*s(t) s(y)*s(z)  s(z)^2     s(z)*s(t) |
    | s(x)*s(t) s(y)*s(t)  s(z)*s(t)  s(z)^2    |

Then:

GDOP = sqrt(s(x)^2 + s(y)^2 + s(z)^2 + s(t)^2)
TDOP = sqrt(s(t)^2)
PDOP = sqrt(s(x)^2 + s(y)^2 + s(z)^2)
HDOP = sqrt(s(x)^2 + s(y)^2)
VDOP = sqrt(s(y)^2)

Here's how we implement it...

First, each compute element P(i,j) of the 4x4 product A~*A.
If S(k=1,k=n): f(...) is the sum of f(...) as k varies from 1 to n, then
applying the definition of matrix product tells us: 

P(i,j) = S(k=1,k=n): B(i, k) * A(k, j)

But because B is the transpose of A, this reduces to 

P(i,j) = S(k=1,k=n): A(k, i) * A(k, j)

This is not, however, the entire algorithm that SiRF uses.  Carl writes:

> As you note, with rounding accounted for, most values agree exactly, and
> those that don't agree our number is higher.  That is because we
> deweight some satellites and account for that in the DOP calculation.
> If a satellite is not used in a solution at the same weight as others,
> it should not contribute to DOP calculation at the same weight.  So our
> internal algorithm does a compensation for that which you would have no
> way to duplicate on the outside since we don't output the weighting
> factors.  In fact those are not even available to API users.

Queried about the deweighting, Carl says:

> In the SiRF tracking engine, each satellite track is assigned a quality
> value based on the tracker's estimate of that signal.  It includes C/No
> estimate, ability to hold onto the phase, stability of the I vs. Q phase
> angle, etc.  The navigation algorithm then ranks all the tracks into
> quality order and selects which ones to use in the solution and what
> weight to give those used in the solution.  The process is actually a
> bit of a "trial and error" method -- we initially use all available
> tracks in the solution, then we sequentially remove the lowest quality
> ones until the solution stabilizes.  The weighting is inherent in the
> Kalman filter algorithm.  Once the solution is stable, the DOP is
> computed from those SVs used, and there is an algorithm that looks at
> the quality ratings and determines if we need to deweight any.
> Likewise, if we use altitude hold mode for a 3-SV solution, we deweight
> the phantom satellite at the center of the Earth.

So we cannot exactly duplicate what SiRF does internally.  We'll leave
HDOP alone and use our computed values for VDOP and PDOP.  Note, this
may have to change in the future if this code is used by a non-SiRF
driver.

******************************************************************************/

/*@ -fixedformalarray -mustdefine @*/
static int invert(double mat[4][4], /*@out@*/double inverse[4][4])
{
  // Find all NECESSARY 2x2 subdeterminants
  double Det2_12_01 = mat[1][0]*mat[2][1] - mat[1][1]*mat[2][0];
  double Det2_12_02 = mat[1][0]*mat[2][2] - mat[1][2]*mat[2][0];
  //double Det2_12_03 = mat[1][0]*mat[2][3] - mat[1][3]*mat[2][0];
  double Det2_12_12 = mat[1][1]*mat[2][2] - mat[1][2]*mat[2][1];
  //double Det2_12_13 = mat[1][1]*mat[2][3] - mat[1][3]*mat[2][1];
  //double Det2_12_23 = mat[1][2]*mat[2][3] - mat[1][3]*mat[2][2];
  double Det2_13_01 = mat[1][0]*mat[3][1] - mat[1][1]*mat[3][0];
  //double Det2_13_02 = mat[1][0]*mat[3][2] - mat[1][2]*mat[3][0];
  double Det2_13_03 = mat[1][0]*mat[3][3] - mat[1][3]*mat[3][0];
  //double Det2_13_12 = mat[1][1]*mat[3][2] - mat[1][2]*mat[3][1];  
  double Det2_13_13 = mat[1][1]*mat[3][3] - mat[1][3]*mat[3][1];
  //double Det2_13_23 = mat[1][2]*mat[3][3] - mat[1][3]*mat[3][2];  
  double Det2_23_01 = mat[2][0]*mat[3][1] - mat[2][1]*mat[3][0];
  double Det2_23_02 = mat[2][0]*mat[3][2] - mat[2][2]*mat[3][0];
  double Det2_23_03 = mat[2][0]*mat[3][3] - mat[2][3]*mat[3][0];
  double Det2_23_12 = mat[2][1]*mat[3][2] - mat[2][2]*mat[3][1];
  double Det2_23_13 = mat[2][1]*mat[3][3] - mat[2][3]*mat[3][1];
  double Det2_23_23 = mat[2][2]*mat[3][3] - mat[2][3]*mat[3][2];

  // Find all NECESSARY 3x3 subdeterminants
  double Det3_012_012 = mat[0][0]*Det2_12_12 - mat[0][1]*Det2_12_02 
  				+ mat[0][2]*Det2_12_01;
  //double Det3_012_013 = mat[0][0]*Det2_12_13 - mat[0][1]*Det2_12_03 
  //				+ mat[0][3]*Det2_12_01;
  //double Det3_012_023 = mat[0][0]*Det2_12_23 - mat[0][2]*Det2_12_03
  //				+ mat[0][3]*Det2_12_02;
  //double Det3_012_123 = mat[0][1]*Det2_12_23 - mat[0][2]*Det2_12_13 
  //				+ mat[0][3]*Det2_12_12;
  //double Det3_013_012 = mat[0][0]*Det2_13_12 - mat[0][1]*Det2_13_02 
  //				+ mat[0][2]*Det2_13_01;
  double Det3_013_013 = mat[0][0]*Det2_13_13 - mat[0][1]*Det2_13_03
				+ mat[0][3]*Det2_13_01;
  //double Det3_013_023 = mat[0][0]*Det2_13_23 - mat[0][2]*Det2_13_03
  //				+ mat[0][3]*Det2_13_02;
  //double Det3_013_123 = mat[0][1]*Det2_13_23 - mat[0][2]*Det2_13_13
  //				+ mat[0][3]*Det2_13_12;
  //double Det3_023_012 = mat[0][0]*Det2_23_12 - mat[0][1]*Det2_23_02 
  //				+ mat[0][2]*Det2_23_01;
  //double Det3_023_013 = mat[0][0]*Det2_23_13 - mat[0][1]*Det2_23_03
  //				+ mat[0][3]*Det2_23_01;
  double Det3_023_023 = mat[0][0]*Det2_23_23 - mat[0][2]*Det2_23_03
				+ mat[0][3]*Det2_23_02;
  //double Det3_023_123 = mat[0][1]*Det2_23_23 - mat[0][2]*Det2_23_13
  //				+ mat[0][3]*Det2_23_12;
  double Det3_123_012 = mat[1][0]*Det2_23_12 - mat[1][1]*Det2_23_02 
				+ mat[1][2]*Det2_23_01;
  double Det3_123_013 = mat[1][0]*Det2_23_13 - mat[1][1]*Det2_23_03 
				+ mat[1][3]*Det2_23_01;
  double Det3_123_023 = mat[1][0]*Det2_23_23 - mat[1][2]*Det2_23_03 
				+ mat[1][3]*Det2_23_02;
  double Det3_123_123 = mat[1][1]*Det2_23_23 - mat[1][2]*Det2_23_13 
				+ mat[1][3]*Det2_23_12;

  // Find the 4x4 determinant
  static double det;
          det =   mat[0][0]*Det3_123_123 
		- mat[0][1]*Det3_123_023 
		+ mat[0][2]*Det3_123_013 
		- mat[0][3]*Det3_123_012;

  // Very small determinants probably reflect floating-point fuzz near zero
  if (fabs(det) == 0.0)
      return 0;

  inverse[0][0] =  Det3_123_123 / det;
  //inverse[0][1] = -Det3_023_123 / det;
  //inverse[0][2] =  Det3_013_123 / det;
  //inverse[0][3] = -Det3_012_123 / det;

  //inverse[1][0] = -Det3_123_023 / det;
  inverse[1][1] =  Det3_023_023 / det;
  //inverse[1][2] = -Det3_013_023 / det;
  //inverse[1][3] =  Det3_012_023 / det;

  //inverse[2][0] =  Det3_123_013 / det;
  //inverse[2][1] = -Det3_023_013 / det;
  inverse[2][2] =  Det3_013_013 / det;
  //inverse[2][3] = -Det3_012_013 / det;

  //inverse[3][0] = -Det3_123_012 / det;
  //inverse[3][1] =  Det3_023_012 / det;
  //inverse[3][2] = -Det3_013_012 / det;
  inverse[3][3] =  Det3_012_012 / det;

  return 1;
}  
/*@ +fixedformalarray +mustdefine @*/

gps_mask_t dop(struct gps_data_t *gpsdata)
{
    double prod[4][4];
    double inv[4][4];
    double satpos[MAXCHANNELS][4];
    int i, j, k, n;

#ifdef __UNUSED__
    gpsd_report(LOG_INF, "Satellite picture:\n");
    for (k = 0; k < MAXCHANNELS; k++) {
	if (gpsdata->used[k])
	    gpsd_report(LOG_INF, "az: %d el: %d  SV: %d\n",
			gpsdata->azimuth[k], gpsdata->elevation[k], gpsdata->used[k]);
    }
#endif /* __UNUSED__ */

    for (n = k = 0; k < gpsdata->satellites_used; k++) {
	if (gpsdata->used[k] == 0)
	    continue;
	satpos[n][0] = sin(gpsdata->azimuth[k]*DEG_2_RAD)
	    * cos(gpsdata->elevation[k]*DEG_2_RAD);
	satpos[n][1] = cos(gpsdata->azimuth[k]*DEG_2_RAD)
	    * cos(gpsdata->elevation[k]*DEG_2_RAD);
	satpos[n][2] = sin(gpsdata->elevation[k]*DEG_2_RAD);
	satpos[n][3] = 1;
	n++;
    }

#ifdef __UNUSED__
    gpsd_report(LOG_INF, "Line-of-sight matrix:\n");
    for (k = 0; k < n; k++) {
	gpsd_report(LOG_INF, "%f %f %f %f\n",
		    satpos[k][0], satpos[k][1], satpos[k][2], satpos[k][3]);
    }
#endif /* __UNUSED__ */

    for (i = 0; i < 4; ++i) { //< rows
        for (j = 0; j < 4; ++j) { //< cols
            prod[i][j] = 0.0;
            for (k = 0; k < n; ++k) {
                prod[i][j] += satpos[k][i] * satpos[k][j];
            }
        }
    }

#ifdef __UNUSED__
    gpsd_report(LOG_INF, "product:\n");
    for (k = 0; k < 4; k++) {
	gpsd_report(LOG_INF, "%f %f %f %f\n",
		    prod[k][0], prod[k][1], prod[k][2], prod[k][3]);
    }
#endif /* __UNUSED__ */

    if (invert(prod, inv)) {
#ifdef __UNUSED__
	/*
	 * Note: this will print garbage unless all the subdeterminants
	 * are computed in the invert() function.
	 */
	gpsd_report(LOG_RAW, "inverse:\n");
	for (k = 0; k < 4; k++) {
	    gpsd_report(LOG_RAW, "%f %f %f %f\n",
			inv[k][0], inv[k][1], inv[k][2], inv[k][3]);
	}
	gpsd_report(LOG_INF, "HDOP: reported = %f, computed = %f\n",
		    gpsdata->hdop, sqrt(inv[0][0] + inv[1][1]));
#endif /* __UNUSED__ */
    } else {
	gpsd_report(LOG_WARN, "LOS matrix is singular, can't calculate DOPs.\n");
	return 0;
    }

    /*@ -usedef @*/
    //gpsdata->hdop = sqrt(inv[0][0] + inv[1][1]);
    gpsdata->vdop = sqrt(inv[1][1]);
    gpsdata->pdop = sqrt(inv[0][0] + inv[1][1] + inv[2][2]);
    gpsdata->tdop = sqrt(inv[3][3]);
    gpsdata->gdop = sqrt(inv[0][0] + inv[1][1] + inv[2][2] + inv[3][3]);
    /*@ +usedef @*/

    return VDOP_SET | PDOP_SET | TDOP_SET | GDOP_SET;
}
