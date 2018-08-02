#ifndef CVGUI_DISPLAYABLE_HPP
#define CVGUI_DISPLAYABLE_HPP

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <string.h>

namespace CvGui {
	
// Base class for all object which can be linked to a frame
class Displayable {
public:
	// Constructors
	explicit Displayable(const std::string& name = "Displayable", const cv::Mat& frame = cv::Mat()) :
		_pName(name),
		_pFrame(frame),
		_pHasChanged(true)
	{
	}
	virtual ~Displayable() {
	}
	
	// Methods
	virtual bool show() {
		if(!_pName.empty())
			cv::namedWindow(_pName);
		else
			return false;
		
		if(_pFrame.empty())
			return false;
		else 
			cv::imshow(_pName, _pFrame);
		
		_onShow();
		return true;
	}
	virtual bool hide() {
		cv::destroyWindow(_pName);
		
		_onHide();
		return true;
	}
	virtual void clear() {
		_pFrame = cv::Mat();
		_pName = "";
	}
	virtual void reset(const std::string& name = "Displayable", const cv::Mat& frame = cv::Mat()) {
		setFrame(frame);
		setName(name);
	}
	
	// Setters
	void setName(const std::string& name) {
		_pName = name;
	}
	void setFrame(const cv::Mat& frame) {
		_pFrame = frame.clone();
		_pHasChanged = true;
	}
	void setHasChanged(const bool b) {
		_pHasChanged = b;
	}
	
	// Getters
	virtual const std::string& getName() const {
		return _pName;
	}
	virtual const int getWidth() const {
		return _pFrame.cols;
	}
	virtual const int getHeight() const {
		return _pFrame.rows;
	}
	virtual const bool empty() const {
		return _pFrame.empty();
	}
	virtual const cv::Mat& getFrame() const {
		return _pFrame;
	}
	
	const bool hasChanged() const {
		return _pHasChanged;
	}
	
protected:
	// Methods
	virtual void _onShow() {
		// To be overrided
	}
	virtual void _onHide() {
		// To be overrided
	}
	
	// Members
	std::string _pName;
	cv::Mat _pFrame;
	bool _pHasChanged;
	
private:

};
	
}

#endif