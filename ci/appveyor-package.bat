ECHO -- Building 7z Archive --
cmake --build build/64 --target PACKAGE_7Z --config RelWithDebInfo
ECHO -- Building Zip Archive --
cmake --build build/64 --target PACKAGE_ZIP --config RelWithDebInfo
ECHO -- Building Installer --
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" /Qp ".\build\64\installer.iss" > nul