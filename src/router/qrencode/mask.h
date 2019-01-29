/*
 * qrencode - QR Code encoder
 *
 * Masking.
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef MASK_H
#define MASK_H

static unsigned char *Mask_makeMask(unsigned int width, unsigned char *frame, int mask, QRecLevel level);
static unsigned char *Mask_mask(unsigned int width, unsigned char *frame, QRecLevel level);

#ifdef WITH_TESTS
static int Mask_calcN2(int width, unsigned char *frame);
static int Mask_calcN1N3(int length, int *runLength);
static int Mask_calcRunLengthH(int width, unsigned char *frame, int *runLength);
static int Mask_calcRunLengthV(int width, unsigned char *frame, int *runLength);
static int Mask_evaluateSymbol(int width, unsigned char *frame);
static unsigned int Mask_writeFormatInformation(unsigned int width, unsigned char *frame, int mask, QRecLevel level);
static unsigned char *Mask_makeMaskedFrame(unsigned int width, unsigned char *frame, int mask);
#endif

#endif /* MASK_H */
