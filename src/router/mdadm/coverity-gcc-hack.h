#if !defined(__KERNEL__) && defined(__x86_64__) && defined(__COVERITY_GCC_VERSION_AT_LEAST)
#if __COVERITY_GCC_VERSION_AT_LEAST(7, 0)
typedef float _Float128 __attribute__((__vector_size__(128)));
typedef float _Float64 __attribute__((__vector_size__(64)));
typedef float _Float32 __attribute__((__vector_size__(32)));
typedef float _Float128x __attribute__((__vector_size__(128)));
typedef float _Float64x __attribute__((__vector_size__(64)));
typedef float _Float32x __attribute__((__vector_size__(32)));
#endif
#endif
