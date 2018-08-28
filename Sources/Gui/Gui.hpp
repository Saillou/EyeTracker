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
		_mouseCbkInitialized(false),
		_wasClosed(false)
	{
	}
	
	~Gui() {
	}
	
	// Methods
	std::shared_ptr<Interface<AddPolicy>> createInterface(const std::string& interfaceName = "Interface") {
		return std::make_shared<Interface<AddPolicy>>(interfaceName);
	}
	
	const int wait(unsigned int ms = 0) {
		this->_wasClosed = cv::getWindowProperty(this->_pName, 0) < 0;
		
		if(!this->_wasClosed) {
			this->_update();
			this->show();
		}
		return cv::waitKey(ms);
	}
	
	bool wasClosed() const {
		return this->_wasClosed;
	}
	
	// Constantes public members
	const int KEY_ESCAPE = 0x1B;
	const int KEY_SPACE = 0x20;
	
private:
	// Copy disabled
	Gui& operator=(const Gui&)  = delete;
	Gui(const Gui&) = delete;
	
	// Methods
	void _onShow() override {
		if(!_mouseCbkInitialized) {
			this->repaint();
			cv::setMouseCallback(
				this->_pName, 
				[](int event, int x, int y, int /*flags*/, void* data){
					Gui* _thisGui = static_cast<Gui*>(data);
					if(_thisGui)
						_thisGui->_dispatchMouseEvent(event, x, y);
				}, 
				this);
			this->_mouseCbkInitialized = true;
		}
	}
	
	void _onHide() override {
		cv::setMouseCallback(this->_pName, NULL, NULL);
		this->_mouseCbkInitialized = false;
	}
	
	// Members
	bool _mouseCbkInitialized;
	bool _wasClosed;
};

}

#endif
