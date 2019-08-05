# FQ_CODEL_FAST

This is an attempt to make fq_codel faster and more relevant

It is intended as a drop-in replacement for the existing
fq_codel code in the stack. As development procedes either
feeding back these changes into the linux mainline can happen
or we can fork it to be more suitable for the openwrt audience

There were also mistakes made in early development,
tracking stats that aren't useful, and new features
added that also weren't used, and since the major
use of fq_codel is in sqm systems, adding in an
fast integral shaper is a goal.

Probably override the ce_threshold for this

I also might fiddle with alternate codel ideas
but first up is to make it fast. Adding in the scheduler
component requires a mild rewrite of codel because it
throws away the timestamp and works in a different
time base.

* also test increasing the bulk dropper signal strength

## major mods

* I always hated the search on the maxbacklog stat
I'd rather it just kept track of the flow with the biggest backlog,
and now that we do bulk drops, we can automagically shift to the
next biggest flow most of the time in 32 (64?) rounds.

* Kathie chickened out on when to turn off codel. outside
  of ns2, there's always another queue....

* I'm increasingly uncomfortable with the "find the rtt"
part of the algo in the case of ecn

