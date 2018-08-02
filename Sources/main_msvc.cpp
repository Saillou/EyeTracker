#include <iostream>

#include "Dk/ManagerConnection.hpp"
#include "Dk/VideoStream.hpp"
#include "Dk/Chronometre.hpp"

#include "Gui/CvGui.hpp"

using namespace CvGui;

int main() {
	// ------------------------ Search server  ---------------------- //
	ManagerConnection managerConnection;
	managerConnection.initialize();
	
	auto ipOpened = managerConnection.snif(ManagerConnection::IpAdress("192.168.128.10", 3000), ManagerConnection::IpAdress("192.168.128.40", 3000), 1);	
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
	
	// ------ Create GUI ------
	Gui<AddPolicy::Col> gui;
	auto interface0 = gui.createInterface();
	auto button 	= std::make_shared<PushButton>("Play/Pause", cv::Size(150, 50));

	interface0->add(button);
	gui.add(interface0);
	
	// Listen event
	button->listen(PushButton::onClick, [=](void* in, void*) {
		Dk::VideoStream* inVideo = static_cast<Dk::VideoStream*>(in);
		if(inVideo) {	
			switch(inVideo->getState()) {
				case Dk::PAUSED: inVideo->play(); break;
				case Dk::PLAYING: inVideo->pause(); break;
			}
		}
	}, (void*)(&video));
	
	// ----------------- Update continuously -----------------	
	Chronometre chrono;
	gui.show();
	while(gui.wait(5) != gui.KEY_ESCAPE) {
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