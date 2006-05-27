/*
 *  Copyright (c) 2000-2002 Atheros Communications, Inc., All Rights Reserved
 *
 */

#ident "$Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/ratectrl/ar5212Phy.c#3 $"

#ifdef __FreeBSD__
#include <dev/ath/ath_rate/atheros/ratectrl.h>
#else
#include "ratectrl.h"
#endif

#define MULTI_RATE_RETRY_ENABLE
#ifdef MULTI_RATE_RETRY_ENABLE
#define SHORT_PRE 1
#define LONG_PRE 0

#define RATEMBPS_TO_RATECODE(_rate, _mode, _shortPreamble)\
    (((_rate) == 0.25) ? 0x3 : \
    (((_rate) == 0.5) ? 0x7 : \
    (((_rate) == 1) ? (((_mode) == WIRELESS_MODE_XR) ? 0x2 : 0x1b) : \
    (((_rate) == 2) ? (((_mode) == WIRELESS_MODE_XR) ? 0x6 : \
                           (((_shortPreamble)) ? 0x1a+0x04 : 0x1a)) : \
    (((_rate) == 3) ? 0x1 : \
    (((_rate) == 5.5) ? (((_shortPreamble)) ? 0x19+0x04 : 0x19) : \
    (((_rate) == 11) ? (((_shortPreamble)) ? 0x18+0x04 : 0x18) : \
    (((_rate) == 6) ? 0xb : \
    (((_rate) == 9) ? 0xf : \
    (((_rate) == 12) ? 0xa : \
    (((_rate) == 18) ? 0xe : \
    (((_rate) == 24) ? 0x9 : \
    (((_rate) == 36) ? 0xd : \
    (((_rate) == 48) ? 0x8 : \
    (((_rate) == 54) ? 0xc : 0)))))))))))))))


#define TXCNTL_WD4(_txRetries0, _txRetries1, _txRetries2, _txRetries3)\
    ((1 << 15) + ((_txRetries0) << 16) + ((_txRetries1) << 20) + ((_txRetries2) << 24) + ((_txRetries3) << 28))

#define TXCNTL_WD5(_txRate0, _txRate1, _txRate2, _txRate3, _mode, _shortPreamble)\
    ((RATEMBPS_TO_RATECODE((_txRate3), (_mode), (_shortPreamble)) << 15) + \
     (RATEMBPS_TO_RATECODE((_txRate2), (_mode), (_shortPreamble)) << 10) + \
     (RATEMBPS_TO_RATECODE((_txRate1), (_mode), (_shortPreamble)) << 5) + \
     (RATEMBPS_TO_RATECODE((_txRate0), (_mode), (_shortPreamble))))


#define A_RETRY_SCHEDULE_6  {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(6,6,6,6,WIRELESS_MODE_11a,LONG_PRE)},\
                            {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(6,6,6,6,WIRELESS_MODE_11a,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,4,0,0),TXCNTL_WD5(6,6,6,6,WIRELESS_MODE_11a,LONG_PRE)},\
			                {TXCNTL_WD4(1,4,0,0),TXCNTL_WD5(6,6,6,6,WIRELESS_MODE_11a,SHORT_PRE)}
#define A_RETRY_SCHEDULE_9  {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(9,6,6,6,WIRELESS_MODE_11a,LONG_PRE)},\
		                    {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(9,6,6,6,WIRELESS_MODE_11a,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(9,6,6,6,WIRELESS_MODE_11a,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(9,6,6,6,WIRELESS_MODE_11a,SHORT_PRE)}
#define A_RETRY_SCHEDULE_12 {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(12,6,6,6,WIRELESS_MODE_11a,LONG_PRE)},\
		                    {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(12,6,6,6,WIRELESS_MODE_11a,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(12,9,6,6,WIRELESS_MODE_11a,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(12,9,6,6,WIRELESS_MODE_11a,SHORT_PRE)}
#define A_RETRY_SCHEDULE_18 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(18,12,6,6,WIRELESS_MODE_11a,LONG_PRE)},\
		                    {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(18,12,6,6,WIRELESS_MODE_11a,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(18,12,6,6,WIRELESS_MODE_11a,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(18,12,6,6,WIRELESS_MODE_11a,SHORT_PRE)}
#define A_RETRY_SCHEDULE_24 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_11a,LONG_PRE)},\
		                    {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_11a,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_11a,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_11a,SHORT_PRE)}
#define A_RETRY_SCHEDULE_36 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(36,24,18,6,WIRELESS_MODE_11a,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(36,24,18,6,WIRELESS_MODE_11a,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(36,24,18,12,WIRELESS_MODE_11a,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(36,24,18,12,WIRELESS_MODE_11a,SHORT_PRE)}
#define A_RETRY_SCHEDULE_48 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(48,36,24,12,WIRELESS_MODE_11a,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(48,36,24,12,WIRELESS_MODE_11a,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(48,36,24,18,WIRELESS_MODE_11a,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(48,36,24,18,WIRELESS_MODE_11a,SHORT_PRE)}
#define A_RETRY_SCHEDULE_54 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_11a,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_11a,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_11a,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_11a,SHORT_PRE)}
#define G_RETRY_SCHEDULE_1  {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(1,1,1,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(1,1,1,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,4,0,0),TXCNTL_WD5(1,1,1,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,4,0,0),TXCNTL_WD5(1,1,1,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_2  {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(2,1,1,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(2,1,1,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(2,1,1,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(2,1,1,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_5_5 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(5.5,2,1,1,WIRELESS_MODE_11g,LONG_PRE)},\
		                    {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(5.5,2,1,1,WIRELESS_MODE_11g,SHORT_PRE)},\
        		            {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(5.5,2,1,1,WIRELESS_MODE_11g,LONG_PRE)},\
		            	    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(5.5,2,1,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_11 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(11,5.5,2,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(11,5.5,2,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(11,5.5,2,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(11,5.5,2,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_6  {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(6,5.5,2,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(6,5.5,2,1,WIRELESS_MODE_11g,SHORT_PRE)},\
        		            {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(6,5.5,2,1,WIRELESS_MODE_11g,LONG_PRE)},\
		            	    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(6,5.5,2,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_9  {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(9,6,5.5,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(9,6,5.5,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(9,6,5.5,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(9,6,5.5,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_12 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(12,11,5.5,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(12,11,5.5,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(12,11,5.5,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(12,11,5.5,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_18 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(18,12,11,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(18,12,11,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(18,12,11,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(18,12,11,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_24 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(24,18,12,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(24,18,12,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(24,18,12,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(24,18,12,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_36 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(36,24,18,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(36,24,18,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(36,24,18,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(36,24,18,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_48 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(48,36,24,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(48,36,24,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(48,36,24,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(48,36,24,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define G_RETRY_SCHEDULE_54 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(54,48,36,1,WIRELESS_MODE_11g,LONG_PRE)},\
        		            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(54,48,36,1,WIRELESS_MODE_11g,SHORT_PRE)},\
		                    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(54,48,36,1,WIRELESS_MODE_11g,LONG_PRE)},\
			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(54,48,36,1,WIRELESS_MODE_11g,SHORT_PRE)}
#define TURBO_RETRY_SCHEDULE_6  {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(6,6,6,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(6,6,6,6,WIRELESS_MODE_TURBO,SHORT_PRE)},\
                                {TXCNTL_WD4(1,4,0,0),TXCNTL_WD5(6,6,6,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(1,4,0,0),TXCNTL_WD5(6,6,6,6,WIRELESS_MODE_TURBO,SHORT_PRE)}
#define TURBO_RETRY_SCHEDULE_9  {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(9,6,6,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(9,6,6,6,WIRELESS_MODE_TURBO,SHORT_PRE)},\
                                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(9,6,6,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(9,6,6,6,WIRELESS_MODE_TURBO,SHORT_PRE)}
#define TURBO_RETRY_SCHEDULE_12 {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(12,6,6,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(12,6,6,6,WIRELESS_MODE_TURBO,SHORT_PRE)},\
                                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(12,9,6,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(12,9,6,6,WIRELESS_MODE_TURBO,SHORT_PRE)}
#define TURBO_RETRY_SCHEDULE_18 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(18,12,6,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(18,12,6,6,WIRELESS_MODE_TURBO,SHORT_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(18,12,6,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(18,12,6,6,WIRELESS_MODE_TURBO,SHORT_PRE)}
#define TURBO_RETRY_SCHEDULE_24 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_TURBO,SHORT_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_TURBO,SHORT_PRE)}
#define TURBO_RETRY_SCHEDULE_36 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(36,24,18,6,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(36,24,18,6,WIRELESS_MODE_TURBO,SHORT_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(36,24,18,12,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(36,24,18,12,WIRELESS_MODE_TURBO,SHORT_PRE)}
#define TURBO_RETRY_SCHEDULE_48 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(48,36,24,12,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(48,36,24,12,WIRELESS_MODE_TURBO,SHORT_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(48,36,24,18,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(48,36,24,18,WIRELESS_MODE_TURBO,SHORT_PRE)}
#define TURBO_RETRY_SCHEDULE_54 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_TURBO,SHORT_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_TURBO,LONG_PRE)},\
                                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_TURBO,SHORT_PRE)}  
#define XR_RETRY_SCHEDULE_0_25 {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(0.25,0.25,0.25,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
                            {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(0.25,0.25,0.25,0.25,WIRELESS_MODE_XR,SHORT_PRE)},\
 			                {TXCNTL_WD4(1,4,0,0),TXCNTL_WD5(0.25,0.25,0.25,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
 			                {TXCNTL_WD4(1,4,0,0),TXCNTL_WD5(0.25,0.25,0.25,0.25,WIRELESS_MODE_XR,SHORT_PRE)}       
#define XR_RETRY_SCHEDULE_0_5 {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(0.5,0.25,0.25,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
                            {TXCNTL_WD4(3,3,0,0),TXCNTL_WD5(0.5,0.25,0.25,0.25,WIRELESS_MODE_XR,SHORT_PRE)},\
 			                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(0.5,0.25,0.25,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
 			                {TXCNTL_WD4(1,3,3,0),TXCNTL_WD5(0.5,0.25,0.25,0.25,WIRELESS_MODE_XR,SHORT_PRE)}        
#define XR_RETRY_SCHEDULE_1 {TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(1,0.5,0.25,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
                			{TXCNTL_WD4(4,3,4,0),TXCNTL_WD5(1,0.5,0.25,0.25,WIRELESS_MODE_XR,SHORT_PRE)},\
 			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(1,0.5,0.25,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
 			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(1,0.5,0.25,0.25,WIRELESS_MODE_XR,SHORT_PRE)}       
#define XR_RETRY_SCHEDULE_2 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(2,1,0.5,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
            			    {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(2,1,0.5,0.25,WIRELESS_MODE_XR,SHORT_PRE)},\
 			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(2,1,0.5,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
             			    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(2,1,0.5,0.25,WIRELESS_MODE_XR,SHORT_PRE)}       
#define XR_RETRY_SCHEDULE_3 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(3,1,0.5,0.25,WIRELESS_MODE_XR,LONG_PRE)},\
			                {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(3,1,0.5,0.25,WIRELESS_MODE_XR,SHORT_PRE)},\
             			    {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(3,2,1,0.5,WIRELESS_MODE_XR,LONG_PRE)},\
 			                {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(3,2,1,0.5,WIRELESS_MODE_XR,SHORT_PRE)}    
#define XR_RETRY_SCHEDULE_6 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(6,2,1,0.5,WIRELESS_MODE_XR,LONG_PRE)},\
                            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(6,2,1,0.5,WIRELESS_MODE_XR,SHORT_PRE)},\
                            {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(6,3,1,0.5,WIRELESS_MODE_XR,LONG_PRE)},\
                            {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(6,3,1,0.5,WIRELESS_MODE_XR,SHORT_PRE)}      
#define XR_RETRY_SCHEDULE_9 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(9,6,3,1,WIRELESS_MODE_XR,LONG_PRE)},\
                            {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(9,6,3,1,WIRELESS_MODE_XR,SHORT_PRE)},\
                            {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(9,6,2,1,WIRELESS_MODE_XR,LONG_PRE)},\
                            {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(9,6,2,1,WIRELESS_MODE_XR,SHORT_PRE)}      
#define XR_RETRY_SCHEDULE_12 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(12,6,3,1,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(12,6,3,1,WIRELESS_MODE_XR,SHORT_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(12,9,6,3,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(12,9,6,3,WIRELESS_MODE_XR,SHORT_PRE)}     
#define XR_RETRY_SCHEDULE_18 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(18,12,6,3,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(18,12,6,3,WIRELESS_MODE_XR,SHORT_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(18,12,6,3,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(18,12,6,3,WIRELESS_MODE_XR,SHORT_PRE)}    
#define XR_RETRY_SCHEDULE_24 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_XR,SHORT_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(24,18,12,6,WIRELESS_MODE_XR,SHORT_PRE)}   
#define XR_RETRY_SCHEDULE_36 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(36,24,18,6,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(36,24,18,6,WIRELESS_MODE_XR,SHORT_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(36,24,18,12,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(36,24,18,12,WIRELESS_MODE_XR,SHORT_PRE)}  
#define XR_RETRY_SCHEDULE_48 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(48,36,24,12,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(48,36,24,12,WIRELESS_MODE_XR,SHORT_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(48,36,24,18,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(48,36,24,18,WIRELESS_MODE_XR,SHORT_PRE)}  
#define XR_RETRY_SCHEDULE_54 {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(4,3,4,2),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_XR,SHORT_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_XR,LONG_PRE)},\
                             {TXCNTL_WD4(1,3,3,4),TXCNTL_WD5(54,48,36,24,WIRELESS_MODE_XR,SHORT_PRE)}

#if TURBO_PRIME
static RATE_TABLE ar5212_TurboPrimeARateTable = {
    16,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck long Preamble short Preamble  */
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration */
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,   6000,  5400,   0x0b,    0x00, (0x80|12),   0,       2,       1,     60,       60, A_RETRY_SCHEDULE_6},
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,   9000,  7800,   0x0f,    0x00,        18,   0,       3,       1,     60,       60, A_RETRY_SCHEDULE_9},
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM,  12000, 10000,   0x0a,    0x00, (0x80|24),   2,       4,       2,     48,       48, A_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM,  18000, 13900,   0x0e,    0x00,        36,   2,       6,       2,     48,       48, A_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM,  24000, 17300,   0x09,    0x00, (0x80|48),   4,      10,       3,     44,       44, A_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM,  36000, 23000,   0x0d,    0x00,        72,   4,      14,       3,     44,       44, A_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM,  48000, 27400,   0x08,    0x00,        96,   4,      19,       3,     44,       44, A_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM,  54000, 29300,   0x0c,    0x00,       108,   4,      23,       3,     44,       44, A_RETRY_SCHEDULE_54},
     /*   6 Mb */ {  TRUE, WLAN_PHY_TURBO,  6000, 10600,   0x0b,    0x00, (0x80|12),   8,       2,       1,     34,       34, TURBO_RETRY_SCHEDULE_6},
     /*   9 Mb */ {  TRUE, WLAN_PHY_TURBO,  9000, 15600,   0x0f,    0x00,        18,   8,       3,       1,     34,       34, TURBO_RETRY_SCHEDULE_9},
     /*  12 Mb */ {  TRUE, WLAN_PHY_TURBO, 12000, 19400,   0x0a,    0x00, (0x80|24),  10,       4,       2,     30,       30, TURBO_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE, WLAN_PHY_TURBO, 18000, 26900,   0x0e,    0x00,        36,  10,       6,       2,     30,       30, TURBO_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE, WLAN_PHY_TURBO, 24000, 33200,   0x09,    0x00, (0x80|48),  12,      10,       3,     26,       26, TURBO_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE, WLAN_PHY_TURBO, 36000, 43600,   0x0d,    0x00,        72,  12,      14,       3,     26,       26, TURBO_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE, WLAN_PHY_TURBO, 48000, 51300,   0x08,    0x00,        96,  12,      19,       3,     26,       26, TURBO_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE, WLAN_PHY_TURBO, 54000, 55100,   0x0c,    0x00,       108,  12,      23,       3,     26,       26, TURBO_RETRY_SCHEDULE_54},
    },
    50,  /* probe interval */
    50,  /* rssi reduce interval */
    5,   /* 36 Mbps for 11a */
    11,  /* 18 Mbps (36 Mbps) for 108a */
    50,  /* packet count threshold */
    7,   /* initial rateMax (index) */
    8    /* # of turboRates */

};
#else
static RATE_TABLE ar5212_11aRateTable = {
    8,  /* number of rates */
    { -1 },
    {/*                                                             short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                 Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,  6000,  5400,   0x0b,    0x00, (0x80|12),   0,       2,       1,      60,        60, A_RETRY_SCHEDULE_6}, 
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,  9000,  7800,   0x0f,    0x00,        18,   0,       3,       1,      60,        60, A_RETRY_SCHEDULE_9}, 
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM, 12000, 10000,   0x0a,    0x00, (0x80|24),   2,       4,       2,      48,        48, A_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM, 18000, 13900,   0x0e,    0x00,        36,   2,       6,       2,      48,        48, A_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM, 24000, 17300,   0x09,    0x00, (0x80|48),   4,      10,       3,      44,        44, A_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM, 36000, 23000,   0x0d,    0x00,        72,   4,      14,       3,      44,        44, A_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM, 48000, 27400,   0x08,    0x00,        96,   4,      19,       3,      44,        44, A_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM, 54000, 29300,   0x0c,    0x00,       108,   4,      23,       3,      44,        44, A_RETRY_SCHEDULE_54},
    },
    50,  /* probe interval */
    50,  /* rssi reduce interval */
    0,
    0, 
    0,
    7,   /* initial rateMax (index) */
    0,   /* # of turboRates */
};
#endif

RATE_TABLE ar5212_11aRateTable_Half = {
    8,  /* number of rates */
    { -1 },
    {/*                                                             short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                 Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,   3000,  2700,   0x0b,    0x00,  (0x80|6),   0,       2,       1,    120,      120,  A_RETRY_SCHEDULE_6},
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,   4500,  3900,   0x0f,    0x00,         9,   0,       3,       1,    120,      120,  A_RETRY_SCHEDULE_9},
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM,   6000,  5000,   0x0a,    0x00, (0x80|12),   2,       4,       2,     96,       96,  A_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM,   9000,  6950,   0x0e,    0x00,        18,   2,       6,       2,     96,       96,  A_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM,  12000,  8650,   0x09,    0x00, (0x80|24),   4,      10,       3,     88,       88,  A_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM,  18000, 11500,   0x0d,    0x00,        36,   4,      14,       3,     88,       88,  A_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM,  24000, 13700,   0x08,    0x00,        48,   4,      19,       3,     88,       88,  A_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM,  27000, 14650,   0x0c,    0x00,        54,   4,      23,       3,     88,       88,  A_RETRY_SCHEDULE_54},
    },
    50,  /* probe interval */
    50,  /* rssi reduce interval */
    0,
    0,
    0,
    7,   /* initial rateMax (index) */
    0,   /* # of turboRates */
};

RATE_TABLE ar5212_11aRateTable_Quarter = {
    8,  /* number of rates */
    { -1 },
    {/*                                                             short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                 Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,   1500,  1350,   0x0b,    0x00,  (0x80|3),   0,       2,       1,    240,      240,  A_RETRY_SCHEDULE_6},
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,   2250,  1950,   0x0f,    0x00,         4,   0,       3,       1,    240,      240,  A_RETRY_SCHEDULE_9},
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM,   3000,  2500,   0x0a,    0x00,  (0x80|6),   2,       4,       2,    192,      192,  A_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM,   4500,  3475,   0x0e,    0x00,         9,   2,       6,       2,    192,      192,  A_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM,   6000,  4325,   0x09,    0x00, (0x80|12),   4,      10,       3,    176,      176,  A_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM,   9000,  5750,   0x0d,    0x00,        18,   4,      14,       3,    176,      176,  A_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM,  12000,  6850,   0x08,    0x00,        24,   4,      19,       3,    176,      176,  A_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM,  13500,  7325,   0x0c,    0x00,        27,   4,      23,       3,    176,      176,  A_RETRY_SCHEDULE_54},
    },
    50,  /* probe interval */
    50,  /* rssi reduce interval */
    0,
    0,
    0,
    7,   /* initial rateMax (index) */
    0,   /* # of turboRates */
};

static RATE_TABLE ar5212_TurboRateTable = {
    8,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   6 Mb */ {  TRUE, WLAN_PHY_TURBO,  6000,  5400,   0x0b,    0x00, (0x80|12),   0,       2,       1,     34,         34, TURBO_RETRY_SCHEDULE_6}, 
     /*   9 Mb */ {  TRUE, WLAN_PHY_TURBO,  9000,  7800,   0x0f,    0x00,        18,   0,       4,       1,     34,         34, TURBO_RETRY_SCHEDULE_9}, 
     /*  12 Mb */ {  TRUE, WLAN_PHY_TURBO, 12000, 10000,   0x0a,    0x00, (0x80|24),   2,       7,       2,     30,         30, TURBO_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE, WLAN_PHY_TURBO, 18000, 13900,   0x0e,    0x00,        36,   2,       9,       2,     30,         30, TURBO_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE, WLAN_PHY_TURBO, 24000, 17300,   0x09,    0x00, (0x80|48),   4,      14,       3,     26,         26, TURBO_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE, WLAN_PHY_TURBO, 36000, 23000,   0x0d,    0x00,        72,   4,      17,       3,     26,         26, TURBO_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE, WLAN_PHY_TURBO, 48000, 27400,   0x08,    0x00,        96,   4,      22,       3,     26,         26, TURBO_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE, WLAN_PHY_TURBO, 54000, 29300,   0x0c,    0x00,       108,   4,      26,       3,     26,         26, TURBO_RETRY_SCHEDULE_54},
    },
    50, /* probe interval */
    50,  /* rssi reduce interval */
    0,
    0, 
    0,
    7,   /* initial rateMax (index) */
    8,   /* # of turboRates */
};

#if TURBO_PRIME
static RATE_TABLE ar5212_TurboPrimeGRateTable = {
    19,  /* number of rates */
    { -1 },
    {/*                                                                short           ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid  phy              Kbps  uKbps   rateCode Preamble dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   1 Mb */ {  TRUE,  WLAN_PHY_CCK,    1000,   900,  0x1b,    0x00,         2,   0,      0,      1,       314,         314, G_RETRY_SCHEDULE_1 },
     /*   2 Mb */ {  TRUE,  WLAN_PHY_CCK,    2000,  1900,  0x1a,    0x04,         4,   1,      1,      1,       258,         162, G_RETRY_SCHEDULE_2 },
     /* 5.5 Mb */ {  TRUE,  WLAN_PHY_CCK,    5500,  4900,  0x19,    0x04,        11,   2,      2,      2,       223,         127, G_RETRY_SCHEDULE_5_5 },
     /*  11 Mb */ {  TRUE,  WLAN_PHY_CCK,   11000,  8100,  0x18,    0x04,        22,   3,      3,      2,       213,         117, G_RETRY_SCHEDULE_11},
     /*   6 Mb */ {  FALSE, WLAN_PHY_OFDM,   6000,  5400,  0x0b,    0x00,        12,   4,      2,      1,        60,          60, G_RETRY_SCHEDULE_6 },
     /*   9 Mb */ {  FALSE, WLAN_PHY_OFDM,   9000,  7800,  0x0f,    0x00,        18,   4,      3,      1,        60,          60, G_RETRY_SCHEDULE_9 },
     /*  12 Mb */ {  TRUE,  WLAN_PHY_OFDM,  12000, 10100,  0x0a,    0x00,        24,   6,      4,      1,        48,          48, G_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE,  WLAN_PHY_OFDM,  18000, 14100,  0x0e,    0x00,        36,   6,      6,      2,        48,          48, G_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE,  WLAN_PHY_OFDM,  24000, 17700,  0x09,    0x00,        48,   8,     10,      3,        44,          44, G_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE,  WLAN_PHY_OFDM,  36000, 23700,  0x0d,    0x00,        72,   8,     14,      3,        44,          44, G_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE,  WLAN_PHY_OFDM,  48000, 27400,  0x08,    0x00,        96,   8,     20,      3,        44,          44, G_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE,  WLAN_PHY_OFDM,  54000, 30900,  0x0c,    0x00,       108,   8,     23,      3,        44,          44, G_RETRY_SCHEDULE_54},
     /*   6 Mb */ {  TRUE,  WLAN_PHY_TURBO,  6000, 10600,  0x0b,    0x00, (0x80|12),   12,     2,      1,        34,          34, TURBO_RETRY_SCHEDULE_6}, 
     /*  12 Mb */ {  TRUE,  WLAN_PHY_TURBO, 12000, 19400,  0x0a,    0x00,        24,   13,     4,      1,        30,          30, TURBO_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE,  WLAN_PHY_TURBO, 18000, 26900,  0x0e,    0x00,        36,   13,     6,      2,        30,          30, TURBO_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE,  WLAN_PHY_TURBO, 24000, 33200,  0x09,    0x00, (0x80|48),   15,    10,      3,        26,          26, TURBO_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE,  WLAN_PHY_TURBO, 36000, 43600,  0x0d,    0x00,        72,   15,    14,      3,        26,          26, TURBO_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE,  WLAN_PHY_TURBO, 48000, 51300,  0x08,    0x00,        96,   15,    19,      3,        26,          26, TURBO_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE,  WLAN_PHY_TURBO, 54000, 55100,  0x0c,    0x00,       108,   15,    23,      3,        26,          26, TURBO_RETRY_SCHEDULE_54},
    },                                                                                                                          
    50,  /* probe interval */
    50,  /* rssi reduce interval */
    9,   /* 36 Mbps for 11g */
    14,  /* 18 Mbps (36 Mbps) for 108g */
    50,  /* packet count threshold */
    11,  /* initial rateMax (index) */
    7,   /* # of turboRates */
};
#else
/* Venice TODO: roundUpRate() is broken when the rate table does not represent rates
 * in increasing order  e.g.  5.5, 11, 6, 9.    
 * An average rate of 6 Mbps will currently map to 11 Mbps. 
 */
static RATE_TABLE ar5212_11gRateTable = {
    12,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   1 Mb */ {  TRUE,  WLAN_PHY_CCK,   1000,   900,  0x1b,    0x00,         2,   0,       0,       1,       314,       314, G_RETRY_SCHEDULE_1 },
     /*   2 Mb */ {  TRUE,  WLAN_PHY_CCK,   2000,  1900,  0x1a,    0x04,         4,   1,       1,       1,       258,       162, G_RETRY_SCHEDULE_2 },
     /* 5.5 Mb */ {  TRUE,  WLAN_PHY_CCK,   5500,  4900,  0x19,    0x04,        11,   2,       2,       2,       223,       127, G_RETRY_SCHEDULE_5_5 },
     /*  11 Mb */ {  TRUE,  WLAN_PHY_CCK,  11000,  8100,  0x18,    0x04,        22,   3,       3,       2,       213,       117, G_RETRY_SCHEDULE_11},
     /*   6 Mb */ {  FALSE, WLAN_PHY_OFDM, 6000,   5400,  0x0b,    0x00,        12,   4,       2,       1,        60,        60, G_RETRY_SCHEDULE_6 },
     /*   9 Mb */ {  FALSE, WLAN_PHY_OFDM, 9000,   7800,  0x0f,    0x00,        18,   4,       3,       1,        60,        60, G_RETRY_SCHEDULE_9 },
     /*  12 Mb */ {  TRUE,  WLAN_PHY_OFDM, 12000, 10000,  0x0a,    0x00,        24,   6,       4,       1,        48,        48, G_RETRY_SCHEDULE_12},
     /*  18 Mb */ {  TRUE,  WLAN_PHY_OFDM, 18000, 13900,  0x0e,    0x00,        36,   6,       6,       2,        48,        48, G_RETRY_SCHEDULE_18},
     /*  24 Mb */ {  TRUE,  WLAN_PHY_OFDM, 24000, 17300,  0x09,    0x00,        48,   8,      10,       3,        44,        44, G_RETRY_SCHEDULE_24},
     /*  36 Mb */ {  TRUE,  WLAN_PHY_OFDM, 36000, 23000,  0x0d,    0x00,        72,   8,      14,       3,        44,        44, G_RETRY_SCHEDULE_36},
     /*  48 Mb */ {  TRUE,  WLAN_PHY_OFDM, 48000, 27400,  0x08,    0x00,        96,   8,      19,       3,        44,        44, G_RETRY_SCHEDULE_48},
     /*  54 Mb */ {  TRUE,  WLAN_PHY_OFDM, 54000, 29300,  0x0c,    0x00,       108,   8,      23,       3,        44,        44, G_RETRY_SCHEDULE_54},
    },
    50,  /* probe interval */
    50,  /* rssi reduce interval */
    0,
    0, 
    0,
    11,  /* initial rateMax (index) */
    0,   /* # of turboRates */
};
#endif

static RATE_TABLE ar5212_11bRateTable = {
    4,  /* number of rates */
    { -1 },
    {/*                                                             short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                 Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   1 Mb */ {  TRUE,  WLAN_PHY_CCK,  1000,  900,  0x1b,    0x00, (0x80| 2),   0,       0,       1,        314,        314, G_RETRY_SCHEDULE_1 },
     /*   2 Mb */ {  TRUE,  WLAN_PHY_CCK,  2000, 1800,  0x1a,    0x04, (0x80| 4),   1,       1,       1,        258,        162, G_RETRY_SCHEDULE_2 },
     /* 5.5 Mb */ {  TRUE,  WLAN_PHY_CCK,  5500, 4300,  0x19,    0x04, (0x80|11),   1,       2,       2,        258,        162, G_RETRY_SCHEDULE_5_5 },
     /*  11 Mb */ {  TRUE,  WLAN_PHY_CCK, 11000, 7100,  0x18,    0x04, (0x80|22),   1,       4,     100,        258,        162, G_RETRY_SCHEDULE_11},
    },
    100, /* probe interval */
    100, /* rssi reduce interval */
    0,
    0, 
    0,
    3,   /* initial rateMax (index) */
    0,   /* # of turboRates */
};

static RATE_TABLE ar5212_XrRateTable = {
    13,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /* 1/4 Mb */ {  TRUE, WLAN_PHY_XR,     250,   200,   0x03,    0x00,         1,   0,      -9,       1,     612,       612, XR_RETRY_SCHEDULE_0_25},
     /* 0.5 Mb */ {  TRUE, WLAN_PHY_XR,     500,   400,   0x07,    0x00,         1,   1,      -8,       4,     457,       457, XR_RETRY_SCHEDULE_0_5 },
     /*   1 Mb */ {  TRUE, WLAN_PHY_XR,    1000,   900,   0x02,    0x00,         2,   2,      -4,       2,     228,       228, XR_RETRY_SCHEDULE_1   },
     /*   2 Mb */ {  TRUE, WLAN_PHY_XR,    2000,  1800,   0x06,    0x00,         4,   3,      -2,       1,     160,       160, XR_RETRY_SCHEDULE_2   },
     /*   3 Mb */ {  TRUE, WLAN_PHY_XR,    3000,  2700,   0x01,    0x00,         6,   4,      -1,       1,     140,       140, XR_RETRY_SCHEDULE_3   },
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,  6000,  5400,   0x0b,    0x00, (0x80|12),   5,       2,       1,      60,        60, XR_RETRY_SCHEDULE_6   },
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,  9000,  7800,   0x0f,    0x00,        18,   5,       3,       1,      60,        60, XR_RETRY_SCHEDULE_9   },
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM, 12000, 10000,   0x0a,    0x00, (0x80|24),   7,       4,       2,      48,        48, XR_RETRY_SCHEDULE_12  },
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM, 18000, 13900,   0x0e,    0x00,        36,   7,       6,       2,      48,        48, XR_RETRY_SCHEDULE_18  },
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM, 24000, 17300,   0x09,    0x00, (0x80|48),   9,      10,       3,      44,        44, XR_RETRY_SCHEDULE_24  },
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM, 36000, 23000,   0x0d,    0x00,        72,   9,      14,       3,      44,        44, XR_RETRY_SCHEDULE_36  },
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM, 48000, 27400,   0x08,    0x00,        96,   9,      19,       3,      44,        44, XR_RETRY_SCHEDULE_48  },
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM, 54000, 29300,   0x0c,    0x00,       108,   9,      23,       3,      44,        44, XR_RETRY_SCHEDULE_54  },
    },
    200, /* probe interval */
    100, /* rssi reduce interval */
    0,
    0, 
    0,
    12,  /* initial rateMax (index) */
    0,   /* # of turboRates */
};


#else
#if TURBO_PRIME
static RATE_TABLE ar5212_TurboPrimeARateTable = {
    16,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck long Preamble short Preamble  */
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration */
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,   6000,  5400,   0x0b,    0x00, (0x80|12),   0,       2,       1,     60,       60},
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,   9000,  7800,   0x0f,    0x00,        18,   0,       3,       1,     60,       60},
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM,  12000, 10000,   0x0a,    0x00, (0x80|24),   2,       4,       2,     48,       48},
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM,  18000, 13900,   0x0e,    0x00,        36,   2,       6,       2,     48,       48},
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM,  24000, 17300,   0x09,    0x00, (0x80|48),   4,      10,       3,     44,       44},
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM,  36000, 23000,   0x0d,    0x00,        72,   4,      14,       3,     44,       44},
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM,  48000, 27400,   0x08,    0x00,        96,   4,      19,       3,     44,       44},
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM,  54000, 29300,   0x0c,    0x00,       108,   4,      23,       3,     44,       44},
     /*   6 Mb */ {  TRUE, WLAN_PHY_TURBO,  6000, 10600,   0x0b,    0x00, (0x80|12),   8,       2,       1,     34,       34},
     /*   9 Mb */ {  TRUE, WLAN_PHY_TURBO,  9000, 15600,   0x0f,    0x00,        18,   8,       3,       1,     34,       34},
     /*  12 Mb */ {  TRUE, WLAN_PHY_TURBO, 12000, 19400,   0x0a,    0x00, (0x80|24),  10,       4,       2,     30,       30},
     /*  18 Mb */ {  TRUE, WLAN_PHY_TURBO, 18000, 26900,   0x0e,    0x00,        36,  10,       6,       2,     30,       30},
     /*  24 Mb */ {  TRUE, WLAN_PHY_TURBO, 24000, 33200,   0x09,    0x00, (0x80|48),  12,      10,       3,     26,       26},
     /*  36 Mb */ {  TRUE, WLAN_PHY_TURBO, 36000, 43600,   0x0d,    0x00,        72,  12,      14,       3,     26,       26},
     /*  48 Mb */ {  TRUE, WLAN_PHY_TURBO, 48000, 51300,   0x08,    0x00,        96,  12,      19,       3,     26,       26},
     /*  54 Mb */ {  TRUE, WLAN_PHY_TURBO, 54000, 55100,   0x0c,    0x00,       108,  12,      23,       3,     26,       26},
    },
    100, /* probe interval */
    50,  /* rssi reduce interval */
    5,   /* 36 Mbps for 11a */
    11,  /* 18 Mbps (36 Mbps) for 108a */
    50,  /* packet count threshold */
    7,   /* initial rateMax (index) */
    8,   /* # of turboRates */
};
#else
static RATE_TABLE ar5212_11aRateTable = {
    8,  /* number of rates */
    { -1 },
    {/*                                                             short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                 Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,  6000,  5400,   0x0b,    0x00, (0x80|12),   0,       2,       1,      60,        60},
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,  9000,  7800,   0x0f,    0x00,        18,   0,       3,       1,      60,        60},
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM, 12000, 10000,   0x0a,    0x00, (0x80|24),   2,       4,       2,      48,        48},
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM, 18000, 13900,   0x0e,    0x00,        36,   2,       6,       2,      48,        48},
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM, 24000, 17300,   0x09,    0x00, (0x80|48),   4,      10,       3,      44,        44},
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM, 36000, 23000,   0x0d,    0x00,        72,   4,      14,       3,      44,        44},
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM, 48000, 27400,   0x08,    0x00,        96,   4,      19,       3,      44,        44},
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM, 54000, 29300,   0x0c,    0x00,       108,   4,      23,       3,      44,        44},
    },
    100, /* probe interval */
    50,  /* rssi reduce interval */
    0,
    0, 
    0,
    7,   /* initial rateMax (index) */
    0,   /* # of turboRates */
};
#endif

static RATE_TABLE ar5212_TurboRateTable = {
    8,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   6 Mb */ {  TRUE, WLAN_PHY_TURBO,  6000,  5400,   0x0b,    0x00, (0x80|12),   0,       2,       1,     34,         34},
     /*   9 Mb */ {  TRUE, WLAN_PHY_TURBO,  9000,  7800,   0x0f,    0x00,        18,   0,       4,       1,     34,         34},
     /*  12 Mb */ {  TRUE, WLAN_PHY_TURBO, 12000, 10000,   0x0a,    0x00, (0x80|24),   2,       7,       2,     30,         30},
     /*  18 Mb */ {  TRUE, WLAN_PHY_TURBO, 18000, 13900,   0x0e,    0x00,        36,   2,       9,       2,     30,         30},
     /*  24 Mb */ {  TRUE, WLAN_PHY_TURBO, 24000, 17300,   0x09,    0x00, (0x80|48),   4,      14,       3,     26,         26},
     /*  36 Mb */ {  TRUE, WLAN_PHY_TURBO, 36000, 23000,   0x0d,    0x00,        72,   4,      17,       3,     26,         26},
     /*  48 Mb */ {  TRUE, WLAN_PHY_TURBO, 48000, 27400,   0x08,    0x00,        96,   4,      22,       3,     26,         26},
     /*  54 Mb */ {  TRUE, WLAN_PHY_TURBO, 54000, 29300,   0x0c,    0x00,       108,   4,      26,       3,     26,         26},
    },
    100, /* probe interval */
    50,  /* rssi reduce interval */
    0,
    0, 
    0,
    7,   /* initial rateMax (index) */
    8,   /* # of turboRates */
};

#if TURBO_PRIME
static RATE_TABLE ar5212_TurboPrimeGRateTable = {
    19,  /* number of rates */
    { -1 },
    {/*                                                                short           ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid  phy              Kbps  uKbps   rateCode Preamble dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   1 Mb */ {  TRUE,  WLAN_PHY_CCK,    1000,   900,  0x1b,    0x00,         2,   0,      0,      1,       314,         314},
     /*   2 Mb */ {  TRUE,  WLAN_PHY_CCK,    2000,  1900,  0x1a,    0x04,         4,   1,      1,      1,       258,         162},
     /* 5.5 Mb */ {  TRUE,  WLAN_PHY_CCK,    5500,  4900,  0x19,    0x04,        11,   2,      2,      2,       223,         127},
     /*  11 Mb */ {  TRUE,  WLAN_PHY_CCK,   11000,  8100,  0x18,    0x04,        22,   3,      3,      2,       213,         117},
     /*   6 Mb */ {  FALSE, WLAN_PHY_OFDM,   6000,  5400,  0x0b,    0x00,        12,   4,      2,      1,        60,          60},
     /*   9 Mb */ {  FALSE, WLAN_PHY_OFDM,   9000,  7800,  0x0f,    0x00,        18,   4,      3,      1,        60,          60},
     /*  12 Mb */ {  TRUE,  WLAN_PHY_OFDM,  12000, 10100,  0x0a,    0x00,        24,   6,      4,      1,        48,          48},
     /*  18 Mb */ {  TRUE,  WLAN_PHY_OFDM,  18000, 14100,  0x0e,    0x00,        36,   6,      6,      2,        48,          48},
     /*  24 Mb */ {  TRUE,  WLAN_PHY_OFDM,  24000, 17700,  0x09,    0x00,        48,   8,     10,      3,        44,          44},
     /*  36 Mb */ {  TRUE,  WLAN_PHY_OFDM,  36000, 23700,  0x0d,    0x00,        72,   8,     14,      3,        44,          44},
     /*  48 Mb */ {  TRUE,  WLAN_PHY_OFDM,  48000, 27400,  0x08,    0x00,        96,   8,     19,      3,        44,          44},
     /*  54 Mb */ {  TRUE,  WLAN_PHY_OFDM,  54000, 29300,  0x0c,    0x00,       108,   8,     23,      3,        44,          44},
     /*   6 Mb */ {  TRUE,  WLAN_PHY_TURBO,  6000, 10600,  0x0b,    0x00, (0x80|12),   12,     2,      1,        34,          34},
     /*  12 Mb */ {  TRUE,  WLAN_PHY_TURBO, 12000, 19400,  0x0a,    0x00,        24,   13,     4,      1,        30,          30},
     /*  18 Mb */ {  TRUE,  WLAN_PHY_TURBO, 18000, 26900,  0x0e,    0x00,        36,   13,     6,      2,        30,          30},
     /*  24 Mb */ {  TRUE,  WLAN_PHY_TURBO, 24000, 33200,  0x09,    0x00, (0x80|48),   15,    10,      3,        26,          26},
     /*  36 Mb */ {  TRUE,  WLAN_PHY_TURBO, 36000, 43600,  0x0d,    0x00,        72,   15,    14,      3,        26,          26},
     /*  48 Mb */ {  TRUE,  WLAN_PHY_TURBO, 48000, 51300,  0x08,    0x00,        96,   15,    19,      3,        26,          26},
     /*  54 Mb */ {  TRUE,  WLAN_PHY_TURBO, 54000, 55100,  0x0c,    0x00,       108,   15,    23,      3,        26,          26},
    },
    100, /* probe interval */
    50,  /* rssi reduce interval */
    9,   /* 36 Mbps for 11g */
    14,  /* 18 Mbps (36 Mbps) for 108g */
    50,  /* packet count threshold */
    11,  /* initial rateMax (index) */
    7,   /* # of turboRates */
};
#else
/* Venice TODO: roundUpRate() is broken when the rate table does not represent rates
 * in increasing order  e.g.  5.5, 11, 6, 9.    
 * An average rate of 6 Mbps will currently map to 11 Mbps. 
 */
static RATE_TABLE ar5212_11gRateTable = {
    12,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   1 Mb */ {  TRUE,  WLAN_PHY_CCK,   1000,   900,  0x1b,    0x00,         2,   0,       0,       1,       314,       314},
     /*   2 Mb */ {  TRUE,  WLAN_PHY_CCK,   2000,  1900,  0x1a,    0x04,         4,   1,       1,       1,       258,       162},
     /* 5.5 Mb */ {  TRUE,  WLAN_PHY_CCK,   5500,  4900,  0x19,    0x04,        11,   2,       2,       2,       223,       127},
     /*  11 Mb */ {  TRUE,  WLAN_PHY_CCK,  11000,  8100,  0x18,    0x04,        22,   3,       3,       2,       213,       117},
     /*   6 Mb */ {  FALSE, WLAN_PHY_OFDM, 6000,   5400,  0x0b,    0x00,        12,   4,       2,       1,        60,        60},
     /*   9 Mb */ {  FALSE, WLAN_PHY_OFDM, 9000,   7800,  0x0f,    0x00,        18,   4,       3,       1,        60,        60},
     /*  12 Mb */ {  TRUE,  WLAN_PHY_OFDM, 12000, 10000,  0x0a,    0x00,        24,   6,       4,       1,        48,        48},
     /*  18 Mb */ {  TRUE,  WLAN_PHY_OFDM, 18000, 13900,  0x0e,    0x00,        36,   6,       6,       2,        48,        48},
     /*  24 Mb */ {  TRUE,  WLAN_PHY_OFDM, 24000, 17300,  0x09,    0x00,        48,   8,      10,       3,        44,        44},
     /*  36 Mb */ {  TRUE,  WLAN_PHY_OFDM, 36000, 23000,  0x0d,    0x00,        72,   8,      14,       3,        44,        44},
     /*  48 Mb */ {  TRUE,  WLAN_PHY_OFDM, 48000, 27400,  0x08,    0x00,        96,   8,      19,       3,        44,        44},
     /*  54 Mb */ {  TRUE,  WLAN_PHY_OFDM, 54000, 29300,  0x0c,    0x00,       108,   8,      23,       3,        44,        44},
    },
    100, /* probe interval */
    50,  /* rssi reduce interval */
    0,
    0, 
    0,
    11,  /* initial rateMax (index) */
    0,   /* # of turboRates */
};
#endif

static RATE_TABLE ar5212_11bRateTable = {
    4,  /* number of rates */
    { -1 },
    {/*                                                             short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                 Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /*   1 Mb */ {  TRUE,  WLAN_PHY_CCK,  1000,  900,  0x1b,    0x00, (0x80| 2),   0,       0,       1,        314,        314},
     /*   2 Mb */ {  TRUE,  WLAN_PHY_CCK,  2000, 1800,  0x1a,    0x04, (0x80| 4),   1,       1,       1,        258,        162},
     /* 5.5 Mb */ {  TRUE,  WLAN_PHY_CCK,  5500, 4300,  0x19,    0x04, (0x80|11),   1,       2,       2,        258,        162},
     /*  11 Mb */ {  TRUE,  WLAN_PHY_CCK, 11000, 7100,  0x18,    0x04, (0x80|22),   1,       4,     100,        258,        162},
    },
    200, /* probe interval */
    100, /* rssi reduce interval */
    0,
    0, 
    0,
    3,   /* initial rateMax (index) */
    0,   /* # of turboRates */
};

static RATE_TABLE ar5212_XrRateTable = {
    13,  /* number of rates */
    { -1 },
    {/*                                                              short            ctrl  RssiAck  RssiAck long Preamble short Preamble*/
     /*              valid                  Kbps  uKbps  rateCode Preamble  dot11Rate Rate ValidMin DeltaMin  ACK Duration   ACK Duration*/
     /* 1/4 Mb */ {  TRUE, WLAN_PHY_XR,     250,   200,   0x03,    0x00,         1,   0,      -9,       1,     612,       612},
     /* 0.5 Mb */ {  TRUE, WLAN_PHY_XR,     500,   400,   0x07,    0x00,         1,   1,      -8,       4,     457,       457},
     /*   1 Mb */ {  TRUE, WLAN_PHY_XR,    1000,   900,   0x02,    0x00,         2,   2,      -4,       2,     228,       228},
     /*   2 Mb */ {  TRUE, WLAN_PHY_XR,    2000,  1800,   0x06,    0x00,         4,   3,      -2,       1,     160,       160},
     /*   3 Mb */ {  TRUE, WLAN_PHY_XR,    3000,  2700,   0x01,    0x00,         6,   4,      -1,       1,     140,       140},
     /*   6 Mb */ {  TRUE, WLAN_PHY_OFDM,  6000,  5400,   0x0b,    0x00, (0x80|12),   5,       2,       1,      60,        60},
     /*   9 Mb */ {  TRUE, WLAN_PHY_OFDM,  9000,  7800,   0x0f,    0x00,        18,   5,       3,       1,      60,        60},
     /*  12 Mb */ {  TRUE, WLAN_PHY_OFDM, 12000, 10000,   0x0a,    0x00, (0x80|24),   7,       4,       2,      48,        48},
     /*  18 Mb */ {  TRUE, WLAN_PHY_OFDM, 18000, 13900,   0x0e,    0x00,        36,   7,       6,       2,      48,        48},
     /*  24 Mb */ {  TRUE, WLAN_PHY_OFDM, 24000, 17300,   0x09,    0x00, (0x80|48),   9,      10,       3,      44,        44},
     /*  36 Mb */ {  TRUE, WLAN_PHY_OFDM, 36000, 23000,   0x0d,    0x00,        72,   9,      14,       3,      44,        44},
     /*  48 Mb */ {  TRUE, WLAN_PHY_OFDM, 48000, 27400,   0x08,    0x00,        96,   9,      19,       3,      44,        44},
     /*  54 Mb */ {  TRUE, WLAN_PHY_OFDM, 54000, 29300,   0x0c,    0x00,       108,   9,      23,       3,      44,        44},
    },
    200, /* probe interval */
    100, /* rssi reduce interval */
    0,
    0, 
    0,
    12,  /* initial rateMax (index) */
    0,   /* # of turboRates */
};
#endif

void
ar5212SetupRateTables(void)
{
    atheros_setuptable(&ar5212_TurboRateTable);
    atheros_setuptable(&ar5212_11bRateTable);
#if TURBO_PRIME
    atheros_setuptable(&ar5212_TurboPrimeARateTable);
    atheros_setuptable(&ar5212_TurboPrimeGRateTable);
#else
    atheros_setuptable(&ar5212_11aRateTable);
    atheros_setuptable(&ar5212_11gRateTable);
#endif
    atheros_setuptable(&ar5212_TurboRateTable);
    atheros_setuptable(&ar5212_XrRateTable);
#if defined(XR_HACKERY)
#if TURBO_PRIME
    atheros_setuptable(&ar5212_TurboPrimeARateTable);
#else
    atheros_setuptable(&ar5212_11aRateTable);
#endif
#endif  
}

void
ar5212AttachRateTables(struct atheros_softc *sc)
{
    /*
     * Attach device specific rate tables; for ar5212.
     * 11a static turbo and 11g static turbo share the same table.
     * Dynamic turbo uses combined rate table.
     */
    sc->hwRateTable[WIRELESS_MODE_11b]   = &ar5212_11bRateTable;
#if TURBO_PRIME
    sc->hwRateTable[WIRELESS_MODE_11a]   = &ar5212_TurboPrimeARateTable;
    sc->hwRateTable[WIRELESS_MODE_11g]   = &ar5212_TurboPrimeGRateTable;
    sc->hwRateTable[WIRELESS_MODE_108a]  = &ar5212_TurboPrimeARateTable;
    sc->hwRateTable[WIRELESS_MODE_108g]  = &ar5212_TurboPrimeGRateTable;
#else
    sc->hwRateTable[WIRELESS_MODE_11a]   = &ar5212_11aRateTable;
    sc->hwRateTable[WIRELESS_MODE_11g]   = &ar5212_11gRateTable;
    sc->hwRateTable[WIRELESS_MODE_108a]  = &ar5212_TurboRateTable;
    sc->hwRateTable[WIRELESS_MODE_108g]  = &ar5212_TurboRateTable;
#endif
    sc->hwRateTable[WIRELESS_MODE_XR]    = &ar5212_XrRateTable;
#if defined(XR_HACKERY)
#if TURBO_PRIME
    sc->hwRateTable[WIRELESS_MODE_XR]    = &ar5212_TurboPrimeARateTable;
#else
    sc->hwRateTable[WIRELESS_MODE_XR]    = &ar5212_11aRateTable;
#endif
#endif  
}

void
ar5212SetQuarterRateTable(struct atheros_softc *sc)
{
	sc->hwRateTable[WIRELESS_MODE_11a] = &ar5212_11aRateTable_Quarter;
	return;
}

void
ar5212SetHalfRateTable(struct atheros_softc *sc)
{
	sc->hwRateTable[WIRELESS_MODE_11a] = &ar5212_11aRateTable_Half;
	return;
}

void
ar5212SetFullRateTable(struct atheros_softc *sc)
{
#if TURBO_PRIME
	sc->hwRateTable[WIRELESS_MODE_11a]   = &ar5212_TurboPrimeARateTable;
#else
    	sc->hwRateTable[WIRELESS_MODE_11a]   = &ar5212_11aRateTable;
#endif
	return;
}
