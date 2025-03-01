@ECHO OFF
IF EXIST .\FirewallExample ( rmdir .\FirewallExample /q/s )
IF EXIST .\Destination_Env ( rmdir .\Destination_Env /q/s )
MKDIR FirewallExample
MKDIR Destination_Env
ECHO Starting message simulator and producing 100,000 messages.
START "" /B ex_firewall_msg.exe
ECHO Starting the destination site.
START "" /B ex_firewall_dest.exe -d destination -h Destination_Env
ECHO Starting the firewall.
START "" /B /WAIT ex_firewall.exe
