@echo off
setlocal EnableDelayedExpansion
set exename=client
set folder=D:\Prog\C++Files\030_Sockets\EyeTracker\
cd /d %folder%

:: Create environnement
if not exist Windows (
	mkdir Windows
)
if not exist Windows\Objects (
	mkdir Windows\Objects
)
if not exist Windows\Release (
	mkdir Windows\Release
	
	:: Put the dlls inside it
	set cvBinPath=D:\Dev\Opencv3\opencv\build_minGW\install\x86\mingw\bin\
	set thisBinPath=Windows\Release\
	
	Copy !cvBinPath!\libopencv_core341.dll !thisBinPath!
	Copy !cvBinPath!\libopencv_highgui341.dll !thisBinPath!
	Copy !cvBinPath!\libopencv_imgcodecs341.dll !thisBinPath!
	Copy !cvBinPath!\libopencv_imgproc341.dll !thisBinPath!
	Copy !cvBinPath!\libopencv_videoio341.dll !thisBinPath!
	Copy !cvBinPath!\opencv_ffmpeg341.dll !thisBinPath!
)

:: Let's go
cd Windows\

set LINK=1
set EXECUTE=1

::Delete existing executable
if exist Release\%exename%.exe (
	del Release\%exename%.exe
)

::Compile sources
echo Compile
cd Objects
g++ -c -std=gnu++11 -O2 -Wall ^
	..\..\Sources\main_windows.cpp ^
	..\..\Sources\Dk\Protocole.cpp ^
	..\..\Sources\Dk\Socket.cpp ^
	..\..\Sources\Dk\Server.cpp ^
	..\..\Sources\Dk\ManagerConnection.cpp ^
	-ID:\Dev\Opencv3\opencv\build_minGW\install\include
	

if %LINK%==1 (
	::Link sources
	echo Link
	cd ..\
	g++ -o Release\%exename%.exe ^
		Objects\main_windows.o ^
		Objects\Protocole.o ^
		Objects\Socket.o ^
		Objects\Server.o ^
		Objects\ManagerConnection.o ^
		-LD:\Dev\Opencv3\opencv\build_minGW\install\x86\mingw\lib ^
		-lopencv_core341 -lopencv_imgproc341 -lopencv_highgui341 -lopencv_imgcodecs341 -lopencv_videoio341 -lm ^
		-lws2_32
		
	if %EXECUTE% == 1 (
		::Execute software
		cd Release
		if exist %exename%.exe (	
			echo Execute
			%exename%.exe 
		)
	)
	
)

PAUSE