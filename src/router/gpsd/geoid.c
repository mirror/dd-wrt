/* $Id: geoid.c 3666 2006-10-26 23:11:51Z ckuethe $ */
/* 
 * geoid.c -- ECEF to WGS84 conversions, including ellipsoid-to-MSL height
 *
 * Geoid separation code by Oleg Gusev, from data by Peter Dana.
 * ECEF conversion by Rob Janssen.
 */

#include <sys/types.h>
#include <math.h>
#include "gpsd_config.h"
#include "gpsd.h"

static double bilinear(double x1, double y1, double x2, double y2, double x, double y, double z11, double z12, double z21, double z22)
{
 double delta;

 if (y1 == y2 && x1 == x2 ) return (z11);
 if (y1 == y2 && x1 != x2 ) return (z22*(x-x1)+z11*(x2-x))/(x2-x1);
 if (x1 == x2 && y1 != y2 ) return (z22*(y-y1)+z11*(y2-y))/(y2-y1);

 delta=(y2-y1)*(x2-x1);

 return (z22*(y-y1)*(x-x1)+z12*(y2-y)*(x-x1)+z21*(y-y1)*(x2-x)+z11*(y2-y)*(x2-x))/delta;
}


/* return geoid separtion (MSL - WGS84) in meters, given a lat/lot in degrees */
/*@ +charint @*/
double wgs84_separation(double lat, double lon)
{
#define GEOID_ROW	19
#define GEOID_COL	37
    const char geoid_delta[GEOID_COL*GEOID_ROW]={
	/* 90S */ -30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30, -30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,
	/* 80S */ -53,-54,-55,-52,-48,-42,-38,-38,-29,-26,-26,-24,-23,-21,-19,-16,-12, -8, -4, -1,  1,  4,  4,  6,  5,  4,   2, -6,-15,-24,-33,-40,-48,-50,-53,-52,-53,
	/* 70S */ -61,-60,-61,-55,-49,-44,-38,-31,-25,-16, -6,  1,  4,  5,  4,  2,  6, 12, 16, 16, 17, 21, 20, 26, 26, 22,  16, 10, -1,-16,-29,-36,-46,-55,-54,-59,-61,
	/* 60S */ -45,-43,-37,-32,-30,-26,-23,-22,-16,-10, -2, 10, 20, 20, 21, 24, 22, 17, 16, 19, 25, 30, 35, 35, 33, 30,  27, 10, -2,-14,-23,-30,-33,-29,-35,-43,-45,
	/* 50S */ -15,-18,-18,-16,-17,-15,-10,-10, -8, -2,  6, 14, 13,  3,  3, 10, 20, 27, 25, 26, 34, 39, 45, 45, 38, 39,  28, 13, -1,-15,-22,-22,-18,-15,-14,-10,-15,
	/* 40S */  21,  6,  1, -7,-12,-12,-12,-10, -7, -1,  8, 23, 15, -2, -6,  6, 21, 24, 18, 26, 31, 33, 39, 41, 30, 24,  13, -2,-20,-32,-33,-27,-14, -2,  5, 20, 21,
	/* 30S */  46, 22,  5, -2, -8,-13,-10, -7, -4,  1,  9, 32, 16,  4, -8,  4, 12, 15, 22, 27, 34, 29, 14, 15, 15,  7,  -9,-25,-37,-39,-23,-14, 15, 33, 34, 45, 46,
	/* 20S */  51, 27, 10,  0, -9,-11, -5, -2, -3, -1,  9, 35, 20, -5, -6, -5,  0, 13, 17, 23, 21,  8, -9,-10,-11,-20, -40,-47,-45,-25,  5, 23, 45, 58, 57, 63, 51,
	/* 10S */  36, 22, 11,  6, -1, -8,-10, -8,-11, -9,  1, 32,  4,-18,-13, -9,  4, 14, 12, 13, -2,-14,-25,-32,-38,-60, -75,-63,-26,  0, 35, 52, 68, 76, 64, 52, 36,
	/* 00N */  22, 16, 17, 13,  1,-12,-23,-20,-14, -3, 14, 10,-15,-27,-18,  3, 12, 20, 18, 12,-13, -9,-28,-49,-62,-89,-102,-63, -9, 33, 58, 73, 74, 63, 50, 32, 22,
	/* 10N */  13, 12, 11,  2,-11,-28,-38,-29,-10,  3,  1,-11,-41,-42,-16,  3, 17, 33, 22, 23,  2, -3, -7,-36,-59,-90, -95,-63,-24, 12, 53, 60, 58, 46, 36, 26, 13,
	/* 20N */   5, 10,  7, -7,-23,-39,-47,-34, -9,-10,-20,-45,-48,-32, -9, 17, 25, 31, 31, 26, 15,  6,  1,-29,-44,-61, -67,-59,-36,-11, 21, 39, 49, 39, 22, 10,  5,
	/* 30N */  -7, -5, -8,-15,-28,-40,-42,-29,-22,-26,-32,-51,-40,-17, 17, 31, 34, 44, 36, 28, 29, 17, 12,-20,-15,-40, -33,-34,-34,-28,  7, 29, 43, 20,  4, -6, -7,
	/* 40N */ -12,-10,-13,-20,-31,-34,-21,-16,-26,-34,-33,-35,-26,  2, 33, 59, 52, 51, 52, 48, 35, 40, 33, -9,-28,-39, -48,-59,-50,-28,  3, 23, 37, 18, -1,-11,-12,
	/* 50N */  -8,  8,  8,  1,-11,-19,-16,-18,-22,-35,-40,-26,-12, 24, 45, 63, 62, 59, 47, 48, 42, 28, 12,-10,-19,-33, -43,-42,-43,-29, -2, 17, 23, 22,  6,  2, -8,
	/* 60N */   2,  9, 17, 10, 13,  1,-14,-30,-39,-46,-42,-21,  6, 29, 49, 65, 60, 57, 47, 41, 21, 18, 14,  7, -3,-22, -29,-32,-32,-26,-15, -2, 13, 17, 19,  6,  2,
	/* 70N */   2,  2,  1, -1, -3, -7,-14,-24,-27,-25,-19,  3, 24, 37, 47, 60, 61, 58, 51, 43, 29, 20, 12,  5, -2,-10, -14,-12,-10,-14,-12, -6, -2,  3,  6,  4,  2,
	/* 80N */   3,  1, -2, -3, -3, -3, -1,  3,  1,  5,  9, 11, 19, 27, 31, 34, 33, 34, 33, 34, 28, 23, 17, 13,  9,  4,   4,  1, -2, -2,  0,  2,  3,  2,  1,  1,  3,
	/* 90N */  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13}; 
/*@ -charint @*/
    int	ilat, ilon;
    int	ilat1, ilat2, ilon1, ilon2;

    ilat=(int)floor(( 90.+lat)/10);
    ilon=(int)floor((180.+lon)/10);
	
    ilat1=ilat;
    ilon1=ilon;
    ilat2=(ilat < GEOID_ROW-1)? ilat+1:ilat;
    ilon2=(ilon < GEOID_COL-1)? ilon+1:ilon;
	
    return bilinear(
	ilon1*10.-180.,ilat1*10.-90.,
	ilon2*10.-180.,ilat2*10.-90.,
	lon,           lat,
	(double)geoid_delta[ilon1+ilat1*GEOID_COL], 
	(double)geoid_delta[ilon2+ilat1*GEOID_COL],
	(double)geoid_delta[ilon1+ilat2*GEOID_COL], 
	(double)geoid_delta[ilon2+ilat2*GEOID_COL]
	);
}


void ecef_to_wgs84fix(struct gps_data_t *gpsdata,
			double x, double y, double z, 
			double vx, double vy, double vz)
/* fill in WGS84 position/velocity fields from ECEF coordinates */
{
    double lambda,phi,p,theta,n,h,vnorth,veast,heading;
    const double a = 6378137;			/* equatorial radius */
    const double f = 1 / 298.257223563;		/* flattening */
    const double b = a * (1 - f);		/* polar radius */
    const double e2 = (a*a - b*b) / (a*a);
    const double e_2 = (a*a - b*b) / (b*b);

    /* geodetic location */
    lambda = atan2(y,x);
    /*@ -evalorder @*/ 
    p = sqrt(pow(x,2) + pow(y,2)); 
    theta = atan2(z*a,p*b);
    phi = atan2(z + e_2*b*pow(sin(theta),3),p - e2*a*pow(cos(theta),3));
    n = a / sqrt(1.0 - e2*pow(sin(phi),2));
    h = p / cos(phi) - n;
    gpsdata->fix.latitude = phi * RAD_2_DEG;
    gpsdata->fix.longitude = lambda * RAD_2_DEG;
    gpsdata->separation = wgs84_separation(gpsdata->fix.latitude, gpsdata->fix.longitude);
    gpsdata->fix.altitude = h - gpsdata->separation;
    /* velocity computation */
    vnorth = -vx*sin(phi)*cos(lambda)-vy*sin(phi)*sin(lambda)+vz*cos(phi);
    veast = -vx*sin(lambda)+vy*cos(lambda);
    gpsdata->fix.climb = vx*cos(phi)*cos(lambda)+vy*cos(phi)*sin(lambda)+vz*sin(phi);
    gpsdata->fix.speed = sqrt(pow(vnorth,2) + pow(veast,2));
    heading = atan2(veast,vnorth);
    /*@ +evalorder @*/
    if (heading < 0)
	heading += 2 * PI;
    gpsdata->fix.track = heading * RAD_2_DEG;
}

#ifdef TESTMAIN
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    double	lat, lon;

    if (argc != 3) {
	fprintf(stderr,"Usage: %s lat lon\n",argv[0]);
	return 1;
    }	

    lat=atof(argv[1]);
    lon=atof(argv[2]);
	
    if (lat > 90. || lat < -90.)
    {
	fprintf(stderr," -90 <= lat=%s(%.f) <= 90 ?\n",argv[1],lat);
	return 1;
    }	

    if (lon > 180. || lat < -180.)
    {
	fprintf(stderr," -180 <= lon=%s(%.f) <= 180 ?\n",argv[2],lon);
	return 1;
    }
	
    printf(" lat= %f lon= %f geoid correction= %f\n",
	   lat, lon, wgs84_separation(lat, lon));

    return 0;
}
#endif /* TESTMAIN */
