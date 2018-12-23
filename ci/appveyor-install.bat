git submodule update --init --force --recursive

IF EXIST inno.exe (
	curl -kL "%INNOSETUP_URL%" -f --retry 5 -o inno.exe -z inno.exe
) else (
	curl -kL "%INNOSETUP_URL%" -f --retry 5 -o inno.exe
)
inno.exe /VERYSILENT /NORETART /SP- /SUPPRESSMSGBOXES