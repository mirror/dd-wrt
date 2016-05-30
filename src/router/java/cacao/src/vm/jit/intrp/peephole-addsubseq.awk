NF>2 {
  ninsts = NF; #actually one too many
  for (i=2; i<6; i++) {
    for (j=0; i+j<ninsts; j++) {
      printf("%14ld\t", $1);
      for (k=j; k<i+j; k++) {
	printf("%s ", $(k+2));
      }
      printf("\n");
    }
  }
}
      
