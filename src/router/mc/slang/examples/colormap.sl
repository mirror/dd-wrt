require ("png");
define slsh_main ()
{
   variable image_row = [0:255];
   variable gray_img = UChar_Type[32,256];
   variable i;
   _for i (0, 31, 1) gray_img[i,*] = image_row;

   variable cmaps = png_get_colormap_names ();
   variable n = length (cmaps);
   variable rgb_image = UInt32_Type [32*n, 256];
   _for i (0, n-1, 1)
     {
	variable cmap = cmaps[i];
	rgb_image[[i*32+[0:31]],*] = png_gray_to_rgb (gray_img, cmap);
     }
   png_write ("colormap.png", rgb_image);
}
