%module cpp_classes

#define __attribute__(x)
%include <std_except.i>
%catches(std::runtime_error, std::exception, std::string);

%include <cpointer.i>
%include <typemaps.i>
%include <stdint.i>
%include <std_pair.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_shared_ptr.i>

%ignore throw_exception;

/* Xml.hpp */
%shared_ptr(libyang::Xml_Ns);
%newobject Xml_Ns::next;

%shared_ptr(libyang::Xml_Attr);
%newobject Xml_Attr::next;
%newobject Xml_Attr::ns;

%shared_ptr(libyang::Xml_Elem);
%newobject Xml_Elem::parent;
%newobject Xml_Elem::attr;
%newobject Xml_Elem::child;
%newobject Xml_Elem::next;
%newobject Xml_Elem::prev;
%newobject Xml_Elem::ns;
%newobject Xml_Elem::get_ns;


/* Libyang.hpp */
%shared_ptr(libyang::Context);
%newobject Context::info;
%newobject Context::get_module;
%newobject Context::get_module_older;
%newobject Context::load_module;
%newobject Context::get_module_by_ns;
%newobject Context::parse_mem;
%newobject Context::parse_fd;
%newobject Context::parse_data_path;
%newobject Context::parse_path;
%newobject Context::parse_xml;
%newobject Context::get_submodule;
%newobject Context::get_submodule2;
%newobject Context::find_path;
%newobject Context::data_instantiables;
%ignore    Context::swig_ctx;
%ignore    Context::wrap_cb_l;

%shared_ptr(libyang::Set);
%newobject Set::dup;

%newobject create_new_Context;

/* Tree_Data.hpp */
%newobject create_new_Data_Node;

%shared_ptr(libyang::Value);
%newobject Value::enm;
%newobject Value::ident;
%newobject Value::instance;
%newobject Value::leafref;

%shared_ptr(libyang::Data_Node);
%newobject Data_Node::schema;
%newobject Data_Node::attr;
%newobject Data_Node::next;
%newobject Data_Node::prev;
%newobject Data_Node::parent;
%newobject Data_Node::child;
%newobject Data_Node::path;
%newobject Data_Node::qualifed_path;
%newobject Data_Node::dup;
%newobject Data_Node::dup_withsiblings;
%newobject Data_Node::dup_to_ctx;
%newobject Data_Node::find_path;
%newobject Data_Node::find_instance;
%ignore    Data_Node::swig_node;
%ignore    Data_Node::swig_deleter;
%newobject Data_Node::diff;
%newobject Data_Node::new_path;
%newobject Data_Node::node_module;
%newobject Data_Node::print_mem;
%newobject Data_Node::C_lyd_node;

%shared_ptr(libyang::Data_Node_Leaf_List);
%newobject Data_Node_Leaf_List::value;
%newobject Data_Node_Leaf_List::schema;
%newobject Data_Node_Leaf_List::attr;
%newobject Data_Node_Leaf_List::next;
%newobject Data_Node_Leaf_List::prev;
%newobject Data_Node_Leaf_List::parent;
%newobject Data_Node_Leaf_List::child;
%newobject Data_Node_Leaf_List::path;
%newobject Data_Node_Leaf_List::qualifed_path;
%newobject Data_Node_Leaf_List::dup;
%newobject Data_Node_Leaf_List::dup_to_ctx;
%newobject Data_Node_Leaf_List::find_path;
%newobject Data_Node_Leaf_List::find_instance;
%ignore    Data_Node_Leaf_List::swig_node;
%ignore    Data_Node_Leaf_List::swig_deleter;
%newobject Data_Node_Leaf_List::diff;
%newobject Data_Node_Leaf_List::new_path;
%newobject Data_Node_Leaf_List::node_module;
%newobject Data_Node_Leaf_List::print_mem;
%newobject Data_Node_Leaf_List::type;
%newobject Data_Node_Leaf_List::C_lyd_node;

%shared_ptr(libyang::Data_Node_Anydata);
%newobject Data_Node_Anydata::schema;
%newobject Data_Node_Anydata::attr;
%newobject Data_Node_Anydata::next;
%newobject Data_Node_Anydata::prev;
%newobject Data_Node_Anydata::parent;
%newobject Data_Node_Anydata::child;
%newobject Data_Node_Anydata::path;
%newobject Data_Node_Anydata::qualifed_path;
%newobject Data_Node_Anydata::dup;
%newobject Data_Node_Anydata::dup_to_ctx;
%newobject Data_Node_Anydata::find_path;
%newobject Data_Node_Anydata::find_instance;
%ignore    Data_Node_Anydata::swig_node;
%ignore    Data_Node_Anydata::swig_deleter;
%newobject Data_Node_Anydata::diff;
%newobject Data_Node_Anydata::new_path;
%newobject Data_Node_Anydata::node_module;
%newobject Data_Node_Anydata::print_mem;
%newobject Data_Node_Anydata::C_lyd_node;

%shared_ptr(libyang::Attr);
%newobject Attr::value;
%newobject Attr::parent;
%newobject Attr::next;

%shared_ptr(libyang::Difflist);

/* Tree_Schema.hpp */
%shared_ptr(libyang::Module);
%newobject Module::rev;
%newobject Module::data;
%newobject Module::data_instantiables;
%newobject Module::print_mem;

%shared_ptr(libyang::Submodule);
%newobject Submodule::ctx;
%newobject Submodule::rev;
%newobject Submodule::belongsto;

%shared_ptr(libyang::Type_Info_Binary);
%newobject Type_Info_Binary::length;

%shared_ptr(libyang::Type_Bit);

%shared_ptr(libyang::Type_Info_Bits);

%shared_ptr(libyang::Type_Info_Dec64);
%newobject Type_Info_Dec64::range;

%shared_ptr(libyang::Type_Enum);

%shared_ptr(libyang::Type_Info_Enums);

%shared_ptr(libyang::Type_Info_Ident);

%shared_ptr(libyang::Type_Info_Inst);

%shared_ptr(libyang::Type_Info_Num);
%newobject Type_Info_Num::range;

%shared_ptr(libyang::Type_Info_Lref);
%newobject Type_Info_Lref::target;

%shared_ptr(libyang::Type_Info_Str);
%newobject Type_Info_Str::length;
%newobject Type_Info_Str::patterns;

%shared_ptr(libyang::Type_Info_Union);

%shared_ptr(libyang::Type_Info);
%newobject Type_Info::binary;
%newobject Type_Info::bits;
%newobject Type_Info::dec64;
%newobject Type_Info::enums;
%newobject Type_Info::ident;
%newobject Type_Info::inst;
%newobject Type_Info::num;
%newobject Type_Info::lref;
%newobject Type_Info::str;
%newobject Type_Info::uni;

%shared_ptr(libyang::Type);
%newobject Type::ext;
%newobject Type::der;
%newobject Type::parent;
%newobject Type::info;

%shared_ptr(libyang::Iffeature);

%shared_ptr(libyang::Ext_Instance);
%newobject Ext_Instance::module;

%shared_ptr(libyang::Schema_Node);
%newobject Schema_Node::parent;
%newobject Schema_Node::child;
%newobject Schema_Node::next;
%newobject Schema_Node::prev;
%newobject Schema_Node::module;
%newobject Schema_Node::path;
%newobject Schema_Node::child_instantiables;
%newobject Schema_Node::find_path;
%newobject Schema_Node::xpath_atomize;
%ignore    Schema_Node::swig_node;
%ignore    Schema_Node::swig_deleter;

%shared_ptr(libyang::Schema_Node_Container);
%newobject Schema_Node_Container::parent;
%newobject Schema_Node_Container::child;
%newobject Schema_Node_Container::next;
%newobject Schema_Node_Container::prev;
%newobject Schema_Node_Container::module;
%newobject Schema_Node_Container::find_path;
%newobject Schema_Node_Container::xpath_atomize;
%ignore    Schema_Node_Container::swig_node;
%ignore    Schema_Node_Container::swig_deleter;
%newobject Schema_Node_Container::must;
%newobject Schema_Node_Container::tpdf;

%shared_ptr(libyang::Schema_Node_Choice);
%newobject Schema_Node_Choice::parent;
%newobject Schema_Node_Choice::child;
%newobject Schema_Node_Choice::next;
%newobject Schema_Node_Choice::prev;
%newobject Schema_Node_Choice::module;
%newobject Schema_Node_Choice::find_path;
%newobject Schema_Node_Choice::xpath_atomize;
%ignore    Schema_Node_Choice::swig_node;
%ignore    Schema_Node_Choice::swig_deleter;
%newobject Schema_Node_Choice::dflt;

%shared_ptr(libyang::Schema_Node_Leaf);
%newobject Schema_Node_Leaf::parent;
%newobject Schema_Node_Leaf::child;
%newobject Schema_Node_Leaf::next;
%newobject Schema_Node_Leaf::prev;
%newobject Schema_Node_Leaf::module;
%newobject Schema_Node_Leaf::find_path;
%newobject Schema_Node_Leaf::xpath_atomize;
%ignore    Schema_Node_Leaf::swig_node;
%ignore    Schema_Node_Leaf::swig_deleter;
%newobject Schema_Node_Leaf::type;
%newobject Schema_Node_Leaf::is_key;

%shared_ptr(libyang::Schema_Node_Leaflist);
%newobject Schema_Node_Leaflist::parent;
%newobject Schema_Node_Leaflist::child;
%newobject Schema_Node_Leaflist::next;
%newobject Schema_Node_Leaflist::prev;
%newobject Schema_Node_Leaflist::module;
%newobject Schema_Node_Leaflist::find_path;
%newobject Schema_Node_Leaflist::xpath_atomize;
%ignore    Schema_Node_Leaflist::swig_node;
%ignore    Schema_Node_Leaflist::swig_deleter;
%newobject Schema_Node_Leaflist::type;

%shared_ptr(libyang::Schema_Node_List);
%newobject Schema_Node_List::parent;
%newobject Schema_Node_List::child;
%newobject Schema_Node_List::next;
%newobject Schema_Node_List::prev;
%newobject Schema_Node_List::module;
%newobject Schema_Node_List::find_path;
%newobject Schema_Node_List::xpath_atomize;
%ignore    Schema_Node_List::swig_node;
%ignore    Schema_Node_List::swig_deleter;

%shared_ptr(libyang::Schema_Node_Anydata);
%newobject Schema_Node_Anydata::parent;
%newobject Schema_Node_Anydata::child;
%newobject Schema_Node_Anydata::next;
%newobject Schema_Node_Anydata::prev;
%newobject Schema_Node_Anydata::module;
%newobject Schema_Node_Anydata::find_path;
%newobject Schema_Node_Anydata::xpath_atomize;
%ignore    Schema_Node_Anydata::swig_node;
%ignore    Schema_Node_Anydata::swig_deleter;

%shared_ptr(libyang::Schema_Node_Uses);
%newobject Schema_Node_Uses::parent;
%newobject Schema_Node_Uses::child;
%newobject Schema_Node_Uses::next;
%newobject Schema_Node_Uses::prev;
%newobject Schema_Node_Uses::module;
%newobject Schema_Node_Uses::find_path;
%newobject Schema_Node_Uses::xpath_atomize;
%newobject Schema_Node_Uses::when;
%ignore    Schema_Node_Uses::swig_node;
%ignore    Schema_Node_Uses::swig_deleter;
%newobject Schema_Node_Uses::grp;

%shared_ptr(libyang::Schema_Node_Grp);
%newobject Schema_Node_Grp::parent;
%newobject Schema_Node_Grp::child;
%newobject Schema_Node_Grp::next;
%newobject Schema_Node_Grp::prev;
%newobject Schema_Node_Grp::module;
%newobject Schema_Node_Grp::find_path;
%newobject Schema_Node_Grp::xpath_atomize;
%ignore    Schema_Node_Grp::swig_node;
%ignore    Schema_Node_Grp::swig_deleter;

%shared_ptr(libyang::Schema_Node_Case);
%newobject Schema_Node_Case::parent;
%newobject Schema_Node_Case::child;
%newobject Schema_Node_Case::next;
%newobject Schema_Node_Case::prev;
%newobject Schema_Node_Case::module;
%newobject Schema_Node_Case::find_path;
%newobject Schema_Node_Case::xpath_atomize;
%ignore    Schema_Node_Case::swig_node;
%ignore    Schema_Node_Case::swig_deleter;

%shared_ptr(libyang::Schema_Node_Inout);
%newobject Schema_Node_Inout::parent;
%newobject Schema_Node_Inout::child;
%newobject Schema_Node_Inout::next;
%newobject Schema_Node_Inout::prev;
%newobject Schema_Node_Inout::module;
%newobject Schema_Node_Inout::find_path;
%newobject Schema_Node_Inout::xpath_atomize;
%ignore    Schema_Node_Inout::swig_node;
%ignore    Schema_Node_Inout::swig_deleter;

%shared_ptr(libyang::Schema_Node_Notif);
%newobject Schema_Node_Notif::parent;
%newobject Schema_Node_Notif::child;
%newobject Schema_Node_Notif::next;
%newobject Schema_Node_Notif::prev;
%newobject Schema_Node_Notif::module;
%newobject Schema_Node_Notif::find_path;
%newobject Schema_Node_Notif::xpath_atomize;
%ignore    Schema_Node_Notif::swig_node;
%ignore    Schema_Node_Notif::swig_deleter;

%shared_ptr(libyang::Schema_Node_Rpc_Action);
%newobject Schema_Node_Rpc_Action::parent;
%newobject Schema_Node_Rpc_Action::child;
%newobject Schema_Node_Rpc_Action::next;
%newobject Schema_Node_Rpc_Action::prev;
%newobject Schema_Node_Rpc_Action::module;
%newobject Schema_Node_Rpc_Action::find_path;
%newobject Schema_Node_Rpc_Action::xpath_atomize;
%ignore    Schema_Node_Rpc_Action::swig_node;
%ignore    Schema_Node_Rpc_Action::swig_deleter;

%shared_ptr(libyang::Schema_Node_Augment);
%newobject Schema_Node_Augment::parent;
%newobject Schema_Node_Augment::child;
%newobject Schema_Node_Augment::next;
%newobject Schema_Node_Augment::prev;
%newobject Schema_Node_Augment::module;
%newobject Schema_Node_Augment::find_path;
%newobject Schema_Node_Augment::xpath_atomize;
%newobject Schema_Node_Augment::target;
%ignore    Schema_Node_Augment::swig_node;
%ignore    Schema_Node_Augment::swig_deleter;

%shared_ptr(libyang::Substmt);

%shared_ptr(libyang::Ext);
%newobject Ext::module;

%shared_ptr(libyang::Refine_Mod_List);

%shared_ptr(libyang::Refine_Mod);
%newobject Refine_Mod::list;

%shared_ptr(libyang::Refine);
%newobject Refine::module;
%newobject Refine::dflt;
%newobject Refine::mod;

%shared_ptr(libyang::Deviate);
%newobject Deviate::must;
%newobject Deviate::unique;
%newobject Deviate::type;

%shared_ptr(libyang::Deviation);
%newobject Deviation::orig_node;

%shared_ptr(libyang::Import);
%newobject Import::module;

%shared_ptr(libyang::Include);
%newobject Include::submodule;

%shared_ptr(libyang::Revision);
%newobject Tpdf::module;

%shared_ptr(libyang::Tpdf);
%newobject Tpdf::type;

%shared_ptr(libyang::Unique);

%shared_ptr(libyang::Feature);
%newobject Feature::module;
%newobject Feature::depfeatures;

%shared_ptr(libyang::Restr);

%shared_ptr(libyang::When);

%shared_ptr(libyang::Ident);
%newobject Ident::module;
%newobject Ident::der;

%shared_ptr(libyang::Error);

%template(vectorData_Node) std::vector<std::shared_ptr<libyang::Data_Node>>;
%template(vectorSchema_Node) std::vector<std::shared_ptr<libyang::Schema_Node>>;
%template(vector_String) std::vector<std::string>;
%template(vectorModules) std::vector<std::shared_ptr<libyang::Module>>;
%template(vectorType) std::vector<std::shared_ptr<libyang::Type>>;
%template(vectorExt_Instance) std::vector<std::shared_ptr<libyang::Ext_Instance>>;
%template(vectorIffeature) std::vector<std::shared_ptr<libyang::Iffeature>>;
%template(vectorFeature) std::vector<std::shared_ptr<libyang::Feature>>;
%template(vectorWhen) std::vector<std::shared_ptr<libyang::When>>;
%template(vectorRefine) std::vector<std::shared_ptr<libyang::Refine>>;
%template(vectorXml_Elem) std::vector<std::shared_ptr<libyang::Xml_Elem>>;
%template(vectorDeviate) std::vector<std::shared_ptr<libyang::Deviate>>;
%template(vectorDeviation) std::vector<std::shared_ptr<libyang::Deviation>>;
%template(vectorIdent) std::vector<std::shared_ptr<libyang::Ident>>;
%template(vectorRestr) std::vector<std::shared_ptr<libyang::Restr>>;
%template(vectorTpdf) std::vector<std::shared_ptr<libyang::Tpdf>>;
%template(vectorUnique) std::vector<std::shared_ptr<libyang::Unique>>;
%template(vectorSchema_Node_Leaf) std::vector<std::shared_ptr<libyang::Schema_Node_Leaf>>;
%template(vectorSchema_Node_Augment) std::vector<std::shared_ptr<libyang::Schema_Node_Augment>>;
%template(vectorType_Bit) std::vector<std::shared_ptr<libyang::Type_Bit>>;
%template(vectorType_Enum) std::vector<std::shared_ptr<libyang::Type_Enum>>;
%template(vectorError) std::vector<std::shared_ptr<libyang::Error>>;

%template(pairStringLysInformat) std::pair<char *, LYS_INFORMAT>;

%{
/* Includes the header in the wrapper code */
#include "Internal.hpp"
#include "Libyang.hpp"
#include "Tree_Data.hpp"
#include "Tree_Schema.hpp"
#include "Xml.hpp"
#include <vector>
%}

%include "Internal.hpp"
%include "Libyang.hpp"
%include "Tree_Data.hpp"
%include "Tree_Schema.hpp"
%include "Xml.hpp"
