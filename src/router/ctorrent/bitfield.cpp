#include "bitfield.h"

#ifdef WINDOWS
#include <io.h>
#include <memory.h>
#else
#include <unistd.h>
#include <sys/param.h>
#endif

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const unsigned char BIT_HEX[] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

#define _isset(idx)		(b[(idx) / 8 ] & BIT_HEX[(idx) % 8])
#define _isempty() 		(nset == 0)
#define _isempty_sp(sp) 	((sp).nset == 0)
#define _isfull() 		(nset >= nbits)
#define _isfull_sp(sp) 		((sp).nset >= nbits)

size_t BitField::nbytes = 0;
size_t BitField::nbits = 0;

BitField::BitField()
{
  b = new unsigned char[nbytes];

#ifndef WINDOWS  
  if( !b ) throw 9;
#endif

  memset(b, 0, nbytes);
  nset = 0;
}

BitField::BitField(size_t npcs)	
{
  nbits = npcs;
  nbytes = nbits / 8;
  if( nbits % 8 ) nbytes++;

  b = new unsigned char[nbytes];

#ifndef WINDOWS
  if( !b ) throw 9;
#endif

  memset(b, 0, nbytes);
  nset = 0;
}

BitField::BitField(const BitField &bf)
{
  nset = bf.nset;
  if( _isfull_sp(bf) ) b = (unsigned char *) 0;
  else{
    b = new unsigned char[nbytes];
#ifndef WINDOWS
    if( !b ) throw 9;
#endif
    memcpy(b, bf.b, nbytes);
  }
}

void BitField::SetReferBuffer(char *buf)
{
  if( !b ){ 
    b = new unsigned char[nbytes];
#ifndef WINDOWS
    if( !b ) throw 9;
#endif
  }
  memcpy((char*)b,buf,nbytes);
  _recalc();
}

void BitField::SetAll()
{
  if( b ){
    delete []b; 
    b = (unsigned char*) 0;
  }
  nset = nbits;
}

int BitField::IsSet(size_t idx) const
{
  if( idx >= nbits ) return 0;
  return _isfull() ? 1 : _isset(idx);
}

void BitField::Set(size_t idx)
{
  if(idx >= nbits) return;

  if( !_isfull() && !_isset(idx) ){
    b[idx / 8] |= BIT_HEX[idx % 8];
    nset++;
    if( _isfull() && b){ delete []b; b = (unsigned char*) 0;}
  }
}

void BitField::UnSet(size_t idx)
{
  if( idx >= nbits ) return;

  if( _isfull() ){
    b = new unsigned char[nbytes];
#ifndef WINDOWS
    if( !b ) throw 9;
#endif
    _setall(b);
    b[idx / 8] &= (~BIT_HEX[idx % 8]);
    nset = nbits - 1;
  }else{
    if( _isset(idx) ){
      b[idx / 8] &= (~BIT_HEX[idx % 8]);
      nset--;
    }
  }
}

void BitField::Invert()
{
  if( _isempty() ){
    if(b){
      delete []b;
      b = (unsigned char*) 0;
    }
    nset = nbits;
  }else if( _isfull() ){
    b = new unsigned char[nbytes];
#ifndef WINDOWS
    if( !b ) throw 9;
#endif
    memset(b, 0, nbytes);
    nset = 0;
  }else{
    size_t i = 0;
    size_t s = nset;
    for( ; i < nbytes - 1; i++ ) b[i] = ~b[i];

    if( nbits % nbytes ){
      for( i = 8 * (nbytes - 1); i < nbits; i++ ) if( _isset(i) ) UnSet(i); else Set(i);
    }else
      b[nbytes - 1] = ~b[nbytes - 1];

    nset = nbits - s;
  }
}

void BitField::Comb(const BitField &bf)
{
  size_t i;
  if( _isfull() || _isfull_sp(bf) ){
    SetAll();
  }else{
    for(i = 0; i < nbytes; i++) b[i] |= bf.b[i];
    _recalc();
  }
}

void BitField::Except(const BitField &bf)
{
   size_t i;
  char c;
  if( bf.nset != 0 ){
    if( nset >= nbits ){
      b = new unsigned char[nbytes];
#ifndef WINDOWS
      if( !b ) throw 9;
#endif
      _setall(b);
    }
    for(i = 0; i < nbytes; i++){
      c = b[i];
      b[i] ^= bf.b[i];
      b[i] &= c;
    }
    _recalc();
  }
}

size_t BitField::Random() const
{
   size_t idx;

  if( _isfull() ) idx = rand() % nbits;
  else{
    size_t i;
    i = rand() % nset + 1;
    for(idx = 0; idx < nbits && i; idx++) 
      if( _isset(idx) ) i--;
    idx--;
  }
  return idx;
}

void BitField::_recalc()
{
  // 重新计算 nset 的值
   size_t i;
  for(nset = 0, i = 0; i < nbits; i++) if( _isset(i) ) nset++;
  if( _isfull() && b){ delete []b; b = (unsigned char*) 0;}
}

void BitField::operator=(const BitField &bf)
{
  nset = bf.nset;
  if( _isfull_sp(bf) ){
    if( b ) { delete []b; b = (unsigned char*) 0; }
  }else{
    if( !b ){ 
      b = new unsigned char[nbytes];
#ifndef WINDOWS
      if( !b ) throw 9;
#endif
    }
    memcpy(b, bf.b, nbytes);
  }
}

void BitField::WriteToBuffer(char *buf)
{
  if(_isfull())
    _setall((unsigned char*)buf);
  else
    memcpy(buf,(char*)b,nbytes);
}

void BitField::_setall(unsigned char *buf)
{
  size_t i;

  memset(buf,0xFF,nbytes - 1);

  if( nbits % nbytes ){
    for(i = 8 * (nbytes - 1); i < nbits; i++)
      buf[i / 8] |= BIT_HEX[i % 8];
  }else
    buf[nbytes - 1] = (unsigned char) 0xFF;
}

int BitField::SetReferFile(const char *fname)
{
  FILE *fp;
  struct stat sb;
  char *bitbuf = (char*) 0;

  if(stat(fname, &sb) < 0) return -1;
  if( sb.st_size != nbytes ) return -1;
  
  fp = fopen(fname, "r");
  if( !fp ) return -1;
  
  bitbuf = new char[nbytes];
#ifndef WINDOWS
  if( !bitbuf ) goto fclose_err;
#endif

  if( fread(bitbuf, nbytes, 1, fp) != 1 ) goto fclose_err;

  fclose(fp);
  
  SetReferBuffer(bitbuf);

  delete []bitbuf;
  return 0;
 fclose_err:
  if( bitbuf ) delete []bitbuf;
  fclose(fp);
  return -1;
}

int BitField::WriteToFile(const char *fname)
{
  FILE *fp;
  char *bitbuf = (char*) 0;

  fp = fopen(fname, "w");
  if( !fp ) return -1;
  
  bitbuf = new char[nbytes];
#ifndef WINDOWS
  if( !bitbuf ) goto fclose_err;
#endif

  WriteToBuffer(bitbuf);

  if( fwrite(bitbuf, nbytes, 1, fp) != 1 ) goto fclose_err;

  delete []bitbuf;
  fclose(fp);
  return 0;
 fclose_err:
  if( bitbuf ) delete []bitbuf;
  fclose(fp);
  return -1;
}
