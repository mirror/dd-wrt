#ifndef MSGENCODE_H
#define MSGENCODE_H

#define M_CHOKE 		(unsigned char) 0
#define M_UNCHOKE 		(unsigned char) 1
#define M_INTERESTED 		(unsigned char) 2
#define M_NOT_INTERESTED	(unsigned char) 3
#define M_HAVE 			(unsigned char) 4
#define M_BITFIELD		(unsigned char) 5
#define M_REQUEST 		(unsigned char) 6
#define M_PIECE 		(unsigned char) 7
#define M_CANCEL 		(unsigned char) 8

#define H_BASE_LEN		1	/* int_siz + chr_siz */

#define H_HAVE_LEN		5	/* chr_siz + int_siz */
#define H_PIECE_LEN		9	/* chr_siz + int_siz*2 */
#define H_REQUEST_LEN		13	/* chr_siz + int_siz*3 */
#define H_CANCEL_LEN		13	/* chr_siz + int_siz*3 */

#endif
