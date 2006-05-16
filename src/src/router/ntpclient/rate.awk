(FNR==1){ day_off=$1;        t1=$2; o1=$5; f1=$7}
        { t2=($1-day_off)*86400+$2; o2=$5; f2=$7; if (f2!=f1) fwarn=1}

END{
  print "delta-t",t2-t1,"seconds"
  print "delta-o",o2-o1,"useconds"
  if (fwarn) print " *** frequency changed in the middle - don't use ***"
  slope=(o2-o1)/(t2-t1)
  print "slope",slope,"ppm"
  print "old frequency",f1,"(",f1/65536,"ppm)"
  f3=f1+int(slope*65536);
  print "new frequency",f3,"(",f3/65536,"ppm)"
}

# the final value is what you should push into the adjtimex(2) field.
# i.e., if the last line shows
# new frequency -1318109 ( -20.1127 ppm)
# you put -1318109 into the -f switch of adjtimex (e.g., adjtimex -f -1318109)
