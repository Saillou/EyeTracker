@echo off
setlocal EnableDelayedExpansion

:: Define working directory
set source=Source
set object=Obj
set release=Release
set exeName=Test

cd /d %~dp0

if not exist %object% (
	mkdir %object%
)
if not exist %release% (
	mkdir %release%
)


:: Import DLL
set cvBinPath=D:\Dev\Opencv3\opencv\build_vc12\bin\Release\
set libjpgPath=D:\Dev\LibJpeg\build\Release\
set thisBinPath=%release%\

echo. Copying dlls needed..
call:copyDLL !cvBinPath! opencv_core320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_highgui320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_imgcodecs320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_imgproc320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_videoio320 !thisBinPath!
call:copyDLL !cvBinPath! opencv_ffmpeg320_64 !thisBinPath!
call:copyDLL !libjpgPath! turbojpeg !thisBinPath!

:: Set environnement variables
call vcvars64.bat

::Delete existing object executable
if exist %object%\*.obj (
	del %object%\*.obj
)
if exist %release%\%exeName%.exe (
	del %release%\%exeName%.exe
)

::Compile sources
cl	/c /EHsc ^
	%source%\main.cpp ^
	%source%\Dk\Protocole.cpp ^
	%source%\Dk\Socket.cpp ^
	%source%\Dk\Server.cpp ^
	%source%\Dk\ManagerConnection.cpp ^
	/I D:\Dev\Opencv3\opencv\build_vc12\include ^
	/I D:\Dev\LibJpeg\libjpeg-turbo-1.5.2 ^
	/D USE_MSVC ^
	/Fo%object%\
	
:: If objects were created, try to link
if exist %object%\*.obj (
	:: Link sources
	link /SUBSYSTEM:CONSOLE ^
		%object%\main.obj ^
		%object%\Protocole.obj ^
		%object%\Socket.obj ^
		%object%\Server.obj ^
		%object%\ManagerConnection.obj ^
		opencv_videoio320.lib ^
		opencv_highgui320.lib ^
		opencv_imgcodecs320.lib ^
		opencv_imgproc320.lib ^
		opencv_core320.lib ^
		turbojpeg.lib ^
		ws2_32.lib ^
		/LIBPATH:D:\Dev\Opencv3\opencv\build_vc12\lib\Release ^
		/LIBPATH:D:\Dev\LibJpeg\build\Release ^
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