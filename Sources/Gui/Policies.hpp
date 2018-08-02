#ifndef CVGUI_POLICIES_HPP
#define CVGUI_POLICIES_HPP

#include "Displayable.hpp"

namespace CvGui {
	
// Policies :
namespace AddPolicy {

struct Base {
protected:
	~Base(){
		
	}
	// For exemple : 10 elements in 3 blocks:
	// {0, 1, 2, 3}, {4, 5}, {6, 7, 8, 9}
	
	// Define how to concat elements into a block
	virtual void _push(const Displayable& input1, const Displayable& input2, Displayable& output, std::pair<int,int>* moveIn1 = nullptr, std::pair<int,int>* moveIn2 = nullptr) const = 0;
	
	// Define how to concat blocks
	virtual void _commit(const Displayable& input1, const Displayable& input2, Displayable& output, std::pair<int,int>* moveIn1 = nullptr, std::pair<int,int>* moveIn2 = nullptr) const = 0;
	
	// Helpers
	void horizontalAdd(const Displayable& input1, const Displayable& input2, Displayable& output, std::pair<int,int>* moveIn1 = nullptr, std::pair<int,int>* moveIn2 = nullptr) const {
		// Allocate frame
		int width 	= input1.getWidth() + input2.getWidth();
		int height	= std::max(input1.getHeight(), input2.getHeight());
		cv::Mat frameRes = cv::Mat::zeros(height, width, CV_8UC3);
		
		// Add input1
		int posLeft1	= 0;
		int posTop1	= (height - input1.getHeight()) / 2;
		_copy(posLeft1, posTop1, input1.getFrame(), frameRes);
		
		// Add input2
		int posLeft2	= input1.getWidth();
		int posTop2	= (height - input2.getHeight()) / 2;
		_copy(posLeft2, posTop2, input2.getFrame(), frameRes);
		
		// Send to output
		output.setFrame(frameRes);
		if(moveIn1) {
			moveIn1->first = posLeft1;
			moveIn1->second = posTop1;
		}
		if(moveIn2) {
			moveIn2->first = posLeft2;
			moveIn2->second = posTop2;
		}
	}
	void verticalAdd(const Displayable& input1, const Displayable& input2, Displayable& output, std::pair<int,int>* moveIn1 = nullptr, std::pair<int,int>* moveIn2 = nullptr) const {
		// Allocate frame
		int width 	= std::max(input1.getWidth(), input2.getWidth());
		int height	= input1.getHeight() + input2.getHeight();
		cv::Mat frameRes = cv::Mat::zeros(height, width, CV_8UC3);
		
		// Add input1
		int posLeft1	= (width - input1.getWidth()) / 2;
		int posTop1	= 0;
		_copy(posLeft1, posTop1, input1.getFrame(), frameRes);
		
		// Add input2
		int posLeft2	= (width - input2.getWidth()) / 2;
		int posTop2	= input1.getHeight();
		_copy(posLeft2, posTop2, input2.getFrame(), frameRes);
		
		// Send to output
		output.setFrame(frameRes);
		if(moveIn1) {
			moveIn1->first = posLeft1;
			moveIn1->second = posTop1;
		}
		if(moveIn2) {
			moveIn2->first = posLeft2;
			moveIn2->second = posTop2;
		}
	}
	
private:
	// Copy a frame into another
	void _copy(const int posLeft, const int posTop, const cv::Mat& src, cv::Mat& dest) const {
		if(!src.empty())
			src.copyTo(dest(cv::Rect(posLeft, posTop, src.cols, src.rows)));
	}
};

struct Row : public Base {
protected:
	// Policy is as follow:
	// 	0 - 1 - 2 - 3 	[Block 0]
	// 	4 - 5				[Block 1]
	// 	6 - 7 - 8 - 9 	[Block 2]
	
	void _push(const Displayable& input1, const Displayable& input2, Displayable& output, std::pair<int,int>* moveIn1 = nullptr, std::pair<int,int>* moveIn2 = nullptr) const override {
		return horizontalAdd(input1, input2, output, moveIn1, moveIn2);
	}
	void _commit(const Displayable& input1, const Displayable& input2, Displayable& output, std::pair<int,int>* moveIn1 = nullptr, std::pair<int,int>* moveIn2 = nullptr) const override {
		return verticalAdd(input1, input2, output, moveIn1, moveIn2);
	}
};

struct Col  : public Base {
protected:
	// Policy is as follow:
	// 	0 - 4 - 6 
	// 	1 - 5 - 7			
	// 	2 - 7 - 8 
	// 	3 - 7 - 9 
	//	 [0 - 1 - 2 Block]
	
	void _push(const Displayable& input1, const Displayable& input2, Displayable& output, std::pair<int,int>* moveIn1 = nullptr, std::pair<int,int>* moveIn2 = nullptr) const override {
		return verticalAdd(input1, input2, output, moveIn1, moveIn2);
	}
	void _commit(const Displayable& input1, const Displayable& input2, Displayable& output, std::pair<int,int>* moveIn1 = nullptr, std::pair<int,int>* moveIn2 = nullptr) const override {
		return horizontalAdd(input1, input2, output, moveIn1, moveIn2);
	}
};
	
	
} // AddPolicy 

} // CvGui

#endif