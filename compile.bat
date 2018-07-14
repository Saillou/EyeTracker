@echo off
setlocal EnableDelayedExpansion

:: Commands
set /a DO_COMPILE=1
set /a DO_LINK=1
set /a DO_EXECUTE=1
set OUTPUT_NAME=client

:: DLL paths
set cvBinPath=D:\Dev\Opencv3\opencv\build_minGW\install\x86\mingw\bin\
set libjpgPath=D:\Dev\LibJpeg\buildMingw\
set thisBinPath=Windows\Release\

:: Create environnement
cd /d %~dp0

if not exist Windows (
	mkdir Windows
)
if not exist Windows\Objects (
	mkdir Windows\Objects
)
if not exist Windows\Release (
	mkdir Windows\Release
)

:: Import DLL
echo. Copying dlls needed..
call:copyDLL !cvBinPath! libopencv_core341 !thisBinPath!
call:copyDLL !cvBinPath! libopencv_highgui341 !thisBinPath!
call:copyDLL !cvBinPath! libopencv_imgcodecs341 !thisBinPath!
call:copyDLL !cvBinPath! libopencv_imgproc341 !thisBinPath!
call:copyDLL !cvBinPath! libopencv_videoio341 !thisBinPath!
call:copyDLL !cvBinPath! opencv_ffmpeg341 !thisBinPath!
call:copyDLL !libjpgPath! libturbojpeg !thisBinPath!

:: Let's go
cd Windows\

::Delete existing objects
if exist Objects\*.o (
	del Objects\*.o
)

::Compile sources to objects in the Objects directory
if %DO_COMPILE% == 1 (
	echo. Compile
	cd Objects
	g++ -c -std=gnu++11 -O2 -Wall ^
		..\..\Sources\main_windows.cpp ^
		..\..\Sources\Dk\Protocole.cpp ^
		..\..\Sources\Dk\Socket.cpp ^
		..\..\Sources\Dk\Server.cpp ^
		..\..\Sources\Dk\ManagerConnection.cpp ^
		-ID:\Dev\LibJpeg\libjpeg-turbo-1.5.2 ^
		-ID:\Dev\Opencv3\opencv\build_minGW\install\include
	cd ..\	
)

if %DO_LINK%==1 (
	::Delete existing executable
	if exist Release\%OUTPUT_NAME%.exe (
		del Release\%OUTPUT_NAME%.exe
	)
	
	::Link objects into an executable
	echo. Link
	g++ -o Release\%OUTPUT_NAME%.exe ^
		Objects\main_windows.o ^
		Objects\Protocole.o ^
		Objects\Socket.o ^
		Objects\Server.o ^
		Objects\ManagerConnection.o ^
		-LD:\Dev\LibJpeg\buildMingw ^
		-LD:\Dev\Opencv3\opencv\build_minGW\install\x86\mingw\lib ^
		-lturbojpeg ^
		-lopencv_core341 -lopencv_imgproc341 -lopencv_highgui341 -lopencv_imgcodecs341 -lopencv_videoio341 -lm ^
		-lws2_32
)

if %DO_EXECUTE% == 1 (
	::Launch executable from its directory
	cd Release
	if exist %OUTPUT_NAME%.exe (	
		echo. Execute
		echo.
		%OUTPUT_NAME%.exe
	) else echo No executable found.
)

echo.&pause&goto:eof

::--------------------------------------------------------
::--------- Function section ---------
::--------------------------------------------------------

:copyDLL
if not exist %~3\%~2.dll (
	copy %~1\%~2.dll %~3
)
goto:eof
