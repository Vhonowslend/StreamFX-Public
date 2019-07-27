@ECHO OFF
ECHO -- Building Installer --
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" /Qp ".\build\64\installer.iss" > nul
