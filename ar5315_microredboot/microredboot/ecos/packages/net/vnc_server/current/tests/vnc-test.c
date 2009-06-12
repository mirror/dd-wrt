//==========================================================================
//
//      vnc-test.c
//
//====================================================================
// Copyright (C) 2003 Sweeney Design Ltd
//
//  This software is provided 'as-is', without any express or implied
//  warranty. Permission is granted to anyone to use this software for
//  any purpose, including commercial applications, and to alter it and
//  redistribute it freely.
//
// http://www.sweeneydesign.co.uk
//
//====================================================================
//
//  Test app for eCos VNC-server
//
//==========================================================================


#include <vnc-server.h>
#include <cyg/kernel/kapi.h>  /* Kernel API */
#include <cyg/infra/diag.h>   /* diag_printf() */
#include <cyg/error/codes.h>  /* Cyg_ErrNo, ENOENT */
#include <cyg/io/io.h>        /* cyg_io_handle_t */
#include <sys/select.h>       /* select() functions */
#include <fcntl.h>            /* open() */
#include <unistd.h>           /* read() */
#include <stdlib.h>           /* rand() */

void DrawCursor(void);
void HideCursor(void);

int cursor_x, cursor_y;  /* Cursor position */
vnc_frame_format_t *display_info;  /* Display Info */

int main()
{
    int mouse_handle = -1;
    int kbd_handle = -1;
    cyg_uint8 mouse_data[8], kbd_data[4];
    cyg_uint8 last_mouse_button = 0;
    cyg_uint32 mouse_len = 0;
    cyg_uint32 kbd_len = 0;
    vnc_printf_return_t print_area;
    cyg_uint16 text_y_pos;
    int i, j;

    cyg_uint16 bell_text_x_pos, bell_text_y_pos;  /* Bell message text position */
    cyg_uint16 bell_text_width, bell_text_height; /* Bell text message size */

    fd_set  sock_desc;  /* Set of descriptors for select */
    int max_handle;

    int bell_text_state = 0;
    char bell_message[] = "***** Click on a yellow pixel to sound the bell *****";

    /* Get information about the display */
    display_info = VncGetInfo();

    /* Initialise the VNC server display */
    VncInit(VNC_WHITE);

    /* Open the mouse device */
    mouse_handle = open("/dev/vnc_mouse", O_RDONLY | O_NONBLOCK);
    if (mouse_handle < 0)
    {
        diag_printf("Could not open mouse device: /dev/vnc_mouse\n");
    }

    /* Open the keyboard device */
    kbd_handle = open("/dev/vnc_kbd", O_RDONLY | O_NONBLOCK);
    if (kbd_handle < 0)
    {
        diag_printf("Could not open kbd device: /dev/vnc_kbd\n");
    }

    /* Draw and label 16 rectangles using the 16 defined colours */
    VncFillRect(0, 0, 75, 50, VNC_BLACK);
    VncPrintf(&font_helvR10, 1, VNC_WHITE, 1, 1, "BLACK");
    VncFillRect(75, 0, 150, 50, VNC_BLUE);
    VncPrintf(&font_helvR10, 1, VNC_WHITE, 76, 1, "BLUE");
    VncFillRect(150, 0, 225, 50, VNC_GREEN);
    VncPrintf(&font_helvR10, 1, VNC_WHITE, 151, 1, "GREEN");
    VncFillRect(225, 0, 300, 50, VNC_CYAN);
    VncPrintf(&font_helvR10, 1, VNC_WHITE, 226, 1, "CYAN");

    VncFillRect(0, 50, 75, 100, VNC_RED);
    VncPrintf(&font_helvR10, 1, VNC_WHITE, 1, 51, "RED");
    VncFillRect(75, 50, 150, 100, VNC_MAGENTA);
    VncPrintf(&font_helvR10, 1, VNC_WHITE, 76, 51, "MAGENTA");
    VncFillRect(150, 50, 225, 100, VNC_BROWN);
    VncPrintf(&font_helvR10, 1, VNC_WHITE, 151, 51, "BROWN");
    VncFillRect(225, 50, 300, 100, VNC_GRAY);
    VncPrintf(&font_helvR10, 1, VNC_WHITE, 226, 51, "GRAY");

    VncFillRect(0, 100, 75, 150, VNC_LTGRAY);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 1, 101, "LTGRAY");
    VncFillRect(75, 100, 150, 150, VNC_LTBLUE);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 76, 101, "LTBLUE");
    VncFillRect(150, 100, 225, 150, VNC_LTGREEN);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 151, 101, "LTGREEN");
    VncFillRect(225, 100, 300, 150, VNC_LTCYAN);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 226, 101, "LTCYAN");

    VncFillRect(0, 150, 75, 200, VNC_LTRED);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 1, 151, "LTRED");
    VncFillRect(75, 150, 150, 200, VNC_LTMAGENTA);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 76, 151, "LTMAGENTA");
    VncFillRect(150, 150, 225, 200, VNC_YELLOW);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 151, 151, "YELLOW");
    VncFillRect(225, 150, 300, 200, VNC_WHITE);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 226, 151, "WHITE");

    /* Fade from black to while */
    for (i = 0; i < 256; i++)
    {
        VncDrawVertLine(i, 215, 265, VNC_RGB2COL(i, i, i));
    }

    VncPrintf(&font_helvR10, 1, VNC_WHITE, 1, 216, "GRAYSCALE");

    /* Draw a strip with random coloured pixels */
    for (i = 0; i < 256; i++)
    {
        for (j = 280; j < 330; j++)
        {
            VncDrawPixel( i, j, VNC_RGB2COL(rand()%256 , rand()%256, rand()%256) );
        }
    }

    /* Write a title for the bar of random colours */
    print_area = VncPrintf(&font_helvR10, 0, VNC_BLACK, 1, 281, "RANDOM");
    VncFillRect(1, 281, 1+print_area.width , 281+print_area.height, VNC_LTGRAY);
    VncPrintf(&font_helvR10, 1, VNC_BLACK, 1, 281, "RANDOM");

    /* Report the pixel format */
    if (display_info->rgb332)
    {
        VncPrintf(0, 1, VNC_BLACK, 1, 345, "Pixel format: RGB332");
    }
    else if (display_info->rgb555)
    {
        VncPrintf(0, 1, VNC_BLACK, 1, 345, "Pixel format: RGB555");
    }
    else if (display_info->rgb565)
    {
        VncPrintf(0, 1, VNC_BLACK, 1, 345, "Pixel format: RGB565");
    }
    else if (display_info->bgr233)
    {
        VncPrintf(0, 1, VNC_BLACK, 1, 345, "Pixel format: BGR233");
    }
    else if (display_info->truecolor0888)
    {
        VncPrintf(0, 1, VNC_BLACK, 1, 345, "Pixel format: TrueColor0888");
    }
    else
    {
        VncPrintf(0, 1, VNC_BLACK, 1, 345, "Pixel format: Unknown");
    }

    /* Report the frame dimensions */
    VncPrintf(0, 1, VNC_BLACK, 1, 360, "Frame size: %d x %d", display_info->frame_width, display_info->frame_height);

    /* Write text messages using each of the available fonts */
    text_y_pos = 5;
    print_area = VncPrintf(&font_rom8x8, 1, VNC_BLACK, 350, text_y_pos, "Hello World!\nUsing rom8x8 font");
    text_y_pos += (print_area.height * 1.5);
    print_area = VncPrintf(&font_rom8x16, 1, VNC_BLUE, 350, text_y_pos, "Hello World!\nUsing rom8x16 font");
    text_y_pos += (print_area.height * 1.5);
    print_area = VncPrintf(&font_winFreeSansSerif11x13, 1, VNC_CYAN, 350, text_y_pos, "Hello World!\nUsing winFreeSansSerif11x13 font");
    text_y_pos += (print_area.height * 1.5);
    print_area = VncPrintf(0, 1, VNC_RED, 350, text_y_pos, "Hello World!\nUsing default (winFreeSystem14x16) font");
    text_y_pos += (print_area.height * 1.5);
    print_area = VncPrintf(&font_helvB10, 1, VNC_MAGENTA, 350, text_y_pos, "Hello World!\nUsing helvB10 font");
    text_y_pos += (print_area.height * 1.5);
    print_area = VncPrintf(&font_helvB12, 1, VNC_BROWN, 350, text_y_pos, "Hello World!\nUsing helvB12 font");
    text_y_pos += (print_area.height * 1.5);
    print_area = VncPrintf(&font_helvR10, 1, VNC_GREEN, 350, text_y_pos, "Hello World!\nUsing helvR10 font");
    text_y_pos += (print_area.height * 1.5);
    print_area = VncPrintf(&font_X5x7, 1, VNC_BLACK, 350, text_y_pos, "Hello World!\nUsing X5x7 font");
    text_y_pos += (print_area.height * 1.5);
    print_area = VncPrintf(&font_X6x13, 1, VNC_LTRED, 350, text_y_pos, "Hello World!\nUsing X6x13 font");

    /* Write the message about ringing the bell */
    /* Get the details of the area the text will occupy */
    print_area = VncPrintf(0, 0, VNC_BLACK, 0, 0, "%s", bell_message);
    bell_text_width = print_area.width;
    bell_text_height = print_area.height;

    /* Calculate x position to center the bell text */
    bell_text_x_pos = (display_info->frame_width - bell_text_width) / 2;

    bell_text_y_pos = 400;  /* y position always the same */

    /* Draw a background for the bell text */
    VncFillRect(bell_text_x_pos,  /* x1 */
                bell_text_y_pos,  /* y1 */
                bell_text_x_pos + bell_text_width,  /* x2 */
                bell_text_y_pos + bell_text_height, /* y2 */
                VNC_BLUE);  /* Colour */
    /* Write the text on the background */
    VncPrintf(0, 1, VNC_YELLOW, bell_text_x_pos, bell_text_y_pos, "%s", bell_message);

    /* Initialise the cursor */
    cursor_x = display_info->frame_width / 2;
    cursor_y = display_info->frame_height / 2;
    DrawCursor();

    /* Initialse the max handle variable */
    max_handle = -1;
    if (kbd_handle > max_handle)
    {
        max_handle = kbd_handle;
    }

    if (mouse_handle > max_handle)
    {
        max_handle = mouse_handle;
    }


    while(1)
    {
        FD_ZERO(&sock_desc);  /* Zero the socket set descriptor */

        if (kbd_handle)
        {
            FD_SET(kbd_handle, &sock_desc);  /* Add the keyboard handle to the set */
        }

        if (mouse_handle)
        {
            FD_SET(mouse_handle, &sock_desc);  /* Add the mouse handle to the set */
        }

        /* Use select to wait until a keyboard or mouse event occurs*/
        select(max_handle+1, &sock_desc, NULL, NULL, NULL);

        /* Check for a keyboard event */
        if (FD_ISSET(kbd_handle, &sock_desc))
        {
            kbd_len = 4;

            /* Read keyboard data until there is none left */
            while (kbd_len == 4)
                {
                /* Read 4 bytes from keyboard */
                kbd_len = read(kbd_handle, kbd_data, 4);

                if (kbd_len == 4)
        {
            if (kbd_data[1])
            {
                diag_printf("Keyboard data: keysym value 0x%x is pressed\n",
                             kbd_data[2]*256 + kbd_data[3]);
            }
            else
            {
                diag_printf("Keyboard data: keysym value 0x%x is released\n",
                             kbd_data[2]*256 + kbd_data[3]);
            }
                }
            }
        }

        /* Check for a mouse event */
        if (FD_ISSET(mouse_handle, &sock_desc))
        {
            mouse_len = 8;

            /* Read mouse data until there is none left */
            while (mouse_len == 8)
            {
                /* Read 8 bytes from mouse */
                mouse_len = read(mouse_handle, mouse_data, 8);

                if (mouse_len == 8)
                {
                    HideCursor();  /* Hide the old cursor */
                    cursor_x = mouse_data[2]*256 + mouse_data[3];
                    cursor_y = mouse_data[4]*256 + mouse_data[5];

                    if (mouse_data[1] && !last_mouse_button)
                    {
                        /* Ring bell and change colours of bell message text if the */
                        /* mouse button is pressed on a yellow pixel                */
                        if (VncReadPixel(mouse_data[2]*256 + mouse_data[3],
                                         mouse_data[4]*256 + mouse_data[5]) == VNC_YELLOW)
                        {
                            VncSoundBell();  /* Ring bell on the client */

                            if (bell_text_state)
                            {
                                bell_text_state = 0;
                                /* Draw a background for the text */
                                VncFillRect(bell_text_x_pos,  /* x1 */
                                            bell_text_y_pos,  /* y1 */
                                            bell_text_x_pos + bell_text_width,  /* x2 */
                                            bell_text_y_pos + bell_text_height, /* y2 */
                                            VNC_BLUE);  /* Colour */
                                /* Write the text on the background */
                                print_area = VncPrintf(0, 1, VNC_YELLOW, bell_text_x_pos, bell_text_y_pos, "%s", bell_message);

                            }
                            else
                            {
                                bell_text_state = 1;
                                /* Draw a background for the text */
                                VncFillRect(bell_text_x_pos,  /* x1 */
                                            bell_text_y_pos,  /* y1 */
                                            bell_text_x_pos + bell_text_width,  /* x2 */
                                            bell_text_y_pos + bell_text_height, /* y2 */
                                            VNC_YELLOW);  /* Colour */
                                /* Write the text on the background */
                                print_area = VncPrintf(0, 1, VNC_BLUE, bell_text_x_pos, bell_text_y_pos, "%s", bell_message);
                            }
                        }
                    }

                    last_mouse_button = mouse_data[1];  /* Save mouse button data */
                    DrawCursor();  /* Draw the new cursor */
                }
            }

        }
    }

    return 1;
}


vnc_colour_t under_cursor[16][16];  /* Buffer to hold display area under cursor */

    /* Buffer holding cursor data */
#define BLK VNC_BLACK
#define TRN VNC_BLUE
#define WHT VNC_WHITE
#define GRY VNC_LTGRAY
vnc_colour_t cursor[16][16] = {{BLK, BLK, BLK, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN},
                               {BLK, GRY, GRY, BLK, BLK, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN},
                               {BLK, WHT, GRY, GRY, GRY, BLK, BLK, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN},
                               {TRN, BLK, WHT, GRY, GRY, GRY, GRY, BLK, BLK, TRN, TRN, TRN, TRN, TRN, TRN, TRN},
                               {TRN, BLK, WHT, WHT, GRY, GRY, GRY, GRY, GRY, BLK, BLK, TRN, TRN, TRN, TRN, TRN},
                               {TRN, TRN, BLK, WHT, WHT, GRY, GRY, GRY, GRY, GRY, GRY, BLK, BLK, TRN, TRN, TRN},
                               {TRN, TRN, BLK, WHT, WHT, WHT, GRY, GRY, GRY, GRY, GRY, GRY, GRY, BLK, TRN, TRN},
                               {TRN, TRN, TRN, BLK, WHT, WHT, WHT, GRY, GRY, BLK, BLK, BLK, BLK, BLK, TRN, TRN},
                               {TRN, TRN, TRN, BLK, WHT, WHT, WHT, WHT, GRY, GRY, BLK, TRN, TRN, TRN, TRN, TRN},
                               {TRN, TRN, TRN, TRN, BLK, WHT, WHT, BLK, WHT, GRY, GRY, BLK, TRN, TRN, TRN, TRN},
                               {TRN, TRN, TRN, TRN, BLK, WHT, WHT, BLK, BLK, WHT, GRY, GRY, BLK, TRN, TRN, TRN},
                               {TRN, TRN, TRN, TRN, TRN, BLK, WHT, BLK, TRN, BLK, WHT, GRY, GRY, BLK, TRN, TRN},
                               {TRN, TRN, TRN, TRN, TRN, BLK, WHT, BLK, TRN, TRN, BLK, WHT, GRY, GRY, BLK, TRN},
                               {TRN, TRN, TRN, TRN, TRN, TRN, BLK, TRN, TRN, TRN, TRN, BLK, WHT, GRY, GRY, BLK},
                               {TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, BLK, WHT, BLK, TRN},
                               {TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, TRN, BLK, TRN, TRN}};
#undef BLK
#undef TRN
#undef WHT
#undef GRY

void DrawCursor(void)
{
    int rect_width, rect_height;

    if (cursor_x >= display_info->frame_width)
    {
        /* Cursor is off the screen */
        return;
    }
    else if ((cursor_x + 16) >= display_info->frame_width)
    {
        /* Cursor is partially off the screen */
        rect_width = display_info->frame_width - cursor_x;
    }
    else
    {
        /* Cursor is fully on the screen */
        rect_width = 16;
    }

    if (cursor_y >= display_info->frame_height)
    {
        /* Cursor is off the screen */
        return;
    }
    else if ((cursor_y + 16) >= display_info->frame_height)
    {
        /* Cursor is partially off the screen */
        rect_height = display_info->frame_height - cursor_y;
    }
    else
    {
        /* Cursor is fully on the screen */
        rect_height = 16;
    }

    /* Save the area under the cursor */
    VncCopyRect2Buffer(cursor_x, cursor_y, rect_width, rect_height, under_cursor, 16, 16, 0, 0);

    /* Draw the new cursor */
    VncCopyBuffer2RectMask(cursor, 16, 16, 0, 0, cursor_x, cursor_y, rect_width, rect_height, VNC_BLUE);
}


void HideCursor(void)
{
    int rect_width, rect_height;

    if (cursor_x >= display_info->frame_width)
    {
        /* Cursor is off the screen */
        return;
    }
    else if ((cursor_x + 16) >= display_info->frame_width)
    {
        /* Cursor is partially off the screen */
        rect_width = display_info->frame_width - cursor_x;
    }
    else
    {
        /* Cursor is fully on the screen */
        rect_width = 16;
    }

    if (cursor_y >= display_info->frame_height)
    {
        /* Cursor is off the screen */
        return;
    }
    else if ((cursor_y + 16) >= display_info->frame_height)
    {
        /* Cursor is partially off the screen */
        rect_height = display_info->frame_height - cursor_y;
    }
    else
    {
        /* Cursor is fully on the screen */
        rect_height = 16;
    }

    /* Restore the saved area under the cursor */
    VncCopyBuffer2Rect(under_cursor, 16, 16, 0, 0, cursor_x, cursor_y, rect_width, rect_height);
}
