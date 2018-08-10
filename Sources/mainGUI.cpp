#include <iostream>
#include <ctime>

#include "Gui/CvGui.hpp"

using namespace CvGui;

// ---- Callbacks -----
void manageStream(void*, void*) {

}

void quit(void* in, void*) {
	bool* inStop = static_cast<bool*>(in);
	if(inStop)
		*inStop = true;	
}

void drawRandom(cv::Mat& output) {
	// Define constantes
	static const int RADIUS = 15;
	static const double MAX_X = output.cols - RADIUS, 	MIN_X = RADIUS;
	static const double MAX_Y = output.rows - RADIUS, 	MIN_Y = RADIUS;
	
	static const cv::Scalar BALL_COLOR 	= cv::Scalar(50, 50, 255);
	static const cv::Scalar SPEED_COLOR = cv::Scalar::all(255) - BALL_COLOR;
	
	// Define variables
	static double x	 = (MAX_X-MIN_X)/2, y  = (MAX_Y-MIN_Y)/2;
	static double vx = 0, 				vy = 0;
	
	// Generate accelerations and integre
	double ax = 0.05*(7*cos(clock()*0.01) + rand()%15 - 7);
	double ay = 0.05*(7*sin(clock()*0.01) + rand()%15 - 7);
	
	x += vx += ax;
	y += vy += ay;

	// Check conditions
	if(x > MAX_X || x < MIN_X) x += 2*(vx *= -1);
	if(y > MAX_Y || y < MIN_Y) y += 2*(vy *= -1);
	
	// Final Draw
	cv::Point center 	= cv::Point((int)x,(int)y);
	cv::Point direction = cv::Point((int)(5*vx), (int)(5*vy));
	
	cv::circle(output, center, RADIUS, BALL_COLOR, -1, cv::LINE_AA);
	cv::line(output, center, center + direction,SPEED_COLOR, 2, cv::LINE_AA);
}

int main() {
	// ------ Create GUI ------
	Gui<AddPolicy::Col> gui;
	cv::Mat frame(480, 640, CV_8UC3, cv::Scalar::all(0));
	cv::Size margin(25, 25);
	
	// Widgets
	auto btn 	= std::make_shared<PushButton>("Play/Pause", cv::Size(150, 50));
	auto btnQ	= std::make_shared<PushButton>("Quit", cv::Size(150, 50));
	auto screen	= std::make_shared<Displayable>("Frame", frame);
	
	auto tbExposure 	= std::make_shared<TrackBar>("Exposure: ");
	auto cbExposure 	= std::make_shared<CheckBox>("Auto ");
	
	auto tbBrightness	= std::make_shared<TrackBar>("Brightness: ");
	auto tbContrast		= std::make_shared<TrackBar>("Contrast: ");
	auto tbHue			= std::make_shared<TrackBar>("Hue: ");
	auto tbSaturation	= std::make_shared<TrackBar>("Saturation: ");
	
	// Interfaces
	auto interface0 	= gui.createInterface();
	interface0->add(btn);
	interface0->add(btnQ);
	
	auto interfaceExpo 	= gui.createInterface();
	interfaceExpo->add(tbExposure);
	interfaceExpo->add(cbExposure);
	
	auto interface1 	= gui.createInterface();
	interface1->add(interfaceExpo, tbBrightness, tbContrast, tbHue, tbSaturation);
	
	auto interface2 	= gui.createInterface();
	interface2->add(std::make_shared<Spacer>(margin));
	interface2->add(interface1);
	interface2->add(screen);
	interface2->add(std::make_shared<Spacer>(margin));
	
	gui.add(std::make_shared<Spacer>(margin), interface0, std::make_shared<Spacer>(margin), interface2, std::make_shared<Spacer>(margin));
	
	// Listen events
	bool stop = false;
	
	btn->listen(PushButton::onClick, manageStream, nullptr);	// Play/Pause video
	btnQ->listen(PushButton::onClick, quit, (void*)(&stop));	// Quit application
	

	gui.show();
	// ----------------- Update continuously -----------------	
	while(!stop) {		
		// Do things
		drawRandom(frame);
		screen->setFrame(frame);
		
		// Sleep a bit
		gui.wait(30);
	}
	// -------------------------------------------------------------- //
	
	return 0;
}