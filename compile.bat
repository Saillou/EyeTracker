@echo off
set exename=client
set folder=D:\Prog\C++Files\030_Sockets\EyeTracker\
cd /d %folder%

::Delete existing executable
if exist Release\%exename%.exe (
	del Release\%exename%.exe
)

::Compile sources
echo Compile
cd Obj
g++ -c -std=gnu++11 -O2 -Wall ^
	..\Sources\main.cpp ^
	..\Sources\Dk\Protocole.cpp ^
	..\Sources\Dk\Socket.cpp ^
	..\Sources\Dk\Server.cpp ^
	..\Sources\Dk\ManagerConnection.cpp ^
	-ID:\Dev\Opencv3\opencv\build_minGW\install\include
	

::Link sources
echo Link
cd ..\
g++ -o Release\%exename%.exe ^
	Obj\main.o ^
	Obj\Protocole.o ^
	Obj\Socket.o ^
	Obj\Server.o ^
	Obj\ManagerConnection.o ^
	-LD:\Dev\Opencv3\opencv\build_minGW\install\x86\mingw\lib ^
	-lopencv_core341 -lopencv_imgproc341 -lopencv_highgui341 -lopencv_imgcodecs341 -lopencv_videoio341 -lm ^
	-lws2_32
	
::Execute software
cd Release
if exist %exename%.exe (	
	echo Execute
	%exename%.exe 
)


PAUSE