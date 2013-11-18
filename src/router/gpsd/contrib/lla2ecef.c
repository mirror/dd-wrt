/* This file is Copyright (c)2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

extern char *__progname;
double A = 6378137.0, B = 6356752.3142;

static double rad2deg(double r){
	return (180.0 * r / M_PI);
}

static double deg2rad(double r){
	return (M_PI * r / 180.0);
}

static void lla2ecef(double *lla, double *ecef){
	double N;

	lla[0] = deg2rad(lla[0]);
	lla[1] = deg2rad(lla[1]);

	N = pow(A,2) / sqrt(
		pow(A,2) * pow(cos(lla[0]),2) +
		pow(B,2) * pow(sin(lla[0]),2)
		);

	ecef[0] = (N + lla[2]) * cos(lla[0]) * cos(lla[1]);
	ecef[1] = (N + lla[2]) * cos(lla[0]) * sin(lla[1]);
	ecef[2] = (pow(B,2) / pow(A,2) * N + lla[2]) * sin(lla[0]);

	return;
}

static void ecef2lla(double *ecef, double *lla){
	double E, F, N, P, T;

	E = (pow(A,2) - pow(B,2)) / pow(A,2);
	F = (pow(A,2) - pow(B,2)) / pow(B,2);
	P = sqrt(pow(ecef[0], 2) + pow(ecef[1],2));

	T = atan((ecef[2] * A)/(P * B));

	lla[0] = atan(
		(ecef[2] + F * B * pow(sin(T), 3) ) /
		(P - E * A * pow(cos(T), 3) )
		);

	lla[1] = atan( ecef[1] / ecef[0] );

	N = pow(A,2) / sqrt(
		pow(A,2) * pow(cos(lla[0]),2) +
		pow(B,2) * pow(sin(lla[0]),2)
		);

	lla[2] = P / cos(lla[0]) - N;
	lla[1] = rad2deg(lla[1])-180;
	lla[0] = rad2deg(lla[0]);
	return;
}

int
main(int argc, char **argv){
	double ecef[3], lla[3];

	if (4 != argc){
		printf("usage:\tecef2lla X Y Z\n\tlla2ecef lat lon alt\n");
		return 1;
	}

	if (0 == strcmp(__progname, "lla2ecef")){
		/* Edmonton:
		lla[0] = 53.52716;
		lla[1] = -113.53013;
		lla[2] = 707.0;
		*/

		lla[0] = atof(argv[1]);
		lla[1] = atof(argv[2]);
		lla[2] = atof(argv[3]);

		lla2ecef(lla, ecef);
		printf("%.2lf %.2lf %.2lf\n", ecef[0], ecef[1], ecef[2]);
	}

	if (0 == strcmp(__progname, "ecef2lla")){
		/* Edmonton:
		ecef[0] = -1517110.0;
		ecef[1] = -3484096.0;
		ecef[2] =  5106188.0;
		*/

		ecef[0] = atof(argv[1]);
		ecef[1] = atof(argv[2]);
		ecef[2] = atof(argv[3]);

		ecef2lla(ecef, lla);
		printf("%.7lf %.7lf %.2lf\n", lla[0], lla[1], lla[2]);
	}
	return 0;
}
