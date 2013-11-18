#!/usr/bin/env python
# encoding: utf-8

# webgps.py
#
# This is a Python port of webgps.c from http://www.wireless.org.au/~jhecker/gpsd/
# by Beat Bolli <me+gps@drbeat.li>
#

import time, calendar, math, socket, sys, os, select, pickle
from gps import *

TRACKMAX = 1024
STALECOUNT = 10

DIAMETER = 200

def polartocart(el, az):
    radius = DIAMETER * (1 - el / 90.0) # * math.cos(Deg2Rad(float(el)))
    theta = Deg2Rad(float(az - 90))
    return (
        -int(radius * math.cos(theta) + 0.5),        # switch sides for a skyview!
        int(radius * math.sin(theta) + 0.5)
    )


class Track:
    '''Store the track of one satellite.'''

    def __init__(self, prn):
        self.prn = prn
        self.stale = 0
        self.posn = []          # list of (x, y) tuples

    def add(self, x, y):
        pos = (x, y)
        self.stale = STALECOUNT
        if not self.posn or self.posn[-1] != pos:
            self.posn.append(pos)
            if len(self.posn) > TRACKMAX:
                self.posn = self.posn[-TRACKMAX:]
            return 1
        return 0

class SatTracks(gps):
    '''gpsd client writing HTML5 and <canvas> output.'''

    def __init__(self):
        gps.__init__(self)
        self.sattrack = {}      # maps PRNs to Tracks
        self.state = None
        self.statetimer = time.time()
        self.needsupdate = 0

    def html(self, fh, jsfile):
        fh.write("""<!DOCTYPE html>

<html>
<head>
\t<meta http-equiv=Refresh content=300>
\t<meta charset='utf-8'>
\t<title>GPSD Satellite Positions and Readings</title>
\t<style type='text/css'>
\t\t.num td { text-align: right; }
\t\tth { text-align: left; }
\t</style>
\t<script src='%s'></script>
</head>
<body>
\t<table border=1>
\t\t<tr>
\t\t\t<td>
\t\t\t\t<table border=0 class=num>
\t\t\t\t\t<tr><th>PRN:</th><th>Elev:</th><th>Azim:</th><th>SNR:</th><th>Used:</th></tr>
""" % jsfile)

        sats = self.satellites[:]
        sats.sort(lambda a, b: a.PRN - b.PRN)
        for s in sats:
            fh.write("\t\t\t\t\t<tr><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%s</td></tr>\n" % (
                s.PRN, s.elevation, s.azimuth, s.ss, s.used and 'Y' or 'N'
            ))

        fh.write("\t\t\t\t</table>\n\t\t\t\t<table border=0>\n")

        def row(l, v):
            fh.write("\t\t\t\t\t<tr><th>%s:</th><td>%s</td></tr>\n" % (l, v))

        def deg_to_str(a, hemi):
            return '%.6f %c' % (abs(a), hemi[a < 0])

        row('Time', self.utc or 'N/A')

        if self.fix.mode >= MODE_2D:
            row('Latitude', deg_to_str(self.fix.latitude, 'SN'))
            row('Longitude', deg_to_str(self.fix.longitude, 'WE'))
            row('Altitude', self.fix.mode == MODE_3D and "%f m" % self.fix.altitude or 'N/A')
            row('Speed', not isnan(self.fix.speed) and "%f m/s" % self.fix.speed or 'N/A')
            row('Course', not isnan(self.fix.track) and "%f°" % self.fix.track or 'N/A')
        else:
            row('Latitude', 'N/A')
            row('Longitude', 'N/A')
            row('Altitude', 'N/A')
            row('Speed', 'N/A')
            row('Course', 'N/A')

        row('EPX', not isnan(self.fix.epx) and "%f m" % self.fix.epx or 'N/A')
        row('EPY', not isnan(self.fix.epy) and "%f m" % self.fix.epy or 'N/A')
        row('EPV', not isnan(self.fix.epv) and "%f m" % self.fix.epv or 'N/A')
        row('Climb', self.fix.mode == MODE_3D and not isnan(self.fix.climb) and
            "%f m/s" % self.fix.climb or 'N/A'
        )

        if not (self.valid & ONLINE_SET):
            newstate = 0
            state = "OFFLINE"
        else:
            newstate = self.fix.mode
            if newstate == MODE_2D:
                state = self.status == STATUS_DGPS_FIX and "2D DIFF FIX" or "2D FIX"
            elif newstate == MODE_3D:
                state = self.status == STATUS_DGPS_FIX and "3D DIFF FIX" or "3D FIX"
            else:
                state = "NO FIX"
        if newstate != self.state:
            self.statetimer = time.time()
            self.state = newstate
        row('State', state + " (%d secs)" % (time.time() - self.statetimer))

        fh.write("""\t\t\t\t</table>
\t\t\t</td>
\t\t\t<td>
\t\t\t\t<canvas id=satview width=425 height=425>
\t\t\t\t\t<p>Your browser needs HTML5 &lt;canvas> support to display the satellite view correctly.</p>
\t\t\t\t</canvas>
\t\t\t\t<script type='text/javascript'>draw_satview();</script>
\t\t\t</td>
\t\t</tr>
\t</table>
</body>
</html>
""")

    def js(self, fh):
        fh.write("""// draw the satellite view

function draw_satview() {
    var c = document.getElementById('satview');
    if (!c.getContext) return;
    var ctx = c.getContext('2d');
    if (!ctx) return;

    var circle = Math.PI * 2,
        M = function (x, y) { ctx.moveTo(x, y); },
        L = function (x, y) { ctx.lineTo(x, y); };

    ctx.save();
    ctx.clearRect(0, 0, c.width, c.height);
    ctx.translate(210, 210);

    // grid and labels
    ctx.strokeStyle = 'black';
    ctx.beginPath();
    ctx.arc(0, 0, 200, 0, circle, 0);
    ctx.stroke();

    ctx.beginPath();
    ctx.strokeText('N', -4, -202);
    ctx.strokeText('E', -210, 4);
    ctx.strokeText('W', 202, 4);
    ctx.strokeText('S', -4, 210);

    ctx.strokeStyle = 'grey';
    ctx.beginPath();
    ctx.arc(0, 0, 100, 0, circle, 0);
    M(2, 0);
    ctx.arc(0, 0,   2, 0, circle, 0);
    ctx.stroke();

    ctx.strokeStyle = 'lightgrey';
    ctx.save();
    ctx.beginPath();
    M(0, -200); L(0, 200); ctx.rotate(circle / 8);
    M(0, -200); L(0, 200); ctx.rotate(circle / 8);
    M(0, -200); L(0, 200); ctx.rotate(circle / 8);
    M(0, -200); L(0, 200);
    ctx.stroke();
    ctx.restore();

    // tracks
    ctx.lineWidth = 0.6;
    ctx.strokeStyle = 'red';
""");

        def M(p):
            return 'M(%d,%d); ' % p
        def L(p):
            return 'L(%d,%d); ' % p

        # Draw the tracks
        for t in self.sattrack.values():
            if t.posn:
                fh.write("    ctx.globalAlpha = %s; ctx.beginPath(); %s%sctx.stroke();\n" % (
                    t.stale == 0 and '0.66' or '1', M(t.posn[0]),
                    ''.join([L(p) for p in t.posn[1:]])
                ))

        fh.write("""
    // satellites
    ctx.lineWidth = 1;
    ctx.strokeStyle = 'black';
""")

        # Draw the satellites
        for s in self.satellites:
            x, y = polartocart(s.elevation, s.azimuth)
            fill = not s.used and 'lightgrey' or \
                s.ss < 30 and 'red' or \
                s.ss < 35 and 'yellow' or \
                s.ss < 40 and 'green' or 'lime'

            # Center PRNs in the marker
            offset = s.PRN < 10 and 3 or s.PRN >= 100 and -3 or 0

            fh.write("    ctx.beginPath(); ctx.fillStyle = '%s'; " % fill)
            if s.PRN > 32:      # Draw a square for SBAS satellites
                fh.write("ctx.rect(%d, %d, 16, 16); " % (x - 8, y - 8))
            else:
                fh.write("ctx.arc(%d, %d, 8, 0, circle, 0); " % (x, y))
            fh.write("ctx.fill(); ctx.stroke(); ctx.strokeText('%s', %d, %d);\n" % (s.PRN, x - 6 + offset, y + 4))

        fh.write("""
    ctx.restore();
}
""")

    def make_stale(self):
        for t in self.sattrack.values():
            if t.stale:
                t.stale -= 1

    def delete_stale(self):
        for prn in self.sattrack.keys():
            if self.sattrack[prn].stale == 0:
                del self.sattrack[prn]
                self.needsupdate = 1

    def insert_sat(self, prn, x, y):
        try:
            t = self.sattrack[prn]
        except KeyError:
            self.sattrack[prn] = t = Track(prn)
        self.needsupdate += t.add(x, y)

    def update_tracks(self):
        self.make_stale()
        for s in self.satellites:
            x, y = polartocart(s.elevation, s.azimuth)
            if self.insert_sat(s.PRN, x, y):
                self.needsupdate = 1
        self.delete_stale()

    def generate_html(self, htmlfile, jsfile):
        fh = open(htmlfile, 'w')
        self.html(fh, jsfile)
        fh.close()

    def generate_js(self, jsfile):
        fh = open(jsfile, 'w')
        self.js(fh)
        fh.close()

    def run(self, suffix, period):
        jsfile = 'gpsd' + suffix + '.js'
        htmlfile = 'gpsd' + suffix + '.html'
        end = time.time() + period
        self.needsupdate = 1
        self.stream(WATCH_ENABLE | WATCH_NEWSTYLE)
        for report in self:
            if report['class'] not in ('TPV', 'SKY'):
                continue
            self.update_tracks()
            if self.needsupdate:
                self.generate_js(jsfile)
                self.needsupdate = 0
            self.generate_html(htmlfile, jsfile)
            if period <= 0 and self.fix.mode >= MODE_2D \
            or period > 0 and time.time() > end:
                break

def main():
    argv = sys.argv[1:]

    factors = {
        's': 1, 'm': 60, 'h': 60 * 60, 'd': 24 * 60 * 60
    }
    arg = argv and argv[0] or ''
    if arg[-1:] in factors.keys():
        period = int(arg[:-1]) * factors[arg[-1]]
    elif arg == 'c':
	period = None
    elif arg:
        period = int(arg)
    else:
        period = 0
    if arg:
	arg = '-' + arg

    sat = SatTracks()

    # restore the tracks
    pfile = 'tracks.p'
    if os.path.isfile(pfile):
        p = open(pfile)
        sat.sattrack = pickle.load(p)
        p.close()

    try:
        sat.run(arg, period)
    except KeyboardInterrupt:
        # save the tracks
        p = open(pfile, 'w')
        pickle.dump(sat.sattrack, p)
        p.close()

if __name__ == '__main__':
    main()
