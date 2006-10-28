#ifndef BTFILES_H
#define BTFILES_H

#include <sys/types.h>
#include <stdio.h>
#include "./def.h"

typedef struct _btfile{
  char *bf_filename;	// full path of file.
  size_t bf_length;		//single file length limits to 4 GB
  FILE *bf_fp;

  time_t bf_last_timestamp;	// last io timestamp.

  size_t bf_completed;		// already downloaded length

  unsigned char bf_flag_opened:1;
  unsigned char bf_flag_need:1;
  unsigned char bf_reserved:6;

  struct _btfile *bf_next;
}BTFILE;


class btFiles
{
 private:
  
  BTFILE *m_btfhead;
  char *m_directory;
  u_int64_t m_total_files_length;
  size_t m_total_opened;	// already opened

  u_int8_t m_flag_automanage:1;
  u_int8_t m_flag_reserved:7;	// current version not implement

  BTFILE* _new_bfnode();
  int _btf_open(BTFILE *sbf_p);
  int _btf_ftruncate(int fd,size_t length);
  int _btf_creat_by_path(const char *pathname, size_t file_length);
  int _btf_destroy();
  int _btf_recurses_directory(const char *cur_path, BTFILE *lastnode);

 public:
  int CreateFiles();

  btFiles();
  ~btFiles();
  
  int BuildFromFS(const char *pathname);
  int BuildFromMI(const char *metabuf, const size_t metabuf_len, const char *saveas);

  u_int64_t GetTotalLength() const { return m_total_files_length; }
  ssize_t IO(char *buf, u_int64_t off, size_t len, const int iotype);
  size_t FillMetaInfo(FILE* fp);
#ifndef WINDOWS
  void PrintOut();
#endif
};

#endif
