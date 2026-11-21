@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%~dp0"
set GSTREAMER_ROOT=C:\Users\Acer\AppData\Local\Programs\gstreamer\1.0\msvc_x86_64
set PATH=%GSTREAMER_ROOT%\bin;%PATH%
cl tutorial3.c /I"%GSTREAMER_ROOT%\include\gstreamer-1.0" /I"%GSTREAMER_ROOT%\include\glib-2.0" /I"%GSTREAMER_ROOT%\lib\glib-2.0\include" /link /LIBPATH:"%GSTREAMER_ROOT%\lib" gstreamer-1.0.lib gobject-2.0.lib glib-2.0.lib /OUT:tutorial3.exe
if %ERRORLEVEL% EQU 0 (
  echo.
  echo Build OK. Run with:
  echo   .\tutorial3.exe
)
