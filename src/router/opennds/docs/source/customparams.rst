Custom Parameters, Variables, Images and files
##############################################

Custom Parameters were first introduced in openNDS version 7.
With version 9.0.0, custom variables, images and files have been added.

Custom Parameters
*****************

Custom parameters are defined in the config file and are sent as fixed values to FAS in the encoded/encrypted query string where they can be parsed and used by the FAS.

This is particularly useful in a remote or centralised FAS that is serving numerous instances of openNDS at different locations/venues.

* Any number of Custom Parameters can be listed in the configuration file, but each one must be in a separate entry in the form of "param_name=param_value"


* param_name and param_value must be urlencoded if containing white space or single quotes.

For example in the OpenWrt UCI config file:

``list fas_custom_parameters_list '<param_name1=param_value1>'``

``list fas_custom_parameters_list '<param_name2=param_value2>'``

etc.

A real example might be:

``list fas_custom_parameters_list 'location=main_building'``

``list fas_custom_parameters_list 'admin_email=norman@bates-motel.com>'``

For a small text strings with spaces, replace each space with %20

 The following Working Example applies to the installed ThemeSpec scripts:

  theme_click-to-continue-custom-placeholders

  and

  theme_user-email-login-custom-placeholders


``list fas_custom_parameters_list 'logo_message=openNDS:%20Perfect%20on%20OpenWrt!'``

``list fas_custom_parameters_list 'banner1_message=BlueWave%20-%20Wireless%20Network%20Specialists'``

``list fas_custom_parameters_list 'banner2_message=HMS%20Pickle'``

``list fas_custom_parameters_list 'banner3_message=SeaWolf%20Cruiser%20Racer'``

Custom Variables
****************

Custom variables are defined in the config file and are sent as fixed placeholders to FAS in the encoded/encrypted query string where they can be parsed and used by the FAS, generally for user input via html forms added by the installer to suit the requirements of the venue.

Custom Dynamically Generated Form Fields
----------------------------------------
 Custom Dynamically Generated Form Fields are a special case of Custom Variables.

 ThemeSpec scripts can dynamically generate Form Field html and inject this into the dynamic splash page sequence.

 This is achieved using a SINGLE line containing the keyword "input", in the form: fieldname:field-description:fieldtype

 Numerous fields can be defined in this single "input=" line, separated by a semicolon (;).

 The following Working Example applies to the installed ThemeSpec scripts:

  theme_click-to-continue-custom-placeholders

  and

  theme_user-email-login-custom-placeholders

``list fas_custom_variables_list 'input=phone:Phone%20Number:text;postcode:Home%20Post%20Code:text'``

Custom Images
*************
Custom images are defined in the config file and are sent as fixed name/URL pairs to FAS in the encoded/encrypted query string where they can be parsed and used by the FAS, and added by the installer to suit the requirements of the venue.

 The following Working Example applies to the installed ThemeSpec scripts:

  theme_click-to-continue-custom-placeholders

  and

  theme_user-email-login-custom-placeholders

**Note:** These pre-installed ThemeSpec script files will automatically download remote images to volatile storage on the router.

``list fas_custom_images_list 'logo_png=https://openwrt.org/_media/logo.png'``

``list fas_custom_images_list 'banner1_jpg=https://raw.githubusercontent.com/openNDS/openNDS/v9.5.0/resources/bannerbw.jpg'``

``list fas_custom_images_list 'banner2_jpg=https://raw.githubusercontent.com/openNDS/openNDS/v9.5.0/resources/bannerpickle.jpg'``

``list fas_custom_images_list 'banner3_jpg=https://raw.githubusercontent.com/openNDS/openNDS/v9.5.0/resources/bannerseawolf.jpg'``

Custom Files
************

Custom files are defined in the config file and are sent as fixed name/URL pairs to FAS in the encoded/encrypted query string where they can be parsed and used by the FAS, and added by the installer to suit the requirements of the venue. (in the same way as custom images)

 The following Working Example applies to the installed ThemeSpec scripts:

  theme_click-to-continue-custom-placeholders

  and

  theme_user-email-login-custom-placeholders

**Note:** These pre-installed ThemeSpec script files will automatically download remote files to volatile storage on the router.

``list fas_custom_files_list 'advert1_htm=https://raw.githubusercontent.com/openNDS/openNDS/v9.5.0/resources/bannerpickle.htm'``
