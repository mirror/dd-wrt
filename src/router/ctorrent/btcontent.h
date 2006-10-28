#ifndef BTCONTENT_H
#define BTCONTENT_H

#include <sys/types.h>

#include "def.h"

#include <stdio.h>
#include "bitfield.h"
#include "btfiles.h"

typedef struct _btcache{
  u_int64_t bc_off;
  size_t bc_len;
  
  unsigned char bc_f_flush:1;
  unsigned char bc_f_reserved:7;
  
  time_t bc_last_timestamp;
  
  char *bc_buf;

  struct _btcache *bc_next;
}BTCACHE;

class btContent
{
  //METAINFO³ÉÔ±
  char *m_announce;
  unsigned char *m_hash_table;
  unsigned char m_shake_buffer[68];

  size_t m_hashtable_length;
  size_t m_piece_length;
  size_t m_npieces;
  time_t m_create_date, m_seed_timestamp, m_start_timestamp;

  u_int64_t m_left_bytes;
  btFiles m_btfiles;

  BTCACHE *m_cache;
  size_t m_cache_size, m_cache_used;
  
  void _Set_InfoHash(unsigned char buf[20]);
  char* _file2mem(const char *fname, size_t *psiz);
  
  void ReleaseHashTable(){
    if(m_hash_table){
      delete []m_hash_table;
      m_hash_table = (unsigned char*) 0;
    }
  }

  int CheckExist();
  void CacheConfigure();
  void CacheClean();
  u_int64_t max_u_int64_t(u_int64_t a,u_int64_t b) { return (a > b) ? a : b; }
  u_int64_t min_u_int64_t(u_int64_t a,u_int64_t b) { return (a > b) ? b : a; }
  ssize_t CacheIO(char *buf, u_int64_t off, size_t len, int method);
  
 public:
  BitField *pBF;
  char *global_piece_buffer;
  
  btContent();
  ~btContent();
  
  void FlushCache();
  
  int CreateMetainfoFile(const char *mifn);
  int InitialFromFS(const char *pathname, char *ann_url, size_t piece_length);
  int InitialFromMI(const char *metainfo_fname,const char *saveas);

  char* GetAnnounce() { return m_announce;}

  unsigned char* GetShakeBuffer() {return m_shake_buffer;}
  unsigned char* GetInfoHash() {return (m_shake_buffer + 28);}
  unsigned char* GetPeerId() {return (m_shake_buffer + 48); }

  size_t GetPieceLength(size_t idx);
  size_t GetPieceLength() const { return m_piece_length; }
  size_t GetNPieces() const { return m_npieces; }

  u_int64_t GetTotalFilesLength() const { return m_btfiles.GetTotalLength(); }
  u_int64_t GetLeftBytes() const { return m_left_bytes; }

  int APieceComplete(size_t idx);
  int GetHashValue(size_t idx,unsigned char *md);

  ssize_t ReadSlice(char *buf,size_t idx,size_t off,size_t len);
  ssize_t WriteSlice(char *buf,size_t idx,size_t off,size_t len);
  ssize_t ReadPiece(char *buf,size_t idx);

  int PrintOut();
  int SeedTimeout(const time_t *pnow);
};

extern btContent BTCONTENT;

#endif
