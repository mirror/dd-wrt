#ifndef EX_HTTPD_ERR_CODE_H
#define EX_HTTPD_ERR_CODE_H

#define CONF_MSG_RET_BEG "\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\r\n\
<html>\r\n\
  <head>\r\n\
"

#define CONF_MSG_RET_END "\
  <hr>\r\n\
  Generated using the <a href=\"http://www.and.org/vstr/httpd\">Vstr httpd example Web Server</a>.\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_MSG__MAKE(code, sum, msg) \
    CONF_MSG_RET_BEG \
"    <title>" code " " sum "</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>" code " " sum "</h1>\r\n\
    <p>" msg ".</p>\r\n" \
    CONF_MSG_RET_END

#define CONF_MSG__FMT_30x_END "\
\">here</a>.</p>\r\n" \
    CONF_MSG_RET_END

#define CONF_LINE_RET_301 "Moved Permanently"
#define CONF_MSG_FMT_301 "%s${vstr:%p%zu%zu%u}%s"
#define CONF_MSG__FMT_301_BEG CONF_MSG_RET_BEG "\
    <title>301 Moved Permanently</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>301 Moved Permanently</h1>\r\n\
    <p>The document has moved <a href=\"\
"
#define CONF_MSG__FMT_301_END CONF_MSG__FMT_30x_END

#define CONF_MSG_LEN_301(s1) (((s1)->len) +                             \
                              strlen(CONF_MSG__FMT_301_BEG) +           \
                              strlen(CONF_MSG__FMT_301_END))

#define CONF_LINE_RET_302 "Found"
#define CONF_MSG_FMT_302 "%s${vstr:%p%zu%zu%u}%s"
#define CONF_MSG__FMT_302_BEG CONF_MSG_RET_BEG "\
    <title>302 Found</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>302 Found</h1>\r\n\
    <p>The document can be found <a href=\"\
"
#define CONF_MSG__FMT_302_END CONF_MSG__FMT_30x_END

#define CONF_MSG_LEN_302(s1) (((s1)->len) +                             \
                              strlen(CONF_MSG__FMT_302_BEG) +           \
                              strlen(CONF_MSG__FMT_302_END))

#define CONF_LINE_RET_303 "See Other"
#define CONF_MSG_FMT_303 "%s${vstr:%p%zu%zu%u}%s"
#define CONF_MSG__FMT_303_BEG CONF_MSG_RET_BEG "\
    <title>303 See Other</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>303 See Other</h1>\r\n\
    <p>The document should be seen <a href=\"\
"
#define CONF_MSG__FMT_303_END CONF_MSG__FMT_30x_END

#define CONF_MSG_LEN_303(s1) (((s1)->len) +                             \
                              strlen(CONF_MSG__FMT_303_BEG) +           \
                              strlen(CONF_MSG__FMT_303_END))

#define CONF_LINE_RET_307 "Temporary Redirect"
#define CONF_MSG_FMT_307 "%s${vstr:%p%zu%zu%u}%s"
#define CONF_MSG__FMT_307_BEG CONF_MSG_RET_BEG "\
    <title>307 Temporary Redirect</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>307 Temporary Redirect</h1>\r\n\
    <p>The document has temporarily moved <a href=\"\
"
#define CONF_MSG__FMT_307_END CONF_MSG__FMT_30x_END

#define CONF_MSG_LEN_307(s1) (((s1)->len) +                             \
                              strlen(CONF_MSG__FMT_307_BEG) +           \
                              strlen(CONF_MSG__FMT_307_END))

#define CONF_LINE_RET_400 "Bad Request"
#define CONF_MSG_RET_400 \
    CONF_MSG__MAKE("400", "Bad Request", "The request could not be understood")

#define CONF_LINE_RET_401 "Unauthorized"
#define CONF_MSG_RET_401 \
    CONF_MSG__MAKE("401", "Unauthorized", "The request requires user authentication")

#define CONF_LINE_RET_403 "Forbidden"
#define CONF_MSG_RET_403 \
    CONF_MSG__MAKE("403", "Forbidden", "The request is forbidden")

#define CONF_LINE_RET_404 "Not Found"
#define CONF_MSG_RET_404 \
    CONF_MSG__MAKE("404", "Not Found", "The document requested was not found")

#define CONF_LINE_RET_405 "Method Not Allowed"
#define CONF_MSG_RET_405 \
    CONF_MSG__MAKE("405", "Method Not Allowed", "The method specified is not allowed")

#define CONF_LINE_RET_406 "Not Acceptable"
#define CONF_MSG_RET_406 \
    CONF_MSG__MAKE("406", "Not Acceptable", "The resource identified by the request is only capable of generating response entities which have content characteristics not acceptable according to the accept headers sent in the request")

#define CONF_LINE_RET_410 "Gone"
#define CONF_MSG_RET_410 \
    CONF_MSG__MAKE("410", "Gone", "The requested resource is no longer available at the server and no forwarding address is known. This condition is expected to be considered permanent")

#define CONF_LINE_RET_412 "Precondition Failed"
#define CONF_MSG_RET_412 \
    CONF_MSG__MAKE("412", "Precondition Failed", "The precondition given in one or more of the request-header fields evaluated to false")

#define CONF_LINE_RET_413 "Request Entity Too Large"
#define CONF_MSG_RET_413 \
    CONF_MSG__MAKE("413", "Request Entity Too Large", "The server does not accept any requests with Content (In other words if either a Content-Length or a Transfer-Encoding header is passed to the server, the request will fail)")

#define CONF_LINE_RET_414 "Request-URI Too Long"
#define CONF_MSG_RET_414 \
    CONF_MSG__MAKE("414", "Request-URI Too Long", "The document request was too long")

/*    CONF_MSG__MAKE("415", "", "") */
#define CONF_LINE_RET_415 "Unsupported Media Type"
#define CONF_MSG_RET_415 \
    CONF_MSG__MAKE("415", "Unsupported Media Type", "The server is refusing to service the request because the result of the request is in a format not supported, by the request")

#define CONF_LINE_RET_416 "Requested Range Not Satisfiable"
#define CONF_MSG_RET_416 \
    CONF_MSG__MAKE("416", "Requested Range Not Satisfiable", "The document request range was not valid")

#define CONF_LINE_RET_417 "Expectation Failed"
#define CONF_MSG_RET_417 \
    CONF_MSG__MAKE("417", "Expectation Failed", "The expectation given in an Expect request-header field could not be met by this server")

#define CONF_LINE_RET_500 "Internal Server Error"
#define CONF_MSG_RET_500 \
    CONF_MSG__MAKE("500", "Internal Server Error", "The server encountered something unexpected")

#define CONF_LINE_RET_501 "Not Implemented"
#define CONF_MSG_RET_501 \
    CONF_MSG__MAKE("501", "Not Implemented", "The request method is not implemented")

#define CONF_LINE_RET_503 "Service Unavailable"
#define CONF_MSG_RET_503 \
    CONF_MSG__MAKE("503", "Service Unavailable", "The server is currently unable to handle the request due to a temporary overloading or maintenance of the server")

#define CONF_LINE_RET_505 "Version not supported"
#define CONF_MSG_RET_505 \
    CONF_MSG__MAKE("505", "Version not supported", "The version of http used is not supported")

#define HTTPD_ERR_301(req) do {                           \
      (req)->error_code = 301;                            \
      (req)->error_line = CONF_LINE_RET_301;              \
      (req)->error_len  = CONF_MSG_LEN_301((req)->fname); \
    } while (0)

#define HTTPD_ERR_302(req) do {                           \
      (req)->error_code = 302;                            \
      (req)->error_line = CONF_LINE_RET_302;              \
      (req)->error_len  = CONF_MSG_LEN_302((req)->fname); \
    } while (0)

#define HTTPD_ERR_303(req) do {                           \
      (req)->error_code = 303;                            \
      (req)->error_line = CONF_LINE_RET_303;              \
      (req)->error_len  = CONF_MSG_LEN_303((req)->fname); \
    } while (0)

#define HTTPD_ERR_307(req) do {                           \
      (req)->error_code = 307;                            \
      (req)->error_line = CONF_LINE_RET_307;              \
      (req)->error_len  = CONF_MSG_LEN_307((req)->fname); \
    } while (0)

#define HTTPD_ERR(req, code) do {                           \
      (req)->error_code  = (code);                          \
      (req)->error_line  = CONF_LINE_RET_ ## code ;         \
      (req)->error_len   = strlen( CONF_MSG_RET_ ## code ); \
      if (!(req)->head_op)                                  \
        (req)->error_msg = CONF_MSG_RET_ ## code ;          \
    } while (0)

#define HTTPD_ERR_RET(req, code, val) do {              \
      HTTPD_ERR(req, code);                             \
      return val ;                                      \
    } while (0)

#define HTTPD_ERR_GOTO(req, code, label) do {           \
      HTTPD_ERR(req, code);                             \
      goto label ;                                      \
    } while (0)
      
#endif
