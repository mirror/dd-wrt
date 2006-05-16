/*
 *  $Id: ether_mod_load.c,v 1.1 2004/04/27 01:34:21 dyang Exp $
 *
 *  libnet
 *  OpenBSD ether_mod_load.c - lkm replacement for ether_output
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *                           route|daemon9 <route@infonexus.com>
 *  Original code and idea 1997 Thomas Ptacek <tqbf@pobox.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */


#include <sys/param.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/mount.h>
#include <sys/exec.h>
#include <sys/lkm.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/queue.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/kernel.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/radix.h>
                                                                
extern int ether_output_spoof __P((struct ifnet *, struct mbuf *, struct sockaddr *, struct rtentry *));
int ether_spoof(struct lkm_table *lkp, int cmd, int ver);

MOD_MISC("etherspoof");

static int 
etherspoof_load(struct lkm_table *lkp, int cmd)
{
    struct ifnet *ifp;
    int err = 0;

    switch (cmd)
    {
        case LKM_E_LOAD:
            for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_list.tqe_next)
            {
                if (ifp->if_output == ether_output)
                {
                    ifp->if_output = ether_output_spoof;
                    printf("Interface %s fixed\n", ifp->if_xname);
                }
            }			
	
            printf("Ethernet MAC Address Fix Loaded\n");
            break;

        case LKM_E_UNLOAD:
            for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_list.tqe_next)
            {
                if (ifp->if_output == ether_output_spoof)
                {
                    ifp->if_output = ether_output;
                    printf("Interface %s unfixed\n", ifp->if_xname);
                }
            }
            printf("Ethernet MAC Address Fix Unloaded\n");
            break;
        default:
            err = EINVAL;
            break;
    }
    return(err);
}


int ether_spoof(struct lkm_table *lkp, int cmd, int ver)
{
    DISPATCH(lkp, cmd, ver, etherspoof_load, etherspoof_load, lkm_nofunc);
}
