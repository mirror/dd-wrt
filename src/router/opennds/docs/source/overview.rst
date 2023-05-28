Overview
########

What is openNDS?
****************
 openNDS (a short form of "open Network Demarcation Service") is a high performance, small footprint, Captive Portal.

 It provides a border control gateway between a public local area network and the Internet, offering by default a simple splash page restricted Internet connection, yet incorporates an API that allows the creation of sophisticated authentication applications.

 It is a fork of the NoDogSplash project that in turn was derived originally from the codebase of the Wifi Guard Dog project.

Captive Portal Detection (CPD)
******************************
 All modern mobile devices, most desktop operating systems and most browsers now have a CPD process that automatically issues a port 80 request on connection to a network. openNDS detects this and serves a special "**splash**" web page to the connecting client device.

Provide simple and immediate public Internet access
***************************************************
 openNDS provides two selectable ready to run methods.

 * **Click to Continue**. A simple "Click to Continue" dynamic splash page sequence requiring a client user to accept Terms of Service before continuing to access the Internet (*default*). Basic client device information is recorded in a log file.
 * **username/email-address login**. A simple dynamic splash page sequence that requires the client user to enter their username and email-address before accepting the Terms of Service. A welcome page and landing page that can carry an information or advertising payload are served to the client in sequence. Client user and client device information is recorded in the log file. (*This mode is selected in the configuration file*)

  Both these modes are generated using default ThemeSpec script files. These ThemeSpec script files contain easy to edit html blocks, allowing basic content changes to be made very simply.

  Modifying the content seen by users is a simple matter of editing the html blocks within the script file.

  Additional more advanced ThemeSpec files are included and can also be enabled from the config file. These additional files make use of the custom_parameters, custom_images and custom_files config options. Form input fields and text comments can be added, and images and content blocks can be downloaded on demand from remote servers.

Write Your Own Captive Portal.
******************************
 openNDS can be used as the "Engine" behind the most sophisticated Captive Portal systems using the tools provided.

 * **Forward Authentication Service (FAS)**. FAS provides pre-authentication user validation in the form of a set of dynamic web pages, typically served by a web service independent of openNDS, located remotely on the Internet, on the local area network or on the openNDS router.
 * **PreAuth**. A special case of FAS that runs locally on the openNDS router with dynamic html served by NDS itself. This requires none of the overheads of a full FAS implementation and is ideal for openNDS routers with limited RAM and Flash memory.
 * **BinAuth**. A method of running a post authentication script or extension program.
