/* $Id$ */
/* Snort Preprocessor Plugin Source File Template */

/* spp_template
 *
 * Purpose:
 *
 * Preprocessors perform some function *once* for *each* packet.  This is
 * different from detection plugins, which are accessed depending on the
 * standard rules.  When adding a plugin to the system, be sure to
 * add the "Setup" function to the InitPreprocessors() function call in
 * plugbase.c!
 *
 * Arguments:
 *
 * This is the list of arguments that the plugin can take at the
 * "preprocessor" line in the rules file
 *
 * Effect:
 *
 * What the preprocessor does.  Check out some of the default ones
 * (e.g. spp_frag2) for a good example of this description.
 *
 * Comments:
 *
 * Any comments?
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <rpc/types.h>

/*
 * If you're going to issue any alerts from this preproc you
 * should include generators.h and event_wrapper.h
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "generators.h"
#include "event_wrapper.h"

#include "util.h"
#include "plugbase.h"
#include "parser.h"

/*
 * put in other inculdes as necessary
 */

/*
 * your preprocessor header file goes here if necessary, don't forget
 * to include the header file in plugbase.h too!
 */
#include "spp_template.h"

/*
 * define any needed data structs for things like configuration
 */
typedef struct _TemplateData
{
    /* Your struct members here */
} TemplateData;

/*
 * If you need to instantiate the preprocessor's
 * data structure, do it here
 */
TemplateData SomeData;

/*
 * function prototypes go here
 */

static void TemplateInit(u_char *);
static void ParseTemplateArgs(char *);
static void PreprocFunction(Packet *);
static void PreprocCleanExitFunction(int, void *);
static void PreprocRestartFunction(int, void *);

/*
 * Function: SetupTemplate()
 *
 * Purpose: Registers the preprocessor keyword and initialization
 *          function into the preprocessor list.  This is the function that
 *          gets called from InitPreprocessors() in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void SetupTemplate()
{
    /*
     * link the preprocessor keyword to the init function in
     * the preproc list
     */
    RegisterPreprocessor("keyword", TemplateInit);

    DebugMessage(DEBUG_PLUGIN,"Preprocessor: Template is setup...\n");
}


/*
 * Function: TemplateInit(u_char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void TemplateInit(u_char *args)
{
    DebugMessage(DEBUG_PLUGIN,"Preprocessor: Template Initialized\n");

    /*
     * parse the argument list from the rules file
     */
    ParseTemplateArgs(args);

    /*
     * perform any other initialization functions that are required here
     */

    /*
     * Set the preprocessor function into the function list
     */
    AddFuncToPreprocList(PreprocFunction);
    AddFuncToCleanExitList(PreprocCleanExitFunction, NULL);
    AddFuncToRestartList(PreprocRestartFunction, NULL);
}



/*
 * Function: ParseTemplateArgs(char *)
 *
 * Purpose: Process the preprocessor arguments from the rules file and
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init
 *          function.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
static void ParseTemplateArgs(char *args)
{
    /* your parsing function goes here, check out the other spp files
       for examples */
}


/*
 * Function: PreprocFunction(Packet *)
 *
 * Purpose: Perform the preprocessor's intended function.  This can be
 *          simple (statistics collection) or complex (IP defragmentation)
 *          as you like.  Try not to destroy the performance of the whole
 *          system by trying to do too much....
 *
 * Arguments: p => pointer to the current packet data struct
 *
 * Returns: void function
 *
 */
static void PreprocFunction(Packet *p)
{

    /* your preproc function goes here.... */

    /*
     * if you need to issue an alert from your preprocessor, check out
     * event_wrapper.h, there are some useful helper functions there
     */
}


/*
 * Function: PreprocCleanExitFunction(int, void *)
 *
 * Purpose: This function gets called when Snort is exiting, if there's
 *          any cleanup that needs to be performed (e.g. closing files)
 *          it should be done here.
 *
 * Arguments: signal => the code of the signal that was issued to Snort
 *            data => any arguments or data structs linked to this
 *                    functioin when it was registered, may be
 *                    needed to properly exit
 *
 * Returns: void function
 */
static void PreprocCleanExitFunction(int signal, void *data)
{
       /* clean exit code goes here */
}


/*
 * Function: PreprocRestartFunction(int, void *)
 *
 * Purpose: This function gets called when Snort is restarting on a SIGHUP,
 *          if there's any initialization or cleanup that needs to happen
 *          it should be done here.
 *
 * Arguments: signal => the code of the signal that was issued to Snort
 *            data => any arguments or data structs linked to this
 *                    functioin when it was registered, may be
 *                    needed to properly exit
 *
 * Returns: void function
 */
static void PreprocRestartFunction(int signal, void *foo)
{
       /* restart code goes here */
}
