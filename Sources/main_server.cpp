#include <iostream>

#include "Dk/ManagerConnection.hpp"
#include "Dk/Chronometre.hpp"
#include "Dk/VideoStreamWriter.hpp"

#include "Gui/CvGui.hpp"

using namespace CvGui;

// ---- Callbacks ----
void quit(void* in, void*) {
	bool* inStop = static_cast<bool*>(in);
	if(inStop)
		*inStop = true;	
}

int main() {
	// -------------------- Open the cam --------------------
	std::shared_ptr<cv::VideoCapture> pCap = std::make_shared<cv::VideoCapture>(0);
	if(!pCap || !pCap->isOpened())
		return 0;
	
	// ---------------- Create Video Streamer ----------------
	ManagerConnection managerConnection;
	managerConnection.initialize();
	Dk::VideoStreamWriter videoWriter(managerConnection, 3000);
	
	Protocole::FormatStream format = videoWriter.startBroadcast(pCap);
	if(format.isEmpty())
		return 0;

	// ------ Create GUI ------
	Gui<AddPolicy::Col> gui;
	auto interface0 = gui.createInterface();
	auto button 	= std::make_shared<PushButton>("Stop", cv::Size(150, 50));

	interface0->add(button);
	gui.add(std::make_shared<Spacer>(cv::Size(25,25)), interface0, std::make_shared<Spacer>(cv::Size(25,25)));
	
	// Attach events
	bool stop = false;
	button->listen(PushButton::onClick, quit, (void*)(&stop));

	// ----------------- Update continuously -----------------	
	Chronometre chrono;		
	gui.show();
	while(!stop) {
		videoWriter.update();
		
		// Display info
		if(chrono.elapsed_ms() >= 1000) { 			
			chrono.beg();
			int clockMs = (int)chrono.clock_ms();
			int mn 	= clockMs/60/1000;
			int sec	= clockMs/1000%60;
			std::cout << std::endl << (mn < 10 ? "0" : "") << mn << "'" << (sec < 10 ? "0" : "") << sec << std::endl;
		}
		
		gui.wait(5);
	}

	videoWriter.release();
	return 0;	
}
