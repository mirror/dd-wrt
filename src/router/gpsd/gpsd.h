/* $Id: gpsd.h 4064 2006-12-04 05:21:11Z esr $ */
#ifndef _gpsd_h_
#define _gpsd_h_

/* gpsd.h -- fundamental types and structures for the gpsd library */

#include <stdbool.h>
#include <stdio.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_SYS_TERMIOS_H
#include <sys/termios.h>
#endif
#include "gps.h"

/* Some internal capabilities depend on which drivers we're compiling. */
#ifdef EARTHMATE_ENABLE
#define ZODIAC_ENABLE	
#endif
#if defined(ZODIAC_ENABLE) || defined(SIRF_ENABLE) || defined(GARMIN_ENABLE) || defined(TSIP_ENABLE) || defined(EVERMORE_ENABLE) || defined(ITALK_ENABLE)
#define BINARY_ENABLE	
#endif
#if defined(TRIPMATE_ENABLE) || defined(BINARY_ENABLE)
#define NON_NMEA_ENABLE	
#endif

/* First, declarations for the packet layer... */

#define NMEA_MAX	82		/* max length of NMEA sentence */
#define NMEA_BIG_BUF	(2*NMEA_MAX+1)	/* longer than longest NMEA sentence */

/* a few bits of ISGPS magic */
enum isgpsstat_t {
    ISGPS_NO_SYNC, ISGPS_SYNC, ISGPS_SKIP, ISGPS_MESSAGE,
};
#define ISGPS_ERRLEVEL_BASE	5

#define RTCM_MAX	(RTCM_WORDS_MAX * sizeof(isgps30bits_t))

/*
 * The packet buffers need to be as long than the longest packet we
 * expect to see in any protocol, because we have to be able to hold
 * an entire packet for checksumming.  Thus, in particular, they need
 * to be as long as a SiRF MID 4 packet, 188 bytes payload plus eight bytes 
 * of header/length/checksum/trailer. 
 */
#define MAX_PACKET_LENGTH	196	/* 188 + 8 */

/*
 * We used to define the input buffer length as MAX_PACKET_LENGTH*2+1.
 * However, as it turns out, this isn't enough.  We've had a report
 * from one user with a GPS that reports at 20Hz that "sometimes a
 * long/slow context switch will cause the next read from the serial
 * device to be very big. I sometimes see a read of 250 characters or
 * more."
 */
#define INPUT_BUFFER_LENGTH	1024

struct gps_packet_t {
    /* packet-getter internals */
    int	type;
#define BAD_PACKET	-1
#define COMMENT_PACKET	0
#define NMEA_PACKET	1
#define SIRF_PACKET	2
#define ZODIAC_PACKET	3
#define TSIP_PACKET	4
#define EVERMORE_PACKET	5
#define ITALK_PACKET	6
#define RTCM_PACKET	7
#define GARMIN_PACKET	8
    unsigned int state;
    size_t length;
    unsigned char inbuffer[MAX_PACKET_LENGTH*2+1];
    size_t inbuflen;
    unsigned /*@observer@*/char *inbufptr;
    /* outbuffer needs to be able to hold 4 GPGSV records at once */
    unsigned char outbuffer[MAX_PACKET_LENGTH*2+1];
    size_t outbuflen;
    unsigned long char_counter;		/* count characters processed */
    unsigned long retry_counter;	/* count sniff retries */
    unsigned counter;			/* packets since last driver switch */
    /*
     * This is not conditionalized on RTCM104_ENABLE because we need to
     * be able to build rtcmdecode even when RTCM support is not
     * configured in the daemon.
     */
    struct {
	/* ISGPS200 decoding */
	bool            locked;
	int             curr_offset;
	isgps30bits_t   curr_word;
	isgps30bits_t   buf[RTCM_WORDS_MAX];
	unsigned int    bufindex;
    } isgps;
};

extern void packet_reset(struct gps_packet_t *);
extern void packet_pushback(struct gps_packet_t *);
extern ssize_t packet_parse(struct gps_packet_t *, size_t);
extern ssize_t packet_get(int, struct gps_packet_t *);
extern int packet_sniff(struct gps_packet_t *);

extern void isgps_init(/*@out@*/struct gps_packet_t *);
enum isgpsstat_t isgps_decode(struct gps_packet_t *, 
			      bool (*preamble_match)(isgps30bits_t *),
			      bool (*length_check)(struct gps_packet_t *),
			      size_t,
			      unsigned int);
extern unsigned int isgps_parity(isgps30bits_t);
extern void isgps_output_magnavox(isgps30bits_t *, unsigned int, FILE *);

extern enum isgpsstat_t rtcm_decode(struct gps_packet_t *, unsigned int);
extern void rtcm_dump(struct rtcm_t *, /*@out@*/char[], size_t);
extern int rtcm_undump(/*@out@*/struct rtcm_t *, char *);
extern void rtcm_unpack(/*@out@*/struct rtcm_t *, char *);
extern bool rtcm_repack(struct rtcm_t *, isgps30bits_t *);
extern void rtcm_output_magnavox(isgps30bits_t *, FILE *);

/* Next, declarations for the core library... */

/* factors for converting among confidence interval units */
#define CEP50_SIGMA	1.18
#define DRMS_SIGMA	1.414
#define CEP95_SIGMA	2.45

/* this is where we choose the confidence level to use in reports */
#define GPSD_CONFIDENCE	CEP95_SIGMA

/* several places in the code try setuid. put our preferred username here */
#define GPSD_USER "nobody"

#define NTPSHMSEGS	4		/* number of NTP SHM segments */

/* Some internal capabilities depend on which drivers we're compiling. */
#ifdef EARTHMATE_ENABLE
#define ZODIAC_ENABLE	
#endif
#if defined(ZODIAC_ENABLE) || defined(SIRF_ENABLE) || defined(GARMIN_ENABLE) || defined(TSIP_ENABLE) || defined(EVERMORE_ENABLE) || defined(ITALK_ENABLE)
#define BINARY_ENABLE	
#endif
#if defined(TRIPMATE_ENABLE) || defined(BINARY_ENABLE)
#define NON_NMEA_ENABLE	
#endif

struct gps_context_t {
   int valid;				/* member validity flags */
#define LEAP_SECOND_VALID	0x01	/* we have or don't need correction */
    /* DGPSIP status */
    bool sentdgps;			/* have we sent a DGPS report? */
    enum { dgnss_none, dgnss_dgpsip, dgnss_ntrip } dgnss_service;	/* type of DGNSS service */
    int fixcnt;				/* count of good fixes seen */
    int dsock;			        /* socket to DGPSIP server/Ntrip caster */
    void *dgnss_privdata;		/* DGNSS service specific data */
    ssize_t rtcmbytes;			/* byte count of last RTCM104 report */
    char rtcmbuf[RTCM_MAX];		/* last RTCM104 report */
    double rtcmtime;			/* timestamp of last RTCM104 report */ 
    /* timekeeping */
    int leap_seconds;			/* Unix seconds to UTC */
    int century;			/* for NMEA-only devices without ZDA */
#ifdef NTPSHM_ENABLE
    bool enable_ntpshm;
    /*@reldef@*/struct shmTime *shmTime[NTPSHMSEGS];
    bool shmTimeInuse[NTPSHMSEGS];
# ifdef PPS_ENABLE
    bool shmTimePPS;
# endif /* PPS_ENABLE */
#endif /* NTPSHM_ENABLE */
};

struct gps_device_t;

#if defined (HAVE_SYS_TERMIOS_H)
#include <sys/termios.h>
#else
#if defined (HAVE_TERMIOS_H)
#include <termios.h>
#endif
#endif

struct gps_type_t {
/* GPS method table, describes how to talk to a particular GPS type */
    /*@observer@*/char *typename;
    /*@observer@*//*@null@*/char *trigger;
    int channels;
    /*@null@*/bool (*probe_detect)(struct gps_device_t *session);
    /*@null@*/void (*probe_wakeup)(struct gps_device_t *session);
    /*@null@*/void (*probe_subtype)(struct gps_device_t *session, unsigned int seq);
#ifdef ALLOW_RECONFIGURE 
    /*@null@*/void (*configurator)(struct gps_device_t *session, unsigned int seq);
#endif /* ALLOW_RECONFIGURE */
    /*@null@*/ssize_t (*get_packet)(struct gps_device_t *session);
    /*@null@*/gps_mask_t (*parse_packet)(struct gps_device_t *session);
    /*@null@*/ssize_t (*rtcm_writer)(struct gps_device_t *session, char *rtcmbuf, size_t rtcmbytes);
    /*@null@*/bool (*speed_switcher)(struct gps_device_t *session, speed_t speed);
    /*@null@*/void (*mode_switcher)(struct gps_device_t *session, int mode);
    /*@null@*/bool (*rate_switcher)(struct gps_device_t *session, double rate);
    int cycle_chars;
#ifdef ALLOW_RECONFIGURE 
    /*@null@*/void (*revert)(struct gps_device_t *session);
#endif /* ALLOW_RECONFIGURE */
    /*@null@*/void (*wrapup)(struct gps_device_t *session);
    double cycle;
};

struct gps_device_t {
/* session object, encapsulates all global state */
    struct gps_data_t gpsdata;
    /*@relnull@*/struct gps_type_t *device_type;
    struct gps_context_t	*context;
#ifdef ALLOW_RECONFIGURE
    bool enable_reconfigure;		/* OK to hack GPS settings? */ 
#endif /* ALLOW_RECONFIGURE */
    double rtcmtime;	/* timestamp of last RTCM104 correction to GPS */
    struct termios ttyset, ttyset_old;
    unsigned int baudindex;
    int saved_baud;
    struct gps_packet_t packet;
    char subtype[64];			/* firmware version or subtype ID */
    double poll_times[FD_SETSIZE];	/* last daemon poll time */
#ifdef NTPSHM_ENABLE
    int shmindex;
    double last_fixtime;		/* so updates happen once */
# ifdef PPS_ENABLE
    int shmTimeP;
# endif /* PPS_ENABLE */
#endif /* NTPSHM_ENABLE */
    double mag_var;		/* Magnetic variation in degrees */  
    bool back_to_nmea;		/* back to NMEA on revert? */
    /*
     * The rest of this structure is driver-specific private storage.
     * Because the Garmin driver uses a long buffer, you can have
     * up to 4096+12 bytes of private storage in your own union member
     * without making this structure larger or changing the API at all.
     */
    union {
#ifdef NMEA_ENABLE
	struct {
	    int part, await;		/* for tracking GSV parts */
	    struct tm date;
	    double subseconds;
	} nmea;
#endif /* NMEA_ENABLE */
#ifdef BINARY_ENABLE
#ifdef SIRF_ENABLE
	struct {
	    unsigned int driverstate;	/* for private use */
#define SIRF_LT_231	0x01		/* SiRF at firmware rev < 231 */
#define SIRF_EQ_231     0x02            /* SiRF at firmware rev == 231 */
#define SIRF_GE_232     0x04            /* SiRF at firmware rev >= 232 */
#define UBLOX   	0x08		/* uBlox firmware with packet 0x62 */
	    unsigned long satcounter;
	    unsigned int time_seen;
#define TIME_SEEN_GPS_1	0x01	/* Seen GPS time variant 1? */
#define TIME_SEEN_GPS_2	0x02	/* Seen GPS time variant 2? */
#define TIME_SEEN_UTC_1	0x04	/* Seen UTC time variant 1? */
#define TIME_SEEN_UTC_2	0x08	/* Seen UTC time variant 2? */
#ifdef ALLOW_RECONFIGURE
	    /* fields from Navigation Parameters message */
	    bool nav_parameters_seen;	/* have we seen one? */
	    unsigned char altitude_hold_mode;
	    unsigned char altitude_hold_source;
	    int16_t altitude_source_input;
	    unsigned char degraded_mode;
	    unsigned char degraded_timeout;
	    unsigned char dr_timeout;
	    unsigned char track_smooth_mode;
#endif /* ALLOW_RECONFIGURE */
	} sirf;
#endif /* SIRF_ENABLE */
#ifdef TSIP_ENABLE
	struct {
	    int16_t gps_week;		/* Current GPS week number */
	    bool superpkt;		/* Super Packet mode requested */
	    time_t last_41;		/* Timestamps for packet requests */
	    time_t last_5c;
	    time_t last_6d;
	    time_t last_46;
	    unsigned int parity, stopbits; /* saved RS232 link parameters */
	} tsip;
#endif /* TSIP_ENABLE */
#ifdef GARMIN_ENABLE	/* private housekeeping stuff for the Garmin driver */
	struct {
	    unsigned char Buffer[4096+12];	/* Garmin packet buffer */
	    size_t BufferLen;		/* current GarminBuffer Length */
	} garmin;
#endif /* GARMIN_ENABLE */
#ifdef ZODIAC_ENABLE	/* private housekeeping stuff for the Zodiac driver */
	struct {
	    unsigned short sn;		/* packet sequence number */
	    /*
	     * Zodiac chipset channel status from PRWIZCH. Keep it so
	     * raw-mode translation of Zodiac binary protocol can send
	     * it up to the client.
	     */
#define ZODIAC_CHANNELS	12
	    unsigned int Zs[ZODIAC_CHANNELS];	/* satellite PRNs */
	    unsigned int Zv[ZODIAC_CHANNELS];	/* signal values (0-7) */
	} zodiac;
#endif /* ZODIAC_ENABLE */
#endif /* BINARY_ENABLE */
    } driver;
};

/* logging levels */
#define LOG_ERROR 	0	/* errors, display always */
#define LOG_SHOUT	0	/* not an error but we should always see it */
#define LOG_WARN	1	/* not errors but may indicate a problem */
#define LOG_INF 	2	/* key informative messages */
#define LOG_PROG	3	/* progress messages */
#define LOG_IO  	4	/* IO to and from devices */
#define LOG_RAW 	5	/* raw low-level I/O */

#define IS_HIGHEST_BIT(v,m)	(v & ~((m<<1)-1))==0

/* here are the available GPS drivers */
extern struct gps_type_t **gpsd_drivers;

/* gpsd library internal prototypes */
extern gps_mask_t nmea_parse_input(struct gps_device_t *);
extern gps_mask_t nmea_parse(char *, struct gps_device_t *);
extern int nmea_send(int, const char *, ... );
extern void nmea_add_checksum(char *);

ssize_t generic_get(struct gps_device_t *);
ssize_t pass_rtcm(struct gps_device_t *, char *, size_t);

extern gps_mask_t sirf_parse(struct gps_device_t *, unsigned char *, size_t);
extern gps_mask_t evermore_parse(struct gps_device_t *, unsigned char *, size_t);
extern gps_mask_t garmin_ser_parse(struct gps_device_t *);

extern bool dgnss_url(char *);
extern int dgnss_open(struct gps_context_t *, char *);
extern int dgnss_poll(struct gps_context_t *);
extern void dgnss_report(struct gps_device_t *);
extern void dgnss_autoconnect(struct gps_context_t *, double, double);

extern void rtcm_relay(struct gps_device_t *);
extern void rtcm_output_mag(isgps30bits_t *, FILE *);

extern int dgpsip_open(struct gps_context_t *, const char *);
extern void dgpsip_report(struct gps_device_t *);
extern void dgpsip_autoconnect(struct gps_context_t *, 
			       double, double, const char *);
extern int ntrip_open(struct gps_context_t *, char *);
extern void ntrip_report(struct gps_device_t *);

extern void gpsd_tty_init(struct gps_device_t *);
extern int gpsd_open(struct gps_device_t *);
extern bool gpsd_set_raw(struct gps_device_t *);
extern bool gpsd_write(struct gps_device_t *, void const *, size_t);
extern bool gpsd_next_hunt_setting(struct gps_device_t *);
extern int gpsd_switch_driver(struct gps_device_t *, char *);
extern void gpsd_set_speed(struct gps_device_t *, speed_t, unsigned char, unsigned int);
extern speed_t gpsd_get_speed(struct termios *);
extern void gpsd_assert_sync(struct gps_device_t *);
extern void gpsd_close(struct gps_device_t *);

extern void gpsd_zero_satellites(/*@out@*/struct gps_data_t *sp)/*@modifies sp@*/;
extern void gpsd_interpret_subframe(struct gps_device_t *, unsigned int[]);
extern /*@ observer @*/ char *gpsd_hexdump(const void *, size_t);
extern int gpsd_hexpack(char *, char *, int);
extern int hex2bin(char *);
extern char /*@observer@*/ *gpsd_id(/*@in@*/struct gps_device_t *);
extern void gpsd_position_fix_dump(struct gps_device_t *, /*@out@*/char[], size_t);
extern void gpsd_error_model(struct gps_device_t *, 
			     struct gps_fix_t *, struct gps_fix_t *);
extern void gpsd_clear_data(struct gps_device_t *);
extern int netlib_connectsock(const char *, const char *, const char *);

extern void ntpshm_init(struct gps_context_t *, bool);
extern int ntpshm_alloc(struct gps_context_t *);
extern bool ntpshm_free(struct gps_context_t *, int);
extern int ntpshm_put(struct gps_device_t *, double);
extern int ntpshm_pps(struct gps_device_t *,struct timeval *);

extern void ecef_to_wgs84fix(struct gps_data_t *,
			     double, double, double, 
			     double, double, double);
extern gps_mask_t dop(struct gps_data_t *);

/* srecord.c */
extern void hexdump(size_t, unsigned char *, unsigned char *);
extern unsigned char sr_sum(unsigned int, unsigned int, unsigned char *);
extern int bin2srec(unsigned int, unsigned int, unsigned int, unsigned char *, unsigned char *);
extern int srec_hdr(unsigned int, unsigned char *, unsigned char *);
extern int srec_fin(unsigned int, unsigned char *);
extern unsigned char hc(unsigned char);

/* exported bits for the GPS flasher */
bool sirf_write(int fd, unsigned char *msg);

/* application interface */
extern void gpsd_init(struct gps_device_t *, struct gps_context_t *, char *);
extern int gpsd_activate(struct gps_device_t *, bool);
extern void gpsd_deactivate(struct gps_device_t *);
extern gps_mask_t gpsd_poll(struct gps_device_t *);
extern void gpsd_wrap(struct gps_device_t *);

/* caller should supply this */
void gpsd_report(int, const char *, ...);

#ifdef S_SPLINT_S
extern bool finite(double);
extern int cfmakeraw(struct termios *);
extern struct protoent *getprotobyname(const char *);
extern /*@observer@*/char *strptime(const char *,const char *tp,/*@out@*/struct tm *)/*@modifies tp@*/;
extern struct tm *gmtime_r(const time_t *,/*@out@*/struct tm *tp)/*@modifies tp@*/;
extern struct tm *localtime_r(const time_t *,/*@out@*/struct tm *tp)/*@modifies tp@*/;
extern float roundf(float x);
#endif /* S_SPLINT_S */

/* some OSes don't have round(). fake it if need be */
#ifndef HAVE_ROUND
#define	round(x) ((double)rint(x))
#define roundf(x) ((float)rintf(x))
#endif /* !HAVE_ROUND */

/* OpenBSD and FreeBSD and Cygwin don't seem to have NAN, NetBSD does, others? */
/* XXX test for this in configure? */
#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__CYGWIN__)
#ifndef NAN
#define NAN (0.0/0.0)
#endif /* !NAN */
#endif /* list of Operating Systems */

/* Cygwin, in addition to NAN, doesn't have cfmakeraw */
#if defined(__CYGWIN__)
void cfmakeraw(struct termios *);
#endif /* defined(__CYGWIN__) */

#endif /* _gpsd_h_ */
