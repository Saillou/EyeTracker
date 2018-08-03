#include <iostream>
#include <ctime>

#include "Gui/CvGui.hpp"

#include <opencv2/videoio.hpp>

using namespace CvGui;

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

// ------------------  M a i n ---------------------
int main() {
	// ----- Simulacre datas -----
	cv::Mat frame = cv::Mat::zeros(240, 320, CV_8UC3);
	
	// ------ Create GUI ------
	Gui<AddPolicy::Col> gui;
	auto interface0 = gui.createInterface("Interface_0");
	auto interface1 = gui.createInterface("Interface_1");
	
	auto label 		= std::make_shared<Label>("List of Widgets: ");
	auto trackbar 	= std::make_shared<TrackBar>("Move Me: ");
	auto button 	= std::make_shared<PushButton>("Click Me !", cv::Size(150, 50));
	auto checkBox 	= std::make_shared<CheckBox>("Check Me: ");
	auto screen		= std::make_shared<Displayable>("Frame", frame);
	auto spacer 	= std::make_shared<Spacer>(cv::Size(50, 250));
	
	interface0->add(spacer);
	interface0->add(label, trackbar, button, checkBox);
	interface0->add(spacer);
	
	interface1->add(screen);
	
	gui.add(interface0);
	gui.add(interface1);
	
	// Attach events
	trackbar->listen(TrackBar::onValueChanged, [=](void*, void*) {
		std::cout << "TrackBar: " << trackbar->getValue() << std::endl;
	});
	button->listen(PushButton::onClick, [=, &frame](void*, void*) {
		frame = cv::Mat::zeros(240, 320, CV_8UC3);
		std::cout << "Button clicked : frame reset." << std::endl;
	});
	checkBox->listen(CheckBox::onValueChanged, [=](void*, void*) {
		std::cout << "CheckBox: " << checkBox->getChecked() << std::endl;
	});

	// ------ Loop it ------	
	gui.show();
	while(gui.wait(30) != gui.KEY_ESCAPE) {
		// Do things
		drawRandom(frame);
		screen->setFrame(frame);
	}
	
	return 0;
}