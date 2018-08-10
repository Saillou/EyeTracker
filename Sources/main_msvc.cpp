#include <iostream>

#include "Dk/ManagerConnection.hpp"
#include "Dk/VideoStream.hpp"
#include "Dk/Chronometre.hpp"

#include "Gui/CvGui.hpp"

using namespace CvGui;

// ---- Callbacks -----
void manageStream(void* in, void*) {
	Dk::VideoStream* inVideo = static_cast<Dk::VideoStream*>(in);
	if(inVideo) {	
		switch(inVideo->getState()) {
			case Dk::PAUSED: inVideo->play(); break;
			case Dk::PLAYING: inVideo->pause(); break;
		}
	}	
}

void quit(void* in, void*) {
	bool* inStop = static_cast<bool*>(in);
	if(inStop)
		*inStop = true;	
}

int main() {
	// ------------------------ Search server  ---------------------- //
	ManagerConnection managerConnection;
	managerConnection.initialize();
	
	auto dhcpv4 	= managerConnection.getMyDHCP();
	dhcpv4.setPort(3000);

	auto ipOpened 	= managerConnection.snif(dhcpv4+1, dhcpv4+255, 1);	
	if(ipOpened.size() == 0)
		return 0;
	// -------------------------------------------------------------- //
	
	// ----------------------- Video stream ------------------------- //
	Dk::VideoStream video(managerConnection, ipOpened[0]);
	
	if(!video.isValide())
		return 0;
	
	Protocole::FormatStream format = video.initFormat();
	if(!format)
		return 0;
	
	// Start stream [Play in its own thread]
	video.play();
	
	// ------ Create GUI ------
	cv::Mat frame = cv::Mat::zeros(format.height, format.width, format.channels == 1 ? CV_8UC1 : CV_8UC3);
	
	Gui<AddPolicy::Col> gui;
	auto interface0 = gui.createInterface();
	auto btn 	= std::make_shared<PushButton>("Play/Pause", cv::Size(150, 50));
	auto btnQ	= std::make_shared<PushButton>("Quit", cv::Size(150, 50));
	interface0->add(btn);
	interface0->add(btnQ);
	
	auto tbExposure 	= std::make_shared<TrackBar>("Exposure: ");
	auto cbExposure 	= std::make_shared<CheckBox>("Auto ");
	
	auto tbBrightness	= std::make_shared<TrackBar>("Brightness: ");
	auto tbContrast		= std::make_shared<TrackBar>("Contrast: ");
	auto tbHue			= std::make_shared<TrackBar>("Hue: ");
	auto tbSaturation	= std::make_shared<TrackBar>("Saturation: ");
	
	auto interfaceExpo 	= gui.createInterface();
	auto interface1 	= gui.createInterface();
	
	interfaceExpo->add(tbExposure);
	interfaceExpo->add(cbExposure);
	interface1->add(interfaceExpo, tbBrightness, tbContrast, tbHue, tbSaturation);
	
	auto interface2 = gui.createInterface();
	auto screen	= std::make_shared<Displayable>("Frame", frame);
	interface2->add(std::make_shared<Spacer>(cv::Size(25,25)));
	interface2->add(interface1);
	interface2->add(screen);
	interface2->add(std::make_shared<Spacer>(cv::Size(25,25)));
	gui.add(std::make_shared<Spacer>(cv::Size(25,25)), interface0, std::make_shared<Spacer>(cv::Size(25,25)), interface2, std::make_shared<Spacer>(cv::Size(25,25)));
	
	// Listen events
	bool stop = false;
	
	btn->listen(PushButton::onClick, manageStream, (void*)(&video));	// Play/Pause video
	btnQ->listen(PushButton::onClick, quit, (void*)(&stop));			// Quit application
	
	// ----------------- Update continuously -----------------	
	Chronometre chrono;
	gui.show();
	while(!stop) {
		// Display info
		if(chrono.clock_ms() >= 1000) { 			
			std::cout << "Freq: " << ((int)(10*video.getFpsRate()))/10.0 << " hz \t | \t Lag: " << video.getLag() << " ms." << std::endl;
			chrono.reset();
		}
		
		// Do things
		screen->setFrame(video.getFrame());
		
		// Sleep a bit
		gui.wait(30);
	}
	
	video.release();
	// -------------------------------------------------------------- //
	
	return 0;
}