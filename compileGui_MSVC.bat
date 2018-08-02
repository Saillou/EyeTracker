@echo off
setlocal EnableDelayedExpansion

:: Define working directory
set source=..\Sources
set object=Objects
set release=Release
set exeName=TestGUI


cd /d %~dp0

if not exist Windows_msvc (
	mkdir Windows_msvc
)
cd Windows_msvc

if not exist %object% (
	mkdir %object%
)
if not exist %release% (
	mkdir %release%
)


:: Import DLL
set cvBinPath=D:\Dev\Opencv3\opencv\build_vc12\bin\Release\
set thisBinPath=%release%\

echo. Copying dlls needed..
call:copyDLL !cvBinPath! opencv_core320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_highgui320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_imgcodecs320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_imgproc320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_videoio320 !thisBinPath!

:: Set environnement variables
call vcvars64.bat

::Delete existing object executable
if exist %object%\*.obj (
	del %object%\*.obj
)

::Compile sources 
cl	/c /EHa /W4 ^
	%source%\mainGUI.cpp ^
	/I D:\Dev\Opencv3\opencv\build_vc12\include ^
	/Fo%object%\
	
:: If objects were created, try to link
if exist %object%\*.obj (
	:: Link sources for Client
	link /SUBSYSTEM:CONSOLE ^
		%object%\mainGUI.obj ^
		opencv_highgui320.lib ^
		opencv_imgproc320.lib ^
		opencv_core320.lib ^
		/LIBPATH:D:\Dev\Opencv3\opencv\build_vc12\lib\Release ^
		/MACHINE:X64 /INCREMENTAL:NO /NOLOGO /DYNAMICBASE /ERRORREPORT:PROMPT ^
		/out:%release%\%exeName%.exe
)

:: If created, execute software
if exist %release%\%exeName%.exe (
	echo Execute
	echo.
	%release%\%exeName%.exe
) else echo No executable found.
	
echo.&pause&goto:eof

::--------------------------------------------------------
::--------- Function section ---------
::--------------------------------------------------------

:copyDLL
if not exist %~3\%~2.dll (
	copy %~1\%~2.dll %~3
)
goto:eof