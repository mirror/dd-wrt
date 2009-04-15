// ///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Program is based on the
// http://www.imasy.or.jp/~gotoh/ssh/connect.c
// Written By Shun-ichi GOTO <gotoh@taiyo.co.jp>
//
// If the source code for the program is not available from the place
// from which you received this file, check
// http://ultravnc.sourceforge.net/
//
// Linux port (C) 2005- Jari Korhonen, jarit1.korhonen@dnainternet.net
//////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "commondefines.h"
#include "repeater.h"

#define LOCAL_SOCKET	1
#ifndef FD_ALLOC
#define FD_ALLOC(nfds) ((fd_set*) malloc((nfds+7) / 8 ) )
#endif


//Return values:
// 1: Error (in select)
// 2: Server has disconnected
// 3: Viewer has disconnected
// 4: Error when reading from viewer 
// 5: Error when reading from server
int doRepeater(int server, int viewer)
{
    /** vars for viewer input data **/
    char viewerBuf[1024];           /* viewer input buffer */
    int viewerBufLen;               /* available data in viewerBuf */

    /** vars for server input data **/
    char serverBuf[1024];           /* server input buffer */
    int serverBufLen;               /* available data in serverBuf */

    /** other variables **/
    int nfds, len;
    fd_set ifds;
    fd_set ofds;
    
    /*
     * repeater between server and viewer 
     */
    nfds = ((viewer < server) ? server : viewer) + 1;
    viewerBufLen = 0;
    serverBufLen = 0;

    while (true) {
        FD_ZERO(&ifds);
        FD_ZERO(&ofds);

        /** prepare for reading viewer input **/
        if ((viewerBufLen < (int) sizeof(viewerBuf))) {
            FD_SET(viewer, &ifds);
        }

        /** prepare for reading server input **/
        if ((serverBufLen < (int) sizeof(serverBuf))) {
            FD_SET(server, &ifds);
        }

        if (select(nfds, &ifds, &ofds, NULL, NULL) == -1) {

            /* EINTR is normal if user presses ctrl+c */
            if (errno != EINTR) {
                /*
                * error in select, parent process should close both connections  
                */
                debug(LEVEL_2, "doRepeater(): select() failed, errno=%d (%s)\n", errno, strerror(errno));
            }
            return 1;
        }

        /*
         * server => viewer 
         */
        if (FD_ISSET(server, &ifds)
            && (serverBufLen < (int) sizeof(serverBuf))) {
            len = recv(server, serverBuf + serverBufLen, sizeof(serverBuf) - serverBufLen, 0);

            if (len == 0) {
                debug(LEVEL_3, "doRepeater(): connection closed by server\n");
                
                //When server closes connection, parent process should also disconnect viewer
                return 2;
            }
            else if (len == -1) {
                if (errno != ECONNRESET) {

                    /*
                     * error 
                     */
                    debug(LEVEL_2, "doRepeater(): recv() from server failed, errno=%d (%s)\n", errno, strerror(errno));
                    return 5;
                }
                else {
                    debug(LEVEL_2, "doRepeater(): ECONNRESET detected\n");
                }
            }
            else {
                serverBufLen += len;
            }
        }

        /*
         * viewer => server 
         */
        if (FD_ISSET(viewer, &ifds)
            && (viewerBufLen < (int) sizeof(viewerBuf))) {
            len = recv(viewer, viewerBuf + viewerBufLen, sizeof(viewerBuf) - viewerBufLen, 0);

            if (len == 0) {
                debug(LEVEL_3, "doRepeater(): connection closed by viewer\n");

                //When viewer closes connection,parent process should also disconnect server 
                return 3;
            }
            else if (len == -1) {

                /*
                 * error on reading from viewer 
                 */
                debug(LEVEL_2, "doRepeater(): recv() from viewer failed, errno = %d (%s)\n", errno, strerror(errno));
                return 4;
            }
            else {

                /*
                 * repeat 
                 */
                viewerBufLen += len;
            }
        }

        /*
         * flush data in viewerbuffer to server 
         */
        if (0 < viewerBufLen) {
            len = send(server, viewerBuf, viewerBufLen, 0);

            if (len == -1) {
                debug(LEVEL_2, "doRepeater(): send() (to server) failed, errno=%d (%s)\n", errno, strerror(errno));
            }
            else if (0 < len) {

                /*
                 * move data on to top of buffer 
                 */
                viewerBufLen -= len;

                if (0 < viewerBufLen)
                    memcpy(viewerBuf, viewerBuf + len, viewerBufLen);

                assert(0 <= viewerBufLen);
            }
        }

        /*
         * flush data in serverbuffer to viewer 
         */
        if (0 < serverBufLen) {
            len = send(viewer, serverBuf, serverBufLen, 0);

            if (len == -1) {
                debug(LEVEL_2, "doRepeater(): send() (to viewer) failed, errno=%d (%s)\n", errno, strerror(errno));
            }
            else {
                serverBufLen -= len;

                if (len < serverBufLen)
                    memcpy(serverBuf, serverBuf + len, serverBufLen);

                assert(0 <= serverBufLen);
            }
        }
    }
}
