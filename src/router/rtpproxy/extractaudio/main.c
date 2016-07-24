/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2007 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
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
 * $Id$
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#if defined(__FreeBSD__)
#include <sys/rtprio.h>
#else
#include <sys/resource.h>
#endif
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sndfile.h>

#include "format_au.h"
#include "g711.h"
#include "rtp_info.h"
#include "decoder.h"
#include "session.h"
#include "rtpp_record_private.h"
#include "rtpp_loader.h"
#include "rtp.h"
#include "rtp_analyze.h"

static void
usage(void)
{

    fprintf(stderr, "usage: extractaudio [-idsn] rdir outfile [link1] ... [linkN]\n");
    exit(1);
}

/* Lookup session given ssrc */
struct session *
session_lookup(struct channels *channels, uint32_t ssrc)
{
    struct channel *cp;

    MYQ_FOREACH(cp, channels) {
        if (MYQ_FIRST(&(cp->session))->rpkt->ssrc == ssrc)
            return &(cp->session);
    }
    return NULL;
}

/* Insert channel keeping them ordered by time of first packet arrival */
void
channel_insert(struct channels *channels, struct channel *channel)
{
    struct channel *cp;

    MYQ_FOREACH_REVERSE(cp, channels)
        if (MYQ_FIRST(&(cp->session))->pkt->time <
          MYQ_FIRST(&(channel->session))->pkt->time) {
            MYQ_INSERT_AFTER(channels, cp, channel);
            return;
        }
    MYQ_INSERT_HEAD(channels, channel);
}

static int
load_session(const char *path, struct channels *channels, enum origin origin)
{
    int pcount;
    struct rtpp_session_stat stat;
    struct rtpp_loader *loader;

    loader = rtpp_load(path);
    if (loader == NULL)
        return -1;

    pcount = loader->load(loader, channels, &stat, origin);

    update_rtpp_totals(&stat);
    printf("pcount=%u, min_seq=%u, max_seq=%u, seq_offset=%u, ssrc=%u, duplicates=%u\n",
      (unsigned int)stat.last.pcount, (unsigned int)stat.last.min_seq, (unsigned int)stat.last.max_seq,
      (unsigned int)stat.last.seq_offset, (unsigned int)stat.last.ssrc, (unsigned int)stat.last.duplicates);
    printf("ssrc_changes=%u, psent=%u, precvd=%u\n", stat.ssrc_changes, stat.psent, stat.precvd);

    loader->destroy(loader);

    return pcount;
}

int
main(int argc, char **argv)
{
    int ch;
    int oblen, delete, stereo, idprio, nch, neof;
    int32_t osample, asample, csample;
    uint64_t nasamples, nosamples;
    struct channels channels;
    struct channel *cp;
#if defined(__FreeBSD__)
    struct rtprio rt;
#endif
    int16_t obuf[1024];
    char aname[MAXPATHLEN], oname[MAXPATHLEN];
    double basetime;
    SF_INFO sfinfo;
    SNDFILE *sffile;
    int dflags;

    MYQ_INIT(&channels);
    memset(&sfinfo, 0, sizeof(sfinfo));

    delete = stereo = idprio = 0;
    dflags = D_FLAG_NONE;
    while ((ch = getopt(argc, argv, "dsin")) != -1)
        switch (ch) {
        case 'd':
            delete = 1;
            break;

        case 's':
            stereo = 1;
            break;

        case 'i':
            idprio = 1;
            break;

        case 'n':
            dflags |= D_FLAG_NOSILENCE;
            break;

        case '?':
        default:
            usage();
        }
    argc -= optind;
    argv += optind;

    if (argc < 2)
        usage();

    if (idprio != 0) {
#if defined(__FreeBSD__)
        rt.type = RTP_PRIO_IDLE;
        rt.prio = RTP_PRIO_MAX;
        rtprio(RTP_SET, 0, &rt);
#else
        setpriority(PRIO_PROCESS, 0, 20);
#endif
    }

    sprintf(aname, "%s.a.rtp", argv[0]);
    sprintf(oname, "%s.o.rtp", argv[0]);

    load_session(aname, &channels, A_CH);
    load_session(oname, &channels, O_CH);

    if (MYQ_EMPTY(&channels))
        goto theend;

    nch = 0;
    basetime = MYQ_FIRST(&(MYQ_FIRST(&channels)->session))->pkt->time;
    MYQ_FOREACH(cp, &channels) {
        if (basetime > MYQ_FIRST(&(cp->session))->pkt->time)
            basetime = MYQ_FIRST(&(cp->session))->pkt->time;
    }
    MYQ_FOREACH(cp, &channels) {
        cp->skip = (MYQ_FIRST(&(cp->session))->pkt->time - basetime) * 8000;
        cp->decoder = decoder_new(&(cp->session), dflags);
        nch++;
    }

    oblen = 0;

    sfinfo.samplerate = 8000;
    if (stereo == 0) {
        sfinfo.channels = 1;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_GSM610;
    } else {
        /* GSM+WAV doesn't work with more than 1 channels */
        sfinfo.channels = 2;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_MS_ADPCM;
    }

    sffile = sf_open(argv[1], SFM_WRITE, &sfinfo);
    if (sffile == NULL)
        errx(2, "%s: can't open output file", argv[1]);

    nasamples = nosamples = 0;
    do {
        neof = 0;
        asample = osample = 0;
        MYQ_FOREACH(cp, &channels) {
            if (cp->skip > 0) {
                cp->skip--;
		continue;
            }
            do {
                csample = decoder_get(cp->decoder);
            } while (csample == DECODER_SKIP);
            if (csample == DECODER_EOF || csample == DECODER_ERROR) {
                neof++;
                continue;
            }
            if (cp->origin == A_CH) {
                asample += csample;
                nasamples++;
            } else {
                osample += csample;
                nosamples++;
            }
        }
        if (neof < nch) {
            if (stereo == 0) {
                obuf[oblen] = (asample + osample) / 2;
                oblen += 1;
            } else {
                obuf[oblen] = asample;
                oblen += 1;
                obuf[oblen] = osample;
                oblen += 1;
            }
        }
        if (neof == nch || oblen == sizeof(obuf) / sizeof(obuf[0])) {
            sf_write_short(sffile, obuf, oblen);
            oblen = 0;
        }
    } while (neof < nch);
    fprintf(stderr, "samples decoded: O: %lu, A: %lu\n", nosamples, nasamples);

    sf_close(sffile);

    while (argc > 2) {
        link(argv[1], argv[argc - 1]);
        argc--;
    }

theend:
    if (delete != 0) {
        unlink(aname);
        unlink(oname);
    }

    return 0;
}
