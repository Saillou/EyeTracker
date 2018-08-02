#ifndef CVGUI_LABEL_HPP
#define CVGUI_LABEL_HPP

#include "../Widget.hpp"

namespace CvGui {

class Label : public Widget {
public:	
	// Constructors
	explicit Label(const std::string& text = "", const std::string& name = "Label") : 
		Widget(name),
		_text(text)
	{
		_design();
	}
	
	~Label() {
	}
	
	// Methods
	bool onEvent(int /*event*/, int /*x*/= -1, int /*y*/= -1) {
		return false;
	}
	
	// Getters

	
private:
	// Methods
	void _design() override {
		int baseLine(0);
		cv::Size textSize = cv::getTextSize(_text, cv::FONT_HERSHEY_DUPLEX, 0.5, 1, &baseLine);

		_pFrame = cv::Mat(textSize.height + 10 + baseLine, textSize.width + 10, CV_8UC3, cv::Scalar(0, 0, 0));
		cv::putText(_pFrame, _text,
			cv::Point(5, textSize.height + baseLine), 	// Top-left position
			cv::FONT_HERSHEY_DUPLEX, 0.5,				// Font
			cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
	}

	// Member
	std::string _text;
};
	
}

#endif