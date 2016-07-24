TESTS = startstop basic_versions command_parser makeann extractaudio \
  session_timeouts playback1 forwarding1
command_parser_EXTRA_DIST = command_parser command_parser.input command_parser.output
command_parser_CLEANFILES = command_parser.rout 238uwedguw.rtcp 238uwedguw.rtp
extractaudio_EXTRA_DIST = extractaudio call1_alaw.a.rtp call1_alaw.o.rtp call1_g722.a.rtp \
  call1_g722.o.rtp call1_g729.a.rtp call1_g729.o.rtp call1_ulaw.a.rtp \
  call1_ulaw.o.rtp call1_gsm.a.rtp call1_gsm.o.rtp extractaudio.checksums
extractaudio_CLEANFILES = call1_alaw.mono.wav call1_alaw.stereo.wav \
  call1_g722.mono.wav call1_g722.stereo.wav call1_g729.mono.wav \
  call1_g729.stereo.wav call1_ulaw.mono.wav call1_ulaw.stereo.wav \
  call1_gsm.mono.wav call1_gsm.stereo.wav
playback1_EXTRA_DIST = playback1 playback1.output playback1.checksums \
  playback1.type1.input playback1.type2.input
playback1_CLEANFILES = playback1.0.a.rtp playback1.18.a.rtp playback1.3.a.rtp \
  playback1.8.a.rtp playback1.9.a.rtp playback1.0 playback1.18 playback1.3 \
  playback1.8 playback1.9 playback1.rout playback1.0.wav playback1.18.wav \
  playback1.3.wav playback1.8.wav playback1.9.wav playback1.tout
forwarding1_EXTRA_DIST = forwarding1 forwarding1.checksums forwarding1.gen.input \
  forwarding1.gena.output forwarding1.geno.output forwarding1.input forwarding1.output \
  forwarding1.stats.input forwarding1.delete.input forwarding1.rec.input
forwarding1_CLEANFILES = forwarding1.0.wav forwarding1.18.wav forwarding1.3.wav \
  forwarding1.8.wav forwarding1.9.wav forwarding1.gena.rout forwarding1.geno.rout \
  forwarding1.rout forwarding1_[ao]_*.rtcp forwarding1_[ao]_*.rtp \
  forwarding1.0 forwarding1.3 forwarding1.8 forwarding1.9 forwarding1.18
session_timeouts_EXTRA_DIST = session_timeouts session_timeouts.full.input \
  session_timeouts.full6.input session_timeouts.half.input \
  session_timeouts.half6.input session_timeouts.lookup.input \
  session_timeouts.lookup6.input session_timeouts.output1 session_timeouts.output2 \
  session_timeouts.stats.input session_timeouts6.output1 session_timeouts6.output2
session_timeouts_CLEANFILES = session_timeouts.rout[1234]
EXTRA_DIST = Makefile.am startstop basic_versions basic_versions.input \
    basic_versions.output ${command_parser_EXTRA_DIST} \
    ringback.sln makeann makeann.output \
    ${extractaudio_EXTRA_DIST} ${playback1_EXTRA_DIST} \
    ${forwarding1_EXTRA_DIST} ${session_timeouts_EXTRA_DIST}
# NB: AM_TESTS_ENVIRONMENT not available until automake 1.12
TESTS_ENVIRONMENT = \
        BASEDIR=${abs_srcdir} ; export BASEDIR ; \
        BUILDDIR=${abs_builddir} ; export BUILDDIR ; \
        TOP_BUILDDIR=${abs_top_builddir} ; export TOP_BUILDDIR ;
CLEANFILES = ringback.0 ringback.3 ringback.8 ringback.18 ringback.9 \
  ${extractaudio_CLEANFILES} ${playback1_CLEANFILES} ${forwarding1_CLEANFILES} \
  ${session_timeouts_CLEANFILES} ${command_parser_CLEANFILES}
