//=================================================================
//
//        crc_test.c
//
//        crc test cases
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Andrew Lunn
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
// Author(s):     asl
// Contributors:  asl
// Date:          2002-08-06
// Description:   Tests the calculation code
//####DESCRIPTIONEND####

#include <cyg/infra/testcase.h>
#include <cyg/crc/crc.h>

static char license_txt[] = 
"		    GNU GENERAL PUBLIC LICENSE\n"
"		       Version 2, June 1991\n"
"\n"
" Copyright (C) 1989, 1991 Free Software Foundation, Inc.\n"
"     59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
" Everyone is permitted to copy and distribute verbatim copies\n"
" of this license document, but changing it is not allowed.\n"
"\n"
"			    Preamble\n"
"\n"
"  The licenses for most software are designed to take away your\n"
"freedom to share and change it.  By contrast, the GNU General Public\n"
"License is intended to guarantee your freedom to share and change free\n"
"software--to make sure the software is free for all its users.  This\n"
"General Public License applies to most of the Free Software\n"
"Foundation's software and to any other program whose authors commit to\n"
"using it.  (Some other Free Software Foundation software is covered by\n"
"the GNU Library General Public License instead.)  You can apply it to\n"
"your programs, too.";

externC void
cyg_start( void )
{
  unsigned long crc1,crc2;
   
  CYG_TEST_INIT();
  
  CYG_TEST_INFO("Calculating CRCs");
 
  if (1500790746UL != cyg_posix_crc32(license_txt,sizeof(license_txt)-1)) {
    CYG_TEST_FAIL("Wrong POSIX CRC32 calculation");
  } else {
    CYG_TEST_PASS("POSIX CRC32 calculation");
  }
  
  if (1667500021UL != cyg_ether_crc32(license_txt,sizeof(license_txt)-1)) {
    CYG_TEST_FAIL("Wrong Ethernet crc32 calculation");
  } else {
    CYG_TEST_PASS("Ethernet crc32 calculation");
  }
  
  if (0 != cyg_ether_crc32_accumulate(0,0,0)) {
    CYG_TEST_FAIL("Ethernet crc32 accumulate setup");
  } else {
    crc1= cyg_ether_crc32_accumulate(0, license_txt,sizeof(license_txt)-1);
    crc2 = cyg_ether_crc32_accumulate(crc1, license_txt,sizeof(license_txt)-1);
    
    if ((1667500021UL != crc1) || (3478736840UL != crc2)) {
      CYG_TEST_FAIL("Wrong Etheret crc32 accumulate");
    } else {
      CYG_TEST_PASS("Ethernet crc32_accumulate");
    }
  }

  if (1247800780UL != cyg_crc32(license_txt,sizeof(license_txt)-1)) {
    CYG_TEST_FAIL("Wrong Gary S. Browns' crc32 calculation");
  } else {
    CYG_TEST_PASS("Gary S. Browns' crc32 calculation");
  }

  crc1 = cyg_crc32_accumulate(0,license_txt,sizeof(license_txt)-1);
  crc2 = cyg_crc32_accumulate(crc1,license_txt,sizeof(license_txt)-1);
    
  if ((1247800780UL != crc1) || (926002294UL != crc2)) {
    CYG_TEST_FAIL("Wrong Gary S. Browns' crc32 accumulate calculation");
  } else {
    CYG_TEST_PASS("Gary S. Browns' crc32 accumulate calculation");
  }
    
  if (32256UL != cyg_crc16(license_txt,sizeof(license_txt)-1)) {
    CYG_TEST_FAIL_FINISH("Wrong 16bit CRC calculation");
  } else {
    CYG_TEST_PASS_FINISH("16bit CRC calculation");
  }
}

  

