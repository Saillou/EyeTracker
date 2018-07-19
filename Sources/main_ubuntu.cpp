#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <turbojpeg.h>

#include "Dk/ManagerConnection.hpp"
#include "Dk/Chronometre.hpp"
#include "Dk/VideoStreamWriter.hpp"

int main() {
	// -------------------- Open the cam --------------------
	cv::Mat frame = cv::Mat::zeros(480, 640, CV_8UC3);
	cv::VideoCapture cap(0);
	
	if(!cap.isOpened())
		return 0;
	
	// ---------------- Create Video Streamer ----------------
	ManagerConnection managerConnection;
	managerConnection.initialize();
	Dk::VideoStreamWriter videoWriter(managerConnection, 3000);
	
	if(!videoWriter.startBroadcast(frame))
		return 0;

	// ----------------- Update continuously -----------------
	cv::namedWindow("keyboard input");
	
	Chronometre chrono;		
	while(cv::waitKey(1) != 32) {
		cap >> frame;
		videoWriter.update(frame);
		
		// Display info
		if(chrono.elapsed_ms() >= 1000) { 			
			chrono.beg();
			int clockMs = (int)chrono.clock_ms();
			int mn 	= clockMs/60/1000;
			int sec	= clockMs/1000%60;
			std::cout << std::endl << (mn < 10 ? "0" : "") << mn << "'" << (sec < 10 ? "0" : "") << sec << std::endl;
		}
		
		Chronometre::wait(5); // Sleep a bit
	}
	
	videoWriter.release();
	return 0;	
}
