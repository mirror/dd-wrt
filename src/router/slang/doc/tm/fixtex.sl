#!/usr/bin/env jed-script

private variable Version = "0.3.2-0";

if (__argc != 2)
{
   message ("Version $Version Usage: ./fixtex.sl <filename>"$);
   quit_jed ();
}

variable file = __argv[1];
() = read_file (file);

% Patch up the >,< signs
bob ();
replace ("$<$", "<");
replace ("$>$", ">");

% It appears that sgml2tex screws up _for in section titles, producing \_{for}.
replace ("ion\\_{", "ion{\\_");

% Make the first chapter a preface
bob ();
if (bol_fsearch ("\\chapter{Preface}"))
{
   push_spot ();
   push_mark ();
   go_right (8); insert ("*");	       %  \chapter{ --> \chapter*{
   () = bol_fsearch ("\\chapter{");
   push_spot ();

   insert("\\tableofcontents\n");
   eol ();
   insert ("\n\\pagenumbering{arabic}");

   pop_spot ();
   narrow ();
   bob ();
   replace ("\\section{", "\\section*{");
   widen ();

   if (bol_bsearch ("\\tableofcontents"))
     delete_line ();

   pop_spot ();
   if (bol_bsearch ("\\maketitle"))
     insert ("\\pagenumbering{roman}\n");

}

static define fixup_urldefs ()
{
   % pdflatex cannot grok urldef
   bob ();
   while (bol_fsearch("\\urldef{") and ffind ("\\url{"))
     {
	variable line = line_as_string ();
	bol ();
	insert ("\\ifpdf\n");

	deln (7); insert ("\\newcommand");
	push_mark ();
	()=ffind ("}");
	variable macro = bufsubstr ();
	() = ffind ("\\url");
	go_left (1);
	trim ();
	insert("{");

	% pdflatex cannot grok # in urls.  Nuke em.
	if (ffind ("#"))
	  {
	     del_eol ();
	     insert ("}");
	  }
	eol ();
	insert ("}\n\\else\n");
	insert (line); newline ();
	insert ("\\fi\n");
     }
}

static define remove_repeated_urls ()
{
   variable name, url;
   variable names = Assoc_Type[Int_Type, 0];
   while (bol_fsearch ("{\\em "))
     {
	go_right (4);
	skip_white ();
	push_mark ();
	() = ffind ("}");
	!if (looking_at ("} {\\tt "))
	  {
	     pop_mark(0);
	     continue;
	  }
	name = bufsubstr ();
	if (names[name])
	  {
	     go_right(1);
	     push_mark ();
	     () = ffind ("}");
	     go_right(1);
	     del_region ();
	  }
	else
	  {
	     names[name] = 1;
	     go_right(1);
	     () = ffind ("}");
	     go_right (1);
	  }

	% Now remove empty lines inserted by the broken sgml2latex program.
	skip_white ();
	!if (eolp ())
	  continue;
	go_right(1);
	skip_white ();
	if (eolp ())
	  del ();
     }
}

fixup_urldefs ();
remove_repeated_urls ();
save_buffer ();
quit_jed ();

