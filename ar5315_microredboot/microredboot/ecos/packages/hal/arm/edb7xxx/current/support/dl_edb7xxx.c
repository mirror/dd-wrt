//****************************************************************************
//
// DOWNLOAD.C - Automates the download of code into the flash on the
//              Cirrus Logic EDB7XXX boards.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
// Adapted for Linux by Red Hat, Inc.
//   Renamed 'dl_edb7xxx' to indicate support for all Cirrus Logic eval boards.
//
//****************************************************************************
#include <stdio.h>
#include "bootcode.h"

#ifndef _WIN32
#define CHAR_READY_IS_RELIABLE
#endif

#define DEFAULT_PORT "/dev/ttyS0"
extern char _ReceiveChar(long);
extern void _SendChar(long, char);
extern void _SetBaud(long, long);
extern int  _CharRead(long port);
extern void _WaitForOutputReady(long port);
extern long _OpenPort(char *);
extern int  _CharReady(long port);

#define ReceiveChar _ReceiveChar
#define SendChar    _SendChar
#define SetBaud     _SetBaud
#define CharRead    _CharRead
#define WaitForOutputEmpty _WaitForOutputEmpty
#define OpenPort    _OpenPort
#define CharReady   _CharReady

//****************************************************************************
//
// WaitFor waits until a specific character is read from the comm port.
//
//****************************************************************************
void
WaitFor(long lPort, char cWaitChar)
{
    char cChar;

    //
    // Wait until we read a specific character from the comm port.
    //
    while(1)
    {
        //
        // Read a character.
        //
        cChar = ReceiveChar(lPort);

        //
        // Stop waiting if we received the character.
        //
        if(cChar == cWaitChar)
        {
            break;
        }
    }
}

//****************************************************************************
//
// This program waits for the '<' character from the boot ROM, sends the boot
// code, waits for the '>' from the boot ROM, waits for the '?' from the boot
// code, changes the serial port rate (preferably to 115200), downloads the
// user data file, and then prints out progress status as the boot code writes
// the user data file to the flash.
//
//****************************************************************************
void
main(int argc, char *argv[])
{
    long lPort;
    long lRate = 38400;
    long lFileSize, lIdx;
    char cChar, cFirstChar, *pcFile, cRateChar;
    FILE *pFile;

    //
    // Make sure that a filename was specified.
    //
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename> {<baud rate> {<comm port>}}\n", argv[0]);
        return;
    }

    //
    // If a baud rate was specified, then read it and make sure it is valid.
    //
    if(argc > 2)
    {
        lRate = atoi(argv[2]);
        if((lRate != 9600) && (lRate != 19200) && (lRate != 28800) &&
           (lRate != 38400) && (lRate != 57600) && (lRate != 115200))
        {
            fprintf(stderr, "Invalid baud rate: %d(%s).\n", lRate, argv[2]);
            return;
        }
    }

    //
    // If a comm port was specified, then read it and make sure it is valid.
    //
    if(argc > 3)
    {
        lPort = OpenPort(argv[3]);
        if (lPort < 0)
        {
            fprintf(stderr, "Can't open port: %s\n", argv[3]);
            return;
        }
    } else 
    {
        lPort = OpenPort(DEFAULT_PORT);
        if (lPort < 0)
        {
            fprintf(stderr, "Can't open port: %s\n", DEFAULT_PORT);
            return;
        }
    }

    //
    // Open the file to be downloaded.
    //
    pFile = fopen(argv[1], "rb");
    if(!pFile)
    {
        fprintf(stderr, "Could not open file '%s'.\n", argv[1]);
        return;
    }

    //
    // Get the size of the file.
    //
    fseek(pFile, 0, SEEK_END);
    lFileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    //
    // Allocate memory to hold the file contents.
    //
    pcFile = (char *)malloc(lFileSize);
    if(!pcFile)
    {
        fprintf(stderr, "Failed to allocate memory for the file.\n");
        return;
    }

    //
    // Read the contents of the file into memory.
    //
    if(fread(pcFile, 1, lFileSize, pFile) != lFileSize)
    {
        fprintf(stderr, "Failed to read file '%s'.\n", argv[1]);
        return;
    }

    //
    // Close the file.
    //
    fclose(pFile);

    //
    // Get the baud rate divisor for the given baud rate.
    //
    SetBaud(lPort, 9600);
    switch(lRate)
    {
        case 9600:
        {
            cRateChar = '0';
            break;
        }

        case 19200:
        {
            cRateChar = '1';
            break;
        }

        case 28800:
        {
            cRateChar = '2';
            break;
        }

        case 38400:
        {
            cRateChar = '3';
            break;
        }

        case 57600:
        {
            cRateChar = '4';
            break;
        }

        case 115200:
        {
            cRateChar = '5';
            break;
        }
    }

    //
    // Empty out the input queue.
    //
#ifdef CHAR_READY_IS_RELIABLE
    while(CharReady(lPort))
    {
        cChar = ReceiveChar(lPort);
    }
#endif

    //
    // Tell the user to reset the board.
    //
    fprintf(stderr, "Waiting for the board to wakeup...");

    //
    // Wait until we read a '<' from the comm port.
    //
    WaitFor(lPort, '<');

    //
    // Tell the user that we are downloading the boot code.
    //
    fprintf(stderr, "\nDownloading boot code...(  0%%)");

    //
    // Write the boot code to the comm port.
    //
    for(lIdx = 0; lIdx < 2048; lIdx++)
    {
        //
        // Write this character.
        //
        SendChar(lPort, pcBoot[lIdx]);

        //
        // Periodically print out our progress.
        //
        if((lIdx & 127) == 127)
        {
            fprintf(stderr, "\b\b\b\b\b%3d%%)", ((lIdx + 1) * 100) / 2048);
        }
    }
            
    //
    // Wait until we read a '>' from the comm port.
    //
    WaitFor(lPort, '>');

    //
    // Wait until we read a '?' from the comm port.
    //
    WaitFor(lPort, '?');

    //
    // Tell the boot code to switch to the desired baud rate.
    //
    SendChar(lPort, 'B');
    SendChar(lPort, cRateChar);

    //
    // Wait until the output buffer is empty.
    //
    WaitForOutputEmpty();

    //
    // Switch our baud rate to the desired rate.
    //
    SetBaud(lPort, lRate);

    //
    // Send a '-' character until we receive back a '?' character.
    //
    while(1)
    {
        //
        // Send a '-' character.
        //
        SendChar(lPort, '-');

        //
        // Wait a little bit.
        //
        for(lIdx = 0; lIdx < 1024 * 1024; lIdx++)
        {
        }

        //
        // See if there is a character waiting to be read.
        //
#ifdef CHAR_READY_IS_RELIABLE
        if(CharReady(lPort))
#else
        if (1)
#endif
        {
            //
            // Read the character.
            //
            cChar = ReceiveChar(lPort);

            //
            // Quit waiting if this is a '?'.
            //
            if(cChar == '?')
            {
                break;
            }
        }
    }

    //
    // Empty out the input queue.
    //
#ifdef CHAR_READY_IS_RELIABLE
    while(CharReady(lPort))
    {
        cChar = ReceiveChar(lPort);
    }
#endif

    //
    // Send the program flash command.
    //
    SendChar(lPort, 'F');

    //
    // We always program the flash at location 0.
    //
    SendChar(lPort, 0);
    SendChar(lPort, 0);
    SendChar(lPort, 0);
    SendChar(lPort, 0);

    //
    // Send the length of the data file.
    //
    SendChar(lPort, (char)(lFileSize & 0xFF));
    SendChar(lPort, (char)((lFileSize >> 8) & 0xFF));
    SendChar(lPort, (char)((lFileSize >> 16) & 0xFF));
    SendChar(lPort, (char)((lFileSize >> 24) & 0xFF));

    //
    // Tell the user that we are downloading the file data.
    //
    fprintf(stderr, "\nDownloading file data...(  0%%)");

    //
    // Send the actual data in the file.
    //
    for(lIdx = 0; lIdx < lFileSize; lIdx++)
    {
        //
        // Send this byte.
        //
        SendChar(lPort, pcFile[lIdx]);

        //
        // Periodically print out our progress.
        //
        if((lIdx & 127) == 127)
        {
            fprintf(stderr, "\b\b\b\b\b%3d%%)", ((lIdx + 1) * 100) / lFileSize);
        }
    }

    //
    // Tell the user that we are erasing the flash.
    //
    fprintf(stderr, "\nErasing the flash...(  0%%)");

    //
    // Wait until the flash has been erased.
    //
    cFirstChar = cChar = ReceiveChar(lPort);
    while(cChar != '1')
    {
        //
        // Print out our progress.
        //
        fprintf(stderr, "\b\b\b\b\b%3d%%)",
               ((cFirstChar - cChar + 1) * 100) / (cFirstChar - '0'));
        fprintf(stderr, "%c\n", cChar);

        //
        // Read a character from the boot code.
        //
        cChar = ReceiveChar(lPort);
    }

    //
    // Tell the user that we are programming the flash.
    //
    fprintf(stderr, "\nProgramming the flash...(  0%%)");

    //
    // Wait until the flash has been programmed.
    //
    lIdx = 0;
    while(1)
    {
        //
        // Read a character from the boot code.
        //
        cChar = ReceiveChar(lPort);

        //
        // If the character is a '?', then we are done.
        //
        if(cChar == '?')
        {
            break;
        }

        //
        // Print out our progress.
        //
        fprintf(stderr, "\b\b\b\b\b%3d%%)",
               (++lIdx * 100) / ((lFileSize + 1023) / 1024));
    }

    //
    // Tell the user we are done.
    //
    fprintf(stderr, "\nSuccessfully downloaded '%s'.\n", argv[1]);
}

