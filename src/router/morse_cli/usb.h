/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#pragma once

#include <libusb.h>

#define MORSE_ID_VENDOR 0x325b
#define MORSE_MM810X_PRODUCT_ID 0x8100

#define MORSE_INTF_NUM 0
#define MORSE_BULK_OUT_EP 2

#define MORSE_CMD_SIZE 12
#define MORSE_CMD_RESET 0x2

/**
 * @brief Detect if a morse USB device is present and send morse ndr reset command.
 *
 * @note This performs a digital reset while keeping the USB enumerated.
 *
 * @return 0 if success, otherwise negative return code.
 */
int usb_ndr_reset(void);
