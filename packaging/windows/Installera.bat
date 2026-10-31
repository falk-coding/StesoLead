@echo off
rem Installerar StesoLead.vst3 i Windows standardmapp for VST3-plugins.
rem Kraver administratorsbehorighet (mappen ligger under Program Files).

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Ber om administratorsbehorighet ...
    powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

set "SRC=%~dp0StesoLead.vst3"
set "DST=%CommonProgramFiles%\VST3\StesoLead.vst3"

if not exist "%SRC%" (
    echo Hittar inte StesoLead.vst3 bredvid den har filen. Packa upp hela zip-filen forst.
    pause
    exit /b 1
)

if not exist "%CommonProgramFiles%\VST3" mkdir "%CommonProgramFiles%\VST3"
if exist "%DST%" rmdir /s /q "%DST%"
xcopy "%SRC%" "%DST%\" /e /i /q /y >nul

echo.
echo Klart! StesoLead ar installerat i:
echo   %DST%
echo.
echo Starta om din DAW (t.ex. Reason). StesoLead finns under Instruments - Falk.
echo Den fristaende appen StesoLead.exe kan startas direkt fran zip-mappen.
echo.
pause
