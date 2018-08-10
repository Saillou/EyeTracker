#ifndef CVGUI_CHECKBOX_HPP
#define CVGUI_CHECKBOX_HPP

#include "../Widget.hpp"

namespace CvGui {

class CheckBox : public Widget {
public:	
	// Constructors
	explicit CheckBox(const std::string& text = "", bool state = false, const std::string& name = "CheckBox") : 
		Widget(name),
		_btnText(text),
		_isDown(false),
		_mouseOver(false),
		_isChecked(state)
	{
		_design();
	}
	
	~CheckBox() {
	}
	
	// Methods
	bool onEvent(int event, int /*x*/ = -1, int /*y*/ = -1) override {
		bool update = false;
		bool valueBefore = _isChecked;
		
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
				_isChecked = !_isChecked;
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
		if(valueBefore != _isChecked) 
			_invokeCallback(Widget::onValueChanged, &_isChecked);
		_invokeCallback(event);
		
		return update;
	}
	
	// Getters
	const std::string& getText() const {
		return _btnText;
	}
	const bool getChecked() const {
		return _isChecked;
	}
	
private:
	// Methods
	void _design() override {
		int baseLine(0);
		cv::Size textSize 	= cv::getTextSize(_btnText, cv::FONT_HERSHEY_DUPLEX, 0.5, 1, &baseLine);
		
		int frameMinWidthNeed = textSize.width + textSize.height + 15;
		cv::Size frameSize	= cv::Size(
			frameMinWidthNeed > 150 ? frameMinWidthNeed : 150,
			textSize.height + 10
		);
		
		_pFrame = cv::Mat(frameSize.height, frameSize.width, CV_8UC3, cv::Scalar(0, 0, 0));
		
		cv::putText(_pFrame, _btnText,
			cv::Point(5, textSize.height + baseLine), 	// Top-left position
			cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
			
		// Button
		cv::Scalar bkColor = _isDown ? cv::Scalar(42, 45, 45) : cv::Scalar(82 + 20*_mouseOver, 85 + 20*_mouseOver, 85 + 20*_mouseOver);
		if(_isChecked)
			bkColor = cv::Scalar(50, 255, 25);
		cv::rectangle(_pFrame, cv::Rect(cv::Point(textSize.width+5, baseLine + 1), cv::Size(textSize.height, textSize.height)), bkColor, -1);
	}

	// Member
	std::string _btnText;
	bool _isDown;
	bool _mouseOver;
	bool _isChecked;
};
	
}

#endif