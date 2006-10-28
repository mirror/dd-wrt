#include "btcontent.h"

#ifdef WINDOWS
#include <direct.h>
#include <io.h>
#include <memory.h>
// include windows sha1 header here.

#else
#include <unistd.h>
#include <sys/param.h>
#include <openssl/sha.h>
#endif

#include <time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "btconfig.h"
#include "bencode.h"
#include "peer.h"
#include "httpencode.h"

#define meta_str(keylist,pstr,pint) decode_query(b,flen,(keylist),(pstr),(pint),QUERY_STR)
#define meta_int(keylist,pint) decode_query(b,flen,(keylist),(const char**) 0,(pint),QUERY_INT)
#define meta_pos(keylist) decode_query(b,flen,(keylist),(const char**) 0,(size_t*) 0,QUERY_POS)

#define CACHE_FIT(ca,roff,rlen)	\
(max_u_int64_t((ca)->bc_off,(roff)) <= \
 min_u_int64_t(((ca)->bc_off + (ca)->bc_len - 1),(roff + rlen - 1)))

#define MAX_OPEN_FILES 20

btContent BTCONTENT;

static void Sha1(char *ptr,size_t len,unsigned char *dm)
{
#ifdef WINDOWS
  ;
#else
  SHA_CTX context;
  SHA1_Init(&context);
  SHA1_Update(&context,(unsigned char*)ptr,len);
  SHA1_Final(dm,&context);
#endif
}

btContent::btContent()
{
  m_announce = global_piece_buffer = (char*) 0;
  m_hash_table = (unsigned char *) 0;
  pBF = (BitField*) 0;
  m_create_date = m_seed_timestamp = (time_t) 0;
  time(&m_start_timestamp);
  m_cache = (BTCACHE*) 0;
  m_cache_size = m_cache_used = 0;
}

int btContent::CreateMetainfoFile(const char *mifn)
{
  FILE *fp;
  fp = fopen(mifn, "r");
  if( fp ){
    fprintf(stderr,"error, file %s already exist.\n",mifn);
    return -1;
  }else if( ENOENT != errno ){
    fprintf(stderr,"error, couldn't create %s.\n",mifn);
    return -1;
  }

  fp = fopen(mifn, "w");

  if( !fp ){
    fprintf(stderr,"error, open %s failed. %s\n",mifn, strerror(errno));
    return -1;
  }
  if( bencode_begin_dict(fp) != 1 ) goto err;

  // announce
  if( bencode_str("announce", fp) != 1 ) goto err;
  if( bencode_str(m_announce, fp) !=1 ) goto err;
  // create date
  if( bencode_str("creation date", fp) != 1) goto err;
  if( bencode_int(m_create_date, fp) != 1 ) goto err;

  // info dict
  if( bencode_str("info", fp) != 1) goto err;
  if( bencode_begin_dict(fp) != 1 ) goto err;

  if( m_btfiles.FillMetaInfo(fp) != 1 ) goto err;

  // piece length
  if( bencode_str("piece length", fp) != 1 ) goto err;
  if( bencode_int(m_piece_length, fp) != 1 ) goto err;
  
  // hash table;
  if( bencode_str("pieces", fp) != 1) goto err;
  if( bencode_buf((const char*) m_hash_table, m_hashtable_length, fp) != 1 ) goto err;

  if( bencode_end_dict_list(fp) != 1 ) goto err; // end info
  if( bencode_end_dict_list(fp) != 1 ) goto err; // end torrent

  fclose(fp);
  return 0;
 err:
  if( fp ) fclose(fp);
  return -1;
}

int btContent::InitialFromFS(const char *pathname, char *ann_url, size_t piece_length)
{
  size_t n, percent;

  // piece length
  m_piece_length = piece_length;
  if( m_piece_length % 65536 ){ 
    m_piece_length /= 65536;
    m_piece_length *= 65536;
  }

  if( !m_piece_length || m_piece_length > cfg_req_queue_length * cfg_req_slice_size )
    m_piece_length = 262144;
  
  m_announce = ann_url;
  m_create_date = time((time_t*) 0);

  if(m_btfiles.BuildFromFS(pathname) < 0) return -1;

  global_piece_buffer = new char[m_piece_length];
#ifndef WINDOWS
  if( !global_piece_buffer ) return -1;
#endif
  
  // n pieces
  m_npieces = m_btfiles.GetTotalLength() / m_piece_length;
  if( m_btfiles.GetTotalLength() % m_piece_length ) m_npieces++;

  // create hash table.
  m_hashtable_length = m_npieces * 20;
  m_hash_table = new unsigned char[m_hashtable_length];
#ifndef WINDOWS
  if( !m_hash_table ) return -1;
#endif

  percent = m_npieces / 100;
  if( !percent ) percent = 1;

  for( n = 0; n < m_npieces; n++){
    if( GetHashValue(n, m_hash_table + n * 20) < 0) return -1;
    if( 0 == n % percent ){
      printf("\rCreate hash table: %u/%u", n, m_npieces);
      fflush(stdout);
    }
  }
  printf("Complete.\n");

  return 0;
}

int btContent::PrintOut()
{
  printf("META INFO\n");
  printf("Announce: %s\n",m_announce);
  if( m_create_date ) printf("Created On: %s",ctime(&m_create_date));
  printf("Piece length: %u\n\n",m_piece_length);
  m_btfiles.PrintOut();
  return 0;
}

int btContent::InitialFromMI(const char *metainfo_fname,const char *saveas)
{
#define ERR_RETURN()	{if(b) delete []b; return -1;}
  unsigned char *ptr = m_shake_buffer;
  char *b;
  const char *s;
  size_t flen, q, r;

  b = _file2mem(metainfo_fname,&flen);
  if ( !b ) return -1;

  // announce
  if( !meta_str("announce",&s,&r) ) ERR_RETURN();
  if( r > MAXPATHLEN ) ERR_RETURN();
  m_announce = new char [r + 1];
  memcpy(m_announce, s, r);
  m_announce[r] = '\0';
  
  // infohash
  if( !(r = meta_pos("info")) ) ERR_RETURN();
  if( !(q = decode_dict(b + r, flen - r, (char *) 0)) ) ERR_RETURN();
  Sha1(b + r, q, m_shake_buffer + 28);

  if( meta_int("creation date",&r)) m_create_date = (time_t) r;
 
  // hash table
  if( !meta_str("info|pieces",&s,&m_hashtable_length) ||
      m_hashtable_length % 20 != 0) ERR_RETURN();

  m_hash_table = new unsigned char[m_hashtable_length];

#ifndef WINDOWS
  if( !m_hash_table ) ERR_RETURN();
#endif
  memcpy(m_hash_table, s, m_hashtable_length);

  if(!meta_int("info|piece length",&m_piece_length)) ERR_RETURN();
  m_npieces = m_hashtable_length / 20;

  if( m_piece_length > cfg_max_slice_size * cfg_req_queue_length ){
    fprintf(stderr,"error, piece length too long[%u]. please recompile CTorrent with a larger cfg_max_slice_size in <btconfig.h>.\n", m_piece_length);
    ERR_RETURN();
  }

  if( m_piece_length < cfg_req_slice_size )
    cfg_req_slice_size = m_piece_length;
  else{
    for( ;(m_piece_length / cfg_req_slice_size) >= cfg_req_queue_length; ){
      cfg_req_slice_size *= 2;
      if( cfg_req_slice_size > cfg_max_slice_size ) ERR_RETURN();
    }
  }
  
  if( m_btfiles.BuildFromMI(b, flen, saveas) < 0) ERR_RETURN();

  delete []b;
  PrintOut();
  
  if( arg_flg_exam_only ) return 0;

  if( ( r = m_btfiles.CreateFiles() ) < 0) ERR_RETURN();

  global_piece_buffer = new char[m_piece_length];
#ifndef WINDOWS
  if( !global_piece_buffer ) ERR_RETURN();
#endif

  pBF = new BitField(m_npieces);
#ifndef WINDOWS
  if( !pBF ) ERR_RETURN();
#endif

  m_left_bytes = m_btfiles.GetTotalLength() / m_piece_length;
  if( m_btfiles.GetTotalLength() % m_piece_length ) m_left_bytes++;
  if( m_left_bytes != m_npieces ) ERR_RETURN();
  
  m_left_bytes = m_btfiles.GetTotalLength();

  if( arg_bitfield_file ){

    if( !arg_flg_check_only ){
      if( pBF->SetReferFile(arg_bitfield_file) >= 0){
	size_t idx;
	r = 0;
	for( idx = 0; idx < m_npieces; idx++ )
	  if( pBF->IsSet(idx) ) m_left_bytes -= GetPieceLength(idx);
      }
      else{
	fprintf(stderr,"warn, couldn't set bit field refer file %s.\n",arg_bitfield_file);
      }
    }
    
    if( r ) CheckExist();
    
  }else if( arg_flg_force_seed_mode ){
    pBF->SetAll();
    m_left_bytes = 0;
  }else if( r ){
    CheckExist();
  }
  
  printf("Already/Total: %u/%u\n",pBF->Count(),m_npieces);
  
  if( arg_flg_check_only ){
    if( arg_bitfield_file ) pBF->WriteToFile(arg_bitfield_file);
    exit(1);
  }
  
  CacheConfigure();

  *ptr = (unsigned char) 19; ptr++; // protocol string length
  memcpy(ptr,"BitTorrent protocol",19); ptr += 19; //  protocol string
  memset(ptr,0,8);		// reserved set zero.

  {				// peer id
	char *sptr = arg_user_agent;
	char *dptr = (char *)m_shake_buffer + 48;
	char *eptr = dptr + PEER_ID_LEN;
	while (*sptr) *dptr++ = *sptr++;
	while (dptr < eptr) *dptr++ = (unsigned char)random();
  }
  return 0;
}

btContent::~btContent()
{
  if(m_hash_table) delete []m_hash_table;
  if(m_announce) delete []m_announce;
  if(global_piece_buffer) delete []global_piece_buffer;
  if(pBF) delete pBF;
}

void btContent::_Set_InfoHash(unsigned char buf[20]) 
{ 
  memcpy(m_shake_buffer + 28, buf, 20);
}

ssize_t btContent::ReadSlice(char *buf,size_t idx,size_t off,size_t len)
{
  u_int64_t offset = idx * m_piece_length + off;

  if( !m_cache_size ) return m_btfiles.IO(buf, offset, len, 0);
  else{
    size_t len2;
    int flg_rescan;
    BTCACHE *p = m_cache;
  
    for( ; p && (offset + len) > p->bc_off && !CACHE_FIT(p,offset,len); p = p->bc_next) ;

    for( ; len && p && CACHE_FIT(p, offset, len);){
      flg_rescan = 0;
      if( offset < p->bc_off ){
	len2 = p->bc_off - offset;
	if( CacheIO(buf, offset, len2, 0) < 0) return -1;
	flg_rescan = 1;
      }else if( offset > p->bc_off ){
	len2 = p->bc_off + p->bc_len - offset;
	if( len2 > len ) len2 = len;
	memcpy(buf, p->bc_buf + offset - p->bc_off, len2);
      }else{
	len2 = (len > p->bc_len) ? p->bc_len : len;
	memcpy(buf, p->bc_buf, len2);
      }

      buf += len2;
      offset += len2;
      len -= len2;

      if( len ){
	if( flg_rescan ){
	  for( p = m_cache;
	       p && (offset + len) > p->bc_off && !CACHE_FIT(p,offset,len);
	       p = p->bc_next) ;
	}else{
	  time(&p->bc_last_timestamp);
	  p = p->bc_next;
	}
      }
    }// end for;
  
    if( len ) return CacheIO(buf, offset, len, 0);
  }
  return 0;
}

void btContent::CacheClean()
{
  BTCACHE *p, *pp, *prm, *prmp;

 again:
  pp = prm = prmp = (BTCACHE*) 0;
  for( p = m_cache; p; p = p->bc_next){
    if( !p->bc_f_flush ){
      if( !prm || prm->bc_last_timestamp > p->bc_last_timestamp){ prm = p; prmp = pp;}
    }
    pp = p;
  }
  
  if( !prm ){
    if( m_cache_used ) { FlushCache(); goto again; }
    else return;
  }

  if( prmp ) prmp->bc_next = prm->bc_next; else m_cache = prm->bc_next;

  m_cache_used -= prm->bc_len;
  
  delete []prm->bc_buf;
  delete prm;
}

void btContent::CacheConfigure()
{
  if( cfg_cache_size ){
    if( cfg_cache_size > 128 ) cfg_cache_size = 128;
    
    m_cache_size = cfg_cache_size * 1024 * 1024;
    
    if( m_cache_size < 4 * m_piece_length ) m_cache_size = 4 * m_piece_length;
  }
}

void btContent::FlushCache()
{
  BTCACHE *p = m_cache;
  for( ; p; p = p->bc_next)
    if( p->bc_f_flush ){
      p->bc_f_flush = 0;
      if(m_btfiles.IO(p->bc_buf, p->bc_off, p->bc_len, 1) < 0)
	fprintf(stderr,"warn, write file failed while flush cache.\n");
    }
}

ssize_t btContent::WriteSlice(char *buf,size_t idx,size_t off,size_t len)
{
  u_int64_t offset = (u_int64_t)(idx * m_piece_length + off);

  if( !m_cache_size ) return m_btfiles.IO(buf, offset, len, 1);
  else{
    size_t len2;
    int flg_rescan;
    BTCACHE *p;
  
    for(p = m_cache ; p && (offset + len) > p->bc_off && !CACHE_FIT(p,offset,len); p = p->bc_next) ;

    for( ; len && p && CACHE_FIT(p, offset, len);){
      flg_rescan = 0;
      if( offset < p->bc_off ){
	len2 = p->bc_off - offset;
	if( CacheIO(buf, offset, len2, 1) < 0) return -1;
	flg_rescan = 1;
      }else if( offset > p->bc_off ){
	len2 = p->bc_off + p->bc_len - offset;
	if( len2 > len ) len2 = len;
	memcpy(p->bc_buf + offset - p->bc_off, buf, len2);
	p->bc_f_flush = 1;
      }else{
	len2 = (len > p->bc_len) ? p->bc_len : len;
	memcpy(p->bc_buf, buf, len2);
	p->bc_f_flush = 1;
      }

      buf += len2;
      offset += len2;
      len -= len2;

      if( len ){
	if( flg_rescan ){
	  for( p = m_cache; p && (offset + len) > p->bc_off && !CACHE_FIT(p,offset,len); p = p->bc_next) ;
	}else{
	  time(&p->bc_last_timestamp);
	  p = p->bc_next;
	}
      }
    }// end for;
  
    if( len ) return CacheIO(buf, offset, len, 1);
  }
  return 0;
}

ssize_t btContent::CacheIO(char *buf, u_int64_t off, size_t len, int method)
{
  BTCACHE *p;
  BTCACHE *pp = (BTCACHE*) 0;
  BTCACHE *pnew = (BTCACHE*) 0;

  for( ; m_cache_size < (m_cache_used + len); ) CacheClean();
  
  if( 0 == method && m_btfiles.IO(buf, off, len, method) < 0) return -1;
  
  pnew = new BTCACHE;

#ifndef WINDOWS
  if( !pnew )
    return method ? m_btfiles.IO(buf, off, len, method) : 0;
#endif

  pnew->bc_buf = new char[len];

#ifndef WINDOWS
  if( !(pnew->bc_buf) ){ 
    delete pnew; 
    return method ? m_btfiles.IO(buf, off, len, method) : 0;
  }
#endif
  
  memcpy(pnew->bc_buf, buf, len);
  pnew->bc_off = off;
  pnew->bc_len = len;
  pnew->bc_f_flush = method;
  m_cache_used += len;
  time(&pnew->bc_last_timestamp);
  
  // find insert point.
  for(p = m_cache; p && off > p->bc_off; pp = p, p = pp->bc_next) ;

  pnew->bc_next = p;

  if( pp ) pp->bc_next = pnew; else m_cache = pnew;
  return 0;
}

ssize_t btContent::ReadPiece(char *buf,size_t idx)
{
  return ReadSlice(buf, idx, 0, GetPieceLength(idx));
}

size_t btContent::GetPieceLength(size_t idx)
{
  return (idx == m_btfiles.GetTotalLength() / m_piece_length) ?
    (size_t)(m_btfiles.GetTotalLength() % m_piece_length) 
    :m_piece_length;
}

int btContent::CheckExist()
{
  size_t idx = 0;
  size_t percent = GetNPieces() / 100;
  unsigned char md[20];

  if( !percent ) percent = 1;

  for( ; idx < m_npieces; idx++){
    if( GetHashValue(idx, md) == 0 && memcmp(md, m_hash_table + idx * 20, 20) == 0){
      m_left_bytes -= GetPieceLength(idx);
      pBF->Set(idx);
    }
    if(idx % percent == 0){
      printf("\rCheck exist: %d/%d",idx,pBF->NBits());
      fflush(stdout);
    }
  }
  printf(" Complete\n");
  return 0;
}

char* btContent::_file2mem(const char *fname, size_t *psiz)
{
  char *b = (char*) 0;
  struct stat sb;
  FILE* fp;
  fp = fopen(fname,"r");
  if( !fp ){
    fprintf(stderr,"error, open %s failed. %s\n",fname,strerror(errno));
    return (char*) 0;
  }

  if(stat(fname,&sb) < 0){
    fprintf(stderr,"error, stat %s failed, %s\n",fname,strerror(errno));
    return (char*) 0;
  }

  if( sb.st_size > MAX_METAINFO_FILESIZ ){
    fprintf(stderr,"error, %s is really a metainfo file???\n",fname);
    return (char*) 0;
  }

  b = new char[sb.st_size];
#ifndef WINDOWS
  if( !b ) return (char*) 0;
#endif

  if(fread(b, sb.st_size, 1, fp) != 1){
    if( ferror(fp) ){
      delete []b;
      return (char*) 0;
    }
  }
  fclose(fp);

  if(psiz) *psiz = sb.st_size;
  return b;
}

int btContent::APieceComplete(size_t idx)
{
  unsigned char md[20];
  if(pBF->IsSet(idx)) return 1;
  if( GetHashValue(idx, md) < 0 ) return -1;

  if( memcmp(md,(m_hash_table + idx * 20), 20) != 0){
    fprintf(stderr,"warn,piece %d hash check failed.\n",idx);
    return 0;
  }

  pBF->Set(idx);
  m_left_bytes -= GetPieceLength(idx);
  return 1;
}

int btContent::GetHashValue(size_t idx,unsigned char *md)
{
  if( ReadPiece(global_piece_buffer,idx) < 0) return -1;
  Sha1(global_piece_buffer,GetPieceLength(idx),md);
  return 0;
}

int btContent::SeedTimeout(const time_t *pnow)
{
  if( pBF->IsFull() ){
    if( !m_seed_timestamp ){
      Self.ResetDLTimer();
      Self.ResetULTimer();
      ReleaseHashTable();
      m_seed_timestamp = *pnow;
      FlushCache();
      printf("\nDownload complete.\n");
      printf("Total time used: %lu minutes.\n",(*pnow - m_start_timestamp) / 60);
      printf("Seed for other %lu hours.\n\n", cfg_seed_hours);
    }
    if( (*pnow - m_seed_timestamp) >= (cfg_seed_hours * 60 * 60) ) return 1;
  }
  return 0;
}
