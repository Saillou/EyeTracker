#include <iostream>

#include "Dk/ManagerConnection.hpp"
#include "Dk/VideoStream.hpp"
#include "Dk/Chronometre.hpp"

#include "Gui/CvGui.hpp"

using namespace CvGui;

Protocole::FormatStream format;

// --- Str ---
struct strChangeParam {
	int cvParam;
	Dk::VideoStream* ptrVideo;
};

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

void changeParam(void* in, void* out) {
	// Get values
	strChangeParam* ptrParam = static_cast<strChangeParam*>(in);
	if(!ptrParam)
		return;
	
	int* pTrackValue = static_cast<int*>(out);
	if(!pTrackValue)
		return;
	
	// Get datas
	int value 				= *pTrackValue;
	const int CV_PARAM 		= ptrParam->cvParam;
	Dk::VideoStream* video 	= ptrParam->ptrVideo;
	
	if(!video)
		return;
	
	// Do something
	switch(CV_PARAM) {
		case cv::CAP_PROP_EXPOSURE: 	format.exposure 	= value; break;
		case cv::CAP_PROP_BRIGHTNESS: 	format.brightness 	= value; break;
		case cv::CAP_PROP_CONTRAST: 	format.contrast 	= value; break;
		case cv::CAP_PROP_HUE: 			format.hue 			= value; break;
		case cv::CAP_PROP_SATURATION: 	format.saturation 	= value; break;
		
		default: return; // Nothing changed.
	}
	
	video->setFormat(format);
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
	
	format = video.initFormat();
	if(format.isEmpty())
		return 0;
	
	// Start stream [Play in its own thread]
	video.play();
	
	// ------ GUI ------
	cv::Mat frame = cv::Mat::zeros(format.height, format.width, format.channels == 1 ? CV_8UC1 : CV_8UC3);
	Gui<AddPolicy::Col> gui;
	
	// Widgets
	auto buttonPlay 	= std::make_shared<PushButton>("Play/Pause", 	cv::Size(150, 50));
	auto buttonQuit		= std::make_shared<PushButton>("Quit", 			cv::Size(150, 50));
	
	auto tbExposure 	= std::make_shared<TrackBar>("Exposure: ", 		-15, -1, (int)format.exposure);
	auto cbExposure 	= std::make_shared<CheckBox>("Auto ");
	
	auto tbBrightness	= std::make_shared<TrackBar>("Brightness: ", 	-100, 100, 	(int)format.brightness);
	auto tbContrast		= std::make_shared<TrackBar>("Contrast: ", 		0, 100, 	(int)format.contrast);
	auto tbHue			= std::make_shared<TrackBar>("Hue: ", 			-100, 100, 	(int)format.hue);
	auto tbSaturation	= std::make_shared<TrackBar>("Saturation: ", 	0, 100, 	(int)format.saturation);
	
	auto screen	= std::make_shared<Displayable>("Frame", frame);
	auto margin	= std::make_shared<Spacer>(cv::Size(25,25));
	
	// Interfaces
	auto interface0 	= gui.createInterface();
	interface0->add(margin, buttonPlay, margin);
	interface0->add(margin, buttonQuit, margin);
	
	auto interfaceExpo 	= gui.createInterface();
	interfaceExpo->add(tbExposure);
	interfaceExpo->add(cbExposure);
	
	auto interface1 	= gui.createInterface();
	interface1->add(interfaceExpo, tbBrightness, tbContrast, tbHue, tbSaturation);
	
	auto interface2 	= gui.createInterface();
	interface2->add(margin);
	interface2->add(interface1);
	interface2->add(margin, screen, margin);
	interface2->add(margin);
	
	gui.add(interface0, interface2);
	
	// Listen events
	bool stop = false;
	std::vector<strChangeParam> params {
		{cv::CAP_PROP_EXPOSURE, 	&video},
		{cv::CAP_PROP_BRIGHTNESS, 	&video},
		{cv::CAP_PROP_CONTRAST, 	&video},
		{cv::CAP_PROP_HUE, 			&video},
		{cv::CAP_PROP_SATURATION, 	&video}
	};

	buttonPlay->listen(PushButton::onClick, manageStream, (void*)(&video));	// Play/Pause video
	buttonQuit->listen(PushButton::onClick, quit, (void*)(&stop));			// Quit application
	
	tbExposure	->listen(TrackBar::onValueChanged, changeParam, (void*)(&params[0]));
	tbBrightness->listen(TrackBar::onValueChanged, changeParam, (void*)(&params[1]));
	tbContrast	->listen(TrackBar::onValueChanged, changeParam, (void*)(&params[2]));
	tbHue		->listen(TrackBar::onValueChanged, changeParam, (void*)(&params[3]));
	tbSaturation->listen(TrackBar::onValueChanged, changeParam, (void*)(&params[4]));
	
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