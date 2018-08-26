autoload ("glob", "glob");
import ("png");

% Colormap functions

private define linear_range (a, b, xa, xb)
{
   return typecast (xa + ([0:b-a] * (xb-xa)/double(b-a)), UChar_Type);
}

private define build_colormap_channel (ranges)
{
   variable r = UChar_Type[256];
   variable i = 0;
   loop (length(ranges)/4)
     {
	variable a = ranges[i];
	variable b = ranges[i+1];
	variable xa = ranges[i+2];
	variable xb = ranges[i+3];
	r[[a:b]] = linear_range (a, b, xa, xb);
	i += 4;
     }
   return r;
}

define png_build_colormap (r_ranges, g_ranges, b_ranges)
{
   return ((build_colormap_channel (r_ranges) shl 16)
	   + (build_colormap_channel (g_ranges) shl 8)
	   + (build_colormap_channel (b_ranges)));
}

private variable Color_Maps = Assoc_Type[Array_Type];
private variable Color_Map_Dir = path_concat (path_dirname (__FILE__), "cmaps");
private variable Png_Namespace = current_namespace ();

define png_add_colormap (name, map)
{
   Color_Maps[name] = map;
}

define png_get_colormap (name)
{
   if (assoc_key_exists (Color_Maps, name))
     return Color_Maps[name];

   variable mapfile = strcat (name, ".map");
   variable file = path_concat (Color_Map_Dir, mapfile);
   if (stat_file (file) == NULL)
     throw OpenError, "Unable to load colormap $mapfile"$;

   () = evalfile (file, Png_Namespace);

   if (assoc_key_exists (Color_Maps, name))
     return Color_Maps[name];

   throw DataError, "$file does not contain the $name color map"$;
}

define png_get_colormap_names ()
{
   variable maps = glob (path_concat (Color_Map_Dir, "*.map"));
   maps = array_map (String_Type, &path_basename_sans_extname, maps);
   variable n = length (maps);
   variable idx = Char_Type[n];
   _for (0, n-1, 1)
     {
	variable i = ();
	!if (assoc_key_exists (Color_Maps, maps[i]))
	  idx[i] = 1;
     }
   return [maps[where(idx)], assoc_get_keys (Color_Maps)];

}

define png_rgb_to_gray (rgb)
{
   variable gray = ((rgb&0xFF) + ((rgb&0xFF00)shr 8) + ((rgb&0xFF0000)shr 16));
   return typecast ((__tmp(gray)/3.0), UChar_Type);
}

private define normalize_gray (gray, nlevels)
{
   variable g0 = qualifier ("gmin");
   variable g1 = qualifier ("gmax");

   if ((typeof (gray) == UChar_Type) && (nlevels == 256)
       && (g0 == NULL) && (g1 == NULL))
     return gray;

   variable is_bad = isnan(gray) or isinf(gray);
   variable any_is_bad = any(is_bad);
   if (any_is_bad)
     {
	variable good_gray = gray [where(is_bad == 0)];
	if (g0 == NULL) g0 = min (good_gray);
	if (g1 == NULL) g1 = max (good_gray);
     }
   else
     {
	if (g0 == NULL) g0 = min(gray);
	if (g1 == NULL) g1 = max(gray);
     }
   if (g0 > g1) (g0, g1) = (g1, g0);

   if (g0 != g1)
     {
	variable factor = nlevels/double(g1-g0);
	gray = typecast ((gray-g0)*factor, Int_Type);
	gray[where (gray<0)] = 0;
	gray[where (gray>=nlevels)] = (nlevels-1);
     }
   else
     gray = typecast (gray * 0 + 127, Int_Type);

   variable bad_level = 0;
   if (any_is_bad)
     gray[where(is_bad)] = bad_level;

   return gray;
}

private define gray_to_rgb_with_cmap (gray, cmap)
{
   if (typeof (cmap) == String_Type)
     cmap = png_get_colormap (cmap);

   gray = normalize_gray (gray, length(cmap);;__qualifiers());
   return cmap[gray];
}

define png_gray_to_rgb ()
{
   variable gray;
   if (_NARGS == 2)
     return gray_to_rgb_with_cmap (;;__qualifiers ());

   gray = ();
   gray = normalize_gray (gray, 256;;__qualifiers ());
   return gray + (gray shl 8) + (gray shl 16);
}

define png_rgb_get_r (rgb)
{
   return typecast ((rgb shr 16) & 0xFF, UChar_Type);
}
define png_rgb_get_g (rgb)
{
   return typecast ((rgb shr 8) & 0xFF, UChar_Type);
}
define png_rgb_get_b (rgb)
{
   return typecast (rgb & 0xFF, UChar_Type);
}

$1 = path_concat (path_dirname (__FILE__), "help/pngfuns.hlp");
if (NULL != stat_file ($1))
  add_doc_file ($1);

provide("png");
