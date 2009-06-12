//==========================================================================
//
//      vnc-server.c
//
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Chris Garry <cgarry@sweeneydesign.co.uk>
// Contributors:
// Date:         2003-08-22
// Purpose:
// Description:  VNC server for eCos
//
//####DESCRIPTIONEND####
//
//========================================================================*/


#include <cyg/hal/hal_arch.h>    /* CYGNUM_HAL_STACK_SIZE_MINIMUM & CYGNUM_HAL_STACK_SIZE_TYPICAL */
#include <pkgconf/system.h>
#include <cyg/infra/diag.h>      /* diag_printf */
#include <string.h>
#include <stdlib.h>
#include <vnc-server.h>

#include <network.h>

#define BACKLOG             5       /* Number of pending connections queue will hold */
#define MESSAGE_BUFFER_SIZE 50
#define TILE_SIZE           CYGNUM_VNC_SERVER_TILE_SIZE
#define TRUE_COLOUR_FLAG    1       /* True colour is set */
#define BIG_ENDIAN_FLAG     1       /* Always send colour data big endian */

/* Various definitions for different pixel types */
#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB332
#define BITS_PER_PIXEL      8      /* Bits per pixel */
#define PIXEL_DEPTH         8      /* Usefull bits per pixel */
#define RED_MAX             7
#define GREEN_MAX           7
#define BLUE_MAX            3
#define RED_SHIFT           5
#define GREEN_SHIFT         2
#define BLUE_SHIFT          0
#endif

#ifdef CYGNUM_VNC_SERVER_PIXEL_BGR233
#define BITS_PER_PIXEL      8      /* Bits per pixel */
#define PIXEL_DEPTH         8      /* Usefull bits per pixel */
#define RED_MAX             7
#define GREEN_MAX           7
#define BLUE_MAX            3
#define RED_SHIFT           0
#define GREEN_SHIFT         3
#define BLUE_SHIFT          6
#endif

#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB555
#define BITS_PER_PIXEL      16     /* Bits per pixel */
#define PIXEL_DEPTH         15     /* Usefull bits per pixel */
#define RED_MAX             31
#define GREEN_MAX           31
#define BLUE_MAX            31
#define RED_SHIFT           10
#define GREEN_SHIFT         5
#define BLUE_SHIFT          0
#endif

#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB565
#define BITS_PER_PIXEL      16     /* Bits per pixel */
#define PIXEL_DEPTH         16     /* Usefull bits per pixel */
#define RED_MAX             31
#define GREEN_MAX           63
#define BLUE_MAX            31
#define RED_SHIFT           11
#define GREEN_SHIFT         5
#define BLUE_SHIFT          0
#endif

#ifdef CYGNUM_VNC_SERVER_PIXEL_TRUECOLOR0888
#define BITS_PER_PIXEL      32     /* Bits per pixel */
#define PIXEL_DEPTH         24     /* Usefull bits per pixel */
#define RED_MAX             255
#define GREEN_MAX           255
#define BLUE_MAX            255
#define RED_SHIFT           16
#define GREEN_SHIFT         8
#define BLUE_SHIFT          0
#endif

/* Client to Server message types */
#define SET_PIXEL_FORMAT         0
#define FIX_COLOUR_MAP_ENTRIES   1
#define SET_ENCODINGS            2
#define FRAME_BUFFER_UPDATE_REQ  3
#define KEY_EVENT                4
#define POINTER_EVENT            5
#define CLIENT_CUT_TEXT          6

/* Macros to split colour to bytes */
#define COLOUR2BYTE1(col) ((col>>8)&0xFF)
#define COLOUR2BYTE0(col) (col&0xFF)

/* Function prototype */
static int GetMessageData(int, char *, int);
static int GenTileUpdateData(cyg_uint8 *);

/* Mouse handler funcrion - in vnc_mouse.c */
void vnc_mouse_handler(cyg_uint8 *data);
/* Keyboard handler funcrion - in vnc_kbd.c */
void vnc_kbd_handler(cyg_uint8 *data);


/* Thread function prototypes */
#ifdef CYGPKG_NET
cyg_thread_entry_t client_handler, frame_update;
#else
cyg_thread_entry_t tMain;
static void client_handler(void *);
static void frame_update(void *);
#endif

#ifdef CYGPKG_NET
/* Handles for the threads */
cyg_handle_t client_handler_hndl, frame_update_hndl;
/* Thread objects for the system to manipulate threads */
cyg_thread thread_s[2];
#else
cyg_handle_t lwip_startup_hndl;
cyg_thread thread_s;
#endif

/* Define size of each thread's stack */
#define MIN_STACK_SIZE (CYGNUM_HAL_STACK_SIZE_MINIMUM)
#define MAIN_STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL)

/* Allocate space for stacks */
#ifdef CYGPKG_NET
static char client_handler_stack[MAIN_STACK_SIZE];
static char frame_update_stack[MAIN_STACK_SIZE];
#else
static char lwip_startup_stack[MIN_STACK_SIZE];
#endif

/* Messages */
char server_ProtocolVersion[] = "RFB 003.003\n";
char bad_protocol[] = "Unsupported ProtocolVersion";
char sound_bell[] = "\2";
char desktop_name[] = CYGDAT_VNC_DESKTOP_NAME;

/* Frame Buffer */
volatile vnc_colour_t frame_buffer[CYGNUM_VNC_SERVER_FRAME_HEIGHT][CYGNUM_VNC_SERVER_FRAME_WIDTH];

/* Calculate the number of tiles in the X and Y directions */
#if (CYGNUM_VNC_SERVER_FRAME_HEIGHT % TILE_SIZE) != 0
#define NUM_TILES_Y_AXIS (CYGNUM_VNC_SERVER_FRAME_HEIGHT/TILE_SIZE + 1)
#define LAST_TILE_HEIGHT (CYGNUM_VNC_SERVER_FRAME_HEIGHT % TILE_SIZE)
#else
#define NUM_TILES_Y_AXIS (CYGNUM_VNC_SERVER_FRAME_HEIGHT/TILE_SIZE)
#define LAST_TILE_HEIGHT TILE_SIZE
#endif

#if (CYGNUM_VNC_SERVER_FRAME_WIDTH % TILE_SIZE) != 0
#define NUM_TILES_X_AXIS (CYGNUM_VNC_SERVER_FRAME_WIDTH/TILE_SIZE + 1)
#define LAST_TILE_WIDTH (CYGNUM_VNC_SERVER_FRAME_WIDTH % TILE_SIZE)
#else
#define NUM_TILES_X_AXIS (CYGNUM_VNC_SERVER_FRAME_WIDTH/TILE_SIZE)
#define LAST_TILE_WIDTH TILE_SIZE
#endif

/* Array for marking tiles that have been updated */
volatile int tile_updated[NUM_TILES_Y_AXIS][NUM_TILES_X_AXIS];

/* Conditional variable to signal that a client is connected and initialised */
cyg_mutex_t client_active_lock;
cyg_cond_t client_active_wait;
volatile int update_req;

/* Mutex and variable for sounding the client's bell */
cyg_mutex_t SoundBell_lock;
volatile int SoundBellCount;

volatile int client_sock;   /* Socket descriptor for client connection */


/* Variable to hold the frame format details */
vnc_frame_format_t frame_format = {CYGNUM_VNC_SERVER_FRAME_WIDTH,
                                   CYGNUM_VNC_SERVER_FRAME_HEIGHT,
                                   frame_buffer,
#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB332
                                   1,
#else
                                   0,
#endif
#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB555
                                   1,
#else
                                   0,
#endif
#ifdef CYGNUM_VNC_SERVER_PIXEL_RGB565
                                   1,
#else
                                   0,
#endif
#ifdef CYGNUM_VNC_SERVER_PIXEL_BGR233
                                   1,
#else
                                   0,
#endif
#ifdef CYGNUM_VNC_SERVER_PIXEL_TRUECOLOR0888
                                   1,
#else
                                   0,
#endif
};


/* Structure to hold the encoding type details */
volatile struct encoding_type_struct
{
    cyg_uint8 raw;
    cyg_uint8 copy_rectangle;
    cyg_uint8 rre;
    cyg_uint8 corre;
    cyg_uint8 hextile;
} encoding_type;


/*****************************************************************************/
/** System initializer
 *
 * This is called from the static constructor in init.cxx. It spawns
 * the main server thread and makes it ready to run.
 *
 *****************************************************************************/
__externC void cyg_vnc_server_startup(void)
{
#ifdef CYGPKG_NET
    /* BSD TCP/IP stack version */
    cyg_thread_create(CYGNUM_VNC_SERVER_PRIORITY,
                      client_handler,
                      0,
                      "ClientHandler",
                      (void *) client_handler_stack,
                      MAIN_STACK_SIZE,
                      &client_handler_hndl,
                      &thread_s[0]);

    cyg_thread_resume(client_handler_hndl);

#else
    /* lwIP TCP/IP stack version */
    cyg_thread_create(CYGNUM_VNC_SERVER_PRIORITY,
                      tMain,
                      0,
                      "lwIP_Startup",
                      (void *) lwip_startup_stack,
                      MIN_STACK_SIZE,
                      &lwip_startup_hndl,
                      &thread_s);

    cyg_thread_resume(lwip_startup_hndl);
#endif
}


#ifndef CYGPKG_NET
/* Startup thread for lwIP stack */
void tMain(cyg_addrword_t data)
{
    lwip_init();
    sys_thread_new(client_handler, (void *) 0, CYGNUM_VNC_SERVER_PRIORITY);
}
#endif


/*****************************************************************************/
/** Client Handler Thread.
 *
 *  @param  data     Ignored
 *
 *  This thread handles the client initialisation sequence.  Once the client
 *  is initialised this thread handles all received messages from the client,
 *  but does not send any data to the client.
 *
 *****************************************************************************/
#ifdef CYGPKG_NET
void client_handler(cyg_addrword_t data)
#else
static void client_handler(void *data)
#endif
{
    int server_sock;                /* Socket descriptor for server */
    struct sockaddr_in server_addr; /* Details of my IP address and port */
    struct sockaddr_in client_addr; /* Details of new connection IP address */
    int client_addr_size;

    int i, j;
    long int temp_long;
    char message_buffer[MESSAGE_BUFFER_SIZE];
    char protocol_ver[8];
    int message_len;
    int ProtocolOkay;
    cyg_uint32 *ptr_to_uint32;
    cyg_uint16 *ptr_to_uint16;

    if (CYGNUM_VNC_SERVER_DELAY)
    {
        cyg_thread_delay(CYGNUM_VNC_SERVER_DELAY);
    }

    /* Initialise mutex & cond vars */
    cyg_mutex_init(&client_active_lock);
    cyg_cond_init(&client_active_wait, &client_active_lock);
    update_req = 0;
    cyg_mutex_init(&SoundBell_lock);

    /* Create thread to handle frame_updates */
#ifdef CYGPKG_NET
    cyg_thread_create(CYGNUM_VNC_SERVER_PRIORITY,
                      frame_update,
                      0,
                      "FrameUpdate",
                      (void *) frame_update_stack,
                      MAIN_STACK_SIZE,
                      &frame_update_hndl,
                      &thread_s[1]);

    cyg_thread_resume(frame_update_hndl);
#else
    sys_thread_new(frame_update, (void *) 0, CYGNUM_VNC_SERVER_PRIORITY);
#endif

    /* Clear the encoding type structure */
    encoding_type.raw = 0;
    encoding_type.copy_rectangle = 0;
    encoding_type.rre = 0;
    encoding_type.corre = 0;
    encoding_type.hextile = 0;

    /* Clear the sound bell counter */
    SoundBellCount = 0;

#ifdef CYGPKG_NET
    /* Initialisation routine for BSD TCP/IP stack */
    init_all_network_interfaces();
#endif

    /* Create socket for incomming connections */
    if ((server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        diag_printf("socket() function failed\n");
        exit(1);
    }

    /* Construct the server address structure */
    memset(&server_addr, 0, sizeof(server_addr));  /* Fill entire structure with 0's */
    server_addr.sin_family      = AF_INET;         /* Internet address family */
    server_addr.sin_addr.s_addr = INADDR_ANY;      /* Autofill with my IP address */
    server_addr.sin_port        = htons(CYGNUM_VNC_SERVER_PORT);

    /* Bind socket to local address */
    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        diag_printf("bind() function failed");
        exit(1);
    }

    /* Set the socket to listen for incoming connections */
    if (listen(server_sock, BACKLOG) < 0)
    {
        diag_printf("listen() function failed");
        exit(1);
    }

    while(1)
    {

        /* Wait to accept a connection on this socket */
        client_addr_size = sizeof(client_addr);
        client_sock = (accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_size));

        if(client_sock < 0)
        {
            diag_printf("accept() function failed");
            exit(1);
        }


        /*
          ProtocolVersion Handshake - begin
        */

        /* Send ProtocolVersion we want to use to client */
        message_len = diag_sprintf(message_buffer, "RFB 003.003\n");
        if (send(client_sock, message_buffer, message_len, 0) != message_len)
        {
            goto close_connection;
        }

        /* Receive ProtocolVersion the client wants to use */
        if (GetMessageData(client_sock, &(message_buffer[0]), 12) == 0)
        {
            goto close_connection;
        }

        /* Check this is acceptable (RFB 003.xxx is okay) */
        ProtocolOkay = 1;
        for (i=0; i < 8; i++)
        {
            if (message_buffer[i] != server_ProtocolVersion[i])
            {
              ProtocolOkay = 0;
            }

            /* Store the protocol version - ignoring thr 'RFB ' part */
            protocol_ver[i] = message_buffer[i + 4];
        }

        protocol_ver[7] = 0;
        /*
          ProtocolVersion Handshake - end
        */


        /*
          Authentication - begin
        */

        /* Send Authentication scheme to be used to client */
        if (ProtocolOkay == 0)
        {
            /* ProtocolVerion is not okay */

            /* Generate the Bad ProtocolVerion message */
            ptr_to_uint32 = (cyg_uint32 *) &(message_buffer[0]);
            *ptr_to_uint32 = htonl(0);
            ptr_to_uint32 = (cyg_uint32 *) &(message_buffer[4]);
            *ptr_to_uint32 = htonl(strlen(bad_protocol));
            strcpy(&(message_buffer[8]), bad_protocol);

            if (send(client_sock, message_buffer, 8 + strlen(bad_protocol), 0) != (8 + strlen(bad_protocol)))
            {
                diag_printf("Call to send() failed\n");
            }

            goto close_connection;
        }

        else
        {
            /* ProtocolVerion is not okay - connect with no authentication*/

            /* Generate the No Authentication message */
            ptr_to_uint32 = (cyg_uint32 *) &(message_buffer[0]);
            *ptr_to_uint32 = htonl((cyg_uint32)1);

            if (send(client_sock, message_buffer, 4, 0) != 4)
            {
                diag_printf("Call to send() failed\n");
                goto close_connection;
            }
        }

        /*
          Authentication - end
        */


        /*
          ClientInitialisation - begin
        */

        /* Receive initialisation message from client (1 byte) */
        if (GetMessageData(client_sock, &(message_buffer[0]), 1) == 0)
        {
            goto close_connection;
        }

        /* Do nothing with this as we only support 1 Client at a time */

        /*
          ClientInitialisation - end
        */


        /*
          ServerInitialisation - begin
        */

        /* Create Initialisation message for client */
        ptr_to_uint16 = (cyg_uint16 *) &(message_buffer[0]);
        *ptr_to_uint16 = htons((cyg_uint16)CYGNUM_VNC_SERVER_FRAME_WIDTH);

        ptr_to_uint16 = (cyg_uint16 *) &(message_buffer[2]);
        *ptr_to_uint16 = htons((cyg_uint16)CYGNUM_VNC_SERVER_FRAME_HEIGHT);

        message_buffer[4] = (cyg_uint8)BITS_PER_PIXEL;
        message_buffer[5] = (cyg_uint8)PIXEL_DEPTH;
        message_buffer[6] = (cyg_uint8)BIG_ENDIAN_FLAG;
        message_buffer[7] = (cyg_uint8)TRUE_COLOUR_FLAG;

        ptr_to_uint16 = (cyg_uint16 *) &(message_buffer[8]);
        *ptr_to_uint16 = htons((cyg_uint16)RED_MAX);

        ptr_to_uint16 = (cyg_uint16 *) &(message_buffer[10]);
        *ptr_to_uint16 = htons((cyg_uint16)GREEN_MAX);

        ptr_to_uint16 = (cyg_uint16 *) &(message_buffer[12]);
        *ptr_to_uint16 = htons((cyg_uint16)BLUE_MAX);

        message_buffer[14] = (cyg_uint8)RED_SHIFT;
        message_buffer[15] = (cyg_uint8)GREEN_SHIFT;
        message_buffer[16] = (cyg_uint8)BLUE_SHIFT;

        ptr_to_uint32 = (cyg_uint32 *) &(message_buffer[20]);
        *ptr_to_uint32 = htonl(strlen(desktop_name));
        strcpy(&(message_buffer[24]), desktop_name);

        if (send(client_sock, message_buffer, 24 + strlen(desktop_name), 0) != (24 + strlen(desktop_name)))
        {
            diag_printf("Call to send() failed\n");
        }

        /*
          ServerInitialisation - end
        */

        /* Cancel any outstanding Sound Bell requests */
        cyg_mutex_lock(&SoundBell_lock);
        SoundBellCount = 0;
        cyg_mutex_unlock(&SoundBell_lock);

        diag_printf("VNC client connected (RFB Protocol Ver: %s)\n", protocol_ver);

        /* Main message handling loop */
        while(1)
        {
            int num_of_encodings;

            /* Receive 1st byte of message from client */
            if (GetMessageData(client_sock, &(message_buffer[0]), 1) == 0)
            {
                goto close_connection;
            }

            switch(message_buffer[0])
            {
            case SET_PIXEL_FORMAT:

                /* Get the remainder (19 bytes) of message */
                if (GetMessageData(client_sock, &(message_buffer[1]), 19) == 0)
                {
                    goto close_connection;
                }

                /* Check pixel format is as expected */
                i = 0;
                if (message_buffer[4] != BITS_PER_PIXEL)
                {
                    diag_printf("SetPixelFormat message from client has incompatible bits-per-pixel field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", BITS_PER_PIXEL, message_buffer[4]);
                    i++;
                }

                if (message_buffer[5] != PIXEL_DEPTH)
                {
                    diag_printf("SetPixelFormat message from client has incompatible pixel-depth field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", PIXEL_DEPTH, message_buffer[5]);
                    i++;
                }

                if (message_buffer[7] != TRUE_COLOUR_FLAG)
                {
                    diag_printf("SetPixelFormat message from client has incompatible true-colour-flag field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", TRUE_COLOUR_FLAG, message_buffer[7]);
                    i++;
                }

                ptr_to_uint16 = (cyg_uint16 *)&(message_buffer[8]);
                if (htons(*ptr_to_uint16) != RED_MAX)
                {
                    diag_printf("SetPixelFormat message from client has incompatible red-max field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", RED_MAX, htons(*ptr_to_uint16));
                    i++;
                }

                ptr_to_uint16 = (cyg_uint16 *)&(message_buffer[10]);
                if (htons(*ptr_to_uint16) != GREEN_MAX)
                {
                    diag_printf("SetPixelFormat message from client has incompatible green-max field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", GREEN_MAX, htons(*ptr_to_uint16));
                    i++;
                }

                ptr_to_uint16 = (cyg_uint16 *)&(message_buffer[12]);
                if (htons(*ptr_to_uint16) != BLUE_MAX)
                {
                    diag_printf("SetPixelFormat message from client has incompatible blue-max field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", BLUE_MAX, htons(*ptr_to_uint16));
                    i++;
                }

                if (message_buffer[14] != RED_SHIFT)
                {
                    diag_printf("SetPixelFormat message from client has incompatible red-shift field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", RED_SHIFT, message_buffer[14]);
                    i++;
                }

                if (message_buffer[15] != GREEN_SHIFT)
                {
                    diag_printf("SetPixelFormat message from client has incompatible green-shift field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", GREEN_SHIFT, message_buffer[15]);
                    i++;
                }

                if (message_buffer[16] != BLUE_SHIFT)
                {
                    diag_printf("SetPixelFormat message from client has incompatible blue-shift field:\n");
                    diag_printf("    Expected: %d - Got: %d\n", BLUE_SHIFT, message_buffer[16]);
                    i++;
                }

                if (i)
                {
                    diag_printf("Disconnecting from client\n");
                    diag_printf("Please ensure the 'Auto select' is not enabled in your vncviewer options,\n");
                    diag_printf("then try connecting again.\n");
                    goto close_connection_quietly;
                }

                break;

            case FIX_COLOUR_MAP_ENTRIES:
                /* Not supported, just get the data from the buffer and ignore it */

                /* Get the next 5 bytes of message */
                if (GetMessageData(client_sock, &(message_buffer[1]), 5) == 0)
                {
                    goto close_connection;
                }

                /* Calculate how many colour entries are in the buffer */
                i = message_buffer[4]*255 + message_buffer[5];
                i *= 6;

                /* Get this amount of data from the buffer */
                for (j = 0; j < i; j++)
                {
                    if (GetMessageData(client_sock, &(message_buffer[6]), 6) == 0)
                    {
                        goto close_connection;
                    }
                }

                break;

            case SET_ENCODINGS:
                /* Get the next 3 bytes of message */
                if (GetMessageData(client_sock, &(message_buffer[1]), 3) == 0)
                {
                    goto close_connection;
                }

                num_of_encodings = message_buffer[2]*255 + message_buffer[3];

                /* Get the remainder of message */
                if (GetMessageData(client_sock, &(message_buffer[0]), 4 * num_of_encodings) == 0)
                {
                    goto close_connection;
                }

                /* Clear the encoding type structure */
                encoding_type.raw = 0;
                encoding_type.copy_rectangle = 0;
                encoding_type.rre = 0;
                encoding_type.corre = 0;
                encoding_type.hextile = 0;

                for (i = 0; i < num_of_encodings; i++)
                {
                    switch(message_buffer[3 + (i*4)])
                    {
                    case 0:  /* Raw encoding */
                        encoding_type.raw = i + 1;
                        break;
                    case 1:  /* Copy rectangle encoding */
                        encoding_type.copy_rectangle = i + 1;
                        break;
                    case 2:  /* RRE encoding */
                        encoding_type.rre = i + 1;
                        break;
                    case 4:  /* CoRRE encoding */
                        encoding_type.corre = i + 1;
                        break;
                    case 5:  /* Hextile encoding */
                        encoding_type.hextile = i + 1;
                        break;
                    default:  /* Unknown coding type - do nothing */
                        break;
                    }
                }

                if (!encoding_type.corre)
                {
                    diag_printf("Warning: SetEncoding message from client does not support CoRRE Encoding\n");
                    diag_printf("Raw encoding will be used instead\n");
                }

                break;

            case FRAME_BUFFER_UPDATE_REQ:
                /* Get the remainder of message (9 bytes) */
                if (GetMessageData(client_sock, &(message_buffer[1]), 9) == 0)
                {
                    goto close_connection;
                }

                if (!message_buffer[1])
                {
                    /* Non-incremental mode - mark the squares that need to be updated */
                    for (i = (message_buffer[2]*255 + message_buffer[3])/TILE_SIZE;
                         i <= (message_buffer[2]*255 + message_buffer[3] + message_buffer[6]*255 + message_buffer[7])/TILE_SIZE;
                         i++)
                    {
                        for (j = (message_buffer[4]*255 + message_buffer[5])/TILE_SIZE;
                             j <= (message_buffer[4]*255 + message_buffer[5] + message_buffer[8]*255 + message_buffer[9])/TILE_SIZE;
                             j++)
                        {
                            tile_updated[j][i] = 1;
                        }
                    }
                }

                /* Signal that there is now a pending update request */
                cyg_mutex_lock(&client_active_lock);
                update_req = 1;
                cyg_cond_signal(&client_active_wait);
                cyg_mutex_unlock(&client_active_lock);

                break;

            case KEY_EVENT:
                /* Handle the key event */
                /* Get the remainder of message (7 bytes) */
                if (GetMessageData(client_sock, &(message_buffer[1]), 7) == 0)
                {
                    goto close_connection;
                }

#ifdef CYGPKG_VNC_SERVER_BUILD_KEYBOARD_DRIVER
                /* Call the keyboard handle function */
                vnc_kbd_handler(&(message_buffer[0]));
#endif
                break;

            case POINTER_EVENT:
                /* Handle the pointer event */
                /* Get the remainder of message (5 bytes) */
                if (GetMessageData(client_sock, &(message_buffer[1]), 5) == 0)
                {
                    goto close_connection;
                }
                /* Button mask: message_buffer[1] */
                /* X-position: message_buffer[2]*255 + message_buffer[3] */
                /* Y-position: message_buffer[4]*255 + message_buffer[5] */

#ifdef CYGPKG_VNC_SERVER_BUILD_MOUSE_DRIVER
                /* Called the mouse handler function */
                vnc_mouse_handler(&(message_buffer[0]));
#endif
                break;

            case CLIENT_CUT_TEXT:
                /* Handle the client has cut text event */
                /* Current we just get and discard the data */
                /* In future it might be nice to expand the API to */
                /* allow this data to be passed to the application */

                /* Get the next 7 bytes of the message */
                if (GetMessageData(client_sock, &(message_buffer[1]), 7) == 0)
                {
                    goto close_connection;
                }

                ptr_to_uint32 = (cyg_uint32 *)&(message_buffer[4]);
                temp_long = htonl(*ptr_to_uint32);

                while (temp_long > 0)
                {
                    /* Get the text in chunks MESSAGE_BUFFER_SIZE-1 characters */
                    if (temp_long > MESSAGE_BUFFER_SIZE-2)
                    {
                        if (GetMessageData(client_sock, &(message_buffer[0]), MESSAGE_BUFFER_SIZE-1) == 0)
                        {
                            goto close_connection;
                        }

                        message_buffer[MESSAGE_BUFFER_SIZE-1] = 0;
                        temp_long -= (MESSAGE_BUFFER_SIZE-1);
                    }
                    else
                    {
                        if (GetMessageData(client_sock, &(message_buffer[0]), temp_long) == 0)
                        {
                            goto close_connection;
                        }

                        message_buffer[temp_long] = 0;
                        temp_long = 0;
                    }
                }

                break;

            default:
                diag_printf("Unknown message from client\n");
            }
        }

        close_connection:
        diag_printf("VNC client disconnected\n");

        close_connection_quietly:

        /* Cancel any outstanding update requests */
        cyg_mutex_lock(&client_active_lock);
        update_req = 0;
        cyg_mutex_unlock(&client_active_lock);

        close(client_sock);
    }
}


/*****************************************************************************/
/** Frame update thread.
 *
 *  @param  data     Ignored
 *
 *  This thread handles the sending of all frame update data to the client and
 *  sends the 'ring bell' message to the client when required.
 *
 *****************************************************************************/
#ifdef CYGPKG_NET
void frame_update(cyg_addrword_t data)
#else
static void frame_update(void *data)
#endif
{
    int i, j;
    int x_pos, y_pos;
    int packet_length;
    int num_updated_tiles;
    cyg_uint16 *ptr_to_uint16;
    static long rtc_resolution[] = CYGNUM_KERNEL_COUNTERS_RTC_RESOLUTION;

    /* These are declared static so they don't use thread stack memory */
    static cyg_uint8 FramebufferUpdate_msg[4 + 12 + TILE_SIZE*TILE_SIZE*BITS_PER_PIXEL/8 + 1460];
    static int FrameBufferPtr;  /* Pointer to next space in buffer */
    static int tile_updated_local[NUM_TILES_Y_AXIS][NUM_TILES_X_AXIS];

    while(1)
    {
        /* Wait until client sends a frame update request */
        wait_for_client:
        cyg_mutex_lock(&client_active_lock);   /* Lock the mutex */
        while( update_req == 0 )               /* Wait until the client is active */
        {
            cyg_cond_wait(&client_active_wait);
        }

        cyg_mutex_unlock(&client_active_lock);

        /* Copy tile_updated array to local version and clear copied tiles */
        cyg_scheduler_lock();
        num_updated_tiles = 0;
        for (i = 0; i < NUM_TILES_Y_AXIS; i++)
        {
            for (j = 0; j < NUM_TILES_X_AXIS; j++)
            {
                if (tile_updated[i][j])
                {
                    tile_updated_local[i][j] = 1;
                    tile_updated[i][j] = 0;
                    num_updated_tiles++;  /* Keep count of the updated tiles */
                }
            }
        }

        cyg_scheduler_unlock();

        if (num_updated_tiles)
        {
            /* Fill in constant parts of FramebufferUpdate Message */
            FramebufferUpdate_msg[0] = 0;  /* Message-type */
            FramebufferUpdate_msg[1] = 0;  /* Padding */
            ptr_to_uint16 = (cyg_uint16 *) &(FramebufferUpdate_msg[2]);
            *ptr_to_uint16 = htons(num_updated_tiles);  /* Number-of-rectangles */
            FrameBufferPtr = 4;

            for (y_pos = 0; y_pos < NUM_TILES_Y_AXIS; y_pos++)
            {
                for (x_pos = 0; x_pos < NUM_TILES_X_AXIS; x_pos++)
                {
                    if (tile_updated_local[y_pos][x_pos])
                    {
                        if (update_req)
                        {
                            cyg_mutex_lock(&client_active_lock);      /* Lock the mutex */
                            update_req = 0;                           /* Cancel the update request flag */
                            cyg_mutex_unlock(&client_active_lock);    /* Unlock the mutex */
                        }

                        /* Send current square data to client */

                        /* x-position */
                        FramebufferUpdate_msg[FrameBufferPtr+0] = (x_pos * TILE_SIZE) / 256;
                        FramebufferUpdate_msg[FrameBufferPtr+1] = (x_pos * TILE_SIZE) % 256;

                        /* y-position */
                        FramebufferUpdate_msg[FrameBufferPtr+2] = (y_pos * TILE_SIZE) / 256;
                        FramebufferUpdate_msg[FrameBufferPtr+3] = (y_pos * TILE_SIZE) % 256;


                        /* Set width of tile in packet */
                        if (x_pos == (NUM_TILES_X_AXIS -1))
                        {
                            /* Last tile in X-axis */
                            FramebufferUpdate_msg[FrameBufferPtr+4] = LAST_TILE_WIDTH / 256;
                            FramebufferUpdate_msg[FrameBufferPtr+5] = LAST_TILE_WIDTH % 256;
                        }
                        else
                        {
                            FramebufferUpdate_msg[FrameBufferPtr+4] = TILE_SIZE / 256;
                            FramebufferUpdate_msg[FrameBufferPtr+5] = TILE_SIZE % 256;
                        }

                        if (y_pos == (NUM_TILES_Y_AXIS -1))
                        {
                            /* Last tile in Y-axis */
                            FramebufferUpdate_msg[FrameBufferPtr+6] = LAST_TILE_HEIGHT / 256;
                            FramebufferUpdate_msg[FrameBufferPtr+7] = LAST_TILE_HEIGHT % 256;
                        }
                        else
                        {
                            FramebufferUpdate_msg[FrameBufferPtr+6] = TILE_SIZE / 256;
                            FramebufferUpdate_msg[FrameBufferPtr+7] = TILE_SIZE % 256;
                        }

                        /* Generate the packet data for this tile */
                        packet_length = GenTileUpdateData(&(FramebufferUpdate_msg[FrameBufferPtr]));

                        /* Send the packet data for this tile to the client */
                        FrameBufferPtr += packet_length;

                        if (FrameBufferPtr > 1460)
                        {
                            /* Send the data to the client */
                            if (send(client_sock, FramebufferUpdate_msg, FrameBufferPtr, 0) != FrameBufferPtr)
                        {
                            goto wait_for_client;
                        }

                            FrameBufferPtr = 0;
                        }

                        tile_updated_local[y_pos][x_pos] = 0;  /* Clear the update bit for this square */
                    }
                }
            }

            if (FrameBufferPtr > 0)
            {
                /* Last data for this update, send it to the client */
                if (send(client_sock, FramebufferUpdate_msg, FrameBufferPtr, 0) != FrameBufferPtr)
                {
                    goto wait_for_client;
                }

                FrameBufferPtr = 0;
            }

        }
        else  /* if (num_updated_tiles) */
        {
            /* There was no new display data to send to the client */
            /* Sleep for 1/20th second before checking again */
            cyg_thread_delay(rtc_resolution[1]/20);
        }

        /* Check for sound bell event */
        cyg_mutex_lock(&SoundBell_lock);
        if (SoundBellCount)
        {
            --SoundBellCount;
            cyg_mutex_unlock(&SoundBell_lock);

            if (send(client_sock, sound_bell, 1, 0) != 1)
            {
                goto wait_for_client;
            }
        }
        else
        {
            cyg_mutex_unlock(&SoundBell_lock);
        }
    }
}


/*****************************************************************************/
/** Generate tile update data function
 *
 *  @param  *packet_buffer  Buffer to store tile data
 *
 *  @return Length of generated data in bytes
 *
 *  This function is called by the frame_update thread to generate the message
 *  data for a tile to send to the client.  This function expects the
 *  x-position, y-position, width and height fields of the buffer to be filled
 *  in before it is called.
 *
 *  The format of the buffer is:
 *  packet_buffer[0:1] - x-position of tile
 *  packet_buffer[2:3] - y-position of tile
 *  packet_buffer[4:5] - width of tile
 *  packet_buffer[6:7] - height of tile
 *  packet_buffer[8 onwards] - Pixel data for the tile
 *
 *  The pixel data will be encoded with CoRRE encoding (if the CDL option is
 *  enabled and the client can handle it) or RAW encoding if that is smaller
 *  than CoRRE encoding for that particular tile.
 *
 *****************************************************************************/
static int GenTileUpdateData(cyg_uint8 *packet_buffer)
{
    cyg_uint16 x_pos, y_pos;
    int i, j;
    int packet_length;
    int tile_width, tile_height;

#ifdef CYGNUM_VNC_SERVER_USE_CORRE_ENCODING
    static vnc_colour_t tile_buffer[TILE_SIZE][TILE_SIZE];  /* Buffer to hold tile to be encoded */
    vnc_colour_t  pixel_colour;
    vnc_colour_t bg_colour;
    int no_of_subrects, subrect_width, subrect_height;
    int k, l;

    no_of_subrects = 0;  /* Set to no sub-rectangles to start with */
#endif   /* CYGNUM_VNC_SERVER_USE_CORRE_ENCODING */

    packet_length = 20-4+(BITS_PER_PIXEL/8);  /* Set to minimum packet length to start with */

    /* Get the X and Y positions of this tile from the packet buffer */
    x_pos = packet_buffer[0] * 256 + packet_buffer[1];
    y_pos = packet_buffer[2] * 256 + packet_buffer[3];

    /* Get the tile width and height from the packet buffer */
    tile_width = packet_buffer[4] * 256 + packet_buffer[5];
    tile_height = packet_buffer[6] * 256 + packet_buffer[7];

#ifdef CYGNUM_VNC_SERVER_USE_CORRE_ENCODING
    /* Set the encoding type to RRE  */
    if (!encoding_type.corre)
    {
        /* CoRRE encoding is not supported - just use raw encoding */
        goto use_raw_encoding;
    }

    /* Set encoding type to CoRRE encoding in packet buffer */
    packet_buffer[8+0] = 0;
    packet_buffer[8+1] = 0;
    packet_buffer[8+2] = 0;
    packet_buffer[8+3] = 4;

    /* Copy tile from the main frame buffer to the local tile buffer */
    for (i = 0; i < tile_height; i++)
    {
        for (j = 0; j < tile_width; j++)
        {
            tile_buffer[i][j] = frame_buffer[y_pos + i][x_pos + j];
        }
    }

    /* Find the background colour */
    /* We just assume the (0, 0) pixel in the tile is the bgcolour */
    /* Its quick!!! */
    bg_colour = frame_buffer[y_pos][x_pos];

    /* Set the background colour in the packet buffer */
#if BITS_PER_PIXEL == 8
    packet_buffer[16] = (vnc_colour_t) bg_colour;
#endif
#if BITS_PER_PIXEL == 16
    packet_buffer[16]   = COLOUR2BYTE0(bg_colour);
    packet_buffer[16+1] = COLOUR2BYTE1(bg_colour);
#endif

#ifdef CYGNUM_VNC_SERVER_CORRE_ENCODING_HACK
    /* Add an initial sub-rectangle to paint the background the background colour */
    /* This is required because of a known bug in the VNC viewer (x86 version) */
#if BITS_PER_PIXEL == 8
    packet_buffer[packet_length] = (vnc_colour_t) bg_colour;
    packet_length++;
#endif
#if BITS_PER_PIXEL == 16
    packet_buffer[packet_length]   = packet_buffer[16];
    packet_buffer[packet_length+1] = packet_buffer[16+1];
    packet_length += 2;
#endif
    packet_buffer[packet_length]   = (cyg_uint8) 0;  /* Sub-rect x-pos */
    packet_buffer[packet_length+1] = (cyg_uint8) 0;  /* Sub-rect y-pos*/
    packet_buffer[packet_length+2] = (cyg_uint8) tile_width;  /* Sub-rect width*/
    packet_buffer[packet_length+3] = (cyg_uint8) tile_height;  /* Sub-rect height*/
    packet_length += 4;
    no_of_subrects++;  /* Increment sub-rectangle count */
#endif

    /* Scan trough tile and find sub-rectangles */
    for (i = 0; i < tile_height; i++)
    {
        for (j = 0; j < tile_width; j++)
        {
            if (tile_buffer[i][j] != bg_colour)
            {
                /* This is a non-background pixel */
                subrect_width = 1;
                pixel_colour = tile_buffer[i][j];

                /* Extend the sub-rectangle to its maximum width */
                for (subrect_width = 1; subrect_width <= tile_width-j-1; subrect_width++)
                {
                    if (tile_buffer[i][j+subrect_width] != pixel_colour)
                    {
                        goto got_subrect_width;
                    }
                }

                got_subrect_width:

                /* Extend the sub-rectangle to its maximum height */
                for (subrect_height=1; subrect_height <= tile_height-i-1; subrect_height++)
                {
                    for (k = j; k < j+subrect_width; k++)
                    {
                        if (tile_buffer[i+subrect_height][k] != pixel_colour)
                        {
                            goto got_subrect_height;
                        }
                    }
                }

                got_subrect_height:

                /* Delete the pixels for the sub-rectangle from the sub-rectangle */
                for (k = i; k < i+subrect_height; k++)
                {
                    for (l = j; l < j+subrect_width; l++)
                    {
                        tile_buffer[k][l] = bg_colour;
                    }
                }

                /* Append new sub-rectangle data to the packet buffer */
#if BITS_PER_PIXEL == 8
                packet_buffer[packet_length] = (vnc_colour_t) pixel_colour;
                packet_length++;
#endif
#if BITS_PER_PIXEL == 16
                packet_buffer[packet_length]   = COLOUR2BYTE0(pixel_colour);
                packet_buffer[packet_length+1] = COLOUR2BYTE1(pixel_colour);
                packet_length += 2;
#endif

                packet_buffer[packet_length] = (cyg_uint8) j;  /* Sub-rect x-pos */
                packet_length++;

                packet_buffer[packet_length] = (cyg_uint8) i;  /* Sub-rect y-pos*/
                packet_length++;

                packet_buffer[packet_length] = (cyg_uint8) subrect_width;  /* Sub-rect width*/
                packet_length++;

                packet_buffer[packet_length] = (cyg_uint8) subrect_height;  /* Sub-rect height*/
                packet_length++;

                no_of_subrects++;  /* Increment sub-rectangle count */

                if (packet_length >= 12 + tile_height*tile_width*(BITS_PER_PIXEL/8) - 6)
                {
                    /* The next sub-rectangle will make the packet size   */
                    /* larger than a rew encoded packet - so just use raw */
                    goto use_raw_encoding;
                }
            }
        }
    }

    /* Fill in no_of_sub-rectangles field in packet buffer */
    packet_buffer[12+0] = 0;
    packet_buffer[12+1] = 0;
    packet_buffer[12+2] = no_of_subrects / 256;
    packet_buffer[12+3] = no_of_subrects % 256;

    /* CoRRE data encoding for tile complete */
    return packet_length;

    use_raw_encoding:
#endif   /* CYGNUM_VNC_SERVER_USE_CORRE_ENCODING */

    /* Create packet data using RAW encoding */
    for (i = 0; i < tile_height; i++)
    {
        for (j = 0; j < tile_width; j++)
        {
#if BITS_PER_PIXEL == 8
             packet_buffer[12 + tile_width * i + j] = frame_buffer[y_pos + i][x_pos + j];
#endif
#if BITS_PER_PIXEL == 16
            packet_buffer[12 + 2 * tile_width * i + 2*j]    = COLOUR2BYTE0(frame_buffer[y_pos + i][x_pos + j]);
            packet_buffer[12 + 2 * tile_width * i + 2*j+ 1] = COLOUR2BYTE1(frame_buffer[y_pos + i][x_pos + j]);
#endif
        }
    }

    /* Set the encoding type to raw */
    packet_buffer[8+0] = 0;
    packet_buffer[8+1] = 0;
    packet_buffer[8+2] = 0;
    packet_buffer[8+3] = 0;

    return (12 + tile_width*tile_height*(BITS_PER_PIXEL/8));
}


/*****************************************************************************/
/** Get message data function
 *
 *  @param  socket_fd    File descriptor of the socket to get the data from.
 *  @param  *buffer      Buffer to store received data in.
 *  @param  num_bytes    Number of bytes to attempt to get.
 *
 *  @return  1 on sucessfull completion - 0 on error.
 *
 *  This function is called by the client_handler thread to get data from the
 * client's socket.
 *
 *****************************************************************************/
static int GetMessageData(int socket_fd, char *buffer, int num_bytes)
{
    int bytes_rxd;
    int message_len = 0;

    while (message_len < num_bytes)
    {
        if ((bytes_rxd = recv(socket_fd, buffer, num_bytes, 0)) <= 0)
        {
            return 0;
        }
        message_len += bytes_rxd;
    }

    return 1;
}


/* Driver functions */
vnc_frame_format_t* VncGetInfo(void)
{
    return &frame_format;
}


void VncInit(vnc_colour_t colour)
{
    /* Initialise the frame buffer */
    int i, j;

    for (i = 0; i < CYGNUM_VNC_SERVER_FRAME_HEIGHT; i++)
    {
        for (j = 0; j < CYGNUM_VNC_SERVER_FRAME_WIDTH; j++)
        {
            frame_buffer[i][j] = colour;
        }
    }

    for (i = 0; i < CYGNUM_VNC_SERVER_FRAME_HEIGHT/TILE_SIZE; i++)
    {
        for (j = 0; j < CYGNUM_VNC_SERVER_FRAME_WIDTH/TILE_SIZE; j++)
        {
            tile_updated[i][j] = 1;
        }
    }
}


void VncDrawPixel(cyg_uint16 x, cyg_uint16 y, vnc_colour_t colour)
{
    /* Set that pixel to 'colour' */
    frame_buffer[y][x] = colour;

    /* Mark the tile for update */
    tile_updated[y/TILE_SIZE][x/TILE_SIZE] = 1;
}


vnc_colour_t VncReadPixel(cyg_uint16 x, cyg_uint16 y)
{
    return frame_buffer[y][x];
}


void VncDrawHorzLine(cyg_uint16 x1, cyg_uint16 x2, cyg_uint16 y, vnc_colour_t colour)
{
    int i;

    /* Draw the line */
    for (i = x1; i <= x2; i++)
    {

        frame_buffer[y][i] = colour;
    }

    /* Mark the tiles for update */
    for (i = x1/TILE_SIZE; i <= x2/TILE_SIZE; i++)
    {
        tile_updated[y/TILE_SIZE][i] = 1;
    }
}

void VncDrawVertLine(cyg_uint16 x, cyg_uint16 y1, cyg_uint16 y2, vnc_colour_t colour)
{
    int i;

    /* Draw the line */
    for (i = y1; i <= y2; i++)
    {
        frame_buffer[i][x] = colour;
    }

    /* Mark the tiles for update */
    for (i = y1/TILE_SIZE; i <= y2/TILE_SIZE; i++)
    {
        tile_updated[i][x/TILE_SIZE] = 1;
    }
}


void VncFillRect(cyg_uint16 x1, cyg_uint16 y1, cyg_uint16 x2, cyg_uint16 y2, vnc_colour_t colour)
{
    /* Draw a solid rectangle */
    int i, j;

    for (i = y1; i <= y2; i++)
    {
        for (j = x1; j <= x2; j++)
        {
            frame_buffer[i][j] = colour;
        }
    }

    for (i = y1/TILE_SIZE; i <= y2/TILE_SIZE; i++)
    {
        for (j = x1/TILE_SIZE; j <= x2/TILE_SIZE; j++)
        {
            tile_updated[i][j] = 1;
        }
    }
}


/* Copy rectangle of size widthxheight form (x1, y1) to (x2, y2) */
void VncCopyRect(cyg_uint16 x1, cyg_uint16 y1, cyg_uint16 width, cyg_uint16 height, cyg_uint16 x2, cyg_uint16 y2)
{
    int i, j;
    int xmove, ymove;

    /* Calulate how much to move the rectangle by */
    xmove = x2 - x1;
    ymove = y2 - y1;

    if ((xmove == 0) && (ymove == 0))
    {
        /* No move required */
        return;
    }

    if (ymove < 0)
    {
        /* Copy pixels from top to bottom */
        if (x1 >= x2)
        {
            /* Copy pixels from left to right */
            for (i = y1; i < y1 + height; i++)
            {
                for (j = x1; j < x1 + width; j++)
                {
                    frame_buffer[i + ymove][j + xmove] = frame_buffer[i][j];
                }
            }
        }
        else
        {
            /* Copy pixels from right to left */
            for (i = y1; i < y1 + height; i++)
            {
                for (j = x1 + width - 1; j >= x1 ; j--)
                {
                    frame_buffer[i + ymove][j + xmove] = frame_buffer[i][j];
                }
            }
        }
    }
    else
    {
        /* Copy pixels from bottom to top */
        if (xmove < 0)
        {
            /* Copy pixels from left to right */
            for (i = y1 + height - 1; i >= y1; i--)
            {
                for (j = x1; j < x1 + width; j++)
                {
                    frame_buffer[i + ymove][j + xmove] = frame_buffer[i][j];
                }
            }
        }
        else
        {
            /* Copy pixels from right to left */
            for (i = y1 + height - 1; i >= y1; i--)
            {
                for (j = x1 + width - 1; j >= x1 ; j--)
                {
                    frame_buffer[i + ymove][j + xmove] = frame_buffer[i][j];
                }
            }
        }

    }

    /* Mark the required tiles for update */
    for (i = y2/TILE_SIZE; i <= (y2 + height - 1) /TILE_SIZE; i++)
    {
        for (j = x2/TILE_SIZE; j <= (x2 + width - 1)/TILE_SIZE; j++)
        {
            tile_updated[i][j] = 1;
        }
    }
}


/* Function to copy a rectangle from the frame buffer to a supplied buffer */
void VncCopyRect2Buffer(cyg_uint16 x, cyg_uint16 y, cyg_uint16 width, cyg_uint16 height,
                        void *buffer, cyg_uint16 buff_w, cyg_uint16 buff_h, cyg_uint16 x_off, cyg_uint16 y_off)
{
    int i, j;
    cyg_uint16 eol_padding = buff_w - width - x_off;

#if (BITS_PER_PIXEL == 8)
    cyg_uint8 *dst_buffer;
    dst_buffer = (cyg_uint8 *)buffer;
#else
    cyg_uint16 *dst_buffer;
    dst_buffer = (cyg_uint16 *)buffer;
#endif

    dst_buffer += ((x_off + width) * y_off);  /* Allow for a y offset into supplied buffer */
    for (i = y; i < y + height; i++)
    {
        dst_buffer += x_off;  /* Allow for an x offset into supplied buffer */
        for (j = x; j < x + width; j++)
        {
            /* Copy each pixel in the rectangle to the supplied buffer */
            *dst_buffer = frame_buffer[i][j];
            dst_buffer++;
        }

        dst_buffer += eol_padding;  /* Allow for unused space at the end of each line */
    }
}


/* Function to copy data from a supplied buffer to a rectangle in the frame buffer */
void VncCopyBuffer2Rect( void *buffer, cyg_uint16 buff_w, cyg_uint16 buff_h ,cyg_uint16 x_off, cyg_uint16 y_off,
                         cyg_uint16 x, cyg_uint16 y, cyg_uint16 width, cyg_uint16 height)
{
    int i, j;
    cyg_uint16 eol_padding = buff_w - width - x_off;

#if (BITS_PER_PIXEL == 8)
    cyg_uint8 *src_buffer = (cyg_uint8 *)buffer;
#else
    cyg_uint16 *src_buffer = (cyg_uint16 *)buffer;
#endif

    src_buffer += ((x_off + width) * y_off);  /* Allow for a y offset into supplied buffer */
    for (i = y; i < y + height; i++)
    {
        src_buffer += x_off;  /* Allow for an x offset into supplied buffer */
        for (j = x; j < x + width; j++)
        {
            /* Copy each pixel in the supplied buffer to the frame buffer */
            frame_buffer[i][j] = *src_buffer;
            src_buffer++;
        }

        src_buffer += eol_padding;  /* Allow for unused space at the end of each line */
    }

    /* Mark the required tiles for update */
    for (i = y/TILE_SIZE; i <= (y + height - 1) /TILE_SIZE; i++)
    {
        for (j = x/TILE_SIZE; j <= (x + width - 1)/TILE_SIZE; j++)
        {
            tile_updated[i][j] = 1;
        }
    }
}


/* Function to copy data from a supplied buffer to a rectangle in the frame buffer with mask */
void VncCopyBuffer2RectMask( void *buffer, cyg_uint16 buff_w, cyg_uint16 buff_h ,cyg_uint16 x_off, cyg_uint16 y_off,
                             cyg_uint16 x, cyg_uint16 y, cyg_uint16 width, cyg_uint16 height, vnc_colour_t col)
{
    int i, j;
    cyg_uint16 eol_padding = buff_w - width - x_off;

#if (BITS_PER_PIXEL == 8)
    cyg_uint8 *src_buffer = (cyg_uint8 *)buffer;
#else
    cyg_uint16 *src_buffer = (cyg_uint16 *)buffer;
#endif

    src_buffer += ((x_off + width) * y_off);  /* Allow for a y offset into supplied buffer */
    for (i = y; i < y + height; i++)
    {
        src_buffer += x_off;  /* Allow for an x offset into supplied buffer */
        for (j = x; j < x + width; j++)
        {
            /* Copy each non-mask pixel in the supplied buffer to the frame buffer */
            if (*src_buffer != col)
            {
                frame_buffer[i][j] = *src_buffer;
            }

            src_buffer++;
        }

        src_buffer += eol_padding;  /* Allow for unused space at the end of each line */
    }

    /* Mark the required tiles for update */
    for (i = y/TILE_SIZE; i <= (y + height - 1) /TILE_SIZE; i++)
    {
        for (j = x/TILE_SIZE; j <= (x + width - 1)/TILE_SIZE; j++)
        {
            tile_updated[i][j] = 1;
        }
    }
}


void VncSoundBell(void)
{
    cyg_mutex_lock(&SoundBell_lock);
    SoundBellCount++;
    cyg_mutex_unlock(&SoundBell_lock);
}


#ifdef CYGNUM_VNC_SERVER_INCLUDE_VNC_PRINTF

vnc_printf_return_t VncPrintf(MWCFONT* font, int do_print, vnc_colour_t colour, int x, int y, const char *fmt, ... )
{
    va_list args;
    static char buf[200];
    int x_pos, x_max, y_pos, y_max, char_pos;
    int char_offset, char_width;
    int ret, i, j;
    MWCFONT* sel_font;
    vnc_printf_return_t ret_vals;

    cyg_scheduler_lock();  /* Prevent other threads from running */

    va_start(args, fmt);
    ret = diag_vsprintf(buf, fmt, args);
    va_end(args);

    if (ret <= 0)
    {
        /* sprintf failed */
        ret_vals.width = 0;
        ret_vals.height = 0;
        return ret_vals;
    }

    x_pos = x;  /* Initial print positions */
    x_max = x_pos;
    y_pos = y;
    y_max = y_pos;
    char_pos = 0;

    if (font == NULL)
    {
        /* No font specified - use default font */
        sel_font = &font_winFreeSystem14x16;
    }
    else
    {
        /* Font has been specified */
        sel_font = font;
    }

    while (buf[char_pos] != 0)
    {
        /* Check for '\n' character */
        if (buf[char_pos] == '\n')
        {
            x_pos = x;
            y_pos += sel_font->height;
            char_pos++;
            continue;
        }

        /* Check for characters not if the font - set to first char */
        if (buf[char_pos] < sel_font->firstchar)
        {
            buf[char_pos] = sel_font->firstchar;
        }


        char_offset = ((cyg_uint8) buf[char_pos]) - sel_font->firstchar;

        /* Get the character width */
        if (sel_font->width != 0)
        {
            char_width = sel_font->width[char_offset];
        }
        else
        {
            char_width = sel_font->maxwidth;
        }

        if (sel_font->offset != 0)
        {
            char_offset = sel_font->offset[char_offset];
        }
        else
        {
            char_offset *= sel_font->height;
        }

        y_max = y_pos;

        if (do_print)
        {
        /* Draw the character in the frame buffer */
        for (i = char_offset; i < (char_offset + sel_font->height); i++)
        {
            if ((y_pos + i - char_offset) < CYGNUM_VNC_SERVER_FRAME_HEIGHT)
            {
                    /* This has not gone off the bottom of the frame */
                    for (j = 0; j < char_width; j++)
                    {
                        if ((x_pos + j) < CYGNUM_VNC_SERVER_FRAME_WIDTH)
                        {
                            /* This has not gone off the right edge of the frame */
                            if (sel_font->bits[i] & (0x8000 >> j))
                            {
                                /* This pixel should be drawn */
                                frame_buffer[y_pos + i - char_offset][x_pos + j] = colour;
                            }
                        }
                    }
                }
            }
        }

        x_pos += char_width;
        if (x_pos > x_max)
        {
            x_max = x_pos;
        }

        char_pos++;
    }

    /* Mark the required tiles for update */
    if (do_print)
    {
    for (i = y/TILE_SIZE; i <= (y_max + sel_font->height)/TILE_SIZE; i++)
    {
        for (j = x/TILE_SIZE; j <= x_max/TILE_SIZE; j++)
        {
            tile_updated[i][j] = 1;
            }
        }
    }

    ret_vals.width = x_max - x;
    ret_vals.height = y_max + sel_font->height - y;

    cyg_scheduler_unlock();  /* Allow other threads to run */

    return ret_vals;
}
#endif
