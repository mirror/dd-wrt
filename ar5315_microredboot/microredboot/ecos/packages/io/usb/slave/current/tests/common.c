/*{{{  Banner                                                   */

/*=================================================================
//
//        common.c
//
//        USB testing - code common to host and target
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// This module contains some definitions and functions that are common to
// both the host and target side of USB testing, for example filling in
// a buffer with well-known data and validating the contents at the other end.
// The module is #include'd by other code rather than compiled separately,
// which simplifies the build process.
//
// Author(s):     bartv
// Date:          2001-08-14
//####DESCRIPTIONEND####
//==========================================================================
*/

/*}}}*/

/*{{{  Simple data pack and unpack operations                   */

// ----------------------------------------------------------------------------
// Utilities to pack and unpack data into buffers. 
//
// Integers are transferred with 32 bits of precision, irrespective
// of the capabilities of either target and host.

static inline void
pack_int(int datum, unsigned char* buffer, int* index_ptr)
{
    int index = *index_ptr;
    buffer[index++] = (datum >>  0) & 0x0FF;
    buffer[index++] = (datum >>  8) & 0x0FF;
    buffer[index++] = (datum >> 16) & 0x0FF;
    buffer[index++] = (datum >> 24) & 0x0FF;
    *index_ptr = index;
}

static inline int
unpack_int(unsigned char* buffer, int* index_ptr)
{
    int index   = *index_ptr;
    int result;

    result  = (buffer[index++] <<  0);
    result |= (buffer[index++] <<  8);
    result |= (buffer[index++] << 16);
    result |= (buffer[index++] << 24);
    *index_ptr = index;
    return result;
}

/*}}}*/
/*{{{  Buffer data and validation                               */

// ----------------------------------------------------------------------------
// The data required for a given test. For some test cases, for
// example when trying to achieve maximum throughput, it does not
// matter what data is transferred. For other tests it is important to
// validate that the data sent and received match up, and there should
// be some control over the actual data: some tests might want to send
// a long sequence of byte 0, while others want to send more random data
// for which a simple random number generator is useful.
//
// Exactly the same routines are used on both host and target to fill in
// and check buffers, and they are sufficiently simple that the routines
// should get compiled in compatible ways.
//
// There is no support at present for sending specific data, e.g. a
// specific ethernet packet that appears to be causing problems. Knowledge
// of specific data cannot be compiled into the test code, so the only
// way to implement something like this would be to transfer the
// problematical data over the USB bus in order to determine whether or
// not the bus is capable of reliably transferring this data. That is
// not entirely impossible (checksums, use of alternative endpoints),
// but it is not implemented.
//
// An alternative approach would be to support a bounce operation
// involving both an IN and an OUT endpoint, doing validation only on
// the host. Again that is not yet implemented.
//
// The byte_fill and int_fill options are actually redundant because the
// same effect can be achieved using a multiplier of 1 and an increment
// of 0, but they can be implemented much more efficiently so may be
// useful for benchmarks.

typedef enum usbtestdata {
    usbtestdata_none        = 0,       // There is nothing useful in the data
    usbtestdata_bytefill    = 1,       // The data consists of a single byte, repeated
    usbtestdata_wordfill    = 2,       // Or a single integer
    usbtestdata_byteseq     = 3,       // Or a pseudo-random sequence (a * seed) + b
    usbtestdata_wordseq     = 4        // as either bytes or integers
} usbtestdata;

typedef struct UsbTestData {
    usbtestdata     format;
    int             seed;
    int             multiplier; // 1103515245
    int             increment;  // 12345
    int             transfer_seed_multiplier;
    int             transfer_seed_increment;
    int             transfer_multiplier_multiplier;
    int             transfer_multiplier_increment;
    int             transfer_increment_multiplier;
    int             transfer_increment_increment;
} UsbTestData;

static void
usbtest_fill_buffer(UsbTestData* how, unsigned char* buffer, int length)
{
    switch(how->format)
    {
      case usbtestdata_none:
        return;
        
      case usbtestdata_bytefill:
        // Leave it to the system to optimise memset().
        memset(buffer, (how->seed & 0x0FF), length);
        break;

      case usbtestdata_wordfill:
      {
          // The buffer may not be a multiple of four bytes, so the last entry is always
          // zero'd.
          int i;
          int index = 0;
          for (i = 0; i < (length / 4); i++) {
              pack_int(how->seed, buffer, &index);
          }
          pack_int(0, buffer, &index);
          break;
      }

      case usbtestdata_byteseq:
      {
          int i;
          for (i = 0; i < length; i++) {
              buffer[i] = (how->seed & 0x00FF);
              how->seed *= how->multiplier;
              how->seed += how->increment;
          }
          break;
      }

      case usbtestdata_wordseq:
      {
          int i;
          int index = 0;
          for (i = 0; i < (length / 4); i++) {
              pack_int(how->seed, buffer, &index);
              how->seed *= how->multiplier;
              how->seed += how->increment;
          }
          pack_int(0, buffer, &index);
          break;
      }
    }

    // After each transfer update the seed, multiplier and increment
    // ready for the next one.
    how->seed       *= how->transfer_seed_multiplier;
    how->seed       += how->transfer_seed_increment;
    how->multiplier *= how->transfer_multiplier_multiplier;
    how->multiplier += how->transfer_multiplier_increment;
    how->increment  *= how->transfer_increment_multiplier;
    how->increment  += how->transfer_increment_increment;
}

static int
usbtest_check_buffer(UsbTestData* how, unsigned char* buffer, int length)
{
    int result  = 1;

    switch(how->format) {
      case usbtestdata_none:
        break;

      case usbtestdata_bytefill:
      {
          int i;
          result = 1;
          for (i = 0; i < length; i++) {
              if (buffer[i] != (how->seed & 0x00FF)) {
                  result = 0;
                  break;
              }
          }
          break;
      }

      case usbtestdata_wordfill:
      {
          int i;
          int index = 0;
          for (i = 0; i < (length / 4); i++) {
              int datum = unpack_int(buffer, &index);
              if (datum != (how->seed & 0x0FFFFFFFF)) {
                  result = 0;
                  break;
              }
          }
          for (i = 4 * i; result && (i < length); i++) {
              if (0 != buffer[i]) {
                  result = 0;
                  break;
              }
          }
          break;
      }

      case usbtestdata_byteseq:
      {
          int i;
          for (i = 0; i < length; i++) {
              if (buffer[i] != (how->seed & 0x00FF)) {
                  result = 0;
                  break;
              }
              how->seed *= how->multiplier;
              how->seed += how->increment;
          }
          break;
      }

      case usbtestdata_wordseq:
      {
          int   i;
          int   index = 0;
          
          for (i = 0; i < (length / 4); i++) {
              int datum = unpack_int(buffer, &index);
              if (datum != (how->seed & 0x0FFFFFFFF)) {
                  result = 0;
                  break;
              }
              how->seed *= how->multiplier;
              how->seed += how->increment;
          }
          for (i = 4 * i; result && (i < length); i++) {
              if (0 != buffer[i]) {
                  result = 0;
                  break;
              }
          }
          break;
      }
    }

    // After each transfer update the seed, multiplier and increment
    // ready for the next transfer.
    how->seed       *= how->transfer_seed_multiplier;
    how->seed       += how->transfer_seed_increment;
    how->multiplier *= how->transfer_multiplier_multiplier;
    how->multiplier += how->transfer_multiplier_increment;
    how->increment  *= how->transfer_increment_multiplier;
    how->increment  += how->transfer_increment_increment;
    
    return result;
}

#ifdef HOST
static void
pack_usbtestdata(UsbTestData* data, unsigned char* buf, int* index)
{
    pack_int((int)data->format,                         buf, index);
    pack_int((int)data->seed,                           buf, index);
    pack_int((int)data->multiplier,                     buf, index);
    pack_int((int)data->increment,                      buf, index);
    pack_int((int)data->transfer_seed_multiplier,       buf, index);
    pack_int((int)data->transfer_seed_increment,        buf, index);
    pack_int((int)data->transfer_multiplier_multiplier, buf, index);
    pack_int((int)data->transfer_multiplier_increment,  buf, index);
    pack_int((int)data->transfer_increment_multiplier,  buf, index);
    pack_int((int)data->transfer_increment_increment,   buf, index);
}
#endif

#ifdef TARGET
static void
unpack_usbtestdata(UsbTestData* data, unsigned char* buf, int* index)
{
    data->format                        = (usbtestdata) unpack_int(buf, index);
    data->seed                          = unpack_int(buf, index);
    data->multiplier                    = unpack_int(buf, index);
    data->increment                     = unpack_int(buf, index);
    data->transfer_seed_multiplier      = unpack_int(buf, index);
    data->transfer_seed_increment       = unpack_int(buf, index);
    data->transfer_multiplier_multiplier= unpack_int(buf, index);
    data->transfer_multiplier_increment = unpack_int(buf, index);
    data->transfer_increment_multiplier = unpack_int(buf, index);
    data->transfer_increment_increment  = unpack_int(buf, index);
}
#endif

/*}}}*/
/*{{{  Testcase definitions                                     */

// ----------------------------------------------------------------------------
// Definitions of the supported test cases. The actual implementations need
// to vary between host and target.

typedef enum usbtest {
    usbtest_invalid     = 0,
    usbtest_bulk_out    = 1,
    usbtest_bulk_in     = 2,
    usbtest_control_in  = 3
} usbtest;

// What I/O mechanism should be used on the target to process data?
typedef enum usb_io_mechanism {
    usb_io_mechanism_usb    = 1,        // The low-level USB-specific API
    usb_io_mechanism_dev    = 2         // cyg_devio_cread() et al
} usb_io_mechanism;

// Bulk transfers. The same structure can be used for IN and OUT transfers.
// The endpoint number will be or'd with either USB_DIR_IN or USB_DIR_OUT,
// or the equivalent under eCos.
typedef struct UsbTest_Bulk {
    int                 number_packets;
    int                 endpoint;
    int                 tx_size;
    int                 tx_size_min;
    int                 tx_size_max;
    int                 tx_size_multiplier;
    int                 tx_size_divisor;
    int                 tx_size_increment;
    int                 rx_size;
    int                 rx_size_min;
    int                 rx_size_max;
    int                 rx_size_multiplier;
    int                 rx_size_divisor;
    int                 rx_size_increment;
    int                 rx_padding;
    int                 tx_delay;
    int                 tx_delay_min;
    int                 tx_delay_max;
    int                 tx_delay_multiplier;
    int                 tx_delay_divisor;
    int                 tx_delay_increment;
    int                 rx_delay;
    int                 rx_delay_min;
    int                 rx_delay_max;
    int                 rx_delay_multiplier;
    int                 rx_delay_divisor;
    int                 rx_delay_increment;
    usb_io_mechanism    io_mechanism;
    UsbTestData         data;
} UsbTest_Bulk;

#ifdef HOST
static void
pack_usbtest_bulk(UsbTest_Bulk* test, unsigned char* buffer, int* index)
{
    pack_int(test->number_packets,          buffer, index);
    pack_int(test->endpoint,                buffer, index);
    pack_int(test->tx_size,                 buffer, index);
    pack_int(test->tx_size_min,             buffer, index);
    pack_int(test->tx_size_max,             buffer, index);
    pack_int(test->tx_size_multiplier,      buffer, index);
    pack_int(test->tx_size_divisor,         buffer, index);
    pack_int(test->tx_size_increment,       buffer, index);
    pack_int(test->rx_size,                 buffer, index);
    pack_int(test->rx_size_min,             buffer, index);
    pack_int(test->rx_size_max,             buffer, index);
    pack_int(test->rx_size_multiplier,      buffer, index);
    pack_int(test->rx_size_divisor,         buffer, index);
    pack_int(test->rx_size_increment,       buffer, index);
    // There is no need to transfer the padding field. It is only of
    // interest on the host, and this message is being packed
    // for the target side.
    pack_int(test->tx_delay,                buffer, index);
    pack_int(test->tx_delay_min,            buffer, index);
    pack_int(test->tx_delay_max,            buffer, index);
    pack_int(test->tx_delay_multiplier,     buffer, index);
    pack_int(test->tx_delay_divisor,        buffer, index);
    pack_int(test->tx_delay_increment,      buffer, index);
    pack_int(test->rx_delay,                buffer, index);
    pack_int(test->rx_delay_min,            buffer, index);
    pack_int(test->rx_delay_max,            buffer, index);
    pack_int(test->rx_delay_multiplier,     buffer, index);
    pack_int(test->rx_delay_divisor,        buffer, index);
    pack_int(test->rx_delay_increment,      buffer, index);
    pack_int((int)test->io_mechanism,       buffer, index);
    pack_usbtestdata(&(test->data),         buffer, index);
}
#endif

#ifdef TARGET
static void
unpack_usbtest_bulk(UsbTest_Bulk* test, unsigned char* buffer, int* index)
{
    test->number_packets            = unpack_int(buffer, index);
    test->endpoint                  = unpack_int(buffer, index);
    test->tx_size                   = unpack_int(buffer, index);
    test->tx_size_min               = unpack_int(buffer, index);
    test->tx_size_max               = unpack_int(buffer, index);
    test->tx_size_multiplier        = unpack_int(buffer, index);
    test->tx_size_divisor           = unpack_int(buffer, index);
    test->tx_size_increment         = unpack_int(buffer, index);
    test->rx_size                   = unpack_int(buffer, index);
    test->rx_size_min               = unpack_int(buffer, index);
    test->rx_size_max               = unpack_int(buffer, index);
    test->rx_size_multiplier        = unpack_int(buffer, index);
    test->rx_size_divisor           = unpack_int(buffer, index);
    test->rx_size_increment         = unpack_int(buffer, index);
    test->tx_delay                  = unpack_int(buffer, index);
    test->tx_delay_min              = unpack_int(buffer, index);
    test->tx_delay_max              = unpack_int(buffer, index);
    test->tx_delay_multiplier       = unpack_int(buffer, index);
    test->tx_delay_divisor          = unpack_int(buffer, index);
    test->tx_delay_increment        = unpack_int(buffer, index);
    test->rx_delay                  = unpack_int(buffer, index);
    test->rx_delay_min              = unpack_int(buffer, index);
    test->rx_delay_max              = unpack_int(buffer, index);
    test->rx_delay_multiplier       = unpack_int(buffer, index);
    test->rx_delay_divisor          = unpack_int(buffer, index);
    test->rx_delay_increment        = unpack_int(buffer, index);
    test->io_mechanism              = (usb_io_mechanism) unpack_int(buffer, index);
    unpack_usbtestdata(&(test->data), buffer, index);
}
#endif

// A macro for moving on the next packet size. This also has to be shared between host
// and target, if the two got out of synch then testing would go horribly wrong.
//
// The new packet size is determined using a multiplier and increment,
// so to e.g. increase packet sizes by 4 bytes each time the
// multiplier would be 1 and the increment would be 4, or to double
// packet sizes the multiplier would be 2 and the increment would be
// 0. On underflow or overflow the code tries to adjust the packet size
// back to within the accepted range.

#define USBTEST_NEXT_TX_SIZE(_x_)                               \
    do {                                                        \
        _x_.tx_size *= _x_.tx_size_multiplier;                  \
        _x_.tx_size /= _x_.tx_size_divisor;                     \
        _x_.tx_size += _x_.tx_size_increment;                   \
        if (_x_.tx_size < _x_.tx_size_min) {                    \
            if (_x_.tx_size_min == _x_.tx_size_max) {           \
                _x_.tx_size = _x_.tx_size_min;                  \
            } else {                                            \
                int tmp  = _x_.tx_size_min - _x_.tx_size;       \
                tmp     %= _x_.tx_size_max - _x_.tx_size_min;   \
                _x_.tx_size = tmp + _x_.tx_size_min;            \
            }                                                   \
        } else if (_x_.tx_size > _x_.tx_size_max) {             \
            if (_x_.tx_size_min == _x_.tx_size_max) {           \
                _x_.tx_size = _x_.tx_size_max;                  \
            } else {                                            \
                int tmp  = _x_.tx_size - _x_.tx_size_max;       \
                tmp     %= _x_.tx_size_max - _x_.tx_size_min;   \
                _x_.tx_size = tmp + _x_.tx_size_min;            \
            }                                                   \
        }                                                       \
    } while ( 0 )

// A similar macro for moving on to the next receive size. This is less
// critical since care is taken to always receive at least the current
// tx size plus padding.
// Note that padding needs to be added by the calling code, not here,
// since padding is only applicable on the host-side and this macro
// is used on both host and target.
#define USBTEST_NEXT_RX_SIZE(_x_)                               \
    do {                                                        \
        _x_.rx_size *= _x_.rx_size_multiplier;                  \
        _x_.rx_size /= _x_.rx_size_divisor;                     \
        _x_.rx_size += _x_.rx_size_increment;                   \
        if (_x_.rx_size < _x_.rx_size_min) {                    \
            if (_x_.rx_size_min == _x_.rx_size_max) {           \
                _x_.rx_size = _x_.rx_size_min;                  \
            } else {                                            \
                int tmp  = _x_.rx_size_min - _x_.rx_size;       \
                tmp     %= _x_.rx_size_max - _x_.rx_size_min;   \
                _x_.rx_size = tmp + _x_.rx_size_min;            \
            }                                                   \
        } else if (_x_.rx_size > _x_.rx_size_max) {             \
            if (_x_.rx_size_min == _x_.rx_size_max) {           \
                _x_.rx_size = _x_.rx_size_max;                  \
            } else {                                            \
                int tmp  = _x_.rx_size - _x_.rx_size_max;       \
                tmp     %= _x_.rx_size_max - _x_.rx_size_min;   \
                _x_.rx_size = tmp + _x_.rx_size_min;            \
            }                                                   \
        }                                                       \
    } while ( 0 )

// And a macro for adjusting the transmit delay.
#define USBTEST_NEXT_TX_DELAY(_x_)                              \
    do {                                                        \
        _x_.tx_delay *= _x_.tx_delay_multiplier;                \
        _x_.tx_delay /= _x_.tx_delay_divisor;                   \
        _x_.tx_delay += _x_.tx_delay_increment;                 \
        if (_x_.tx_delay < _x_.tx_delay_min) {                  \
            if (_x_.tx_delay_min == _x_.tx_delay_max) {         \
                _x_.tx_delay = _x_.tx_delay_min;                \
            } else {                                            \
                int tmp  = _x_.tx_delay_min - _x_.tx_delay;     \
                tmp     %= _x_.tx_delay_max - _x_.tx_delay_min; \
                _x_.tx_delay = tmp + _x_.tx_delay_min;          \
            }                                                   \
        } else if (_x_.tx_delay > _x_.tx_delay_max) {           \
            if (_x_.tx_delay_min == _x_.tx_delay_max) {         \
                _x_.tx_delay = _x_.tx_delay_max;                \
            } else {                                            \
                int tmp  = _x_.tx_delay - _x_.tx_delay_max;     \
                tmp     %= _x_.tx_delay_max - _x_.tx_delay_min; \
                _x_.tx_delay = tmp + _x_.tx_delay_min;          \
            }                                                   \
        }                                                       \
    } while ( 0 )

#define USBTEST_NEXT_RX_DELAY(_x_)                              \
    do {                                                        \
        _x_.rx_delay *= _x_.rx_delay_multiplier;                \
        _x_.rx_delay /= _x_.rx_delay_divisor;                   \
        _x_.rx_delay += _x_.rx_delay_increment;                 \
        if (_x_.rx_delay < _x_.rx_delay_min) {                  \
            if (_x_.rx_delay_min == _x_.rx_delay_max) {         \
                _x_.rx_delay = _x_.rx_delay_min;                \
            } else {                                            \
                int tmp  = _x_.rx_delay_min - _x_.rx_delay;     \
                tmp     %= _x_.rx_delay_max - _x_.rx_delay_min; \
                _x_.rx_delay = tmp + _x_.rx_delay_min;          \
            }                                                   \
        } else if (_x_.rx_delay > _x_.rx_delay_max) {           \
            if (_x_.rx_delay_min == _x_.rx_delay_max) {         \
                _x_.rx_delay = _x_.rx_delay_max;                \
            } else {                                            \
                int tmp  = _x_.rx_delay - _x_.rx_delay_max;     \
                tmp     %= _x_.rx_delay_max - _x_.rx_delay_min; \
                _x_.rx_delay = tmp + _x_.rx_delay_min;          \
            }                                                   \
        }                                                       \
    } while ( 0 )

#define USBTEST_BULK_NEXT(_bulk_)                               \
    USBTEST_NEXT_TX_SIZE(_bulk_);                               \
    USBTEST_NEXT_RX_SIZE(_bulk_);                               \
    USBTEST_NEXT_TX_DELAY(_bulk_);                              \
    USBTEST_NEXT_RX_DELAY(_bulk_);

// Control transfers, receives
typedef struct UsbTest_ControlIn {
    int         number_packets;
    int         packet_size_initial;
    int         packet_size_min;
    int         packet_size_max;
    int         packet_size_multiplier;
    int         packet_size_increment;
    UsbTestData data;
} UsbTest_ControlIn;

#ifdef HOST
static void
pack_usbtest_control_in(UsbTest_ControlIn* test, unsigned char* buffer, int* index)
{
    pack_int(test->number_packets,          buffer, index);
    pack_int(test->packet_size_initial,     buffer, index);
    pack_int(test->packet_size_min,         buffer, index);
    pack_int(test->packet_size_max,         buffer, index);
    pack_int(test->packet_size_multiplier,  buffer, index);
    pack_int(test->packet_size_increment,   buffer, index);
    pack_usbtestdata(&(test->data),         buffer, index);
}
#endif

#ifdef TARGET
static void
unpack_usbtest_control_in(UsbTest_ControlIn* test, unsigned char* buffer, int* index)
{
    test->number_packets            = unpack_int(buffer, index);
    test->packet_size_initial       = unpack_int(buffer, index);
    test->packet_size_min           = unpack_int(buffer, index);
    test->packet_size_max           = unpack_int(buffer, index);
    test->packet_size_multiplier    = unpack_int(buffer, index);
    test->packet_size_increment     = unpack_int(buffer, index);
    unpack_usbtestdata(&(test->data), buffer, index);
}
#endif

// For now control packet sizes are adjusted in exactly the same way as bulk transfers.
#define USBTEST_CONTROL_NEXT_PACKET_SIZE(_packet_size_, _control_)                                          \
    _packet_size_ = (_packet_size_ * _control_.packet_size_multiplier) + _control_.packet_size_increment;   \
    if (_packet_size_ < _control_.packet_size_min) {                                                        \
        _packet_size_ += _control_.packet_size_max - _control_.packet_size_min;                             \
        if (_packet_size_ < _control_.packet_size_min) {                                                    \
            _packet_size_ = _control_.packet_size_initial;                                                  \
        }                                                                                                   \
    } else if (_packet_size_ > _control_.packet_size_max) {                                                 \
        _packet_size_ -= _control_.packet_size_max - _control_.packet_size_min;                             \
        if (_packet_size_ > _control_.packet_size_max) {                                                    \
            _packet_size_ = _control_.packet_size_initial;                                                  \
        }                                                                                                   \
    }

/*}}}*/
/*{{{  Recovery support                                         */

// ----------------------------------------------------------------------------
// When things go wrong threads on either the host or the target may get
// locked up waiting for further communication that never happens, because
// the other side has already raised an error. Recovery is possible by
// performing an extra I/O operation. For example, if a thread on the
// target is blocked waiting on an OUT endpoint then recovery is possible
// by the host sending some data to that endpoint. Similarly if a thread
// on the host is blocked then recovery involves the target either sending
// or receiving some additional data. There are alternative approaches such
// as stalling endpoints, but making sure that the requested communication
// actually happens involves fewer dependencies on exactly how those
// operations behave.

typedef struct UsbTest_Recovery {
    int     endpoint;       // Top bit indicates direction, -1 indicates invalid
    int     protocol;
    int     size;
} UsbTest_Recovery;

static void
pack_usbtest_recovery(UsbTest_Recovery* recovery, unsigned char* buffer, int* index)
{
    pack_int(recovery->endpoint, buffer, index);
    pack_int(recovery->protocol, buffer, index);
    pack_int(recovery->size,     buffer, index);
}

static void
unpack_usbtest_recovery(UsbTest_Recovery* recovery, unsigned char* buffer, int *index)
{
    recovery->endpoint  = unpack_int(buffer, index);
    recovery->protocol  = unpack_int(buffer, index);
    recovery->size      = unpack_int(buffer, index);
}

static void
usbtest_recovery_reset(UsbTest_Recovery* recovery)
{
    recovery->endpoint  = -1;
    recovery->protocol  = 0;
    recovery->size      = 0;
}

/*}}}*/
