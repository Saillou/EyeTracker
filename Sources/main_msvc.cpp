#include <iostream>
#include <windows.h>

#include "Dk/ManagerConnection.hpp"
#include "Dk/VideoStream.hpp"
#include "Dk/Chronometre.hpp"

using namespace Protocole;

int main() {
	// ------------------------ Search server  ---------------------- //
	ManagerConnection managerConnection;
	managerConnection.initialize();
	
	auto ipOpened = managerConnection.snif(ManagerConnection::IpAdress("192.168.128.200", 3000), ManagerConnection::IpAdress("192.168.128.210", 3000), 1);	
	if(ipOpened.size() == 0)
		return 0;
	// -------------------------------------------------------------- //
	
	// ----------------------- Video stream ------------------------- //
	Dk::VideoStream video(managerConnection, ipOpened[0]);
	
	if(!video.isValide())
		return 0;
	
	if(!video.initFormat())
		return 0;
	
	// Start stream [Play in its own thread]
	video.play();
	
	// Listen event
	Chronometre chrono;
	bool buttonA = false;
	
	while(!(GetKeyState(VK_ESCAPE) & 0x8000)) { // Key Escape pressed : Stop
		int state = GetKeyState(0x41) & 0x8000;

		if(state && buttonA == false) {  // Key A pressed : Pause / Play.
			switch(video.getState()) {
				case Dk::PAUSED: video.play(); break;
				case Dk::PLAYING: video.pause(); break;
			}
			
			buttonA = true;
		}
		else if(!state) // Button A released.
			buttonA = false;

		// Display info
		if(chrono.clock_ms() >= 1000) { 			
			std::cout << "Freq: " << ((int)(10*video.getFpsRate()))/10.0 << " hz \t | \t Lag: " << video.getLag() << " ms." << std::endl;
			chrono.reset();
		}
		
		// Sleep a bit
		Chronometre::wait(5);
	}
	
	video.release();
	// -------------------------------------------------------------- //
	
	return 0;
}