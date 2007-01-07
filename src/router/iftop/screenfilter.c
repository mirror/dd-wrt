/*
 * screenfilter.c:
 *
 * Copyright (c) 2002 DecisionSoft Ltd.
 * Paul Warren (pdw) Fri Oct 25 10:21:00 2002
 *
 */

#include "config.h"

#ifdef HAVE_REGCOMP

#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include "iftop.h"
#include "options.h"

static const char rcsid[] = "$Id: screenfilter.c,v 1.3 2002/11/04 12:27:35 chris Exp $";

extern options_t options ;

regex_t preg;

int screen_filter_set(char* s) {
    int r;

    if(options.screenfilter != NULL) {
        xfree(options.screenfilter);
        options.screenfilter = NULL;
        regfree(&preg);
    }

    r = regcomp(&preg, s, REG_ICASE|REG_EXTENDED);
      
    if(r == 0) {
        options.screenfilter = s;
        return 1;
    }
    else {
        xfree(s);
        return 0;
    }
}

int screen_filter_match(char *s) {
    int r;
    if(options.screenfilter == NULL) {
        return 1;
    }

    r = regexec(&preg, s, 0, NULL, 0);
    if(r == 0) {
        return 1;
    }
    else {
        return 0;
    }
}

#endif /* HAVE_REGCOMP */
