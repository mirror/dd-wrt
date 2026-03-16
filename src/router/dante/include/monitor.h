/*
 * Copyright (c) 2014
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/* $Id: monitor.h,v 1.5.2.5 2014/08/24 11:41:35 karls Exp $ */

#ifndef _MONITOR_H_
#define _MONITOR_H_

/*
 * Variables related to MTU-testing.
 */

#define TEST_MTU_TCP_KEEPIDLE  (10)    /* value to set TCP_KEEPIDLE to.       */
#define TEST_MTU_ACKWAIT       (10)    /* how long to wait for keepalive ack. */
typedef enum { TEST_MTU = 1 } monitor_test_type_t;

typedef struct {
   int              s;
   iocount_t        send;
   iocount_t        recv;
} monitor_test_data_t;

typedef struct {
   monitor_test_data_t    internal;
   monitor_test_data_t    external;

   /*
    * Tests to do, and their current state if so.
    */
   struct {
      networktest_t         internal;
      networktest_t         external;
   } test;


   /*
    * Just for latency checking/debuging.
    */
   struct timeval         ts_sent;
   struct timeval         ts_received;
} monitor_test_t;


typedef struct {
   struct {
      unsigned internal_recv;
      unsigned internal_send;
      unsigned external_recv;
      unsigned external_send;

      unsigned internal_disconnect;
      unsigned external_disconnect;
   } event;
} monitor_tests_t;

void
checknetworktests(const struct timeval *tnow);
/*
 * Runs any previously queued networktests that should be done now, at
 * time "tnow".
 */

time_t
time_till_networktest_start(const sockd_io_t *io, const struct timeval *tnow,
                            networktest_t *internal, networktest_t *external);
/*
 * Checks how long it is till we should start a network test on the i/o
 * object "io".
 *
 * "tnow" is the time now.
 * "internal" and "external", if NULL, are set to indicate what tests should
 * be done when the returned number of seconds is up.
 *
 * Returns the number of seconds till we should do the test, with "internal"
 * and "external" attributes set for the tests that should be done.
 *
 * Returns -1 if no test is currently configured for this i/o object.
 */



time_t
time_till_mtu_test_start(const struct timeval *tnow,
                         const struct timeval *lastio);
/*
 * Returns the number of seconds till we should initiate a MTU test,
 * assuming the time now is "tnow", and the last time any i/o was done
 * is "lastio".
 */

void
mtutest(const int s, const interfaceside_t side, const struct timeval *tnow,
        mtutest_t *mtu);
/*
 * "s" is the socket the session is connected on, on the interfaceside
 * "side".
 * "tnow" is the current time.
 *
 * Checks if any MTU-related tests on "mtu" have now completed,
 * and logs as appropriately if so.  If not, continues checking,
 * possibly progressing to the next state.
 *
 * Upon return, "mtu" will be updated to indicate its current state,
 * possibly after reseting the socket "s" to it's initial state and
 * disabling further testing on this socket.
 */

int
initmtuattr(const int s, const interfaceside_t side, mtu_test_state_t *state);
/*
 * Initialises mtu-related attributes in order to start testing for
 * mtu-related problems.
 *
 * Returns 0 of inited ok, or -1 on error.
 */

void
networktest_add(networktest_tested_t *tested, const networktest_t *scheduled);
/*
 * Marks the tests set in "scheduled" as tested in "tested".
 */



int
monitor_hastimedout(const struct timeval *tnow, const monitor_t *monitor,
                    monitor_tests_t *tests);
/*
 * "tnow" is the current time.
 *
 * If the monitor "monitor" has any events or network tests that should
 * be done now, the function returns true and sets the tests that should
 * be done in "tests".
 *
 * Otherwise, the function returns false.
 */

struct timeval *
get_networktesttimeout(const struct timeval *tnow, struct timeval *timeout);
/*
 * "tnow" is the time now.
 *
 * Returns the time till the next network test, or NULL if no network
 * tests are scheduled currently.
 */

struct timeval *
get_monitor_eventtimeout(const struct timeval *tnow, struct timeval *timeout);
/*
 * "tnow" is the time now.
 *
 * Returns the time till the next network event check timeout, or NULL if no
 * events are currently being monitored.
 */

int
recv_monitor(const int s, monitor_test_t *object);
/*
 * Reads a monitor object from the socket 's'  and stores it in "object"
 * on success.
 *
 * Returns 0 if an object was read successfully, or -1 otherwise.
 */

int
send_monitor(const int s, const monitor_test_t *monitor);
/*
 * Sends the monitor object "object" to the monitor process connected to
 * socket  's'.
 *
 * Returns 0 if an object was sent successfully, or -1 otherwise.
 */

void
add_monitor_test(const monitor_test_t *monitor);
/*
 * Adds the monitor test object "monitor" to the monitor process's
 * queue of tests to do.
 */


void
remove_monitor_test(const size_t index);
/*
 * Removes the monitor test object with index "index" from our
 * queue of tests to do.
 */

const monitor_t *
monitormatch(const sockshost_t *from, const sockshost_t *to,
             const authmethod_t *auth, const connectionstate_t *state);
/*
 * If there is a monitor matching a session from "from" to "to",
 * with matching "auth" and "state", returns a pointer to that
 * monitor.
 * If "src" or "dst" is NULL, that address is not used when looking for a
 * matching monitor.
 *
 * If no matching monitor is found, NULL is returned.
 *
 */

void
monitor_move(monitor_t *oldmonitor, monitor_t *newmonitor,
             const int stayattached, const size_t sidesconnected,
             const clientinfo_t *cinfo,
             const int lock);
/*
 * Moves monitor/alarm-settings from "oldmonitor" to "newmonitor".
 * If "stayattached" is true, the function will remain attached to the
 * shmem segment belonging to "newmonitor"
 *
 * If oldmonitor shmid is 0, there were no old monitor matched, and
 * we should just configure the settings in the new monitor, if any.
 * Otherwise we need to unuse the settings in the old monitor also.
 */

monitor_t *
addmonitor(const monitor_t *monitor);
/*
 * Appends a copy of "monitor" to our list of monitors.
 */

void
monitor_detachfromlist(monitor_t *head);
/*
 * sockd_shmdt() from monitor-list starting at "head".
 */


const monitor_t *
shmid2monitor(const unsigned long shmid);
/*
 * Returns the sockscf.monitor matching "shmid", or NULL if no match.
 */


#endif /* !_MONITOR_H_ */
