#ifndef SCREEN_ENCODING_H
#define SCREEN_ENCODING_H

#include "window.h"

void  InitBuiltinTabs (void);
struct mchar *recode_mchar (struct mchar *, int, int);
struct mline *recode_mline (struct mline *, int, int, int);
int   FromUtf8 (int, int *);
void  AddUtf8 (uint32_t);
size_t ToUtf8 (char *, uint32_t);
size_t ToUtf8_comb (char *, uint32_t);
bool  utf8_isdouble (uint32_t);
bool  utf8_iscomb (uint32_t);
void  utf8_handle_comb (unsigned int, struct mchar *);
int   ContainsSpecialDeffont (struct mline *, int, int, int);
int   LoadFontTranslation (int, char *);
void  LoadFontTranslationsForEncoding (int);
void  WinSwitchEncoding (Window *, int);
int   FindEncoding (char *);
char *EncodingName (int);
int   EncodingDefFont (int);
void  ResetEncoding (Window *);
int   CanEncodeFont (int, int);
int   DecodeChar (int, int, int *);
int   RecodeBuf (unsigned char *, int, int, int, unsigned char *);
int   PrepareEncodedChar (int);
int   EncodeChar (char *, int, int, int *);

#endif /* SCREEN_ENCODING_H */
