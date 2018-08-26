% Command-line option parsing.
%
% Examples:
%   (a,b,c values)
%   -i -ja3 -b 4  ==> -i -j -a 3 -b 4
%    -q 3 -sli --foo=3
%
private variable CMDOPT_REQ_VALUE =	0x1;   %  value required
private variable CMDOPT_OPT_VALUE =	0x2;   %  value optional
private variable CMDOPT_INC_VALUE =	0x4;   %  increment value by 1
private variable CMDOPT_APPEND_VALUE =	0x8;   %  append to list
private variable CMDOPT_BOR_VALUE =    0x10;   %  bitwise-or
private variable CMDOPT_BAND_VALUE =   0x20;   %  bitwise-and

private variable CmdOpt_Type = struct
{
   names, flags, convert_method, valuep, bor_value, band_value, default_value, callback_args
};

private define usage_error (opts, name, str)
{
   variable msg = sprintf ("Option %s: %s", name, str);
   if (opts.usage_error != NULL)
     (@opts.usage_error) (msg);
   throw UsageError, msg;
}

private define convert_to_string (opts, opt, name, value)
{
   return value;
}

private define convert_to_int (opts, opt, name, value)
{
   try
     {
	if (1 != __is_datatype_numeric (_slang_guess_type (value)))
	  throw SyntaxError;

	return integer (value);
     }
   catch SyntaxError: usage_error (opts, name, "error parsing value as an integer");
}

private define convert_to_double (opts, opt, name, value)
{
   try
     {
	if (0 == __is_datatype_numeric (_slang_guess_type (value)))
	  throw SyntaxError;

	return atof (value);
     }
   catch SyntaxError: usage_error (opts, name, "error parsing value as a float");
}

define cmdopt_add ()
{
   variable opts, name, valuep;
   variable s = @CmdOpt_Type;
   s.callback_args = __pop_args (_NARGS-3);
   (opts, name, valuep) = ();

   s.flags = 0;
   if (qualifier_exists ("append")) s.flags |= CMDOPT_APPEND_VALUE;
   if (qualifier_exists ("inc")) s.flags |= CMDOPT_INC_VALUE;

   variable type = qualifier ("type");
   switch (type)
     {
      case "string" or case "str":
	s.convert_method = &convert_to_string;
     }
     {
      case "int":
	s.convert_method = &convert_to_int;
     }
     {
      case "float" or case "double":
	s.convert_method = &convert_to_double;
     }
     {
      case NULL:
	s.convert_method = &convert_to_string;
     }
     {
	throw InvalidParmError, sprintf ("type=%s is not supported", type);
     }

   variable default_value = 1;
   if (qualifier_exists ("optional"))
     {
	if (type == NULL)
	  throw InvalidParmError, sprintf ("option %s requires the 'type' qualifier", name);

	s.flags |= CMDOPT_OPT_VALUE;
	default_value = qualifier ("optional");
     }
   else if (type != NULL)
     s.flags |= CMDOPT_REQ_VALUE;

   if (qualifier_exists ("bor"))
     {
	s.bor_value = qualifier ("bor");
	s.flags |= CMDOPT_BOR_VALUE;
     }
   if (qualifier_exists ("band"))
     {
	s.band_value = qualifier ("band");
	s.flags |= CMDOPT_BAND_VALUE;
     }

   s.names = strchop (name, '|', 0);
   s.valuep = valuep;
   s.default_value = qualifier ("default", default_value);

   list_append (opts.opt_list, s);
}

private define set_opt_value (opt, value)
{
   variable opt_value;
   ifnot (opt.flags & CMDOPT_APPEND_VALUE)
     {
	@opt.valuep = value;
	return;
     }
   if ((0 == __is_initialized (opt.valuep))
       || (@opt.valuep == NULL))
     @opt.valuep = {};

   if (typeof (@opt.valuep) != List_Type)
     {
	@opt.valuep = {@opt.valuep};
     }
   list_append (@opt.valuep, value);
}

private define process_value (opts, opt, name, value)
{
   set_opt_value (opt, (@opt.convert_method) (opts, opt, name, value));
}

private define process_option (opts, opt, name, value)
{
   if (opt.flags & CMDOPT_REQ_VALUE)
     {
	if (value == NULL)
	  usage_error (opts, name, "value required");

	if (__is_callable (opt.valuep))
	  {
	     value = (@opt.convert_method) (opts, opt, name, value);
	     (@opt.valuep)(value, __push_args(opt.callback_args));
	     return;
	  }
	process_value (opts, opt, name, value);
	return;
     }

   if (opt.flags & CMDOPT_OPT_VALUE)
     {
	if (__is_callable (opt.valuep))
	  {
	     if (value != NULL)
	       value = (@opt.convert_method) (opts, opt, name, value);

	     (@opt.valuep)(value, __push_args(opt.callback_args));
	     return;
	  }
	if (value != NULL)
	  {
	     process_value (opts, opt, name, value);
	     return;
	  }
	set_opt_value (opt, opt.default_value);
	return;
     }

   if (value != NULL)
     usage_error (opts, name, "value not supported");

   if (__is_callable (opt.valuep))
     {
	(@opt.valuep)(__push_args(opt.callback_args));
	return;
     }

   if (opt.flags & CMDOPT_INC_VALUE)
     {
	@opt.valuep += 1;
	return;
     }

   ifnot (opt.flags & (CMDOPT_BAND_VALUE|CMDOPT_BOR_VALUE))
     {
	set_opt_value (opt, opt.default_value);
	return;
     }

   if (opt.flags & CMDOPT_BAND_VALUE)
     @opt.valuep &= opt.band_value;

   if (opt.flags & CMDOPT_BOR_VALUE)
     @opt.valuep |= opt.bor_value;

}

private define find_opt (opts, name)
{
   foreach (opts.opt_list)
     {
	variable opt = ();
	if (any (opt.names == name))
	  return opt;
     }
   usage_error (opts, name, "not supported/unknown");
}

private define find_short_opt (opts, name)
{
   return find_opt (opts, name);
}

private define find_long_opt (opts, name)
{
   return find_opt (opts, name);
}

private define process_short_args (opts, letters)
{
   variable i = 0, n = strlen (letters);
   while (i < n)
     {
	i++;
	variable name = substr (letters, i, 1);
	variable opt = find_short_opt (opts, name);
	variable value = NULL;
	if (opt.flags & CMDOPT_REQ_VALUE)
	  {
	     if (i < n)
	       value = substr (letters, i+1, n);
	     i = n;
	  }
	process_option (opts, opt, name, NULL);
     }
}

private define parse_arg (arg)
{
   variable pos = is_substr (arg, "=");
   if (pos == 0)
     return (arg, NULL);
   variable value = substr (arg, pos+1, -1);
   arg = substr (arg, 1, pos-1);

   return arg, value;
}

define cmdopt_process (opts, argv, istart)
{
   variable iend = length (argv);
   variable i = istart;
   while (i < iend)
     {
	variable arg = argv[i];
	variable opt, value;

	if (arg == "--")
	  return i+1;

	if (arg[0] != '-')
	  return i;

	if (arg == "-")
	  return i;

	if (arg[1] == '-')
	  {
	     % --long-opt
	     (arg, value) = parse_arg (arg);
	     arg = substr(arg, 3, -1);
	     opt = find_long_opt (opts, arg);
	  }
	else
	  {
	     % short arg: -a -ab -abc value
	     % -abc value is equiv to -ab -c value
	     arg = substr (arg, 2, -1);
	     value = NULL;
	     variable j = 0, n = strlen(arg);
	     while (j < n)
	       {
		  j++;
		  variable name = substr (arg, j, 1);
		  opt = find_short_opt (opts, name);
		  if (opt.flags & CMDOPT_REQ_VALUE)
		    {
		       if (j < n)
			 value = substr (arg, j+1, n);
		       arg = name;
		       break;
		    }
		  process_option (opts, opt, name, NULL);
	       }
	     then
	       {
		  i++;
		  continue;
	       }
	  }

	if (opt == NULL)
	  return -1;

	if ((opt.flags & CMDOPT_REQ_VALUE) && (value == NULL))
	  {
	     i++;
	     if (i == iend)
	       usage_error (opts, opt, "value expected");
	     value = argv[i];
	  }

	process_option (opts, opt, arg, value);
	i++;
     }

   return i;
}

define cmdopt_new ()
{
   variable error_routine = NULL;
   if (_NARGS == 1)
     error_routine = ();

   variable s = struct
     {
	usage_error = error_routine,
	opt_list = {},
	add = &cmdopt_add,
	process = &cmdopt_process
     };
   return s;
}
