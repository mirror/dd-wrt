/**
 * file usbprivatetypes.h
 *
 * author Intel Corporation
 * date 30-OCT-2001
 *
 * This file contains the private USB Driver data types
 *
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#ifndef usbprivatetypes_H
#define usbprivatetypes_H

#include "usbbasictypes.h"
#include "usbconstants.h"
#include "usbprivateconstants.h"
#include "usbdeviceparam.h"

typedef struct /* USBDeviceCounters */
{
    UINT32 frames;   /* number of processed USB frames */

    UINT32 irqCount; /* number of serviced IRQs */

    UINT32 Rx;       /* received packets */
    UINT32 Tx;       /* transmitted packets */
    UINT32 DRx;      /* dropped received packets */
    UINT32 DTx;      /* dropped transmitted packets */

    UINT32 bytesRx;  /* transmitted bytes */
    UINT32 bytesTx;  /* received bytes */

    UINT32 setup;    /* number of serviced SETUP transactions */
} USBDeviceCounters;

typedef struct /* USBEndpointCounters */
{
    UINT32 Rx;  /* number of received packets */
    UINT32 Tx;  /* number of transmitted packets */
    UINT32 DRx; /* number of dropped received packets */
    UINT32 DTx; /* number of dropped transmitted packets */

    UINT32 fifoUnderruns; /* FIFO underruns */
    UINT32 fifoOverflows; /* FIFO overflows */

    UINT32 bytesRx; /* bytes received */
    UINT32 bytesTx; /* bytes transmitted */

    UINT32 irqCount; /* number of serviced IRQs for endpoint */
} USBEndpointCounters;

typedef struct /* USBDataQueue */
{
    IX_USB_MBLK *base[MAX_QUEUE_SIZE];
    UINT32 head; /* offset in elements from base */
    UINT32 len;  /* length in elements */
} USBDataQueue;

typedef struct /* EPStatusData */
{
    USBDevice *device;                  /* Parent device */
   
    USBEndpointNumber endpointNumber;   /* Endpoint number */

    USBEndpointDirection direction;     /* Endpoint direction */

    USBEndpointType type;               /* Endpoint type */

    UINT32 fifoSize;                    /* FIFO depth */

#ifdef IX_USB_DMA

    BOOL dmaEnabled;                    /* DMA enabled for this endpoint? */
   
    UINT32 dmaSize;                     /* DMA buffer depth */

#endif /* IX_USB_DMA */
    
    IX_USB_MBLK *currentBuffer;         /* Current send or receive buffer */
  
    UINT32 currentOffset;               /* Current fill offset in buffer */
    
    UINT32 currentTransferSize;         /* Current transfer chunk size (packet, DMA buffer or less) */

    BOOL transferAllowed;               /* FIFO semaphore */

#ifdef IX_USB_HAS_TIMESTAMP_CHECKS

    UINT32 lastTimestamp;               /* Last sent/received packet timestamp */

#endif /* IX_USB_HAS_TIMESTAMP_CHECKS */

    USBDataQueue queue;                 /* Data queue for endpoint */
  
    USBEndpointCounters counters;       /* Endpoint counters */

    volatile UINT32 *UDCCS;             /* control/status register */
    volatile UINT32 *UDDR;              /* data register */
    volatile UINT32 *UBCR;              /* FIFO byte-count register */
} EPStatusData;

typedef struct /* EP0ControlData */
{
    USBToken currentToken;                  /* Last received token type */
    
    EP0State state;                         /* Endpoint 0 state machine */
    
    USBControlTransfer transferType;        /* Current transfer type */
    
    char setupBuffer[SETUP_PACKET_SIZE];    /* 8-byte buffer for SETUP packets */
    
    UINT16 expected, transferred;           /* Expected and actual transfer length */
} EP0ControlData;

typedef struct /* USBEventProcessor */
{
    USBEventCallback eventCallback;     /* Reset, suspend, resume callback */
    
    USBEventMap eventMap;               /* Event map */
    
    USBSetupCallback setupCallback;     /* SETUP packet receive function */
    
    USBReceiveCallback receiveCallback; /* Data receive function */
} USBEventProcessor;

/*  UDC Registers */
typedef struct  /* UDCRegisters */
{
    volatile UINT32 UDCCR;
    volatile UINT32	RESERVED[3];
    volatile UINT32	UDCCS0;
    volatile UINT32	UDCCS1;
    volatile UINT32	UDCCS2;
    volatile UINT32	UDCCS3;
    volatile UINT32	UDCCS4;
    volatile UINT32	UDCCS5;
    volatile UINT32	UDCCS6;
    volatile UINT32	UDCCS7;
    volatile UINT32	UDCCS8;
    volatile UINT32	UDCCS9;
    volatile UINT32	UDCCS10;
    volatile UINT32	UDCCS11;
    volatile UINT32	UDCCS12;
    volatile UINT32	UDCCS13;
    volatile UINT32	UDCCS14;
    volatile UINT32	UDCCS15;
    volatile UINT32	UICR0;
    volatile UINT32	UICR1;
    volatile UINT32	USIR0;
    volatile UINT32	USIR1;
    volatile UINT32	UFNHR;
    volatile UINT32	UFNLR;
    volatile UINT32	UBCR2;
    volatile UINT32	UBCR4;
    volatile UINT32	UBCR7;
    volatile UINT32	UBCR9;
    volatile UINT32	UBCR12;
    volatile UINT32	UBCR14;
    volatile UINT32	UDDR0;			
    volatile UINT32	RESERVED0[7];
    volatile UINT32	UDDR5;			
    volatile UINT32	RESERVED5[7];
    volatile UINT32	UDDR10;		    
    volatile UINT32	RESERVED10[7];	
    volatile UINT32	UDDR15;			
    volatile UINT32	RESERVED15[7];
    volatile UINT32	UDDR1;			
    volatile UINT32	RESERVED1[31];	
    volatile UINT32	UDDR2;			
    volatile UINT32	RESERVED2[31];	
    volatile UINT32	UDDR3;			
    volatile UINT32	RESERVED3[127];	
    volatile UINT32	UDDR4;			
    volatile UINT32	RESERVED4[127];	
    volatile UINT32	UDDR6;			
    volatile UINT32	RESERVED6[31];	
    volatile UINT32	UDDR7;			
    volatile UINT32	RESERVED7[31];	
    volatile UINT32	UDDR8;			
    volatile UINT32	RESERVED8[127];	
    volatile UINT32	UDDR9;			
    volatile UINT32	RESERVED9[127];	
    volatile UINT32	UDDR11;			
    volatile UINT32	RESERVED11[31];	
    volatile UINT32	UDDR12;			
    volatile UINT32	RESERVED12[31];	
    volatile UINT32	UDDR13;			
    volatile UINT32	RESERVED13[127];
    volatile UINT32	UDDR14;			
} UDCRegisters;

typedef struct /* USBDeviceContext */
{
    UINT32 checkPattern;                      /* Check pattern for verifying the context */

    USBDevice *device;                        /* Reference to parent USBDevice structure */
    
    UDCRegisters *registers;                  /* Device registers */
    
    EP0ControlData ep0ControlData;            /* Endpoint 0 control data */
    
    EPStatusData epStatusData[NUM_ENDPOINTS]; /* Endpoint data array */
    
    USBEventProcessor eventProcessor;         /* Event callbacks and masks */

    USBDeviceCounters counters;	              /* Global (device) counters */

    BOOL enabled;                             /* Enabled/disabled indicator */
} USBDeviceContext;

#endif /* usbprivatetypes_H */
