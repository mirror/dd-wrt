ThemeSpec Script Files
######################

Overview
********
ThemeSpec Script files are an evolution of the original PreAuth/Login-Option scripts that were introduced to provide dynamic splash page sequences without requiring a separate web server or additional dependencies.

From openNDS version 9 onwards, a library script provides resources for openNDS to use. In addition, a standardised method for including themed specification files is provided. These files are used by the library script to generate the required html for the specified theme and functionality required for the venue.

Pre-installed ThemeSpec script files
************************************

Four pre-installed ThemeSpec script files are included. These provide a "Click to Continue" and a "Username/Email" sequence of "splash" pages.

The first two of these examples are selected using:

``option login_option_enabled '1'`` (for click to continue - the default)

and

``option login_option_enabled '2'`` (for username/email login)

Mode 1 selects the script file theme_click-to-continue.sh for inclusion.

Mode 2 selects the script file theme_user-email-login.sh for inclusion.

Automated Content Updates
-------------------------

The final two examples enable automated updating of content from remote sources. This is ideal for dynamic serving of venue information, news and advertising. These options are selected using:

``option login_option_enabled '3'``

``option themespec_path '/usr/lib/opennds/theme_click-to-continue-custom-placeholders.sh'``

for click to continue (with custom content) and

``option themespec_path '/usr/lib/opennds/theme_user-email-login-custom-placeholders.sh'``

for username-email login (with custom content).

The content to be downloaded is then specified in the configuration. See "ThemeSpec Templates" below.

libopennds.sh
*************
This utility controls many of the functions required for PreAuth/ThemeSpec scripts.

**Warning**: This file will not normally be modified for customisation of the splash page sequence. *Do not edit unless you know what you are doing!*

Customisation should be carried out in a related Themespec file.

  Usage: libopennds arg1 arg2 ... argN

    **arg1**: "?fas=<b64string>", generates ThemeSpec html using b64encoded data sent from openNDS

        **arg2**: urlencoded_useragent_string

        **arg3**: mode (1, 2 or 3) (this is the mode specified in option login_option in the config file.

        **arg4**: themespecpath (if mode = 3. Will be supplied by the call from openNDS)


    *returns*: html for the specified ThemeSpec.

ThemeSpec Templates
*******************

Two additional example Themespec script files are provided. These make full use of custom parameters, custom variables, custom images and custom files to define custom placeholders.

These are:
 1. theme_click-to-continue-custom-placeholders.sh
 2. theme_user-email-login-custom-placeholders.sh

The first gives a click to continue login, but with the custom placeholders in place.

The second gives the user email login with custom placeholders.

These two are very similar with a login form added to the second.

The second file will be used here as an example template.

Template: theme_user-email-login-custom-placeholders.sh
*******************************************************

The default openNDS config file contains the custom options ready to uncomment.

These options are as follows:

``list fas_custom_parameters_list 'logo_message=openNDS:%20Perfect%20on%20OpenWrt!'``

``list fas_custom_parameters_list 'banner1_message=BlueWave%20-%20Wireless%20Network%20Specialists'``

``list fas_custom_parameters_list 'banner2_message=HMS%20Pickle'``

``list fas_custom_parameters_list 'banner3_message=SeaWolf%20Cruiser%20Racer'``

``list fas_custom_variables_list 'input=phone:Phone%20Number:text;postcode:Home%20Post%20Code:text'``

``list fas_custom_images_list 'logo_png=https://openwrt.org/_media/logo.png'``

``list fas_custom_images_list 'banner1_jpg=https://raw.githubusercontent.com/openNDS/openNDS/9.0.0/resources/bannerbw.jpg'``

``list fas_custom_images_list 'banner2_jpg=https://raw.githubusercontent.com/openNDS/openNDS/9.0.0/resources/bannerpickle.jpg'``

``list fas_custom_images_list 'banner3_jpg=https://raw.githubusercontent.com/openNDS/openNDS/9.0.0/resources/bannerseawolf.jpg'``

``list fas_custom_files_list 'advert1_htm=https://raw.githubusercontent.com/openNDS/openNDS/9.0.0/resources/bannerpickle.htm'``

As can be seen, these are of the form: ``placeholder=value``

Custom Parameters
-----------------
In this case the custom parameters each define a short text string.
These strings will be used as titles for custom images.

Custom Images
-------------
The custom images define remote image files with the URL for download. The images will be downloaded and saved in the tmpfs volatile storage of the router. The themespec script will add the images and the custom parameters (as titles) to the output html served to the client.

Custom Files
------------
A single custom file is defined. This is a remote html file that is downloaded in the same way as custom images are. This downloaded file is included in the html at the relevant placeholder location in the html served to the client.

Custom Variables
----------------
A single custom variable is defined. Instead of a single placeholder, in this case, the variable definition has the keyword "input=".

The value of this variable is used by the themespec script to inject a list of html form input fields, the location in the output html determined by placeholders.

In this case the custom variable value is:

``phone:Phone%20Number:text;postcode:Home%20Post%20Code:text``

This is a list of semi-colon separated fields.

Each field is a colon separated field specification in the form of name:title:type.

In this example we have two input fields:

 * name=phone, title=Phone%20Number, and input type=text
 * name=postcode, title=Home%20Post%20Code

The resulting html served to the client will have two additional input fields on the login page ie. phone number and post code.

**Note**: Spaces must be url encoded ie replaced with %20, to prevent parsing issues.

Serving the Splash Page Sequence
--------------------------------
When a client connects, openNDS calls the libopennds.sh library script passing a request for client verification along with information about the client device. This information is b64encoded into a single argument.

This argument is identified by the initial character string "?fas="

The libopennds library then decodes the string and parses for data required for verification and logging.

The libopennds library then calls the themespec file configured in the openNDS config.

For this example theme_user-email-login-custom-placeholders is called:

 * The themespec script sets Quotas and Data Rates that may be required for this theme, overriding global values. These new values, if set, can be set again later in this script on a client by client basis if required. In this case we will set them to "0" (zero), meaning the global values will take effect.
 * The themespec script then configures itself for any custom requirements such as parameters, images, files and form inputs.
 * Control is then passed back to libopennds
 * libopennds then calls download_image_files() if required by themespec. Files are not downloaded if already present.
 * libopennds then calls download_data_files() if required by themespec. Files are not downloaded if already present.
 * libopennds then sends the html page header to openNDS to be served to the client.
 * libopennds checks if "Terms of Service" has been clicked and if it has, calls display_terms().
 * libopennds checks if the landing page has been requested and if it has, calls landing_page().
 * libopennds calls generate_splash_sequence() in the themespec script.
 * themespec checks if this is the initial redirect of the client. If is is, the first page of the splash page sequence is then served ie the "login page".
 * themespec serves the second page of the splash sequence (thankyou page) once the login page is completed by the client.
 * themespec returns to libopennds with a request for authentication once the "thankyou page" is accepted by the client.
 * libopennds calls landing_page() - the landing page defined in themespec is served to the client.
 * libopennds finally calls openNDS to authenticate the client, passing on any quotas specific to the theme or client.

Community Provided ThemeSpec scripts
************************************

Community provided Themespec scripts are not included in the compiled package, instead they are kept in the Github repository for download.

A description and installation instructions will be provided in a README.md file for each community script.

The first two community scripts are:

Theme Voucher
-------------
This ThemeSpec provides a simple portal requiring a voucher to login.

Theme Legacy
------------
This ThemeSpec enables the legacy splash.html portal page from a Themespec script.

Link to community Scripts
-------------------------
The full list of Community scripts can be seen here:

https://github.com/openNDS/openNDS/tree/master/community/themespec

