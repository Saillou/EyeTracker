#ifndef CVGUI_SPACER_HPP
#define CVGUI_SPACER_HPP

#include "../Widget.hpp"

namespace CvGui {

class Spacer : public Widget {
public:	
	// Constructors
	explicit Spacer(const cv::Size& s = cv::Size(150, 150), const std::string& name = "Spacer") : 
		Widget(name),
		_size(s)
	{
		_design();
	}
	
	~Spacer() {
	}
	
	// Methods
	bool onEvent(int /*event*/, int /*x*/= -1, int /*y*/= -1) {
		return false;
	}
	
	// Getters

	// Setters
	
private:
	// Methods
	void _design() override {
		_pFrame = cv::Mat(_size.height, _size.width, CV_8UC3, cv::Scalar::all(0));
	}

	// Member
	cv::Size _size;
};
	
}

#endif