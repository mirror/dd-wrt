
#ifndef VSTR__HEADER_H
# error " You must _just_ #include <vstr.h>"
#endif
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */

/* macro functions to make flags readable ... don't forget that expantion
 * happens on second macro call */
#define VSTR__FLAG01(B, x1) \
   B ## x1
#define VSTR__FLAG02(B, x1, x2) ( \
 ( B ## x1 ) | \
 ( B ## x2 ) | \
 0)
#define VSTR__FLAG04(B, x1, x2, x3, x4) ( \
 ( B ## x1 ) | \
 ( B ## x2 ) | \
 ( B ## x3 ) | \
 ( B ## x4 ) | \
 0)

/* Note none of these are documented seperately ... so they have the
 * space there to fool the documentation checker */
# define VSTR_FLAG01(T, x1) ( \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## x1 ) | \
 0)
# define VSTR_FLAG02(T, x1, x2) ( \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 ) | \
 0)
# define VSTR_FLAG03(T, x1, x2, x3) ( \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## x3 ) | \
 0)
# define VSTR_FLAG04(T, x1, x2, x3, x4) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 0)
# define VSTR_FLAG05(T, x1, x2, x3, x4, x5) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## x5 ) | \
 0)
# define VSTR_FLAG06(T, x1, x2, x3, x4, x5, x6) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 ) | \
 0)
# define VSTR_FLAG07(T, x1, x2, x3, x4, x5, x6, x7) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## x7 ) | \
 0)
# define VSTR_FLAG08(T, x1, x2, x3, x4, x5, x6, x7, x8) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 0)
# define VSTR_FLAG09(T, x1, x2, x3, x4, x5, x6, x7, x8, x9) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## x9 ) | \
 0)
# define VSTR_FLAG10(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA ) | \
 0)
# define VSTR_FLAG11(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xB ) | \
 0)
# define VSTR_FLAG12(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 0)
# define VSTR_FLAG13(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xD ) | \
 0)
# define VSTR_FLAG14(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xD , _ ## xE ) | \
 0)
# define VSTR_FLAG15(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xD , _ ## xE ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xF ) | \
 0)
# define VSTR_FLAG16(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 0)
# define VSTR_FLAG17(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xH ) | \
 0)
# define VSTR_FLAG18(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xH , _ ## xI ) | \
 0)
# define VSTR_FLAG19(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xH , _ ## xI ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xJ ) | \
 0)
# define VSTR_FLAG20(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 0)
# define VSTR_FLAG21(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xL ) | \
 0)
# define VSTR_FLAG22(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xL , _ ## xM ) | \
 0)
# define VSTR_FLAG23(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xL , _ ## xM ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xN ) | \
 0)
# define VSTR_FLAG24(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN, xO) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xL , _ ## xM , _ ## xN , _ ## xO ) | \
 0)
# define VSTR_FLAG25(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN, xO, xP) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xL , _ ## xM , _ ## xN , _ ## xO ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xP ) | \
 0)
# define VSTR_FLAG26(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN, xO, xP, xQ) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xL , _ ## xM , _ ## xN , _ ## xO ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xP , _ ## xQ ) | \
 0)
# define VSTR_FLAG27(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN, xO, xP, xQ, xR) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xL , _ ## xM , _ ## xN , _ ## xO ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xP , _ ## xQ ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xR ) | \
 0)
# define VSTR_FLAG28(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN, xO, xP, xQ, xR, xS) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xL , _ ## xM , _ ## xN , _ ## xO ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xP , _ ## xQ , _ ## xR , _ ## xS ) | \
 0)
# define VSTR_FLAG29(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN, xO, xP, xQ, xR, xS, xT) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xL , _ ## xM , _ ## xN , _ ## xO ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xP , _ ## xQ , _ ## xR , _ ## xS ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xT ) | \
 0)
# define VSTR_FLAG30(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN, xO, xP, xQ, xR, xS, xT, xU) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xL , _ ## xM , _ ## xN , _ ## xO ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xP , _ ## xQ , _ ## xR , _ ## xS ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xT , _ ## xU ) | \
 0)
# define VSTR_FLAG31(T, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF, xG, xH, xI, xJ, xK, xL, xM, xN, xO, xP, xQ, xR, xS, xT, xU, xV) ( \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x1 , _ ## x2 , _ ## x3 , _ ## x4 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x5 , _ ## x6 , _ ## x7 , _ ## x8 ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## x9 , _ ## xA , _ ## xB , _ ## xC ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xD , _ ## xE , _ ## xF , _ ## xG ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xH , _ ## xI , _ ## xJ , _ ## xK ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xL , _ ## xM , _ ## xN , _ ## xO ) | \
 VSTR__FLAG04( VSTR_FLAG_ ## T , _ ## xP , _ ## xQ , _ ## xR , _ ## xS ) | \
 VSTR__FLAG02( VSTR_FLAG_ ## T , _ ## xT , _ ## xU ) | \
 VSTR__FLAG01( VSTR_FLAG_ ## T , _ ## xV ) | \
 0)

/* start of constants ... */
#define VSTR_TYPE_NODE_BUF 1
#define VSTR_TYPE_NODE_NON 2
#define VSTR_TYPE_NODE_PTR 3
#define VSTR_TYPE_NODE_REF 4

#define VSTR_TYPE_ITER_DEF 0
#define VSTR_TYPE_ITER_END 1
#define VSTR_TYPE_ITER_NON 2

#define VSTR_TYPE_ADD_DEF 0
#define VSTR_TYPE_ADD_BUF_PTR 1
#define VSTR_TYPE_ADD_BUF_REF 2
#define VSTR_TYPE_ADD_ALL_REF 3
#define VSTR_TYPE_ADD_ALL_BUF 4

#define VSTR_TYPE_FMT_END 0
/* uchar/ushort -- internal */
#define VSTR_TYPE_FMT_INT 3
#define VSTR_TYPE_FMT_UINT 4
#define VSTR_TYPE_FMT_LONG 5
#define VSTR_TYPE_FMT_ULONG 6
#define VSTR_TYPE_FMT_LONG_LONG 7
#define VSTR_TYPE_FMT_ULONG_LONG 8
#define VSTR_TYPE_FMT_SSIZE_T 9
#define VSTR_TYPE_FMT_SIZE_T 10
#define VSTR_TYPE_FMT_PTRDIFF_T 11
#define VSTR_TYPE_FMT_INTMAX_T 12
#define VSTR_TYPE_FMT_UINTMAX_T 13
#define VSTR_TYPE_FMT_DOUBLE 14
#define VSTR_TYPE_FMT_DOUBLE_LONG 15
#define VSTR_TYPE_FMT_PTR_VOID 16
#define VSTR_TYPE_FMT_PTR_CHAR 17
#define VSTR_TYPE_FMT_PTR_WCHAR_T 18
#define VSTR_TYPE_FMT_ERRNO 20
#define VSTR_TYPE_FMT_PTR_SIGNED_CHAR 21
#define VSTR_TYPE_FMT_PTR_SHORT 22
#define VSTR_TYPE_FMT_PTR_INT 23
#define VSTR_TYPE_FMT_PTR_LONG 24
#define VSTR_TYPE_FMT_PTR_LONG_LONG 25
#define VSTR_TYPE_FMT_PTR_SSIZE_T 26
#define VSTR_TYPE_FMT_PTR_PTRDIFF_T 27
#define VSTR_TYPE_FMT_PTR_INTMAX_T 28

/* aliases to make life more readable */
#define VSTR_TYPE_SUB_DEF VSTR_TYPE_ADD_DEF
#define VSTR_TYPE_SUB_BUF_PTR VSTR_TYPE_ADD_BUF_PTR
#define VSTR_TYPE_SUB_BUF_REF VSTR_TYPE_ADD_BUF_REF
#define VSTR_TYPE_SUB_ALL_REF VSTR_TYPE_ADD_ALL_REF
#define VSTR_TYPE_SUB_ALL_BUF VSTR_TYPE_ADD_ALL_BUF

#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_NONE   0U
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_NUL   (1U<<0)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_BEL   (1U<<1)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_BS    (1U<<2)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_HT    (1U<<3)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_LF    (1U<<4)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_VT    (1U<<5)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_FF    (1U<<6)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_CR    (1U<<7)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_SP    (1U<<8)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_COMMA (1U<<9)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_DOT   (1U<<9)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW__     (1U<<10)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_ESC   (1U<<11)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_DEL   (1U<<12)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_HSP   (1U<<13)
#define VSTR_FLAG_CONV_UNPRINTABLE_ALLOW_HIGH  (1U<<14)
#define VSTR_FLAG_CONV_UNPRINTABLE_DEF \
 VSTR_FLAG04(CONV_UNPRINTABLE_ALLOW, SP, COMMA, DOT, _)

#define VSTR_TYPE_PARSE_NUM_ERR_NONE 0
#define VSTR_TYPE_PARSE_NUM_ERR_ONLY_S 1
#define VSTR_TYPE_PARSE_NUM_ERR_ONLY_SPM 2
#define VSTR_TYPE_PARSE_NUM_ERR_ONLY_SPMX 3
#define VSTR_TYPE_PARSE_NUM_ERR_OOB 4
#define VSTR_TYPE_PARSE_NUM_ERR_OVERFLOW 5
#define VSTR_TYPE_PARSE_NUM_ERR_NEGATIVE 6
#define VSTR_TYPE_PARSE_NUM_ERR_BEG_ZERO 7

#define VSTR_FLAG_PARSE_NUM_DEF          0U
#define VSTR__MASK_PARSE_NUM_BASE (63) /* (1<<6) - 1 */
#define VSTR_FLAG_PARSE_NUM_LOCAL       (1U<<6)
#define VSTR_FLAG_PARSE_NUM_SEP         (1U<<7)
#define VSTR_FLAG_PARSE_NUM_OVERFLOW    (1U<<8)
#define VSTR_FLAG_PARSE_NUM_SPACE       (1U<<9)
#define VSTR_FLAG_PARSE_NUM_NO_BEG_ZERO (1U<<10)
#define VSTR_FLAG_PARSE_NUM_NO_BEG_PM   (1U<<11)
#define VSTR_FLAG_PARSE_NUM_NO_NEGATIVE (1U<<12)
/* FIXME: #define VSTR_FLAG_PARSE_NUM_LOC_SEP ???? */

#define VSTR_TYPE_PARSE_IPV4_ERR_NONE 0
#define VSTR_TYPE_PARSE_IPV4_ERR_IPV4_OOB 1
#define VSTR_TYPE_PARSE_IPV4_ERR_IPV4_FULL 2
#define VSTR_TYPE_PARSE_IPV4_ERR_CIDR_OOB 3
#define VSTR_TYPE_PARSE_IPV4_ERR_CIDR_FULL 4
#define VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_OOB 5
#define VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_FULL 6
#define VSTR_TYPE_PARSE_IPV4_ERR_ONLY 7

#define VSTR_FLAG_PARSE_IPV4_DEF           0U
#define VSTR_FLAG_PARSE_IPV4_LOCAL        (1U<<0)
#define VSTR_FLAG_PARSE_IPV4_ZEROS        (1U<<1)
#define VSTR_FLAG_PARSE_IPV4_FULL         (1U<<2)
#define VSTR_FLAG_PARSE_IPV4_CIDR         (1U<<3)
#define VSTR_FLAG_PARSE_IPV4_CIDR_FULL    (1U<<4)
#define VSTR_FLAG_PARSE_IPV4_NETMASK      (1U<<5)
#define VSTR_FLAG_PARSE_IPV4_NETMASK_FULL (1U<<6)
#define VSTR_FLAG_PARSE_IPV4_ONLY         (1U<<7)

#define VSTR_TYPE_PARSE_IPV6_ERR_NONE 0
#define VSTR_TYPE_PARSE_IPV6_ERR_IPV6_OOB 1
#define VSTR_TYPE_PARSE_IPV6_ERR_IPV6_NULL 2
#define VSTR_TYPE_PARSE_IPV6_ERR_IPV6_FULL 3
#define VSTR_TYPE_PARSE_IPV6_ERR_CIDR_OOB 4
#define VSTR_TYPE_PARSE_IPV6_ERR_CIDR_FULL 5
#define VSTR_TYPE_PARSE_IPV6_ERR_ONLY 6

#define VSTR_FLAG_PARSE_IPV6_DEF           0U
#define VSTR_FLAG_PARSE_IPV6_LOCAL        (1U<<0)
#define VSTR_FLAG_PARSE_IPV6_CIDR         (1U<<1)
#define VSTR_FLAG_PARSE_IPV6_CIDR_FULL    (1U<<2)
#define VSTR_FLAG_PARSE_IPV6_ONLY         (1U<<3)

#define VSTR_FLAG_SPLIT_DEF        0U
#define VSTR_FLAG_SPLIT_BEG_NULL  (1U<<0)
#define VSTR_FLAG_SPLIT_MID_NULL  (1U<<1)
#define VSTR_FLAG_SPLIT_END_NULL  (1U<<2)
#define VSTR_FLAG_SPLIT_POST_NULL (1U<<3)
#define VSTR_FLAG_SPLIT_NO_RET    (1U<<4)
#define VSTR_FLAG_SPLIT_REMAIN    (1U<<5)

#define VSTR_FLAG_SECTS_FOREACH_DEF         0U
#define VSTR_FLAG_SECTS_FOREACH_BACKWARD   (1U<<0)
#define VSTR_FLAG_SECTS_FOREACH_ALLOW_NULL (1U<<1)

#define VSTR_TYPE_SECTS_FOREACH_DEF 0
#define VSTR_TYPE_SECTS_FOREACH_DEL 1
#define VSTR_TYPE_SECTS_FOREACH_RET 2

#define VSTR_TYPE_CACHE_ADD 1
#define VSTR_TYPE_CACHE_DEL 2
#define VSTR_TYPE_CACHE_SUB 3
#define VSTR_TYPE_CACHE_FREE 4
/* #define VSTR_TYPE_CACHE_LOC 5 */

#define VSTR_TYPE_SC_MMAP_FD_ERR_NONE 0
#define VSTR_TYPE_SC_MMAP_FD_ERR_FSTAT_ERRNO 2
#define VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO 3
#define VSTR_TYPE_SC_MMAP_FD_ERR_MEM 5
#define VSTR_TYPE_SC_MMAP_FD_ERR_TOO_LARGE 6

#define VSTR_TYPE_SC_MMAP_FILE_ERR_NONE 0
#define VSTR_TYPE_SC_MMAP_FILE_ERR_OPEN_ERRNO 1
#define VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO 2
#define VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO 3
#define VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO 4
#define VSTR_TYPE_SC_MMAP_FILE_ERR_MEM 5
#define VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE 6

#define VSTR_TYPE_SC_READ_FD_ERR_NONE 0
#define VSTR_TYPE_SC_READ_FD_ERR_FSTAT_ERRNO 2
#define VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO 4
#define VSTR_TYPE_SC_READ_FD_ERR_EOF 6
#define VSTR_TYPE_SC_READ_FD_ERR_MEM 7
#define VSTR_TYPE_SC_READ_FD_ERR_TOO_LARGE 8

#define VSTR_TYPE_SC_READ_FILE_ERR_NONE 0
#define VSTR_TYPE_SC_READ_FILE_ERR_OPEN_ERRNO 1
#define VSTR_TYPE_SC_READ_FILE_ERR_FSTAT_ERRNO 2
#define VSTR_TYPE_SC_READ_FILE_ERR_SEEK_ERRNO 3
#define VSTR_TYPE_SC_READ_FILE_ERR_READ_ERRNO 4
#define VSTR_TYPE_SC_READ_FILE_ERR_CLOSE_ERRNO 5
#define VSTR_TYPE_SC_READ_FILE_ERR_EOF 6
#define VSTR_TYPE_SC_READ_FILE_ERR_MEM 7
#define VSTR_TYPE_SC_READ_FILE_ERR_TOO_LARGE 8

#define VSTR_TYPE_SC_WRITE_FD_ERR_NONE 0
#define VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO 3
#define VSTR_TYPE_SC_WRITE_FD_ERR_MEM 5

#define VSTR_TYPE_SC_WRITE_FILE_ERR_NONE 0
#define VSTR_TYPE_SC_WRITE_FILE_ERR_OPEN_ERRNO 1
#define VSTR_TYPE_SC_WRITE_FILE_ERR_SEEK_ERRNO 2
#define VSTR_TYPE_SC_WRITE_FILE_ERR_WRITE_ERRNO 3
#define VSTR_TYPE_SC_WRITE_FILE_ERR_CLOSE_ERRNO 4
#define VSTR_TYPE_SC_WRITE_FILE_ERR_MEM 5

#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR      (1U<<1)
#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM     (1U<<2)
#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM      (1U<<3)
#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NEG      (1U<<4)
#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_HEXNUM_L (1U<<5)
#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_HEXNUM_H (1U<<6)
#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_OCTNUM   (1U<<7)
#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_BINNUM_L (1U<<8)
#define VSTR_FLAG_SC_FMT_CB_BEG_OBJ_BINNUM_H (1U<<9)
#define VSTR_FLAG_SC_FMT_CB_BEG_DEF VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM

#define VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED      (1U)
#define VSTR_TYPE_SC_FMT_CB_IPV6_STD          (2U)
#define VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT      (3U)
#define VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED (5U)
#define VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_STD     (6U)
#define VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT (7U)

#define VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE  (0U)
#define VSTR_TYPE_CNTL_CONF_GRPALLOC_POS   (1U)
#define VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC (2U)
#define VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR  (3U)

#define VSTR_MAX_NODE_BUF 0xFFFF /* used is 16 bits */
#define VSTR_MAX_NODE_ALL 0xFFFFFFF /* node->len is 28 bits */

#define VSTR__CNTL(x, y) ((VSTR__CNTL_ ## x ## _OFFSET) + (y))

#define VSTR__CNTL_OPT_OFFSET 4000
#define VSTR__CNTL_BASE_OFFSET 5000
#define VSTR__CNTL_CONF_OFFSET 6000

#define VSTR_CNTL_OPT_GET_CONF VSTR__CNTL(OPT, 1)
#define VSTR_CNTL_OPT_SET_CONF VSTR__CNTL(OPT, 2)

#define VSTR_CNTL_BASE_GET_CONF VSTR__CNTL(BASE, 1)
#define VSTR_CNTL_BASE_SET_CONF VSTR__CNTL(BASE, 2)
#define VSTR_CNTL_BASE_GET_FLAG_HAVE_CACHE VSTR__CNTL(BASE, 3)
/* #define VSTR_CNTL_BASE_SET_FLAG_HAVE_CACHE VSTR__CNTL(BASE, 4) */
#define VSTR_CNTL_BASE_GET_TYPE_GRPALLOC_CACHE VSTR__CNTL(BASE, 4)
/* #define VSTR_CNTL_BASE_SET_TYPE_GRPALLOC_CACHE VSTR__CNTL(BASE, 5) */

#define VSTR_CNTL_CONF_GET_NUM_REF VSTR__CNTL(CONF, 1)
/* #define VSTR_CNTL_CONF_SET_NUM_REF VSTR__CNTL(CONF, 2) */
#define VSTR_CNTL_CONF_GET_NUM_IOV_MIN_ALLOC VSTR__CNTL(CONF, 3)
#define VSTR_CNTL_CONF_SET_NUM_IOV_MIN_ALLOC VSTR__CNTL(CONF, 4)
#define VSTR_CNTL_CONF_GET_NUM_IOV_MIN_OFFSET VSTR__CNTL(CONF, 5)
#define VSTR_CNTL_CONF_SET_NUM_IOV_MIN_OFFSET VSTR__CNTL(CONF, 6)
#define VSTR_CNTL_CONF_GET_NUM_BUF_SZ VSTR__CNTL(CONF, 7)
#define VSTR_CNTL_CONF_SET_NUM_BUF_SZ VSTR__CNTL(CONF, 8)
/* #define VSTR_CNTL_CONF_GET_LOC_CSTR_AUTO_NAME_NUMERIC VSTR__CNTL(CONF, 9) */
#define VSTR_CNTL_CONF_SET_LOC_CSTR_AUTO_NAME_NUMERIC VSTR__CNTL(CONF, 10)
#define VSTR_CNTL_CONF_GET_LOC_CSTR_NAME_NUMERIC VSTR__CNTL(CONF, 11)
#define VSTR_CNTL_CONF_SET_LOC_CSTR_NAME_NUMERIC VSTR__CNTL(CONF, 12)
#define VSTR_CNTL_CONF_GET_LOC_CSTR_DEC_POINT VSTR__CNTL(CONF, 13)
#define VSTR_CNTL_CONF_SET_LOC_CSTR_DEC_POINT VSTR__CNTL(CONF, 14)
#define VSTR_CNTL_CONF_GET_LOC_CSTR_THOU_SEP VSTR__CNTL(CONF, 15)
#define VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_SEP VSTR__CNTL(CONF, 16)
#define VSTR_CNTL_CONF_GET_LOC_CSTR_THOU_GRP VSTR__CNTL(CONF, 17)
#define VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP VSTR__CNTL(CONF, 18)
#define VSTR_CNTL_CONF_GET_FLAG_IOV_UPDATE VSTR__CNTL(CONF, 19)
#define VSTR_CNTL_CONF_SET_FLAG_IOV_UPDATE VSTR__CNTL(CONF, 20)
#define VSTR_CNTL_CONF_GET_FLAG_DEL_SPLIT VSTR__CNTL(CONF, 21)
#define VSTR_CNTL_CONF_SET_FLAG_DEL_SPLIT VSTR__CNTL(CONF, 22)
#define VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE VSTR__CNTL(CONF, 23)
#define VSTR_CNTL_CONF_SET_FLAG_ALLOC_CACHE VSTR__CNTL(CONF, 24)
#define VSTR_CNTL_CONF_GET_FMT_CHAR_ESC VSTR__CNTL(CONF, 25)
#define VSTR_CNTL_CONF_SET_FMT_CHAR_ESC VSTR__CNTL(CONF, 26)
#define VSTR_CNTL_CONF_GET_NUM_SPARE_BUF VSTR__CNTL(CONF, 27)
#define VSTR_CNTL_CONF_SET_NUM_SPARE_BUF VSTR__CNTL(CONF, 28)
#define VSTR_CNTL_CONF_GET_NUM_SPARE_NON VSTR__CNTL(CONF, 29)
#define VSTR_CNTL_CONF_SET_NUM_SPARE_NON VSTR__CNTL(CONF, 30)
#define VSTR_CNTL_CONF_GET_NUM_SPARE_PTR VSTR__CNTL(CONF, 31)
#define VSTR_CNTL_CONF_SET_NUM_SPARE_PTR VSTR__CNTL(CONF, 32)
#define VSTR_CNTL_CONF_GET_NUM_SPARE_REF VSTR__CNTL(CONF, 33)
#define VSTR_CNTL_CONF_SET_NUM_SPARE_REF VSTR__CNTL(CONF, 34)
#define VSTR_CNTL_CONF_GET_FLAG_ATOMIC_OPS VSTR__CNTL(CONF, 35)
#define VSTR_CNTL_CONF_SET_FLAG_ATOMIC_OPS VSTR__CNTL(CONF, 36)
/* #define VSTR_CNTL_CONF_GET_NUM_RANGE_SPARE_BUF VSTR__CNTL(CONF, 37) */
#define VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF VSTR__CNTL(CONF, 38)
/* #define VSTR_CNTL_CONF_GET_NUM_RANGE_SPARE_NON VSTR__CNTL(CONF, 39) */
#define VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_NON VSTR__CNTL(CONF, 40)
/* #define VSTR_CNTL_CONF_GET_NUM_RANGE_SPARE_PTR VSTR__CNTL(CONF, 41) */
#define VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_PTR VSTR__CNTL(CONF, 42)
/* #define VSTR_CNTL_CONF_GET_NUM_RANGE_SPARE_REF VSTR__CNTL(CONF, 43) */
#define VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF VSTR__CNTL(CONF, 44)
#define VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE VSTR__CNTL(CONF, 45)
#define VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE VSTR__CNTL(CONF, 46)
#define VSTR_CNTL_CONF_GET_NUM_SPARE_BASE VSTR__CNTL(CONF, 47)
#define VSTR_CNTL_CONF_SET_NUM_SPARE_BASE VSTR__CNTL(CONF, 48)
/* #define VSTR_CNTL_CONF_GET_NUM_RANGE_SPARE_BASE VSTR__CNTL(CONF, 49) */
#define VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BASE VSTR__CNTL(CONF, 50)
#define VSTR_CNTL_CONF_GET_LOC_REF_NAME_NUMERIC VSTR__CNTL(CONF, 51)
#define VSTR_CNTL_CONF_SET_LOC_REF_NAME_NUMERIC VSTR__CNTL(CONF, 52)
#define VSTR_CNTL_CONF_GET_LOC_REF_DEC_POINT VSTR__CNTL(CONF, 53)
#define VSTR_CNTL_CONF_SET_LOC_REF_DEC_POINT VSTR__CNTL(CONF, 54)
#define VSTR_CNTL_CONF_GET_LOC_REF_THOU_SEP VSTR__CNTL(CONF, 55)
#define VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP VSTR__CNTL(CONF, 56)
#define VSTR_CNTL_CONF_GET_LOC_REF_THOU_GRP VSTR__CNTL(CONF, 57)
#define VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP VSTR__CNTL(CONF, 58)
#define VSTR_CNTL_CONF_GET_LOC_REF_NULL_PTR VSTR__CNTL(CONF, 59)
#define VSTR_CNTL_CONF_SET_LOC_REF_NULL_PTR VSTR__CNTL(CONF, 60)
