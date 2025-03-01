@ECHO OFF
IF EXIST .\AdvertisingExample ( rmdir .\AdvertisingExample /q/s )
IF EXIST .\Stores ( rmdir .\Stores /q/s )
IF EXIST .\Users ( rmdir .\Users /q/s )
MKDIR AdvertisingExample
MKDIR Stores
MKDIR Users
ECHO Generating example data, this may take a minute.
START "" /B /WAIT ex_ad_data.exe
ECHO Simulating GPS coordinates arriving at the central advertising server.
START "" /B ex_ad_event.exe
ECHO The advertising server will return ads that match the customer's location and shopping preferences.
START "" /B /WAIT ex_advertising.exe
