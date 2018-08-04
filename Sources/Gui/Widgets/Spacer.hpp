#ifndef CVGUI_SPACER_HPP
#define CVGUI_SPACER_HPP

#include "../Displayable.hpp"

namespace CvGui {

class Spacer : public Displayable {
public:	
	// Constructors
	explicit Spacer(const cv::Size& s = cv::Size(150, 150), const std::string& name = "Spacer") : 
		Displayable(name, cv::Mat::zeros(s.height, s.width, CV_8UC3))
	{
	}
	
	~Spacer() {
	}
	
	// Methods
	
	// Getters

	// Setters
	
private:
	// Methods

	// Member
};
	
}

#endif