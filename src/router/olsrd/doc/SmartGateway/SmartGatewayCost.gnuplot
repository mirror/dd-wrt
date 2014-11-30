sizex=1920
sizey=1080

set terminal svg dynamic enhanced fname 'Helvetica' fsize 16 mousing name "SmartGatewayCost" butt solid size sizex,sizey
set output 'SmartGatewayCost.svg'

set grid xtics lt 0 lw 1 lc rgb "#bbbbbb"
set grid ytics lt 0 lw 1 lc rgb "#bbbbbb"
set grid ztics lt 0 lw 1 lc rgb "#bbbbbb"

set dummy x,y
set isosamples 100, 100


set title "Smart Gateway Cost (Symmetric Link)" 

#                     WexitU   WexitD   Wetx
# path_cost_weight =  ------ + ------ + ---- * path_cost
#                     exitUm   exitDm   Detx

WexitU=1.0
WexitD=1.0
Wetx=1.0
Detx=4096.0

set xlabel "Uplink / Downlink (Mbps)" 
xlow=0.0
xhigh=1.0
set xlabel  offset character 0, 0, 0 font "Helvetica" textcolor lt -1 norotate
set xrange [ xlow : xhigh ] noreverse nowriteback

set ylabel "Path Cost ( = ETX * 1024 )"
ylow=1000.0
yhigh=10000.0
set ylabel  offset character 0, 0, 0 font "Helvetica" textcolor lt -1 rotate by -270
set yrange [ ylow : yhigh ] noreverse nowriteback


set zlabel "Costs" 
set zlabel  offset character -2, 0, 0 font "" textcolor lt -1 norotate

splot (WexitU/x)+(WexitD/x)+((Wetx/Detx)*y)
