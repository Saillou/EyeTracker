#ifndef CVGUI_INTERFACE_HPP
#define CVGUI_INTERFACE_HPP

#include <vector>
#include <algorithm>
#include <memory>
#include <type_traits>

#include "Policies.hpp"
#include "Widget.hpp"

namespace CvGui {
	
template <class _AddPolicy>
class Interface : public Displayable,  protected _AddPolicy {
	// Assert the policy inheritance
	static_assert(std::is_base_of<AddPolicy::Base, _AddPolicy>::value, "Policy template must inherit from AddPolicy");
	
	// ------------------------  N e s t  ----------------------------
private:
	struct _IntefacedObject {
		// Constructors
		explicit _IntefacedObject(std::shared_ptr<Displayable> ptrObject_) : 
			ptrObject(ptrObject_),
			x(0),
			y(0),
			width(ptrObject_ != nullptr ? ptrObject->getWidth() : 0),
			height(ptrObject_ != nullptr ? ptrObject->getHeight() : 0)
		{
			
		}
		
		// Methods
		const cv::Rect getRect() const {
			return cv::Rect(x, y, width, height);
		}
		void setPos(const std::pair<int,int>& pos) {
			x = pos.first;
			y = pos.second;
		}
		void translate(const std::pair<int,int>& vec) {
			setPos({x+vec.first, y+vec.second});
		}
		void updateOn(cv::Mat& frameDest) const {
			ptrObject->getFrame().copyTo(frameDest(getRect()));
		}
		bool contains(int xTarget, int yTarget) const {
			return 
				(x <= xTarget && x + width >= xTarget) && 
				(y <= yTarget && y + height >= yTarget);
		}
		
		// Members
		std::shared_ptr<Displayable> ptrObject;
		int x;
		int y;
		int width;
		int height;
	};
	// ------------------------------------------------------------------
	
public:	
	// Constructors
	Interface(const std::string name = "Interface") :
		Displayable(name),
		_memoWidgetEvent({nullptr, Widget::noEvent}),
		_tmpRes(std::make_shared<Displayable>("Temporary"+_pName))
	{
	}
	
	virtual ~Interface() {
	}
	
	// Methods
	template<typename ... PtrDisplayable>
	void add(const std::shared_ptr<Displayable>& input = nullptr, const PtrDisplayable&... otherInputs) {
		// Push to tmp following the policy
		if(input != nullptr) {
			_IntefacedObject iobj(input);
			std::pair<int,int> moveTmp, moveInput;
			this->_push(*_tmpRes, *input, *_tmpRes, &moveTmp, &moveInput);
			
			// Update pos in tmp and set the new one
			iobj.setPos(moveInput);
			for(auto& io : _tmpChildren)
				io.translate(moveTmp);
			
			// Temporary store (because of the recursivity)
			_tmpChildren.push_back(iobj);
		}
		// Resursive, continue while there are parameters
		size_t nbInputs = sizeof...(otherInputs);
		if(nbInputs > 0) {
			add(otherInputs...);
		}
		else {
			// Commit result
			std::pair<int,int> moveTmp, moveThis;
			this->_commit(*this, *_tmpRes, *this, &moveThis, &moveTmp);
			
			// Update all prev pos and tmp
			for(auto& io : _tmpChildren)
				io.translate(moveTmp);
			
			for(auto& io : _pChildren)
				io.translate(moveThis);
			
			// Finally store
			_pChildren.insert(_pChildren.end(), std::make_move_iterator(_tmpChildren.begin()), std::make_move_iterator(_tmpChildren.end()));
			_tmpChildren.clear();
			_tmpRes->reset("Temporary"+_pName);
			
			_pHasChanged = true;
		}
	}
	
	void repaint() {
		this->_update(true);
	}
	
	// Getters
	

protected:	
	// Methods
	virtual bool _update(bool forcedUpdate = false) {
		bool somethingChanged = _pHasChanged;
		
		for(_IntefacedObject child: _pChildren) {
			Displayable* childObj = child.ptrObject.get();
			if(!childObj)
				continue;
			
			// Interfaces ?
			Interface* childInterface = dynamic_cast<Interface*>(childObj);
			if(childInterface) {
				somethingChanged |= childInterface->_update(forcedUpdate);
			}
			
			// Every displayable
			if(childObj->hasChanged() || somethingChanged || forcedUpdate) {
				child.updateOn(_pFrame);
				childObj->setHasChanged(false);
				somethingChanged = true;
			}
		}
		_pHasChanged = false;
		
		return somethingChanged;
	}
	
	virtual void _eventOut() {
		// .. cancel events in memory
		if(_memoWidgetEvent.first != nullptr && _memoWidgetEvent.second != Widget::onMouseOut)
			this->_applyEvent(_memoWidgetEvent.first, Widget::onMouseOut);
		_memoWidgetEvent = {nullptr, Widget::noEvent};
	}
	
	void _dispatchMouseEvent(int event, int x, int y) {
		// Dispatch event to concerned children
		bool targetFound = false;
		
		for(auto& child: _pChildren) {
			Displayable* childObj = child.ptrObject.get();
			
			bool concerned = child.contains(x, y);
			targetFound |= concerned;
			
			// Change event's origin referentiel
			int nx = x - child.x;
			int ny = y - child.y;

			// Continue to propagate ?
			Interface* childInterface = dynamic_cast<Interface*>(childObj);
			if(childInterface) {
				if(concerned)
					childInterface->_dispatchMouseEvent(event, nx, ny);
				else
					childInterface->_eventOut();
				continue;
			}
			
			// Activate/Desactivate events ?
			Widget* childWidget = dynamic_cast<Widget*>(childObj);
			if(childWidget && concerned) {
				// Mouse in/out
				if(childWidget != _memoWidgetEvent.first) {
					if(_memoWidgetEvent.second != Widget::onMouseOut) {
						auto prevChild = _findObjFromWidget(_memoWidgetEvent.first);
						if(prevChild)
							this->_applyEvent(_memoWidgetEvent.first, Widget::onMouseOut, x - prevChild->x, y - prevChild->y, prevChild);
						this->_applyEvent(childWidget, Widget::onMouseIn, nx, ny, &child);
					}
				}
				else {
					// Click
					if(_memoWidgetEvent.second == Widget::onButtonDown && event == Widget::onButtonUp)
						this->_applyEvent(childWidget, Widget::onClick, nx, ny, &child);
					
					if(_memoWidgetEvent.second == Widget::onRButtonDown && event == Widget::onRButtonUp)
						this->_applyEvent(childWidget, Widget::onRClick, nx, ny, &child);
				}
				
				// Other event
				this->_applyEvent(childWidget, event, nx, ny, &child);
				continue;
			}
		}
		
		if(!targetFound)
			this->_eventOut();
	}
	
	
	// Members
	std::vector<_IntefacedObject> _pChildren;
	
private:
	// Disable copy
	Interface& operator=(const Interface &)  = delete;
	Interface(const Interface&) = delete;
	
	// Methods
	void _applyEvent(Widget* widget, int event, int x = -1, int y = -1, _IntefacedObject* objInterfaced = nullptr) {
		// If not defined, search object
		if(objInterfaced == nullptr)
			objInterfaced = this->_findObjFromWidget(widget);
		
		// If still not found, quit
		if(objInterfaced == nullptr)
			return;
		
		// Transmit event and update
		bool needUpdate = widget->onEvent(event, x, y);
		if(needUpdate)
			objInterfaced->updateOn(_pFrame);
		
		_memoWidgetEvent = {widget, event};
		_pHasChanged |= needUpdate;	
	}
	
	_IntefacedObject* _findObjFromWidget(Widget* widget) {		
		for(auto& child : _pChildren) {
			Widget* childWidget = dynamic_cast<Widget*>(child.ptrObject.get());
			if(childWidget && childWidget == widget) 
				return &child;
		}
		return nullptr;
	}
	
	// Members
	std::pair<Widget*, int> _memoWidgetEvent;
	std::shared_ptr<Displayable> _tmpRes;
	std::vector<_IntefacedObject> _tmpChildren;
	
};
	
} // Namespace CvGui

#endif
