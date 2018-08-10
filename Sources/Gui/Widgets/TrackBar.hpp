#ifndef CVGUI_TRACKBAR_HPP
#define CVGUI_TRACKBAR_HPP

#include "../Widget.hpp"

namespace CvGui {

class TrackBar : public Widget {
public:	
	// Constructors
	explicit TrackBar(const std::string& text = "", int min = 0, int max = 255, int value = 0, const std::string& name = "TrackBar") : 
		Widget(name),
		_lblText(text),
		_min(min),
		_max(max),
		_value(value),
		_isDown(false),
		_mouseOver(false),
		_barRect(cv::Rect()),
		_maxWidthValue(0)
	{
		// Don't be stupid
		if(_min > _max) {
			int tmp = _min;
			_min = _max;
			_max = tmp;
		}
			
		// Don't divide by 0
		if(_min == _max)
			_max += 1;
		
		// Don't resize frame
		_maxWidthValue = std::max(
			cv::getTextSize(std::to_string(_min), cv::FONT_HERSHEY_DUPLEX, 0.5, 1, nullptr).width,
			cv::getTextSize(std::to_string(_max), cv::FONT_HERSHEY_DUPLEX, 0.5, 1, nullptr).width
		);
		
		// Design
		_design();
	}
	
	~TrackBar() {
	}
	
	// Methods
	bool onEvent(int event, int x = -1, int /*y*/ = -1) override {
		bool update = false;
		int valueBefore = _value;
		
		switch(event) {
			// ---------- Button down ----------------
			case onButtonDown :
				_isDown = true;
				_value = _computeValue(x);	
				_design();
				
				update = true;
			break;
			
			// ---------- Button up ----------------
			case onButtonUp:
				_isDown = false;
				_design();
				
				update = true;
			break;
			
			case onMouseMove:
				if (!_isDown)
					break;
				
				_value = _computeValue(x);				
				_design();
				
				update = true;
			break;
			
			// ---------- Mouse over ----------------
			case onMouseIn :
				_mouseOver = true;
				_design();
				
				update = true;
			break;
			
			// ---------- Mouse out ----------------
			case onMouseOut:
				if (_isDown)
					_value = _computeValue(x);
				
				_mouseOver = false;
				_isDown = false;
				_design();
				
				update = true;
			break;
		}
		
		// Invoke callbacks
		if(valueBefore != _value) 
			_invokeCallback(Widget::onValueChanged, &_value);
		_invokeCallback(event);
		
		return update;
	}
	
	// Getters
	const std::string& getText() const {
		return _lblText;
	}
	const int getValue() const {
		return _value;
	}
	
private:
	// Methods
	int _computeValue(int xMouse) const {
		// Value compute
		int value = 0;
		
		if (xMouse < _barRect.x)
			value = _min;
		else if (xMouse > _barRect.x + _barRect.width)
			value = _max;
		else 
			value = (_max - _min) * (xMouse - _barRect.x) / _barRect.width + _min;
		
		return value;
	}
	void _design() override {
		// Create variables
		int baseLineL(0);
		const cv::Size txtLabelSize = cv::getTextSize(_lblText, cv::FONT_HERSHEY_DUPLEX, 0.5, 1, &baseLineL);
		
		int baseLineV(0);
		const std::string valueTxt = std::to_string(_value);
		const cv::Size txtValueSize = cv::getTextSize(valueTxt, cv::FONT_HERSHEY_DUPLEX, 0.5, 1, &baseLineV);
		
		const int widthBar = 100;
		const int heightBar = 5;
		
		const int buttonCote = 20;
		
		const int totalWidth  = 5 + txtLabelSize.width + 5 + widthBar + buttonCote + _maxWidthValue + 5;
		const int totalHeight = std::max(std::max(std::max(heightBar, buttonCote), txtLabelSize.height), txtValueSize.height) + 8;

		// Create frame
		_pFrame = cv::Mat(totalHeight, totalWidth, CV_8UC3, cv::Scalar(0, 0, 0));
		
		// Update variables for events
		_barRect = cv::Rect(
			cv::Point(5 + txtLabelSize.width + 5, (_pFrame.rows - heightBar) / 2), 
			cv::Size(widthBar, heightBar)
		);
		
		// Draw Label
		cv::putText(_pFrame, _lblText,
			cv::Point(0, (_pFrame.rows - txtLabelSize.height) / 2 + (int)1.5*baseLineL + 3),
			cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
			
		// Draw Value
		cv::putText(_pFrame, valueTxt,
			cv::Point(5 + txtLabelSize.width + 5 + widthBar + buttonCote, (_pFrame.rows - txtValueSize.height) / 2 + (int)1.5*baseLineV + 3), 	// Top-left position
			cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
			
		// Draw Bar
		cv::Scalar barColor = _isDown ? cv::Scalar(42, 45, 45) : cv::Scalar(82 + 20*_mouseOver, 85 + 20*_mouseOver, 85 + 20*_mouseOver);
		cv::rectangle(_pFrame, _barRect, barColor, -1);
			
		// Draw Button
		double p = (_value - _min) / (double)(_max- _min);

		cv::Scalar btnColor = _isDown ? cv::Scalar(220, 210, 210) : cv::Scalar(152 + 20*_mouseOver, 155 + 20*_mouseOver, 155 + 20*_mouseOver);
		cv::rectangle(_pFrame, 
			cv::Rect(
				cv::Point(_barRect.x - buttonCote/2 + (int)(p*_barRect.width), _barRect.y - _barRect.height/2 - buttonCote/2 + 5), 
				cv::Size((int)(buttonCote/1.6), buttonCote)), 
			btnColor, -1);
	}

	// Member
	std::string _lblText;
	int _min;
	int _max;
	int _value;
	bool _isDown;
	bool _mouseOver;
	cv::Rect _barRect;
	int _maxWidthValue;
};
	
}

#endif