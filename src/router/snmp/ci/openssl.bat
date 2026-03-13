REM Download and install OpenSSL
rmdir /s /q C:\OpenSSL-Win32
rmdir /s /q C:\OpenSSL-v11-Win32
rmdir /s /q C:\OpenSSL-Win64
rmdir /s /q C:\OpenSSL-v11-Win64
curl -fsS -o openssl.exe https://slproweb.com/download/Win64OpenSSL-3_6_0.exe
.\openssl.exe /suppressmsgboxes /silent /norestart /nocloseapplications /log=openssl-installation-log.txt /dir=C:\OpenSSL-Win64
rem type openssl-installation-log.txt
