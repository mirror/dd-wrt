/* This module implements and interface to the Newt library */
#include <stdio.h>
#include <slang.h>
#include <newt.h>

SLANG_MODULE(newt);

static int Ok_To_Draw;

static void init (void)
{
   newtInit ();
   Ok_To_Draw = 1;
}

static void cls (void)
{
   if (Ok_To_Draw)
     newtCls ();
}

static void draw_root_text (int *c, int *r, char *s)
{
   if (Ok_To_Draw)
     newtDrawRootText (*c, *r, s);
}

static void open_window (int *c, int *r, int *dc, int *dr, char *title)
{
   if (Ok_To_Draw)
     newtOpenWindow (*c, *r, *dc, *dr, title);
}

static void refresh (void)
{
   if (Ok_To_Draw)
     newtRefresh ();
}

static void finished (void)
{
   if (Ok_To_Draw)
     newtFinished ();
   Ok_To_Draw = 0;
}

#define I SLANG_INT_TYPE
#define S SLANG_STRING_TYPE

static SLang_Intrin_Fun_Type Module_Funs [] =
{
   MAKE_INTRINSIC_0("newtInit", init, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("newtCls", cls, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_IIS("newtDrawRootText", draw_root_text, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_5("newtOpenWindow", open_window, SLANG_VOID_TYPE, I,I,I,I,S),
   MAKE_INTRINSIC_0("newtRefresh", refresh, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("NewtFinished", finished, SLANG_VOID_TYPE),

   SLANG_END_TABLE
};

static SLang_Intrin_Var_Type Module_Variables [] =
{
   SLANG_END_TABLE
};

static SLang_IConstant_Type Module_Constants [] =
{
   SLANG_END_TABLE
};

int init_newt_module_ns (char *ns)
{
   if ((-1 == SLns_add_intrin_fun_table (ns, Module_Funs, "__NEWT__"))
       || (-1 == SLns_add_intrin_var_table (ns, Module_Variables, NULL))
       || (-1 == SLns_add_iconstant_table (ns, Module_Constants, NULL)))
     return -1;

   Ok_To_Draw = 0;

   (void) SLang_add_cleanup_function (finished);

   return 0;
}

/* This function is optional */
void deinit_newt_module (void)
{
   finished ();
}
