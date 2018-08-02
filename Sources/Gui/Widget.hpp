#ifndef CVGUI_WIDGET_HPP
#define CVGUI_WIDGET_HPP

#include <iostream>
#include <functional>
#include <map>

#include "Displayable.hpp"

namespace CvGui {

// Base class for all widgets
class Widget : public Displayable {
public:
	enum EVENT_TYPE {
		noEvent			= -1,
		onMouseMove 	= cv::EVENT_MOUSEMOVE,
		onButtonDown 	= cv::EVENT_LBUTTONDOWN,
		onButtonUp 		= cv::EVENT_LBUTTONUP,
		onRButtonDown 	= cv::EVENT_RBUTTONDOWN,
		onRButtonUp 	= cv::EVENT_RBUTTONUP,
		onDoubleClick	= cv::EVENT_LBUTTONDBLCLK,
		onClick 		= 12, // Last opencv event is 11
		onRClick 		= 13,
		onMouseIn 		= 14,
		onMouseOut 		= 15,
		onValueChanged	= 16
	};
	
	struct Point {
		Point(int x_, int y_) : x(x_), y(y_) {
		}
		
		int x;
		int y;
	};
	struct Size {
		Size(unsigned int w_, unsigned int h_) : w(w_), h(h_) {
		}
		
		unsigned int w;
		unsigned int h;
	};
	struct Color {
		Color(unsigned char r_, unsigned char g_, unsigned char b_) : r(r_), g(g_), b(b_) {
		}
		
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};
	
public:
	// Constructors
	explicit Widget(const std::string& name = "Widget") : 
		Displayable(name)
	{
	}
	
	virtual ~Widget() {
	}
	
	// Methods
	virtual bool onEvent(int event, int x = -1, int y = -1) = 0; // Return true if (design)updated
	virtual void listen(int event, std::function<void(void*, void*)> f, void* data = nullptr) {
		_pCallbacks[event]		= f;
		_pDataCallbacks[event]	= data;
	}
	
	virtual void _invokeCallback(int event, void* dataWidget = nullptr) const {
		auto callback = _pCallbacks.find(event);
		
		if(callback != _pCallbacks.end()) {
			auto data = _pDataCallbacks.find(event);
			callback->second(data != _pDataCallbacks.end() ? data->second : nullptr, dataWidget);
		}
	}
	
	// Getters
	
protected:
	// Methods
	virtual void _design() = 0;
	
	// Members
	std::map<int, std::function<void(void*, void*)>> _pCallbacks;
	std::map<int, void*> _pDataCallbacks;
	
private:

};
	
}

#endif