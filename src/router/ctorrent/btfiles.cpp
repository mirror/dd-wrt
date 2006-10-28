#include "btfiles.h"

#ifdef WINDOWS
#include <io.h>
#include <memory.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/param.h>
#include <openssl/sha.h>
#endif

#include <time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "bencode.h"

#define MAX_OPEN_FILES 20

btFiles::btFiles()
{
  m_btfhead = (BTFILE*) 0;
  m_total_files_length = 0;
  m_total_opened = 0;
  m_flag_automanage = 0;
  m_directory = (char*)0;
}

btFiles::~btFiles()
{
  _btf_destroy();
}

BTFILE* btFiles::_new_bfnode()
{
  BTFILE *pnew = new BTFILE;

#ifndef WINDOWS
  if( !pnew ) return (BTFILE*) 0;
#endif

  pnew->bf_flag_opened = 0;
  pnew->bf_flag_need = 0;

  pnew->bf_filename = (char*) 0;
  pnew->bf_fp = (FILE*) 0;
  pnew->bf_length = 0;

  pnew->bf_last_timestamp = (time_t) 0;
  pnew->bf_next = (BTFILE*) 0;
  return pnew;
}

int btFiles::_btf_open(BTFILE *pbf)
{
  char fn[MAXPATHLEN];
  
  if(m_flag_automanage && (m_total_opened >= MAX_OPEN_FILES)){ // close any files.
    BTFILE *pbf_n,*pbf_close;
    pbf_close = (BTFILE *) 0;
    for(pbf_n = m_btfhead; pbf_n ; pbf_n = pbf_n->bf_next){
      if(!pbf_n->bf_flag_opened) continue; // file not been opened.
      if( !pbf_close || pbf_n->bf_last_timestamp < pbf_close->bf_last_timestamp) 
	pbf_close = pbf_n;
    }
    if(!pbf_close || fclose(pbf_close->bf_fp) < 0) return -1;
    pbf_close->bf_flag_opened = 0;
    m_total_opened--;
  }

  if( m_directory ){
    if( MAXPATHLEN <= snprintf(fn, MAXPATHLEN, "%s%c%s", m_directory, PATH_SP, pbf->bf_filename) )
      return -1;
  }else{
    strcpy(fn, pbf->bf_filename);
  }
  
  if( !(pbf->bf_fp = fopen(fn,"r+")) ) return -1;

  pbf->bf_flag_opened = 1;
  m_total_opened++;
  return 0;
}

ssize_t btFiles::IO(char *buf, u_int64_t off, size_t len, const int iotype)
{
  u_int64_t n = 0;
  size_t pos,nio;
  BTFILE *pbf = m_btfhead;

  if( ( off + (u_int64_t)len ) > m_total_files_length) return -1;

  for(; pbf; pbf = pbf->bf_next){
    n += (u_int64_t) pbf->bf_length;
    if(n > off) break;
  }

  if( !pbf ) return -1;

  pos = (size_t) (off - (n - pbf->bf_length));

  for(; len ;){
    if( !pbf->bf_flag_opened ){
      if( _btf_open(pbf) < 0 ) return -1;
    }

    if( m_flag_automanage ) time(&pbf->bf_last_timestamp);

    if( fseek(pbf->bf_fp,(long) pos,SEEK_SET) < 0) return -1;

    nio = (len < pbf->bf_length - pos) ? len : (pbf->bf_length - pos);

    if(0 == iotype){
      if( 1 != fread(buf,nio,1,pbf->bf_fp) ) return -1;
    }else{
      if( 1 != fwrite(buf,nio,1,pbf->bf_fp) ) return -1;
    }

    len -= nio;
    buf += nio;

    if( len ){
      pbf = pbf->bf_next;
      if( !pbf ) return -1;
      pos = 0;
    }
  } // end for
  return 0;
}

int btFiles::_btf_destroy()
{
  BTFILE *pbf,*pbf_next;
  for(pbf = m_btfhead; pbf;){
    pbf_next = pbf->bf_next;
    if( pbf->bf_fp ) fclose( pbf->bf_fp );
    if( pbf->bf_filename ) delete []pbf->bf_filename;
    delete pbf;
    pbf = pbf_next;
  }
  m_btfhead = (BTFILE*) 0;
  m_total_files_length = (u_int64_t) 0;
  m_total_opened = 0;
  return 0;
}

int btFiles::_btf_ftruncate(int fd,size_t length)
{
#ifdef WINDOWS
  char c = (char)0;
  if(lseek(fd,length - 1, SEEK_SET) < 0 ) return -1;
  return write(fd, &c, 1);
#else
  return ftruncate(fd,length);
#endif
}

int btFiles::_btf_recurses_directory(const char *cur_path, BTFILE* lastnode)
{
  char full_cur[MAXPATHLEN];
  char fn[MAXPATHLEN];
  struct stat sb;
  struct dirent *dirp;
  DIR *dp;
  BTFILE *pbf;

  if( !getcwd(full_cur,MAXPATHLEN) ) return -1;

  if( cur_path ){
    strcpy(fn, full_cur);
    if( MAXPATHLEN <= snprintf(full_cur, MAXPATHLEN, "%s%c%s", fn, PATH_SP, cur_path))
      return -1;
  }
      
  if( (DIR*) 0 == (dp = opendir(full_cur))){
    fprintf(stderr,"error, open directory %s failed, %s\n",cur_path,strerror(errno));
    return -1;
  }

  while( (struct dirent*) 0 != (dirp = readdir(dp)) ){
    
    if( 0 == strcmp(dirp->d_name, ".") ||
	0 == strcmp(dirp->d_name, "..") ) continue;

    if( cur_path ){
      if(MAXPATHLEN < snprintf(fn, MAXPATHLEN, "%s%c%s", cur_path, PATH_SP, dirp->d_name)){
	fprintf(stderr,"error, pathname too long\n");
	return -1;
      }
    }else{
      strcpy(fn, dirp->d_name);
    }

    if( stat(fn, &sb) < 0 ){
      fprintf(stderr,"error, stat %s failed, %s\n",fn,strerror(errno));
      return -1;
    }

    if( S_IFREG & sb.st_mode ){
      
      pbf = _new_bfnode();
#ifndef WINDOWS
      if( !pbf ) return -1;
#endif
      pbf->bf_filename = new char[strlen(fn) + 1];
#ifndef WINDOWS
      if( !pbf->bf_filename ){ closedir(dp); return -1;}
#endif
      strcpy(pbf->bf_filename, fn);
      
      pbf->bf_length = sb.st_size;
      m_total_files_length += sb.st_size;

      if( lastnode ) lastnode->bf_next = pbf; else m_btfhead = pbf;
      
      lastnode = pbf;

    }else if( S_IFDIR & sb.st_mode ){
      if(_btf_recurses_directory(fn, lastnode) < 0){closedir(dp); return -1;}
    }else{
      fprintf(stderr,"error, %s is not a directory or regular file.\n",fn);
      closedir(dp);
      return -1;
    }
  } // end while
  closedir(dp);
  return 0;
}

int btFiles::_btf_creat_by_path(const char *pathname, size_t file_length)
{
  struct stat sb;
  int fd;
  char *p,*pnext,last = 0;
  char sp[MAXPATHLEN];

  strcpy(sp,pathname);

  pnext = sp;
  if(PATH_SP == *pnext) pnext++;

  for(; !last; ){
    for(p = pnext; *p && PATH_SP != *p; p++) ;
    if( !*p ) last = 1;
    if(last && PATH_SP == *p){ last = 0; break;}
    *p = '\0';
    if(stat(sp,&sb) < 0){
      if( ENOENT == errno ){
	if( !last ){
#ifdef WINDOWS
	  if(mkdir(sp) < 0) break;
#else
	  if(mkdir(sp,0755) < 0) break;
#endif
	}else{
	  if((fd = creat(sp,0644)) < 0) { last = 0; break; }
	  if(file_length && _btf_ftruncate(fd, file_length) < 0){close(fd); last = 0; break;}
	  close(fd);
	}
      }else{last = 0; break;}
    }
    if( !last ){ *p = PATH_SP; pnext = p + 1;}
  }
  return last;
}

int btFiles::BuildFromFS(const char *pathname)
{
  struct stat sb;
  BTFILE *pbf = (BTFILE*) 0;

  if( stat(pathname, &sb) < 0 ){
    fprintf(stderr,"error, stat file %s failed, %s\n",pathname,strerror(errno));
    return -1;
  }

  if( S_IFREG & sb.st_mode ){
    pbf = _new_bfnode();
#ifndef WINDOWS
    if( !pbf ) return -1;
#endif
    pbf->bf_length = m_total_files_length = sb.st_size;
    pbf->bf_filename = new char[strlen(pathname) + 1];
#ifndef WINDOWS
    if( !pbf->bf_filename ) return -1;
#endif
    strcpy(pbf->bf_filename, pathname);
    m_btfhead = pbf;
  }else if( S_IFDIR & sb.st_mode ){
    char wd[MAXPATHLEN];
    if( !getcwd(wd,MAXPATHLEN) ) return -1;
    m_directory = new char[strlen(pathname) + 1];
#ifndef WINDOWS
    if( !m_directory ) return -1;
#endif
    strcpy(m_directory, pathname);
    
    if(chdir(m_directory) < 0){
      fprintf(stderr,"error, change work directory to %s failed, %s",m_directory, strerror(errno));
      return -1;
    }

    if(_btf_recurses_directory((const char*)0, (BTFILE*) 0) < 0) return -1;
    if( chdir(wd) < 0) return -1;
  }else{
    fprintf(stderr,"error, %s is not a directory or regular file.\n",pathname);
    return -1;
  }
  return 0;
}

int btFiles::BuildFromMI(const char *metabuf, const size_t metabuf_len, const char *saveas)
{
  char path[MAXPATHLEN];
  const char *s, *p;
  size_t r,q,n;
  if( !decode_query(metabuf, metabuf_len, "info|name",&s,&q,QUERY_STR) ||
      MAXPATHLEN <= q) return -1;

  memcpy(path, s, q);
  path[q] = '\0';

  r = decode_query(metabuf,metabuf_len,"info|files",(const char**) 0, &q,QUERY_POS);

  if( r ){
    BTFILE *pbf_last = (BTFILE*) 0; 
    BTFILE *pbf = (BTFILE*) 0;
    size_t dl;
    if( decode_query(metabuf,metabuf_len,"info|length",
		     (const char**) 0,(size_t*) 0,QUERY_INT) )
      return -1;

    if( saveas ){
      m_directory = new char[strlen(saveas) + 1];
#ifndef WINDOWS
      if(!m_directory) return -1;
#endif
      strcpy(m_directory,saveas);
    }else{
      m_directory = new char[strlen(path) + 1];
#ifndef WINDOWS
      if( !m_directory) return -1;
#endif
      strcpy(m_directory,path);
    }

    /* now r saved the pos of files list. q saved list length */
    p = metabuf + r + 1; 
    q--;
    for(; q && 'e' != *p; p += dl, q -= dl){
      if(!(dl = decode_dict(p, q, (const char*) 0)) ) return -1;
      if( !decode_query(p, dl, "length", (const char**) 0,
			&r,QUERY_INT) ) return -1;
      pbf = _new_bfnode();
#ifndef WINDOWS
      if( !pbf ) return -1;
#endif
      pbf->bf_length = r;
      m_total_files_length += r;
      r = decode_query(p, dl, "path", (const char **) 0, &n,QUERY_POS);
      if( !r ) return -1;
      if(!decode_list2path(p + r, n, path)) return -1;
      pbf->bf_filename = new char[strlen(path) + 1];
#ifndef WINDOWS
      if( !pbf->bf_filename ) return -1;
#endif
      strcpy(pbf->bf_filename, path);
      if(pbf_last) pbf_last->bf_next = pbf; else m_btfhead = pbf;
      pbf_last = pbf;
    }
  }else{
    if( !decode_query(metabuf,metabuf_len,"info|length",
		      (const char**) 0,(size_t*) &q,QUERY_INT) )
      return -1;
    m_btfhead = _new_bfnode();
#ifndef WINDOWS
    if( !m_btfhead) return -1;
#endif
    m_btfhead->bf_length = m_total_files_length = q;
    if( saveas ){
      m_btfhead->bf_filename = new char[strlen(saveas) + 1];
#ifndef WINDOWS
      if(!m_btfhead->bf_filename ) return -1;
#endif
      strcpy(m_btfhead->bf_filename, saveas);
    }else{
      m_btfhead->bf_filename = new char[strlen(path) + 1];
#ifndef WINDOWS
      if(!m_btfhead->bf_filename ) return -1;
#endif
      strcpy(m_btfhead->bf_filename, path);
    }
  }
  return 0;
}

int btFiles::CreateFiles()
{
  int check_exist = 0;
  char fn[MAXPATHLEN];
  BTFILE *pbt = m_btfhead;
  struct stat sb;

  for(; pbt; pbt = pbt->bf_next){
    if( m_directory ){
      if( MAXPATHLEN <= snprintf(fn, MAXPATHLEN, "%s%c%s", m_directory, PATH_SP, pbt->bf_filename) )
	return -1;
    }else{
      strcpy(fn, pbt->bf_filename);
    }
    
    if(stat(fn ,&sb) < 0){
      if(ENOENT == errno){
	if( !_btf_creat_by_path(fn,pbt->bf_length)){
	  fprintf(stderr,"error, create file %s failed.\n",fn);
	  return -1;
	}
      }else{
	fprintf(stderr,"error, couldn't create file %s\n", fn);
	return -1;
      }
    }else{
      if( !check_exist) check_exist = 1;
      if( !(S_IFREG & sb.st_mode) ){
	fprintf(stderr,"error, file %s not a regular file.\n", fn);
	return -1;
      }
      if(sb.st_size != pbt->bf_length){
	fprintf(stderr,"error, file %s 's size not match. must be %u\n",
		fn, pbt->bf_length);
	return -1;
      }
    }
  } //end for
  return check_exist;
}

void btFiles::PrintOut()
{
  BTFILE *p = m_btfhead;
  size_t id = 1;
  printf("FILES INFO\n");
  if(m_directory) printf("Directory: %s\n",m_directory);
  for( ; p ; p = p->bf_next ){
    printf("<%d> %c%s [%u]\n",id++,
	   m_directory ? '\t': ' ',p->bf_filename, p->bf_length);
  }
  printf("Total: %lu MB\n\n",(unsigned long)(m_total_files_length / 1024 / 1024));
}

size_t btFiles::FillMetaInfo(FILE* fp)
{
  BTFILE *p;
  if( m_directory ){
    // multi files
    if( bencode_str("files", fp) != 1 ) return 0;

    if( bencode_begin_list(fp) != 1) return 0;
    
    for( p = m_btfhead; p; p = p->bf_next){
      if( bencode_begin_dict(fp) != 1) return 0;
      
      if( bencode_str("length", fp) != 1 ) return 0;
      if( bencode_int(p->bf_length, fp) != 1) return 0;

      if( bencode_str("path", fp) != 1) return 0;
      if( bencode_path2list(p->bf_filename, fp) != 1 ) return 0;
      
      if( bencode_end_dict_list(fp) != 1) return 0;
    }
    
    if(bencode_end_dict_list(fp) != 1 ) return 0;
    
    if(bencode_str("name", fp) != 1) return 0;
    return bencode_str(m_directory, fp);
    
  }else{
    if( bencode_str("length", fp) != 1 ) return 0;
    if( bencode_int(m_btfhead->bf_length, fp) != 1) return 0;
    
    if( bencode_str("name", fp) != 1 ) return 0;
    return bencode_str(m_btfhead->bf_filename, fp);
  }
  return 1;
}
