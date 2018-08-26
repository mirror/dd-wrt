% -*- mode: slang; mode: fold -*-
import ("zlib");

%{{{ Deflate Object and methods
private define deflate_method ()
{
   if (_NARGS != 2)
     {
	_pop_n (_NARGS);
	usage (".deflate(str [;flush=val])");
     }
   variable z, b;
   (z, b) = ();
   return _zlib_deflate (z.zobj, b, qualifier("flush", ZLIB_NO_FLUSH));
}

private define def_reset_method (z)
{
   _zlib_deflate_reset (z.zobj);
}

private define def_flush_method ()
{
   variable z, flush = ZLIB_FINISH;
   switch (_NARGS)
     {
      case 2:
	(z, flush) = ();
     }
     {
      case 1:
	z = ();
     }
     {
	_pop_n (_NARGS);
	usage (".flush ([val]);  Default is ZLIB_FINISH");
     }

   return _zlib_deflate_flush (z.zobj, flush);
}

private variable Deflate_Object = struct
{
   zobj,
   deflate = &deflate_method,
   reset = &def_reset_method,
   flush = &def_flush_method,
};

define zlib_deflate_new ()
{
   variable z = @Deflate_Object;
   z.zobj = _zlib_deflate_new (qualifier ("level", ZLIB_DEFAULT_COMPRESSION),
			       qualifier ("method", ZLIB_DEFLATED),
			       qualifier ("wbits", 15),
			       qualifier ("memlevel", 8),
			       qualifier ("strategy", ZLIB_DEFAULT_STRATEGY));
   return z;
}

%}}}

%{{{ Inflate Object and methods

private define inflate_method ()
{
   if (_NARGS != 2)
     {
	_pop_n (_NARGS);
	usage (".inflate(str [;flush=val])");
     }
   variable z, b;
   (z, b) = ();
   return _zlib_inflate (z.zobj, b, qualifier("flush", ZLIB_NO_FLUSH));
}

private define inf_reset_method (z)
{
   _zlib_inflate_reset (z.zobj);
}

private define inf_flush_method ()
{
   variable z, flush = ZLIB_FINISH;
   switch (_NARGS)
     {
      case 2:
	(z, flush) = ();
     }
     {
      case 1:
	z = ();
     }
     {
	_pop_n (_NARGS);
	usage (".flush ([val]);  Default is ZLIB_FINISH");
     }

   return _zlib_inflate_flush (z.zobj, flush);
}

private variable Inflate_Object = struct
{
   zobj,
   inflate = &inflate_method,
   reset = &inf_reset_method,
   flush = &inf_flush_method,
};

define zlib_inflate_new ()
{
   variable z = @Inflate_Object;
   z.zobj = _zlib_inflate_new (qualifier ("wbits", 15));
   return z;
}

%}}}

define zlib_deflate ()
{
   if (_NARGS != 1)
     {
	usage ("zstr = zlib_deflate (str [;qualifiers])\n"
	       + " qualifiers:\n"
	       + "  level=val, method=val, wbits=val, memlevel=val, strategy=val");
     }
   variable bstr = ();
   variable z = zlib_deflate_new (;; __qualifiers);

   return _zlib_deflate (z.zobj, bstr, ZLIB_FINISH);
}

define zlib_inflate ()
{
   if (_NARGS != 1)
     {
	usage ("str = zlib_inflate (zstr [;wbits=val])");
     }

   variable zstr = ();
   variable z = zlib_inflate_new (;; __qualifiers);
   return _zlib_inflate (z.zobj, zstr, ZLIB_FINISH);
}
