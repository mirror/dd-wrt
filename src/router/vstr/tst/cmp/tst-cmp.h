
#define TEST_CMP_STR(x) #x
#define TEST_CMP_XSTR(x) TEST_CMP_STR(x)

#define TEST_CMP_BEG(x, y) do { \
 Vstr_base *x_str = NULL; \
 Vstr_base *y_str = NULL; \
 x_str = VSTR_DUP_CSTR_BUF(NULL, (x)); \
 if (!x_str) die(); \
 y_str = VSTR_DUP_CSTR_PTR(NULL, (y)); \
 if (!y_str) die()

#define TEST_CMP_END() \
 vstr_free_base(x_str); \
 vstr_free_base(y_str); \
 } while (FALSE)

#define TEST_CMP_EQ_0(x, y) \
 TEST_CMP_BEG(x, y); \
 ++count; \
 TST_B_TST(ret, count, \
    ((TEST_CMP_VSTR_FUNC (x_str, 1, x_str->len, y_str, 1, y_str->len) != 0) || \
     (TEST_CMP_CSTR_FUNC (x_str, 1, x_str->len, y)                    != 0) || \
     (TEST_CMP_CSTR_FUNC (y_str, 1, y_str->len, x)                    != 0) || \
     FALSE)); \
 VSTR_ADD_CSTR_PTR(x_str, 0, "123456789 123456789 123456789 123456789 "); \
 TST_B_TST(ret, count, \
    ((TEST_CMP_VSTR_FUNC (x_str, 41,y_str->len, y_str, 1, y_str->len) != 0) || \
     (TEST_CMP_CSTR_FUNC (x_str, 41, x_str->len - 40, y)              != 0) || \
     FALSE)); \
 TEST_CMP_END()

#define TEST_CMP_GT_0(x, y) \
 TEST_CMP_BEG(x, y); \
 ++count; \
 TST_B_TST(ret, count, \
    ((TEST_CMP_VSTR_FUNC (x_str, 1, x_str->len, y_str, 1, y_str->len) <= 0) || \
     (TEST_CMP_CSTR_FUNC (x_str, 1, x_str->len, y)                    <= 0) || \
     (TEST_CMP_CSTR_FUNC (y_str, 1, y_str->len, x)                    >= 0) || \
     FALSE)); \
 VSTR_ADD_CSTR_PTR(x_str, 0, "123456789 123456789 123456789 123456789 "); \
 TST_B_TST(ret, count, \
    ((TEST_CMP_VSTR_FUNC (x_str, 41,x_str->len-40,y_str,1,y_str->len) <= 0) || \
     (TEST_CMP_CSTR_FUNC (x_str, 41,x_str->len-40,y)                  <= 0) || \
     FALSE)); \
 TEST_CMP_END(); \
 TEST_CMP_BEG(y, x); \
 TST_B_TST(ret, count, \
    ((TEST_CMP_VSTR_FUNC (x_str, 1, x_str->len, y_str, 1, y_str->len) >= 0) || \
     (TEST_CMP_CSTR_FUNC (x_str, 1, x_str->len, x)                    >= 0) || \
     (TEST_CMP_CSTR_FUNC (y_str, 1, y_str->len, y)                    <= 0) || \
     FALSE)); \
 VSTR_ADD_CSTR_PTR(x_str, 0, "123456789 123456789 123456789 123456789 "); \
 TST_B_TST(ret, count, \
    ((TEST_CMP_VSTR_FUNC (x_str, 41,x_str->len-40,y_str,1,y_str->len) >= 0) || \
     (TEST_CMP_CSTR_FUNC (x_str, 41,x_str->len-40,x)                  >= 0) || \
     FALSE)); \
 TEST_CMP_END()

