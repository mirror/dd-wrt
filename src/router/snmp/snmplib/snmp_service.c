#include <net-snmp/net-snmp-config.h>

#include <stdlib.h>
#include <string.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/snmp_transport.h>

struct netsnmp_lookup_domain {
    char* application;
    char* userDomain;
    char* domain;
    struct netsnmp_lookup_domain* next;
};

static struct netsnmp_lookup_domain* domains = NULL;

int
netsnmp_register_default_domain(const char* application, const char* domain)
{
    struct netsnmp_lookup_domain *run = domains, *prev = NULL;
    int res = 0;

    while (run != NULL && strcmp(run->application, application) < 0) {
	prev = run;
	run = run->next;
    }
    if (run && strcmp(run->application, application) == 0) {
      if (run->domain != NULL) {
	  free (run->domain);
	  res = 1;
      }
    } else {
	run = malloc(sizeof(struct netsnmp_lookup_domain));
	run->application = strdup(application);
	run->userDomain = NULL;
	if (prev) {
	    run->next = prev->next;
	    prev->next = run;
	} else {
	    run->next = domains;
	    domains = run;
	}
    }
    if (domain) {
	run->domain = strdup(domain);
    } else if (run->userDomain == NULL) {
	if (prev)
	    prev->next = run->next;
	else
	    domains = run->next;
	free(run->application);
	free(run);
    }
    return res;
}

void
netsnmp_clear_default_domain(void)
{
    while (domains) {
	struct netsnmp_lookup_domain *tmp = domains;
	domains = domains->next;
	free(tmp->application);
	free(tmp->userDomain);
	free(tmp->domain);
	free(tmp);
    }
}

static void
netsnmp_register_user_domain(const char* token, char* cptr)
{
    struct netsnmp_lookup_domain *run = domains, *prev = NULL;
    size_t len = strlen(cptr) + 1;
    char* application = malloc(len);
    char* domain = malloc(len);

    {
	char* cp = copy_nword(cptr, application, len);
	cp = copy_nword(cp, domain, len);
	if (cp)
	    config_pwarn("Trailing junk found");
    }

    while (run != NULL && strcmp(run->application, application) < 0) {
	prev = run;
	run = run->next;
    }
    if (run && strcmp(run->application, application) == 0) {
	if (run->userDomain != NULL) {
	    config_perror("Default transport already registered for this "
			  "application");
	    goto done;
	}
    } else {
	run = malloc(sizeof(struct netsnmp_lookup_domain));
	run->application = strdup(application);
	run->domain = NULL;
	if (prev) {
	    run->next = prev->next;
	    prev->next = run;
	} else {
	    run->next = domains;
	    domains = run;
	}
    }
    run->userDomain = strdup(domain);
 done:
    free(domain);
    free(application);
}

static void
netsnmp_clear_user_domain(void)
{
    struct netsnmp_lookup_domain *run = domains, *prev = NULL;

    while (run) {
	if (run->userDomain != NULL) {
	    free(run->userDomain);
	    run->userDomain = NULL;
	}
	if (run->domain == NULL) {
	    struct netsnmp_lookup_domain *tmp = run;
	    if (prev)
		run = prev->next = run->next;
	    else
		run = domains = run->next;
	    free(tmp->application);
	    free(tmp);
	} else {
	    prev = run;
	    run = run->next;
	}
    }
}

const char*
netsnmp_lookup_default_domain(const char* application)
{
    const char *res;

    if (application == NULL)
	res = NULL;
    else {
	struct netsnmp_lookup_domain *run = domains;

	while (run && strcmp(run->application, application) < 0)
	    run = run->next;
	if (run && strcmp(run->application, application) == 0)
	    if (run->userDomain)
		res = run->userDomain;
	    else
		res = run->domain;
	else
	    res = NULL;
    }
    DEBUGMSGTL(("defaults",
		"netsnmp_lookup_default_domain(\"%s\") -> \"%s\"\n",
		application ? application : "[NIL]",
		res ? res : "[NIL]"));
    return res;
}

struct netsnmp_lookup_target {
    char* application;
    char* domain;
    char* userTarget;
    char* target;
    struct netsnmp_lookup_target* next;
};

static struct netsnmp_lookup_target* targets = NULL;

int
netsnmp_register_default_target(const char* application, const char* domain,
				const char* target)
{
    struct netsnmp_lookup_target *run = targets, *prev = NULL;
    int i, res = 0;
    while (run && ((i = strcmp(run->application, application)) < 0 ||
		   (i == 0 && strcmp(run->domain, domain) < 0))) {
	prev = run;
	run = run->next;
    }
    if (run && i == 0 && strcmp(run->domain, domain) == 0) {
      if (run->target != NULL) {
	    free(run->target);
	    res = 1;
      }
    } else {
	run = malloc(sizeof(struct netsnmp_lookup_target));
	run->application = strdup(application);
	run->domain = strdup(domain);
	run->userTarget = NULL;
	if (prev) {
	    run->next = prev->next;
	    prev->next = run;
	} else {
	    run->next = targets;
	    targets = run;
	}
    }
    if (target) {
	run->target = strdup(target);
    } else if (run->userTarget == NULL) {
	if (prev)
	    prev->next = run->next;
	else
	    targets = run->next;
	free(run->domain);
	free(run->application);
	free(run);
    }
    return res;
}

void
netsnmp_clear_default_target(void)
{
    while (targets) {
	struct netsnmp_lookup_target *tmp = targets;
	targets = targets->next;
	free(tmp->application);
	free(tmp->domain);
	free(tmp->userTarget);
	free(tmp->target);
	free(tmp);
    }
}

static void
netsnmp_register_user_target(const char* token, char* cptr)
{
    struct netsnmp_lookup_target *run = targets, *prev = NULL;
    size_t len = strlen(cptr) + 1;
    char* application = malloc(len);
    char* domain = malloc(len);
    char* target = malloc(len);
    int i;

    {
	char* cp = copy_nword(cptr, application, len);
	cp = copy_nword(cp, domain, len);
	cp = copy_nword(cp, target, len);
	if (cp)
	    config_pwarn("Trailing junk found");
    }

    while (run && ((i = strcmp(run->application, application)) < 0 ||
		   (i == 0 && strcmp(run->domain, domain) < 0))) {
	prev = run;
	run = run->next;
    }
    if (run && i == 0 && strcmp(run->domain, domain) == 0) {
	if (run->userTarget != NULL) {
	    config_perror("Default target already registered for this "
			  "application-domain combination");
	    goto done;
	}
    } else {
	run = malloc(sizeof(struct netsnmp_lookup_target));
	run->application = strdup(application);
	run->domain = strdup(domain);
	run->target = NULL;
	if (prev) {
	    run->next = prev->next;
	    prev->next = run;
	} else {
	    run->next = targets;
	    targets = run;
	}
    }
    run->userTarget = strdup(target);
 done:
    free(target);
    free(domain);
    free(application);
}

static void
netsnmp_clear_user_target(void)
{
    struct netsnmp_lookup_target *run = targets, *prev = NULL;

    while (run) {
	if (run->userTarget != NULL) {
	    free(run->userTarget);
	    run->userTarget = NULL;
	}
	if (run->target == NULL) {
	    struct netsnmp_lookup_target *tmp = run;
	    if (prev)
		run = prev->next = run->next;
	    else
		run = targets = run->next;
	    free(tmp->application);
	    free(tmp->domain);
	    free(tmp);
	} else {
	    prev = run;
	    run = run->next;
	}
    }
}

const char*
netsnmp_lookup_default_target(const char* application, const char* domain)
{
    int i;
    struct netsnmp_lookup_target *run = targets;
    const char *res;

    if (application == NULL || domain == NULL)
	res = NULL;
    else {
	while (run && ((i = strcmp(run->application, application)) < 0 ||
		       (i == 0 && strcmp(run->domain, domain) < 0)))
	    run = run->next;
	if (run && i == 0 && strcmp(run->domain, domain) == 0)
	    if (run->userTarget != NULL)
		res = run->userTarget;
	    else
		res = run->target;
	else
	    res = NULL;
    }
    DEBUGMSGTL(("defaults",
		"netsnmp_lookup_default_target(\"%s\", \"%s\") -> \"%s\"\n",
		application ? application : "[NIL]",
		domain ? domain : "[NIL]",
		res ? res : "[NIL]"));
    return res;
}

void
netsnmp_register_service_handlers(void)
{
    register_config_handler(NULL, "defDomain",
			    netsnmp_register_user_domain,
			    netsnmp_clear_user_domain,
			    "application domain");
    register_config_handler(NULL, "defTarget",
			    netsnmp_register_user_target,
			    netsnmp_clear_user_target,
			    "application domain target");
}
