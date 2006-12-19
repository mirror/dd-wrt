/*****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       printers.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  14/06/04
//
//  Description:
//      TODO
//
*****************************************************************************/
#ifndef _PRINTERS_H_
#define _PRINTERS_H_

#define PRINTER_NAME_LEN 80
#define DEVICE_LEN 80

// Printer state

#define PRINTER_OPEN              0
#define PRINTER_CLOSED            1
#define PRINTER_PRINTING_WRITE    2
#define PRINTER_PRINTING_WAIT     3

#define BUF_SIZE          2048

typedef struct _ipp_job { int fd;
                          char *name;
                          int id;
                          int size; } Job;
                                                                                
typedef struct _ipp_jobs { Job job;
                           struct _ipp_jobs *next; } Jobs;
                                                                                
typedef struct _printer { char name[PRINTER_NAME_LEN];
                          char make[PRINTER_NAME_LEN];
                          char device[DEVICE_LEN];
                          time_t create_time;
                          struct _ipp_jobs *jobs;
                          int state;
                          int fd;
                          int buf_ptr;
                          int used;
                          char buffer[BUF_SIZE];
                        } Printer;

int init_printers( char *filename );
Printer *get_printer( char *name );
Job *printer_add_job( Printer *printer, int fd, char *name, int size );
Job *printer_list_job( Printer *printer, Job *prev );


#endif
