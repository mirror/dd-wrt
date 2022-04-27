#ifdef SINGLE_MODULE

#undef static
#define static
#undef module_init
#undef module_exit
#define module_init(...)
#define module_exit(...)

#undef MODULE_AUTHOR
#undef MODULE_LICENSE
#undef MODULE_VERSION
#undef MODULE_DESCRIPTION
#define MODULE_AUTHOR(...)
#define MODULE_LICENSE(...)
#define MODULE_VERSION(...)
#define MODULE_DESCRIPTION(...)

#endif
