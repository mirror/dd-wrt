@ECHO OFF
IF EXIST .\TollBoothExample ( rmdir .\TollBoothExample /q/s )
IF EXIST .\Billing ( rmdir .\Billing /q/s )
IF EXIST .\Stolen ( rmdir .\Stolen /q/s )
IF EXIST .\Traffic ( rmdir .\Traffic /q/s )
MKDIR TollBoothExample
MKDIR Billing
MKDIR Traffic
MKDIR Stolen
ECHO Generating example data, this may take a minute.
START "" /B /WAIT ex_toll_data.exe
ECHO Simulating a day worth of cars passing through an automated toll booth.
START "" /B ex_toll_event.exe
ECHO Event Processor will bill the car owners, checking for stolen cars, and sending traffic alerts.
START "" /B /WAIT ex_toll_booth.exe
