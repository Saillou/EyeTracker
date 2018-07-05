@echo off
set exename=client
set folder=D:\Prog\C++Files\030_Sockets\EyeTracker\Windows\
cd /d %folder%

set LINK=0
set EXECUTE=0

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
	

if LINK==1 (
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
		
	if EXECUTE == 1 (
		::Execute software
		cd Release
		if exist %exename%.exe (	
			echo Execute
			%exename%.exe 
		)
	)
	
)

PAUSE