#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "Dk/CvVideoCaptureProperties.hpp"

// ------------------  M a i n ---------------------
int main() {
	// Init variables
	cv::Mat frame;
	std::shared_ptr<cv::VideoCapture> pCap = std::make_shared<cv::VideoCapture>(0);
	
	// Properties
	Dk::CvProperties::CaptureProperties capProperties(pCap, Dk::CvProperties::Camera);
	
	for(auto p: capProperties.getAll()) {
		auto prop = p.second;
		if(prop.availability.manual) {
			std::cout << Dk::CvProperties::GetPropertyName(prop.id) << " = ";
			std::cout << prop.value.manual << std::endl;
		}
	}
	
	// Clean up
	pCap->release();
	pCap.reset();
	return 0;
}