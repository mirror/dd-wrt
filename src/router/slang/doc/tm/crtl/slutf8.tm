\function{SLutf8_skip_char}
\synopsis{Skip past a UTF-8 encoded character}
\usage{SLuchar_Type *SLutf8_skip_char (SLuchar_Type *u, SLuchar_Type *umax)}
\description
 The \cfun{SLutf8_skip_char} function returns a pointer to the
 character immediately following the UTF-8 encoded character at
 \exmp{u}.  It will make no attempt to examine the bytes at the
 position \exmp{umax} and beyond.  If the bytes at \exmp{u} do not
 represent a valid or legal UTF-8 encoded sequence, a pointer to the
 byte following \exmp{u} will be returned.
\notes
 Unicode combining characters are treated as distinct characters by
 this function.
\seealso{SLutf8_skip_chars, SLutf8_bskip_char, SLutf8_strlen}
\done

\function{SLutf8_skip_chars}
\synopsis{Skip past a specified number of characters in a UTF-8
 encoded string}
\usage{SLuchar_Type *SLutf8_skip_chars (u, umax, num, dnum, ignore_combining)}
#v+
    SLuchar_Type *u, *umax;
    unsigned int num;
    unsigned int *dnum;
    int ignore_combining;
#v-
\description
 This functions attempts to skip forward past \exmp{num} UTF-8 encoded
 characters at \exmp{u} returning the actual number skipped via the
 parameter \exmp{dnum}.  It will make no attempt to examine bytes at
 \exmp{umax} and beyond.  Unicode combining characters will not be
 counted if \exmp{ignore_combining} is non-zero, otherwise they will
 be treated as distinct characters.  If the input contains an
 invalid or illegal UTF-8 sequence, then each byte in the sequence
 will be treated as a single character.
\seealso{SLutf8_skip_char, SLutf8_bskip_chars}
\done

\function{SLutf8_bskip_char}
\synopsis{Skip backward past a UTF-8 encoded character}
\usage{SLuchar_Type *SLutf8_bskip_char (SLuchar_Type *umin, SLuchar_Type *u)}
\description
  The \cfun{SLutf8_bskip_char} skips backward to the start of the
  UTF-8 encoded character immediately before the position \exmp{u}.
  The function will make no attempt to examine characters before the
  position \exmp{umin}.  UTF-8 combining characters are treated as
  distinct characters.
\seealso{SLutf8_bskip_chars, SLutf8_skip_char}
\done

\function{SLutf8_bskip_chars}
\synopsis{Skip backward past a specified number of UTF-8 encoded characters}
\usage{SLuchar_Type *SLutf8_bskip_chars (umin, u, num, dnum, ignore_combining)}
#v+
   SLuchar_Type *umin, *u;
   unsigned int num;
   unsigned int *dnum;
   int ignore_combining;
#v-
\description
 This functions attempts to skip backward past \exmp{num} UTF-8
 encoded characters occurring immediately before \exmp{u}.  It returns
 the the actual number skipped via the parameter \exmp{dnum}.  No
 attempt will be made to examine the bytes occurring before \exmp{umin}.
 Unicode combining characters will not be counted if \exmp{ignore_combining}
 is non-zero, otherwise they will be treated as distinct characters.
 If the input contains an invalid or illegal UTF-8 sequence, then each
 byte in the sequence will be treated as a single character.
\seealso{SLutf8_skip_char, SLutf8_bskip_chars}
\done

\function{SLutf8_decode}
\synopsis{Decode a UTF-8 encoded character sequence}
\usage{SLuchar_Type *SLutf8_decode (u, umax, w, nconsumedp}
#v+
   SLuchar_Type *u, *umax;
   SLwchar_Type *w;
   unsigned int *nconsumedp;
#v-
\description
 The \cfun{SLutf8_decode} function decodes the UTF-8 encoded character
 occurring at \exmp{u} and returns the decoded character via the
 parameter \exmp{w}.  No attempt will be made to examine the bytes at
 \exmp{umax} and beyond.  If the parameter \exmp{nconsumedp} is
 non-NULL, then the number of bytes consumed by the function will
 be returned to it.  If the sequence at \exmp{u} is invalid or
 illegal, the function will return \NULL and with the number of
 bytes consumed by the function equal to the size of the invalid
 sequence.  Otherwise the function will return a pointer to byte
 following encoded sequence.
\seealso{SLutf8_decode, SLutf8_strlen, SLutf8_skip_char}
\done

\function{SLutf8_encode}
\synopsis{UTF-8 encode a character}
\usage{SLuchar_Type *SLutf8_encode (w, u, ulen)}
#v+
   SLwchar_Type w;
   SLuchar_Type *u;
   unsigned int ulen;
#v-
\description
 This function UTF-8 encodes the Unicode character represented by
 \exmp{w} and stored the encoded representation in the buffer of size
 \exmp{ulen} bytes at \exmp{u}.  The function will return \NULL if the
 size of the buffer is too small to represent the UTF-8 encoded
 character, otherwise it will return a pointer to the byte following
 encoded representation.
\notes
 This function does not null terminate the resulting byte sequence.
 The function \cfun{SLutf8_encode_null_terminate} may be used for that
 purpose.

 To guarantee that the buffer is large enough to hold the encoded
 bytes, its size should be at least \exmp{SLUTF8_MAX_BLEN} bytes.

 The function will encode illegal Unicode characters, i.e., characters
 in the range 0xD800-0xFFFF (the UTF-16 surrogates) and 0xFFFE-0xFFFF.
\seealso{SLutf8_decode, SLutf8_encode_bytes, SLutf8_encode_null_terminate}
\done

\function{SLutf8_strlen}
\synopsis{Determine the number of characters in a UTF-8 sequence}
\usage{unsigned int SLutf8_strlen (SLuchar_Type *s, int ignore_combining)}
\description
 This function may be used to determine the number of characters
 represented by the null-terminated UTF-8 byte sequence.  If the
 \exmp{ignore_combining} parameter is non-zero, then Unicode combining
 characters will not be counted.
\seealso{SLutf8_skip_chars, SLutf8_decode}
\done

\function{SLutf8_extract_utf8_char}
\synopsis{Extract a UTF-8 encoded character}
\usage{SLuchar_Type *SLutf8_extract_utf8_char (u, umax, buf)}
#v+
   SLuchar_Type *u, *umax, *buf;
#v-
\description
 This function extracts the bytes representing UTF-8 encoded character
 at \exmp{u} and places them in the buffer \exmp{buf}, and then null
 terminates the result.  The buffer is assumed to consist of at least
 \exmp{SLUTF8_MAX_BLEN+1} bytes, where the extra byte may be necessary
 for null termination.  No attempt will be made to examine the
 characters at \exmp{umax} and beyond.  If the byte-sequence at
 \exmp{u} is an illegal or invalid UTF-8 sequence, then the byte at
 \exmp{u} will be copied to the buffer.  The function returns a
 pointer to the byte following copied bytes.
\notes
 One may think of this function as the single byte analogue of
#v+
     if (u < umax)
       {
          buf[0] = *u++;
          buf[1] = 0;
       }
#v-
\seealso{SLutf8_decode, SLutf8_skip_char}
\done

\function{SLutf8_encode_null_terminate}
\synopsis{UTF-8 encode a character and null terminate the result}
\usage{SLuchar_Type *SLutf8_encode_null_terminate (w, buf)}
#v+
   SLwchar_Type w;
   SLuchar_Type *buf;
#v-
\description
 This function has the same functionality as \cfun{SLutf8_encode},
 except that it also null terminates the encoded sequences.  The
 buffer \exmp{buf}, where the encoded sequence is placed, is assumed
 to consist of at least \exmp{SLUTF8_MAX_BLEN+1} bytes.
\seealso{SLutf8_encode}
\done

\function{SLutf8_strup}
\synopsis{Uppercase a UTF-8 encoded string}
\usage{SLuchar_Type *SLutf8_strup (SLuchar_Type *u, SLuchar_Type *umax)}
\description
 The \cfun{SLutf8_strup} function returns the uppercase equivalent of
 UTF-8 encoded sequence of \exmp{umax-u} bytes at \exmp{u}.  The
 result will be returned as a null-terminated \exmp{SLstring} and
 should be freed with \cfun{SLang_free_slstring} when it is nolonger
 needed.  If the function encounters an invalid of illegal byte
 sequence, then the byte-sequence will be copied as as-is.
\seealso{SLutf8_strlow, SLwchar_toupper}
\done

\function{SLutf8_strlo}
\synopsis{Lowercase a UTF-8 encoded string}
\usage{SLuchar_Type *SLutf8_strlo (SLuchar_Type *u, SLuchar_Type *umax)}
\description
 The \cfun{SLutf8_strlo} function returns the lowercase equivalent of
 UTF-8 encoded sequence of \exmp{umax-u} bytes at \exmp{u}.  The
 result will be returned as a null-terminated \exmp{SLstring} and
 should be freed with \cfun{SLang_free_slstring} when it is nolonger
 needed.  If the function encounters an invalid of illegal byte
 sequence, then the byte-sequence will be copied as as-is.
\seealso{SLutf8_strlow, SLwchar_toupper}
\done

\function{SLutf8_subst_wchar}
\synopsis{Replace a character in a UTF-8 encoded string}
\usage{SLstr_Type *SLutf8_subst_wchar (u, umax, wch, nth,ignore_combining)}
#v+
   SLuchar_Type *u, *umax;
   SLwchar_Type wch;
   unsigned int nth;
   int ignore_combining;
#v-
\description
 The \cfun{SLutf8_subst_wchar} function replaces the UTF-8 sequence
 representing the \exmp{nth} character of \exmp{u} by the UTF-8
 representation of the character \exmp{wch}.  If the value of the
 \exmp{ignore_combining} parameter is non-zero, then combining
 characters will not be counted when computing the position of the
 \exmp{nth} character.  In addition, if the \exmp{nth} character
 contains any combining characters, then the byte-sequence associated
 with those characters will also be replaced.

 Since the byte sequence representing \exmp{wch} could be longer than
 the sequence of the \exmp{nth} character, the function returns a new
 copy of the resulting string as an \exmp{SLSTRING}.  Hence, the
 calling function should call \cfun{SLang_free_slstring} when the
 result is nolonger needed.
\seealso{SLutf8_strup, SLutf8_strlow, SLutf8_skip_chars, SLutf8_strlen}
\done

\function{SLutf8_compare}
\synopsis{Compare two UTF-8 encoded sequences}
\usage{int SLutf8_compare (a, amax, b, bmax, nchars, case_sensitive)}
#v+
   SLuchar_Type *a, *amax;
   SLuchar_Type *b, *bmax;
   unsigned int nchars;
   int case_sensitive;
#v-
\description
 This function compares \exmp{nchars} of one UTF-8 encoded character
 sequence to another by performing a character by character comparison.
 The function returns 0, +1, or -1 according to whether the string
 \exmp{a} is is equal to, greater than, or less than the string at
 \exmp{b}.  At most \exmp{nchars} characters will be tested.  The
 parameters \exmp{amax} and \exmp{bmax} serve as upper boundaries of
 the strings \exmp{a} and \exmp{b}, resp.

 If the value of the \exmp{case_sensitive} parameter is non-zero, then
 a case-sensitive comparison will be performed, otherwise characters
 will be compared in a case-insensitive manner.
\notes
 For case-sensitive comparisons, this function is analogous to the
 standard C library's \cfun{strncmp} function.  However,
 \ifun{SLutf8_compare} can also cope with invalid or illegal UTF-8
 sequences.
\seealso{SLutf8_strup, SLutf8_strlen, SLutf8_strlen}
\done

#% \function{SLutf8_decode_bytes}
#% \synopsis{}
#% \usage{int SLutf8_decode_bytes (u, umax, b, np)}
#% #v+
#%   SLuchar_Type *u, *umax;
#%   unsigned char *b;
#%   unsigned int *np;
#% #v-
#% \description
#% \seealso{}
#% \done

#% \function{SLutf8_encode_bytes}
#% \synopsis{UTF-8 encode an byte-sequence}
#% \usage{SLuchar_Type *SLutf8_encode_bytes (b, bmax, u, ulen, np)}
#% #v+
#%    unsigned char *b, *bmax;
#%    SLuchar_Type *u;
#%    unsigned int ulen;
#%    unsigned int *np;
#% #v-
#% \description
#%   The \cfun{SLutf8_encode_bytes} function UTF-8 encodes each byte
#%   between \exmp{b} and \exmp{bmax} and placing the \exmp{bmax-b}
#%   encoded characters into the buffer at \exmp{u}, whose length is
#%   given by \exmp{ulen}.  Upon return, \exmp{*np} will be set to the
#%   number of bytes sucessfully encoded.  The number will be less than
#%   the number requested if the buffer at \exmp{u} is too small.
#% \notes
#%   This function interprets the value of each byte as a wide-character
#%   to be encoded.  As such, the function can be used to UTF-8 encode
#%   characters from an iso-latin-1 character set.
#% \seealso{SLutf8_decode_bytes}
#% \done

