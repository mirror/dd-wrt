#include <stdio.h>
#include <slang.h>

SLANG_MODULE(<MODULE-NAME>);

#define MODULE_MAJOR_VERSION	0
#define MODULE_MINOR_VERSION	0
#define MODULE_PATCH_LEVEL	0
static char *Module_Version_String = "0.0.0";
#define MODULE_VERSION_NUMBER	\
   (MODULE_MAJOR_VERSION*10000+MODULE_MINOR_VERSION*100+MODULE_PATCH_LEVEL)

/* Define intrinsics here */

static SLang_Intrin_Fun_Type Module_Intrinsics [] =
{
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_Intrin_Var_Type Module_Variables [] =
{
   MAKE_VARIABLE("_<MODULE-NAME>_module_version_string", &Module_Version_String, SLANG_STRING_TYPE, 1),
   SLANG_END_INTRIN_VAR_TABLE
};

static SLang_IConstant_Type Module_Constants [] =
{
   MAKE_ICONSTANT("_<MODULE-NAME>_module_version", MODULE_VERSION_NUMBER),
   SLANG_END_ICONST_TABLE
};

int init_<MODULE-NAME>_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if ((-1 == SLns_add_intrin_fun_table (ns, Module_Intrinsics, NULL))
       || (-1 == SLns_add_intrin_var_table (ns, Module_Variables, NULL))
       || (-1 == SLns_add_iconstant_table (ns, Module_Constants, NULL)))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_<MODULE-NAME>_module (void)
{
}
