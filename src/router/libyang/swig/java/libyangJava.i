%module yang

%{
    extern "C" {
        #include "libyang.h"
        #include "tree_data.h"
    }
%}

%include exception.i
%include <std_except.i>
%catches(std::runtime_error, std::exception, std::string);

%inline %{
#include <unistd.h>
#include "libyang.h"
#include <signal.h>
#include <vector>
#include <memory>

#include "Libyang.hpp"
#include "Tree_Data.hpp"

%}
%include "../swig_base/java_base.i"
