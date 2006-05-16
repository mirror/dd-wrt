/*
This is a libary collection
*/

#include <stdio.h>
#include <string>
#include "string"


/*
Parsing section 
*/
#ifndef __STRUTILS_H
#define __STRUTILS_H


#ifdef __cplusplus
extern "C++" {
#endif /* __cplusplus */



/* Returns a std::string before the delimiter del */
// const std::string head(const std::string &str, const std::string &del);
const std::string headc(const std::string &str, const char &c);
/* Returns a std::string after the delimiter del */
// const std::string tail(const std::string &str, const std::string &del);


/* Returns a std::string before the last delimiter del */
// const std::string headr(const std::string &str, const std::string &del);
/* Returns a std::string after the last delimiter del */
// const std::string tailr(const std::string &str, const std::string &del);



/* Returns a std::string after removing the white space at the start and end */
const std::string trim(const std::string &str);
/* Returns a std::string after removing the white space at the start */
const std::string trim_start(const std::string &str);
/* Returns a std::string after removing the white space at the end */
const std::string trim_end(const std::string &str);


/* Return true if std::string ends with test value */
bool ends_with(const std::string &str, const std::string &end);


/*Returns a std::string of str converted to uppercase*/
const std::string case_upper(const std::string &str);
/*Returns a std::string of str converted to lowercase*/
const std::string case_lower(const std::string &str);


// const std::string format(char *fmt, ...);

// const std::string url_encode(const std::string &src);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __STRUTILS_H */
