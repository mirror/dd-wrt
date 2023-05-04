/* $Id$ */
/* Snort Detection Plugin Source File Template */

/* sp_template
 *
 * Purpose:
 *
 * Detection engine plugins test an aspect of the current packet and report
 * their findings.  The function may be called many times per packet with
 * different arguments.  These functions are acccessed from the rules file
 * as standard rule options.  When adding a plugin to the system, be sure to
 * add the "Setup" function to the InitPlugins() function call in
 * plugbase.c!
 *
 * Arguments:
 *
 * This is the type of arguements that the detection plugin can take when
 * referenced as a rule option
 *
 * Effect:
 *
 * What the plugin does.
 *
 * Comments:
 *
 * Any comments?
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"

/*
 * don't forget to include the name of this file in plugbase.c!
 */

/*
 * setup any data structs here
 */
typedef struct _TemplateData
{
    /*
     * your detection option data
     * structure info goes here
     */

} TemplateData;

/* function prototypes go here */
static void TemplateInit(char *, OptTreeNode *, int);
static void TemplateRuleParseFunction(char *, OptTreeNode *, TemplateData *);
static int TemplateDetectorFunction(Packet *, struct _OptTreeNode *,
        OptFpList *);

/*
 *
 * Function: SetupTemplate()
 *
 * Purpose: Generic detection engine plugin template.  Registers the
 *          configuration function and links it to a rule keyword.  This is
 *          the function that gets called from InitPlugins in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void SetupTemplate()
{
    /* map the keyword to an initialization/processing function */
    RegisterPlugin("keyword", TemplateInit);

    DebugMessage(DEBUG_PLUGIN,"Plugin: TemplateName Setup\n");
}


/*
 *
 * Function: TemplateInit(char *, OptTreeNode *)
 *
 * Purpose: Generic rule configuration function.  Handles parsing the rule
 *          information and attaching the associated detection function to
 *          the OTN.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 */
static void TemplateInit(char *data, OptTreeNode *otn, int protocol)
{
    TemplateData *template_data;
    OptFpList *ofl;

    /*
     * allocate the data structure and attach
     * it to the rule's data struct list
     */
    template_data = (TemplateData *) SnortAlloc(sizeof(TemplateData));

    /*
     * If this is a transport layer protocol plugin, be sure to
     * check that the protocol that is passed in matches the
     * transport layer protocol that you're using for this rule!
     */

    /*
     * any other initialization of this plugin should be performed here
     */

    /*
     * this is where the keyword arguments are processed and
     * placed into the rule option's data structure
     */
    TemplateRuleParseFunction(data, otn, template_data);

    /*
     * finally, attach the option's detection function
     * to the rule's detect function pointer list
     *
     * AddOptFuncToList returns a pointer to the node in
     * the function pointer list where the detector function
     * is linked into the detection engine, we will grab the
     * pointer to this node so that we can assign the
     * config data for this rule option to the functional
     * node's context pointer
     */
    ofl = AddOptFuncToList(TemplateDetectorFunction, otn);

    /*
     * this is where we set the functional node's context pointer
     * so that the plugin can find the data to test the network
     * traffic against
     */
    ofl->context = (void *) template_data;
}



/*
 *
 * Function: TemplateRuleParseFunction(char *, OptTreeNode *)
 *
 * Purpose: This is the function that is used to process the option keyword's
 *          arguments and attach them to the rule's data structures.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *            td => pointer to the configuration storage struct
 *
 * Returns: void function
 *
 */
static void TemplateRuleParseFunction(
        char *data,
        OptTreeNode *otn,
        TemplateData *td)
{
    /*
     * manipulate the option arguments here
     */

    /*
     * see the code in src/detection_plugins for examples of parsing Snort
     * rule options
     */

    /*
     * set the final option arguments here
     */
}


/*
 *
 * Function: TemplateDetectorFunction(char *, OptTreeNode *, OptFpList *)
 *
 * Purpose: Use this function to perform the particular detection routine
 *          that this rule keyword is supposed to encompass.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *            fp_list => pointer to the function pointer list current node
 *
 * Returns: If the detection test fails, this function *must* return a zero!
 *          On success, it calls the next function in the detection list
 *
 */
static int TemplateDetectorFunction(
        Packet *p,
        struct _OptTreeNode *otn,
        OptFpList *fp_list)
{
    TemplateData *td;   /* ptr to the detection option's data */

    /*
     * Try to make this function as quick as possible, the faster the
     * detection plugins are, the less packet loss the program will
     * experience!  Avoid doing things like declaring variables or
     * anything other than just doing the test and moving on...
     */

    /*
     * get the current option's context data
     */
    td = (TemplateData *) fp_list->context;

    /*
     * your detection function tests go here
     */
    if (the_test_is_successful)
    {
        /* call the next function in the function list recursively */
        /* THIS CALL *MUST* BE IN THE PLUGIN, OTHERWISE YOU WILL BREAK
           SNORT'S DETECTION ENGINE!!! */
        return fp_list->next->OptTestFunc(p, otn, fp_list->next);
    }
#ifdef DEBUG
    else
    {
        /*
         * you can put debug comments here or not
         */
        DebugMessage(DEBUG_PLUGIN,"No match\n");
    }
#endif

    /*
     * if the test isn't successful, this function *must* return 0
     */
    return 0;
}
