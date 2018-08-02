#ifndef CVGUI_PUSHBUTTON_HPP
#define CVGUI_PUSHBUTTON_HPP

#include "../Widget.hpp"

namespace CvGui {

class PushButton : public Widget {
public:	
	// Constructors
	explicit PushButton(const std::string& text = "", const Size& s = Size(150, 150), const std::string& name = "PushButton") : 
		Widget(name),
		_size(s),
		_btnText(text),
		_isDown(false),
		_mouseOver(false)
	{
		_design();
	}
	
	~PushButton() {
	}
	
	// Methods
	bool onEvent(int event, int /*x*/ = -1, int /*y*/ = -1) override {
		bool update = false;
		
		switch(event) {
			// ---------- Button down ----------------
			case onButtonDown :
				_isDown = true;
				_design();
				
				update = true;
			break;
			
			// ---------- Button up ----------------
			case onButtonUp:
				_isDown = false;
				_design();
				
				update = true;
			break;
			
			// ---------- Left click ----------------
			case onClick :
				
			break;
			
			// ---------- Right click ----------------
			case onRClick :
				
			break;
			
			// ---------- Mouse over ----------------
			case onMouseIn :
				_mouseOver = true;
				_design();
				
				update = true;
			break;
			
			// ---------- Mouse out ----------------
			case onMouseOut:
				_mouseOver = false;
				_isDown = false;
				_design();
				
				update = true;
			break;
		}
		
		// Invoke callbacks
		_invokeCallback(event);
		
		return update;
	}
	
	// Getters
	const std::string& getText() const {
		return _btnText;
	}
	
private:
	// Methods
	void _design() override {
		int baseLine(0);
		cv::Size textSize 	= cv::getTextSize(_btnText, cv::FONT_HERSHEY_DUPLEX, 0.5, 1, &baseLine);
		cv::Size frameSize	= cv::Size (
			textSize.width > (int)_size.w-10 ? textSize.width + 10 : (int)_size.w,
			textSize.height > (int)_size.h-10 ? textSize.height + 10 : (int)_size.h
		);
		
		cv::Scalar bkColor = _isDown ? cv::Scalar(42, 45, 45) : cv::Scalar(82 + 20*_mouseOver, 85 + 20*_mouseOver, 85 + 20*_mouseOver);
		cv::Scalar ftColor = cv::Scalar(255, 255, 255) - bkColor;
		
		_pFrame = cv::Mat(frameSize.height, frameSize.width, CV_8UC3, bkColor);
		
		cv::rectangle(_pFrame, cv::Rect(cv::Point(0,0), frameSize), cv::Scalar(62, 65, 65), 1);
		
		cv::putText(_pFrame, _btnText,
			cv::Point((frameSize.width - textSize.width)/2, (_pFrame.rows - textSize.height) / 2 + (int)1.5*baseLine + 3), 	// Top-left position
			cv::FONT_HERSHEY_DUPLEX, 0.5,				// Font
			ftColor, 1, CV_AA);
	}

	// Member
	std::string _btnText;
	Size _size;
	bool _isDown;
	bool _mouseOver;
};
	
}

#endif