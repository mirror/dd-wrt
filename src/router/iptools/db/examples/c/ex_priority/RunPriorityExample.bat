@ECHO OFF
IF EXIST .\PriorityExample ( rmdir .\PriorityExample /q/s )
IF EXIST .\Destination_Env ( rmdir .\Destination_Env /q/s )
MKDIR PriorityExample
MKDIR Destination_Env
ECHO Starting message simulator and producing 100,000 messages.
START "" /B ex_priority_msg.exe
ECHO Starting the priority sorter.
START "" /B ex_priority.exe
ECHO Starting the destination site.
START "" /B /WAIT ex_priority_dest.exe
