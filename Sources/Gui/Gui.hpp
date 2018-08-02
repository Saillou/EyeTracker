#ifndef CVGUI_GUI_HPP
#define CVGUI_GUI_HPP

#include "Interface.hpp"

namespace CvGui {

template <class AddPolicy>
class Gui : public Interface<AddPolicy> {
public:	
	// Constructors
	explicit Gui(const std::string name = "Gui") :
		Interface<AddPolicy>(name),
		_mouseCbkInitialized(false)
	{
	}
	
	~Gui() {
	}
	
	// Methods
	std::shared_ptr<Interface<AddPolicy>> createInterface(const std::string& interfaceName = "Interface") {
		std::shared_ptr<Interface<AddPolicy>> interface = std::make_shared<Interface<AddPolicy>>(interfaceName);
		
		return interface;
	}
	
	const int wait(unsigned int ms = 0) {
		_update();
		show();
		return cv::waitKey(ms);
	}
	
	// Constantes public members
	const int VK_ESCAPE = 0x1B;
	
	
private:
	// Copy disabled
	Gui& operator=(const Gui&)  = delete;
	Gui(const Gui&) = delete;
	
	// Methods
	void _onShow() override {
		if(!_mouseCbkInitialized) {
			repaint();
			cv::setMouseCallback(_pName, _onMouseEvent, this);
			_mouseCbkInitialized = true;
		}
	}
	void _onHide() override {
		cv::setMouseCallback(_pName, NULL, NULL);
		_mouseCbkInitialized = false;
	}
	
	friend void _onMouseEvent(int event, int x, int y, int /*flags*/, void* userdata) {
		Gui* _thisGui = static_cast<Gui*>(userdata);
		if(_thisGui)
			_thisGui->_dispatchMouseEvent(event, x, y);
	}
	
	// Members
	bool _mouseCbkInitialized;
};

}

#endif