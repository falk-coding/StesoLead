@echo off
rem Installerar Bakelit.vst3 (Falk Instruments) i Windows standardmapp for VST3-plugins.
rem Kraver administratorsbehorighet (mappen ligger under Program Files).

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Ber om administratorsbehorighet ...
    powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

set "SRC=%~dp0Bakelit.vst3"
set "DST=%CommonProgramFiles%\VST3\Bakelit.vst3"
set "OLD=%CommonProgramFiles%\VST3\StesoLead.vst3"

if not exist "%SRC%" (
    echo Hittar inte Bakelit.vst3 bredvid den har filen. Packa upp hela zip-filen forst.
    pause
    exit /b 1
)

if not exist "%CommonProgramFiles%\VST3" mkdir "%CommonProgramFiles%\VST3"
rem Tidigare namn (StesoLead) har samma plugin-ID - ta bort sa att det inte finns tva.
if exist "%OLD%" rmdir /s /q "%OLD%"
if exist "%DST%" rmdir /s /q "%DST%"
xcopy "%SRC%" "%DST%\" /e /i /q /y >nul

echo.
echo Klart! Bakelit ar installerat i:
echo   %DST%
echo.
echo Starta om din DAW (t.ex. Reason). Bakelit finns under Instruments - Falk Instruments.
echo.
pause
