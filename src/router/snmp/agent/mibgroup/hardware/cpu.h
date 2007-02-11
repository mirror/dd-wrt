config_require(hardware/cpu/cpu)

#if defined(linux)
config_require(hardware/cpu/cpu_linux)

#elif defined(darwin)
/* XXX: note this is a duplicate case (see darwin in net/open bsd below */
/* XXX: we need to come up with working code for OSX */
config_require(hardware/cpu/cpu_null)

#elif (defined(netbsd) || defined(netbsd1) || defined(netbsdelf) || defined(netbsdelf2)|| defined(netbsdelf3) || defined(openbsd2)|| defined(openbsd3) || defined(openbsd4) || defined(darwin))
config_require(hardware/cpu/cpu_sysctl)

#elif (defined(freebsd2) || defined(freebsd3) || defined(freebsd4)  || defined(freebsd5)|| defined(freebsd6))
config_require(hardware/cpu/cpu_nlist)

#elif (defined(aix4) || defined(aix5))
config_require(hardware/cpu/cpu_perfstat)

#elif (defined(solaris2))
config_require(hardware/cpu/cpu_kstat)

#elif (defined(hpux10) || defined(hpux11))
config_require(hardware/cpu/cpu_pstat)

#else
config_require(hardware/cpu/cpu_null)
#endif
