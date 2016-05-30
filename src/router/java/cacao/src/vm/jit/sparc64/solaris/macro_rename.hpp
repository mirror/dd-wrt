
/* work around name clash in Solaris */
#if (REG_SP != 14)
#warning the bastard is defined
# define REG_SP_SOLARIS REG_SP
# undef REG_SP
# define REG_SP 14
#endif

