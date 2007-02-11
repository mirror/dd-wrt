#include <EXTERN.h>
#include "perl.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

static PerlInterpreter *my_perl;

void            boot_DynaLoader(CV * cv);

void
xs_init(void)
{
    char            myfile[] = __FILE__;
    char            modulename[] = "DynaLoader::boot_DynaLoader";
    /*
     * DynaLoader is a special case 
     */
    newXS(modulename, boot_DynaLoader, myfile);
}

void
maybe_source_perl_startup(void)
{
    const char     *embedargs[] = { "", "" };
    const char     *perl_init_file = netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID,
							   NETSNMP_DS_AGENT_PERL_INIT_FILE);
    char            init_file[SNMP_MAXBUF];

    static int      have_done_init = 0;

    if (have_done_init)
        return;
    have_done_init = 1;

    if (!perl_init_file) {
        snprintf(init_file, sizeof(init_file) - 1,
                 "%s/%s", SNMPSHAREPATH, "snmp_perl.pl");
        perl_init_file = init_file;
    }
    embedargs[1] = perl_init_file;

    DEBUGMSGTL(("perl", "initializing perl (%s)\n", embedargs[1]));
    my_perl = perl_alloc();
    if (!my_perl)
        goto bail_out;

    perl_construct(my_perl);
    if (perl_parse(my_perl, xs_init, 2, (char **) embedargs, NULL))
        goto bail_out;

    if (perl_run(my_perl))
        goto bail_out;

    DEBUGMSGTL(("perl", "done initializing perl\n"));

    return;

  bail_out:
    snmp_log(LOG_ERR, "embedded perl support failed to initalize\n");
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
			   NETSNMP_DS_AGENT_DISABLE_PERL, 1);
    return;
}

void
do_something_perlish(char *something)
{
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_DISABLE_PERL)) {
        return;
    }
    maybe_source_perl_startup();
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_DISABLE_PERL)) {
        return;
    }
    DEBUGMSGTL(("perl", "calling perl\n"));
#if defined(HAVE_EVAL_PV) || defined(eval_pv)
    /* newer perl */
    eval_pv(something, TRUE);
#else
#if defined(HAVE_PERL_EVAL_PV_LC) || defined(perl_eval_pv)
    /* older perl? */
    perl_eval_pv(something, TRUE);
#else /* HAVE_PERL_EVAL_PV_LC */
#ifdef HAVE_PERL_EVAL_PV_UC
    /* older perl? */
    Perl_eval_pv(my_perl, something, TRUE);
#else /* !HAVE_PERL_EVAL_PV_UC */
#error embedded perl broken 
#endif /* !HAVE_PERL_EVAL_PV_LC */
#endif /* !HAVE_PERL_EVAL_PV_UC */
#endif /* !HAVE_EVAL_PV */
    DEBUGMSGTL(("perl", "finished calling perl\n"));
}

void
perl_config_handler(const char *token, char *line)
{
    do_something_perlish(line);
}

void
init_perl(void)
{
    const char     *appid = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
						  NETSNMP_DS_LIB_APPTYPE);
    const char     *defaultid = "snmpd";

    if (!appid) {
        appid = defaultid;
    }

    /*
     * register config handlers 
     */
    snmpd_register_config_handler("perl", perl_config_handler, NULL,
                                  "PERLCODE");

    /*
     * define the perlInitFile token to point to an init file 
     */
    netsnmp_ds_register_premib(ASN_OCTET_STR, appid, "perlInitFile",
			       NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_PERL_INIT_FILE);

    /*
     * define the perlInitFile token to point to an init file 
     */
    netsnmp_ds_register_premib(ASN_BOOLEAN, appid, "disablePerl",
			       NETSNMP_DS_APPLICATION_ID,
			       NETSNMP_DS_AGENT_DISABLE_PERL);
}

void
shutdown_perl(void)
{
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_DISABLE_PERL)) {
        return;
    }
    DEBUGMSGTL(("perl", "shutting down perl\n"));
    perl_destruct(my_perl);
    perl_free(my_perl);
    DEBUGMSGTL(("perl", "finished shutting down perl\n"));
}
