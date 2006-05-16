#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

#ifndef YYSTYPE
typedef union {
	int			num;
	int			snum;
	double			dec;
	int			bool;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	T_INTERFACE	257
# define	T_PREFIX	258
# define	STRING	259
# define	NUMBER	260
# define	SIGNEDNUMBER	261
# define	DECIMAL	262
# define	SWITCH	263
# define	IPV6ADDR	264
# define	INFINITY	265
# define	T_AdvSendAdvert	266
# define	T_MaxRtrAdvInterval	267
# define	T_MinRtrAdvInterval	268
# define	T_AdvManagedFlag	269
# define	T_AdvOtherConfigFlag	270
# define	T_AdvLinkMTU	271
# define	T_AdvReachableTime	272
# define	T_AdvRetransTimer	273
# define	T_AdvCurHopLimit	274
# define	T_AdvDefaultLifetime	275
# define	T_AdvSourceLLAddress	276
# define	T_AdvOnLink	277
# define	T_AdvAutonomous	278
# define	T_AdvValidLifetime	279
# define	T_AdvPreferredLifetime	280
# define	T_AdvRouterAddr	281
# define	T_AdvHomeAgentFlag	282
# define	T_AdvIntervalOpt	283
# define	T_AdvHomeAgentInfo	284
# define	T_Base6to4Interface	285
# define	T_UnicastOnly	286
# define	T_HomeAgentPreference	287
# define	T_HomeAgentLifetime	288
# define	T_BAD_TOKEN	289


extern YYSTYPE yylval;

#endif /* not BISON_Y_TAB_H */
