\function{png_read}
\synopsis{Read an image from a PNG file}
\usage{img = png_read (file [,&image_type])}
\description
  The \ifun{png_read} function reads the image from the specified file
  and returns it.  If a second argument is present, then it must be a
  reference to a variable that will be set to an integer representing
  the type of image read:
#v+
    PNG_COLOR_TYPE_RGB               (RGB image)
    PNG_COLOR_TYPE_RGBA              (RGB image with an alpha channel)
    PNG_COLOR_TYPE_GRAY              (Grayscale image)
    PNG_COLOR_TYPE_GRAY_ALPHA        (Grayscale image with an alpha channel)
#v-
  The image returned by this function is represented by a 2d array of
  unsigned integers.  If the image type is \ivar{PNG_COLOR_TYPE_RGB}
  or \ivar{PNG_COLOR_TYPE_RGBA}, then the integers will be 32 bits
  wide with the most significant byte corresponding to the alpha
  channel, followed by bytes for the red, green, and blue channels in
  that order.  In this way, the color channels may be extracted from
  the image through simple bit operations, e.g.,
#v+
     (img & 0x0000FF00) shr 8
#v-
  will pick out the green channel.  Alternatively, the
  \exmp{png_rgb_get_?} functions may be used to extract the various
  channels.

  If the image type is \ivar{PNG_COLOR_TYPE_GRAY_ALPHA} then image
  will be represented by 16 bit integers with the most significant
  byte corresponding to the alpha channel and the least significant
  byte representing the grayscale.  If the image type is
  \ivar{PNG_COLOR_TYPE_GRAY} then the image array will consist of
  unsigned 8 bit integers (\exmp{UChar_Type}).
\example
#v+
    define read_grayscale_image (file)
    {
       variable img_type;
       img = png_read (file, &img_type);
       if ((img_type != PNG_COLOR_TYPE_GRAY)
           or (img_type != PNG_COLOR_TYPE_GRAY_ALPHA))
         img = png_rgb_to_gray (img);
       return img;
    }
#v-
\seealso{png_read_flipped, png_write, png_rgb_to_r, png_rgb_to_g, png_rgb_to_b}
\done

\function{png_read_flipped}
\synopsis{Read an image from a PNG file}
\usage{img = png_read_flipped (file [,&image_type])}
\description
  This function is like the \ifun{png_read} function except that it
  returns an image with the rows flipped.  See the documentation for
  \ifun{png_read} for more information.
\seealso{png_read, png_write_flipped}
\done

\function{png_write}
\synopsis{Write an image to a PNG file}
\usage{png_write (file, image [,has_alpha])}
\description
  The \ifun{png_write} function writes a 2d image to the specified
  file as a PNG.  The type of PNG created depends upon the data type
  of the image array according to the following table:
#v+
          data-type           PNG-type
    -----------------------------------
    Char_Type   (8 bit)       grayscale
    UChar_Type  (8 bit)       grayscale
    Int16_Type  (16 bit)      grayscale
    UInt16_Type (16 bit)      grayscale
    Int32_Type  (32 bit)        RGB
    UInt32_Type (32 bit)        RGB
#v-
  If the optional parameter \exmp{has_alpha} is present and non-zero,
  then an alpha channel will also be written to the file.

  See the documentation for the \ifun{png_read} function for
  information about how the various channels are encoded as integers.
\seealso{png_write_flipped, png_read}
\done

\function{png_write_flipped}
\synopsis{Write an image to a PNG file}
\usage{png_write_flipped (file, image [,has_alpha])}
\description
  This function is like the \ifun{png_write} function except that it
  writes the image to the file with the rows reversed. See the
  documentation for \ifun{png_write} for more information.
\seealso{png_read, png_write_flipped}
\done

\function{png_rgb_to_gray}
\synopsis{Convert an RGB image to grayscale}
\usage{gray = png_rgb_to_gray (rgb)}
\description
  This function converts an RGB image represented by an array
  (typically 2d) to a corresponding array of 8 bit integers
  representing the grayscale values. The grayscale values are formed
  by averaging the R, G, and B channels.
\seealso{png_gray_to_rgb}
\done

\function{png_gray_to_rgb}
\synopsis{Convert a grayscale image to RGB}
\usage{rgb = png_gray_to_rgb (gray, colormap)}
\description
  This function creates an RGB image, represented by an array of 32
  bit integers from a grayscale image using the specified colormap.
  The shape of the of the RGB array will be that of the input
  grayscale array.  The values in the input image will be linearly
  mapped onto the elements of the colormap such that the minimum value
  in the grayscale image will be assigned the first RGB value in the
  colormap, and the maximum value will be assigned the last RGB value.

  The colormap argument must either be an array of 32 bit integers
  encoding the RGB information, or must be the name of a supported
  color map.  See the documentation for the \sfun{png_get_colormap}
  function for more information about colormaps.
\qualifiers
  The qualifiers \exmp{gmin} and \exmp{gmax} may be used to define the
  minimum and maximum grayscale values used for the mapping onto the
  colormap.  Grayscale values outside the specified range will be clipped.

\seealso{png_rgb_to_gray, png_get_colormap}
\done

\function{png_get_colormap}
\synopsis{Retrieve a colormap}
\usage{cmap = png_get_colormap (String_Type name)}
\description
  This function returns a colormap of the specified name.  Currently
  recognized names include:
#v+
    cool       Linear change from blue to magenta
    copper     Dark to light copper brown
    gebco      Colors for GEBCO bathymetric charts
    globe      Colors for global bathy-topo relief
    drywet     Goes from dry to wet colors
    gray       Grayramp from black to white
    haxby      Bill Haxby's colortable for geoid & gravity
    hot        Black through red and yellow to white
    jet        Dark to light blue, white, yellow and red
    no_green   For those who hate green
    ocean      white-green-blue bathymetry scale
    polar      Blue via white to red
    rainbow    Rainbow colors magenta-blue-cyan-green-yellow-red
    red2green  Polar scale from red to green via white
    relief     Wessel/Martinez colortable for bathymetry/topography
    topo       Sandwell-Anderson colors for topography
    sealand    Smith bathymetry/topography scale
    seis       R-O-Y-G-B seismic tomography colors
    split      Polar scale like polar, but via black instead of white
    wysiwyg    20 RGB colors for openwin -cubesize large and waxenvy printer
#v-
  The above colormaps were derived from the GMT distribution, which is
  an open source collection of tools for geographic data.  See
  \exmp{http://gmt.soest.hawaii.edu/} for more information about GMT.

  In the current implementation, the colormap is a 256 element array
  of unsigned 32 bit integers.
\seealso{png_gray_to_rgb, png_add_colormap, png_get_colormap_names}
\done

\function{png_add_colormap}
\synopsis{Add a color map}
\usage{png_add_colormap (name, cmap)}
\description
  This function adds a color map of the specified name to the table of
  internal colormaps.  The colormap is simply a 256 element unsigned
  32 integer bit array that maps 256 grayscale values to RGB.
\seealso{png_get_colormap}
\done

\function{png_get_r}
\synopsis{Get the red channel of an RGB image}
\usage{r = png_get_r (rgb)}
\description
  This function returns the red component of the RGB image encoded as
  an unsigned 32 bit integer using the algorithm:
#v+
    r = typecast ((rgb shr 16) & 0xFF, UChar_Type);
#v-
\seealso{png_get_g, png_get_b, png_get_a}
\done

\function{png_get_g}
\synopsis{Get the green channel of an RGB image}
\usage{r = png_get_g (rgb)}
\description
  This function returns the green component of the RGB image encoded as
  an unsigned 32 bit integer using the algorithm:
#v+
    g = typecast ((rgb shr 8) & 0xFF, UChar_Type);
#v-
\seealso{png_get_r, png_get_b, png_get_a}
\done

\function{png_get_b}
\synopsis{Get the blue channel of an RGB image}
\usage{r = png_get_b (rgb)}
\description
  This function returns the blue component of the RGB image encoded as
  an unsigned 32 bit integer using the algorithm:
#v+
    b = typecast ((rgb & 0xFF), UChar_Type);
#v-
\seealso{png_get_r, png_get_g, png_get_a}
\done

\function{png_get_a}
\synopsis{Get the alpha channel of an RGB image}
\usage{r = png_get_a (rgb)}
\description
  This function returns the alpha channel of the RGB image encoded as
  an unsigned 32 bit integer using the algorithm:
#v+
    a = typecast ((rgb shr 24) & 0xFF, UChar_Type);
#v-
\seealso{png_get_g, png_get_b, png_get_a}
\done
