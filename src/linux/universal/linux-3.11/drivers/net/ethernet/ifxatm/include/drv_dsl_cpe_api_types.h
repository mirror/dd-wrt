/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_TYPES_H
#define _DRV_DSL_CPE_TYPES_H

/** \file
   Basic data types.
*/

/** \addtogroup DRV_DSL_CPE_TYPES
 @{ */

/** This is the character datatype. */
typedef char            DSL_char_t;
/** This is the unsigned 8-bit datatype. */
typedef unsigned char   DSL_uint8_t;
/** This is the signed 8-bit datatype. */
typedef signed char     DSL_int8_t;
/** This is the unsigned 16-bit datatype. */
typedef unsigned short  DSL_uint16_t;
/** This is the signed 16-bit datatype. */
typedef signed short    DSL_int16_t;
/** This is the unsigned 32-bit datatype. */
typedef unsigned int    DSL_uint32_t;
/** This is the signed 32-bit datatype. */
typedef signed int      DSL_int32_t;
/** This is the float datatype. */
typedef float           DSL_float_t;
/** This is the void datatype. */
typedef void            DSL_void_t;
/** integer type, width is depending on processor arch */
typedef int             DSL_int_t;
typedef unsigned long   DSL_ulong_t;
/** unsigned integer type, width is depending on processor arch */
typedef unsigned int    DSL_uint_t;
/** This is the time data type */
typedef DSL_uint32_t    DSL_time_t;


#if defined(WIN32)
   /* VC6 and VC7 or GCC < 2.8 */
   typedef DSL_uint16_t    DSL_bf16_t;
#else
   #if !defined(__GNUC__) || !defined(__GNUC_MINOR__)
      typedef DSL_uint32_t    DSL_bf16_t;
   #elif (__GNUC__ == 2 && __GNUC_MINOR__ <= 7)
      /* VC6 and VC7 or GCC < 2.8 */
      typedef DSL_uint32_t    DSL_bf16_t;
   #else
      typedef DSL_uint32_t    DSL_bf16_t;
   #endif
#endif

/** This defines a bitfield with 32 bit. */
typedef DSL_uint32_t    DSL_bf32_t;

/** This is the volatile unsigned 8-bit datatype. */
typedef volatile DSL_uint8_t  DSL_vuint8_t;
/** This is the volatile signed 8-bit datatype. */
typedef volatile DSL_int8_t   DSL_vint8_t;
/** This is the volatile unsigned 16-bit datatype. */
typedef volatile DSL_uint16_t DSL_vuint16_t;
/** This is the volatile signed 16-bit datatype. */
typedef volatile DSL_int16_t  DSL_vint16_t;
/** This is the volatile unsigned 32-bit datatype. */
typedef volatile DSL_uint32_t DSL_vuint32_t;
/** This is the volatile signed 32-bit datatype. */
typedef volatile DSL_int32_t  DSL_vint32_t;
/** This is the volatile float datatype. */
typedef volatile DSL_float_t  DSL_vfloat_t;


/** A type for handling boolean issues. */
typedef enum {
   /** false */
   DSL_FALSE = 0,
   /** true */
   DSL_TRUE = 1
} DSL_boolean_t;


/**
   This type is used for parameters that should enable
   and disable a dedicated feature. */
typedef enum {
   /** disable */
   DSL_DISABLE = 0,
   /** enable */
   DSL_ENABLE = 1
} DSL_enDis_t;


/**
   This type has two states, even and odd.
*/
typedef enum {
   /** even */
   DSL_EVEN = 0,
   /** odd */
   DSL_ODD = 1
} DSL_evenOdd_t;


/**
   This type has two states, high and low.
*/
typedef enum {
   /** low */
   DSL_LOW = 0,
   /** high */
   DSL_HIGH = 1
} DSL_highLow_t;

/**
   Null pointer.
*/
#define DSL_NULL         ((void *)0)

/** @} */

#endif /* _DRV_DSL_CPE_TYPES_H */

