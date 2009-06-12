#target: pp gen sim rand burst chirp
target: serial

pp: pp.c
	cc -o pp -DTEST_PP -O2 pp.c

serial: serial.c
	cc -o serial serial.c -lm

burst: pp.c
	cc -o burst -DBURST -O2 pp.c

sim: sim.c rand.c pp.c
	cc -o sim -DGENSIG rand.c sim.c pp.c

gen: rand.c sim.c pp.c
	cc -o gen -DGENSIG sim.c pp.c rand.c

rand: rand.c sim.c pp.c
	cc -o rand -DGENSIG sim.c pp.c rand.c

#chirp: chirp.c
#	cc -o chirp chirp.c -lm

clean: 
	if (test -e *.o) then rm -f *.o; fi
	if (test -e pp) then rm -f pp; fi
	if (test -e sim) then rm -f sim; fi
	if (test -e gen) then rm -f gen; fi
	if (test -e rand) then rm -f rand; fi
	if (test -e burst) then rm -f burst; fi
	if (test -e *~) then rm -f *~; fi

