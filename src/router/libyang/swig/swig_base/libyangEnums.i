%module libyangEnums

/* %rename("$ignore", "not" %$isenum, "not" %$isconstant, "not" %$isenumitem, regextarget=1, fullname=1) ""; */
%rename("$ignore", "not lyd_node", "not" %$isenum, "not" %$isconstant, "not" %$isenumitem, regextarget=1, fullname=1) "";

%{
#include "libyang.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "extensions.h"
#include "xml.h"
%}

%include "libyang.h"
%include "tree_schema.h"
%include "tree_data.h"
%include "extensions.h"
%include "xml.h"
