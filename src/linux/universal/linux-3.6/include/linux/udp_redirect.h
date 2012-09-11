#ifndef _UDP_REDIRECT_H
#define _UDP_REDIRECT_H

/******************************************************************************

                               Copyright (c) 2006
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  THE DELIVERY OF THIS SOFTWARE AS WELL AS THE HEREBY GRANTED NON-EXCLUSIVE,
  WORLDWIDE LICENSE TO USE, COPY, MODIFY, DISTRIBUTE AND SUBLICENSE THIS
  SOFTWARE IS FREE OF CHARGE.

  THE LICENSED SOFTWARE IS PROVIDED "AS IS" AND INFINEON EXPRESSLY DISCLAIMS
  ALL REPRESENTATIONS AND WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING
  WITHOUT LIMITATION, WARRANTIES OR REPRESENTATIONS OF WORKMANSHIP,
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, DURABILITY, THAT THE
  OPERATING OF THE LICENSED SOFTWARE WILL BE ERROR FREE OR FREE OF ANY THIRD
  PARTY CLAIMS, INCLUDING WITHOUT LIMITATION CLAIMS OF THIRD PARTY INTELLECTUAL
  PROPERTY INFRINGEMENT.

  EXCEPT FOR ANY LIABILITY DUE TO WILFUL ACTS OR GROSS NEGLIGENCE AND EXCEPT
  FOR ANY PERSONAL INJURY INFINEON SHALL IN NO EVENT BE LIABLE FOR ANY CLAIM
  OR DAMAGES OF ANY KIND, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#ifndef _LINUX_TYPES_H
#include <linux/types.h>
#endif


/* ============================= */
/* Definitions                   */
/* ============================= */
#define UDP_REDIRECT_MAGIC (void*)0x55445052L


/* ============================= */
/* Global variable declaration   */
/* ============================= */
extern int (*udp_do_redirect_fn)(struct sock *sk, struct sk_buff *skb);
extern int (*udpredirect_getfrag_fn)(void *p, char * to,
                                     int offset, int fraglen, int odd,
                                     struct sk_buff *skb);
/* ============================= */
/* Global function declaration   */
/* ============================= */

extern int udpredirect_getfrag(void *p, char * to, int offset,
                               int fraglen, int odd, struct sk_buff *skb);
#endif
