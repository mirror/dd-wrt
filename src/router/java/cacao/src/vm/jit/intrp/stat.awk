BEGIN {
  FS="\t";
}
/ *[0-9]+\t/ {
  dyn[$2] += $1;
  stat[$2]++;
  files[$2] += (FILENAME!=filename[$2]);
  filename[$2] = FILENAME;
}
END {
  for (i in dyn)
    printf("%7d\t%7d\t%15d\t %s\n",files[i],stat[i],dyn[i],i);
}
