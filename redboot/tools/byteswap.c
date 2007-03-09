/* byteswap.c */

/* Byte swapping utility for converting binary data files between */
/*   big-endian and little-endian formats. */
/* Revised 2 August 2002, Curtis W. Chen */

/* Copyright 2002 Board of Trustees, Leland Stanford Jr. University */
/* No warranty. */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <limits.h>

#define MAXSTRLEN   4096
#define MAXNELEM    (LONG_MAX-1)
#define MAXNBYTES   32768
#define MAXINFILES  8192
#define BLOCKSIZE   8192
#define TRUE        1
#define FALSE       0


/* prototype for function called when program receives a signal */
void SigHandler(int signum);


/* main program */
int main(int argc, char *argv[]){

  FILE *fp, *outfp;
  unsigned char *chunk1, *chunk2;
  long i, j, chunksize, chunksize2, nbytes, headerlength;
  long elemread, extrabytes, keepgoing, filenum, nfiles;
  long ielem, nelem;
  char *infilearr[MAXINFILES];
  char *filename, *outfilename;
  char *endptr;

  /* parse input */
  nfiles=0;
  headerlength=0;
  outfp=NULL;
  outfilename=NULL;
  nelem=MAXNELEM;
  nbytes=0;
  if(argc<3){
    fprintf(stderr,
	    "usage: byteswap [options] bytes_per_element filename(s)\n");
    fprintf(stderr,"options:\n");
    fprintf(stderr,"  -h <bytes_in_header>  Don't swap first H bytes\n");
    fprintf(stderr,"  -n <num_elements>     Stop after swapping N elements\n");
    fprintf(stderr,"  -o <outfile>          Write to outfile instead of"
	                                    " swapping in place\n");
    exit(1);
  }
  for(i=1;i<argc;i++){
    if(!strncmp(argv[i],"-",1)){
      if(!strcmp(argv[i],"-h")){
	if(argc>=++i+1){
	  headerlength=strtol(argv[i],&endptr,10);
	  if(headerlength<0 || strlen(endptr)){
	    fprintf(stderr,"argument to -h must be nonnegative integer\n");
	    exit(1);
	  }
	}else{
	  fprintf(stderr,"option %s specified with no argument\n",argv[i-1]);
	  exit(1);
	}
      }else if(!strcmp(argv[i],"-n")){
	if(argc>=++i+1){
	  nelem=strtol(argv[i],&endptr,10);
	  if(nelem<0 || strlen(endptr)){
	    fprintf(stderr,"argument to -n must be nonnegative integer\n");
	    exit(1);
	  }
	  if(nelem>MAXNELEM){
	    fprintf(stderr,"argument to -n too large\n");
	    exit(1);
	  }
	}else{
	  fprintf(stderr,"option %s specified with no argument\n",argv[i-1]);
	  exit(1);
	}
      }else if(!strcmp(argv[i],"-o")){
	if(argc>=++i+1){
	  outfilename=argv[i];
	}else{
	  fprintf(stderr,"option %s specified with no argument\n",argv[i-1]);
	  exit(1);
	}
      }else{
	fprintf(stderr,"unrecogized option %s\n",argv[i]);
	exit(1);
      }
    }else{
      if(nbytes<=0){
	nbytes=strtol(argv[i],&endptr,10);
	if(nbytes<=0 || nbytes>MAXNBYTES || strlen(endptr)){
	  fprintf(stderr,"illegal number of bytes per element\n");
	  fprintf(stderr,
		  "usage: byteswap [options] bytes_per_element filename(s)\n");
	  exit(1);
	}
      }else{
	if(nfiles>=MAXINFILES){
	  fprintf(stderr,"too many input files\n");
	  exit(1);
	}
	infilearr[nfiles++]=argv[i];
      }
    }
  }

  /* make sure we have required inputs */
  if(!nbytes || !nfiles){
    fprintf(stderr,"not enough input arguments\n");
    exit(1);
  }

  /* see if output file passed in */
  if(outfilename!=NULL){

    /* output file cannot be given with multiple input files  */
    if(nfiles>1){
      fprintf(stderr,
	      "output file cannot be specified with multiple input files\n");
      exit(1);
    }

    /* refuse to overwrite input file with output file */
    /* (may still be overwritten if specified with different relative path) */
    if(!strcmp(outfilename,infilearr[0])){
      fprintf(stderr,"output file cannot be same as input file\n");
      exit(1);
    }

    /* open output file */
    if((outfp=fopen(outfilename,"w"))==NULL){
      fprintf(stderr,"couldn't open file %s for writing\n",outfilename);
      exit(1);
    }
  }

  /* get memory for chunk buffers */
  if((chunk1=malloc(BLOCKSIZE*nbytes*sizeof(unsigned char)))==NULL ||
     (chunk2=malloc(BLOCKSIZE*nbytes*sizeof(unsigned char)))==NULL){
    fprintf(stderr,"Out of memory.\n");
    exit(1);
  }
    
  /* trap the interrupt signal if swapping in place */
  if(outfp==NULL){
    signal(SIGINT,SigHandler);
  }  
  
  /* loop over all input files */
  for(filenum=0;filenum<nfiles;filenum++){

    /* get file name */
    filename=infilearr[filenum];
    fprintf(stdout,"swapping file %s\n",filename);

    /* open input file */
    if(outfp!=NULL){
      if((fp=fopen(filename,"r"))==NULL){
	fprintf(stderr,"ERROR: unable to open file %s\n",filename);
	/* exit(1) */
	continue;
      }
    }else{
      if((fp=fopen(filename,"r+"))==NULL){
	fprintf(stderr,"ERROR: unable to open file %s\n",filename);
	/* exit(1) */
	continue;
      }
    }

    /* skip the header */
    if(headerlength){

      /* see if we were passed an outfile */
      if(outfp!=NULL){

	/* if we were passed an outfile, copy bytes to outfile */
	ielem=0;
	while(TRUE){
	  chunksize=fread(chunk1,sizeof(unsigned char),BLOCKSIZE*nbytes,fp);
	  if(!chunksize){
	    fprintf(stderr,"EOF before end of header in file %s\nAbort\n",
		    filename);
	    exit(1);
	  }
	  if(ielem+chunksize<headerlength){
	    if(fwrite(chunk1,sizeof(unsigned char),chunksize,outfp)
	       !=chunksize){
	      fprintf(stderr,"Error writing header\nAbort\n");
	      exit(1);
	    }
	    ielem+=chunksize;
	  }else{
	    if(fwrite(chunk1,sizeof(unsigned char),headerlength-ielem,outfp)
	       !=headerlength-ielem){
	      fprintf(stderr,"Error writing header\nAbort\n");
	      exit(1);
	    }
	    break;
	  }
	}
	
      }

      /* seek to correct spot after header in file */
      if(fseek(fp,headerlength,SEEK_SET)){
	fprintf(stderr,"Error occured seeking to end of %ld byte header\n",
		headerlength);
	exit(1);
      }
      fprintf(stdout,"  skipped %ld byte header\n",headerlength);

    } /* end if(headerlength) */

    /* read in elements, swap, and write out */
    ielem=0;
    extrabytes=0;
    keepgoing=TRUE;
    while((chunksize=fread(chunk1,sizeof(unsigned char),BLOCKSIZE*nbytes,fp))
	  && keepgoing){
      
      /* see how many elements we actually put in the input buffer */
      elemread=chunksize/nbytes;  /* cast as long=implicit floor operation */

      /* see if there were extra bytes at the end of the file */
      extrabytes=chunksize % nbytes;
      if(extrabytes){
	keepgoing=FALSE;
      }
      
      /* see if we read more than the specified number of elements */
      if((ielem+=elemread)>nelem){
	elemread=nelem-(ielem-elemread);
	ielem=nelem;
	keepgoing=FALSE;
      }
      
      /* swap the bytes */
      chunksize2=elemread*nbytes;
      for(i=0;i<chunksize2;i+=nbytes){
	for(j=0;j<nbytes;j++){
	  chunk2[i+(nbytes-j-1)]=chunk1[i+j];
	}
      }
      
      /* see if we were passed an output file */
      if(outfp!=NULL){

	/* if output file was given, write to it */
	if(fwrite(chunk2,sizeof(unsigned char),chunksize2,outfp)!=chunksize2){
	  fprintf(stderr,
		  "Error occurred writing element %ld (disk full?)\nAbort\n",
		  ielem+1);
	  exit(1);
	}
	
      }else{
	
	/* no ouput file was given, so swap in place */
	/* seek back in the file */
	if(fseek(fp,-chunksize,SEEK_CUR)){
	  fprintf(stderr,"Error occurred while seeking in file\nAbort\n");
	  exit(1);
	}
	
	/* write the swapped bytes and flush the stream */
	if(fwrite(chunk2,sizeof(unsigned char),chunksize2,fp)!=chunksize2){
	  fprintf(stderr,"Error occurred writing element %ld\nAbort\n",
		  ielem+1);
	  exit(1);
	}
	fflush(fp);
	fseek(fp,0,SEEK_CUR);  /* required for read/write with some i/o libs */
	
      }
    } /* end main loop */
    fprintf(stdout,"  swapped %ld elements (%ld bytes)\n",
	    ielem,ielem*nbytes);
    
    /* if passed outfile and nelem, copy remainder of file (unswapped) */
    if(outfp!=NULL && ielem==nelem){ 
      if(fseek(fp,headerlength+nbytes*nelem,SEEK_SET)){ 
	fprintf(stderr,"Error occurred while seeking in file\nAbort\n");
	exit(1);
      }
      while((chunksize
	     =fread(chunk1,sizeof(unsigned char),BLOCKSIZE*nbytes,fp))){
	if(fwrite(chunk1,sizeof(unsigned char),chunksize,outfp)!=chunksize){
	  fprintf(stderr,"Error writing unswapped bytes\nAbort\n");
	  exit(1);
	}
      }
    }
    
    /* were there extra bytes at end of file? */
    /* (ie, data length was not integer multiple nbytes) */
    if(extrabytes && ielem<nelem){
      fprintf(stderr,
	      "WARNING: %ld extra byte(s) at end of file %s not swapped\n",
	      extrabytes,filename);
      if(outfp!=NULL){
	if(fseek(fp,-extrabytes,SEEK_END)){
	  fprintf(stderr,"Error occurred while seeking in file\nAbort\n");
	  exit(1);
	}
	while((chunksize=
	       fread(chunk1,sizeof(unsigned char),BLOCKSIZE*nbytes,fp))){
	  if(fwrite(chunk1,sizeof(unsigned char),chunksize,outfp)!=chunksize){
	    fprintf(stderr,"Error writing extra bytes\nAbort\n");
	    exit(1);
	  }
	}
      }
    }
    fclose(fp);
  } /* end loop over input files */

  /* finish up */
  if(outfp!=NULL){
    fprintf(stdout,"output written to file %s\n",outfilename);
    fclose(outfp);
  }
  free(chunk1);
  free(chunk2);
  exit(0);

} /* end of main() */


/* function called when program receives a signal */
void SigHandler(int signum){

  char answer[MAXSTRLEN];

  fprintf(stderr,"Interrupt signal caught\n");
  fprintf(stderr,"Aborting may corrupt your data file.  ");
  while(TRUE){
    fprintf(stderr,"Really abort? [y/n] ");
    fgets(answer,MAXSTRLEN,stdin);
    if(answer[0]=='y' || answer[0]=='Y'){
      fprintf(stderr,"Exiting\n");
      exit(1);
    }else if(answer[0]=='n' || answer[0]=='N'){
      fprintf(stderr,"Continuing\n");
      signal(signum,SigHandler);
      return;
    }else{
      fprintf(stderr,"Please answer y or n.  \n");
    }
  }
}
