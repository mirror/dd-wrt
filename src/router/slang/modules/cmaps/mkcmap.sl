define rgb_to_cmap (r, g, b, file)
{
   variable rgb = (r << 16)|(g<<8)|b;
   variable name = path_basename_sans_extname (file);
   if (NULL != stat_file (file))
     throw IOError, sprintf ("File %s exists-- delete it if you want to overwrite", file);

   variable fp = fopen (file, "w");
   () = fputs (`% -*- slang -*-
%

$1 =
[`,
	       fp);
   _for (0, length(rgb)-1, 1)
     {
	variable i = ();
	if ((i mod 8) == 0)
	  () = fputs ("\n  ", fp);
	() = fprintf (fp, "0x%06X,", rgb[i]);
     }
   () = fprintf (fp, `
];

png_add_colormap ("%s", __tmp($1));
`,
		 name);
   () = fclose (fp);
}
